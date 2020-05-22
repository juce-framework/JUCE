/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_AUDIO_PROCESSORS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "juce_audio_processors.h"
#include <juce_gui_extra/juce_gui_extra.h>

//==============================================================================
#if JUCE_MAC
 #if JUCE_SUPPORT_CARBON && (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_AU)
  #include <Carbon/Carbon.h>
  #include <juce_gui_extra/native/juce_mac_CarbonViewWrapperComponent.h>
 #endif
#endif

#if (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3) && JUCE_LINUX
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #include <sys/utsname.h>
 #undef KeyPress
#endif

#if ! JUCE_WINDOWS && ! JUCE_MAC && ! JUCE_LINUX
 #undef JUCE_PLUGINHOST_VST3
 #define JUCE_PLUGINHOST_VST3 0
#endif

#if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
 #include <AudioUnit/AudioUnit.h>
#endif

//==============================================================================
namespace juce
{

#if JUCE_PLUGINHOST_VST || (JUCE_PLUGINHOST_LADSPA && JUCE_LINUX)

static bool arrayContainsPlugin (const OwnedArray<PluginDescription>& list,
                                 const PluginDescription& desc)
{
    for (auto* p : list)
        if (p->isDuplicateOf (desc))
            return true;

    return false;
}

#endif

#if JUCE_MAC || JUCE_IOS

#if JUCE_IOS
 #define JUCE_IOS_MAC_VIEW  UIView
 using ViewComponentBaseClass = UIViewComponent;
#else
 #define JUCE_IOS_MAC_VIEW  NSView
 using ViewComponentBaseClass = NSViewComponent;
#endif

//==============================================================================
struct AutoResizingNSViewComponent  : public ViewComponentBaseClass,
                                      private AsyncUpdater
{
    void childBoundsChanged (Component*) override  { triggerAsyncUpdate(); }
    void handleAsyncUpdate() override              { resizeToFitView(); }
};

//==============================================================================
struct AutoResizingNSViewComponentWithParent  : public AutoResizingNSViewComponent,
                                                private Timer
{
    AutoResizingNSViewComponentWithParent()
    {
        JUCE_IOS_MAC_VIEW* v = [[JUCE_IOS_MAC_VIEW alloc] init];
        setView (v);
        [v release];

        startTimer (30);
    }

    JUCE_IOS_MAC_VIEW* getChildView() const
    {
        if (JUCE_IOS_MAC_VIEW* parent = (JUCE_IOS_MAC_VIEW*) getView())
            if ([[parent subviews] count] > 0)
                return [[parent subviews] objectAtIndex: 0];

        return nil;
    }

    void timerCallback() override
    {
        if (JUCE_IOS_MAC_VIEW* child = getChildView())
        {
            stopTimer();
            setView (child);
        }
    }
};
#endif

} // namespace juce

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations", "-Wcast-align")

#include "format/juce_AudioPluginFormat.cpp"
#include "format/juce_AudioPluginFormatManager.cpp"
#include "format_types/juce_LegacyAudioParameter.cpp"
#include "processors/juce_AudioProcessor.cpp"
#include "processors/juce_AudioPluginInstance.cpp"
#include "processors/juce_AudioProcessorEditor.cpp"
#include "processors/juce_AudioProcessorGraph.cpp"
#include "processors/juce_GenericAudioProcessorEditor.cpp"
#include "processors/juce_PluginDescription.cpp"
#include "format_types/juce_LADSPAPluginFormat.cpp"
#include "format_types/juce_VSTPluginFormat.cpp"
#include "format_types/juce_VST3PluginFormat.cpp"
#include "format_types/juce_AudioUnitPluginFormat.mm"
#include "scanning/juce_KnownPluginList.cpp"
#include "scanning/juce_PluginDirectoryScanner.cpp"
#include "scanning/juce_PluginListComponent.cpp"
#include "processors/juce_AudioProcessorParameterGroup.cpp"
#include "utilities/juce_AudioProcessorParameterWithID.cpp"
#include "utilities/juce_RangedAudioParameter.cpp"
#include "utilities/juce_AudioParameterFloat.cpp"
#include "utilities/juce_AudioParameterInt.cpp"
#include "utilities/juce_AudioParameterBool.cpp"
#include "utilities/juce_AudioParameterChoice.cpp"
#include "utilities/juce_ParameterAttachments.cpp"
#include "utilities/juce_AudioProcessorValueTreeState.cpp"
