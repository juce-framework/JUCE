/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A ComponentBoundsConstrainer that can be used to add a constant border onto another
    ComponentBoundsConstrainer.

    This is useful when trying to constrain the size of a resizable window or
    other component that wraps a constrained component, such as a plugin
    editor.

    @see ResizableCornerComponent, ResizableBorderComponent, ResizableWindow,
         ComponentBoundsConstrainer

    @tags{GUI}
*/
class JUCE_API  BorderedComponentBoundsConstrainer : public ComponentBoundsConstrainer
{
public:
    /** Default constructor. */
    BorderedComponentBoundsConstrainer() = default;

    /** Returns a pointer to another constrainer that will be used as the
        base for any resizing operations.
    */
    virtual ComponentBoundsConstrainer* getWrappedConstrainer() const = 0;

    /** Returns the border that should be applied to the constrained bounds. */
    virtual BorderSize<int> getAdditionalBorder() const = 0;

    /** @internal */
    void checkBounds (Rectangle<int>& bounds,
                      const Rectangle<int>& previousBounds,
                      const Rectangle<int>& limits,
                      bool isStretchingTop,
                      bool isStretchingLeft,
                      bool isStretchingBottom,
                      bool isStretchingRight) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BorderedComponentBoundsConstrainer)
};

} // namespace juce
