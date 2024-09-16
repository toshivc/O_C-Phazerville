# Phraser 

* The sequence is made up of a number of steps, defined by `num_steps`. The MAX_STEPS is 32 steps.
* Every step generate gates with a random probability to be on or off defined by `density`. 
* each gate has a duration which can be multiple steps which is defined probabilistically by `duration_prob`. 
    * TODO consider adding some kind of smart determination of durations for musical results
* Every time we generate a gate or rest, update `new_phrase_prob` to see if it should be the start of a new phrase
    * if the number of steps since the last phrase is a musical number (16, 8, 6, 4)
        * `new_phrase_prob` += 25%
    * if there has been multiple steps of rest since the current step
        * `new_phrase_prob` += 5%
    * if a prior phrase was the same phrase length (eg current-1, current-2, current-4)
        * `new_phrase_prob` += 15%
    * if the number of steps until the end of the sequence is a musical number (16, 8, 6, 4)
        * `new_phrase_prob` += 5%
* using `new_phrase_prob`, probabilisically determine if the note/rest is the start of a new phrase
* If a new phrase is started, `repeat_prob` determines the probability that the phrase will be repeated from a previous phrase.
    * if a phrase is repeate, regenerate the phrase
        * for each note in the phrase, update `similarity_prob` to see if some notes or rests in the sequence should change.
            * if the note is near the start of the phrase
                * `similarity_prob` += 15/(current_step-phase_start_step)
            * if the note is near the end of the phrase
                        * `similarity_prob` += 15/(current_step-phase_end_step)
                        * note: the end step should be known because we are copying an existing phrase, with a known length
            * if the note is very long
                * `similarity_prob` = `duration`* `duration`
            * TODO if the notes pitch is much larger or smaller then the average pitch of the phrase
        * if `similarity_prob` > seed, regenrate the note followwing the same steps as above (using density and duration)

* For all probabilities, use a seed to determine the random number so that it is deterministic. This allows for patterns to be stored/shared/saved in the future.

### User Variables
These variables can be controlled by the user.
* `density`
* `duration_prob`
* `repeat_prob`
* `seed`