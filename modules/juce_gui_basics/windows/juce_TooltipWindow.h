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

namespace juce
{

//==============================================================================
/**
    A window that displays a pop-up tooltip when the mouse hovers over another component.

    To enable tooltips in your app, just create a single instance of a TooltipWindow
    object. Note that if you instantiate more than one instance of this class with the
    same parentComponent (even if both TooltipWindow's parentComponent is nil), you'll
    end up with multiple tooltips being shown! To avoid this use a SharedResourcePointer
    to instantiate the TooltipWindow only once.

    For audio plug-ins (which should not be opening native windows) it is better
    to add a TooltipWindow as a member variable to the editor and ensure that the
    editor is the parentComponent of your TooltipWindow. This will ensure that your
    TooltipWindow is scaled according to your editor and the DAWs scaling setting.

    The TooltipWindow object will then stay invisible, waiting until the mouse
    hovers for the specified length of time - it will then see if it's currently
    over a component which implements the TooltipClient interface, and if so,
    it will make itself visible to show the tooltip in the appropriate place.

    @see TooltipClient, SettableTooltipClient, SharedResourcePointer

    @tags{GUI}
*/
class JUCE_API  TooltipWindow  : public Component,
                                 private Timer
{
public:
    //==============================================================================
    /** Creates a tooltip window.

        Make sure your app only creates one instance of this class, otherwise you'll
        get multiple overlaid tooltips appearing. The window will initially be invisible
        and will make itself visible when it needs to display a tip.

        To change the style of tooltips, see the LookAndFeel class for its tooltip
        methods.

        @param parentComponent  if set to nullptr, the TooltipWindow will appear on the desktop,
                                otherwise the tooltip will be added to the given parent
                                component.
        @param millisecondsBeforeTipAppears     the time for which the mouse has to stay still
                                                before a tooltip will be shown

        @see TooltipClient, LookAndFeel::drawTooltip, LookAndFeel::getTooltipBounds
    */
    explicit TooltipWindow (Component* parentComponent = nullptr,
                            int millisecondsBeforeTipAppears = 700);

    /** Destructor. */
    ~TooltipWindow() override;

    //==============================================================================
    /** Changes the time before the tip appears.
        This lets you change the value that was set in the constructor.
    */
    void setMillisecondsBeforeTipAppears (int newTimeMs = 700) noexcept;

    /** Can be called to manually force a tip to be shown at a particular location. */
    void displayTip (Point<int> screenPosition, const String& text);

    /** Can be called to manually hide the tip if it's showing. */
    void hideTip();

    /** Asks a component for its tooltip.
        This can be overridden if you need custom lookup behaviour or to modify the strings.
    */
    virtual String getTipFor (Component&);

    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the tooltip.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId      = 0x1001b00,    /**< The colour to fill the background with. */
        textColourId            = 0x1001c00,    /**< The colour to use for the text. */
        outlineColourId         = 0x1001c10     /**< The colour to use to draw an outline around the tooltip. */
    };

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes to provide
        window drawing functionality.
    */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() = default;

        /** returns the bounds for a tooltip at the given screen coordinate, constrained within the given desktop area. */
        virtual Rectangle<int> getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea) = 0;
        virtual void drawTooltip (Graphics&, const String& text, int width, int height) = 0;

       #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
        // This method has been replaced by getTooltipBounds()
        virtual int getTooltipSize (const String&, int&, int&) { return 0; }
       #endif
    };

    //==============================================================================
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Point<float> lastMousePos;
    Component* lastComponentUnderMouse = nullptr;
    String tipShowing, lastTipUnderMouse;
    int millisecondsBeforeTipAppears;
    int mouseClicks = 0, mouseWheelMoves = 0;
    unsigned int lastCompChangeTime = 0, lastHideTime = 0;
    bool reentrant = false;

    void paint (Graphics&) override;
    void mouseEnter (const MouseEvent&) override;
    void timerCallback() override;
    void updatePosition (const String&, Point<int>, Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipWindow)
};

} // namespace juce
