#include "../HemisphereApplet.h"

class Histo : public HemisphereApplet {
public:
    enum HistoCursor {
        DECAY, OCTAVE,
        LAST_SETTING = OCTAVE
    };

    const char* applet_name() {
        return "Histo";
    }

    void Start() {
        ResetHistogram();
        output_octave = 4; // Default octave
    }

    void Controller() {
        int32_t pitch = In(0);
        input_note = MIDIQuantizer::NoteNumber(pitch);
        

        // Recalculate range and select note less frequently
        if (++update_counter >= UPDATE_INTERVAL) {
            UpdateHistogram(input_note % 12);
            update_counter = 0;
            // Calculate possible note range
            CalculatePossibleNoteRange();
            // Get CV input and select output note
            int32_t rotate_input = In(1);
            selected_note = SelectNote(rotate_input);
        }
        
        // Output CV
        Out(0, MIDIQuantizer::CV(selected_note + (output_octave * 12)));
    }

    void View() {
        DrawCircle();
        DrawInterface();
        PrintDiag();
    }

    void OnEncoderMove(int direction) {
        if (!EditMode()) {
            MoveCursor(cursor, direction, LAST_SETTING);
            return;
        }

        switch (cursor) {
        case DECAY: decay_interval = constrain(decay_interval + direction, 1, 99); break;
        case OCTAVE: output_octave = constrain(output_octave + direction, 0, 9); break;
        }
    }

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        Pack(data, PackLocation {0,4}, output_octave);
        return data;
    }

    void OnDataReceive(uint64_t data) {
        output_octave = Unpack(data, PackLocation {0,4});
    }

protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=In 2=CV Select";
        help[HEMISPHERE_HELP_CVS]      = "1=In 2=CV Select";
        help[HEMISPHERE_HELP_OUTS]     = "A=Selected Note CV";
        help[HEMISPHERE_HELP_ENCODER]  = "Set output octave";
    }
    
private:
    int cursor;
    static const int NOTES_IN_SCALE = 12;
    const int circle_of_fifths[NOTES_IN_SCALE] = { 0, 7, 2, 9, 4, 11, 6, 1, 8, 3, 10, 5 };  //use MIDI note values in octave -2.
    int hist_bins[NOTES_IN_SCALE] = { 0 };
    int selected_note;
    int input_note;
    int output_octave;
    int lowest_note;
    int highest_note;

    static const int UPDATE_INTERVAL = 167; // Number of controller() cycles before cv input is re-read
    static const int HISTOGRAM_THRESHOLD = 10; // Threshold to detect new output note changes
    int update_counter = 0;
    int cycle_count = 0;
    int decay_interval = 20; // units in very rough centiseconds. Default is 200ms

    void DrawCircle() {
        int center_x = 30;
        int center_y = 42;
        int circle_radius = 20;

        //Draw rim circle
        //gfxCircle(center_x, center_y, circle_radius);

        for (int i = 0; i < NOTES_IN_SCALE; i++) {
            //draw bindividers
            double start_angle = (90 - 30 * i) * M_PI / 180.0;
            double end_angle = (90 - 30 * (i + 1)) * M_PI / 180.0;
            int x_start = center_x + circle_radius * cos(start_angle);
            int y_start = center_y - circle_radius * sin(start_angle);
            gfxLine(center_x, center_y, x_start, y_start);

            //draw rim
            int x_end = center_x + circle_radius * cos(end_angle);
            int y_end = center_y - circle_radius * sin(end_angle);
            if (i <= lowest_note && i >= highest_note){ 
                gfxLine(x_start, y_start, x_end, y_end);
            }
            
            int spacing = 18;
            for (int s = 0; s < spacing; s++) {
                //draw bins
                double fill_angle = start_angle - (start_angle - end_angle) * s / (spacing - 1);
                int x = center_x + hist_bins[i] * cos(fill_angle);
                int y = center_y - hist_bins[i] * sin(fill_angle);
                gfxLine(center_x, center_y, x, y);
            }
        }
    }

    void DrawInterface() {
        gfxPrint(1, 12, "Decay");
        gfxPrint(1, 20, decay_interval);

        gfxPrint(42, 12, "Oct");
        gfxPrint(50, 20, output_octave-2);
        
        switch(cursor){
            case DECAY: gfxCursor(1, 28, 12); break;
            case OCTAVE: gfxCursor(52, 28, 10); break;
        }

        //gfxPrint(1, 56, input_note);
        gfxPrint(2, 56, OC::Strings::note_names_unpadded[input_note % 12]);//input note
        gfxPrint(50, 56, OC::Strings::note_names_unpadded[selected_note]);  //output note
    }
    
    void PrintDiag(){
        Serial.print("Histogram: ");
        for (int i = 0; i < NOTES_IN_SCALE; i++) {
            Serial.print(hist_bins[i]);
            Serial.print(" ");
        }
        Serial.println();
        Serial.print("Lowest Note in Range: ");
        Serial.println(lowest_note);
        Serial.print("Highest Note in Range: ");
        Serial.println(highest_note);
    }

    void UpdateHistogram(int note) {
        hist_bins[circle_of_fifths[note]]++;
        if (hist_bins[circle_of_fifths[note]] > 20) { // Max bin value
            hist_bins[circle_of_fifths[note]] = 20;
        }
        cycle_count++;
        if (cycle_count >= decay_interval) { //called 16,667 times per sec. Times by 167 to aproximate centisecs
            DecayHistogram();
            cycle_count = 0;
        }
    }

    void DecayHistogram() {
        for (int i = 0; i < 12; i++) {
            if (hist_bins[i] > 0) {
                hist_bins[i]--;
            }
        }
    }

    void ResetHistogram() {
        for (int i = 0; i < NOTES_IN_SCALE; i++) {
            hist_bins[i] = 0;
        }
    }

    void CalculatePossibleNoteRange() {
    int lowest_active_note = -1;
    int highest_active_note = -1;

    // Scan the histogram to find the lowest and highest active notes
    for (int i = 0; i < NOTES_IN_SCALE; i++) {
        if (hist_bins[i] >= HISTOGRAM_THRESHOLD) {
            if (lowest_active_note == -1) {
                lowest_active_note = i;
            }
            highest_active_note = i;
        }
    }

    // Single note case - all notes are possible
    if (lowest_active_note == highest_active_note) {
        
        lowest_note = circle_of_fifths[0];  // C
        highest_note = circle_of_fifths[11]; // B
        return;
    }

    // If there are no active notes, keep the previous range
    if (lowest_active_note == -1 || highest_active_note == -1) {
        return;
    }

    // Extend the range by 6 positions in both directions (clockwise and anticlockwise)
    lowest_note = circle_of_fifths[(lowest_active_note - 6 + NOTES_IN_SCALE) % NOTES_IN_SCALE];
    highest_note = circle_of_fifths[(highest_active_note + 6) % NOTES_IN_SCALE];
}

    int SelectNote(int32_t cv_input) {
        int range = 12 - circle_of_fifths[lowest_note] - circle_of_fifths[highest_note];
        //int index = (cv_input * range) / (1 << 12); // Assuming cv_input is 12-bit
        int index = Proportion(cv_input, HEMISPHERE_MAX_INPUT_CV, range);
        return circle_of_fifths[(lowest_note + index) % NOTES_IN_SCALE];
    }

};