/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/


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
