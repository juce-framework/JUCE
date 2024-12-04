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

namespace juce::detail
{

struct WindowingHelpers
{
    WindowingHelpers() = delete;

    static Image createIconForFile (const File& file);

    #if JUCE_WINDOWS
     static bool isEmbeddedInForegroundProcess (Component* c);
     static bool isWindowOnCurrentVirtualDesktop (void*);
    #else
     static bool isEmbeddedInForegroundProcess (Component*) { return false; }
     static bool isWindowOnCurrentVirtualDesktop (void*) { return true; }
    #endif

    /*  Returns true if this process is in the foreground, or if the viewComponent
        is embedded into a window owned by the foreground process.
    */
    static bool isForegroundOrEmbeddedProcess (Component* viewComponent)
    {
        return Process::isForegroundProcess() || isEmbeddedInForegroundProcess (viewComponent);
    }

    template <typename Value>
    static BorderSize<int> roundToInt (BorderSize<Value> border)
    {
        return { ::juce::roundToInt (border.getTop()),
                 ::juce::roundToInt (border.getLeft()),
                 ::juce::roundToInt (border.getBottom()),
                 ::juce::roundToInt (border.getRight()) };
    }
};

} // namespace juce::detail
