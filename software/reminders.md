# To Do
## SegSeq

20/08/2024
- improve UI
    - Ui should inlcude up to 8 boxes representing each segment
    - user can scroll to eachsegment
    - number of segments rendered is max no of segments
    - cursor should skip if not < 8 segments
    - cursor should render the edit_segment
    - when randomising, make the boxes of each segment jump, like the die in TB3PO
- Gate generation
    - need to add note duration
    - notes have a probablity to be ON or OFF (gate/rest).
    - they have a length (which we can use to prioritise regular note/rest lengths, ie so triplets are rare?)
    - there should be some vriable which can control if the sequence is dense with sparse rests, or dense with dense rests.
    - For instance. generate ON gates with short lengths, and rests with long lengths OR generate gates with long length and short rests. OR generate long notes with long rests, etc.
- Pitch generation
    - add a variable which decides if the pitch should continue in the same direction (up/down)
    - add a variable  which determines the amount the pitch should move by (eg tones, 3rds, 5ths) NOTE: this will have to be some arbitrary number, as quantizing happens after
    - Should be a probability, so small steps are likely, but an occasional big step is possible
    - Only generate pitches for steps that have active gates
        - Could consider legato, where pitches are generated for inacive steps
    - maybe include some variable which determines how likely it is to borrow pitch sequences/phrases from other segments?
- Randomization
    - Steal the Seed + gfx from TB3PO
    - randomising all should randomise key, scale, time
    - randomising a segment should not randomise key, scale, time, but all the other parameters

## UI draft

|------------------|
|     Segments     |
|------------------|
|Num:n    d0xABCD  |
|
|  0 X 0 0 0 0 0 0 |
| 4/4   C   AEOL   |
|
|------------------|

## Working Principle
Segments
* The generated sequence is split into musical bars or segments.
* The number of segments in the sequence is determined by the `num_segments`.
* The maximum `num_segments` is defined by `SEQ_MAX_SEGS` which by default is 8.
* Each segment has a `time_signature`. Available time signatures include: 2/4, 3/4, 4/4, 5/4. This determines how long each segment is
* Each segment has its own `key` and `scale`. This determines how each segment, or bar, should be quantized to musical pitches.

Phrases
* Each segment consists of a sequence of note pitches and gates that are generated.
* Segments are constructed in musical phrases. These are short components of a melodic idea.
* Phrases have a `phrase_length` which determines how long the phrase can be. Prases can be as short as a quater note and as long as a semibreve.
* Phrases can go over the bar line. In other words, they can start before in one segment and end in the next segment.
* `Phrase_repetition_probability` decides how likely it is that a phase is repeated. At its lowest value the phrase is entirely new, in the middle the phrase repeats some parts but changes or adds new parts, and at its maximum is entirely repeated.
* When some or all of a phrase is repeated, it is borrowed from a previously generated phrase. 
    * For musical results, when repeating in multiple phrases, it should  somehow borrow from the same phrase. For instance it should be possible to have the phrase sequence ABACABAD where A is borrowed from alot, B is borrowed from sometimes and C and D are unique. Also the order of borrowing is important. Often in music phrases are repeated in even intervals, and seperated by other new phrases. For instance the call and response.

Gate Generation
* Gates are generated for each phrase.
* Generating the gates has a `note_density` which detemines how many notes are in the phrase. The remaining inactive gates are rests.
* Every note also has a `note_duration_probability` which determines the probability of how long the note is. At its minimum notes can be as short as semi-quaver, and at is maximum they can be held for a semibreve.
* The interconnectedness of these two parameters determine the rest duration.Some example scenarios illustrate the concept: 
    * Low `note_density` and low `note_duration_probability` gives few short stacatto notes and long rests.
    * Low `note_density` and high `note_duration_probability` gives few notes with long duration and short rests.
    * High `note_density` and low `note_duration_probability` gives many short notes with short rests
    * High `note_density` and high `note_duration_probability` gives many long notes with very short rests.

TODO Pitch Generation
* For each note that is generated, a pitch is also generated.
* `pitch_range` constrains the generated sequence within a specified range. At its minimum the range is roughly a 5th. At its maximum pitches can have a range up to 3 octaves.
* The `center_pitch` determines the offset of the sequence pitch. at low values generated sequence tend to be closer to the bass register, whil at higher values sequences are in the soprano range.
* However when constructing these segments (ie not repeating them or repeating only some of the phrase) lets use `pitch_interval_distribution` to determine the pitches.

Generation
* All 'randomisation' of the values discussed above are done with a seed. This allows deterministic generation of sequences.
* By using a seed, sequences can be recalled later.
* To avoid stuttering, Amortizing is implemented to split the generation of the sequence over multiple frames.



Some additional notes
* Global parameters for all segments includes: `pitch_range`, `center_pitch`, `note_density`, `note_duration_probability`, `pitch_interval_distribution`
* Local parameters for each segment includes: `time_signature`, `key`, `scale`, `phrase_repetition_probability`, `phrase_length`
