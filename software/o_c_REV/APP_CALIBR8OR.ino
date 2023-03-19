// Copyright (c) 2023, Nicholas J. Michalek
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/*
 * Based on a design spec from Chris Meyer / Alias Zone / Learning Modular
 */

#ifdef ENABLE_APP_CALIBR8OR

#include "HSApplication.h"
#include "HSMIDI.h"
#include "util/util_settings.h"
#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"
#include "SegmentDisplay.h"
#include "src/drivers/FreqMeasure/OC_FreqMeasure.h"

#define CAL8_MAX_TRANSPOSE 60
const int CAL8OR_PRECISION = 10000;

// channel configs
struct Cal8ChannelConfig {
    int scale;
    int root_note;
    int last_note; // for S&H mode

    uint8_t clocked_mode;
    int8_t offset; // fine-tuning offset
    int16_t scale_factor; // precision of 0.01% as an offset from 100%
    int8_t transpose; // in semitones
    int8_t transpose_active; // held value while waiting for trigger
};

// Preset storage spec
enum Cal8Settings {
    CAL8_DATA_VALID, // 1 bit

    CAL8_SCALE_A, // 12 bits
    CAL8_SCALEFACTOR_A, // 10 bits
    CAL8_OFFSET_A, // 8 bits
    CAL8_TRANSPOSE_A, // 8 bits
    CAL8_ROOTKEY_AND_CLOCKMODE_A, // 4 + 2 bits

    CAL8_SCALE_B,
    CAL8_SCALEFACTOR_B,
    CAL8_OFFSET_B,
    CAL8_TRANSPOSE_B,
    CAL8_ROOTKEY_AND_CLOCKMODE_B,

    CAL8_SCALE_C,
    CAL8_SCALEFACTOR_C,
    CAL8_OFFSET_C,
    CAL8_TRANSPOSE_C,
    CAL8_ROOTKEY_AND_CLOCKMODE_C,

    CAL8_SCALE_D,
    CAL8_SCALEFACTOR_D,
    CAL8_OFFSET_D,
    CAL8_TRANSPOSE_D,
    CAL8_ROOTKEY_AND_CLOCKMODE_D,

    CAL8_SETTING_LAST
};
enum Cal8Presets {
    CAL8_PRESET_A,
    CAL8_PRESET_B,
    CAL8_PRESET_C,
    CAL8_PRESET_D,

    NR_OF_PRESETS
};

enum Cal8Channel {
    CAL8_CHANNEL_A,
    CAL8_CHANNEL_B,
    CAL8_CHANNEL_C,
    CAL8_CHANNEL_D,

    NR_OF_CHANNELS
};

enum Cal8ClockMode {
    CONTINUOUS,
    TRIG_TRANS,
    SAMPLE_AND_HOLD,

    NR_OF_CLOCKMODES
};

const char * cal8_preset_id[4] = {"A", "B", "C", "D"};

class Calibr8orPreset : public settings::SettingsBase<Calibr8orPreset, CAL8_SETTING_LAST> {
public:
    bool is_valid() {
        return values_[CAL8_DATA_VALID];
    }
    bool load_preset(Cal8ChannelConfig *channel) {
        if (!is_valid()) return false; // don't try to load a blank

        int ix = 1; // skip validity flag

        for (int ch = 0; ch < NR_OF_CHANNELS; ++ch) {
            channel[ch].scale = values_[ix++];
            channel[ch].scale = constrain(channel[ch].scale, 0, OC::Scales::NUM_SCALES - 1);

            channel[ch].scale_factor = values_[ix++] - 500;
            channel[ch].offset = values_[ix++] - 63;
            channel[ch].transpose = values_[ix++] - CAL8_MAX_TRANSPOSE;

            uint32_t root_and_mode = uint32_t(values_[ix++]);
            channel[ch].clocked_mode = ((root_and_mode >> 4) & 0x03) % NR_OF_CLOCKMODES;
            channel[ch].root_note = constrain(int(root_and_mode & 0x0f), 0, 11);
        }

        return true;
    }
    void save_preset(Cal8ChannelConfig *channel) {
        int ix = 0;

        values_[ix++] = 1; // validity flag

        for (int ch = 0; ch < NR_OF_CHANNELS; ++ch) {
            values_[ix++] = channel[ch].scale;
            values_[ix++] = channel[ch].scale_factor + 500;
            values_[ix++] = channel[ch].offset + 63;
            values_[ix++] = channel[ch].transpose + CAL8_MAX_TRANSPOSE;
            values_[ix++] = ((channel[ch].clocked_mode & 0x03) << 4) | (channel[ch].root_note & 0x0f);
        }
    }

};

Calibr8orPreset cal8_presets[NR_OF_PRESETS];

class Calibr8or : public HSApplication {
public:
	enum Cal8EditMode {
        TRANSPOSE,
        TRACKING,

        NR_OF_EDITMODES
    };

	void Start() {
        segment.Init(SegmentSize::BIG_SEGMENTS);

        // make sure to turn this off, just in case
        FreqMeasure.end();
        OC::DigitalInputs::reInit();

        ClearPreset();
	}
	
    void ClearPreset() {
        for (int ch = 0; ch < NR_OF_CHANNELS; ++ch) {
            quantizer[ch].Init();
            channel[ch].scale = OC::Scales::SCALE_SEMI;
            quantizer[ch].Configure(OC::Scales::GetScale(channel[ch].scale), 0xffff);

            channel[ch].scale_factor = 0;
            channel[ch].offset = 0;
            channel[ch].root_note = 0;
            channel[ch].transpose = 0;
            channel[ch].clocked_mode = 0;
            channel[ch].last_note = 0;
        }
    }
    void LoadPreset() {
        bool success = cal8_presets[index].load_preset(channel);
        if (success) {
            for (int ch = 0; ch < NR_OF_CHANNELS; ++ch) {
                quantizer[ch].Configure(OC::Scales::GetScale(channel[ch].scale), 0xffff);
                quantizer[ch].Requantize();
            }
            preset_modified = 0;
        }
        else
            ClearPreset();
    }
    void SavePreset() {
        cal8_presets[index].save_preset(channel);
        preset_modified = 0;
    }

	void Resume() {
	}

    void Controller() {
        for (int ch = 0; ch < NR_OF_CHANNELS; ++ch) {
            bool clocked = Clock(ch);
            Cal8ChannelConfig &c = channel[ch];

            // clocked transpose
            if (CONTINUOUS == c.clocked_mode || clocked) {
                c.transpose_active = c.transpose;
            }

            // respect S&H mode
            if (c.clocked_mode != SAMPLE_AND_HOLD || clocked) {
                // CV value
                int pitch = In(ch);
                int quantized = quantizer[ch].Process(pitch, c.root_note * 128, c.transpose_active);
                c.last_note = quantized;
            }

            int output_cv = c.last_note * (CAL8OR_PRECISION + c.scale_factor) / CAL8OR_PRECISION;
            output_cv += c.offset;

            Out(ch, output_cv);
        }
    }

    void View() {
        gfxHeader("Calibr8or");

        if (preset_select) {
            gfxPrint(64, 1, "- Presets");
            DrawPresetSelector();
        }
        else {
            gfxPos(110, 1);
            if (preset_modified) gfxPrint("*");
            if (cal8_presets[index].is_valid()) gfxPrint(cal8_preset_id[index]);

            DrawInterface();
        }
    }


    /////////////////////////////////////////////////////////////////
    // Control handlers
    /////////////////////////////////////////////////////////////////
    void OnLeftButtonPress() {
        // Toggle between Transpose mode and Tracking Compensation
        // also doubles as Load or Save for preset select
        ++edit_mode %= NR_OF_EDITMODES;

        // prevent saving to the (clear) slot
        if (edit_mode && preset_select == 5) preset_select = 4;
    }

    void OnLeftButtonLongPress() {
        if (preset_select) return;

        // Toggle triggered transpose mode
        ++channel[sel_chan].clocked_mode %= NR_OF_CLOCKMODES;
        preset_modified = 1;
    }

    void OnRightButtonPress() {
        if (preset_select) {
            // special case to clear values
            if (!edit_mode && preset_select == NR_OF_PRESETS + 1) {
                ClearPreset();
                preset_modified = 1;
            }
            else {
                index = preset_select - 1;
                if (edit_mode) SavePreset();
                else LoadPreset();
            }

            preset_select = 0;
            return;
        }

        // Scale selection
        scale_edit = !scale_edit;
    }

    void OnUpButtonPress() {
        preset_select = 0;

        ++sel_chan %= NR_OF_CHANNELS;
    }

    void OnDownButtonPress() {
        preset_select = 0;

        if (--sel_chan < 0) sel_chan += NR_OF_CHANNELS;
    }

    void OnDownButtonLongPress() {
        // show preset screen, select last loaded
        preset_select = 1 + index;
    }

    // Left encoder: Octave or VScaling + Root Note
    void OnLeftEncoderMove(int direction) {
        if (preset_select) return;

        preset_modified = 1;
        if (scale_edit) {
            channel[sel_chan].root_note = constrain(channel[sel_chan].root_note + direction, 0, 11);
            quantizer[sel_chan].Requantize();
            return;
        }

        if (edit_mode == TRANSPOSE) { // Octave jump
            int s = OC::Scales::GetScale(channel[sel_chan].scale).num_notes;
            channel[sel_chan].transpose += (direction * s);
            while (channel[sel_chan].transpose > CAL8_MAX_TRANSPOSE) channel[sel_chan].transpose -= s;
            while (channel[sel_chan].transpose < -CAL8_MAX_TRANSPOSE) channel[sel_chan].transpose += s;
        }
        else { // Tracking compensation
            channel[sel_chan].scale_factor = constrain(channel[sel_chan].scale_factor + direction, -500, 500);
        }
    }

    // Right encoder: Semitones or Bias Offset + Scale Select
    void OnRightEncoderMove(int direction) {
        if (preset_select) {
            preset_select = constrain(preset_select + direction, 1, NR_OF_PRESETS + (1-edit_mode));
            return;
        }

        preset_modified = 1;
        if (scale_edit) {
            int s_ = channel[sel_chan].scale + direction;
            if (s_ >= OC::Scales::NUM_SCALES) s_ = 0;
            if (s_ < 0) s_ = OC::Scales::NUM_SCALES - 1;

            channel[sel_chan].scale = s_;
            quantizer[sel_chan].Configure(OC::Scales::GetScale(s_), 0xffff);
            quantizer[sel_chan].Requantize();
            return;
        }

        if (edit_mode == TRANSPOSE) {
            channel[sel_chan].transpose = constrain(channel[sel_chan].transpose + direction, -CAL8_MAX_TRANSPOSE, CAL8_MAX_TRANSPOSE);
        }
        else {
            channel[sel_chan].offset = constrain(channel[sel_chan].offset + direction, -63, 64);
        }
    }

private:
    int index = 0;

	int sel_chan = 0;
    int edit_mode = 0; // Cal8EditMode
    bool scale_edit = 0;
    int preset_select = 0; // both a flag and an index
    bool preset_modified = 0;

    SegmentDisplay segment;
    braids::Quantizer quantizer[NR_OF_CHANNELS];
    Cal8ChannelConfig channel[NR_OF_CHANNELS];

    void DrawPresetSelector() {
        // TODO: Preset selection screen
        // index is the currently loaded preset (0-3)
        // preset_select is current selection (1-4, 5=clear)
        int y = 5 + 10*preset_select;
        gfxPrint(25, y, edit_mode ? "Save" : "Load");
        gfxIcon(50, y, RIGHT_ICON);

        for (int i = 0; i < NR_OF_PRESETS; ++i) {
            gfxPrint(60, 15 + i*10, cal8_preset_id[i]);
            if (!cal8_presets[i].is_valid())
                gfxPrint(" (empty)");
            else if (i == index)
                gfxPrint(" *");
        }
        if (!edit_mode)
            gfxPrint(60, 55, "[CLEAR]");
    }

    void DrawInterface() {
        // Draw channel tabs
        for (int i = 0; i < NR_OF_CHANNELS; ++i) {
            gfxLine(i*32, 13, i*32, 22); // vertical line on left
            if (channel[i].clocked_mode) gfxIcon(2 + i*32, 14, CLOCK_ICON);
            if (channel[i].clocked_mode == SAMPLE_AND_HOLD) gfxIcon(22 + i*32, 14, STAIRS_ICON);
            gfxPrint(i*32 + 13, 14, i+1);

            if (i == sel_chan)
                gfxInvert(1 + i*32, 13, 31, 11);
        }
        gfxLine(127, 13, 127, 22); // vertical line
        gfxLine(0, 23, 127, 23);

        // Draw parameters for selected channel
        int y = 32;

        // Transpose
        gfxIcon(9, y, BEND_ICON);

        // -- LCD Display Section --
        gfxFrame(20, y-3, 64, 18);
        gfxIcon(23, y+2, (channel[sel_chan].transpose >= 0)? PLUS_ICON : MINUS_ICON);

        int s = OC::Scales::GetScale(channel[sel_chan].scale).num_notes;
        int octave = channel[sel_chan].transpose / s;
        int semitone = channel[sel_chan].transpose % s;
        segment.PrintWhole(33, y, abs(octave), 10);
        gfxPrint(53, y+5, ".");
        segment.PrintWhole(61, y, abs(semitone), 10);

        // Scale
        gfxIcon(89, y, SCALE_ICON);
        gfxPrint(99, y, OC::scale_names_short[channel[sel_chan].scale]);
        if (scale_edit) {
            gfxInvert(98, y-1, 29, 9);
            gfxIcon(100, y+10, RIGHT_ICON);
        }
        // Root Note
        gfxPrint(110, y+10, OC::Strings::note_names_unpadded[channel[sel_chan].root_note]);

        // Tracking Compensation
        y += 22;
        gfxIcon(9, y, ZAP_ICON);
        int whole = (channel[sel_chan].scale_factor + CAL8OR_PRECISION) / 100;
        int decimal = (channel[sel_chan].scale_factor + CAL8OR_PRECISION) % 100;
        gfxPrint(20 + pad(100, whole), y, whole);
        gfxPrint(".");
        if (decimal < 10) gfxPrint("0");
        gfxPrint(decimal);
        gfxPrint("% ");

        if (channel[sel_chan].offset >= 0) gfxPrint("+");
        gfxPrint(channel[sel_chan].offset);

        // mode indicator
        if (!scale_edit)
            gfxIcon(0, 32 + edit_mode*22, RIGHT_ICON);
    }
};

SETTINGS_DECLARE(Calibr8orPreset, CAL8_SETTING_LAST) {
    {0, 0, 1, "validity flag", NULL, settings::STORAGE_TYPE_U4},

    {0, 0, 65535, "Scale A", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "CV Scaling Factor A", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 255, "Offset Bias A", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Transpose A", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Root Key + Mode A", NULL, settings::STORAGE_TYPE_U8},

    {0, 0, 65535, "Scale B", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "CV Scaling Factor B", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 255, "Offset Bias B", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Transpose B", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Root Key + Mode B", NULL, settings::STORAGE_TYPE_U8},

    {0, 0, 65535, "Scale C", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "CV Scaling Factor C", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 255, "Offset Bias C", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Transpose C", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Root Key + Mode C", NULL, settings::STORAGE_TYPE_U8},

    {0, 0, 65535, "Scale D", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 65535, "CV Scaling Factor D", NULL, settings::STORAGE_TYPE_U16},
    {0, 0, 255, "Offset Bias D", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Transpose D", NULL, settings::STORAGE_TYPE_U8},
    {0, 0, 255, "Root Key + Mode D", NULL, settings::STORAGE_TYPE_U8}
};


Calibr8or Calibr8or_instance;

// App stubs
void Calibr8or_init() { Calibr8or_instance.BaseStart(); }

size_t Calibr8or_storageSize() {
    return Calibr8orPreset::storageSize() * NR_OF_PRESETS;
}

size_t Calibr8or_save(void *storage) {
    size_t used = 0;
    for (int i = 0; i < 4; ++i) {
        used += cal8_presets[i].Save(static_cast<char*>(storage) + used);
    }
    return used;
}

size_t Calibr8or_restore(const void *storage) {
    size_t used = 0;
    for (int i = 0; i < 4; ++i) {
        used += cal8_presets[i].Restore(static_cast<const char*>(storage) + used);
    }
    Calibr8or_instance.LoadPreset();
    return used;
}

void Calibr8or_isr() { return Calibr8or_instance.BaseController(); }

void Calibr8or_handleAppEvent(OC::AppEvent event) {
    switch (event) {
    case OC::APP_EVENT_RESUME:
        Calibr8or_instance.Resume();
        break;

    // The idea is to auto-save when the screen times out...
    case OC::APP_EVENT_SUSPEND:
    case OC::APP_EVENT_SCREENSAVER_ON:
        // TODO: initiate actual EEPROM save
        // app_data_save();
        break;

    default: break;
    }
}

void Calibr8or_loop() {} // Deprecated

void Calibr8or_menu() { Calibr8or_instance.BaseView(); }

void Calibr8or_screensaver() {
    // XXX: Consider a view like Quantermain
    // other ideas: Actual note being played, current transpose setting
    // ...for all 4 channels at once.
}

void Calibr8or_handleButtonEvent(const UI::Event &event) {
    // For left encoder, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_L) {
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) Calibr8or_instance.OnLeftButtonLongPress();
        else Calibr8or_instance.OnLeftButtonPress();
    }

    // For right encoder, only handle press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_R && event.type == UI::EVENT_BUTTON_PRESS) Calibr8or_instance.OnRightButtonPress();

    // For up button, handle only press (long press is reserved)
    if (event.control == OC::CONTROL_BUTTON_UP) Calibr8or_instance.OnUpButtonPress();

    // For down button, handle press and long press
    if (event.control == OC::CONTROL_BUTTON_DOWN) {
        if (event.type == UI::EVENT_BUTTON_PRESS) Calibr8or_instance.OnDownButtonPress();
        if (event.type == UI::EVENT_BUTTON_LONG_PRESS) Calibr8or_instance.OnDownButtonLongPress();
    }
}

void Calibr8or_handleEncoderEvent(const UI::Event &event) {
    // Left encoder turned
    if (event.control == OC::CONTROL_ENCODER_L) Calibr8or_instance.OnLeftEncoderMove(event.value);

    // Right encoder turned
    if (event.control == OC::CONTROL_ENCODER_R) Calibr8or_instance.OnRightEncoderMove(event.value);
}

#endif // ENABLE_APP_CALIBR8OR
