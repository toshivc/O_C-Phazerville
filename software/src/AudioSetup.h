#pragma once

#include <Audio.h>
#include <Wire.h>

// Use the web GUI tool as a guide: https://www.pjrc.com/teensy/gui/

/* Left side is just reverb, mixed in lightly.
 * Right side is a filter, to be modulated by applets...
 */
AudioInputI2S2            i2s1;           //xy=114,242
AudioOutputI2S2           i2s2;           //xy=845,221
AudioEffectFreeverb      freeverb1;     //xy=396,224
AudioAmplifier           amp1;           //xy=332,213
AudioFilterStateVariable filter1;        //xy=493,256
AudioMixer4              mixer1;         //xy=555,132
AudioMixer4              mixer2;         //xy=557,321

AudioConnection          patchCord1(i2s1, 0, mixer1, 0);
AudioConnection          patchCord2(i2s1, 0, freeverb1, 0);
AudioConnection          patchCord3(i2s1, 1, amp1, 0);
AudioConnection          patchCord4(amp1, 0, filter1, 0);
AudioConnection          patchCord5(filter1, 0, mixer2, 0);
AudioConnection          patchCord6(filter1, 1, mixer2, 1);
AudioConnection          patchCord7(filter1, 2, mixer2, 2);
AudioConnection          patchCord8(freeverb1, 0, mixer1, 3);
AudioConnection          patchCord9(mixer2, 0, i2s2, 1);
AudioConnection          patchCord10(mixer1, 0, i2s2, 0);

// this could be used for the Tuner or other pitch-tracking tricks
//AudioAnalyzeNoteFrequency notefreq1;
//AudioConnection          patchCord8(i2s1, 1, notefreq1, 0);


// --- Audio-related functions accessible from elsewhere in the O_C codebase
namespace OC {
  // use this from internal modulation mappings!
  void ModFilter1(int cv) {
    // quartertones squared
    // 1 Volt is 576 Hz
    // 2V = 2304 Hz
    // 3V = 5184 Hz
    // 4V = 9216 Hz
    float f = abs(cv) / 64;
    f *= f;
    filter1.frequency(f);
  }
  void SelectHPF() {
    mixer2.gain(0, 0.0); // LPF
    mixer2.gain(1, 0.0); // BPF
    mixer2.gain(2, 1.0); // HPF
  }
  void SelectBPF() {
    mixer2.gain(0, 0.0); // LPF
    mixer2.gain(1, 1.0); // BPF
    mixer2.gain(2, 0.0); // HPF
  }
  void SelectLPF() {
    mixer2.gain(0, 1.0); // LPF
    mixer2.gain(1, 0.0); // BPF
    mixer2.gain(2, 0.0); // HPF
  }

  // ----- called from setup() in Main.cpp
  static void AudioInit() {
    AudioMemory(16);
    freeverb1.roomsize(0.9);
    freeverb1.damping(0.3);
    mixer1.gain(3, 0.33); // reverb wet
    
    amp1.gain(0.9);
    SelectLPF();

    filter1.resonance(1.05);
  }

}
