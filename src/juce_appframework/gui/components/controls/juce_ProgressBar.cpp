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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ProgressBar.h"
#include "../lookandfeel/juce_LookAndFeel.h"


//==============================================================================
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
            text << roundDoubleToInt (currentValue * 100.0) << T("%");
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

    if (currentValue != newProgress
         || newProgress < 0 || newProgress >= 1.0
         || currentMessage != displayedMessage)
    {
        if (currentValue < newProgress
             && newProgress >= 0 && newProgress < 1.0
             && currentValue >= 0 && currentValue < 1.0)
        {
            const uint32 now = Time::getMillisecondCounter();
            const int timeSinceLastCallback = (int) (now - lastCallbackTime);
            lastCallbackTime = now;

            newProgress = jmin (currentValue + 0.00018 * timeSinceLastCallback,
                                newProgress);
        }

        currentValue = newProgress;
        currentMessage = displayedMessage;
        repaint();
    }
}

END_JUCE_NAMESPACE
