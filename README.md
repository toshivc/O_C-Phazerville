[![PlatformIO CI](https://github.com/djphazer/O_C-Phazerville/actions/workflows/firmware.yml/badge.svg)](https://github.com/djphazer/O_C-Phazerville/actions/workflows/firmware.yml)

Phazerville Suite - an active o_C firmware fork
===
[![SynthDad's video overview](http://img.youtube.com/vi/XRGlAmz3AKM/0.jpg)](http://www.youtube.com/watch?v=XRGlAmz3AKM "Phazerville; newest firmware for Ornament and Crime. Tutorial and patch ideas")

Watch SynthDad's **video overview** (above) or check the [**Wiki**](https://github.com/djphazer/O_C-Phazerville/wiki) for more info.

[Download a **Release**](https://github.com/djphazer/O_C-Phazerville/releases) or [Request a **Custom Build**](https://github.com/djphazer/O_C-Phazerville/discussions/38).

## Stolen Ornaments

Using [**Benisphere**](https://github.com/benirose/O_C-BenisphereSuite) as a starting point, this branch takes the **Hemisphere Suite** in new directions, with several new applets and enhancements to existing ones. I wanted to collect all the bleeding-edge features from other clever developers, with the goal of cramming as much functionality and flexibility into the nifty dual-applet design as possible!

I've also included **all of the stock O&C firmware apps** plus a few others, _but they don't all fit in one .hex_. As a courtesy, I provide **pre-built .hex files** with a selection of Apps in my [**Releases**](https://github.com/djphazer/O_C-Phazerville/releases). You can also tell a robot to make a [**Custom Build**](https://github.com/djphazer/O_C-Phazerville/discussions/38) for you...

...or clone the repo, customize the `platformio.ini` file, and build it yourself! ;-)
I think the beauty of this module is the fact that it's relatively easy to modify and build the source code to reprogram it. You are free to customize the firmware, similar to how you've no doubt already selected a custom set of physical modules.

### New Crimes

* Up to 16 Presets in the new [**Hemisphere Config**](https://github.com/djphazer/O_C-Phazerville/wiki/Hemisphere-Config)
* Modal-editing style cursor navigation (and other usability tweaks)
* Expanded internal [**Clock Setup**](https://github.com/djphazer/O_C-Phazerville/wiki/Clock-Setup)
* New Apps: [**Scenes**](https://github.com/djphazer/O_C-Phazerville/wiki/Scenes) and [**Calibr8or**](https://github.com/djphazer/O_C-Phazerville/wiki/Calibr8or)
* New Applets:
  - **Pigeons** - Fibonacci melodies
  - **DivSeq** - a sequence of clock dividers
  - **PolyDiv** - parallel clock dividers, like the Subharmonicon
  - **ResetClk** - sequencing helper for DFAM
  - **[DualTM](https://github.com/djphazer/O_C-Phazerville/wiki/DualTM)** - two 32-bit shift registers. Assignable I/O.
  - **[EbbAndLfo](https://github.com/djphazer/O_C-Phazerville/wiki/Ebb-&-LFO)** (via [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/trig-and-tides)) - mini implementation of MI Tides, with v/oct tracking
  - **[EuclidX](https://github.com/djphazer/O_C-Phazerville/wiki/EuclidX)** - AnnularFusion got a makeover, now includes padding, configurable CV input modulation - (credit to [qiemem](https://github.com/qiemem/O_C-HemisphereSuite/tree/expanded-clock-div) and [adegani](https://github.com/adegani/O_C-HemisphereSuite))
  - LoFi Tape -> **LoFi Echo** - a crazy bitcrusher & digital delay line - (credit to [armandvedel](https://github.com/armandvedel/O_C-HemisphereSuite_log) for the initial idea)
  - Sequence5 -> **SequenceX** (8 steps max) (from [logarhythm](https://github.com/Logarhythm1/O_C-HemisphereSuite))

Plus lots of other tweaks + experimental applets. Check the Wiki for documentation.

## How To Hack It

This firmware fork is built using Platform IO, a Python-based build toolchain, available as either a [standalone CLI](https://docs.platformio.org/en/latest/core/installation/methods/installer-script.html) or a [full-featured IDE](https://platformio.org/install/ide), as well as a plugin for VSCode and other existing IDEs.

The PlatformIO project lives within the `software/` directory. From there, you can Build the desired configuration and Upload via USB to your module:
```
pio run -e pewpewpew -t upload
```
Have a look inside `platformio.ini` for alternative build environment configurations - VOR, Buchla, flipped screen, etc. To build all the defaults consecutively, simply use `pio run`

_**Pro-tip**_: If you decide to fork the project, and enable GitHub Actions on your own repo, GitHub will build the files for you... ;)

### Arduino IDE
To compile with the original Arduino IDE + Teensyduino extension, simply open the `software/src/src.ino` file. The newest 2.x version should work.
Customize Apps and other flags inside `software/src/OC_options.h`. You can also disable individual applets in `software/src/hemisphere_config.h`.

## Credits

Many minds before me have made this project possible. Attribution is present in the git commit log and within individual files.
Shoutouts:
* **[Logarhythm1](https://github.com/Logarhythm1)** for the incredible **TB-3PO** sequencer, as well as **Stairs**.
* **[herrkami](https://github.com/herrkami)** and **Ben Rosenbach** for their work on **BugCrack**.
* **[benirose](https://github.com/benirose)** also gets massive props for **DrumMap**, **Shredder** and the **ProbDiv / ProbMelo** applets.
* **[qiemem](https://github.com/qiemem)** (Bryan Head) for the **Ebb&LFO** applet and its _tideslite_ backend, among other things.

And, of course, thank you to **[Chysn](https://github.com/Chysn)** for the clever applet framework from which we've all drawn inspiration.

This is a fork of [Benisphere Suite](https://github.com/benirose/O_C-BenisphereSuite) which is a fork of [Hemisphere Suite](https://github.com/Chysn/O_C-HemisphereSuite) by Jason Justian (aka chysn).

ornament**s** & crime**s** is a collaborative project by Patrick Dowling (aka pld), mxmxmx and Tim Churches (aka bennelong.bicyclist) (though mostly by pld and bennelong.bicyclist). it **(considerably) extends** the original firmware for the o_C / ASR eurorack module, designed by mxmxmx.

http://ornament-and-cri.me/
