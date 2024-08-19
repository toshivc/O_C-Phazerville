#define SEQ_MAX_SEGMENTS 8
#define SEQ_MAX_STEPS 32

class SegmentedSequencer : public HemisphereApplet {
public:
    const char* applet_name() {
        return "SegSeq";
    }

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

    void OnButtonPress() {
        if (cursor == 0) {
            RandomizeAllSegments();
        } else {
            RandomizeSegment(current_segment);
        }
    }

    void OnEncoderMove(int direction) {
        if (cursor == 0) {
            num_segments = constrain(num_segments + direction, 1, SEQ_MAX_SEGMENTS);
        } else {
            Segment& seg = segments[current_segment];
            switch(cursor) {
                case 1: seg.scale = constrain(seg.scale + direction, 0, 15); break;
                case 2: seg.key = constrain(seg.key + direction, 0, 11); break;
                case 3: seg.density = constrain(seg.density + direction, 0, 14); break;
                case 4: seg.time_signature = constrain(seg.time_signature + direction, 0, 3); break;
                case 5: seg.duration = constrain(seg.duration + direction, 0, 14); break;
                case 6: seg.pitch_range = constrain(seg.pitch_range + direction, 0, 14); break;
            }
            GenerateSegment(current_segment);
        }
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
    uint8_t current_segment;
    uint32_t step_counter;
    uint8_t cursor;

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
        
        gfxPrint(1, 25, "Seg ");
        gfxPrint(current_segment + 1);
        gfxPrint(":");
        gfxPrint(OC::scale_names_short[segments[current_segment].scale]);
        
        gfxPrint(1, 35, "Key:");
        gfxPrint(OC::Strings::note_names_unpadded[segments[current_segment].key]);
        
        gfxPrint(1, 45, "Den:");
        gfxPrint(segments[current_segment].density);
        
        gfxPrint(1, 55, "Tim:");
        const char* time_sigs[] = {"2/4", "3/4", "4/4", "5/4"};
        gfxPrint(time_sigs[segments[current_segment].time_signature]);

        // Draw cursor
        if (cursor > 0) {
            gfxCursor(1, 25 + (cursor - 1) * 10, 62);
        } else {
            gfxCursor(28, 15, 12);
        }
    }

    int GetSegmentLength(uint8_t time_signature) {
        const int lengths[] = {4, 6, 8, 10};  // Steps per segment for each time signature
        return lengths[time_signature];
    }

    int GetCurrentSegmentLength() {
        return GetSegmentLength(segments[current_segment].time_signature);
    }
};