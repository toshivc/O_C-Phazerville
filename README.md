Welcome to Benisphere Suite, djphazer mod (Phazerville Suite)
===

## An active fork expanding upon Hemisphere Suite.

Using [Benisphere](https://github.com/benirose/O_C-BenisphereSuite) as a starting point, this "phazerville" branch takes the Hemisphere Suite in new directions, with many new applets and enhancements to existing ones, while also removing most full-width apps and MIDI-related stuff, abandoning the original minimalist approach, with the goal of cramming as much functionality and flexibility into the nifty dual-applet design as possible.

I've merged bleeding-edge features from various other branches, and confirmed that it compiles and runs on my uO_C.

### Notable Features in this branch:

* LoFi Tape has been transformed into LoFi Echo (credit to [armandvedel](https://github.com/armandvedel/O_C-HemisphereSuite_log) for the initial idea)
* ShiftReg has been upgraded to DualTM - two concurrent 32-bit registers governed by the same length/prob/scale/range settings, both outputs configurable to Pitch, Mod, Trig, Gate from either register
* AnnularFusion got a makeover, now includes configurable CV input modulation (credit to [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/expanded-clock-div) and [adegani](https://github.com/adegani/O_C-HemisphereSuite))
* Sequence5 -> SequenceX (8 steps max) (from [logarhythm](https://github.com/Logarhythm1/O_C-HemisphereSuite))
* EbbAndLfo (via [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/trig-and-tides)) - mini implementation of MI Tides, with v/oct tracking
* Improved internal clock and left-to-right clock forwarding controls
* Modal-editing style navigation on some applets (TB-3PO, DualTM)

### How do I try it?

I might release a .hex file if there is demand... I just need to make sure I understand my licensing obligations and give proper credit. Meanwhile, I'm contributing my work upstream to Benispheres.

### How do I build it?

You can download this repo and build the code following the ["Method B" instruction](https://ornament-and-cri.me/firmware/#method_b) from the Ornament and Crime website. Very specific legacy versions of the Arduino IDE and Teensyduino add-on are required to build, and are not installable on 64-bit only systems, like Mac OS. You must use an older version (Mojave or before) or a VM to install these versions.

### Credits

This is a fork of [Benisphere Suite](https://github.com/benirose/O_C-BenisphereSuite) which is a fork of [Hemisphere Suite](https://github.com/Chysn/O_C-HemisphereSuite) by Jason Justian (aka chysn).

ornament**s** & crime**s** is a collaborative project by Patrick Dowling (aka pld), mxmxmx and Tim Churches (aka bennelong.bicyclist) (though mostly by pld and bennelong.bicyclist). it **(considerably) extends** the original firmware for the o_C / ASR eurorack module, designed by mxmxmx.

http://ornament-and-cri.me/
