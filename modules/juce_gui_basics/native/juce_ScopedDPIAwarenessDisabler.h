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
