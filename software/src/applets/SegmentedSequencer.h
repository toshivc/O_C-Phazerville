#define SEQ_MAX_SEGMENTS 8
#define SEQ_MAX_STEPS 32

class SegmentedSequencer : public HemisphereApplet {
public:
    const char* applet_name() {
        return "SegSeq";
    }

    enum SegSeqCursor {
      NUM_SEGMENT, RANDOMIZE, KEY, SCALE, PITCH_RANGE, DENSITY, TIME,
      DURATION,
    };

    void Start() {
        num_segments = 4;  // Default to 4 segments
        current_segment = 0;
        step_counter = 0;
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

    /*void OnButtonPress() {
        if (cursor == NUM_SEGMENT) {
            RandomizeAllSegments();
        } else {
            //RandomizeSegment(current_segment);
        }
    }*/

    void OnEncoderMove(int direction) {
        if (!EditMode()) { // move cursor
        MoveCursor(cursor, direction, 7);
        return;
        }


        Segment& seg = segments[current_segment];
        switch(cursor) {
            case NUM_SEGMENT: num_segments = constrain(num_segments + direction, 1, SEQ_MAX_SEGMENTS); break;
            case RANDOMIZE: 
            if(direction>1){RandomizeAllSegments();}else{RandomizeSegment(current_segment);}
            case KEY: seg.key = constrain(seg.key + direction, 0, 11); break;
            case SCALE: seg.scale = constrain(seg.scale + direction, 0, 15); break;
            case PITCH_RANGE: seg.pitch_range = constrain(seg.pitch_range + direction, 0, 14); break;
            case DENSITY: seg.density = constrain(seg.density + direction, 0, 14); break;
            case TIME: seg.time_signature = constrain(seg.time_signature + direction, 0, 3); break;
            case DURATION: seg.duration = constrain(seg.duration + direction, 0, 14); break;
            
        }
        GenerateSegment(current_segment);
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
        gfxIcon(43,13, RANDOM_ICON);
        
        gfxPrint(1, 25, "S ");
        gfxPrint(current_segment + 1);
        gfxPrint(":");
        gfxPrint(OC::Strings::note_names_unpadded[segments[current_segment].key]);
        gfxPrint(38, 25, "");
        gfxPrint(OC::scale_names_short[segments[current_segment].scale]);
        
        gfxPrint(1, 35, "Rng:");
        gfxPrint(segments[current_segment].pitch_range);
        
        gfxPrint(1, 45, "Den:");
        gfxPrint(segments[current_segment].density);
        
        const char* time_sigs[] = {"2/4", "3/4", "4/4", "5/4"};
        gfxPrint(1, 55, time_sigs[segments[current_segment].time_signature]);
        gfxPrint(24, 55, "Dur:");
        gfxPrint(segments[current_segment].duration);


       switch (cursor){
        case NUM_SEGMENT: gfxCursor(28, 23, 12); break;
        case RANDOMIZE: gfxCursor(41,23,11); break;
        case KEY: gfxCursor(24, 33, 12); break;
        case SCALE: gfxCursor(38, 33, 24); break;
        case PITCH_RANGE: gfxCursor(24,43,12); break;
        case DENSITY: gfxCursor(24, 53, 12); break;
        case TIME: gfxCursor(1, 63, 20); break;
        case DURATION: gfxCursor(46,63,12); break;
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