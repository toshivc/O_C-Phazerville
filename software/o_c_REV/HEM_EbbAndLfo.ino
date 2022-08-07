#include "resources/tideslite.h"

class EbbAndLfo : public HemisphereApplet {
public:
  const char *applet_name() { return "Ebb & LFO"; }

  void Start() { phase = 0; }

  void Controller() {
    uint32_t phase_increment = ComputePhaseIncrement(In(0));
    phase += phase_increment;
    Out(0, Proportion(phase >> 16, 1 << 16, HEMISPHERE_MAX_CV));
  }
  
  void View() {

  }
  
  void SetHelp() {

  }

  void OnButtonPress() {}

  void OnEncoderMove(int direction) {}

  uint64_t OnDataRequest() { return 0; }

  void OnDataReceive(uint64_t data) {}

private:
  int16_t pitch;
  int slope;
  int att_shape;
  int dec_shape;
  int smoothness;

  int phase;
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