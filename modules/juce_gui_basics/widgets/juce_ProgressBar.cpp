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

ProgressBar::ProgressBar (double& progress_)
   : progress (progress_),
     displayPercentage (true),
     lastCallbackTime (0)
{
    currentValue = jlimit (0.0, 1.0, progress);
}

ProgressBar::~ProgressBar()
{
}

//==============================================================================
void ProgressBar::setPercentageDisplay (const bool shouldDisplayPercentage)
{
    displayPercentage = shouldDisplayPercentage;
    repaint();
}

void ProgressBar::setTextToDisplay (const String& text)
{
    displayPercentage = false;
    displayedMessage = text;
}

void ProgressBar::lookAndFeelChanged()
{
    setOpaque (findColour (backgroundColourId).isOpaque());
}

void ProgressBar::colourChanged()
{
    lookAndFeelChanged();
}

void ProgressBar::paint (Graphics& g)
{
    String text;

    if (displayPercentage)
    {
        if (currentValue >= 0 && currentValue <= 1.0)
            text << roundToInt (currentValue * 100.0) << '%';
    }
    else
    {
        text = displayedMessage;
    }

    getLookAndFeel().drawProgressBar (g, *this,
                                      getWidth(), getHeight(),
                                      currentValue, text);
}

void ProgressBar::visibilityChanged()
{
    if (isVisible())
        startTimer (30);
    else
        stopTimer();
}

void ProgressBar::timerCallback()
{
    double newProgress = progress;

    const uint32 now = Time::getMillisecondCounter();
    const int timeSinceLastCallback = (int) (now - lastCallbackTime);
    lastCallbackTime = now;

    if (currentValue != newProgress
         || newProgress < 0 || newProgress >= 1.0
         || currentMessage != displayedMessage)
    {
        if (currentValue < newProgress
             && newProgress >= 0 && newProgress < 1.0
             && currentValue >= 0 && currentValue < 1.0)
        {
            newProgress = jmin (currentValue + 0.0008 * timeSinceLastCallback,
                                newProgress);
        }

        currentValue = newProgress;
        currentMessage = displayedMessage;
        repaint();
    }
}
