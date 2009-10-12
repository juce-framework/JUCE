/*
    This file contains settings that you might want to explicitly apply to
    your Juce build.

    These flags enable or disable juce features - if you're linking to juce as
    a library, then to change them, you'd need to alter your juce_Config.h file and
    recompile the juce lib. But because we're using the amalgamated file, you can
    just include this file before including your juce_amalgamated.cpp file to
    have the same effect.

    If you leave any of these commented-out, they'll take on the default value
    assigned to them in juce_Config.h, so to force them on or off, just set them
    to an explicit 0 or 1 in here.
*/

#define JUCE_ONLY_BUILD_CORE_LIBRARY   1
//#define JUCE_FORCE_DEBUG  1
//#define JUCE_LOG_ASSERTIONS  1
//#define JUCE_ASIO  1
//#define JUCE_ALSA  1
//#define JUCE_QUICKTIME  1
//#define JUCE_OPENGL  1
//#define JUCE_USE_FLAC  1
//#define JUCE_USE_OGGVORBIS  1
//#define JUCE_USE_CDBURNER  1
//#define JUCE_ENABLE_REPAINT_DEBUGGING  1
//#define JUCE_USE_XINERAMA  1
//#define JUCE_USE_XSHM  1
//#define JUCE_PLUGINHOST_VST  1
//#define JUCE_PLUGINHOST_AU  1
//#define JUCE_CHECK_MEMORY_LEAKS  1
//#define JUCE_CATCH_UNHANDLED_EXCEPTIONS  1
//#define JUCE_STRINGS_ARE_UNICODE  1
