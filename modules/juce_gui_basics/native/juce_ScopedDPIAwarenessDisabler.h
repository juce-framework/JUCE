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

namespace juce
{

//==============================================================================
/**
    A Windows-specific class that temporarily sets the DPI awareness context of
    the current thread to be DPI unaware and resets it to the previous context
    when it goes out of scope.

    If you create one of these before creating a top-level window, the window
    will be DPI unaware and bitmap stretched by the OS on a display with >100%
    scaling.

    You shouldn't use this unless you really know what you are doing and
    are dealing with native HWNDs.

    @tags{GUI}
*/
class JUCE_API  ScopedDPIAwarenessDisabler
{
public:
    ScopedDPIAwarenessDisabler();
    ~ScopedDPIAwarenessDisabler();

private:
    void* previousContext = nullptr;
};

} // namespace juce
