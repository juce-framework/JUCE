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

#pragma once

#if JUCE_MAC

#include <juce_audio_plugin_client/detail/juce_IncludeModuleHeaders.h>

namespace juce::detail
{

struct VSTWindowUtilities
{
    VSTWindowUtilities() = delete;

    static void* attachComponentToWindowRefVST (Component* comp,
                                                int desktopFlags,
                                                void* parentWindowOrView)
    {
        JUCE_AUTORELEASEPOOL
        {
            NSView* parentView = [(NSView*) parentWindowOrView retain];

            const auto defaultFlags = JucePlugin_EditorRequiresKeyboardFocus
                                    ? 0
                                    : ComponentPeer::windowIgnoresKeyPresses;
            comp->addToDesktop (desktopFlags | defaultFlags, parentView);

            // (this workaround is because Wavelab provides a zero-size parent view..)
            if (approximatelyEqual ([parentView frame].size.height, 0.0))
                [((NSView*) comp->getWindowHandle()) setFrameOrigin: NSZeroPoint];

            comp->setVisible (true);
            comp->toFront (false);

            [[parentView window] setAcceptsMouseMovedEvents: YES];
            return parentView;
        }
    }

    static void detachComponentFromWindowRefVST (Component* comp,
                                                void* window)
    {
        JUCE_AUTORELEASEPOOL
        {
            comp->removeFromDesktop();
            [(id) window release];
        }
    }

    static void setNativeHostWindowSizeVST (void* window,
                                            Component* component,
                                            int newWidth,
                                            int newHeight)
    {
        JUCE_AUTORELEASEPOOL
        {
            if (NSView* hostView = (NSView*) window)
            {
                const int dx = newWidth  - component->getWidth();
                const int dy = newHeight - component->getHeight();

                NSRect r = [hostView frame];
                r.size.width += dx;
                r.size.height += dy;
                r.origin.y -= dy;
                [hostView setFrame: r];
            }
        }
    }
};

} // namespace juce::detail

#endif
