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

#include "juce_Slider.h"
#include "../lookandfeel/juce_LookAndFeel.h"
#include "../menus/juce_PopupMenu.h"
#include "../juce_Desktop.h"
#include "../special/juce_BubbleComponent.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
class SliderPopupDisplayComponent  : public BubbleComponent
{
public:
    //==============================================================================
    SliderPopupDisplayComponent (Slider* const owner_)
        : owner (owner_),
          font (15.0f, Font::bold)
    {
        setAlwaysOnTop (true);
    }

    ~SliderPopupDisplayComponent()
    {
    }

    void paintContent (Graphics& g, int w, int h)
    {
        g.setFont (font);
        g.setColour (Colours::black);

        g.drawFittedText (text, 0, 0, w, h, Justification::centred, 1);
    }

    void getContentSize (int& w, int& h)
    {
        w = font.getStringWidth (text) + 18;
        h = (int) (font.getHeight() * 1.6f);
    }

    void updatePosition (const String& newText)
    {
        if (text != newText)
        {
            text = newText;
            repaint();
        }

        BubbleComponent::setPosition (owner);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Slider* owner;
    Font font;
    String text;

    SliderPopupDisplayComponent (const SliderPopupDisplayComponent&);
    const SliderPopupDisplayComponent& operator= (const SliderPopupDisplayComponent&);
};

//==============================================================================
Slider::Slider (const String& name)
  : Component (name),
    listeners (2),
    currentValue (0.0),
    valueMin (0.0),
    valueMax (0.0),
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
    pixelsForFullDragExtent (250),
    style (LinearHorizontal),
    textBoxPos (TextBoxLeft),
    textBoxWidth (80),
    textBoxHeight (20),
    incDecButtonMode (incDecButtonsNotDraggable),
    editableText (true),
    doubleClickToValue (false),
    isVelocityBased (false),
    rotaryStop (true),
    incDecButtonsSideBySide (false),
    sendChangeOnlyOnRelease (false),
    popupDisplayEnabled (false),
    menuEnabled (false),
    menuShown (false),
    valueBox (0),
    incButton (0),
    decButton (0),
    popupDisplay (0),
    parentForPopupDisplay (0)
{
    setWantsKeyboardFocus (false);
    setRepaintsOnMouseActivity (true);

    lookAndFeelChanged();
    updateText();
}

Slider::~Slider()
{
    deleteAndZero (popupDisplay);
    deleteAllChildren();
}


//==============================================================================
void Slider::handleAsyncUpdate()
{
    cancelPendingUpdate();

    for (int i = listeners.size(); --i >= 0;)
    {
        ((SliderListener*) listeners.getUnchecked (i))->sliderValueChanged (this);
        i = jmin (i, listeners.size());
    }
}

void Slider::sendDragStart()
{
    startedDragging();

    for (int i = listeners.size(); --i >= 0;)
    {
        ((SliderListener*) listeners.getUnchecked (i))->sliderDragStarted (this);
        i = jmin (i, listeners.size());
    }
}

void Slider::sendDragEnd()
{
    stoppedDragging();

    for (int i = listeners.size(); --i >= 0;)
    {
        ((SliderListener*) listeners.getUnchecked (i))->sliderDragEnded (this);
        i = jmin (i, listeners.size());
    }
}

void Slider::addListener (SliderListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void Slider::removeListener (SliderListener* const listener) throw()
{
    listeners.removeValue (listener);
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

void Slider::setVelocityBasedMode (const bool velBased) throw()
{
    isVelocityBased = velBased;
}

void Slider::setVelocityModeParameters (const double sensitivity,
                                        const int threshold,
                                        const double offset) throw()
{
    jassert (threshold >= 0);
    jassert (sensitivity > 0);
    jassert (offset >= 0);

    velocityModeSensitivity = sensitivity;
    velocityModeOffset = offset;
    velocityModeThreshold = threshold;
}

void Slider::setSkewFactor (const double factor) throw()
{
    skewFactor = factor;
}

void Slider::setSkewFactorFromMidPoint (const double sliderValueToShowAtMidPoint) throw()
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
    textBoxPos = newPosition;
    editableText = ! isReadOnly;
    textBoxWidth = textEntryBoxWidth;
    textBoxHeight = textEntryBoxHeight;

    repaint();
    lookAndFeelChanged();
}

void Slider::setTextBoxIsEditable (const bool shouldBeEditable) throw()
{
    editableText = shouldBeEditable;

    if (valueBox != 0)
        valueBox->setEditable (shouldBeEditable && isEnabled());
}

void Slider::hideTextBox (const bool discardCurrentEditorContents)
{
    if (valueBox != 0)
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

void Slider::setPopupDisplayEnabled (const bool enabled,
                                     Component* const parentComponentToUse)
{
    popupDisplayEnabled = enabled;
    parentForPopupDisplay = parentComponentToUse;
}

//==============================================================================
void Slider::colourChanged()
{
    lookAndFeelChanged();
}

void Slider::lookAndFeelChanged()
{
    const String previousTextBoxContent (valueBox != 0 ? valueBox->getText()
                                                       : getTextFromValue (currentValue));

    deleteAllChildren();
    valueBox = 0;

    LookAndFeel& lf = getLookAndFeel();

    if (textBoxPos != NoTextBox)
    {
        addAndMakeVisible (valueBox = getLookAndFeel().createSliderTextBox (*this));

        valueBox->setWantsKeyboardFocus (false);
        valueBox->setText (previousTextBoxContent, false);

        valueBox->setEditable (editableText && isEnabled());
        valueBox->addListener (this);

        if (style == LinearBar)
            valueBox->addMouseListener (this, false);
    }

    if (style == IncDecButtons)
    {
        addAndMakeVisible (incButton = lf.createSliderButton (true));
        incButton->addButtonListener (this);

        addAndMakeVisible (decButton = lf.createSliderButton (false));
        decButton->addButtonListener (this);

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

        setValue (currentValue, false, false);
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

double Slider::getValue() const throw()
{
    // for a two-value style slider, you should use the getMinValue() and getMaxValue()
    // methods to get the two values.
    jassert (style != TwoValueHorizontal && style != TwoValueVertical);

    return currentValue;
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
        jassert (valueMin <= valueMax);
        newValue = jlimit (valueMin, valueMax, newValue);
    }

    if (currentValue != newValue)
    {
        if (valueBox != 0)
            valueBox->hideEditor (true);

        currentValue = newValue;
        updateText();
        repaint();

        if (popupDisplay != 0)
        {
            ((SliderPopupDisplayComponent*) popupDisplay)->updatePosition (getTextFromValue (currentValue));
            popupDisplay->repaint();
        }

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

double Slider::getMinValue() const throw()
{
    // The minimum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    return valueMin;
}

double Slider::getMaxValue() const throw()
{
    // The maximum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    return valueMax;
}

void Slider::setMinValue (double newValue, const bool sendUpdateMessage, const bool sendMessageSynchronously)
{
    // The minimum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    newValue = constrainedValue (newValue);

    if (style == TwoValueHorizontal || style == TwoValueVertical)
        newValue = jmin (valueMax, newValue);
    else
        newValue = jmin (currentValue, newValue);

    if (valueMin != newValue)
    {
        valueMin = newValue;
        repaint();

        if (popupDisplay != 0)
        {
            ((SliderPopupDisplayComponent*) popupDisplay)->updatePosition (getTextFromValue (valueMin));
            popupDisplay->repaint();
        }

        if (sendUpdateMessage)
            triggerChangeMessage (sendMessageSynchronously);
    }
}

void Slider::setMaxValue (double newValue, const bool sendUpdateMessage, const bool sendMessageSynchronously)
{
    // The maximum value only applies to sliders that are in two- or three-value mode.
    jassert (style == TwoValueHorizontal || style == TwoValueVertical
              || style == ThreeValueHorizontal || style == ThreeValueVertical);

    newValue = constrainedValue (newValue);

    if (style == TwoValueHorizontal || style == TwoValueVertical)
        newValue = jmax (valueMin, newValue);
    else
        newValue = jmax (currentValue, newValue);

    if (valueMax != newValue)
    {
        valueMax = newValue;
        repaint();

        if (popupDisplay != 0)
        {
            ((SliderPopupDisplayComponent*) popupDisplay)->updatePosition (getTextFromValue (valueMax));
            popupDisplay->repaint();
        }

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
    if (valueBox != 0)
        valueBox->setText (getTextFromValue (currentValue), false);
}

void Slider::setTextValueSuffix (const String& suffix)
{
    if (textSuffix != suffix)
    {
        textSuffix = suffix;
        updateText();
    }
}

const String Slider::getTextFromValue (double v)
{
    if (numDecimalPlaces > 0)
        return String (v, numDecimalPlaces) + textSuffix;
    else
        return String (roundDoubleToInt (v)) + textSuffix;
}

double Slider::getValueFromText (const String& text)
{
    String t (text.trimStart());

    if (t.endsWith (textSuffix))
        t = t.substring (0, t.length() - textSuffix.length());

    while (t.startsWithChar (T('+')))
        t = t.substring (1).trimStart();

    return t.initialSectionContainingOnly (T("0123456789.-"))
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

//==============================================================================
void Slider::labelTextChanged (Label* label)
{
    const double newValue = snapValue (getValueFromText (label->getText()), false);

    if (getValue() != newValue)
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
double Slider::constrainedValue (double value) const throw()
{
    if (interval > 0)
        value = minimum + interval * floor ((value - minimum) / interval + 0.5);

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

    if (style == LinearVertical || style == IncDecButtons)
        sliderPosProportional = 1.0 - sliderPosProportional;

    return (float) (sliderRegionStart + sliderPosProportional * sliderRegionSize);
}

bool Slider::isHorizontal() const throw()
{
    return style == LinearHorizontal
        || style == LinearBar
        || style == TwoValueHorizontal
        || style == ThreeValueHorizontal;
}

bool Slider::isVertical() const throw()
{
    return style == LinearVertical
        || style == TwoValueVertical
        || style == ThreeValueVertical;
}

bool Slider::incDecDragDirectionIsHorizontal() const throw()
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
        jassertfalse // not a valid call on a slider that doesn't work linearly!
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
            const float sliderPos = (float) valueToProportionOfLength (currentValue);
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
                                               getLinearSliderPos (currentValue),
                                               getLinearSliderPos (valueMin),
                                               getLinearSliderPos (valueMax),
                                               style,
                                               *this);
        }

        if (style == LinearBar && valueBox == 0)
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
        minYSpace = 30;

    const int tbw = jmax (0, jmin (textBoxWidth, getWidth() - minXSpace));
    const int tbh = jmax (0, jmin (textBoxHeight, getHeight() - minYSpace));

    if (style == LinearBar)
    {
        if (valueBox != 0)
            valueBox->setBounds (0, 0, getWidth(), getHeight());
    }
    else
    {
        if (textBoxPos == NoTextBox)
        {
            sliderRect.setBounds (0, 0, getWidth(), getHeight());
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

    const int indent = getLookAndFeel().getSliderThumbRadius (*this) + 2;

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
        Rectangle buttonRect (sliderRect);

        if (textBoxPos == TextBoxLeft || textBoxPos == TextBoxRight)
            buttonRect.expand (-2, 0);
        else
            buttonRect.expand (0, -2);

        incDecButtonsSideBySide = buttonRect.getWidth() > buttonRect.getHeight();

        if (incDecButtonsSideBySide)
        {
            decButton->setBounds (buttonRect.getX(),
                                  buttonRect.getY(),
                                  buttonRect.getWidth() / 2,
                                  buttonRect.getHeight());

            decButton->setConnectedEdges (Button::ConnectedOnRight);

            incButton->setBounds (buttonRect.getCentreX(),
                                  buttonRect.getY(),
                                  buttonRect.getWidth() / 2,
                                  buttonRect.getHeight());

            incButton->setConnectedEdges (Button::ConnectedOnLeft);
        }
        else
        {
            incButton->setBounds (buttonRect.getX(),
                                  buttonRect.getY(),
                                  buttonRect.getWidth(),
                                  buttonRect.getHeight() / 2);

            incButton->setConnectedEdges (Button::ConnectedOnBottom);

            decButton->setBounds (buttonRect.getX(),
                                  buttonRect.getCentreY(),
                                  buttonRect.getWidth(),
                                  buttonRect.getHeight() / 2);

            decButton->setConnectedEdges (Button::ConnectedOnTop);
        }
    }
}

void Slider::focusOfChildComponentChanged (FocusChangeType)
{
    repaint();
}

void Slider::mouseDown (const MouseEvent& e)
{
    mouseWasHidden = false;
    incDecDragged = false;

    if (isEnabled())
    {
        if (e.mods.isPopupMenu() && menuEnabled)
        {
            menuShown = true;

            PopupMenu m;
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

            const int r = m.show();

            if (r == 1)
            {
                setVelocityBasedMode (! isVelocityBased);
            }
            else if (r == 2)
            {
                setSliderStyle (Rotary);
            }
            else if (r == 3)
            {
                setSliderStyle (RotaryHorizontalDrag);
            }
            else if (r == 4)
            {
                setSliderStyle (RotaryVerticalDrag);
            }
        }
        else if (maximum > minimum)
        {
            menuShown = false;

            if (valueBox != 0)
                valueBox->hideEditor (true);

            sliderBeingDragged = 0;

            if (style == TwoValueHorizontal
                 || style == TwoValueVertical
                 || style == ThreeValueHorizontal
                 || style == ThreeValueVertical)
            {
                const float mousePos = (float) (isVertical() ? e.y : e.x);

                const float normalPosDistance = fabsf (getLinearSliderPos (currentValue) - mousePos);
                const float minPosDistance = fabsf (getLinearSliderPos (valueMin) - 0.1f - mousePos);
                const float maxPosDistance = fabsf (getLinearSliderPos (valueMax) + 0.1f - mousePos);

                if (style == TwoValueHorizontal || style == TwoValueVertical)
                {
                    if (maxPosDistance <= minPosDistance)
                        sliderBeingDragged = 2;
                    else
                        sliderBeingDragged = 1;
                }
                else if (style == ThreeValueHorizontal || style == ThreeValueVertical)
                {
                    if (normalPosDistance >= minPosDistance && maxPosDistance >= minPosDistance)
                        sliderBeingDragged = 1;
                    else if (normalPosDistance >= maxPosDistance)
                        sliderBeingDragged = 2;
                }
            }

            mouseXWhenLastDragged = e.x;
            mouseYWhenLastDragged = e.y;
            lastAngle = rotaryStart + (rotaryEnd - rotaryStart)
                                        * valueToProportionOfLength (currentValue);

            if (sliderBeingDragged == 2)
                valueWhenLastDragged = valueMax;
            else if (sliderBeingDragged == 1)
                valueWhenLastDragged = valueMin;
            else
                valueWhenLastDragged = currentValue;

            valueOnMouseDown = valueWhenLastDragged;

            if (popupDisplayEnabled)
            {
                SliderPopupDisplayComponent* const popup = new SliderPopupDisplayComponent (this);
                popupDisplay = popup;

                if (parentForPopupDisplay != 0)
                {
                    parentForPopupDisplay->addChildComponent (popup);
                }
                else
                {
                    popup->addToDesktop (0);
                }

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

        if (sendChangeOnlyOnRelease && valueOnMouseDown != currentValue)
            triggerChangeMessage (false);

        sendDragEnd();

        deleteAndZero (popupDisplay);

        if (style == IncDecButtons)
        {
            incButton->setState (Button::buttonNormal);
            decButton->setState (Button::buttonNormal);
        }
    }
}

void Slider::restoreMouseIfHidden()
{
    if (mouseWasHidden)
    {
        mouseWasHidden = false;

        Component* c = Component::getComponentUnderMouse();

        if (c == 0)
            c = this;

        c->enableUnboundedMouseMovement (false);

        int x = getWidth() / 2;
        int y = getHeight() / 2;

        if (isHorizontal())
            x = (int) getLinearSliderPos (currentValue);
        else if (isVertical())
            y = (int) getLinearSliderPos (currentValue);

        relativePositionToGlobal (x, y);
        Desktop::setMousePosition (x, y);
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

static double smallestAngleBetween (double a1, double a2)
{
    return jmin (fabs (a1 - a2),
                 fabs (a1 + double_Pi * 2.0 - a2),
                 fabs (a2 + double_Pi * 2.0 - a1));
}

void Slider::mouseDrag (const MouseEvent& e)
{
    if (isEnabled()
         && (! menuShown)
         && (maximum > minimum))
    {
        if (style == Rotary)
        {
            int dx = e.x - sliderRect.getCentreX();
            int dy = e.y - sliderRect.getCentreY();

            if (dx * dx + dy * dy > 25)
            {
                double angle = atan2 ((double) dx, (double) -dy);
                while (angle < 0.0)
                    angle += double_Pi * 2.0;

                if (rotaryStop && ! e.mouseWasClicked())
                {
                    if (fabs (angle - lastAngle) > double_Pi * 1.5)
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
                        if (smallestAngleBetween (angle, rotaryStart) <= smallestAngleBetween (angle, rotaryEnd))
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
        else
        {
            if (style == LinearBar && e.mouseWasClicked()
                 && valueBox != 0 && valueBox->isEditable())
                return;

            if (style == IncDecButtons)
            {
                if (! incDecDragged)
                    incDecDragged = e.getDistanceFromDragStart() > 10 && ! e.mouseWasClicked();

                if (! incDecDragged)
                    return;
            }

            if (isVelocityBased == e.mods.isAnyModifierKeyDown()
                || ((maximum - minimum) / sliderRegionSize < interval))
            {
                const int mousePos = (isHorizontal() || style == RotaryHorizontalDrag) ? e.x : e.y;

                double scaledMousePos = (mousePos - sliderRegionStart) / (double) sliderRegionSize;

                if (style == RotaryHorizontalDrag || style == RotaryVerticalDrag
                     || style == IncDecButtons)
                {
                    const int mouseDiff = (style == RotaryHorizontalDrag
                                             || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                            ? e.getDistanceFromDragStartX()
                                            : -e.getDistanceFromDragStartY();

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
                    if (style == LinearVertical)
                        scaledMousePos = 1.0 - scaledMousePos;

                    valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, scaledMousePos));
                }
            }
            else
            {
                const int mouseDiff = (isHorizontal() || style == RotaryHorizontalDrag
                                         || (style == IncDecButtons && incDecDragDirectionIsHorizontal()))
                                        ? e.x - mouseXWhenLastDragged
                                        : e.y - mouseYWhenLastDragged;

                const double maxSpeed = jmax (200, sliderRegionSize);
                double speed = jlimit (0.0, maxSpeed, (double) abs (mouseDiff));

                if (speed != 0)
                {
                    speed = 0.2 * velocityModeSensitivity
                              * (1.0 + sin (double_Pi * (1.5 + jmin (0.5, velocityModeOffset
                                                                            + jmax (0.0, (double) (speed - velocityModeThreshold))
                                                                                / maxSpeed))));

                    if (mouseDiff < 0)
                        speed = -speed;

                    if (style == LinearVertical || style == RotaryVerticalDrag
                         || (style == IncDecButtons && ! incDecDragDirectionIsHorizontal()))
                        speed = -speed;

                    const double currentPos = valueToProportionOfLength (valueWhenLastDragged);

                    valueWhenLastDragged = proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + speed));

                    e.originalComponent->enableUnboundedMouseMovement (true, false);
                    mouseWasHidden = true;
                }
            }
        }

        valueWhenLastDragged = jlimit (minimum, maximum, valueWhenLastDragged);

        if (sliderBeingDragged == 0)
        {
            setValue (snapValue (valueWhenLastDragged, true),
                      ! sendChangeOnlyOnRelease, false);
        }
        else if (sliderBeingDragged == 1)
        {
            setMinValue (snapValue (valueWhenLastDragged, true),
                          ! sendChangeOnlyOnRelease, false);
        }
        else
        {
            jassert (sliderBeingDragged == 2);

            setMaxValue (snapValue (valueWhenLastDragged, true),
                          ! sendChangeOnlyOnRelease, false);
        }

        mouseXWhenLastDragged = e.x;
        mouseYWhenLastDragged = e.y;
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

void Slider::mouseWheelMove (const MouseEvent&, float wheelIncrementX, float wheelIncrementY)
{
    if (isEnabled()
         && (maximum > minimum)
         && ! isMouseButtonDownAnywhere())
    {
        if (valueBox != 0)
            valueBox->hideEditor (false);

        const double proportionDelta = (wheelIncrementX != 0 ? -wheelIncrementX : wheelIncrementY) * 0.15f;
        const double currentPos = valueToProportionOfLength (currentValue);
        const double newValue = proportionOfLengthToValue (jlimit (0.0, 1.0, currentPos + proportionDelta));

        double delta = (newValue != currentValue)
                        ? jmax (fabs (newValue - currentValue), interval) : 0;

        if (currentValue > newValue)
            delta = -delta;

        sendDragStart();
        setValue (snapValue (currentValue + delta, false), true, true);
        sendDragEnd();
    }
}

void SliderListener::sliderDragStarted (Slider*)
{
}

void SliderListener::sliderDragEnded (Slider*)
{
}


END_JUCE_NAMESPACE
