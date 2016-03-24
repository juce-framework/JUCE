/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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

#include "juce_audio_processors.h"
#include "../juce_gui_extra/juce_gui_extra.h"

//==============================================================================
#if JUCE_MAC
 #if JUCE_SUPPORT_CARBON \
      && ((JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_AU) \
           || ! (defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6))
  #include <Carbon/Carbon.h>
 #endif
#endif

#if JUCE_PLUGINHOST_VST && JUCE_LINUX
 #include <X11/Xlib.h>
 #include <X11/Xutil.h>
 #undef KeyPress
#endif

#if ! JUCE_WINDOWS && ! JUCE_MAC
 #undef JUCE_PLUGINHOST_VST3
 #define JUCE_PLUGINHOST_VST3 0
#endif

//==============================================================================
namespace juce
{

static inline bool arrayContainsPlugin (const OwnedArray<PluginDescription>& list,
                                        const PluginDescription& desc)
{
    for (int i = list.size(); --i >= 0;)
        if (list.getUnchecked(i)->isDuplicateOf (desc))
            return true;

    return false;
}

#if JUCE_MAC
//==============================================================================
struct AutoResizingNSViewComponent  : public NSViewComponent,
                                      private AsyncUpdater
{
    AutoResizingNSViewComponent() : recursive (false) {}

    void childBoundsChanged (Component*) override
    {
        if (recursive)
        {
            triggerAsyncUpdate();
        }
        else
        {
            recursive = true;
            resizeToFitView();
            recursive = true;
        }
    }

    void handleAsyncUpdate() override               { resizeToFitView(); }

    bool recursive;
};

//==============================================================================
struct AutoResizingNSViewComponentWithParent  : public AutoResizingNSViewComponent,
                                                private Timer
{
    AutoResizingNSViewComponentWithParent()
    {
        NSView* v = [[NSView alloc] init];
        setView (v);
        [v release];

        startTimer (30);
    }

    NSView* getChildView() const
    {
        if (NSView* parent = (NSView*) getView())
            if ([[parent subviews] count] > 0)
                return [[parent subviews] objectAtIndex: 0];

        return nil;
    }

    void timerCallback() override
    {
        if (NSView* child = getChildView())
        {
            stopTimer();
            setView (child);
        }
    }
};
#endif

#if JUCE_CLANG
 #pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif

#include "format/juce_AudioPluginFormat.cpp"
#include "format/juce_AudioPluginFormatManager.cpp"
#include "processors/juce_AudioProcessor.cpp"
#include "processors/juce_AudioChannelSet.cpp"
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
#include "utilities/juce_AudioProcessorValueTreeState.cpp"
#include "utilities/juce_AudioProcessorParameters.cpp"

}
