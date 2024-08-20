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