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

// A "tick" is one ISR cycle, which happens 16666.667 times per second, or a million
// times per minute. A "tock" is a metronome beat.

#ifndef CLOCK_MANAGER_H
#define CLOCK_MANAGER_H

#define MIDI_CLOCK_LATENCY 30

const uint16_t CLOCK_TEMPO_MIN = 10;
const uint16_t CLOCK_TEMPO_MAX = 300;

class ClockManager {
    static ClockManager *instance;

    uint16_t tempo; // The set tempo, for display somewhere else
    uint32_t ticks_per_tock; // Based on the selected tempo in BPM
    bool running = 0; // Specifies whether the clock is running for interprocess communication
    bool paused = 0; // Specifies whethr the clock is paused
    bool forwarded = 0; // Master clock forwarding is enabled when true

    uint32_t beat_tick[3] = {0,0,0}; // The tick to count from
    uint32_t last_tock_check[3] = {0,0,0}; // To avoid checking the tock more than once per tick
    bool tock = 0; // The most recent tock value
    int8_t tocks_per_beat[3] = {1, 1, 24}; // Multiplier
    bool cycle = 0; // Alternates for each tock, for display purposes
    int8_t count[3] = {0,0,0}; // Multiple counter

    ClockManager() {
        SetTempoBPM(120);
    }

public:
    static ClockManager *get() {
        if (!instance) instance = new ClockManager;
        return instance;
    }

    void SetMultiply(int8_t multiply, bool ch = 0) {
        multiply = constrain(multiply, 1, 24);
        tocks_per_beat[ch] = multiply;
    }

    /* Set ticks per tock, based on one million ticks per minute divided by beats per minute.
     * This is approximate, because the arithmetical value is likely to be fractional, and we
     * need to live with a certain amount of imprecision here. So I'm not even rounding up.
     */
    void SetTempoBPM(uint16_t bpm) {
        bpm = constrain(bpm, CLOCK_TEMPO_MIN, CLOCK_TEMPO_MAX);
        ticks_per_tock = 1000000 / bpm; // NJM: scale by 24 to reduce rounding errors
        tempo = bpm;
    }

    int8_t GetMultiply(bool ch = 0) {return tocks_per_beat[ch];}

    /* Gets the current tempo. This can be used between client processes, like two different
     * hemispheres.
     */
    uint16_t GetTempo() {return tempo;}

    void Reset() {
        for (int ch = 0; ch < 3; ch++) {
            beat_tick[ch] = OC::CORE::ticks;
            count[ch] = 0;
        }
        beat_tick[2] -= MIDI_CLOCK_LATENCY;
        cycle = 1 - cycle;
    }

    void Start(bool p = 0) {
        // forwarded = 0; // NJM- logical clock can be forwarded, too
        running = 1;
        paused = p;
    }

    void Stop() {
        running = 0;
        paused = 0;
    }

    void Pause() {paused = 1;}

    void ToggleForwarding() {forwarded = 1 - forwarded;}

    void SetForwarding(bool f) {forwarded = f;}

    bool IsRunning() {return (running && !paused);}

    bool IsPaused() {return paused;}

    bool IsForwarded() {return forwarded;}

    /* Returns true if the clock should fire on this tick, based on the current tempo and multiplier */
    bool Tock(int ch = 0) {
        uint32_t now = OC::CORE::ticks;
        if (now == last_tock_check[ch]) return false; // cancel redundant check
        last_tock_check[ch] = now;

        tock = (now - beat_tick[ch]) >= (count[ch]+1)*ticks_per_tock / static_cast<uint32_t>(tocks_per_beat[ch]);
        if (tock) {
            if (++count[ch] >= tocks_per_beat[ch])
            {
                beat_tick[ch] += ticks_per_tock;
                count[ch] = 0;
                if (ch == 0) cycle = 1 - cycle;
            }
        }

        return tock;
    }

    // Returns true if MIDI Clock should be sent on this tick
    bool MIDITock() {
        return Tock(2);
    }

    bool EndOfBeat(bool ch = 0) {return count[ch] == 0;}

    bool Cycle(bool ch = 0) {return cycle;}
};

ClockManager *ClockManager::instance = 0;

#endif // CLOCK_MANAGER_H
