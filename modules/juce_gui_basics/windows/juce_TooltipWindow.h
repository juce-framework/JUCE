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

    /** Can be called to manually force a tip to be shown at a particular location.

        The tip will be shown until hideTip() is called, or a dismissal mouse event
        occurs.

        @see hideTip
    */
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
    };

    //==============================================================================
    /** @internal */
    float getDesktopScaleFactor() const override;
    /** @internal */
    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override;

private:
    //==============================================================================
    Point<float> lastMousePos;
    SafePointer<Component> lastComponentUnderMouse;
    String tipShowing, lastTipUnderMouse, manuallyShownTip;
    int millisecondsBeforeTipAppears;
    unsigned int lastCompChangeTime = 0, lastHideTime = 0;
    bool reentrant = false, dismissalMouseEventOccurred = false;

    enum ShownManually { yes, no };
    void displayTipInternal (Point<int>, const String&, ShownManually);

    void paint (Graphics&) override;
    void mouseEnter (const MouseEvent&) override;
    void mouseDown (const MouseEvent&) override;
    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails&) override;
    void timerCallback() override;
    void updatePosition (const String&, Point<int>, Rectangle<int>);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipWindow)
};

} // namespace juce
