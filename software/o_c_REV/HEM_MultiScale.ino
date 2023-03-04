// Copyright (c) 2023, Jakob Zerbian
//
// Based on Braids Quantizer, Copyright 2015 Émilie Gillet.
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

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

#define MS_QUANT_SCALES_COUNT 4

class MultiScale : public HemisphereApplet {
public:

    const char* applet_name() {
        return "MultiScale";
    }

    void Start() {
        // init all scales no all notes
        for (uint8_t i = 0; i < MS_QUANT_SCALES_COUNT; i++) {
            scale_mask[i] = 0x0001;
        }
        quantizer.Init();
        quantizer.Configure(OC::Scales::GetScale(5), scale_mask[0]);
    }

    void Controller() {

        // user selected scale from input 2
        int scale = DetentedIn(1);
        if (scale < 0) scale = 0;        
        if (scale > 0) {
            scale = constrain(ProportionCV(scale, MS_QUANT_SCALES_COUNT + 1), 0, MS_QUANT_SCALES_COUNT);
        }
        if (scale != current_scale) {
            current_scale = scale;
            quantizer.Configure(OC::Scales::GetScale(5), scale_mask[current_scale]);
            ClockOut(1); // send clock at second output
        }

        // quantize notes from input 1, can be clocked
        if (Clock(0)) {
            continuous = false;
            StartADCLag(0);
        }

        if (continuous || EndOfADCLag(0)) {

            int32_t pitch = In(0);
            int32_t quantized = quantizer.Process(pitch, 0, 0);
            Out(0, quantized);
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawKeyboard();
        DrawIndicators();
    }

    void OnButtonPress() {
        if (cursor == 0) { // scale page selection mode
            change_page = !change_page;
        } else { // scale note edit mode
            uint8_t bit = cursor - 1;
            scale_mask[scale_page] ^= (0x01 << bit); // togle bit at position
            if (scale_page == current_scale) {
                quantizer.Configure(OC::Scales::GetScale(5), scale_mask[current_scale]);
            }
        }

    }

    void OnEncoderMove(int direction) {
        if (change_page) {
            // select page
            scale_page = constrain(scale_page + direction, 0, MS_QUANT_SCALES_COUNT - 1);
        } else {
            // move cursor along the "keyboard"
            cursor += direction;
            if (cursor > 12) cursor = 0;
            if (cursor < 0) cursor = 12;
        }
        ResetCursor();
    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        Pack(data, PackLocation {0,12}, scale_mask[0]);
        Pack(data, PackLocation {12,12}, scale_mask[1]);
        Pack(data, PackLocation {24,8}, scale_mask[2]);
        return data;
    }

    void OnDataReceive(uint32_t data) {
        scale_mask[0] = Unpack(data, PackLocation {0,12});
        scale_mask[1] = Unpack(data, PackLocation {12,12});
        scale_mask[2] = Unpack(data, PackLocation {24, 8});
        quantizer.Configure(OC::Scales::GetScale(5), scale_mask[0]);
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS]      = "1=CV     2=Scale";
        help[HEMISPHERE_HELP_OUTS]     = "A=Pitch  B=Gate";
        help[HEMISPHERE_HELP_ENCODER]  = "Edit Scales";
        //                               "------------------" <-- Size Guide
    }
    
private:
    braids::Quantizer quantizer;
    uint16_t scale_mask[MS_QUANT_SCALES_COUNT];

    bool continuous = true;

    int8_t current_scale = 0;
    int8_t cursor = 0;
    int8_t scale_page = 0;
    bool change_page = 0;

    #define X_ 4
    
    void DrawKeyboard() {
        gfxFrame(4, 27, 56, 32);

        // white keys
        for (uint8_t x = 0; x < 7; x++) {
            gfxLine(x * 8 + 4, 27, x*8 + 4, 58);
        }

        // black keys
        for (uint8_t i = 0; i < 6; i++) {
            if (i != 2) {
                uint8_t x = (i * 8) + 10;
                gfxRect(x, 27, 5, 16);
            }
        }

        gfxBitmap(1, 14, 8, PLAY_ICON);
        gfxPrint(12, 15, current_scale);

        gfxBitmap(32, 14, 8, EDIT_ICON);
        gfxPrint(43, 15, scale_page);
        
    }

    void DrawIndicators() {
        if (cursor == 0) {
            // draw cursor at scale page text
            if (change_page) {
                gfxInvert(31, 13, 20, 10);
            } else {
                gfxCursor(31, 23, 20);
            }
        }
        uint8_t x[12] = {2, 7, 10, 15, 18, 26, 31, 34, 39, 42, 47, 50};
        uint8_t p[12] = {0, 1,  0,  1,  0,  0,  1,  0,  1,  0,  1,  0};
        for (uint8_t i = 0; i < 12; i++) {
            if ((scale_mask[scale_page] >> i) & 0x01) gfxInvert(x[i] + 4, (p[i] ? 37 : 51), 4 - p[i], 4 - p[i]);
            
            if (i == (cursor - 1)) gfxCursor(x[i] + 3, p[i] ? 25 : 60, 6);
        }
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to MultiScale,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
MultiScale MultiScale_instance[2];

void MultiScale_Start(bool hemisphere) {MultiScale_instance[hemisphere].BaseStart(hemisphere);}
void MultiScale_Controller(bool hemisphere, bool forwarding) {MultiScale_instance[hemisphere].BaseController(forwarding);}
void MultiScale_View(bool hemisphere) {MultiScale_instance[hemisphere].BaseView();}
void MultiScale_OnButtonPress(bool hemisphere) {MultiScale_instance[hemisphere].OnButtonPress();}
void MultiScale_OnEncoderMove(bool hemisphere, int direction) {MultiScale_instance[hemisphere].OnEncoderMove(direction);}
void MultiScale_ToggleHelpScreen(bool hemisphere) {MultiScale_instance[hemisphere].HelpScreen();}
uint32_t MultiScale_OnDataRequest(bool hemisphere) {return MultiScale_instance[hemisphere].OnDataRequest();}
void MultiScale_OnDataReceive(bool hemisphere, uint32_t data) {MultiScale_instance[hemisphere].OnDataReceive(data);}
