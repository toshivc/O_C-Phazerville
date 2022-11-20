// Copyright (c) 2018, Jason Justian
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

#define HEM_LOFI_PCM_BUFFER_SIZE 3000
#define HEM_LOFI_PCM_SPEED 4
#define LOFI_PCM2CV(S) ((uint32_t)S << 8) - 32512 //32767 is 128 << 8 32512 is 127 << 8 // 0-126 is neg, 127 is 0, 128-254 is pos
// #define CLIPLIMIT 32512
#define CLIPLIMIT HEMISPHERE_3V_CV

class LoFiPCM : public HemisphereApplet {
public:

    const char* applet_name() { // Maximum 10 characters
        return "LoFi Echo";
    }

    void Start() {
        countdown = HEM_LOFI_PCM_SPEED;
        for (int i = 0; i < HEM_LOFI_PCM_BUFFER_SIZE; i++) pcm[i] = 0; //char is unsigned in teensy (0-255)?
        cursor = 1; //for gui
    }

    void Controller() {
        play = !Gate(0); // Continuously play unless gated
        int fdbk = Gate(1) ? 100 : feedback; // Feedback = 100 when gated

        if (play) {
            countdown--;
            if (countdown == 0) {
                if (++head >= length) {
                    head = 0;               
                    ClockOut(1);
                }

                int dt = delaytime_pct * length / 100; //convert delaytime to length in samples 
                int writehead = (head+length + dt) % length; //have to add the extra length to keep modulo positive in case delaytime is neg
                int16_t feedbackmix = constrain((pcm[head] * fdbk / 100 + In(0)), -CLIPLIMIT, CLIPLIMIT);
                pcm[writehead] = feedbackmix;
                
                Out(0, pcm[head]);
                countdown = rate_factor;
            }
        }
    }

    void View() {
        gfxHeader(applet_name());
        DrawSelector();
        if (play) DrawWaveform();
    }

    void OnButtonPress() {
        if (++cursor > 2) cursor = 0;
        ResetCursor();
    }

    void OnEncoderMove(int direction) {
        switch (cursor) {
        case 0:
            delaytime_pct = constrain(delaytime_pct += direction, 0, 99);
            break;
        case 1:
            feedback = constrain(feedback += direction, 0, 125);
            break;
        case 2:
            rate_factor = constrain(rate_factor += direction, 1, 20);
            break;
        }

        //amp_offset_cv = Proportion(amp_offset_pct, 100, HEMISPHERE_MAX_CV);
        //p[cursor] = constrain(p[cursor] += direction, 0, 100);

    
    }

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        return data;
    }

    void OnDataReceive(uint64_t data) {
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1=Mute 2=LoopMode";
        help[HEMISPHERE_HELP_CVS]      = "1=Audio 2=";
        help[HEMISPHERE_HELP_OUTS]     = "A=Audio B=EOC Trg";
        help[HEMISPHERE_HELP_ENCODER]  = "Time/Fdbk/Speed";
        //                               "------------------" <-- Size Guide
    }
    
private:
    int16_t pcm[HEM_LOFI_PCM_BUFFER_SIZE];
    bool play = 0; //play always on unless gated on Digital 1
    int head = 0; // Location of play/record head
    int delaytime_pct = 50; //delaytime as percentage of delayline buffer
    int feedback = 50;
    int countdown = HEM_LOFI_PCM_SPEED;
    int rate_factor = HEM_LOFI_PCM_SPEED;
    int length = HEM_LOFI_PCM_BUFFER_SIZE;
    int cursor; //for gui
     
    
    void DrawWaveform() {
        int inc = HEM_LOFI_PCM_BUFFER_SIZE / 256;
        int disp[32];
        int high = 128;
        int pos = head - (inc * 15) - random(1,3); // Try to center the head
        if (head < 0) head += length;
        for (int i = 0; i < 32; i++)
        {
            int v = pcm[pos];
            if (v < 0) v = 0;
            if (v > high) high = v;
            pos += inc;
            if (pos >= HEM_LOFI_PCM_BUFFER_SIZE) pos -= length;
            disp[i] = v;
        }
        
        for (int x = 0; x < 32; x++)
        {
            int height = Proportion(disp[x], high, 30);
            int margin = (32 - height) / 2;
            gfxLine(x * 2, 30 + margin, x * 2, height + 30 + margin);
        }
    }
    
    void DrawSelector()
    {
        if (cursor < 2) {
            for (int param = 0; param < 2; param++)
            {
                gfxIcon(31 * param, 15, param ? GAUGE_ICON : CLOCK_ICON );
                gfxPrint(4 + pad(100, delaytime_pct), 15, delaytime_pct);
                gfxPrint(36 + pad(1000, feedback), 15, feedback);
                if (param == cursor) gfxCursor(0 + (31 * param), 23, 30);
            }
        } else {
            gfxPrint(0, 15, "Rate:");
            gfxPrint(30, 15, rate_factor);
            gfxCursor(30, 23, 16);
        }
    }
    
};


////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to LoFiPCM,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
LoFiPCM LoFiPCM_instance[2];

void LoFiPCM_Start(bool hemisphere) {
    LoFiPCM_instance[hemisphere].BaseStart(hemisphere);
}

void LoFiPCM_Controller(bool hemisphere, bool forwarding) {
    LoFiPCM_instance[hemisphere].BaseController(forwarding);
}

void LoFiPCM_View(bool hemisphere) {
    LoFiPCM_instance[hemisphere].BaseView();
}

void LoFiPCM_OnButtonPress(bool hemisphere) {
    LoFiPCM_instance[hemisphere].OnButtonPress();
}

void LoFiPCM_OnEncoderMove(bool hemisphere, int direction) {
    LoFiPCM_instance[hemisphere].OnEncoderMove(direction);
}

void LoFiPCM_ToggleHelpScreen(bool hemisphere) {
    LoFiPCM_instance[hemisphere].HelpScreen();
}

uint64_t LoFiPCM_OnDataRequest(bool hemisphere) {
    return LoFiPCM_instance[hemisphere].OnDataRequest();
}

void LoFiPCM_OnDataReceive(bool hemisphere, uint64_t data) {
    LoFiPCM_instance[hemisphere].OnDataReceive(data);
}
