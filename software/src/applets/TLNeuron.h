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

// How fast the axon pulses when active
#define HEM_TLN_ACTIVE_TICKS 1500
#define NUM_DENDRITES 5
#define NUM_AXONS 2

class TLNeuron : public HemisphereApplet {
public:
    // Enum for cursor management (optional, can be expanded)
    enum TLNeuronCursor {
        OUTPUTAXON,
        DENDRITE1,
        DENDRITE2,
        DENDRITE3,
        DENDRITE4,
        DENDRITE5,
        AXON,
        LAST_SETTING = AXON
    };


    const char* applet_name() { // Maximum 10 characters
        return "TL Neuron";
    }

    void Start() {
        cursor = 0;
        //selected = 0;
    }

    void Controller() {
        
        
        //Update outputs for each axon
        for(int a = 0; a < NUM_AXONS; a++){
            // Summing function: add up the 5 weights
            int sum = 0;

            ForEachChannel(ch){
                //gate dendrites (1 and 2)
                if (Gate(ch)) {
                    sum += dendrite_weight[a][ch];
                    dendrite_activated[a][ch] = 1;
                } else {
                    dendrite_activated[a][ch] = 0;
                }

                //CV dendrites (3 and 4)
                if (In(ch) > (HEMISPHERE_MAX_INPUT_CV / 2)) {
                    sum += dendrite_weight[a][2+ch];
                    dendrite_activated[a][2+ch] = 1;
                } else {
                    dendrite_activated[a][2+ch] = 0;
                }
            }
            //Feedback dendrite (5)
            if (axon_activated[a^1]) {
                sum += dendrite_weight[a][4];
                dendrite_activated[a][4] = 1;
            } else {
                dendrite_activated[a][4] = 0;
            }

            // Threshold function: fire the axon if the sum is GREATER THAN the threshhold
            if (!axon_activated[a]) axon_radius[a] = 5;
            axon_activated[a] = (sum > threshold[a]);
            GateOut(a, axon_activated[a]);
            
            // Increase the axon radius via timer
            if (--axon_countdown[a] < 0) {
                axon_countdown[a] = HEM_TLN_ACTIVE_TICKS;
                ++axon_radius[a];
                if (axon_radius[a] > 14) axon_radius[a] = 5;
            }
        }
    }

    void View() {
        DrawDendrites();
        DrawAxon();
        DrawStates();
    }


    void OnEncoderMove(int direction) {
        if (!EditMode()) { // move cursor
        MoveCursor(cursor, direction, LAST_SETTING);
        return;
        }
        
        switch(cursor) {
        case OUTPUTAXON: selected_axon = constrain(selected_axon + direction, 0, 1); break;
        case DENDRITE1:
        case DENDRITE2:
        case DENDRITE3:
        case DENDRITE4:
        case DENDRITE5:{
            dendrite_weight[selected_axon][cursor-1] = constrain(dendrite_weight[selected_axon][cursor-1] + direction, -9, 9);
        }; break;
        case AXON: threshold[selected_axon] = constrain(threshold[selected_axon] + direction, -27, 27); break;
        }
    }

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        Pack(data, PackLocation {0,5}, dendrite_weight[0][0] + 9);
        Pack(data, PackLocation {5,5}, dendrite_weight[0][1] + 9);
        Pack(data, PackLocation {10,5}, dendrite_weight[0][2] + 9);
        Pack(data, PackLocation {15,6}, threshold[0] + 27);
        return data;
    }

    void OnDataReceive(uint64_t data) {
        dendrite_weight[0][0] = Unpack(data, PackLocation {0,5}) - 9;
        dendrite_weight[0][1] = Unpack(data, PackLocation {5,5}) - 9;
        dendrite_weight[0][2] = Unpack(data, PackLocation {10,5}) - 9;
        threshold[0] = Unpack(data, PackLocation {15,6}) - 27;
    }

protected:
    void SetHelp() {
        //                               "------------------" <-- Size Guide
        help[HEMISPHERE_HELP_DIGITALS] = "1,2=Dendrites 1,2";
        help[HEMISPHERE_HELP_CVS]      = "1,2=Dendrites 3,4";
        help[HEMISPHERE_HELP_OUTS]     = "A,B=Axon Outputs";
        help[HEMISPHERE_HELP_ENCODER]  = "Weights & Threshold";
        //                               "------------------" <-- Size Guide
    }
    
private:
    
    int cursor; // Which thing is selected (Dendrite 1, 2, 3, 4, FB weights; Axon threshold)
    //int selected; 
    int selected_axon = 0;
    int dendrite_weight[NUM_AXONS][NUM_DENDRITES] = {{5, 5, 0, 0, 0}, {0, 0, 0, 0, 0}};
    int threshold[NUM_AXONS] = {9,9};
    bool dendrite_activated[NUM_AXONS][NUM_DENDRITES];
    bool axon_activated[NUM_AXONS];
    int axon_radius[NUM_AXONS] = {5,5};
    int axon_countdown[NUM_AXONS];
    

    void DrawDendrites() {
        int dendrite_location[NUM_DENDRITES][2]= {
            {9, 31},   
            {32-9, 20},
            {32+9, 20},
            {64-9, 31},
            {13, 50},
        };
        
        dendrite_location[4][0] = selected_axon ? 13 : 64-13; 

        for (int d = 0; d < NUM_DENDRITES; d++)
        {        
            int weight = dendrite_weight[selected_axon][d];
            gfxCircle(dendrite_location[d][0],dendrite_location[d][1], 8); // Dendrite
            gfxPrint(dendrite_location[d][0] + (weight < 0 ? -6 : -3) , dendrite_location[d][1]-3, weight); //add (weight < 0 ? 1 : 6) to offset weight into center of circle
            if (EditMode()){
                if (cursor == d + 1 ) gfxCircle(dendrite_location[d][0],dendrite_location[d][1], 7);
            } else {
                if (cursor == d + 1 && CursorBlink()) gfxCircle(dendrite_location[d][0],dendrite_location[d][1], 7);
            }

            //Draw the synapses
            gfxDottedLine(dendrite_location[d][0], dendrite_location[d][1], (selected_axon ? 39 : 25), 41, dendrite_activated[selected_axon][d] ? 1 : 3);  
        }
    }

    void DrawAxon() {
        
        int axon_x = selected_axon ? 64-19 : 19;
        const int axon_y = 51;
        
        //Draw axon
        gfxCircle(axon_x, axon_y, 12);
        //Print threshold        
        int x = CenterDigits(axon_x, threshold[selected_axon]);
        gfxPrint(x-3, axon_y-3, threshold[selected_axon]);


        //Draw Axon highlight if it is selected
        if (EditMode()){
            if (cursor == AXON ) gfxCircle(axon_x, axon_y, 11);
        } else {
            if (cursor == AXON && CursorBlink()) gfxCircle(axon_x, axon_y, 11);
        }

        //Draw Axon activation
        if (axon_activated[selected_axon]) {
            gfxCircle(axon_x, axon_y, 12);
            gfxCircle(axon_x, axon_y, axon_radius[selected_axon]);
        } 
    }

    void DrawStates() {
        //Draw which axon is selected
        gfxPrint(5, 13, selected_axon?"B":"A");
        if (cursor == OUTPUTAXON){
            gfxCursor(5, 21, 7);
        }            
    }

    int CenterDigits(int starting_x, int value){
        int x = starting_x; // Starting x position for number
        //if (value > 9 || value < -9) x -= 2; // Shove left a bit if a two-digit number
        if (value < 0) x -= 3; // Pull back if a sign is necessary
        return x;
    }
};
