/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__
#define __JUCE_TOOLTIPWINDOW_JUCEHEADER__

#include "../components/juce_Component.h"
#include "../mouse/juce_TooltipClient.h"


//==============================================================================
/**
    A window that displays a pop-up tooltip when the mouse hovers over another component.

    To enable tooltips in your app, just create a single instance of a TooltipWindow
    object.

    The TooltipWindow object will then stay invisible, waiting until the mouse
    hovers for the specified length of time - it will then see if it's currently
    over a component which implements the TooltipClient interface, and if so,
    it will make itself visible to show the tooltip in the appropriate place.

    @see TooltipClient, SettableTooltipClient
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

        @param parentComponent  if set to 0, the TooltipWindow will appear on the desktop,
                                otherwise the tooltip will be added to the given parent
                                component.
        @param millisecondsBeforeTipAppears     the time for which the mouse has to stay still
                                                before a tooltip will be shown

        @see TooltipClient, LookAndFeel::drawTooltip, LookAndFeel::getTooltipSize
    */
    explicit TooltipWindow (Component* parentComponent = nullptr,
                            int millisecondsBeforeTipAppears = 700);

    /** Destructor. */
    ~TooltipWindow();

    //==============================================================================
    /** Changes the time before the tip appears.
        This lets you change the value that was set in the constructor.
    */
    void setMillisecondsBeforeTipAppears (int newTimeMs = 700) noexcept;

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


private:
    //==============================================================================
    int millisecondsBeforeTipAppears;
    Point<int> lastMousePos;
    int mouseClicks, mouseWheelMoves;
    unsigned int lastCompChangeTime, lastHideTime;
    Component* lastComponentUnderMouse;
    bool changedCompsSinceShown;
    String tipShowing, lastTipUnderMouse;

    void paint (Graphics& g);
    void mouseEnter (const MouseEvent& e);
    void timerCallback();

    static String getTipFor (Component* c);
    void showFor (const String& tip);
    void hide();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TooltipWindow)
};


#endif   // __JUCE_TOOLTIPWINDOW_JUCEHEADER__
