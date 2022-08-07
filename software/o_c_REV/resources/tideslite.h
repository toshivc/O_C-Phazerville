
const int16_t kOctave = 12 * 128;

/*
import numpy as np

sample_rate = 16666

excursion = float(1 << 32)

a4_midi = 69
a4_pitch = 440.0
notes = np.arange(0 * 128.0, 12 * 128.0 + 16, 16) / 128.0
pitches = a4_pitch * 2 **((notes - a4_midi) / 12)
increments = excursion / sample_rate * pitches

increments.astype(int)
*/
const uint32_t lut_increments[] = {
    2106971, 2122239, 2137618, 2153108, 2168710, 2184425, 2200255, 2216199,
    2232258, 2248434, 2264727, 2281138, 2297668, 2314318, 2331089, 2347981,
    2364995, 2382133, 2399395, 2416782, 2434295, 2451935, 2469702, 2487599,
    2505625, 2523782, 2542070, 2560491, 2579046, 2597734, 2616559, 2635519,
    2654617, 2673854, 2693230, 2712746, 2732404, 2752204, 2772147, 2792235,
    2812469, 2832850, 2853377, 2874054, 2894881, 2915858, 2936988, 2958270,
    2979707, 3001300, 3023048, 3044954, 3067019, 3089244, 3111630, 3134178,
    3156890, 3179766, 3202808, 3226017, 3249394, 3272940, 3296657, 3320546,
    3344608, 3368845, 3393257, 3417846, 3442613, 3467560, 3492687, 3517996,
    3543489, 3569167, 3595031, 3621082, 3647321, 3673751, 3700373, 3727187,
    3754196, 3781401, 3808802, 3836402, 3864202, 3892204, 3920409, 3948818,
    3977432, 4006254, 4035285, 4064527, 4093980, 4123647, 4153528, 4183626,
    4213943
};

uint32_t ComputePhaseIncrement(int16_t pitch) {
  int16_t num_shifts = 0;
  while (pitch < 0) {
    pitch += kOctave;
    --num_shifts;
  }
  while (pitch >= kOctave) {
    pitch -= kOctave;
    ++num_shifts;
  }
  // Lookup phase increment
  uint32_t a = lut_increments[pitch >> 4];
  uint32_t b = lut_increments[(pitch >> 4) + 1];
  uint32_t phase_increment = a + ((b - a) * (pitch & 0xf) >> 4);
  // Compensate for downsampling
  return num_shifts >= 0 ? phase_increment << num_shifts
                          : phase_increment >> -num_shifts;
}
