//#define MAX_STEPS 32  // Define the maximum number of steps
#include "../HemisphereApplet.h"

class Phraser : public HemisphereApplet {
public:
    // Enum for cursor management (optional, can be expanded)
    enum PhraserCursor {
        NUM_STEPS,
        DENSITY,
        DURATION_PROB,
        REPEAT_PROB,
        SEED,
        LAST_SETTING = SEED
    };

    // Applet name for display
    const char* applet_name() {
        return "Phraser";
    }

    // Start function to initialize the applet
    void Start() {
        num_steps = 16;  // Default number of steps
        density = 50;    // Default density percentage
        duration_prob = 25;  // Default probability for note duration
        repeat_prob = 50;    // Default probability to repeat a phrase
        seed = micros();            // Default seed value
        phrase_length = 0;   // Initialize phrase length
        current_step = 0;    // Start at step 0
        randomSeed(seed);   // Initialize random number generator with seed

        cursor = 0;
        GenerateSeq();
    }

    // Controller function to handle real-time processing
    void Controller() {
        // Main logic loop to control the sequence generation and probability calculations

        // Example: Checking for clock signal and updating sequence
        if (Clock(0)) {
            current_step++;
            if (current_step >= num_steps) {
                current_step = 0;
            }
        }

        // Update outputs
        Out(1, gates & (1 << current_step) ? HEMISPHERE_MAX_CV : 0);
    }
            


    // View function to handle the display (optional)
    void View() {
        DrawInterface();
        DrawBeatMap();
    }

    void OnEncoderMove(int direction) {
        if (!EditMode()) { // move cursor
        MoveCursor(cursor, direction, LAST_SETTING);
        return;
        }

        switch(cursor) {
            case NUM_STEPS: num_steps = constrain(num_steps + direction, 1, 32); break;
            case DENSITY: density = constrain(density + direction, 1, 100); break; 
            case DURATION_PROB: duration_prob = constrain(duration_prob + direction, 1, 100); break; 
            case REPEAT_PROB: repeat_prob = constrain(repeat_prob + direction, 1, 100); break; 
            case SEED: seed = constrain(seed + direction, 0, 100); break;
        }
        GenerateSeq();
    }

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        Pack(data, PackLocation {0, 8}, density);
        Pack(data, PackLocation {8, 8}, duration_prob);
        Pack(data, PackLocation {16, 8}, repeat_prob);
        Pack(data, PackLocation {24, 8}, seed);
        return data;
    }

    void OnDataReceive(uint64_t data) {
        density = Unpack(data, PackLocation {0, 8});
        duration_prob = Unpack(data, PackLocation {8, 8});
        repeat_prob = Unpack(data, PackLocation {16, 8});
        seed = Unpack(data, PackLocation {24, 8});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS] = "1=Density";
        help[HEMISPHERE_HELP_OUTS] = "A=Gate";
        help[HEMISPHERE_HELP_ENCODER] = "Edit Params";
    }

private:
    int cursor;  // PhraserCursor
    int num_steps;
    int density;  // Probability of gate being ON
    int duration_prob;  // Probability of gate duration
    int repeat_prob;  // Probability of repeating phrase
    uint16_t seed;  // Random seed for deterministic generation
    

    int phrase_length;
    int current_step;
    uint32_t gates;

    
    void GenerateSeq() {
        //randomSeed(seed);
        
        int gate_duration = 0;
        int gate_prob = 0;
        int new_phase_prob = 100;   //first phrase of a new sequence will always be new
        int prev_phrase_start = 0;

        for (int i = 0; i < num_steps; ++i) {
            randomSeed(micros());
            //Generate a new phrase
            if (gate_duration == 0){
                //generate a new note with duration
                gate_prob = random(1,100); // Generate a random number between 0-99
                gate_duration = random(1,6); // generate a length for 1-6
            }
            //populate bit field
            if (gate_prob < density) {
                gates |= (1 << i);
            } else {
                gates &= (0 << i);
            }
            gate_duration--;  
            }  
        } 
    /*
    void GenerateSeq() {
        int gate_duration = 0;
        int gate_prob = 0;
        int new_phase_prob = 100;   //first phrase of a new sequence will always be new
        int prev_phrase_start = 0;

        for (int i = 0; i < num_steps; ++i) {
            

            //update new_phase_prob
            if (i+1-prev_phrase_start % 8 == 0) new_phase_prob += 10;
            if (i+1-prev_phrase_start % 6 == 0) new_phase_prob += 10;
            if (i+1-prev_phrase_start % 4 == 0) new_phase_prob += 10;
            if (num_steps-i+1 % 8 ==0)new_phase_prob += 10;
            if (num_steps-i+1 % 4 ==0)new_phase_prob += 10;
            new_phase_prob += random(1,20);

            if (new_phase_prob > repeat_prob){      //TODO change this to be a different repeat_prob
                //new phrase confirmed
                if (random(1,100)< repeat_prob){
                    //repeat is confirmed
                    
                    i = i-prev_phrase_start;
                } else{
                    //Generate a new phrase
                    if (gate_duration = 0){
                    //generate a new note with duration
                    gate_prob = random(1,100); // Generate a random number between 0-99
                    gate_duration = random(1,8); // generate a length for 1-8
                    }
                    //populate bit field
                    if (gate_prob < density) {
                        gates |= (1 << i);
                    } else {
                        gates |= (0 << i);
                    }
                    gate_duration--;  
                    }  
                prev_phrase_start = i;  

            }
            new_phase_prob = 0;
        }
    }*/

    void DrawInterface() {
        // Function to draw the interface on the OLED or other display
        gfxPrint(1, 15, "Steps:");
        gfxPrint(48, 15, num_steps);

        gfxPrint(1, 25, "Density:");
        gfxPrint(48, 25, density);

        gfxPrint(1, 35, "Length:");
        gfxPrint(48, 35, duration_prob);

        gfxPrint(1, 45, "Repeat:");
        gfxPrint(48, 45, repeat_prob);

        //gfxPrint(1, 55, "Seed:");
        //gfxPrint(48, 55, seed);

        switch (cursor){
        case NUM_STEPS: gfxCursor(47, 23, 8); break;
        case DENSITY: gfxCursor(47, 33, 8); break;
        case DURATION_PROB: gfxCursor(47, 43, 8); break;
        case REPEAT_PROB: gfxCursor(47, 53, 8); break;
        //case SEED: gfxCursor(47, 63, 8); break;
        }
    }

    void DrawBeatMap(){
        for (int i = 0; i < num_steps; i++)
        {
            if (gates & (1 << i)){
                gfxRect(i*2,56, 2, 8);
            }
            
        }
    }
};