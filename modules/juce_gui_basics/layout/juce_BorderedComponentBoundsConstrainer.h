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
