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
    A component for showing a message or other graphics inside a speech-bubble-shaped
    outline, pointing at a location on the screen.

    This is a base class that just draws and positions the bubble shape, but leaves
    the drawing of any content up to a subclass. See BubbleMessageComponent for a subclass
    that draws a text message.

    To use it, create your subclass, then either add it to a parent component or
    put it on the desktop with addToDesktop (0), use setPosition() to
    resize and position it, then make it visible.

    @see BubbleMessageComponent

    @tags{GUI}
*/
class JUCE_API  BubbleComponent  : public Component
{
protected:
    //==============================================================================
    /** Creates a BubbleComponent.

        Your subclass will need to implement the getContentSize() and paintContent()
        methods to draw the bubble's contents.
    */
    BubbleComponent();

public:
    /** Destructor. */
    ~BubbleComponent() override;

    //==============================================================================
    /** A list of permitted placements for the bubble, relative to the coordinates
        at which it should be pointing.

        @see setAllowedPlacement
    */
    enum BubblePlacement
    {
        above   = 1,
        below   = 2,
        left    = 4,
        right   = 8
    };

    /** Tells the bubble which positions it's allowed to put itself in, relative to the
        point at which it's pointing.

        By default when setPosition() is called, the bubble will place itself either
        above, below, left, or right of the target area. You can pass in a bitwise-'or' of
        the values in BubblePlacement to restrict this choice.

        E.g. if you only want your bubble to appear above or below the target area,
        use setAllowedPlacement (above | below);

        @see BubblePlacement
    */
    void setAllowedPlacement (int newPlacement);

    //==============================================================================
    /** Moves and resizes the bubble to point at a given component.

        This will resize the bubble to fit its content, then find a position for it
        so that it's next to, but doesn't overlap the given component.

        It'll put itself either above, below, or to the side of the component depending
        on where there's the most space, honouring any restrictions that were set
        with setAllowedPlacement().
    */
    void setPosition (Component* componentToPointTo,
                      int distanceFromTarget = 15, int arrowLength = 10);

    /** Moves and resizes the bubble to point at a given point.

        This will resize the bubble to fit its content, then position it
        so that the tip of the bubble points to the given coordinate. The coordinates
        are relative to either the bubble component's parent component if it has one, or
        they are screen coordinates if not.

        It'll put itself either above, below, or to the side of this point, depending
        on where there's the most space, honouring any restrictions that were set
        with setAllowedPlacement().
    */
    void setPosition (Point<int> arrowTipPosition, int arrowLength = 10);

    /** Moves and resizes the bubble to point at a given rectangle.

        This will resize the bubble to fit its content, then find a position for it
        so that it's next to, but doesn't overlap the given rectangle. The rectangle's
        coordinates are relative to either the bubble component's parent component
        if it has one, or they are screen coordinates if not.

        It'll put itself either above, below, or to the side of the component depending
        on where there's the most space, honouring any restrictions that were set
        with setAllowedPlacement().

        distanceFromTarget is the amount of space to leave between the bubble and the
        target rectangle, and arrowLength is the length of the arrow that it will draw.
    */
    void setPosition (Rectangle<int> rectangleToPointTo,
                      int distanceFromTarget = 15, int arrowLength = 10);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the bubble component.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId            = 0x1000af0, /**< A background colour to fill the bubble with. */
        outlineColourId               = 0x1000af1  /**< The colour to use for an outline around the bubble. */
    };


    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        virtual void drawBubble (Graphics&, BubbleComponent&,
                                 const Point<float>& positionOfTip,
                                 const Rectangle<float>& body) = 0;
    };

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

protected:
    //==============================================================================
    /** Subclasses should override this to return the size of the content they
        want to draw inside the bubble.
    */
    virtual void getContentSize (int& width, int& height) = 0;

    /** Subclasses should override this to draw their bubble's contents.

        The graphics object's clip region and the dimensions passed in here are
        set up to paint just the rectangle inside the bubble.
    */
    virtual void paintContent (Graphics& g, int width, int height) = 0;

private:
    Rectangle<int> content;
    Point<int> arrowTip;
    int allowablePlacements;
    DropShadowEffect shadow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BubbleComponent)
};

} // namespace juce
