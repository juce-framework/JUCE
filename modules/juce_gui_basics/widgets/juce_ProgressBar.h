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

#ifndef JUCE_PROGRESSBAR_H_INCLUDED
#define JUCE_PROGRESSBAR_H_INCLUDED


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

    //==============================================================================
    /** This abstract base class is implemented by LookAndFeel classes. */
    struct JUCE_API  LookAndFeelMethods
    {
        virtual ~LookAndFeelMethods() {}

        /** Draws a progress bar.

            If the progress value is less than 0 or greater than 1.0, this should draw a spinning
            bar that fills the whole space (i.e. to say that the app is still busy but the progress
            isn't known). It can use the current time as a basis for playing an animation.

            (Used by progress bars in AlertWindow).
        */
        virtual void drawProgressBar (Graphics&, ProgressBar&, int width, int height,
                                      double progress, const String& textToShow) = 0;
    };

protected:
    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void lookAndFeelChanged() override;
    /** @internal */
    void visibilityChanged() override;
    /** @internal */
    void colourChanged() override;

private:
    double& progress;
    double currentValue;
    bool displayPercentage;
    String displayedMessage, currentMessage;
    uint32 lastCallbackTime;

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBar)
};


#endif   // JUCE_PROGRESSBAR_H_INCLUDED
