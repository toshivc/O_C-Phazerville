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



class DrumMap : public HemisphereApplet {
public:

    const char* applet_name() {
        return "DrumMap";
    }

    void Start() {
    }

    void Controller() {
        cv1 = Proportion(DetentedIn(0), HEMISPHERE_MAX_CV, 255);
        cv2 = Proportion(DetentedIn(1), HEMISPHERE_MAX_CV, 255);
    }

    void View() {
        gfxHeader(applet_name());
        DrawInterface();
    }

    void OnButtonPress() {
        if (++cursor > 8) cursor = 0;
        if (mode_b > 2 && cursor == 3) cursor = 4;
    }

    void OnEncoderMove(int direction) {
        // modes
        if (cursor == 0) {
            mode_a += direction;
            if (mode_a > 2) mode_a = 0;
            if (mode_a < 0) mode_a = 2;
        }
        if (cursor == 1) {
            mode_b += direction;
            if (mode_b > 5) mode_b = 0;
            if (mode_b < 0) mode_b = 5;
        }
        // fill
        if (cursor == 2) fill_a = constrain(fill_a += direction, 0, 255);
        if (cursor == 3) fill_b = constrain(fill_b += direction, 0, 255);
        // x/y
        if (cursor == 4) x = constrain(x += direction, 0, 255);
        if (cursor == 5) y = constrain(y += direction, 0, 255);
        // chaos
        if (cursor == 6) chaos = constrain(chaos += direction, 0, 255);
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
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock   2=Reset";
        help[HEMISPHERE_HELP_CVS]      = "1=X/FilA  2=Y/FilB";
        help[HEMISPHERE_HELP_OUTS]     = "A=Part A  B=Part B";
        help[HEMISPHERE_HELP_ENCODER]  = "Operation";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int cursor;
    const uint8_t *MODE_ICONS[3] = {BD_ICON,SN_ICON,HH_ICON};
    int mode_a = 0;
    int mode_b = 1;
    int fill_a = 128;
    int fill_b = 128;
    int x = 0;
    int y = 0;
    int chaos = 0;
    int cv_mode_a = 0;
    int cv_mode_b = 0;
    int cv1 = 0;
    int cv2 = 0;
    
    void DrawInterface() {
        // output selection
        gfxPrint(1,15,"A:");
        gfxIcon(14,14,MODE_ICONS[mode_a]);
        gfxPrint(32,15,"B:");
        gfxIcon(45,14,MODE_ICONS[mode_b%3]);
        if (mode_b > 2) {
            gfxPrint(53,15,">");
        }

        // fill
        gfxPrint(1,25,"F");
        // add cv1 to fill_a value if cv1 mode is set to Fill A
        int fa = fill_a;
        if (cv_mode_a == 0) fa += cv1;
        DrawKnobAt(9,25,20,constrain(fa, 0, 255),cursor == 2);
        // don't show fill for channel b if it is an accent mode
        if (mode_b < 3) {
            gfxPrint(32,25,"F");
            // add cv1 to fill_a value if cv1 mode is set to Fill A
            int fb = fill_b;
            if (cv_mode_b == 0) fb += cv2;
            DrawKnobAt(40,25,20,constrain(fb, 0, 255),cursor == 3);
        }
        
        // x & y
        gfxPrint(1,35,"X");
        DrawKnobAt(9,35,20,x,cursor == 4);
        gfxPrint(32,35,"Y");
        DrawKnobAt(40,35,20,y,cursor == 5);
        
        // chaos
        gfxPrint(1,45,"CHAOS");
        DrawKnobAt(32,45,28,chaos,cursor == 6);
        
        // cv input assignment
//        gfxIcon(1,47,CV_ICON);
        gfxPrint(1,55,"1:FA");
//        gfxIcon(32,47,CV_ICON);
        gfxPrint(32,55,"2:FB");

        // cursor
        if (cursor == 0) gfxCursor(14,23,16); // Part A
        if (cursor == 1) gfxCursor(45,23,16); // Part B
        if (cursor == 7) gfxCursor(14,63,16); // CV1 Assign
        if (cursor == 8) gfxCursor(45,63,16); // CV2 Assign

    }

    void DrawKnobAt(byte x, byte y, byte len, byte value, bool is_cursor) {
        byte w = Proportion(value, 255, len);
        byte p = is_cursor ? 1 : 3;
        gfxDottedLine(x, y + 4, x + len, y + 4, p);
        gfxRect(x + w, y, 2, 7);
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
DrumMap DrumMap_instance[2];

void DrumMap_Start(bool hemisphere) {DrumMap_instance[hemisphere].BaseStart(hemisphere);}
void DrumMap_Controller(bool hemisphere, bool forwarding) {DrumMap_instance[hemisphere].BaseController(forwarding);}
void DrumMap_View(bool hemisphere) {DrumMap_instance[hemisphere].BaseView();}
void DrumMap_OnButtonPress(bool hemisphere) {DrumMap_instance[hemisphere].OnButtonPress();}
void DrumMap_OnEncoderMove(bool hemisphere, int direction) {DrumMap_instance[hemisphere].OnEncoderMove(direction);}
void DrumMap_ToggleHelpScreen(bool hemisphere) {DrumMap_instance[hemisphere].HelpScreen();}
uint32_t DrumMap_OnDataRequest(bool hemisphere) {return DrumMap_instance[hemisphere].OnDataRequest();}
void DrumMap_OnDataReceive(bool hemisphere, uint32_t data) {DrumMap_instance[hemisphere].OnDataReceive(data);}
