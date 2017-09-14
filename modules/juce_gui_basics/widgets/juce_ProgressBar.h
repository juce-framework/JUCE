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

//==============================================================================
/**
    A progress bar component.

    To use this, just create one and make it visible. It'll run its own timer
    to keep an eye on a variable that you give it, and will automatically
    redraw itself when the variable changes.

    If using LookAndFeel_V4 a circular spinning progress bar will be drawn if
    the width and height of the ProgressBar are equal, otherwise the standard,
    linear ProgressBar will be drawn.

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
                            when the value changes. The range is from 0 to 1.0 and JUCE
                            LookAndFeel classes will draw a spinning animation for values
                            outside this range. Obviously you'd better be careful not to
                            delete this variable while the ProgressBar still exists!
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

        virtual bool isProgressBarOpaque (ProgressBar&) = 0;
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

} // namespace juce
