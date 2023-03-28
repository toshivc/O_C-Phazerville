"What's the worst that could happen?"
===

## Phazerville Suite - an active o_C firmware fork

Using [Benisphere](https://github.com/benirose/O_C-BenisphereSuite) as a starting point, this branch takes the Hemisphere Suite in new directions, with several new applets and enhancements to existing ones. I've merged bleeding-edge features from other clever developers, with the goal of cramming as much functionality and flexibility into the nifty dual-applet design as possible!

I've also managed to squeeze in several stock O&C firmware apps: **Quantermain, Piqued, Acid Curds, Harrington 1200, Sequins** & **Quadraturia** are all here! Check the [Wiki](https://github.com/djphazer/O_C-BenisphereSuite/wiki) for more info.

### Notable Features in this branch:

* A new App called [**Calibr8or**](https://github.com/djphazer/O_C-BenisphereSuite/wiki/Calibr8or)
  - quad performance quantizer + pitch CV fine-tuning tool, 4 preset banks
* Expanded internal clock
  - Note: press both UP+DOWN buttons quickly to access the [**Clock Setup**](https://github.com/djphazer/O_C-BenisphereSuite/wiki/Clock-Setup) screen
  - Syncs to external clock on TR1, configurable PPQN
  - MIDI Clock out via USB
  - Independent multipliers for each internal trigger
  - Manual triggers (convenient for jogging or resetting a sequencer, testing)
* Modal-editing style navigation (push to toggle editing)
* **DualTM** - ShiftReg has been upgraded to two concurrent 32-bit registers governed by the same length/prob/scale/range settings
  - outputs assignable to Pitch, Mod, Trig, Gate from either register. Assignable CV inputs. Massive modulation potential!
* **EbbAndLfo** (via [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/trig-and-tides))
  - mini implementation of MI Tides, with v/oct tracking
* **EuclidX** - AnnularFusion got a makeover, now includes padding, configurable CV input modulation
  - (credit to [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/expanded-clock-div) and [adegani](https://github.com/adegani/O_C-HemisphereSuite))
* LoFi Tape has been transformed into **LoFi Echo** - a crazy bitcrushing digital delay line
  - (credit to [armandvedel](https://github.com/armandvedel/O_C-HemisphereSuite_log) for the initial idea)
* Sequence5 -> **SequenceX** (8 steps max) (from [logarhythm](https://github.com/Logarhythm1/O_C-HemisphereSuite))
* lots of other small tweaks + experimental applets

### How do I try it?

Check the [Releases](https://github.com/djphazer/O_C-BenisphereSuite/releases) section for a .hex file, or clone the repository and build it yourself! I think the beauty of this module is the fact that it's relatively easy to modify and build the source code to reprogram it. You are free to customize the firmware, similar to how you've no doubt already selected a custom set of physical modules.

### How do I build it?

Building the code is fairly simple using Platform IO, a Python-based build toolchain, available as either a [standalone CLI](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html):
```
wget https://raw.githubusercontent.com/platformio/platformio-core-installer/master/get-platformio.py -O get-platformio.py
python3 get-platformio.py
```
...or as a [a full-featured IDE](https://platformio.org/install/ide), as well as a plugin for VSCode and other existing IDEs.

The project lives within the `software/o_c_REV` directory. From there, you can Build and Upload via USB to your module:
```
pio run -e oc_prod -t upload
```
Alternate build environment configurations exist in `platformio.ini` for VOR, Buchla, etc. To build all the defaults, simply use `pio run`

### Credits

Shoutout to Logarhythm for the incredible **TB-3PO** sequencer.
To herrkami and Beni for their work on **BugCrack**.
Beni also gets massive props for **DrumMap** and the **ProbDiv / ProbMelo** applets.
And of course thank you to Chysn for the fantastic framework from which we've all drawn inspiration.

This is a fork of [Benisphere Suite](https://github.com/benirose/O_C-BenisphereSuite) which is a fork of [Hemisphere Suite](https://github.com/Chysn/O_C-HemisphereSuite) by Jason Justian (aka chysn).

ornament**s** & crime**s** is a collaborative project by Patrick Dowling (aka pld), mxmxmx and Tim Churches (aka bennelong.bicyclist) (though mostly by pld and bennelong.bicyclist). it **(considerably) extends** the original firmware for the o_C / ASR eurorack module, designed by mxmxmx.

http://ornament-and-cri.me/
