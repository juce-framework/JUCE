/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_PLUGIN_CHARACTERISTICS_H__
#define __JUCE_PLUGIN_CHARACTERISTICS_H__


//==============================================================================
/*  All of the following settings need to be defined for your plugin.

    Go through each of these definitions and check that it's correctly
    set-up before trying to do a build.
*/

//==============================================================================
/*                              Generic settings                              */

/** The name of your plugin. (Try to keep this as short as possible)
*/
#define JucePlugin_Name                     "Juce Demo Plugin"

/** A longer decription of your plugin.
*/
#define JucePlugin_Desc                     "A Demo Plugin demonstrating Juce"

/** The name of your company. (Try to keep this as short as possible)
*/
#define JucePlugin_Manufacturer             "Raw Material Software"

/** A four-character code for your company.
    Use single quotes - this isn't a string!
*/
#define JucePlugin_ManufacturerCode         'RawM'

/** A unique four-character code for your plugin.
    Use single quotes - this isn't a string!

    Note that for AU compatibility, this must contain at least one
    upper-case letter.
*/
#define JucePlugin_PluginCode               'JcDm'

//==============================================================================
/** The maximum number of channels of audio input that the plugin can handle.

    The actual number of channels supplied may be less than this, depending on the host.
    For VSTs, you specify a maximum number of channels, for AUs and RTAS a set
    of channel configurations is specified in JucePlugin_PreferredChannelConfigurations
    and the host will choose one of these, but you should still set the max number of
    channels correctly.

    As soon as a plugin's prepareToPlay() method is called, you can find out the actual
    number of channels that will be used with the AudioFilterBase::getNumInputChannels()
    method.
*/
#define JucePlugin_MaxNumInputChannels              2

/** The maximum number of channels of audio output that the plugin can handle.

    The actual number of channels supplied may be less than this, depending on the host.
    For VSTs, you specify a maximum number of channels, for AUs and RTAS a set
    of channel configurations is specified in JucePlugin_PreferredChannelConfigurations
    and the host will choose one of these, but you should still set the max number of
    channels correctly.

    As soon as a plugin's prepareToPlay() method is called, you can find out the actual
    number of channels that will be used with the AudioFilterBase::getNumOutputChannels()
    method.
*/
#define JucePlugin_MaxNumOutputChannels             2

/** This allows the plugin to specify the configurations of input/output channels that
    they can support.

    AU and RTAS hosts will use this information, although VSTs only have a concept of
    a maximum number of channels.

    The list is a set of pairs of values in the form { numInputs, numOutputs }, and each
    pair indicates a valid configuration that the plugin can handle.

    So for example, {1, 1}, {2, 2} means that the plugin can be used in just two
    configurations: either with 1 input and 1 output, or with 2 inputs and 2 outputs. If
    you used this in Pro-Tools, the plugin could be placed on a mono or stereo track.
    If the list was just {1, 1}, then Pro-Tools would only allow it to be used as a mono
    plugin.

    As soon as a plugin's prepareToPlay() method is called, you can find out the actual
    number of channels that the host has connected to the plugin by using the
    AudioFilterBase::getNumOutputChannels() and AudioFilterBase::getNumInputChannels()
    methods.
*/
#define JucePlugin_PreferredChannelConfigurations   { 1, 1 }, { 2, 2 }

//==============================================================================
/** The plugin's internal latency, as a number of samples.
*/
#define JucePlugin_Latency                          0

/** Set this value to 1 if your plugin is a synth, or 0 if it isn't.
*/
#define JucePlugin_IsSynth                          1

/** Set this to 1 if your plugin needs to receive midi messages, or 0 if
    it doesn't.
*/
#define JucePlugin_WantsMidiInput                   1

/** Set this to 1 if your plugin wants to output midi messages, or 0 if
    it doesn't.
*/
#define JucePlugin_ProducesMidiOutput               1

/** If this is 1, it means that when the plugins input buffers are
    silent, it's output will be too.

    Some hosts may use this to avoid calling the plugin when no audio
    would be produced.
*/
#define JucePlugin_SilenceInProducesSilenceOut      0

/** If set to 1, this hints that the host should ignore any keys that are pressed
    when the plugin has keyboard focus. If 0, then the host should still execute
    any shortcut keys that are pressed, even if the plugin does have focus.

    Various hosts/platforms may deal with this differently, or ignore it.
*/
#define JucePlugin_EditorRequiresKeyboardFocus      1


//==============================================================================
/** A version number
*/
#define JucePlugin_VersionCode              0x00010100

#define JucePlugin_VersionString            "1.1"


//==============================================================================
/*                                VST settings                                */

/** For VSTs, if you're compiling against the V2.3 SDK, set this to zero. If
    you're using V2.4 or later, make sure it's set to 1.
*/
#define JUCE_USE_VSTSDK_2_4                 1

/** Defines a UID for your VST plugin.
    The default setting here is probably fine, unless you specifically need
    a custom value. It's passed to the setUniqueID() method of the plugin class.
*/
#define JucePlugin_VSTUniqueID              JucePlugin_PluginCode

/** Defines the type of plugin. For most pursposes, you don't need to change this
    setting.
*/
#if JucePlugin_IsSynth
  #define JucePlugin_VSTCategory            kPlugCategSynth
#else
  #define JucePlugin_VSTCategory            kPlugCategEffect
#endif

//==============================================================================
/*                              AudioUnit settings                            */

/** Defines the major type of plugin - see AUComponent.h for the available options.
    If it's an effect, you should use kAudioUnitType_Effect. For a synth, you'll
    need to use kAudioUnitType_MusicEffect or kAudioUnitType_MusicDevice.
*/
#if JucePlugin_IsSynth
  #define JucePlugin_AUMainType             kAudioUnitType_MusicEffect
#else
  #define JucePlugin_AUMainType             kAudioUnitType_Effect
#endif

/** A 4-character plugin ID code that should be unique.

    You can leave this using the generic value JucePlugin_PluginCode, or
    override it if necessary.

    Note that for AU, this must contain at least one upper-case letter.
*/
#define JucePlugin_AUSubType                JucePlugin_PluginCode

/** A prefix for the names of exported entry-point functions that the component exposes.

    It's very important that your plugin's .exp file contains two entries that correspond to
    this name. So for example if you set the prefix to "abc" then your exports
    file must contain:

    _abcEntry
    _abcViewEntry
*/
#define JucePlugin_AUExportPrefix           JuceDemoAU

/** This is the same as JucePlugin_AUExportPrefix, but in quotes
    (needed for the resource compiler...)
*/
#define JucePlugin_AUExportPrefixQuoted     "JuceDemoAU"

/** A 4-character manufacturer code - this is your company name.
    You can leave this using the generic value JucePlugin_ManufacturerCode, or
    override it if necessary.
*/
#define JucePlugin_AUManufacturerCode       JucePlugin_ManufacturerCode

/** If you define this value to be the same as the CFBundleIdentifier in your
    plugin's plist, it allows the plugin to work out its own path, which is
    needed if you want to use File::getSpecialLocation (currentExecutableFile)
*/
#define JucePlugin_CFBundleIdentifier       "com.rawmaterialsoftware.JuceDemo"

//==============================================================================
/*                                RTAS settings                               */

/** How to categorise this plugin.

    For a synth you probably want to set this to ePlugInCategory_SWGenerators.
    For an effect, you could choose one of:
      ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics,
      ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay,
      ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction,
      ePlugInCategory_Dither, ePlugInCategory_SoundField

    (All values are listed in FicPluginEnums.h)
*/
#if JucePlugin_IsSynth
  #define JucePlugin_RTASCategory           ePlugInCategory_SWGenerators
#else
  #define JucePlugin_RTASCategory           ePlugInCategory_None
#endif

/** A 4-character manufacturer code - this is your company name.
    You can leave this using the generic value JucePlugin_ManufacturerCode, or
    override it if necessary.
*/
#define JucePlugin_RTASManufacturerCode     JucePlugin_ManufacturerCode

/** A 4-character plugin ID code that should be unique.
    You can leave this using the generic value JucePlugin_PluginCode, or
    override it if necessary.
*/
#define JucePlugin_RTASProductId            JucePlugin_PluginCode


//==============================================================================

#endif
