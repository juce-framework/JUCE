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

#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__
#define __JUCE_PROGRESSBAR_JUCEHEADER__

#include "../juce_Component.h"
#include "../mouse/juce_TooltipClient.h"
#include "../../../events/juce_Timer.h"


//==============================================================================
/**
    A progress bar component.

    To use this, just create one and make it visible. It'll run its own timer
    to keep an eye on a variable that you give it, and will automatically
    redraw itself when the variable changes.

    For an easy way of running a background task with a dialog box showing its
    progress, see the ThreadWithProgressWindow class.

    @see ThreadWithProgressWindow
*/
class JUCE_API  ProgressBar  : public Component,
                               public SettableTooltipClient,
                               private Timer
{
public:
    //==============================================================================
    /** Creates a ProgressBar.

        @param progress     pass in a reference to a double that you're going to
                            update with your task's progress. The ProgressBar will
                            monitor the value of this variable and will redraw itself
                            when the value changes. The range is from 0 to 1.0. Obviously
                            you'd better be careful not to delete this variable while the
                            ProgressBar still exists!
    */
    ProgressBar (double& progress);

    /** Destructor. */
    ~ProgressBar();

    //==============================================================================
    /** Turns the percentage display on or off.

        By default this is on, and the progress bar will display a text string showing
        its current percentage.
    */
    void setPercentageDisplay (const bool shouldDisplayPercentage);


    //==============================================================================
    /** A set of colour IDs to use to change the colour of various aspects of the bar.

        These constants can be used either via the Component::setColour(), or LookAndFeel::setColour()
        methods.

        @see Component::setColour, Component::findColour, LookAndFeel::setColour, LookAndFeel::findColour
    */
    enum ColourIds
    {
        backgroundColourId              = 0x1001900,    /**< The background colour, behind the bar. */
        foregroundColourId              = 0x1001a00,    /**< The colour to use to draw the bar itself. LookAndFeel
                                                             classes will probably use variations on this colour. */
    };

    //==============================================================================
    juce_UseDebuggingNewOperator

protected:
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void lookAndFeelChanged();
    /** @internal */
    void visibilityChanged();
    /** @internal */
    void colourChanged();

private:
    double& progress;
    double currentValue;
    bool displayPercentage;

    void timerCallback();

    ProgressBar (const ProgressBar&);
    const ProgressBar& operator= (const ProgressBar&);
};


#endif   // __JUCE_PROGRESSBAR_JUCEHEADER__
