# PlugData
Plugin wrapper around PureData to allow patching in a wide selection of DAWs.

<img width="1054" alt="Screenshot 2022-04-21 at 03 30 17" src="https://user-images.githubusercontent.com/44585538/164353656-c087ebe3-4325-4f02-89d9-ac57c5ca332f.png">

<img width="1054" alt="Screenshot 2022-04-21 at 03 35 01" src="https://user-images.githubusercontent.com/44585538/164354121-bb196ebc-599e-4d4a-9083-ee580ad4317a.png">

PlugData is a plugin wrapper for PureData, featuring a new GUI made with JUCE. This is still a WIP, and there are probably still some bugs. By default, it ships with the ELSE collection of externals and abstractions. The aim is to provide a more comfortable patching experience for a large selection of DAWs. It can also be used as a standalone replacement for pure-data.

For Arch users, you can install [the AUR package](https://aur.archlinux.org/packages/plugdata-git) manually or via an AUR helper.

## Build

```
git clone --recursive https://github.com/timothyschoen/PlugData.git
cd PlugData
mkdir build && cd build
cmake .. (the generator can be specified using -G"Unix Makefiles", -G"XCode" or -G"Visual Studio 16 2019" -A x64)
cmake --build .
```

**Important:**
- Please ensure that the git submodules are initialized and updated! You can use the `--recursive` option while cloning or `git submodule update --init --recursive` in the PlugData repository .
- On Linux, Juce framework requires to install dependencies, please refer to [Linux Dependencies.md](https://github.com/juce-framework/JUCE/blob/master/docs/Linux%20Dependencies.md) and use the full command.
- The CMake build system has been tested with *Unix Makefiles*, *XCode* and *Visual Studio 16 2019*.

## Credits
Supported by [Deskew Technologies](https://gigperformer.com)

- [Camomile](https://github.com/pierreguillot/Camomile) by Pierre Guillot
- [ELSE](https://github.com/porres/pd-else) by Alexandre Torres Porres
- [cyclone](https://github.com/porres/pd-cyclone) by Krzysztof Czaja, Hans-Christoph Steiner, Fred Jan Kraan, Alexandre Torres Porres, Derek Kwan, Matt Barber and others (note: Cyclone is included to offer an easy entry point for Max users but ELSE contains several alternatives to objects in Cyclone and Pure Data Vanilla itself also has some alternatives. Not only that, but objects that weren't cloned into Cyclone also have alternatives in ELSE, see: [this](https://github.com/porres/pd-else/wiki/Cyclone-alternatives))
- [Pure Data](https://puredata.info) by Miller Puckette and others
- [libpd](https://github.com/libpd/libpd) by the Peter Brinkmann, Dan Wilcox and others
- [Juce](https://github.com/WeAreROLI/JUCE) by ROLI Ltd.
- [MoodyCamel](https://github.com/cameron314/concurrentqueue) by Cameron Desrochers
- [Kiwi](https://github.com/Musicoll/Kiwi) by Eliott Paris, Pierre Guillot and Jean Millot
- [LV2 PlugIn Technology](http://lv2plug.in) by Steve Harris, David Robillard and others
- [VST PlugIn Technology](https://www.steinberg.net/en/company/technologies/vst3.html) by Steinberg Media Technologies
- [Audio Unit PlugIn Technology](https://developer.apple.com/documentation/audiounit) by Apple
- [Juce LV2 interface](http://www.falktx.com) by Filipe Coelho

## Status
What works:
- Nearly complete support for pd
- Most ELSE and cyclone library objects work
- LV2, AU and VST3 formats available, tested on Windows (x64), Mac (ARM/x64) and Linux (ARM/x64), also works as AU MIDI processor for Logic
- Receive 512 DAW parameters using [receive param1], [receive param2], etc. 
- Receive DAW playhead position, tempo and more using the [playhead] abstraction

Known issues:
- Externals are broken on Windows
- Tabs sometimes close when closing patch editor
- Broken objects:
  - text define
- There may still be some more bugs

## Roadmap

These are all the things I'm hoping to implement before version 0.6.
I might release some smaller versions inbetween if any serious bugs are being found.

FEATURES:
- Allow multiple opened projects
- Connection style per individual connection
- Smart patching:
	- Shift-drag to place object inbetween connection
	- Shift while creating connection to create multiple connections
	- Dragging over toggles and radiocomponents to activate them
- Documentation browser
- Panel to view and set DAW parameters

PD COMPATIBILITY:
- Fix missing canvas properties
- Allow setting main canvas properties
- Fix text define object
- Fix broken ELSE objects: rec, store, various canvas related objects
- Improve accuracy of pd's "draw on canvas" system (used by [circle] for example)
- Decrease text object height a bit and allow text wrapping like in pd
- Improve comment object behaviour to be more like pd
- Support for dynamic patching

BUGS:
- Fix externals on Windows
- Fix help-file and abstraction tabs sometimes closing when exiting plugin editor in DAW
- Fix signing/notarising problem with some plugin hosts on Mac
- Fix no window outline on some Linux distros
- Set maximum number of console messages
- Improve stability of custom connection paths
- Sometimes inlets/outlets remain highlighted after drag action
- Listatom has wrong background colour
- All atoms have wrong background colours if they're in a graph

OTHER:
- Expand pddocs with more object descriptions and inlet/outlet hover messages
- Improve number box dragging behaviour
- Improve resize-while-typing behaviour
- Improve library path settings component
- Restructure build directory
- Clean up and document code


Please contact me if you wish to contribute, I could use some help! Bug reports are also appricated, they help me to get to a stable version much faster.
