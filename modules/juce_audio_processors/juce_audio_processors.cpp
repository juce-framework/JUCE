/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

//==============================================================================
#if (JUCE_PLUGINHOST_VST || JUCE_PLUGINHOST_VST3) && (JUCE_LINUX || JUCE_BSD)
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wvariadic-macros")
 #include <X11/Xlib.h>
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
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

#if JUCE_PLUGINHOST_VST || (JUCE_PLUGINHOST_LADSPA && (JUCE_LINUX || JUCE_BSD))

static bool arrayContainsPlugin (const OwnedArray<PluginDescription>& list,
                                 const PluginDescription& desc)
{
    for (auto* p : list)
        if (p->isDuplicateOf (desc))
            return true;

    return false;
}

#endif

#if JUCE_MAC

//==============================================================================
/*  This is an NSViewComponent which holds a long-lived NSView which acts
    as the parent view for plugin editors.

    Note that this component does not auto-resize depending on the bounds
    of the owned view. VST2 and VST3 plugins have dedicated interfaces to
    request that the editor bounds are updated. We can call `setSize` on this
    component from inside those dedicated callbacks.
*/
struct NSViewComponentWithParent : public NSViewComponent,
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
            [view setFrame: makeCGRect (newArea.withHeight (newArea.getHeight() + 1))];
            [view setFrame: makeCGRect (newArea)];
        }
    }

    struct InnerNSView final : public ObjCClass<NSView>
    {
        InnerNSView()
            : ObjCClass ("JuceInnerNSView_")
        {
            addIvar<NSViewComponentWithParent*> ("owner");

            addMethod (@selector (isOpaque), [] (id, SEL) { return YES; });

            addMethod (@selector (didAddSubview:), [] (id self, SEL, NSView*)
            {
                if (auto* owner = getIvar<NSViewComponentWithParent*> (self, "owner"))
                    if (owner->wantsNudge == WantsNudge::yes)
                        owner->triggerAsyncUpdate();
            });

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (clipsToBounds), [] (id, SEL) { return YES; });
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }
    };

    static InnerNSView& getViewClass()
    {
        static InnerNSView result;
        return result;
    }
};

#endif

} // namespace juce

#include "utilities/juce_FlagCache.h"
#include "format/juce_AudioPluginFormat.cpp"
#include "format/juce_AudioPluginFormatManager.cpp"
#include "format_types/juce_LegacyAudioParameter.cpp"
#include "processors/juce_AudioProcessorParameter.cpp"
#include "processors/juce_AudioProcessor.cpp"
#include "processors/juce_AudioProcessorListener.cpp"
#include "processors/juce_AudioPluginInstance.cpp"
#include "processors/juce_AudioProcessorEditor.cpp"
#include "processors/juce_AudioProcessorGraph.cpp"
#include "processors/juce_GenericAudioProcessorEditor.cpp"
#include "processors/juce_PluginDescription.cpp"
#include "format_types/juce_ARACommon.cpp"
#include "format_types/juce_LADSPAPluginFormat.cpp"
#include "format_types/juce_VSTPluginFormat.cpp"
#include "format_types/juce_VST3PluginFormat.cpp"
#include "format_types/juce_AudioUnitPluginFormat.mm"
#include "format_types/juce_ARAHosting.cpp"
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
#include "utilities/juce_AAXClientExtensions.cpp"
#include "utilities/juce_VST2ClientExtensions.cpp"
#include "utilities/juce_VST3ClientExtensions.cpp"
#include "utilities/ARA/juce_ARA_utils.cpp"

#include "format_types/juce_LV2PluginFormat.cpp"

#if JUCE_UNIT_TESTS
 #if JUCE_PLUGINHOST_VST3
  #include "format_types/juce_VST3PluginFormat_test.cpp"
 #endif

 #if JUCE_PLUGINHOST_AU && (JUCE_MAC || JUCE_IOS)
  #include "format_types/juce_AudioUnitPluginFormat_test.cpp"
 #endif

 #if JUCE_PLUGINHOST_LV2 && (! (JUCE_ANDROID || JUCE_IOS))
  #include "format_types/juce_LV2PluginFormat_test.cpp"
 #endif
#endif
