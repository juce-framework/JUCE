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
    setOpaque (getLookAndFeel().isProgressBarOpaque (*this));
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

        if (auto* handler = getAccessibilityHandler())
            handler->notifyAccessibilityEvent (AccessibilityEvent::valueChanged);
    }
}

//==============================================================================
std::unique_ptr<AccessibilityHandler> ProgressBar::createAccessibilityHandler()
{
    class ProgressBarAccessibilityHandler  : public AccessibilityHandler
    {
    public:
        explicit ProgressBarAccessibilityHandler (ProgressBar& progressBarToWrap)
            : AccessibilityHandler (progressBarToWrap,
                                    AccessibilityRole::progressBar,
                                    AccessibilityActions{},
                                    AccessibilityHandler::Interfaces { std::make_unique<ValueInterface> (progressBarToWrap) }),
              progressBar (progressBarToWrap)
        {
        }

        String getHelp() const override   { return progressBar.getTooltip(); }

    private:
        class ValueInterface  : public AccessibilityRangedNumericValueInterface
        {
        public:
            explicit ValueInterface (ProgressBar& progressBarToWrap)
                : progressBar (progressBarToWrap)
            {
            }

            bool isReadOnly() const override                { return true; }
            void setValue (double) override                 { jassertfalse; }
            double getCurrentValue() const override         { return progressBar.progress; }
            AccessibleValueRange getRange() const override  { return { { 0.0, 1.0 }, 0.001 }; }

        private:
            ProgressBar& progressBar;

            //==============================================================================
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueInterface)
        };

        ProgressBar& progressBar;

        //==============================================================================
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProgressBarAccessibilityHandler)
    };

    return std::make_unique<ProgressBarAccessibilityHandler> (*this);
}

} // namespace juce
