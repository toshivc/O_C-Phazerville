#define SEQ_MAX_SEGMENTS 8
#define SEQ_MAX_STEPS 32

class SegmentedSequencer : public HemisphereApplet {
public:
    const char* applet_name() {
        return "SegSeq";
    }

    enum SegSeqCursor {
      NUM_SEGMENT, RANDOMIZE, PITCH_RANGE, SELECTED_SEG, TIME, KEY, SCALE, DENSITY, 
      DURATION,
    };

    void Start() {
        num_segments = 4;  // Default to 4 segments
        edit_segment = 0;
        current_segment = 0;
        step_counter = 0;
        rand_all_apply_anim = 0;
        for (int i = 0; i < SEQ_MAX_SEGMENTS; ++i) {
            segments[i] = {0, 0, 7, 2, 7, 7};  // Default values
        }
        cursor = 0;
        GenerateSequence();
    }

    void Controller() {
        if (Clock(0)) {  // External clock input
            step_counter++;
            if (step_counter >= GetCurrentSegmentLength()) {
                step_counter = 0;
                current_segment = (current_segment + 1) % num_segments;
            }

            // Update outputs
            Out(0, GetNextPitch());
            Out(1, GetNextGate() ? HEMISPHERE_MAX_CV : 0);
        }
    }




    void View() {
        DrawInterface();
    }


    void OnEncoderMove(int direction) {
        if (!EditMode()) { // move cursor
        MoveCursor(cursor, direction, 8);
        return;
        }
    

        Segment& seg = segments[edit_segment];
        switch(cursor) {
            case NUM_SEGMENT: 
            num_segments = constrain(num_segments + direction, 1, SEQ_MAX_SEGMENTS); 
            if (edit_segment>num_segments){
                edit_segment=num_segments-2;}  //change currently selected seg to be in range
                break;  
            
            case RANDOMIZE: 
            if(direction>0){RandomizeAllSegments();}else{RandomizeSegment(edit_segment);}; break;
           
            case PITCH_RANGE: seg.pitch_range = constrain(seg.pitch_range + direction, 0, 14); break;
            case SELECTED_SEG: edit_segment = constrain(edit_segment + direction, 0, num_segments-1); break;
            case TIME: seg.time_signature = constrain(seg.time_signature + direction, 0, 3); break;
            case KEY: seg.key = constrain(seg.key + direction, 0, 11); break;
            case SCALE: seg.scale = constrain(seg.scale + direction, 0, 15); break;
            case DENSITY: seg.density = constrain(seg.density + direction, 0, 14); break;
            case DURATION: seg.duration = constrain(seg.duration + direction, 0, 14); break;
            
        }
        GenerateSegment(edit_segment);
    }
 

    uint64_t OnDataRequest() {
        uint64_t data = 0;
        Pack(data, PackLocation {0, 4}, num_segments);
        return data;
    }

    void OnDataReceive(uint64_t data) {
        num_segments = Unpack(data, PackLocation {0, 4});
        GenerateSequence();
    }


protected:
    void SetHelp() {
        help[HEMISPHERE_HELP_DIGITALS] = "1=Clock";
        help[HEMISPHERE_HELP_CVS] = "Unused";
        help[HEMISPHERE_HELP_OUTS] = "A=Pitch B=Gate";
        help[HEMISPHERE_HELP_ENCODER] = "Segment/Params";
    }

private:
    struct Segment {
        uint8_t scale;
        uint8_t key;
        uint8_t density;
        uint8_t time_signature;
        uint8_t duration;
        uint8_t pitch_range;
    };

    Segment segments[SEQ_MAX_SEGMENTS];
    uint8_t num_segments;
    uint8_t current_segment;       //segment that is playing
    uint8_t edit_segment;       //segment that is being edited
    uint32_t step_counter;
    int cursor;

    //display
    uint8_t rand_all_apply_anim = 0; // Countdown to animate icons for regenerate of all segments

    uint8_t notes[SEQ_MAX_STEPS];
    uint32_t gates;

    void GenerateSequence() {
        for (int i = 0; i < num_segments; ++i) {
            GenerateSegment(i);
        }
    }

    void GenerateSegment(int index) {
        Segment& seg = segments[index];
        int steps = GetSegmentLength(seg.time_signature);
        int start_step = index * SEQ_MAX_STEPS / num_segments;

        // Generate notes
        for (int i = 0; i < steps; ++i) {
            notes[start_step + i] = random(0, seg.pitch_range + 1);
        }

        // Generate gates
        uint32_t segment_gates = 0;
        for (int i = 0; i < steps; ++i) {
            if (random(0, 15) < seg.density) {
                segment_gates |= (1 << i);
            }
        }
        gates &= ~(((1 << steps) - 1) << start_step);
        gates |= (segment_gates << start_step);
    }

    int32_t GetNextPitch() {
        Segment& seg = segments[current_segment];
        int step = step_counter + current_segment * SEQ_MAX_STEPS / num_segments;
        int note = notes[step];
        return QuantizerLookup(seg.scale, note + 60) + (seg.key * 128);
    }

    bool GetNextGate() {
        int step = step_counter + current_segment * SEQ_MAX_STEPS / num_segments;
        return (gates & (1 << step)) != 0;
    }

    void RandomizeAllSegments() {
        for (int i = 0; i < num_segments; ++i) {
            RandomizeSegment(i);
        }
        rand_all_apply_anim = 160; // Show that regeneration for all egments (anim for this many display updates)
    }

    void RandomizeSegment(int index) {
        Segment& seg = segments[index];
        seg.scale = random(0, 16);
        seg.key = random(0, 12);
        seg.density = random(0, 15);
        seg.time_signature = random(0, 4);
        seg.duration = random(0, 15);
        seg.pitch_range = random(0, 15);
        GenerateSegment(index);
    }

    void DrawInterface() {
        gfxHeader("SegSeq");

        gfxPrint(1, 15, "Segs:");
        gfxPrint(30, 15, num_segments);
        gfxIcon(43,14, RANDOM_ICON);
        
        gfxPrint(1, 25, "Rng:");
        gfxPrint(segments[edit_segment].pitch_range);
        
        //draw segments with randomization anim
        int rand_y_offset = 0;
        if (rand_all_apply_anim > 0) {
            --rand_all_apply_anim;
        }

        for (int i =0; i < num_segments; ++i){
            if (20*(8-i) > rand_all_apply_anim && rand_all_apply_anim > 20*(7-i)) {     //add a y offset to icons for 20
                rand_y_offset = 2;
            } 
            else {rand_y_offset = 0;}

            if (i==edit_segment && i == current_segment){
                gfxRect(i*8+1, 34-rand_y_offset, 6, 8); //draw selected segment that is playing with a large solid box
            }else if(i == edit_segment){
                gfxRect(i*8+2, 35-rand_y_offset, 5, 6); //draw selected segment with a small solid box
            }else if(i==current_segment){
                gfxFrame(i*8+1, 34-rand_y_offset, 6, 8); //draw playing segment with a large hollow box
            }else{
                gfxFrame(i*8+2, 35-rand_y_offset, 5, 6); //draw all other segments with a small hollow box
            }
        }

        const char* time_sigs[] = {"2/4", "3/4", "4/4", "5/4"};
        gfxPrint(1, 45, time_sigs[segments[edit_segment].time_signature]);
        gfxPrint(24,45,"");
        gfxPrint(OC::Strings::note_names_unpadded[segments[edit_segment].key]);
        gfxPrint(38, 45, "");
        gfxPrint(OC::scale_names_short[segments[edit_segment].scale]);
       
        
        gfxPrint(1, 55, "Den:");
        gfxPrint(segments[edit_segment].density);
        gfxPrint(32, 55, "Dur:");
        gfxPrint(segments[edit_segment].duration);


       switch (cursor){
        case NUM_SEGMENT: gfxCursor(30, 23, 8); break;
        case RANDOMIZE: gfxCursor(41,23,11); break;
        case PITCH_RANGE: gfxCursor(24,33,10); break;
        case SELECTED_SEG: gfxCursor(1, 43, 62, 1); break;
        case TIME: gfxCursor(1, 53, 20); break;
        case KEY: gfxCursor(24, 53, 14); break;
        case SCALE: gfxCursor(38, 53, 24); break;
        case DENSITY: gfxCursor(24, 63, 10); break;
        case DURATION: gfxCursor(52,63,12); break;
       }
    }

    int GetSegmentLength(uint8_t time_signature) {
        const int lengths[] = {8, 12, 16, 20};  // Steps per segment for each time signature
        return lengths[time_signature];
    }

    int GetCurrentSegmentLength() {
        return GetSegmentLength(segments[current_segment].time_signature);
    }
};