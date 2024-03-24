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

ProgressBar::ProgressBar (double& progress_, std::optional<Style> style_)
   : progress { progress_ },
     style { style_ }
{
}

ProgressBar::ProgressBar (double& progress_)
   : progress { progress_ }
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

void ProgressBar::setStyle (std::optional<Style> newStyle)
{
    style = newStyle;
    repaint();
}

ProgressBar::Style ProgressBar::getResolvedStyle() const
{
    return style.value_or (getLookAndFeel().getDefaultProgressBarStyle (*this));
}

void ProgressBar::lookAndFeelChanged()
{
    setOpaque (getLookAndFeel().isProgressBarOpaque (*this));
}

void ProgressBar::colourChanged()
{
    lookAndFeelChanged();
    repaint();
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

    const auto w = getWidth();
    const auto h = getHeight();
    const auto v = currentValue;

    getLookAndFeel().drawProgressBar (g, *this, w, h, v, text);
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

    if (! approximatelyEqual (currentValue, newProgress)
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
    class ProgressBarAccessibilityHandler final : public AccessibilityHandler
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
        class ValueInterface final : public AccessibilityRangedNumericValueInterface
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
