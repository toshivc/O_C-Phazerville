#include "resources/tideslite.h"

class EbbAndLfo : public HemisphereApplet {
public:
  const char *applet_name() { return "Ebb & LFO"; }

  void Start() { phase = 0; }

  void Controller() {
    uint32_t phase_increment = ComputePhaseIncrement(pitch + In(0));
    phase += phase_increment;
    int slope_cv = In(1) * 32768 / HEMISPHERE_3V_CV;
    int s = constrain(slope * 65535 / 127 + slope_cv, 0, 65535);
    GenerateSample(s, att_shape * 65535 / 127, dec_shape * 65535 / 127, phase,
                   sample);

    Out(0, Proportion(sample.unipolar, 65535, HEMISPHERE_MAX_CV));
    Out(1, Proportion(sample.bipolar, 32767, HEMISPHERE_MAX_CV / 2));

    if (knob_accel > (1 << 8))
      knob_accel--;
  }

  void View() {
    gfxHeader(applet_name());

    gfxPrintFreq(0, 15, pitch);
    if (cursor == 0)
      gfxCursor(0, 23, 32);

    gfxPrint(0, 25, slope);
    if (cursor == 1)
      gfxCursor(0, 33, 32);

    gfxPrint(0, 35, att_shape);
    if (cursor == 2)
      gfxCursor(0, 43, 32);

    gfxPrint(0, 45, dec_shape);
    if (cursor == 3)
      gfxCursor(0, 53, 32);
  }

  void SetHelp() {}

  void OnButtonPress() {
    cursor++;
    cursor %= 4;
  }

  void OnEncoderMove(int direction) {
    switch (cursor) {
    case 0: {
      uint32_t old_pi = ComputePhaseIncrement(pitch);
      pitch += (knob_accel >> 8) * direction;
      while (ComputePhaseIncrement(pitch) == old_pi) {
        pitch += direction;
      }
      break;
    }
    case 1: {
      // slope += (knob_accel >> 4) * direction;
      slope = constrain(slope + direction, 0, 127);
      break;
    }
    case 2: {
      att_shape = constrain(att_shape + direction, 0, 127);
      break;
    }
    case 3: {
      dec_shape = constrain(dec_shape + direction, 0, 127);
      break;
    }
    }
    if (knob_accel < (1 << 13))
      knob_accel <<= 1;
  }

  uint64_t OnDataRequest() { return 0; }

  void OnDataReceive(uint64_t data) {}

private:
  int cursor;
  int16_t pitch;
  int slope = 64;
  int att_shape = 64;
  int dec_shape = 64;
  int smoothness;
  TidesLiteSample sample;

  int knob_accel = 1 << 8;

  uint32_t phase;

  void gfxPrintFreq(int x, int y, int16_t pitch) {
    uint32_t num = ComputePhaseIncrement(pitch);
    uint32_t denom = 0xffffffff / 16666;
    bool swap = num < denom;
    if (swap) {
      uint32_t t = num;
      num = denom;
      denom = t;
    }
    int int_part = num / denom;
    int digits = 0;
    if (int_part < 10)
      digits = 1;
    else if (int_part < 100)
      digits = 2;
    else if (int_part < 1000)
      digits = 3;
    else
      digits = 4;

    gfxPos(x, y);
    gfxPrint(int_part);
    gfxPrint(".");

    num %= denom;
    while (digits < 4) {
      num *= 10;
      gfxPrint(num / denom);
      num %= denom;
      digits++;
    }
    if (swap) {
      gfxPrint("s");
    } else {
      gfxPrint("Hz");
    }
    // int oom = 1;
    // int i = -3;
    // while (oom * phase_inc < 1000 * (uint64_t) 0xffffffff) {
    //   oom *= 10;
    //   i++;
    // }
    // gfxCursor(x, y);
    // gfxPrint(x, y, phase_inc * oom / 0xffffffff);
  }
};
////////////////////////////////////////////////////////////////////////////////
//// Hemisphere Applet Functions
///
///  Once you run the find-and-replace to make these refer to EbbAndLfo,
///  it's usually not necessary to do anything with these functions. You
///  should prefer to handle things in the HemisphereApplet child class
///  above.
////////////////////////////////////////////////////////////////////////////////
EbbAndLfo EbbAndLfo_instance[2];

void EbbAndLfo_Start(bool hemisphere) {
  EbbAndLfo_instance[hemisphere].BaseStart(hemisphere);
}

void EbbAndLfo_Controller(bool hemisphere, bool forwarding) {
  EbbAndLfo_instance[hemisphere].BaseController(forwarding);
}

void EbbAndLfo_View(bool hemisphere) {
  EbbAndLfo_instance[hemisphere].BaseView();
}

void EbbAndLfo_OnButtonPress(bool hemisphere) {
  EbbAndLfo_instance[hemisphere].OnButtonPress();
}

void EbbAndLfo_OnEncoderMove(bool hemisphere, int direction) {
  EbbAndLfo_instance[hemisphere].OnEncoderMove(direction);
}

void EbbAndLfo_ToggleHelpScreen(bool hemisphere) {
  EbbAndLfo_instance[hemisphere].HelpScreen();
}

uint64_t EbbAndLfo_OnDataRequest(bool hemisphere) {
  return EbbAndLfo_instance[hemisphere].OnDataRequest();
}

void EbbAndLfo_OnDataReceive(bool hemisphere, uint64_t data) {
  EbbAndLfo_instance[hemisphere].OnDataReceive(data);
}