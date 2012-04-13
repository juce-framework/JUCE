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

class Slider::PopupDisplayComponent  : public BubbleComponent,
                                       public Timer
{
public:
    PopupDisplayComponent (Slider& owner_)
        : owner (owner_),
          font (15.0f, Font::bold)
    {
        setAlwaysOnTop (true);
    }

    void paintContent (Graphics& g, int w, int h)
    {
        g.setFont (font);
        g.setColour (findColour (TooltipWindow::textColourId, true));
        g.drawFittedText (text, 0, 0, w, h, Justification::centred, 1);
    }

    void getContentSize (int& w, int& h)
    {
        w = font.getStringWidth (text) + 18;
        h = (int) (font.getHeight() * 1.6f);
    }

    void updatePosition (const String& newText)
    {
        text = newText;
        BubbleComponent::setPosition (&owner);
        repaint();
    }

    void timerCallback()
    {
        owner.popupDisplay = nullptr;
    }

private:
    Slider& owner;
    Font font;
    String text;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupDisplayComponent);
};

//==============================================================================
Slider::Slider (const String& name)
  : Component (name),
    lastCurrentValue (0),
    lastValueMin (0),
    lastValueMax (0),
    minimum (0),
    maximum (10),
    interval (0),
    skewFactor (1.0),
    velocityModeSensitivity (1.0),
    velocityModeOffset (0.0),
    velocityModeThreshold (1),
    rotaryStart (float_Pi * 1.2f),
    rotaryEnd (float_Pi * 2.8f),
    numDecimalPlaces (7),
    sliderRegionStart (0),
    sliderRegionSize (1),
    sliderBeingDragged (-1),
    pixelsForFullDragExtent (250),
    style (LinearHorizontal),
    textBoxPos (TextBoxLeft),
    textBoxWidth (80),
    textBoxHeight (20),
    incDecButtonMode (incDecButtonsNotDraggable),
    editableText (true),
    doubleClickToValue (false),
    isVelocityBased (false),
    userKeyOverridesVelocity (true),
    rotaryStop (true),
    incDecButtonsSideBySide (false),
    sendChangeOnlyOnRelease (false),
    popupDisplayEnabled (false),
    menuEnabled (false),
    menuShown (false),
    scrollWheelEnabled (true),
    snapsToMousePos (true),
    parentForPopupDisplay (nullptr)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    Slider::lookAndFeelChanged();
    updateText();

    currentValue.addListener (this);
    valueMin.addListener (this);
    valueMax.addListener (this);
}

Slider::~Slider()
{
    currentValue.removeListener (this);
    valueMin.removeListener (this);
    valueMax.removeListener (this);
    popupDisplay = nullptr;
}


//==============================================================================
void Slider::handleAsyncUpdate()
{
    cancelPendingUpdate();

    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &SliderListener::sliderValueChanged, this);  // (can't use Slider::Listener due to idiotic VC2005 bug)
}

void Slider::sendDragStart()
{
    startedDragging();

    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &SliderListener::sliderDragStarted, this);
}

void Slider::sendDragEnd()
{
    stoppedDragging();

    sliderBeingDragged = -1;

    Component::BailOutChecker checker (this);
    listeners.callChecked (checker, &SliderListener::sliderDragEnded, this);
}

void Slider::addListener (SliderListener* const listener)
{
    listeners.add (listener);
}

void Slider::removeListener (SliderListener* const listener)
{
    listeners.remove (listener);
}

//==============================================================================
void Slider::setSliderStyle (const SliderStyle newStyle)
{
    if (style != newStyle)
    {
        style = newStyle;
        repaint();
        lookAndFeelChanged();
    }
}

void Slider::setRotaryParameters (const float startAngleRadians,
                                  const float endAngleRadians,
                                  const bool stopAtEnd)
{
    // make sure the values are sensible..
    jassert (rotaryStart >= 0 && rotaryEnd >= 0);
    jassert (rotaryStart < float_Pi * 4.0f && rotaryEnd < float_Pi * 4.0f);
    jassert (rotaryStart < rotaryEnd);

    rotaryStart = startAngleRadians;
    rotaryEnd = endAngleRadians;
    rotaryStop = stopAtEnd;
}

void Slider::setVelocityBasedMode (const bool velBased)
{
    isVelocityBased = velBased;
}

void Slider::setVelocityModeParameters (const double sensitivity,
                                        const int threshold,
                                        const double offset,
                                        const bool userCanPressKeyToSwapMode)
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    velocityModeSensitivity = sensitivity;
    velocityModeOffset = offset;
    velocityModeThreshold = threshold;
    userKeyOverridesVelocity = userCanPressKeyToSwapMode;
}

void Slider::setSkewFactor (const double factor)
{
    skewFactor = factor;
}

void Slider::setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
{
    if (maximum > minimum)
        skewFactor = log (0.5) / log ((sliderValueToShowAtMidPoint - minimum)
                                        / (maximum - minimum));
}

void Slider::setMouseDragSensitivity (const int distanceForFullScaleDrag)
{
    jassert (distanceForFullScaleDrag > 0);

    pixelsForFullDragExtent = distanceForFullScaleDrag;
}

void Slider::setIncDecButtonsMode (const IncDecButtonMode mode)
{
    if (incDecButtonMode != mode)
    {
        incDecButtonMode = mode;
        lookAndFeelChanged();
    }
}

void Slider::setTextBoxStyle (const TextEntryBoxPosition newPosition,
                              const bool isReadOnly,
                              const int textEntryBoxWidth,
                              const int textEntryBoxHeight)
{
    if (textBoxPos != newPosition
         || editableText != (! isReadOnly)
         || textBoxWidth != textEntryBoxWidth
         || textBoxHeight != textEntryBoxHeight)
    {
        textBoxPos = newPosition;
        editableText = ! isReadOnly;
        textBoxWidth = textEntryBoxWidth;
        textBoxHeight = textEntryBoxHeight;

        repaint();
        lookAndFeelChanged();
    }
}

void Slider::setTextBoxIsEditable (const bool shouldBeEditable)
{
    editableText = shouldBeEditable;

    if (valueBox != nullptr)
        valueBox->setEditable (shouldBeEditable && isEnabled());
}

void Slider::showTextBox()
{
    jassert (editableText); // this should probably be avoided in read-only sliders.

    if (valueBox != nullptr)
        valueBox->showEditor();
}

void Slider::hideTextBox (const bool discardCurrentEditorContents)
{
    if (valueBox != nullptr)
    {
        valueBox->hideEditor (discardCurrentEditorContents);

        if (discardCurrentEditorContents)
            updateText();
    }
}

void Slider::setChangeNotificationOnlyOnRelease (const bool onlyNotifyOnRelease)
{
    sendChangeOnlyOnRelease = onlyNotifyOnRelease;
}

void Slider::setSliderSnapsToMousePosition (const bool shouldSnapToMouse)
{
    snapsToMousePos = shouldSnapToMouse;
}

void Slider::setPopupDisplayEnabled (const bool enabled, Component* const parentComponentToUse)
{
    popupDisplayEnabled = enabled;
    parentForPopupDisplay = parentComponentToUse;
}

Component* Slider::getCurrentPopupDisplay() const noexcept
{
    return popupDisplay.get();
}

//==============================================================================
void Slider::colourChanged()
{
    lookAndFeelChanged();
}

void Slider::lookAndFeelChanged()
{
    LookAndFeel& lf = getLookAndFeel();

    if (textBoxPos != NoTextBox)
    {
        const String previousTextBoxContent (valueBox != nullptr ? valueBox->getText()
                                                                 : getTextFromValue (currentValue.getValue()));

        valueBox = nullptr;
        addAndMakeVisible (valueBox = getLookAndFeel().createSliderTextBox (*this));

        valueBox->setWantsKeyboardFocus (false);
        valueBox->setText (previousTextBoxContent, false);

        if (valueBox->isEditable() != editableText) // (avoid overriding the single/double click flags unless we have to)
            valueBox->setEditable (editableText && isEnabled());

        valueBox->addListener (this);

        if (style == LinearBar)
            valueBox->addMouseListener (this, false);
        else
            valueBox->setTooltip (getTooltip());
    }
    else
    {
        valueBox = nullptr;
    }

    if (style == IncDecButtons)
    {
        addAndMakeVisible (incButton = lf.createSliderButton (true));
        incButton->addListener (this);

        addAndMakeVisible (decButton = lf.createSliderButton (false));
        decButton->addListener (this);

        if (incDecButtonMode != incDecButtonsNotDraggable)
        {
            incButton->addMouseListener (this, false);
            decButton->addMouseListener (this, false);
        }
        else
        {
            incButton->setRepeatSpeed (300, 100, 20);
            incButton->addMouseListener (decButton, false);

            decButton->setRepeatSpeed (300, 100, 20);
            decButton->addMouseListener (incButton, false);
        }

        incButton->setTooltip (getTooltip());
        decButton->setTooltip (getTooltip());
    }
    else
    {
        incButton = nullptr;
        decButton = nullptr;
    }

    setComponentEffect (lf.getSliderEffect());

    resized();
    repaint();
}

//==============================================================================
void Slider::setRange (const double newMin,
                       const double newMax,
                       const double newInt)
{
    if (minimum != newMin
        || maximum != newMax
        || interval != newInt)
    {
        minimum = newMin;
        maximum = newMax;
        interval = newInt;

        // figure out the number of DPs needed to display all values at this
        // interval setting.
        numDecimalPlaces = 7;

        if (newInt != 0)
        {
            int v = abs ((int) (newInt * 10000000));

            while ((v % 10) == 0)
            {
                --numDecimalPlaces;
                v /= 10;
            }
        }

        // keep the current values inside the new range..
        if (style != TwoValueHorizontal && style != TwoValueVertical)
        {
            setValue (getValue(), false, false);
        }
        else
        {
            setMinValue (getMinValue(), false, false);
            setMaxValue (getMaxValue(), false, false);
        }

        updateText();
    }
}

void Slider::triggerChangeMessage (const bool synchronous)
{
    if (synchronous)
        handleAsyncUpdate();
    else
        triggerAsyncUpdate();

    valueChanged();
}

void Slider::valueChanged (Value& value)
{
    if (value.refersToSameSourceAs (currentValue))
    {
        if (style != TwoValueHorizontal && style != TwoValueVertical)
            setValue (currentValue.getValue(), false, false);
    }
    else if (value.refersToSameSourceAs (valueMin))
        setMinValue (valueMin.getValue(), false, false, true);
    else if (value.refersToSameSourceAs (valueMax))
        setMaxValue (valueMax.getValue(), false, false, true);
}

double Slider::getValue() const
{
    // for a two-value style slider, you should use the getMinValue() and getMaxValue()
    // methods to get the two values.
    jassert (style != TwoValueHorizontal && style != TwoValueVertical);

    return currentValue.getValue();
}

void Slider::setValue (double newValue,
                       const bool sendUpdateMessage,
                       const bool sendMessageSynchronously)
{
    // for a two-value style slider, you should use the setMinValue() and setMaxValue()
    // methods to set the two values.
    jassert (style != TwoValueHorizontal && style != TwoValueVertical);

    newValue = constrainedValue (newValue);

    if (style == ThreeValueHorizontal || style == ThreeValueVertical)
    {
        jassert ((double) valueMin.getValue() <= (double) valueMax.getValue());

        newValue = jlimit ((double) valueMin.getValue(),
                           (double) valueMax.getValue(),
                           newValue);
    }

    if (newValue != lastCurrentValue)
    {
        if (valueBox != nullptr)
            valueBox->hideEditor (true);

        lastCurrentValue = newValue;

        // (need to do this comparison because the Value will use equalsWithSameType to compare
        // the new and old values, so will generate unwanted change events if the type changes)
        if (currentValue != newValue)
            currentValue = newValue;

        updateText();
        repaint();

        if (popupDisplay != nullptr)
            popupDisplay->updatePosition (getTextFromValue (newValue));

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

double Slider::getMinValue() const
{
    // The minimum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    return valueMin.getValue();
}

double Slider::getMaxValue() const
{
    // The maximum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    return valueMax.getValue();
}

void Slider::setMinValue (double newValue, const bool sendUpdateMessage, const bool sendMessageSynchronously, const bool allowNudgingOfOtherValues)
{
    // The minimum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    newValue = constrainedValue (newValue);

    if (style == TwoValueHorizontal || style == TwoValueVertical)
    {
        if (allowNudgingOfOtherValues && newValue > (double) valueMax.getValue())
            setMaxValue (newValue, sendUpdateMessage, sendMessageSynchronously);

        newValue = jmin ((double) valueMax.getValue(), newValue);
    }
    else
    {
        if (allowNudgingOfOtherValues && newValue > lastCurrentValue)
            setValue (newValue, sendUpdateMessage, sendMessageSynchronously);

        newValue = jmin (lastCurrentValue, newValue);
    }

    if (lastValueMin != newValue)
    {
        lastValueMin = newValue;
        valueMin = newValue;
        repaint();

        if (popupDisplay != nullptr)
            popupDisplay->updatePosition (getTextFromValue (newValue));

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

void Slider::setMaxValue (double newValue, const bool sendUpdateMessage, const bool sendMessageSynchronously, const bool allowNudgingOfOtherValues)
{
    // The maximum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    newValue = constrainedValue (newValue);

    if (style == TwoValueHorizontal || style == TwoValueVertical)
    {
        if (allowNudgingOfOtherValues && newValue < (double) valueMin.getValue())
            setMinValue (newValue, sendUpdateMessage, sendMessageSynchronously);

        newValue = jmax ((double) valueMin.getValue(), newValue);
    }
    else
    {
        if (allowNudgingOfOtherValues && newValue < lastCurrentValue)
            setValue (newValue, sendUpdateMessage, sendMessageSynchronously);

        newValue = jmax (lastCurrentValue, newValue);
    }

    if (lastValueMax != newValue)
    {
        lastValueMax = newValue;
        valueMax = newValue;
        repaint();

        if (popupDisplay != nullptr)
            popupDisplay->updatePosition (getTextFromValue (valueMax.getValue()));

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

void Slider::setMinAndMaxValues (double newMinValue, double newMaxValue, bool sendUpdateMessage, bool sendMessageSynchronously)
{
    // The maximum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    if (newMaxValue < newMinValue)
        std::swap (newMaxValue, newMinValue);

    newMinValue = constrainedValue (newMinValue);
    newMaxValue = constrainedValue (newMaxValue);

    if (lastValueMax != newMaxValue || lastValueMin != newMinValue)
    {
        lastValueMax = newMaxValue;
        lastValueMin = newMinValue;
        valueMin = newMinValue;
        valueMax = newMaxValue;
        repaint();

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

void Slider::setDoubleClickReturnValue (const bool isDoubleClickEnabled,
                                        const double valueToSetOnDoubleClick)
{
    doubleClickToValue = isDoubleClickEnabled;
    doubleClickReturnValue = valueToSetOnDoubleClick;
}

double Slider::getDoubleClickReturnValue (bool& isEnabled_) const
{
    isEnabled_ = doubleClickToValue;
    return doubleClickReturnValue;
}

void Slider::updateText()
{
    if (valueBox != nullptr)
        valueBox->setText (getTextFromValue (currentValue.getValue()), false);
}

void Slider::setTextValueSuffix (const String& suffix)
{
    if (textSuffix != suffix)
    {
        textSuffix = suffix;
        updateText();
    }
}

String Slider::getTextValueSuffix() const
{
    return textSuffix;
}

String Slider::getTextFromValue (double v)
{
    if (getNumDecimalPlacesToDisplay() > 0)
        return String (v, getNumDecimalPlacesToDisplay()) + getTextValueSuffix();
    else
        return String (roundToInt (v)) + getTextValueSuffix();
}

double Slider::getValueFromText (const String& text)
{
    String t (text.trimStart());

    if (t.endsWith (textSuffix))
        t = t.substring (0, t.length() - textSuffix.length());

    while (t.startsWithChar ('+'))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly ("0123456789.,-")
            .getDoubleValue();
}

double Slider::proportionOfLengthToValue (double proportion)
{
    if (skewFactor != 1.0 && proportion > 0.0)
        proportion = exp (log (proportion) / skewFactor);

    return minimum + (maximum - minimum) * proportion;
}

double Slider::valueToProportionOfLength (double value)
{
    const double n = (value - minimum) / (maximum - minimum);

    return skewFactor == 1.0 ? n : pow (n, skewFactor);
}

double Slider::snapValue (double attemptedValue, const bool)
{
    return attemptedValue;
}

//==============================================================================
void Slider::startedDragging()
{
}

void Slider::stoppedDragging()
{
}

void Slider::valueChanged()
{
}

//==============================================================================
void Slider::enablementChanged()
{
    repaint();
}

void Slider::setPopupMenuEnabled (const bool menuEnabled_)
{
    menuEnabled = menuEnabled_;
}

void Slider::setScrollWheelEnabled (const bool enabled)
{
    scrollWheelEnabled = enabled;
}

//==============================================================================
void Slider::labelTextChanged (Label* label)
{
    const double newValue = snapValue (getValueFromText (label->getText()), false);

    if (newValue != (double) currentValue.getValue())
    {
        sendDragStart();
        setValue (newValue, true, true);
        sendDragEnd();
    }

    updateText(); // force a clean-up of the text, needed in case setValue() hasn't done this.
}

void Slider::buttonClicked (Button* button)
{
    if (style == IncDecButtons)
    {
        sendDragStart();

        if (button == incButton)
            setValue (snapValue (getValue() + interval, false), true, true);
        else if (button == decButton)
            setValue (snapValue (getValue() - interval, false), true, true);

        sendDragEnd();
    }
}

//==============================================================================
double Slider::constrainedValue (double value) const
{
    if (interval > 0)
        value = minimum + interval * std::floor ((value - minimum) / interval + 0.5);

    if (value <= minimum || maximum <= minimum)
        value = minimum;
    else if (value >= maximum)
        value = maximum;

    return value;
}

float Slider::getLinearSliderPos (const double value)
{
    double sliderPosProportional;

    if (maximum > minimum)
    {
        if (value < minimum)
        {
            sliderPosProportional = 0.0;
        }
        else if (value > maximum)
        {
            sliderPosProportional = 1.0;
        }
        else
        {
            sliderPosProportional = valueToProportionOfLength (value);
            jassert (sliderPosProportional >= 0 && sliderPosProportional <= 1.0);
        }
    }
    else
    {
        sliderPosProportional = 0.5;
    }

    if (isVertical() || style == IncDecButtons)
        sliderPosProportional = 1.0 - sliderPosProportional;

    return (float) (sliderRegionStart + sliderPosProportional * sliderRegionSize);
}

bool Slider::isHorizontal() const
{
    return style == LinearHorizontal
        || style == LinearBar
        || style == TwoValueHorizontal
        || style == ThreeValueHorizontal;
}

bool Slider::isVertical() const
{
    return style == LinearVertical
        || style == TwoValueVertical
        || style == ThreeValueVertical;
}

bool Slider::incDecDragDirectionIsHorizontal() const
{
    return incDecButtonMode == incDecButtonsDraggable_Horizontal
            || (incDecButtonMode == incDecButtonsDraggable_AutoDirection && incDecButtonsSideBySide);
}

float Slider::getPositionOfValue (const double value)
{
    if (isHorizontal() || isVertical())
    {
        return getLinearSliderPos (value);
    }
    else
    {
        jassertfalse; // not a valid call on a slider that doesn't work linearly!
        return 0.0f;
    }
}

//==============================================================================
void Slider::paint (Graphics& g)
{
    if (style != IncDecButtons)
    {
        if (style == Rotary || style == RotaryHorizontalDrag || style == RotaryVerticalDrag)
        {
            const float sliderPos = (float) valueToProportionOfLength (lastCurrentValue);
            jassert (sliderPos >= 0 && sliderPos <= 1.0f);

            getLookAndFeel().drawRotarySlider (g,
                                               sliderRect.getX(),
                                               sliderRect.getY(),
                                               sliderRect.getWidth(),
                                               sliderRect.getHeight(),
                                               sliderPos,
                                               rotaryStart, rotaryEnd,
                                               *this);
        }
        else
        {
            getLookAndFeel().drawLinearSlider (g,
                                               sliderRect.getX(),
                                               sliderRect.getY(),
                                               sliderRect.getWidth(),
                                               sliderRect.getHeight(),
                                               getLinearSliderPos (lastCurrentValue),
                                               getLinearSliderPos (lastValueMin),
                                               getLinearSliderPos (lastValueMax),
                                               style,
                                               *this);
        }

        if (style == LinearBar && valueBox == nullptr)
        {
            g.setColour (findColour (Slider::textBoxOutlineColourId));
            g.drawRect (0, 0, getWidth(), getHeight(), 1);
        }
    }
}

void Slider::resized()
{
    int minXSpace = 0;
    int minYSpace = 0;

    if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
        minXSpace = 30;
    else
        minYSpace = 15;

    const int tbw = jmax (0, jmin (textBoxWidth, getWidth() - minXSpace));
    const int tbh = jmax (0, jmin (textBoxHeight, getHeight() - minYSpace));

    if (style == LinearBar)
    {
        if (valueBox != nullptr)
            valueBox->setBounds (getLocalBounds());
    }
    else
    {
        if (textBoxPos == NoTextBox)
        {
            sliderRect = getLocalBounds();
        }
        else if (textBoxPos == TextBoxLeft)
        {
            valueBox->setBounds (0, (getHeight() - tbh) / 2, tbw, tbh);
            sliderRect.setBounds (tbw, 0, getWidth() - tbw, getHeight());
        }
        else if (textBoxPos == TextBoxRight)
        {
            valueBox->setBounds (getWidth() - tbw, (getHeight() - tbh) / 2, tbw, tbh);
            sliderRect.setBounds (0, 0, getWidth() - tbw, getHeight());
        }
        else if (textBoxPos == TextBoxAbove)
        {
            valueBox->setBounds ((getWidth() - tbw) / 2, 0, tbw, tbh);
            sliderRect.setBounds (0, tbh, getWidth(), getHeight() - tbh);
        }
        else if (textBoxPos == TextBoxBelow)
        {
            valueBox->setBounds ((getWidth() - tbw) / 2, getHeight() - tbh, tbw, tbh);
            sliderRect.setBounds (0, 0, getWidth(), getHeight() - tbh);
        }
    }

    const int indent = getLookAndFeel().getSliderThumbRadius (*this);

    if (style == LinearBar)
    {
        const int barIndent = 1;
        sliderRegionStart = barIndent;
        sliderRegionSize = getWidth() - barIndent * 2;

        sliderRect.setBounds (sliderRegionStart, barIndent,
                              sliderRegionSize, getHeight() - barIndent * 2);
    }
    else if (isHorizontal())
    {
        sliderRegionStart = sliderRect.getX() + indent;
        sliderRegionSize = jmax (1, sliderRect.getWidth() - indent * 2);

        sliderRect.setBounds (sliderRegionStart, sliderRect.getY(),
                              sliderRegionSize, sliderRect.getHeight());
    }
    else if (isVertical())
    {
        sliderRegionStart = sliderRect.getY() + indent;
        sliderRegionSize = jmax (1, sliderRect.getHeight() - indent * 2);

        sliderRect.setBounds (sliderRect.getX(), sliderRegionStart,
                              sliderRect.getWidth(), sliderRegionSize);
    }
    else
    {
        sliderRegionStart = 0;
        sliderRegionSize = 100;
    }

    if (style == IncDecButtons)
    {
        Rectangle<int> buttonRect (sliderRect);

        if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
            buttonRect.expand (-2, 0);
        else
            buttonRect.expand (0, -2);

        incDecButtonsSideBySide = buttonRect.getWidth() > buttonRect.getHeight();

        if (incDecButtonsSideBySide)
        {
            decButton->setBounds (buttonRect.removeFromLeft (buttonRect.getWidth() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnRight);
            incButton->setConnectedEdges (Button::ConnectedOnLeft);
        }
        else
        {
            decButton->setBounds (buttonRect.removeFromBottom (buttonRect.getHeight() / 2));
            decButton->setConnectedEdges (Button::ConnectedOnTop);
            incButton->setConnectedEdges (Button::ConnectedOnBottom);
        }

        incButton->setBounds (buttonRect);
    }
}

void Slider::focusOfChildComponentChanged (FocusChangeType)
{
    repaint();
}

namespace SliderHelpers
{
    static double smallestAngleBetween (double a1, double a2) noexcept
    {
        return jmin (std::abs (a1 - a2),
                     std::abs (a1 + double_Pi * 2.0 - a2),
                     std::abs (a2 + double_Pi * 2.0 - a1));
    }

    static void sliderMenuCallback (int result, Slider* slider)
    {
        if (slider != nullptr)
        {
            switch (result)
            {
                case 1: slider->setVelocityBasedMode (! slider->getVelocityBasedMode()); break;
                case 2: slider->setSliderStyle (Slider::Rotary); break;
                case 3: slider->setSliderStyle (Slider::RotaryHorizontalDrag); break;
                case 4: slider->setSliderStyle (Slider::RotaryVerticalDrag); break;
                default: break;
            }
        }
    }
}

void Slider::showPopupMenu()
{
    menuShown = true;

    PopupMenu m;
    m.setLookAndFeel (&getLookAndFeel());
    m.addItem (1, TRANS ("velocity-sensitive mode"), true, isVelocityBased);
    m.addSeparator();

    if (style == Rotary || style == RotaryHorizontalDrag || style == RotaryVerticalDrag)
    {
        PopupMenu rotaryMenu;
        rotaryMenu.addItem (2, TRANS ("use circular dragging"), true, style == Rotary);
        rotaryMenu.addItem (3, TRANS ("use left-right dragging"), true, style == RotaryHorizontalDrag);
        rotaryMenu.addItem (4, TRANS ("use up-down dragging"), true, style == RotaryVerticalDrag);

        m.addSubMenu (TRANS ("rotary mode"), rotaryMenu);
    }

    m.showMenuAsync (PopupMenu::Options(),
                     ModalCallbackFunction::forComponent (SliderHelpers::sliderMenuCallback, this));
}

int Slider::getThumbIndexAt (const MouseEvent& e)
{
    const bool isTwoValue   = (style == TwoValueHorizontal   || style == TwoValueVertical);
    const bool isThreeValue = (style == ThreeValueHorizontal || style == ThreeValueVertical);

    if (isTwoValue || isThreeValue)
    {
        const float mousePos = (float) (isVertical() ? e.y : e.x);

        const float normalPosDistance = std::abs (getLinearSliderPos (currentValue.getValue()) - mousePos);
        const float minPosDistance    = std::abs (getLinearSliderPos (valueMin.getValue()) - 0.1f - mousePos);
        const float maxPosDistance    = std::abs (getLinearSliderPos (valueMax.getValue()) + 0.1f - mousePos);

        if (isTwoValue)
            return maxPosDistance <= minPosDistance ? 2 : 1;

        if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
            return 1;
        else if (normalPosDistance >= maxPosDistance)
            return 2;
    }

    return 0;
}

void Slider::mouseDown (const MouseEvent& e)
{
    mouseWasHidden = false;
    incDecDragged = false;
    mouseDragStartPos = mousePosWhenLastDragged = e.getPosition();

    if (isEnabled())
    {
        if (e.mods.isPopupMenu() && menuEnabled)
        {
            showPopupMenu();
        }
        else if (maximum > minimum)
        {
            menuShown = false;

            if (valueBox != nullptr)
                valueBox->hideEditor (true);

            sliderBeingDragged = getThumbIndexAt (e);

            minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();

            lastAngle = rotaryStart + (rotaryEnd - rotaryStart)
                                        * valueToProportionOfLength (currentValue.getValue());

            valueWhenLastDragged = (sliderBeingDragged == 2 ? valueMax
                                                            : (sliderBeingDragged == 1 ? valueMin
                                                                                       : currentValue)).getValue();
            valueOnMouseDown = valueWhenLastDragged;

            if (popupDisplayEnabled)
            {
                PopupDisplayComponent* const popup = new PopupDisplayComponent (*this);
                popupDisplay = popup;

                if (parentForPopupDisplay != nullptr)
                    parentForPopupDisplay->addChildComponent (popup);
                else
                    popup->addToDesktop (0);

                popup->setVisible (true);
            }

            sendDragStart();
            mouseDrag (e);
        }
    }
}

void Slider::mouseUp (const MouseEvent&)
{
    if (isEnabled()
         && (! menuShown)
         && (maximum > minimum)
         && (style != IncDecButtons || incDecDragged))
    {
        restoreMouseIfHidden();

        if (sendChangeOnlyOnRelease && valueOnMouseDown != (double) currentValue.getValue())
            triggerChangeMessage (false);

        sendDragEnd();
        popupDisplay = nullptr;

        if (style == IncDecButtons)
        {
            incButton->setState (Button::buttonNormal);
            decButton->setState (Button::buttonNormal);
        }
    }
    else if (popupDisplay != nullptr)
    {
        popupDisplay->startTimer (2000);
    }
}

void Slider::restoreMouseIfHidden()
{
    if (mouseWasHidden)
    {
        mouseWasHidden = false;

        for (int i = Desktop::getInstance().getNumMouseSources(); --i >= 0;)
            Desktop::getInstance().getMouseSource(i)->enableUnboundedMouseMovement (false);

        const double pos = sliderBeingDragged == 2 ? getMaxValue()
                                                   : (sliderBeingDragged == 1 ? getMinValue()
                                                                              : (double) currentValue.getValue());
        Point<int> mousePos;

        if (style == RotaryHorizontalDrag || style == RotaryVerticalDrag)
        {
            mousePos = Desktop::getLastMouseDownPosition();

            if (style == RotaryHorizontalDrag)
            {
                const double posDiff = valueToProportionOfLength (pos) - valueToProportionOfLength (valueOnMouseDown);
                mousePos += Point<int> (roundToInt (pixelsForFullDragExtent * posDiff), 0);
            }
            else
            {
                const double posDiff = valueToProportionOfLength (valueOnMouseDown) - valueToProportionOfLength (pos);
                mousePos += Point<int> (0, roundToInt (pixelsForFullDragExtent * posDiff));
            }
        }
        else
        {
            const int pixelPos = (int) getLinearSliderPos (pos);

            mousePos = localPointToGlobal (Point<int> (isHorizontal() ? pixelPos : (getWidth() / 2),
                                                       isVertical()   ? pixelPos : (getHeight() / 2)));
        }

        Desktop::setMousePosition (mousePos);
    }
}

void Slider::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (isEnabled()
         && style != IncDecButtons
         && style != Rotary
         && isVelocityBased == modifiers.isAnyModifierKeyDown())
    {
        restoreMouseIfHidden();
    }
}

void Slider::handleRotaryDrag (const MouseEvent& e)
{
    const int dx = e.x - sliderRect.getCentreX();
    const int dy = e.y - sliderRect.getCentreY();

    if (dx * dx + dy * dy > 25)
    {
        double angle = std::atan2 ((double) dx, (double) -dy);
        while (angle < 0.0)
            angle += double_Pi * 2.0;

        if (rotaryStop && ! e.mouseWasClicked())
        {
            if (std::abs (angle - lastAngle) > double_Pi)
            {
                if (angle >= lastAngle)
                    angle -= double_Pi * 2.0;
                else
                    angle += double_Pi * 2.0;
            }

            if (angle >= lastAngle)
                angle = jmin (angle, (double) jmax (rotaryStart, rotaryEnd));
            else
                angle = jmax (angle, (double) jmin (rotaryStart, rotaryEnd));
        }
        else
        {
            while (angle < rotaryStart)
                angle += double_Pi * 2.0;

            if (angle > rotaryEnd)
            {
                if (SliderHelpers::smallestAngleBetween (angle, rotaryStart)
                     <= SliderHelpers::smallestAngleBetween (angle, rotaryEnd))
                    angle = rotaryStart;
                else
                    angle = rotaryEnd;
            }
        }

        const double proportion = (angle - rotaryStart) / (rotaryEnd - rotaryStart);
        valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, proportion));
        lastAngle = angle;
    }
}

void Slider::handleAbsoluteDrag (const MouseEvent& e)
{
    const int mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.x : e.y;

    double scaledMousePos = (mousePos - sliderRegionStart) / (double) sliderRegionSize;

    if (style == RotaryHorizontalDrag
        || style == RotaryVerticalDrag
        || style == IncDecButtons
        || ((style == LinearHorizontal || style == LinearVertical || style == LinearBar)
            && ! snapsToMousePos))
    {
        const int mouseDiff = (style == RotaryHorizontalDrag
                                 || style == LinearHorizontal
                                 || style == LinearBar
                                 || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                ? e.x - mouseDragStartPos.x
                                : mouseDragStartPos.y - e.y;

        double newPos = valueToProportionOfLength (valueOnMouseDown)
                           + mouseDiff * (1.0 / pixelsForFullDragExtent);

        valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, newPos));

        if (style == IncDecButtons)
        {
            incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
            decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
        }
    }
    else
    {
        if (isVertical())
            scaledMousePos = 1.0 - scaledMousePos;

        valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, scaledMousePos));
    }
}

void Slider::handleVelocityDrag (const MouseEvent& e)
{
    const int mouseDiff = (isHorizontal() || style == RotaryHorizontalDrag
                             || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                            ? e.x - mousePosWhenLastDragged.x
                            : e.y - mousePosWhenLastDragged.y;

    const double maxSpeed = jmax (200, sliderRegionSize);
    double speed = jlimit (0.0, maxSpeed, (double) abs (mouseDiff));

    if (speed != 0)
    {
        speed = 0.2 * velocityModeSensitivity
                  * (1.0 + std::sin (double_Pi * (1.5 + jmin (0.5, velocityModeOffset
                                                                + jmax (0.0, (double) (speed - velocityModeThreshold))
                                                                    / maxSpeed))));

        if (mouseDiff < 0)
            speed = -speed;

        if (isVertical() || style == RotaryVerticalDrag
             || (style == IncDecButtons && ! incDecDragDirectionIsHorizontal()))
            speed = -speed;

        const double currentPos = valueToProportionOfLength (valueWhenLastDragged);

        valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + speed));

        e.source.enableUnboundedMouseMovement (true, false);
        mouseWasHidden = true;
    }
}

void Slider::mouseDrag (const MouseEvent& e)
{
    if (isEnabled()
         && (! menuShown)
         && (maximum > minimum)
         && ! (style == LinearBar && e.mouseWasClicked() && valueBox != nullptr && valueBox->isEditable()))
    {
        if (style == Rotary)
        {
            handleRotaryDrag (e);
        }
        else
        {
            if (style == IncDecButtons && ! incDecDragged)
            {
                if (e.getDistanceFromDragStart() < 10 || e.mouseWasClicked())
                    return;

                incDecDragged = true;
                mouseDragStartPos = e.getPosition();
            }

            if (isVelocityBased == (userKeyOverridesVelocity && e.mods.testFlags (ModifierKeys::ctrlModifier
                                                                                    | ModifierKeys::commandModifier
                                                                                    | ModifierKeys::altModifier))
                 || (maximum - minimum) / sliderRegionSize < interval)
                handleAbsoluteDrag (e);
            else
                handleVelocityDrag (e);
        }

        valueWhenLastDragged = jlimit (minimum, maximum, valueWhenLastDragged);

        if (sliderBeingDragged == 0)
        {
            setValue (snapValue (valueWhenLastDragged, true),
                      ! sendChangeOnlyOnRelease, true);
        }
        else if (sliderBeingDragged == 1)
        {
            setMinValue (snapValue (valueWhenLastDragged, true),
                         ! sendChangeOnlyOnRelease, false, true);

            if (e.mods.isShiftDown())
                setMaxValue (getMinValue() + minMaxDiff, false, false, true);
            else
                minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
        }
        else if (sliderBeingDragged == 2)
        {
            setMaxValue (snapValue (valueWhenLastDragged, true),
                         ! sendChangeOnlyOnRelease, false, true);

            if (e.mods.isShiftDown())
                setMinValue (getMaxValue() - minMaxDiff, false, false, true);
            else
                minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
        }

        mousePosWhenLastDragged = e.getPosition();
    }
}

void Slider::mouseDoubleClick (const MouseEvent&)
{
    if (doubleClickToValue
         && isEnabled()
         && style != IncDecButtons
         && minimum <= doubleClickReturnValue
         && maximum >= doubleClickReturnValue)
    {
        sendDragStart();
        setValue (doubleClickReturnValue, true, true);
        sendDragEnd();
    }
}

void Slider::mouseWheelMove (const MouseEvent& e, float wheelIncrementX, float wheelIncrementY)
{
    if (scrollWheelEnabled && isEnabled()
         && style != TwoValueHorizontal
         && style != TwoValueVertical)
    {
        if (maximum > minimum && ! e.mods.isAnyMouseButtonDown())
        {
            if (valueBox != nullptr)
                valueBox->hideEditor (false);

            const double value = (double) currentValue.getValue();
            const double proportionDelta = (wheelIncrementX != 0 ? -wheelIncrementX : wheelIncrementY) * 0.15f;
            const double currentPos = valueToProportionOfLength (value);
            const double newValue = proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + proportionDelta));

            double delta = (newValue != value)
                            ? jmax (std::abs (newValue - value), interval) : 0;

            if (value > newValue)
                delta = -delta;

            sendDragStart();
            setValue (snapValue (value + delta, false), true, true);
            sendDragEnd();
        }
    }
    else
    {
        Component::mouseWheelMove (e, wheelIncrementX, wheelIncrementY);
    }
}

void SliderListener::sliderDragStarted (Slider*)  // (can't write Slider::Listener due to idiotic VC2005 bug)
{
}

void SliderListener::sliderDragEnded (Slider*)
{
}

const Identifier Slider::Ids::tagType ("SLIDER");
const Identifier Slider::Ids::min ("min");
const Identifier Slider::Ids::max ("max");
const Identifier Slider::Ids::interval ("interval");
const Identifier Slider::Ids::type ("type");
const Identifier Slider::Ids::editable ("editable");
const Identifier Slider::Ids::textBoxPos ("textBoxPos");
const Identifier Slider::Ids::textBoxWidth ("textBoxWidth");
const Identifier Slider::Ids::textBoxHeight ("textBoxHeight");
const Identifier Slider::Ids::skew ("skew");

void Slider::refreshFromValueTree (const ValueTree& state, ComponentBuilder&)
{
    ComponentBuilder::refreshBasicComponentProperties (*this, state);

    setRange (static_cast <double> (state [Ids::min]),
              static_cast <double> (state [Ids::max]),
              static_cast <double> (state [Ids::interval]));

    setSliderStyle ((SliderStyle) static_cast <int> (state [Ids::type]));

    setTextBoxStyle ((TextEntryBoxPosition) static_cast <int> (state [Ids::textBoxPos]),
                     ! static_cast <bool> (state [Ids::editable]),
                     static_cast <int> (state [Ids::textBoxWidth]),
                     static_cast <int> (state [Ids::textBoxHeight]));

    setSkewFactor (static_cast <double> (state [Ids::skew]));
}
