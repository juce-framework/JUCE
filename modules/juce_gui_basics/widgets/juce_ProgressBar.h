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

#ifndef __JUCE_PROGRESSBAR_JUCEHEADER__
#define __JUCE_PROGRESSBAR_JUCEHEADER__

#include "../components/juce_Component.h"
#include "../mouse/juce_TooltipClient.h"

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
    explicit ProgressBar (double& progress);

    /** Destructor. */
    ~ProgressBar();

    //==============================================================================
    /** Turns the percentage display on or off.

        By default this is on, and the progress bar will display a text string showing
        its current percentage.
    */
    void setPercentageDisplay (bool shouldDisplayPercentage);

    /** Gives the progress bar a string to display inside it.

        If you call this, it will turn off the percentage display.
        @see setPercentageDisplay
    */
    void setTextToDisplay (const String& text);


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

protected:
    //==============================================================================
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
    String displayedMessage, currentMessage;
    uint32 lastCallbackTime;

    void timerCallback();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBar)
};


#endif   // __JUCE_PROGRESSBAR_JUCEHEADER__
