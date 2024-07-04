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

ParameterAttachment::ParameterAttachment (RangedAudioParameter& param,
                                          std::function<void (float)> parameterChangedCallback,
                                          UndoManager* um)
    : parameter (param),
      undoManager (um),
      setValue (std::move (parameterChangedCallback))
{
    parameter.addListener (this);
}

ParameterAttachment::~ParameterAttachment()
{
    parameter.removeListener (this);
    cancelPendingUpdate();
}

void ParameterAttachment::sendInitialUpdate()
{
    parameterValueChanged ({}, parameter.getValue());
}

void ParameterAttachment::setValueAsCompleteGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        beginGesture();
        parameter.setValueNotifyingHost (f);
        endGesture();
    });
}

void ParameterAttachment::beginGesture()
{
    if (undoManager != nullptr)
        undoManager->beginNewTransaction();

    parameter.beginChangeGesture();
}

void ParameterAttachment::setValueAsPartOfGesture (float newDenormalisedValue)
{
    callIfParameterValueChanged (newDenormalisedValue, [this] (float f)
    {
        parameter.setValueNotifyingHost (f);
    });
}

void ParameterAttachment::endGesture()
{
    parameter.endChangeGesture();
}

template <typename Callback>
void ParameterAttachment::callIfParameterValueChanged (float newDenormalisedValue,
                                                       Callback&& callback)
{
    const auto newValue = normalise (newDenormalisedValue);

    if (! approximatelyEqual (parameter.getValue(), newValue))
        callback (newValue);
}

void ParameterAttachment::parameterValueChanged (int, float newValue)
{
    lastValue = newValue;

    if (MessageManager::getInstance()->isThisTheMessageThread())
    {
        cancelPendingUpdate();
        handleAsyncUpdate();
    }
    else
    {
        triggerAsyncUpdate();
    }
}

void ParameterAttachment::handleAsyncUpdate()
{
    NullCheckedInvocation::invoke (setValue, parameter.convertFrom0to1 (lastValue));
}

//==============================================================================
SliderParameterAttachment::SliderParameterAttachment (RangedAudioParameter& param,
                                                      Slider& s,
                                                      UndoManager* um)
    : slider (s),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    slider.valueFromTextFunction = [&param] (const String& text) { return (double) param.convertFrom0to1 (param.getValueForText (text)); };
    slider.textFromValueFunction = [&param] (double value) { return param.getText (param.convertTo0to1 ((float) value), 0); };
    slider.setDoubleClickReturnValue (true, param.convertFrom0to1 (param.getDefaultValue()));

    auto range = param.getNormalisableRange();

    auto convertFrom0To1Function = [range] (double currentRangeStart,
                                            double currentRangeEnd,
                                            double normalisedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.convertFrom0to1 ((float) normalisedValue);
    };

    auto convertTo0To1Function = [range] (double currentRangeStart,
                                          double currentRangeEnd,
                                          double mappedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.convertTo0to1 ((float) mappedValue);
    };

    auto snapToLegalValueFunction = [range] (double currentRangeStart,
                                             double currentRangeEnd,
                                             double mappedValue) mutable
    {
        range.start = (float) currentRangeStart;
        range.end = (float) currentRangeEnd;
        return (double) range.snapToLegalValue ((float) mappedValue);
    };

    NormalisableRange<double> newRange { (double) range.start,
                                         (double) range.end,
                                         std::move (convertFrom0To1Function),
                                         std::move (convertTo0To1Function),
                                         std::move (snapToLegalValueFunction) };
    newRange.interval = range.interval;
    newRange.skew = range.skew;
    newRange.symmetricSkew = range.symmetricSkew;

    slider.setNormalisableRange (newRange);

    sendInitialUpdate();
    slider.valueChanged();
    slider.addListener (this);
}

SliderParameterAttachment::~SliderParameterAttachment()
{
    slider.removeListener (this);
}

void SliderParameterAttachment::sendInitialUpdate() { attachment.sendInitialUpdate(); }

void SliderParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    slider.setValue (newValue, sendNotificationSync);
}

void SliderParameterAttachment::sliderValueChanged (Slider*)
{
    if (! ignoreCallbacks)
        attachment.setValueAsPartOfGesture ((float) slider.getValue());
}

//==============================================================================
ComboBoxParameterAttachment::ComboBoxParameterAttachment (RangedAudioParameter& param,
                                                          ComboBox& c,
                                                          UndoManager* um)
    : comboBox (c),
      storedParameter (param),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    sendInitialUpdate();
    comboBox.addListener (this);
}

ComboBoxParameterAttachment::~ComboBoxParameterAttachment()
{
    comboBox.removeListener (this);
}

void ComboBoxParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

void ComboBoxParameterAttachment::setValue (float newValue)
{
    const auto normValue = storedParameter.convertTo0to1 (newValue);
    const auto index = roundToInt (normValue * (float) (comboBox.getNumItems() - 1));

    if (index == comboBox.getSelectedItemIndex())
        return;

    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    comboBox.setSelectedItemIndex (index, sendNotificationSync);
}

void ComboBoxParameterAttachment::comboBoxChanged (ComboBox*)
{
    if (ignoreCallbacks)
        return;

    const auto numItems = comboBox.getNumItems();
    const auto selected = (float) comboBox.getSelectedItemIndex();
    const auto newValue = numItems > 1 ? selected / (float) (numItems - 1)
                                       : 0.0f;

    attachment.setValueAsCompleteGesture (storedParameter.convertFrom0to1 (newValue));
}

//==============================================================================
ButtonParameterAttachment::ButtonParameterAttachment (RangedAudioParameter& param,
                                                      Button& b,
                                                      UndoManager* um)
    : button (b),
      attachment (param, [this] (float f) { setValue (f); }, um)
{
    sendInitialUpdate();
    button.addListener (this);
}

ButtonParameterAttachment::~ButtonParameterAttachment()
{
    button.removeListener (this);
}

void ButtonParameterAttachment::sendInitialUpdate()
{
    attachment.sendInitialUpdate();
}

void ButtonParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    button.setToggleState (newValue >= 0.5f, sendNotificationSync);
}

void ButtonParameterAttachment::buttonClicked (Button*)
{
    if (ignoreCallbacks)
        return;

    attachment.setValueAsCompleteGesture (button.getToggleState() ? 1.0f : 0.0f);
}

//==============================================================================
#if JUCE_WEB_BROWSER
WebSliderParameterAttachment::WebSliderParameterAttachment (RangedAudioParameter& parameterIn,
                                                            WebSliderRelay& sliderStateIn,
                                                            UndoManager* undoManager)
    : sliderState (sliderStateIn),
      parameter (parameterIn),
      attachment (parameter, [this] (float newValue) { setValue (newValue); }, undoManager)
{
    sendInitialUpdate();
    sliderState.addListener (this);
}

WebSliderParameterAttachment::~WebSliderParameterAttachment()
{
    sliderState.removeListener (this);
}

void WebSliderParameterAttachment::sendInitialUpdate()
{
    const auto range = parameter.getNormalisableRange();
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("start", range.start);
    object->setProperty ("end", range.end);
    object->setProperty ("skew", range.skew);
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("label", parameter.getLabel());

    // We use the NormalisableRange defined num steps even for an AudioParameterFloat.
    const auto numSteps = range.interval > 0
                        ? static_cast<int> ((range.end - range.start) / range.interval) + 1
                        : AudioProcessor::getDefaultNumParameterSteps();

    object->setProperty ("numSteps", numSteps);
    object->setProperty ("interval", range.interval);
    object->setProperty ("parameterIndex", parameter.getParameterIndex());
    sliderState.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

void WebSliderParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    sliderState.setValue (newValue);
}

void WebSliderParameterAttachment::sliderValueChanged (WebSliderRelay* slider)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsPartOfGesture (slider->getValue());
}

//==============================================================================
WebToggleButtonParameterAttachment::WebToggleButtonParameterAttachment (RangedAudioParameter& parameterIn,
                                                                        WebToggleButtonRelay& button,
                                                                        UndoManager* undoManager)
    : relay (button),
      parameter (parameterIn),
      attachment (parameter, [this] (float f) { setValue (f); }, undoManager)
{
    sendInitialUpdate();
    relay.addListener (this);
}

WebToggleButtonParameterAttachment::~WebToggleButtonParameterAttachment()
{
    relay.removeListener (this);
}

void WebToggleButtonParameterAttachment::sendInitialUpdate()
{
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("parameterIndex", parameter.getParameterIndex());
    relay.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

void WebToggleButtonParameterAttachment::setValue (float newValue)
{
    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    relay.setToggleState (newValue >= 0.5f);
}

void WebToggleButtonParameterAttachment::toggleStateChanged (bool newValue)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsCompleteGesture (newValue ? 1.0f : 0.0f);
}

void WebToggleButtonParameterAttachment::initialUpdateRequested()
{
    sendInitialUpdate();
}

//==============================================================================
WebComboBoxParameterAttachment::WebComboBoxParameterAttachment (RangedAudioParameter& parameterIn,
                                                                WebComboBoxRelay& combo,
                                                                UndoManager* undoManager)
    : relay (combo),
      parameter (parameterIn),
      attachment (parameter, [this] (float f) { setValue (f); }, undoManager)
{
    sendInitialUpdate();
    relay.addListener (this);
}

WebComboBoxParameterAttachment::~WebComboBoxParameterAttachment()
{
    relay.removeListener (this);
}

void WebComboBoxParameterAttachment::sendInitialUpdate()
{
    DynamicObject::Ptr object { new DynamicObject };
    object->setProperty (detail::WebSliderRelayEvents::Event::eventTypeKey, "propertiesChanged");
    object->setProperty ("name", parameter.getName (100));
    object->setProperty ("parameterIndex", parameter.getParameterIndex());

    if (auto* choiceParameter = dynamic_cast<AudioParameterChoice*> (&parameter))
        object->setProperty ("choices", choiceParameter->choices);
    else
        object->setProperty ("choices", StringArray{});

    relay.emitEvent (object.get());
    attachment.sendInitialUpdate();
}

void WebComboBoxParameterAttachment::setValue (float newValue)
{
    const auto normValue = parameter.convertTo0to1 (newValue);

    const ScopedValueSetter<bool> svs (ignoreCallbacks, true);
    relay.setValue (normValue);
}

void WebComboBoxParameterAttachment::valueChanged (float newValue)
{
    if (ignoreCallbacks)
    {
        jassertfalse;
        return;
    }

    attachment.setValueAsCompleteGesture (parameter.convertFrom0to1 (newValue));
}

void WebComboBoxParameterAttachment::initialUpdateRequested()
{
    sendInitialUpdate();
}
#endif

} // namespace juce
