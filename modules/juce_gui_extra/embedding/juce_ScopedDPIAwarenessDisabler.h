/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if (JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE) || DOXYGEN

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
*/
class JUCE_API  ScopedDPIAwarenessDisabler
{
public:
    ScopedDPIAwarenessDisabler();
    ~ScopedDPIAwarenessDisabler();

private:
    void* previousContext = nullptr;
};
#endif

} // namespace juce
