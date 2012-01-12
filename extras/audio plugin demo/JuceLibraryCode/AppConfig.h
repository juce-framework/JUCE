/*

    IMPORTANT! This file is auto-generated each time you save your
    project - if you alter its contents, your changes may be overwritten!

    If you want to change any of these values, use the Introjucer to do so,
    rather than editing this file directly!

    Any commented-out settings will assume their default values.

*/

#ifndef __JUCE_APPCONFIG_0NRD9LLGO__
#define __JUCE_APPCONFIG_0NRD9LLGO__

//==============================================================================
#define JUCE_MODULE_AVAILABLE_juce_audio_basics             1
#define JUCE_MODULE_AVAILABLE_juce_audio_devices            1
#define JUCE_MODULE_AVAILABLE_juce_audio_formats            1
#define JUCE_MODULE_AVAILABLE_juce_audio_plugin_client      1
#define JUCE_MODULE_AVAILABLE_juce_audio_processors         1
#define JUCE_MODULE_AVAILABLE_juce_audio_utils              1
#define JUCE_MODULE_AVAILABLE_juce_core                     1
#define JUCE_MODULE_AVAILABLE_juce_data_structures          1
#define JUCE_MODULE_AVAILABLE_juce_events                   1
#define JUCE_MODULE_AVAILABLE_juce_graphics                 1
#define JUCE_MODULE_AVAILABLE_juce_gui_basics               1

//==============================================================================
// juce_audio_devices flags:

#ifndef    JUCE_ASIO
 //#define JUCE_ASIO
#endif

#ifndef    JUCE_WASAPI
 //#define JUCE_WASAPI
#endif

#ifndef    JUCE_DIRECTSOUND
 //#define JUCE_DIRECTSOUND
#endif

#ifndef    JUCE_ALSA
 //#define JUCE_ALSA
#endif

#ifndef    JUCE_JACK
 //#define JUCE_JACK
#endif

#ifndef    JUCE_USE_CDREADER
 //#define JUCE_USE_CDREADER
#endif

#ifndef    JUCE_USE_CDBURNER
 //#define JUCE_USE_CDBURNER
#endif

//==============================================================================
// juce_audio_formats flags:

#ifndef    JUCE_USE_FLAC
 //#define JUCE_USE_FLAC
#endif

#ifndef    JUCE_USE_OGGVORBIS
 //#define JUCE_USE_OGGVORBIS
#endif

#ifndef    JUCE_USE_MP3AUDIOFORMAT
 //#define JUCE_USE_MP3AUDIOFORMAT
#endif

//==============================================================================
// juce_audio_processors flags:

#ifndef    JUCE_PLUGINHOST_VST
 #define   JUCE_PLUGINHOST_VST 0
#endif

#ifndef    JUCE_PLUGINHOST_AU
 #define   JUCE_PLUGINHOST_AU 0
#endif

//==============================================================================
// juce_core flags:

#ifndef    JUCE_FORCE_DEBUG
 //#define JUCE_FORCE_DEBUG
#endif

#ifndef    JUCE_LOG_ASSERTIONS
 //#define JUCE_LOG_ASSERTIONS
#endif

#ifndef    JUCE_CHECK_MEMORY_LEAKS
 //#define JUCE_CHECK_MEMORY_LEAKS
#endif

#ifndef    JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
 //#define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
#endif

//==============================================================================
// juce_graphics flags:

#ifndef    JUCE_USE_COREIMAGE_LOADER
 //#define JUCE_USE_COREIMAGE_LOADER
#endif

#ifndef    JUCE_USE_DIRECTWRITE
 //#define JUCE_USE_DIRECTWRITE
#endif

//==============================================================================
// juce_gui_basics flags:

#ifndef    JUCE_ENABLE_REPAINT_DEBUGGING
 //#define JUCE_ENABLE_REPAINT_DEBUGGING
#endif

#ifndef    JUCE_USE_XSHM
 //#define JUCE_USE_XSHM
#endif

#ifndef    JUCE_USE_XRENDER
 //#define JUCE_USE_XRENDER
#endif

#ifndef    JUCE_USE_XCURSOR
 //#define JUCE_USE_XCURSOR
#endif


//==============================================================================
// Audio plugin settings..

#define JucePlugin_Build_VST    1
#define JucePlugin_Build_AU     1
#define JucePlugin_Build_RTAS   0

#define JucePlugin_Name                 "Juce Demo Plugin"
#define JucePlugin_Desc                 "Juce Demo Plugin"
#define JucePlugin_Manufacturer         "Raw Material Software"
#define JucePlugin_ManufacturerCode     'RawM'
#define JucePlugin_PluginCode           'Jcdm'
#define JucePlugin_MaxNumInputChannels  2
#define JucePlugin_MaxNumOutputChannels 2
#define JucePlugin_PreferredChannelConfigurations   {1, 1}, {2, 2}
#define JucePlugin_IsSynth              0
#define JucePlugin_WantsMidiInput       1
#define JucePlugin_ProducesMidiOutput   1
#define JucePlugin_SilenceInProducesSilenceOut  0
#define JucePlugin_TailLengthSeconds    0
#define JucePlugin_EditorRequiresKeyboardFocus  1
#define JucePlugin_VersionCode          0x10000
#define JucePlugin_VersionString        "1.0.0"
#define JucePlugin_VSTUniqueID          JucePlugin_PluginCode
#define JucePlugin_VSTCategory          kPlugCategEffect
#define JucePlugin_AUMainType           kAudioUnitType_Effect
#define JucePlugin_AUSubType            JucePlugin_PluginCode
#define JucePlugin_AUExportPrefix       JuceDemoProjectAU
#define JucePlugin_AUExportPrefixQuoted "JuceDemoProjectAU"
#define JucePlugin_AUManufacturerCode   JucePlugin_ManufacturerCode
#define JucePlugin_CFBundleIdentifier   com.rawmaterialsoftware.JuceDemoPlugin
#define JucePlugin_AUCocoaViewClassName JuceDemoProjectAU_V1
#define JucePlugin_RTASCategory         ePlugInCategory_None
#define JucePlugin_RTASManufacturerCode JucePlugin_ManufacturerCode
#define JucePlugin_RTASProductId        JucePlugin_PluginCode

#endif  // __JUCE_APPCONFIG_0NRD9LLGO__
