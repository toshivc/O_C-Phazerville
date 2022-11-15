// Copyright (c) 2018, Jason Justian
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

/*
 * Turing Machine based on https://thonk.co.uk/documents/random%20sequencer%20documentation%20v2-1%20THONK%20KIT_LargeImages.pdf
 *
 * Thanks to Tom Whitwell for creating the concept, and for clarifying some things
 * Thanks to Jon Wheeler for the CV length and probability updates
 *
 * adapted as DualTM by djphazer (Nicholas J. Michalek)
 */

#include "braids_quantizer.h"
#include "braids_quantizer_scales.h"
#include "OC_scales.h"

#define TM2_MAX_SCALE OC::Scales::NUM_SCALES

#define TM2_MIN_LENGTH 2
#define TM2_MAX_LENGTH 32

class DualTM : public HemisphereApplet {
public:

    const char* applet_name() {
        return "DualTM";
    }

    void Start() {
        reg = random(0, 65535);
        reg2 = ~reg;
        p = 0;
        length = 16;
        quant_range = 24;  //APD: Quantizer range
        cursor = 0;
        quantizer.Init();
        scale = OC::Scales::SCALE_SEMI;
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff); // Semi-tone
    }

    void Controller() {

        // CV 1 control over length
        int lengthCv = DetentedIn(0);
        if (lengthCv < 0) length = TM2_MIN_LENGTH;
        if (lengthCv > 0) {
            length = constrain(ProportionCV(lengthCv, TM2_MAX_LENGTH + 1), TM2_MIN_LENGTH, TM2_MAX_LENGTH);
        }
      
        // CV 2 bi-polar modulation of probability
        int pCv = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 100);
        bool clk = Clock(0);
        
        if (clk) {
            // If the cursor is not on the p value, and Digital 2 is not gated, the sequence remains the same
            int prob = (cursor == 1 || Gate(1)) ? p + pCv : 0;
            prob = constrain(prob, 0, 100);

            // Grab the bit that's about to be shifted away
            int last = (reg >> (length - 1)) & 0x01;
            int last2 = (reg2 >> (length - 1)) & 0x01;

            // Does it change?
            if (random(0, 99) < prob) last = 1 - last;
            if (random(0, 99) < prob) last2 = 1 - last2;

            // Shift left, then potentially add the bit from the other side
            reg = (reg << 1) + last;
            reg2 = (reg2 << 1) + last2;
        }
 
        // Send 5-bit scaled and quantized CV
        // scaled = note * quant_range / 0x1f
        int32_t note = Proportion(reg & 0x1f, 0x1f, quant_range);
        int32_t note2 = Proportion(reg2 & 0x1f, 0x1f, quant_range);

        /*
        note *= quant_range;
        simfloat x = int2simfloat(note) / (int32_t)0x1f;
        note = simfloat2int(x);

        Out(0, quantizer.Lookup(note + 64));
        */

        ForEachChannel(ch) {
            switch (outmode[ch]) {
            case 0: // pitch 1
              Out(ch, quantizer.Lookup(note + 64));
              break;
            case 1: // pitch 2
              Out(ch, quantizer.Lookup(note2 + 64));
              break;
            case 2: // mod A - 8-bit proportioned CV
              Out(ch, Proportion(reg & 0x00ff, 255, HEMISPHERE_MAX_CV) );
              break;
            case 3: // mod B - 8-bit proportioned CV
              Out(ch, Proportion(reg2 & 0x00ff, 255, HEMISPHERE_MAX_CV) );
              break;
            case 4: // trig A
              if (clk && (reg & 0x01) == 1) // trigger if 1st bit is high
                ClockOut(ch);
              break;
            case 5: // trig B
              if (clk && (reg2 & 0x01) == 1) // trigger if 1st bit is high
                ClockOut(ch);
              break;
            case 6: // gate A
              Out(ch, (reg & 0x01)*HEMISPHERE_MAX_CV);
              break;
            case 7: // gate B
              Out(ch, (reg2 & 0x01)*HEMISPHERE_MAX_CV);
              break;
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        DrawIndicator();
    }

    void OnButtonPress() {
        isEditing = !isEditing;
    }

    void OnEncoderMove(int direction) {
        if (!isEditing) {
            cursor += direction;
            if (cursor < 0) cursor = 5;
            if (cursor > 5) cursor = 0;

            ResetCursor();  // Reset blink so it's immediately visible when moved
        } else {
            switch (cursor) {
            case 0:
                length = constrain(length + direction, TM2_MIN_LENGTH, TM2_MAX_LENGTH);
                break;
            case 1:
                p = constrain(p + direction, 0, 100);
                break;
            case 2:
                scale += direction;
                if (scale >= TM2_MAX_SCALE) scale = 0;
                if (scale < 0) scale = TM2_MAX_SCALE - 1;
                quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);
                break;
            case 3:
                quant_range = constrain(quant_range + direction, 1, 32);
                break;
            case 4:
                outmode[0] = constrain(outmode[0] + direction, 0, 7);
                break;
            case 5:
                outmode[1] = constrain(outmode[1] + direction, 0, 7);
                break;
            }
        }
    }
        
    uint64_t OnDataRequest() {
        uint64_t data = 0;
        Pack(data, PackLocation {0,7}, p);
        Pack(data, PackLocation {7,5}, length - 1);
        Pack(data, PackLocation {12,5}, quant_range - 1);
        Pack(data, PackLocation {17,3}, outmode[0]);
        Pack(data, PackLocation {20,3}, outmode[1]);
        Pack(data, PackLocation {23,8}, constrain(scale, 0, 255));

        Pack(data, PackLocation {32,32}, reg);

        return data;
    }

    void OnDataReceive(uint64_t data) {
        p = Unpack(data, PackLocation {0,7});
        length = Unpack(data, PackLocation {7,5}) + 1;
        quant_range = Unpack(data, PackLocation{12,5}) + 1;
        outmode[0] = Unpack(data, PackLocation {17,3});
        outmode[1] = Unpack(data, PackLocation {20,3});
        scale = Unpack(data, PackLocation {23,8});
        quantizer.Configure(OC::Scales::GetScale(scale), 0xffff);

        reg = Unpack(data, PackLocation {32,32});
        reg2 = ~reg;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=p Gate";
        help[HEMISPHERE_HELP_CVS]      = "1=Length 2=p Mod";
        help[HEMISPHERE_HELP_OUTS]     = "A=Quant5-bit B=CV2";
        help[HEMISPHERE_HELP_ENCODER]  = "Len/Prob/Scl/Range";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int length; // Sequence length
    int cursor;  // 0 = length, 1 = p, 2 = scale
    bool isEditing = false;
    braids::Quantizer quantizer;

    // Settings
    uint32_t reg; // 32-bit sequence register
    uint32_t reg2; // DJP
    int p; // Probability of bit flipping on each cycle
    int scale; // Scale used for quantized output
    int quant_range;

    // output modes:
    // 0=pitch A; 2=mod A; 4=trig A; 6=gate A
    // 1=pitch B; 3=mod B; 5=trig B; 7=gate B
    int outmode[2] = {0, 5};

    void DrawSelector() {
        gfxBitmap(1, 14, 8, LOOP_ICON);
        gfxPrint(12 + pad(10, length), 15, length);
        gfxPrint(32, 15, "p=");
        if (cursor == 1 || Gate(1)) { // p unlocked
            int pCv = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 100);
            int prob = constrain(p + pCv, 0, 100);
            gfxPrint(pad(100, prob), prob);
        } else { // p is disabled
            gfxBitmap(49, 14, 8, LOCK_ICON);
        }
        gfxBitmap(1, 24, 8, SCALE_ICON);
        gfxPrint(12, 25, OC::scale_names_short[scale]);
        gfxBitmap(41, 24, 8, NOTE4_ICON);
        gfxPrint(49, 25, quant_range); // APD
        gfxPrint(1, 35, "A:");
        gfxPrint(32, 35, "B:");

        ForEachChannel(ch) {
            switch (outmode[ch]) {
            case 0: // pitch output
            case 1:
                gfxBitmap(13 + ch*32, 35, 8, NOTE_ICON);
                break;
            case 2: // mod output
            case 3:
                gfxBitmap(13 + ch*32, 35, 8, WAVEFORM_ICON);
                break;
            case 4: // trig output
            case 5:
                gfxBitmap(13 + ch*32, 35, 8, CLOCK_ICON);
                break;
            case 6: // gate output
            case 7:
                gfxBitmap(13 + ch*32, 35, 8, METER_ICON);
                break;
            }
            // indicator for reg1 or reg2
            gfxBitmap(22+ch*32, 35, 3, (outmode[ch] % 2) ? SUB_TWO : SUP_ONE);
        }

        switch (cursor) {
            case 0: gfxCursor(13, 23, 12); break; // Length Cursor
            case 1: gfxCursor(45, 23, 18); break; // Probability Cursor
            case 2: gfxCursor(12, 33, 25); break; // Scale Cursor
            case 3: gfxCursor(49, 33, 14); break; // Quant Range Cursor // APD
            case 4: gfxCursor(12, 43, 10); break; // Out A
            case 5: gfxCursor(44, 43, 10); break; // Out B
        }
        if (isEditing) {
            switch (cursor) {
                case 0: gfxInvert(13, 14, 12, 9); break;
                case 1: gfxInvert(45, 14, 18, 9); break;
                case 2: gfxInvert(12, 24, 25, 9); break;
                case 3: gfxInvert(49, 24, 14, 9); break;
                case 4: gfxInvert(12, 34, 14, 9); break;
                case 5: gfxInvert(44, 34, 14, 9); break;
            }
        }
    }

    void DrawIndicator() {
        gfxLine(0, 45, 63, 45);
        gfxLine(0, 62, 63, 62);
        for (int b = 0; b < 16; b++)
        {
            int v = (reg >> b) & 0x01;
            int v2 = (reg2 >> b) & 0x01;
            if (v) gfxRect(60 - (4 * b), 47, 3, 7);
            if (v2) gfxRect(60 - (4 * b), 54, 3, 7);
        }
    }

};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to DualTM,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
DualTM DualTM_instance[2];

void DualTM_Start(bool hemisphere) {
    DualTM_instance[hemisphere].BaseStart(hemisphere);
}

void DualTM_Controller(bool hemisphere, bool forwarding) {
    DualTM_instance[hemisphere].BaseController(forwarding);
}

void DualTM_View(bool hemisphere) {
    DualTM_instance[hemisphere].BaseView();
}

void DualTM_OnButtonPress(bool hemisphere) {
    DualTM_instance[hemisphere].OnButtonPress();
}

void DualTM_OnEncoderMove(bool hemisphere, int direction) {
    DualTM_instance[hemisphere].OnEncoderMove(direction);
}

void DualTM_ToggleHelpScreen(bool hemisphere) {
    DualTM_instance[hemisphere].HelpScreen();
}

uint64_t DualTM_OnDataRequest(bool hemisphere) {
    return DualTM_instance[hemisphere].OnDataRequest();
}

void DualTM_OnDataReceive(bool hemisphere, uint64_t data) {
    DualTM_instance[hemisphere].OnDataReceive(data);
}
