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

class Shredder : public HemisphereApplet {
public:

    const char* applet_name() {
        return "Shredder";
    }

    void Start() {
        step = 0;
        replay = 0;
        transpose = 0;
    }

    void Controller() {
    }

    void View() {
        gfxHeader(applet_name());
        DrawGrid();
    }

    void OnButtonPress() {
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

    // Variables to handle imprint confirmation animation
    int confirm_animation_countdown;
    int confirm_animation_position;
    
    void DrawGrid() {
        // Draw the Cartesian plane
        for (int s = 0; s < 16; s++) gfxFrame(1 + (8 * (s % 4)), 26 + (8 * (s / 4)), 5, 5);

        // Crosshairs for play position
        int cxy = step / 4;
        int cxx = step % 4;
        gfxDottedLine(3 + (8 * cxx), 26, 3 + (8 * cxx), 58, 2);
        gfxDottedLine(1, 28 + (8 * cxy), 32, 28 + (8 * cxy), 2);
        gfxRect(1 + (8 * cxx), 26 + (8 * cxy), 5, 5);

        // Draw confirmation animation, if necessary
        if (confirm_animation_position > -1) {
            int progress = 16 - confirm_animation_position;
            for (int s = 0; s < progress; s++)
            {
                gfxRect(1 + (8 * (s / 4)), 26 + (8 * (s % 4)), 7, 7);
            }
        }
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
