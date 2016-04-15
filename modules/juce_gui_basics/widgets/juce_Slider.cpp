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

class Slider::Pimpl   : public AsyncUpdater,
                        public ButtonListener,  // (can't use Button::Listener due to idiotic VC2005 bug)
                        public LabelListener,
                        public ValueListener
{
public:
    Pimpl (Slider& s, SliderStyle sliderStyle, TextEntryBoxPosition textBoxPosition)
      : owner (s),
        style (sliderStyle),
        lastCurrentValue (0), lastValueMin (0), lastValueMax (0),
        minimum (0), maximum (10), interval (0), doubleClickReturnValue (0),
        skewFactor (1.0), velocityModeSensitivity (1.0),
        velocityModeOffset (0.0), velocityModeThreshold (1),
        sliderRegionStart (0), sliderRegionSize (1), sliderBeingDragged (-1),
        pixelsForFullDragExtent (250),
        textBoxPos (textBoxPosition),
        numDecimalPlaces (7),
        textBoxWidth (80), textBoxHeight (20),
        incDecButtonMode (incDecButtonsNotDraggable),
        editableText (true),
        doubleClickToValue (false),
        isVelocityBased (false),
        userKeyOverridesVelocity (true),
        incDecButtonsSideBySide (false),
        sendChangeOnlyOnRelease (false),
        popupDisplayEnabled (false),
        menuEnabled (false),
        useDragEvents (false),
        scrollWheelEnabled (true),
        snapsToMousePos (true),
        parentForPopupDisplay (nullptr)
    {
        rotaryParams.startAngleRadians = float_Pi * 1.2f;
        rotaryParams.endAngleRadians   = float_Pi * 2.8f;
        rotaryParams.stopAtEnd = true;
    }

    ~Pimpl()
    {
        currentValue.removeListener (this);
        valueMin.removeListener (this);
        valueMax.removeListener (this);
        popupDisplay = nullptr;
    }

    //==============================================================================
    void registerListeners()
    {
        currentValue.addListener (this);
        valueMin.addListener (this);
        valueMax.addListener (this);
    }

    bool isHorizontal() const noexcept
    {
        return style == LinearHorizontal
            || style == LinearBar
            || style == TwoValueHorizontal
            || style == ThreeValueHorizontal;
    }

    bool isVertical() const noexcept
    {
        return style == LinearVertical
            || style == LinearBarVertical
            || style == TwoValueVertical
            || style == ThreeValueVertical;
    }

    bool isRotary() const noexcept
    {
        return style == Rotary
            || style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == RotaryHorizontalVerticalDrag;
    }

    bool isBar() const noexcept
    {
        return style == LinearBar
            || style == LinearBarVertical;
    }

    bool incDecDragDirectionIsHorizontal() const noexcept
    {
        return incDecButtonMode == incDecButtonsDraggable_Horizontal
                || (incDecButtonMode == incDecButtonsDraggable_AutoDirection && incDecButtonsSideBySide);
    }

    float getPositionOfValue (const double value) const
    {
        if (isHorizontal() || isVertical())
            return getLinearSliderPos (value);

        jassertfalse; // not a valid call on a slider that doesn't work linearly!
        return 0.0f;
    }

    void setRange (const double newMin, const double newMax, const double newInt)
    {
        if (minimum != newMin || maximum != newMax || interval != newInt)
        {
            minimum = newMin;
            maximum = newMax;
            interval = newInt;

            // figure out the number of DPs needed to display all values at this
            // interval setting.
            numDecimalPlaces = 7;

            if (newInt != 0)
            {
                int v = std::abs (roundToInt (newInt * 10000000));

                while ((v % 10) == 0)
                {
                    --numDecimalPlaces;
                    v /= 10;
                }
            }

            // keep the current values inside the new range..
            if (style != TwoValueHorizontal && style != TwoValueVertical)
            {
                setValue (getValue(), dontSendNotification);
            }
            else
            {
                setMinValue (getMinValue(), dontSendNotification, false);
                setMaxValue (getMaxValue(), dontSendNotification, false);
            }

            updateText();
        }
    }

    double getValue() const
    {
        // for a two-value style slider, you should use the getMinValue() and getMaxValue()
        // methods to get the two values.
        jassert (style != TwoValueHorizontal && style != TwoValueVertical);

        return currentValue.getValue();
    }

    void setValue (double newValue, const NotificationType notification)
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
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (newValue));

            triggerChangeMessage (notification);
        }
    }

    void setMinValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue > (double) valueMax.getValue())
                setMaxValue (newValue, notification, false);

            newValue = jmin ((double) valueMax.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue > lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmin (lastCurrentValue, newValue);
        }

        if (lastValueMin != newValue)
        {
            lastValueMin = newValue;
            valueMin = newValue;
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (newValue));

            triggerChangeMessage (notification);
        }
    }

    void setMaxValue (double newValue, const NotificationType notification,
                      const bool allowNudgingOfOtherValues)
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        newValue = constrainedValue (newValue);

        if (style == TwoValueHorizontal || style == TwoValueVertical)
        {
            if (allowNudgingOfOtherValues && newValue < (double) valueMin.getValue())
                setMinValue (newValue, notification, false);

            newValue = jmax ((double) valueMin.getValue(), newValue);
        }
        else
        {
            if (allowNudgingOfOtherValues && newValue < lastCurrentValue)
                setValue (newValue, notification);

            newValue = jmax (lastCurrentValue, newValue);
        }

        if (lastValueMax != newValue)
        {
            lastValueMax = newValue;
            valueMax = newValue;
            owner.repaint();

            if (popupDisplay != nullptr)
                popupDisplay->updatePosition (owner.getTextFromValue (valueMax.getValue()));

            triggerChangeMessage (notification);
        }
    }

    void setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
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
            owner.repaint();

            triggerChangeMessage (notification);
        }
    }

    double getMinValue() const
    {
        // The minimum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMin.getValue();
    }

    double getMaxValue() const
    {
        // The maximum value only applies to sliders that are in two- or three-value mode.
        jassert (style == TwoValueHorizontal || style == TwoValueVertical
                  || style == ThreeValueHorizontal || style == ThreeValueVertical);

        return valueMax.getValue();
    }

    void triggerChangeMessage (const NotificationType notification)
    {
        if (notification != dontSendNotification)
        {
            owner.valueChanged();

            if (notification == sendNotificationSync)
                handleAsyncUpdate();
            else
                triggerAsyncUpdate();
        }
    }

    void handleAsyncUpdate() override
    {
        cancelPendingUpdate();

        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderValueChanged, slider);  // (can't use Slider::Listener due to idiotic VC2005 bug)
    }

    void sendDragStart()
    {
        owner.startedDragging();

        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderDragStarted, slider);
    }

    void sendDragEnd()
    {
        owner.stoppedDragging();

        sliderBeingDragged = -1;

        Component::BailOutChecker checker (&owner);
        Slider* slider = &owner; // (must use an intermediate variable here to avoid a VS2005 compiler bug)
        listeners.callChecked (checker, &SliderListener::sliderDragEnded, slider);
    }

    struct DragInProgress
    {
        DragInProgress (Pimpl& p)  : owner (p)      { owner.sendDragStart(); }
        ~DragInProgress()                           { owner.sendDragEnd(); }

        Pimpl& owner;

        JUCE_DECLARE_NON_COPYABLE (DragInProgress)
    };

    void buttonClicked (Button* button) override
    {
        if (style == IncDecButtons)
        {
            const double delta = (button == incButton) ? interval : -interval;

            DragInProgress drag (*this);
            setValue (owner.snapValue (getValue() + delta, notDragging), sendNotificationSync);
        }
    }

    void valueChanged (Value& value) override
    {
        if (value.refersToSameSourceAs (currentValue))
        {
            if (style != TwoValueHorizontal && style != TwoValueVertical)
                setValue (currentValue.getValue(), dontSendNotification);
        }
        else if (value.refersToSameSourceAs (valueMin))
            setMinValue (valueMin.getValue(), dontSendNotification, true);
        else if (value.refersToSameSourceAs (valueMax))
            setMaxValue (valueMax.getValue(), dontSendNotification, true);
    }

    void labelTextChanged (Label* label) override
    {
        const double newValue = owner.snapValue (owner.getValueFromText (label->getText()), notDragging);

        if (newValue != (double) currentValue.getValue())
        {
            DragInProgress drag (*this);
            setValue (newValue, sendNotificationSync);
        }

        updateText(); // force a clean-up of the text, needed in case setValue() hasn't done this.
    }

    void updateText()
    {
        if (valueBox != nullptr)
        {
            String newValue (owner.getTextFromValue (currentValue.getValue()));

            if (newValue != valueBox->getText())
                valueBox->setText (newValue, dontSendNotification);
        }
    }

    double constrainedValue (double value) const
    {
        if (interval > 0)
            value = minimum + interval * std::floor ((value - minimum) / interval + 0.5);

        if (value <= minimum || maximum <= minimum)
            value = minimum;
        else if (value >= maximum)
            value = maximum;

        return value;
    }

    float getLinearSliderPos (const double value) const
    {
        double pos;

        if (maximum <= minimum)
            pos = 0.5;
        else if (value < minimum)
            pos = 0.0;
        else if (value > maximum)
            pos = 1.0;
        else
            pos = owner.valueToProportionOfLength (value);

        if (isVertical() || style == IncDecButtons)
            pos = 1.0 - pos;

        jassert (pos >= 0 && pos <= 1.0);
        return (float) (sliderRegionStart + pos * sliderRegionSize);
    }

    void setSliderStyle (const SliderStyle newStyle)
    {
        if (style != newStyle)
        {
            style = newStyle;
            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    void setVelocityModeParameters (const double sensitivity, const int threshold,
                                    const double offset, const bool userCanPressKeyToSwapMode)
    {
        velocityModeSensitivity = sensitivity;
        velocityModeOffset = offset;
        velocityModeThreshold = threshold;
        userKeyOverridesVelocity = userCanPressKeyToSwapMode;
    }

    void setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
    {
        if (maximum > minimum)
            skewFactor = log (0.5) / log ((sliderValueToShowAtMidPoint - minimum)
                                            / (maximum - minimum));
    }

    void setIncDecButtonsMode (const IncDecButtonMode mode)
    {
        if (incDecButtonMode != mode)
        {
            incDecButtonMode = mode;
            owner.lookAndFeelChanged();
        }
    }

    void setTextBoxStyle (const TextEntryBoxPosition newPosition,
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

            owner.repaint();
            owner.lookAndFeelChanged();
        }
    }

    void setTextBoxIsEditable (const bool shouldBeEditable)
    {
        editableText = shouldBeEditable;
        updateTextBoxEnablement();
    }

    void showTextBox()
    {
        jassert (editableText); // this should probably be avoided in read-only sliders.

        if (valueBox != nullptr)
            valueBox->showEditor();
    }

    void hideTextBox (const bool discardCurrentEditorContents)
    {
        if (valueBox != nullptr)
        {
            valueBox->hideEditor (discardCurrentEditorContents);

            if (discardCurrentEditorContents)
                updateText();
        }
    }

    void setTextValueSuffix (const String& suffix)
    {
        if (textSuffix != suffix)
        {
            textSuffix = suffix;
            updateText();
        }
    }

    void updateTextBoxEnablement()
    {
        if (valueBox != nullptr)
        {
            const bool shouldBeEditable = editableText && owner.isEnabled();

            if (valueBox->isEditable() != shouldBeEditable) // (to avoid changing the single/double click flags unless we need to)
                valueBox->setEditable (shouldBeEditable);
        }
    }

    void lookAndFeelChanged (LookAndFeel& lf)
    {
        if (textBoxPos != NoTextBox)
        {
            const String previousTextBoxContent (valueBox != nullptr ? valueBox->getText()
                                                                     : owner.getTextFromValue (currentValue.getValue()));

            valueBox = nullptr;
            owner.addAndMakeVisible (valueBox = lf.createSliderTextBox (owner));

            valueBox->setWantsKeyboardFocus (false);
            valueBox->setText (previousTextBoxContent, dontSendNotification);
            valueBox->setTooltip (owner.getTooltip());
            updateTextBoxEnablement();
            valueBox->addListener (this);

            if (style == LinearBar || style == LinearBarVertical)
            {
                valueBox->addMouseListener (&owner, false);
                valueBox->setMouseCursor (MouseCursor::ParentCursor);
            }
        }
        else
        {
            valueBox = nullptr;
        }

        if (style == IncDecButtons)
        {
            owner.addAndMakeVisible (incButton = lf.createSliderButton (owner, true));
            incButton->addListener (this);

            owner.addAndMakeVisible (decButton = lf.createSliderButton (owner, false));
            decButton->addListener (this);

            if (incDecButtonMode != incDecButtonsNotDraggable)
            {
                incButton->addMouseListener (&owner, false);
                decButton->addMouseListener (&owner, false);
            }
            else
            {
                incButton->setRepeatSpeed (300, 100, 20);
                decButton->setRepeatSpeed (300, 100, 20);
            }

            const String tooltip (owner.getTooltip());
            incButton->setTooltip (tooltip);
            decButton->setTooltip (tooltip);
        }
        else
        {
            incButton = nullptr;
            decButton = nullptr;
        }

        owner.setComponentEffect (lf.getSliderEffect (owner));

        owner.resized();
        owner.repaint();
    }

    void showPopupMenu()
    {
        PopupMenu m;
        m.setLookAndFeel (&owner.getLookAndFeel());
        m.addItem (1, TRANS ("Velocity-sensitive mode"), true, isVelocityBased);
        m.addSeparator();

        if (isRotary())
        {
            PopupMenu rotaryMenu;
            rotaryMenu.addItem (2, TRANS ("Use circular dragging"),           true, style == Rotary);
            rotaryMenu.addItem (3, TRANS ("Use left-right dragging"),         true, style == RotaryHorizontalDrag);
            rotaryMenu.addItem (4, TRANS ("Use up-down dragging"),            true, style == RotaryVerticalDrag);
            rotaryMenu.addItem (5, TRANS ("Use left-right/up-down dragging"), true, style == RotaryHorizontalVerticalDrag);

            m.addSubMenu (TRANS ("Rotary mode"), rotaryMenu);
        }

        m.showMenuAsync (PopupMenu::Options(),
                         ModalCallbackFunction::forComponent (sliderMenuCallback, &owner));
    }

    static void sliderMenuCallback (const int result, Slider* slider)
    {
        if (slider != nullptr)
        {
            switch (result)
            {
                case 1:   slider->setVelocityBasedMode (! slider->getVelocityBasedMode()); break;
                case 2:   slider->setSliderStyle (Rotary); break;
                case 3:   slider->setSliderStyle (RotaryHorizontalDrag); break;
                case 4:   slider->setSliderStyle (RotaryVerticalDrag); break;
                case 5:   slider->setSliderStyle (RotaryHorizontalVerticalDrag); break;
                default:  break;
            }
        }
    }

    int getThumbIndexAt (const MouseEvent& e)
    {
        const bool isTwoValue   = (style == TwoValueHorizontal   || style == TwoValueVertical);
        const bool isThreeValue = (style == ThreeValueHorizontal || style == ThreeValueVertical);

        if (isTwoValue || isThreeValue)
        {
            const float mousePos = isVertical() ? e.position.y : e.position.x;

            const float normalPosDistance = std::abs (getLinearSliderPos (currentValue.getValue()) - mousePos);
            const float minPosDistance    = std::abs (getLinearSliderPos (valueMin.getValue()) - 0.1f - mousePos);
            const float maxPosDistance    = std::abs (getLinearSliderPos (valueMax.getValue()) + 0.1f - mousePos);

            if (isTwoValue)
                return maxPosDistance <= minPosDistance ? 2 : 1;

            if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
                return 1;

            if (normalPosDistance >= maxPosDistance)
                return 2;
        }

        return 0;
    }

    //==============================================================================
    void handleRotaryDrag (const MouseEvent& e)
    {
        const float dx = e.position.x - sliderRect.getCentreX();
        const float dy = e.position.y - sliderRect.getCentreY();

        if (dx * dx + dy * dy > 25.0f)
        {
            double angle = std::atan2 ((double) dx, (double) -dy);
            while (angle < 0.0)
                angle += double_Pi * 2.0;

            if (rotaryParams.stopAtEnd && e.mouseWasDraggedSinceMouseDown())
            {
                if (std::abs (angle - lastAngle) > double_Pi)
                {
                    if (angle >= lastAngle)
                        angle -= double_Pi * 2.0;
                    else
                        angle += double_Pi * 2.0;
                }

                if (angle >= lastAngle)
                    angle = jmin (angle, (double) jmax (rotaryParams.startAngleRadians, rotaryParams.endAngleRadians));
                else
                    angle = jmax (angle, (double) jmin (rotaryParams.startAngleRadians, rotaryParams.endAngleRadians));
            }
            else
            {
                while (angle < rotaryParams.startAngleRadians)
                    angle += double_Pi * 2.0;

                if (angle > rotaryParams.endAngleRadians)
                {
                    if (smallestAngleBetween (angle, rotaryParams.startAngleRadians)
                         <= smallestAngleBetween (angle, rotaryParams.endAngleRadians))
                        angle = rotaryParams.startAngleRadians;
                    else
                        angle = rotaryParams.endAngleRadians;
                }
            }

            const double proportion = (angle - rotaryParams.startAngleRadians) / (rotaryParams.endAngleRadians - rotaryParams.startAngleRadians);
            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, proportion));
            lastAngle = angle;
        }
    }

    void handleAbsoluteDrag (const MouseEvent& e)
    {
        const float mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.position.x : e.position.y;
        double newPos = 0;

        if (style == RotaryHorizontalDrag
            || style == RotaryVerticalDrag
            || style == IncDecButtons
            || ((style == LinearHorizontal || style == LinearVertical || style == LinearBar || style == LinearBarVertical)
                && ! snapsToMousePos))
        {
            const float mouseDiff = (style == RotaryHorizontalDrag
                                        || style == LinearHorizontal
                                        || style == LinearBar
                                        || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                      ? e.position.x - mouseDragStartPos.x
                                      : mouseDragStartPos.y - e.position.y;

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);

            if (style == IncDecButtons)
            {
                incButton->setState (mouseDiff < 0 ? Button::buttonNormal : Button::buttonDown);
                decButton->setState (mouseDiff > 0 ? Button::buttonNormal : Button::buttonDown);
            }
        }
        else if (style == RotaryHorizontalVerticalDrag)
        {
            const float mouseDiff = (e.position.x - mouseDragStartPos.x)
                                      + (mouseDragStartPos.y - e.position.y);

            newPos = owner.valueToProportionOfLength (valueOnMouseDown)
                       + mouseDiff * (1.0 / pixelsForFullDragExtent);
        }
        else
        {
            newPos = (mousePos - sliderRegionStart) / (double) sliderRegionSize;

            if (isVertical())
                newPos = 1.0 - newPos;
        }

        valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, newPos));
    }

    void handleVelocityDrag (const MouseEvent& e)
    {
        const float mouseDiff = style == RotaryHorizontalVerticalDrag
                                   ? (e.position.x - mousePosWhenLastDragged.x) + (mousePosWhenLastDragged.y - e.position.y)
                                   : (isHorizontal()
                                       || style == RotaryHorizontalDrag
                                       || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                         ? e.position.x - mousePosWhenLastDragged.x
                                         : e.position.y - mousePosWhenLastDragged.y;

        const double maxSpeed = jmax (200, sliderRegionSize);
        double speed = jlimit (0.0, maxSpeed, (double) std::abs (mouseDiff));

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

            const double currentPos = owner.valueToProportionOfLength (valueWhenLastDragged);

            valueWhenLastDragged = owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + speed));

            e.source.enableUnboundedMouseMovement (true, false);
        }
    }

    void mouseDown (const MouseEvent& e)
    {
        incDecDragged = false;
        useDragEvents = false;
        mouseDragStartPos = mousePosWhenLastDragged = e.position;
        currentDrag = nullptr;

        if (owner.isEnabled())
        {
            if (e.mods.isPopupMenu() && menuEnabled)
            {
                showPopupMenu();
            }
            else if (canDoubleClickToValue()
                      && e.mods.withoutMouseButtons() == ModifierKeys (ModifierKeys::altModifier))
            {
                mouseDoubleClick();
            }
            else if (maximum > minimum)
            {
                useDragEvents = true;

                if (valueBox != nullptr)
                    valueBox->hideEditor (true);

                sliderBeingDragged = getThumbIndexAt (e);

                minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();

                lastAngle = rotaryParams.startAngleRadians
                                + (rotaryParams.endAngleRadians - rotaryParams.startAngleRadians)
                                     * owner.valueToProportionOfLength (currentValue.getValue());

                valueWhenLastDragged = (sliderBeingDragged == 2 ? valueMax
                                                                : (sliderBeingDragged == 1 ? valueMin
                                                                                           : currentValue)).getValue();
                valueOnMouseDown = valueWhenLastDragged;

                if (popupDisplayEnabled)
                {
                    PopupDisplayComponent* const popup = new PopupDisplayComponent (owner);
                    popupDisplay = popup;

                    if (parentForPopupDisplay != nullptr)
                        parentForPopupDisplay->addChildComponent (popup);
                    else
                        popup->addToDesktop (ComponentPeer::windowIsTemporary);

                    popup->setVisible (true);
                }

                currentDrag = new DragInProgress (*this);
                mouseDrag (e);
            }
        }
    }

    void mouseDrag (const MouseEvent& e)
    {
        if (useDragEvents && maximum > minimum
             && ! ((style == LinearBar || style == LinearBarVertical)
                    && e.mouseWasClicked() && valueBox != nullptr && valueBox->isEditable()))
        {
            DragMode dragMode = notDragging;

            if (style == Rotary)
            {
                handleRotaryDrag (e);
            }
            else
            {
                if (style == IncDecButtons && ! incDecDragged)
                {
                    if (e.getDistanceFromDragStart() < 10 || ! e.mouseWasDraggedSinceMouseDown())
                        return;

                    incDecDragged = true;
                    mouseDragStartPos = e.position;
                }

                if (isAbsoluteDragMode (e.mods) || (maximum - minimum) / sliderRegionSize < interval)
                {
                    dragMode = absoluteDrag;
                    handleAbsoluteDrag (e);
                }
                else
                {
                    dragMode = velocityDrag;
                    handleVelocityDrag (e);
                }
            }

            valueWhenLastDragged = jlimit (minimum, maximum, valueWhenLastDragged);

            if (sliderBeingDragged == 0)
            {
                setValue (owner.snapValue (valueWhenLastDragged, dragMode),
                          sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationSync);
            }
            else if (sliderBeingDragged == 1)
            {
                setMinValue (owner.snapValue (valueWhenLastDragged, dragMode),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMaxValue (getMinValue() + minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }
            else if (sliderBeingDragged == 2)
            {
                setMaxValue (owner.snapValue (valueWhenLastDragged, dragMode),
                             sendChangeOnlyOnRelease ? dontSendNotification : sendNotificationAsync, true);

                if (e.mods.isShiftDown())
                    setMinValue (getMaxValue() - minMaxDiff, dontSendNotification, true);
                else
                    minMaxDiff = (double) valueMax.getValue() - (double) valueMin.getValue();
            }

            mousePosWhenLastDragged = e.position;
        }
    }

    void mouseUp()
    {
        if (owner.isEnabled()
             && useDragEvents
             && (maximum > minimum)
             && (style != IncDecButtons || incDecDragged))
        {
            restoreMouseIfHidden();

            if (sendChangeOnlyOnRelease && valueOnMouseDown != (double) currentValue.getValue())
                triggerChangeMessage (sendNotificationAsync);

            currentDrag = nullptr;
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

        currentDrag = nullptr;
    }

    bool canDoubleClickToValue() const
    {
        return doubleClickToValue
                && style != IncDecButtons
                && minimum <= doubleClickReturnValue
                && maximum >= doubleClickReturnValue;
    }

    void mouseDoubleClick()
    {
        if (canDoubleClickToValue())
        {
            DragInProgress drag (*this);
            setValue (doubleClickReturnValue, sendNotificationSync);
        }
    }

    double getMouseWheelDelta (double value, double wheelAmount)
    {
        if (style == IncDecButtons)
            return interval * wheelAmount;

        const double proportionDelta = wheelAmount * 0.15f;
        const double currentPos = owner.valueToProportionOfLength (value);
        return owner.proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + proportionDelta)) - value;
    }

    bool mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
    {
        if (scrollWheelEnabled
             && style != TwoValueHorizontal
             && style != TwoValueVertical)
        {
            // sometimes duplicate wheel events seem to be sent, so since we're going to
            // bump the value by a minimum of the interval, avoid doing this twice..
            if (e.eventTime != lastMouseWheelTime)
            {
                lastMouseWheelTime = e.eventTime;

                if (maximum > minimum && ! e.mods.isAnyMouseButtonDown())
                {
                    if (valueBox != nullptr)
                        valueBox->hideEditor (false);

                    const double value = (double) currentValue.getValue();
                    const double delta = getMouseWheelDelta (value, (std::abs (wheel.deltaX) > std::abs (wheel.deltaY)
                                                                        ? -wheel.deltaX : wheel.deltaY)
                                                                      * (wheel.isReversed ? -1.0f : 1.0f));
                    if (delta != 0)
                    {
                        const double newValue = value + jmax (interval, std::abs (delta)) * (delta < 0 ? -1.0 : 1.0);

                        DragInProgress drag (*this);
                        setValue (owner.snapValue (newValue, notDragging), sendNotificationSync);
                    }
                }
            }

            return true;
        }

        return false;
    }

    void modifierKeysChanged (const ModifierKeys& modifiers)
    {
        if (style != IncDecButtons && style != Rotary && isAbsoluteDragMode (modifiers))
            restoreMouseIfHidden();
    }

    bool isAbsoluteDragMode (ModifierKeys mods) const
    {
        return isVelocityBased == (userKeyOverridesVelocity
                                    && mods.testFlags (ModifierKeys::ctrlAltCommandModifiers));
    }

    void restoreMouseIfHidden()
    {
        const Array<MouseInputSource>& mouseSources = Desktop::getInstance().getMouseSources();

        for (MouseInputSource* mi = mouseSources.begin(), * const e = mouseSources.end(); mi != e; ++mi)
        {
            if (mi->isUnboundedMouseMovementEnabled())
            {
                mi->enableUnboundedMouseMovement (false);

                const double pos = sliderBeingDragged == 2 ? getMaxValue()
                                                           : (sliderBeingDragged == 1 ? getMinValue()
                                                                                      : (double) currentValue.getValue());
                Point<float> mousePos;

                if (isRotary())
                {
                    mousePos = mi->getLastMouseDownPosition();

                    const float delta = (float) (pixelsForFullDragExtent * (owner.valueToProportionOfLength (valueOnMouseDown)
                                                                                - owner.valueToProportionOfLength (pos)));

                    if (style == RotaryHorizontalDrag)      mousePos += Point<float> (-delta, 0.0f);
                    else if (style == RotaryVerticalDrag)   mousePos += Point<float> (0.0f, delta);
                    else                                    mousePos += Point<float> (delta / -2.0f, delta / 2.0f);

                    mousePos = owner.getScreenBounds().reduced (4).toFloat().getConstrainedPoint (mousePos);
                    mouseDragStartPos = mousePosWhenLastDragged = owner.getLocalPoint (nullptr, mousePos);
                    valueOnMouseDown = valueWhenLastDragged;
                }
                else
                {
                    const float pixelPos = (float) getLinearSliderPos (pos);

                    mousePos = owner.localPointToGlobal (Point<float> (isHorizontal() ? pixelPos : (owner.getWidth() / 2.0f),
                                                                       isVertical()   ? pixelPos : (owner.getHeight() / 2.0f)));
                }

                mi->setScreenPosition (mousePos);
            }
        }
    }

    //==============================================================================
    void paint (Graphics& g, LookAndFeel& lf)
    {
        if (style != IncDecButtons)
        {
            if (isRotary())
            {
                const float sliderPos = (float) owner.valueToProportionOfLength (lastCurrentValue);
                jassert (sliderPos >= 0 && sliderPos <= 1.0f);

                lf.drawRotarySlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     sliderPos, rotaryParams.startAngleRadians,
                                     rotaryParams.endAngleRadians, owner);
            }
            else
            {
                lf.drawLinearSlider (g,
                                     sliderRect.getX(), sliderRect.getY(),
                                     sliderRect.getWidth(), sliderRect.getHeight(),
                                     getLinearSliderPos (lastCurrentValue),
                                     getLinearSliderPos (lastValueMin),
                                     getLinearSliderPos (lastValueMax),
                                     style, owner);
            }

            if ((style == LinearBar || style == LinearBarVertical) && valueBox == nullptr)
            {
                g.setColour (owner.findColour (Slider::textBoxOutlineColourId));
                g.drawRect (0, 0, owner.getWidth(), owner.getHeight(), 1);
            }
        }
    }

    //==============================================================================
    void resized (LookAndFeel& lf)
    {
        SliderLayout layout = lf.getSliderLayout (owner);

        sliderRect = layout.sliderBounds;

        if (valueBox != nullptr)
            valueBox->setBounds (layout.textBoxBounds);

        if (isHorizontal())
        {
            sliderRegionStart = layout.sliderBounds.getX();
            sliderRegionSize = layout.sliderBounds.getWidth();
        }
        else if (isVertical())
        {
            sliderRegionStart = layout.sliderBounds.getY();
            sliderRegionSize = layout.sliderBounds.getHeight();
        }
        else if (style == IncDecButtons)
        {
            resizeIncDecButtons();
        }
    }

    //==============================================================================

    void resizeIncDecButtons()
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

    //==============================================================================
    Slider& owner;
    SliderStyle style;

    ListenerList <SliderListener> listeners;
    Value currentValue, valueMin, valueMax;
    double lastCurrentValue, lastValueMin, lastValueMax;
    double minimum, maximum, interval, doubleClickReturnValue;
    double valueWhenLastDragged, valueOnMouseDown, skewFactor, lastAngle;
    double velocityModeSensitivity, velocityModeOffset, minMaxDiff;
    int velocityModeThreshold;
    RotaryParameters rotaryParams;
    Point<float> mouseDragStartPos, mousePosWhenLastDragged;
    int sliderRegionStart, sliderRegionSize;
    int sliderBeingDragged;
    int pixelsForFullDragExtent;
    Time lastMouseWheelTime;
    Rectangle<int> sliderRect;
    ScopedPointer<DragInProgress> currentDrag;

    TextEntryBoxPosition textBoxPos;
    String textSuffix;
    int numDecimalPlaces;
    int textBoxWidth, textBoxHeight;
    IncDecButtonMode incDecButtonMode;

    bool editableText;
    bool doubleClickToValue;
    bool isVelocityBased;
    bool userKeyOverridesVelocity;
    bool incDecButtonsSideBySide;
    bool sendChangeOnlyOnRelease;
    bool popupDisplayEnabled;
    bool menuEnabled;
    bool useDragEvents;
    bool incDecDragged;
    bool scrollWheelEnabled;
    bool snapsToMousePos;

    ScopedPointer<Label> valueBox;
    ScopedPointer<Button> incButton, decButton;

    //==============================================================================
    class PopupDisplayComponent  : public BubbleComponent,
                                   public Timer
    {
    public:
        PopupDisplayComponent (Slider& s)
            : owner (s),
              font (s.getLookAndFeel().getSliderPopupFont (s))
        {
            setAlwaysOnTop (true);
            setAllowedPlacement (owner.getLookAndFeel().getSliderPopupPlacement (s));
            setLookAndFeel (&s.getLookAndFeel());
        }

        void paintContent (Graphics& g, int w, int h) override
        {
            g.setFont (font);
            g.setColour (owner.findColour (TooltipWindow::textColourId, true));
            g.drawFittedText (text, Rectangle<int> (w, h), Justification::centred, 1);
        }

        void getContentSize (int& w, int& h) override
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

        void timerCallback() override
        {
            owner.pimpl->popupDisplay = nullptr;
        }

    private:
        Slider& owner;
        Font font;
        String text;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PopupDisplayComponent)
    };

    ScopedPointer<PopupDisplayComponent> popupDisplay;
    Component* parentForPopupDisplay;

    //==============================================================================
    static double smallestAngleBetween (const double a1, const double a2) noexcept
    {
        return jmin (std::abs (a1 - a2),
                     std::abs (a1 + double_Pi * 2.0 - a2),
                     std::abs (a2 + double_Pi * 2.0 - a1));
    }
};


//==============================================================================
Slider::Slider()
{
    init (LinearHorizontal, TextBoxLeft);
}

Slider::Slider (const String& name)  : Component (name)
{
    init (LinearHorizontal, TextBoxLeft);
}

Slider::Slider (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    init (style, textBoxPos);
}

void Slider::init (SliderStyle style, TextEntryBoxPosition textBoxPos)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    pimpl = new Pimpl (*this, style, textBoxPos);

    Slider::lookAndFeelChanged();
    updateText();

    pimpl->registerListeners();
}

Slider::~Slider() {}

//==============================================================================
void Slider::addListener (SliderListener* const listener)       { pimpl->listeners.add (listener); }
void Slider::removeListener (SliderListener* const listener)    { pimpl->listeners.remove (listener); }

//==============================================================================
Slider::SliderStyle Slider::getSliderStyle() const noexcept     { return pimpl->style; }
void Slider::setSliderStyle (const SliderStyle newStyle)        { pimpl->setSliderStyle (newStyle); }

void Slider::setRotaryParameters (RotaryParameters p) noexcept
{
    // make sure the values are sensible..
    jassert (p.startAngleRadians >= 0 && p.endAngleRadians >= 0);
    jassert (p.startAngleRadians < float_Pi * 4.0f && p.endAngleRadians < float_Pi * 4.0f);
    jassert (p.startAngleRadians < p.endAngleRadians);

    pimpl->rotaryParams = p;
}

void Slider::setRotaryParameters (float startAngleRadians, float endAngleRadians, bool stopAtEnd) noexcept
{
    RotaryParameters p = { startAngleRadians, endAngleRadians, stopAtEnd };
    setRotaryParameters (p);
}

Slider::RotaryParameters Slider::getRotaryParameters() const noexcept
{
    return pimpl->rotaryParams;
}

void Slider::setVelocityBasedMode (bool vb)                 { pimpl->isVelocityBased = vb; }
bool Slider::getVelocityBasedMode() const noexcept          { return pimpl->isVelocityBased; }
bool Slider::getVelocityModeIsSwappable() const noexcept    { return pimpl->userKeyOverridesVelocity; }
int Slider::getVelocityThreshold() const noexcept           { return pimpl->velocityModeThreshold; }
double Slider::getVelocitySensitivity() const noexcept      { return pimpl->velocityModeSensitivity; }
double Slider::getVelocityOffset() const noexcept           { return pimpl->velocityModeOffset; }

void Slider::setVelocityModeParameters (const double sensitivity, const int threshold,
                                        const double offset, const bool userCanPressKeyToSwapMode)
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    pimpl->setVelocityModeParameters (sensitivity, threshold, offset, userCanPressKeyToSwapMode);
}

double Slider::getSkewFactor() const noexcept               { return pimpl->skewFactor; }
void Slider::setSkewFactor (const double factor)            { pimpl->skewFactor = factor; }

void Slider::setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint)
{
    pimpl->setSkewFactorFromMidPoint (sliderValueToShowAtMidPoint);
}

int Slider::getMouseDragSensitivity() const noexcept        { return pimpl->pixelsForFullDragExtent; }

void Slider::setMouseDragSensitivity (const int distanceForFullScaleDrag)
{
    jassert (distanceForFullScaleDrag > 0);

    pimpl->pixelsForFullDragExtent = distanceForFullScaleDrag;
}

void Slider::setIncDecButtonsMode (const IncDecButtonMode mode)             { pimpl->setIncDecButtonsMode (mode); }

Slider::TextEntryBoxPosition Slider::getTextBoxPosition() const noexcept    { return pimpl->textBoxPos; }
int Slider::getTextBoxWidth() const noexcept                                { return pimpl->textBoxWidth; }
int Slider::getTextBoxHeight() const noexcept                               { return pimpl->textBoxHeight; }

void Slider::setTextBoxStyle (const TextEntryBoxPosition newPosition, const bool isReadOnly,
                              const int textEntryBoxWidth, const int textEntryBoxHeight)
{
    pimpl->setTextBoxStyle (newPosition, isReadOnly, textEntryBoxWidth, textEntryBoxHeight);
}

bool Slider::isTextBoxEditable() const noexcept                     { return pimpl->editableText; }
void Slider::setTextBoxIsEditable (const bool shouldBeEditable)     { pimpl->setTextBoxIsEditable (shouldBeEditable); }
void Slider::showTextBox()                                          { pimpl->showTextBox(); }
void Slider::hideTextBox (const bool discardCurrentEditorContents)  { pimpl->hideTextBox (discardCurrentEditorContents); }

void Slider::setChangeNotificationOnlyOnRelease (bool onlyNotifyOnRelease)
{
    pimpl->sendChangeOnlyOnRelease = onlyNotifyOnRelease;
}

bool Slider::getSliderSnapsToMousePosition() const noexcept                 { return pimpl->snapsToMousePos; }
void Slider::setSliderSnapsToMousePosition (const bool shouldSnapToMouse)   { pimpl->snapsToMousePos = shouldSnapToMouse; }

void Slider::setPopupDisplayEnabled (const bool enabled, Component* const parentComponentToUse)
{
    pimpl->popupDisplayEnabled = enabled;
    pimpl->parentForPopupDisplay = parentComponentToUse;
}

Component* Slider::getCurrentPopupDisplay() const noexcept      { return pimpl->popupDisplay.get(); }

//==============================================================================
void Slider::colourChanged()        { lookAndFeelChanged(); }
void Slider::lookAndFeelChanged()   { pimpl->lookAndFeelChanged (getLookAndFeel()); }
void Slider::enablementChanged()    { repaint(); pimpl->updateTextBoxEnablement(); }

//==============================================================================
double Slider::getMaximum() const noexcept      { return pimpl->maximum; }
double Slider::getMinimum() const noexcept      { return pimpl->minimum; }
double Slider::getInterval() const noexcept     { return pimpl->interval; }

void Slider::setRange (double newMin, double newMax, double newInt)
{
    pimpl->setRange (newMin, newMax, newInt);
}

Value& Slider::getValueObject() noexcept        { return pimpl->currentValue; }
Value& Slider::getMinValueObject() noexcept     { return pimpl->valueMin; }
Value& Slider::getMaxValueObject() noexcept     { return pimpl->valueMax; }

double Slider::getValue() const                 { return pimpl->getValue(); }

void Slider::setValue (double newValue, const NotificationType notification)
{
    pimpl->setValue (newValue, notification);
}

double Slider::getMinValue() const      { return pimpl->getMinValue(); }
double Slider::getMaxValue() const      { return pimpl->getMaxValue(); }

void Slider::setMinValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMinValue (newValue, notification, allowNudgingOfOtherValues);
}

void Slider::setMaxValue (double newValue, const NotificationType notification, bool allowNudgingOfOtherValues)
{
    pimpl->setMaxValue (newValue, notification, allowNudgingOfOtherValues);
}

void Slider::setMinAndMaxValues (double newMinValue, double newMaxValue, const NotificationType notification)
{
    pimpl->setMinAndMaxValues (newMinValue, newMaxValue, notification);
}

void Slider::setDoubleClickReturnValue (bool isDoubleClickEnabled,  double valueToSetOnDoubleClick)
{
    pimpl->doubleClickToValue = isDoubleClickEnabled;
    pimpl->doubleClickReturnValue = valueToSetOnDoubleClick;
}

double Slider::getDoubleClickReturnValue() const noexcept       { return pimpl->doubleClickReturnValue; }
bool Slider::isDoubleClickReturnEnabled() const noexcept        { return pimpl->doubleClickToValue; }

void Slider::updateText()
{
    pimpl->updateText();
}

void Slider::setTextValueSuffix (const String& suffix)
{
    pimpl->setTextValueSuffix (suffix);
}

String Slider::getTextValueSuffix() const
{
    return pimpl->textSuffix;
}

String Slider::getTextFromValue (double v)
{
    if (getNumDecimalPlacesToDisplay() > 0)
        return String (v, getNumDecimalPlacesToDisplay()) + getTextValueSuffix();

    return String (roundToInt (v)) + getTextValueSuffix();
}

double Slider::getValueFromText (const String& text)
{
    String t (text.trimStart());

    if (t.endsWith (getTextValueSuffix()))
        t = t.substring (0, t.length() - getTextValueSuffix().length());

    while (t.startsWithChar ('+'))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly ("0123456789.,-")
            .getDoubleValue();
}

double Slider::proportionOfLengthToValue (double proportion)
{
    const double skew = getSkewFactor();

    if (skew != 1.0 && proportion > 0.0)
        proportion = exp (log (proportion) / skew);

    return getMinimum() + (getMaximum() - getMinimum()) * proportion;
}

double Slider::valueToProportionOfLength (double value)
{
    const double n = (value - getMinimum()) / (getMaximum() - getMinimum());
    const double skew = getSkewFactor();

    return skew == 1.0 ? n : pow (n, skew);
}

double Slider::snapValue (double attemptedValue, DragMode)
{
    return attemptedValue;
}

int Slider::getNumDecimalPlacesToDisplay() const noexcept    { return pimpl->numDecimalPlaces; }

//==============================================================================
int Slider::getThumbBeingDragged() const noexcept            { return pimpl->sliderBeingDragged; }

void Slider::startedDragging() {}
void Slider::stoppedDragging() {}
void Slider::valueChanged() {}

//==============================================================================
void Slider::setPopupMenuEnabled (const bool menuEnabled)   { pimpl->menuEnabled = menuEnabled; }
void Slider::setScrollWheelEnabled (const bool enabled)     { pimpl->scrollWheelEnabled = enabled; }

bool Slider::isHorizontal() const noexcept                  { return pimpl->isHorizontal(); }
bool Slider::isVertical() const noexcept                    { return pimpl->isVertical(); }
bool Slider::isRotary() const noexcept                      { return pimpl->isRotary(); }
bool Slider::isBar() const noexcept                         { return pimpl->isBar(); }

float Slider::getPositionOfValue (const double value) const { return pimpl->getPositionOfValue (value); }

//==============================================================================
void Slider::paint (Graphics& g)        { pimpl->paint (g, getLookAndFeel()); }
void Slider::resized()                  { pimpl->resized (getLookAndFeel()); }

void Slider::focusOfChildComponentChanged (FocusChangeType)     { repaint(); }

void Slider::mouseDown (const MouseEvent& e)    { pimpl->mouseDown (e); }
void Slider::mouseUp (const MouseEvent&)        { pimpl->mouseUp(); }

void Slider::modifierKeysChanged (const ModifierKeys& modifiers)
{
    if (isEnabled())
        pimpl->modifierKeysChanged (modifiers);
}

void Slider::mouseDrag (const MouseEvent& e)
{
    if (isEnabled())
        pimpl->mouseDrag (e);
}

void Slider::mouseDoubleClick (const MouseEvent&)
{
    if (isEnabled())
        pimpl->mouseDoubleClick();
}

void Slider::mouseWheelMove (const MouseEvent& e, const MouseWheelDetails& wheel)
{
    if (! (isEnabled() && pimpl->mouseWheelMove (e, wheel)))
        Component::mouseWheelMove (e, wheel);
}
