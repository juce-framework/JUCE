/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

namespace juce
{

struct JUCE_API StackShadow
{
    /** Creates a default stack-shadow effect. */
    StackShadow() = default;

    /** Creates a stack-shadow object with the given parameters. */
    StackShadow (Colour shadowColour, Point<int> offset, int blur, int spread) noexcept;

    /** Renders a stack-shadow-based on the shape of a path. */
    void drawOuterShadowForPath (Graphics& g, const Path& path) const;

    /** Renders a stack-shadow-based inner-shadow on the shape of a path. */
    void drawInnerShadowForPath (Graphics& g, const Path& path) const;

    /** The colour with which to render the shadow. */
    Colour colour { 0x90000000 };

    /** The offset of the shadow. */
    Point<int> offset { 0, 0 };

    /** The ammount of blur of the shadow. */
    int blur { 4 };

    /** The spread of the shadow. */
    int spread { 0 };
};

} // namespace juce
