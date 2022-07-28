![alt text](https://assets.juce.com/juce/JUCE_banner_github.png "JUCE")

JUCE is an open-source cross-platform C++ application framework for creating high quality
desktop and mobile applications, including VST, VST3, AU, AUv3, AAX and LV2 audio plug-ins
and plug-in hosts. JUCE can be easily integrated with existing projects via CMake, or can
be used as a project generation tool via the [Projucer](https://juce.com/discover/projucer),
which supports exporting projects for Xcode (macOS and iOS), Visual Studio, Android Studio,
Code::Blocks and Linux Makefiles as well as containing a source code editor.

## Getting Started

The JUCE repository contains a [master](https://github.com/juce-framework/JUCE/tree/master)
and [develop](https://github.com/juce-framework/JUCE/tree/develop) branch. The develop branch
contains the latest bugfixes and features and is periodically merged into the master
branch in stable [tagged releases](https://github.com/juce-framework/JUCE/releases)
(the latest release containing pre-built binaries can be also downloaded from the
[JUCE website](https://juce.com/get-juce)).

JUCE projects can be managed with either the Projucer (JUCE's own project-configuration
tool) or with CMake.

### The Projucer

The repository doesn't contain a pre-built Projucer so you will need to build it
for your platform - Xcode, Visual Studio and Linux Makefile projects are located in
[extras/Projucer/Builds](/extras/Projucer/Builds)
(the minimum system requirements are listed in the __System Requirements__ section below).
The Projucer can then be used to create new JUCE projects, view tutorials and run examples.
It is also possible to include the JUCE modules source code in an existing project directly,
or build them into a static or dynamic library which can be linked into a project.

For further help getting started, please refer to the JUCE
[documentation](https://juce.com/learn/documentation) and
[tutorials](https://juce.com/learn/tutorials).

### CMake

Version 3.15 or higher is required. To use CMake, you will need to install it,
either from your system package manager or from the [official download
page](https://cmake.org/download/). For comprehensive documentation on JUCE's
CMake API, see the [JUCE CMake documentation](/docs/CMake%20API.md). For
examples which may be useful as starting points for new CMake projects, see the
[CMake examples directory](/examples/CMake).

#### Building Examples

To use CMake to build the examples and extras bundled with JUCE, simply clone
JUCE and then run the following commands, replacing "DemoRunner" with the name
of the target you wish to build.

    cd /path/to/JUCE
    cmake . -B cmake-build -DJUCE_BUILD_EXAMPLES=ON -DJUCE_BUILD_EXTRAS=ON
    cmake --build cmake-build --target DemoRunner

## Minimum System Requirements

#### Building JUCE Projects

- __macOS/iOS__: Xcode 10.1 (macOS 10.13.6)
- __Windows__: Windows 8.1 and Visual Studio 2015 Update 3 64-bit
- __Linux__: g++ 5.0 or Clang 3.4 (for a full list of dependencies, see
[here](/docs/Linux%20Dependencies.md)).
- __Android__: Android Studio on Windows, macOS or Linux

#### Deployment Targets

- __macOS__: macOS 10.7
- __Windows__: Windows Vista
- __Linux__: Mainstream Linux distributions
- __iOS__: iOS 9.0
- __Android__: Jelly Bean (API 16)

## Contributing

Please see our [contribution guidelines](.github/contributing.md).

## Licensing

The core JUCE modules (juce_audio_basics, juce_audio_devices, juce_core and juce_events)
are permissively licensed under the terms of the
[ISC license](http://www.isc.org/downloads/software-support-policy/isc-license/).
Other modules are covered by a
[GPL](https://www.gnu.org/licenses/gpl-3.0.en.html)/Commercial license.

There are multiple commercial licensing tiers for JUCE, with different terms for each:
- JUCE Personal (developers or startup businesses with revenue under 50K USD) - free
- JUCE Indie (small businesses with revenue under 500K USD) - $40/month or $800 perpetual
- JUCE Pro (no revenue limit) - $130/month or $2600 perpetual
- JUCE Educational (no revenue limit) - free for bona fide educational institutes

For full terms see [LICENSE.md](LICENSE.md).

The JUCE framework contains the following dependencies:
- [Oboe](modules/juce_audio_devices/native/oboe/) ([Apache 2.0](modules/juce_audio_devices/native/oboe/LICENSE))
- [FLAC](modules/juce_audio_formats/codecs/flac/) ([BSD](modules/juce_audio_formats/codecs/flac/Flac%20Licence.txt))
- [Ogg Vorbis](modules/juce_audio_formats/codecs/oggvorbis/) ([BSD](modules/juce_audio_formats/codecs/oggvorbis/Ogg%20Vorbis%20Licence.txt))
- [CoreAudioUtilityClasses](modules/juce_audio_plugin_client/AU/CoreAudioUtilityClasses/) ([Apple](modules/juce_audio_plugin_client/AU/CoreAudioUtilityClasses/AUBase.cpp))
- [AUResources.r](modules/juce_audio_plugin_client/AUResources.r) ([Apple](modules/juce_audio_plugin_client/AUResources.r))
- [LV2](modules/juce_audio_processors/format_types/LV2_SDK/) ([ISC](modules/juce_audio_processors/format_types/LV2_SDK/lv2/COPYING))
- [pslextensions](modules/juce_audio_processors/format_types/pslextensions/ipslcontextinfo.h) ([Public domain](modules/juce_audio_processors/format_types/pslextensions/ipslcontextinfo.h))
- [VST3](modules/juce_audio_processors/format_types/VST3_SDK/) ([Proprietary Steinberg VST3/GPLv3](modules/juce_audio_processors/format_types/VST3_SDK/LICENSE.txt))
- [zlib](modules/juce_core/zip/zlib/) ([zlib](modules/juce_core/zip/zlib/README))
- [Box2D](modules/juce_box2d/box2d/) ([zlib](modules/juce_box2d/box2d/Box2D.h))
- [jpeglib](modules/juce_graphics/image_formats/jpglib/) ([Independent JPEG Group License](modules/juce_graphics/image_formats/jpglib/README))
- [pnglib](modules/juce_graphics/image_formats/pnglib/) ([zlib](modules/juce_graphics/image_formats/pnglib/LICENSE))
- [GLEW](modules/juce_opengl/opengl/juce_gl.h) ([BSD](modules/juce_opengl/opengl/juce_gl.h)), including [Mesa](modules/juce_opengl/opengl/juce_gl.h) ([MIT](modules/juce_opengl/opengl/juce_gl.h)) and [Khronos](modules/juce_opengl/opengl/juce_gl.h) ([MIT](modules/juce_opengl/opengl/juce_gl.h))

The JUCE examples are licensed under the terms of the
[ISC license](http://www.isc.org/downloads/software-support-policy/isc-license/).

Dependencies in the examples:
- [reaper-sdk](examples/Plugins/extern/) ([zlib](examples/Plugins/extern/LICENSE.md))

Dependencies in the bundled applications:
- [Projucer icons](extras/Projucer/Source/Utility/UI/jucer_Icons.cpp) ([MIT](extras/Projucer/Source/Utility/UI/jucer_Icons.cpp))

Dependencies in the build system:
- [Android Gradle](examples/DemoRunner/Builds/Android/gradle/wrapper/LICENSE-for-gradlewrapper.txt) ([Apache 2.0](examples/DemoRunner/Builds/Android/gradle/wrapper/LICENSE-for-gradlewrapper.txt))
