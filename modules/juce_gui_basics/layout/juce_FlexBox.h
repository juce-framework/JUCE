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

/**
    Represents a FlexBox container, which contains and manages the layout of a set
    of FlexItem objects.

    To use this class, set its parameters appropriately (you can search online for
    more help on exactly how the FlexBox protocol works!), then add your sub-items
    to the items array, and call performLayout().

    @see FlexItem
*/
class JUCE_API  FlexBox
{
public:
    /** Possible values for the flexDirection property. */
    enum class Direction       { row, rowReverse, column, columnReverse };
    /** Possible values for the flexWrap property. */
    enum class Wrap            { noWrap, wrap, wrapReverse };
    /** Possible values for the alignContent property. */
    enum class AlignContent    { stretch, flexStart, flexEnd, center, spaceBetween, spaceAround };
    /** Possible values for the alignItems property. */
    enum class AlignItems      { stretch, flexStart, flexEnd, center };
    /** Possible values for the justifyContent property. */
    enum class JustifyContent  { flexStart, flexEnd, center, spaceBetween, spaceAround };

    //==============================================================================
    /** Creates an empty FlexBox container with default parameters. */
    FlexBox() noexcept;

    /** Creates an empty FlexBox container with these parameters. */
    FlexBox (Direction, Wrap, AlignContent, AlignItems, JustifyContent) noexcept;

    /** Creates an empty FlexBox container with the given content-justification mode. */
    FlexBox (JustifyContent) noexcept;

    /** Destructor. */
    ~FlexBox() noexcept;

    //==============================================================================
    /** Lays-out the box's items within the given rectangle. */
    void performLayout (Rectangle<float> targetArea);

    /** Lays-out the box's items within the given rectangle. */
    void performLayout (Rectangle<int> targetArea);

    //==============================================================================
    /** Specifies how flex items are placed in the flex container, and defines the
        direction of the main axis.
    */
    Direction flexDirection = Direction::row;

    /** Specifies whether items are forced into a single line or can be wrapped onto multiple lines.
        If wrapping is allowed, this property also controls the direction in which lines are stacked.
    */
    Wrap flexWrap = Wrap::noWrap;

    /** Specifies how a flex container's lines are placed within the flex container when
        there is extra space on the cross-axis.
        This property has no effect on single line layouts.
    */
    AlignContent alignContent = AlignContent::stretch;

    /** Specifies the alignment of flex items along the cross-axis of each line. */
    AlignItems alignItems = AlignItems::stretch;

    /** Defines how the container distributes space between and around items along the main-axis.
        The alignment is done after the lengths and auto margins are applied, so that if there is at
        least one flexible element, with flex-grow different from 0, it will have no effect as there
        won't be any available space.
    */
    JustifyContent justifyContent = JustifyContent::flexStart;

    /** The set of items to lay-out. */
    Array<FlexItem> items;

private:
    JUCE_LEAK_DETECTOR (FlexBox)
};

} // namespace juce
