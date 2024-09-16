# SegmentedSequencer Documentation

## Overview
The SegmentedSequencer is an applet for the eurorack module O_c that generates musical sequences and outputs gate and pitch patterns. It uses a segmented approach to create varied and interesting musical phrases.

## Key Concepts

### Segments
- The generated sequence is split into musical bars or segments.
- The number of segments is determined by `num_segments` (max 8).
- Each segment has its own `time_signature` (2/4, 3/4, 4/4, 5/4), `key`, and `scale`.
- Segments represent distinct parts of the overall sequence, allowing for variation in musical structure.

### Phrases
- Segments consist of sequences of note pitches and gates.
- Phrases are short components of a melodic idea.
- `phrase_length` determines phrase duration (quarter note to semibreve, default: minim).
- Phrases can cross segment boundaries, allowing for musical ideas to flow naturally across bar lines.
- `phrase_repetition_probability` determines likelihood and extent of phrase repetition.

### Phrase Repetition
- Controlled by `phrase_repetition_probability`:
  - At 0: generates an entirely new phrase
  - At maximum: repeats an entire phrase
  - In between: repeats a proportional amount of the steps in a phrase
- When repeating, priority is given to:
  1. Changing pitches
  2. Adding new notes
  3. Removing notes

### Gate Generation
- Gates are generated probabilistically for each phrase.
- `note_density` determines how many notes are in the phrase.
- `note_duration_probability` determines note length probability.

### Pitch Generation
- `pitch_range` constrains the sequence within a specified range.
- `center_pitch` defines the middle of the pitch range.
- `pitch_interval_distribution` is a single continuous parameter that interpolates between multiple internal probability distributions, ranging from mostly small intervals to mostly large intervals.
- Melodic contour is considered per phrase, with a continuous parameter controlling the tendency for pitch movement (ascending to descending).
- Pitch sequence is generated before quantization to allow seamless transitions between segments.



### Generation and Playback
- Randomization uses a seed for deterministic generation and recall of sequences.
- Sequence continues playing while new segments are being generated.
- The use of a seed allows for reproducible results and the ability to save and recall specific sequences.

## Parameters

### Global Parameters
- `pitch_range`
- `center_pitch`
- `note_density`
- `note_duration_probability`
- `pitch_interval_distribution`

### Local Parameters (per segment)
- `time_signature`
- `key`
- `scale`
- `phrase_repetition_probability`
- `phrase_length`
- `melodic_contour`

## Implementation Notes
- All steps (e.g., 16th notes) are the same length across all segments.
- Pitch sequence is generated before quantization to keys and scales.
- Large jumps or undesired intervals may occur due to quantization between segments.
- The use of a seed for randomization ensures that sequences can be deterministically generated and recalled.

# scratchpad
### event approach ideas
* Instead of defining a sequence by steps that are on or off, define it by an event with a length.
* events have the following attributes
  * `length`: how many steps the event happens for
  * `event_type`: either a note (on) or a rest (off)
  * `phrase_start_flag`: indicates that the event is the start of a phrase.
*   