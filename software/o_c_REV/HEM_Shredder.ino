// Hemisphere Applet Boilerplate. Follow these steps to add a Hemisphere app:
//
// (1) Save this file as HEM_ClassName.ino
// (2) Find and replace "ClassName" with the name of your Applet class
// (3) Implement all of the public methods below
// (4) Add text to the help section below in SetHelp()
// (5) Add a declare line in hemisphere_config.h, which looks like this:
//     DECLARE_APPLET(id, categories, ClassName), \
// (6) Increment HEMISPHERE_AVAILABLE_APPLETS in hemisphere_config.h
// (7) Add your name and any additional copyright info to the block below

// Copyright (c) 2018, __________
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

#define HEM_SHREDDER_ANIMATION_SPEED 500
#define HEM_SHREDDER_DOUBLE_CLICK_DELAY 5000

class Shredder : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Shredder";
    }

    void Start() {
        quant_channels = 0;
        scale = 4;
        step = 0;
        replay = 0;
    }

    void Controller() {

        // Handle imprint confirmation animation
        if (--confirm_animation_countdown < 0) {
            confirm_animation_position--;
            confirm_animation_countdown = HEM_SHREDDER_ANIMATION_SPEED;
        }

        // Handle double click delay
        if (double_click_delay > 0) {
            // decrement delay and if it's 0, move the cursor
            if (--double_click_delay < 1) {
                // if we hit zero before being reset (aka no double click), move the cursor
                if (++cursor > 3) cursor = 0; // we should never be > 3, so this is just for safety
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawParams();
        DrawGrid();
    }

    void OnButtonPress() {
        if (cursor < 2) {
            // first two cursor params support double-click to shred voltages
            if (double_click_delay == 0) {
                // first click
                double_click_delay = HEM_SHREDDER_DOUBLE_CLICK_DELAY;    
            } else {
                // second click
                double_click_delay = 0; // kill the delay
                ImprintVoltage(cursor);
            }
        } else {
            if (++cursor > 3) cursor = 0;
        }
    }

    void OnEncoderMove(int direction) {
        

    }
        
    uint32_t OnDataRequest() {
        uint32_t data = 0;
        // example: pack property_name at bit 0, with size of 8 bits
        // Pack(data, PackLocation {0,8}, property_name); 
        return data;
    }

    void OnDataReceive(uint32_t data) {
        // example: unpack value at bit 0 with size of 8 bits to property_name
        // property_name = Unpack(data, PackLocation {0,8}); 
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock 2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=X     2=Y";
        help[HEMISPHERE_HELP_OUTS]     = "A=Ch 1  B=Ch 2";
        help[HEMISPHERE_HELP_ENCODER]  = "Range/Quant";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;

    // Sequencer state
    uint8_t step; // Current step number
    int16_t sequence1[16];
    int16_t sequence2[16];
    bool replay; // When the encoder is moved, re-quantize the output

    // settings
    int range[2] = {0,0};
    int8_t quant_channels;
    int scale;

    // Variables to handle imprint confirmation animation
    int confirm_animation_countdown;
    int confirm_animation_position;
    // Variable for double-clicking to shred voltage
    int double_click_delay;

    void DrawParams() {
        // Channel 1 voltage
        gfxPrint(1, 15, "1:");
        gfxPrint(13, 15, "+5");
        if (cursor == 0) gfxCursor(13, 23, 18);

        // Channel 2 voltage
        gfxPrint(32, 15, "2:");
        gfxPrint(44, 15, "+5");
        if (cursor == 1) gfxCursor(44, 23, 18);

        // quantize channel selection
        gfxIcon(32, 25, SCALE_ICON);
        gfxPrint(42, 25, "1+2");
        if (cursor == 2) gfxCursor(42, 33, 20);

        // quantize scale selection
        gfxPrint(32, 35, OC::scale_names_short[scale]);
        if (cursor == 3) gfxCursor(32, 43, 30);

    }
    
    void DrawGrid() {
        // Draw the Cartesian plane
        for (int s = 0; s < 16; s++) gfxFrame(1 + (8 * (s % 4)), 26 + (8 * (s / 4)), 5, 5);

        // Crosshairs for play position
        int cxy = step / 4;
        int cxx = step % 4;
        gfxDottedLine(3 + (8 * cxx), 26, 3 + (8 * cxx), 58, 2);
        gfxDottedLine(1, 28 + (8 * cxy), 32, 28 + (8 * cxy), 2);
        gfxRect(1 + (8 * cxx), 26 + (8 * cxy), 5, 5);

        // Draw imprint animation, if necessary
        if (confirm_animation_position > -1) {
            int progress = 16 - confirm_animation_position;
            for (int s = 0; s < progress; s++)
            {
                gfxRect(1 + (8 * (s / 4)), 26 + (8 * (s % 4)), 7, 7);
            }
        }
    }

    void ImprintVoltage(int channel) {

        // start imprint animation
        confirm_animation_position = 16;
        confirm_animation_countdown = HEM_SHREDDER_ANIMATION_SPEED;
    }
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to ClassName,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
Shredder Shredder_instance[2];

void Shredder_Start(bool hemisphere) {Shredder_instance[hemisphere].BaseStart(hemisphere);}
void Shredder_Controller(bool hemisphere, bool forwarding) {Shredder_instance[hemisphere].BaseController(forwarding);}
void Shredder_View(bool hemisphere) {Shredder_instance[hemisphere].BaseView();}
void Shredder_OnButtonPress(bool hemisphere) {Shredder_instance[hemisphere].OnButtonPress();}
void Shredder_OnEncoderMove(bool hemisphere, int direction) {Shredder_instance[hemisphere].OnEncoderMove(direction);}
void Shredder_ToggleHelpScreen(bool hemisphere) {Shredder_instance[hemisphere].HelpScreen();}
uint32_t Shredder_OnDataRequest(bool hemisphere) {return Shredder_instance[hemisphere].OnDataRequest();}
void Shredder_OnDataReceive(bool hemisphere, uint32_t data) {Shredder_instance[hemisphere].OnDataReceive(data);}
