# The JUCE Framework

The JUCE Framework is an open source framework licensed under a combination of
open source and commercial licences.

The JUCE Framework modules are dual-licensed under the
[AGPLv3](https://www.gnu.org/licenses/agpl-3.0.en.html) and the commercial [JUCE
licence](https://juce.com/legal/juce-8-licence/).

## The JUCE Licence

If you are not licensing the JUCE Framework modules under the
[AGPLv3](https://www.gnu.org/licenses/agpl-3.0.en.html) then by downloading,
installing, or using the JUCE Framework, or combining the JUCE Framework with
any other source code, object code, content or any other copyrightable work, you
agree to the terms of the the [JUCE 8 End User Licence
Agreement](https://juce.com/legal/juce-8-licence/), and all incorporated terms
including the [JUCE Privacy Policy](https://juce.com/legal/juce-privacy-policy/)
and the [JUCE Website Terms of
Service](https://juce.com/legal/juce-website-terms-of-service/), as applicable,
which will bind you. If you do not agree to the terms of this Agreement, we will
not license the JUCE Framework to you, and you must discontinue the installation
or download process and cease use of the JUCE Framework.

THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF MERCHANTABILITY OR FITNESS
FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

For more information, visit the [JUCE website](https://juce.com).

Full licence terms:
- [JUCE 8 End User Licence Agreement](https://juce.com/legal/juce-8-licence/)
- [JUCE Privacy Policy](https://juce.com/legal/juce-privacy-policy/)
- [JUCE Website Terms of Service](https://juce.com/legal/juce-website-terms-of-service/)

## The JUCE Framework Dependencies

The JUCE modules contain the following dependencies:
- [AudioUnitSDK](modules/juce_audio_plugin_client/AU/AudioUnitSDK/) ([Apache 2.0](modules/juce_audio_plugin_client/AU/AudioUnitSDK/LICENSE.txt))
- [Oboe](modules/juce_audio_devices/native/oboe/) ([Apache 2.0](modules/juce_audio_devices/native/oboe/LICENSE))
- [FLAC](modules/juce_audio_formats/codecs/flac/) ([BSD](modules/juce_audio_formats/codecs/flac/Flac%20Licence.txt))
- [GLEW](modules/juce_opengl/opengl/juce_gl.h) ([BSD](modules/juce_opengl/opengl/juce_gl.h)), including [Mesa](modules/juce_opengl/opengl/juce_gl.h) ([MIT](modules/juce_opengl/opengl/juce_gl.h)) and [Khronos](modules/juce_opengl/opengl/juce_gl.h) ([MIT](modules/juce_opengl/opengl/juce_gl.h))
- [Ogg Vorbis](modules/juce_audio_formats/codecs/oggvorbis/) ([BSD](modules/juce_audio_formats/codecs/oggvorbis/Ogg%20Vorbis%20Licence.txt))
- [jpeglib](modules/juce_graphics/image_formats/jpglib/) ([Independent JPEG Group License](modules/juce_graphics/image_formats/jpglib/README))
- [CHOC](modules/juce_core/javascript/choc/) ([ISC](modules/juce_core/javascript/choc/LICENSE.md)), including [QuickJS](modules/juce_core/javascript/choc/javascript/choc_javascript_QuickJS.h) ([MIT](modules/juce_core/javascript/choc/javascript/choc_javascript_QuickJS.h))
- [LV2](modules/juce_audio_processors/format_types/LV2_SDK/) ([ISC](modules/juce_audio_processors/format_types/LV2_SDK/lv2/COPYING))
- [pslextensions](modules/juce_audio_processors/format_types/pslextensions/ipslcontextinfo.h) ([Public domain](modules/juce_audio_processors/format_types/pslextensions/ipslcontextinfo.h))
- [AAX](modules/juce_audio_plugin_client/AAX/SDK/) ([Proprietary Avid AAX License/GPLv3](modules/juce_audio_plugin_client/AAX/SDK/LICENSE.txt))
- [VST3](modules/juce_audio_processors/format_types/VST3_SDK/) ([Proprietary Steinberg VST3 License/GPLv3](modules/juce_audio_processors/format_types/VST3_SDK/LICENSE.txt))
- [Box2D](modules/juce_box2d/box2d/) ([zlib](modules/juce_box2d/box2d/Box2D.h))
- [pnglib](modules/juce_graphics/image_formats/pnglib/) ([zlib](modules/juce_graphics/image_formats/pnglib/LICENSE))
- [zlib](modules/juce_core/zip/zlib/) ([zlib](modules/juce_core/zip/zlib/README))

The JUCE examples are licensed under the terms of the
[ISC license](http://www.isc.org/downloads/software-support-policy/isc-license/).

Dependencies in the examples:
- [reaper-sdk](examples/Plugins/extern/) ([zlib](examples/Plugins/extern/LICENSE.md))

Dependencies in the bundled applications:
- [Projucer icons](extras/Projucer/Source/Utility/UI/jucer_Icons.cpp) ([MIT](extras/Projucer/Source/Utility/UI/jucer_Icons.cpp))

Dependencies in the build system:
- [Android Gradle](examples/DemoRunner/Builds/Android/gradle/wrapper/LICENSE-for-gradlewrapper.txt) ([Apache 2.0](examples/DemoRunner/Builds/Android/gradle/wrapper/LICENSE-for-gradlewrapper.txt))
