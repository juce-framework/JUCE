/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_TOOLTIPWINDOW_JUCEHEADER__
#define __JUCE_TOOLTIPWINDOW_JUCEHEADER__

#include "../juce_Component.h"
#include "../../../events/juce_Timer.h"
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
    TooltipWindow (Component* parentComponent = 0,
                   const int millisecondsBeforeTipAppears = 700);

    /** Destructor. */
    ~TooltipWindow();

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
    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    const int millisecondsBeforeTipAppears;
    int mouseX, mouseY, mouseClicks;
    unsigned int lastMouseMoveTime, lastHideTime;
    Component* lastComponentUnderMouse;
    bool changedCompsSinceShown;
    String tip;

    void paint (Graphics& g);
    void mouseEnter (const MouseEvent& e);
    void timerCallback();

    void showFor (Component* const c);

    TooltipWindow (const TooltipWindow&);
    const TooltipWindow& operator= (const TooltipWindow&);
};


#endif   // __JUCE_TOOLTIPWINDOW_JUCEHEADER__
