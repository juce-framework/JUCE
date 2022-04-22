/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

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
#define JUCE_GUI_BASICS_INCLUDE_SCOPED_THREAD_DPI_AWARENESS_SETTER 1
#define JUCE_GRAPHICS_INCLUDE_COREGRAPHICS_HELPERS 1

#include "juce_audio_processors.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_core/containers/juce_Optional.h>

//==============================================================================
#if (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3) && (JUCE_LINUX || JUCE_BSD)
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

template <typename Callback>
void callOnMessageThread (Callback&& callback)
{
    if (MessageManager::getInstance()->existsAndIsLockedByCurrentThread())
    {
        callback();
        return;
    }

    WaitableEvent completionEvent;

    MessageManager::callAsync ([&callback, &completionEvent]
                               {
                                   callback();
                                   completionEvent.signal();
                               });

    completionEvent.wait();
}

#if JUCE_MAC

//==============================================================================
/*  This is an NSViewComponent which holds a long-lived NSView which acts
    as the parent view for plugin editors.

    Note that this component does not auto-resize depending on the bounds
    of the owned view. VST2 and VST3 plugins have dedicated interfaces to
    request that the editor bounds are updated. We can call `setSize` on this
    component from inside those dedicated callbacks.
*/
struct NSViewComponentWithParent  : public NSViewComponent,
                                    private AsyncUpdater
{
    enum class WantsNudge { no, yes };

    explicit NSViewComponentWithParent (WantsNudge shouldNudge)
        : wantsNudge (shouldNudge)
    {
        auto* view = [[getViewClass().createInstance() init] autorelease];
        object_setInstanceVariable (view, "owner", this);
        setView (view);
    }

    explicit NSViewComponentWithParent (AudioPluginInstance& instance)
        : NSViewComponentWithParent (getWantsNudge (instance)) {}

    ~NSViewComponentWithParent() override
    {
        if (auto* view = static_cast<NSView*> (getView()))
            object_setInstanceVariable (view, "owner", nullptr);

        cancelPendingUpdate();
    }

    JUCE_DECLARE_NON_COPYABLE (NSViewComponentWithParent)
    JUCE_DECLARE_NON_MOVEABLE (NSViewComponentWithParent)

private:
    WantsNudge wantsNudge = WantsNudge::no;

    static WantsNudge getWantsNudge (AudioPluginInstance& instance)
    {
        PluginDescription pd;
        instance.fillInPluginDescription (pd);
        return pd.manufacturerName == "FabFilter" ? WantsNudge::yes : WantsNudge::no;
    }

    void handleAsyncUpdate() override
    {
        if (auto* peer = getTopLevelComponent()->getPeer())
        {
            auto* view = static_cast<NSView*> (getView());
            const auto newArea = peer->getAreaCoveredBy (*this);
            [view setFrame: makeNSRect (newArea.withHeight (newArea.getHeight() + 1))];
            [view setFrame: makeNSRect (newArea)];
        }
    }

    struct FlippedNSView : public ObjCClass<NSView>
    {
        FlippedNSView()
            : ObjCClass ("JuceFlippedNSView_")
        {
            addIvar<NSViewComponentWithParent*> ("owner");

            addMethod (@selector (isFlipped),      isFlipped);
            addMethod (@selector (isOpaque),       isOpaque);
            addMethod (@selector (didAddSubview:), didAddSubview);

            registerClass();
        }

        static BOOL isFlipped (id, SEL) { return YES; }
        static BOOL isOpaque  (id, SEL) { return YES; }

        static void nudge (id self)
        {
            if (auto* owner = getIvar<NSViewComponentWithParent*> (self, "owner"))
                if (owner->wantsNudge == WantsNudge::yes)
                    owner->triggerAsyncUpdate();
        }

        static void viewDidUnhide (id self, SEL)               { nudge (self); }
        static void didAddSubview (id self, SEL, NSView*)      { nudge (self); }
        static void viewDidMoveToSuperview (id self, SEL)      { nudge (self); }
        static void viewDidMoveToWindow (id self, SEL)         { nudge (self); }
    };

    static FlippedNSView& getViewClass()
    {
        static FlippedNSView result;
        return result;
    }
};

#endif

} // namespace juce

#include "utilities/juce_FlagCache.h"
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
#include "utilities/juce_PluginHostType.cpp"
#include "utilities/juce_NativeScaleFactorNotifier.cpp"

#include "format_types/juce_LV2PluginFormat.cpp"

#if JUCE_UNIT_TESTS
 #include "format_types/juce_VST3PluginFormat_test.cpp"
 #include "format_types/juce_LV2PluginFormat_test.cpp"
#endif
