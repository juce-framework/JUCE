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

struct AudioProcessorValueTreeState::Parameter   : public AudioProcessorParameterWithID,
                                                   private ValueTree::Listener
{
    Parameter (AudioProcessorValueTreeState& s,
               const String& parameterID, const String& paramName, const String& labelText,
               NormalisableRange<float> r, float defaultVal,
               std::function<String (float)> valueToText,
               std::function<float (const String&)> textToValue,
               bool meta,
               bool automatable,
               bool discrete,
               AudioProcessorParameter::Category category,
               bool boolean)
        : AudioProcessorParameterWithID (parameterID, paramName, labelText, category),
          owner (s), valueToTextFunction (valueToText), textToValueFunction (textToValue),
          range (r), value (defaultVal), defaultValue (defaultVal),
          listenersNeedCalling (true),
          isMetaParam (meta),
          isAutomatableParam (automatable),
          isDiscreteParam (discrete),
          isBooleanParam (boolean)
    {
        value = defaultValue;
        state.addListener (this);
    }

    ~Parameter()
    {
        // should have detached all callbacks before destroying the parameters!
        jassert (listeners.size() <= 1);
    }

    float getValue() const override                             { return range.convertTo0to1 (value); }
    float getDefaultValue() const override                      { return range.convertTo0to1 (defaultValue); }

    float getValueForText (const String& text) const override
    {
        return range.convertTo0to1 (textToValueFunction != nullptr ? textToValueFunction (text)
                                                                   : text.getFloatValue());
    }

    String getText (float normalisedValue, int length) const override
    {
        auto v = range.convertFrom0to1 (normalisedValue);
        return valueToTextFunction != nullptr ? valueToTextFunction (v)
                                              : AudioProcessorParameter::getText (v, length);
    }

    int getNumSteps() const override
    {
        if (range.interval > 0)
            return (static_cast<int> ((range.end - range.start) / range.interval) + 1);

        return AudioProcessor::getDefaultNumParameterSteps();
    }

    void setValue (float newValue) override
    {
        newValue = range.snapToLegalValue (range.convertFrom0to1 (newValue));

        if (value != newValue || listenersNeedCalling)
        {
            value = newValue;

            listeners.call ([=] (AudioProcessorValueTreeState::Listener& l) { l.parameterChanged (paramID, value); });
            listenersNeedCalling = false;

            needsUpdate = true;
        }
    }

    void setNewState (const ValueTree& v)
    {
        state = v;
        updateFromValueTree();
    }

    void setUnnormalisedValue (float newUnnormalisedValue)
    {
        if (value != newUnnormalisedValue)
        {
            const float newValue = range.convertTo0to1 (newUnnormalisedValue);
            setValueNotifyingHost (newValue);
        }
    }

    void updateFromValueTree()
    {
        setUnnormalisedValue (state.getProperty (owner.valuePropertyID, defaultValue));
    }

    void copyValueToValueTree()
    {
        if (auto* valueProperty = state.getPropertyPointer (owner.valuePropertyID))
        {
            if ((float) *valueProperty != value)
            {
                ScopedValueSetter<bool> svs (ignoreParameterChangedCallbacks, true);
                state.setProperty (owner.valuePropertyID, value, owner.undoManager);
            }
        }
        else
        {
            state.setProperty (owner.valuePropertyID, value, nullptr);
        }
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (ignoreParameterChangedCallbacks)
            return;

        if (property == owner.valuePropertyID)
            updateFromValueTree();
    }

    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    static Parameter* getParameterForID (AudioProcessor& processor, StringRef paramID) noexcept
    {
        for (auto* ap : processor.getParameters())
        {
            // When using this class, you must allow it to manage all the parameters in your AudioProcessor, and
            // not add any parameter objects of other types!
            jassert (dynamic_cast<Parameter*> (ap) != nullptr);

            auto* p = static_cast<Parameter*> (ap);

            if (paramID == p->paramID)
                return p;
        }

        return nullptr;
    }

    bool isMetaParameter() const override      { return isMetaParam; }
    bool isAutomatable() const override        { return isAutomatableParam; }
    bool isDiscrete() const override           { return isDiscreteParam; }
    bool isBoolean() const override            { return isBooleanParam; }

    AudioProcessorValueTreeState& owner;
    ValueTree state;
    ListenerList<AudioProcessorValueTreeState::Listener> listeners;
    std::function<String (float)> valueToTextFunction;
    std::function<float (const String&)> textToValueFunction;
    NormalisableRange<float> range;
    float value, defaultValue;
    std::atomic<bool> needsUpdate { true };
    bool listenersNeedCalling;
    const bool isMetaParam, isAutomatableParam, isDiscreteParam, isBooleanParam;
    bool ignoreParameterChangedCallbacks = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parameter)
};

//==============================================================================
AudioProcessorValueTreeState::AudioProcessorValueTreeState (AudioProcessor& p, UndoManager* um)
    : processor (p), undoManager (um)
{
    startTimerHz (10);
    state.addListener (this);
}

AudioProcessorValueTreeState::~AudioProcessorValueTreeState() {}

AudioProcessorParameterWithID* AudioProcessorValueTreeState::createAndAddParameter (const String& paramID, const String& paramName,
                                                                                    const String& labelText, NormalisableRange<float> r,
                                                                                    float defaultVal, std::function<String (float)> valueToTextFunction,
                                                                                    std::function<float (const String&)> textToValueFunction,
                                                                                    bool isMetaParameter,
                                                                                    bool isAutomatableParameter,
                                                                                    bool isDiscreteParameter,
                                                                                    AudioProcessorParameter::Category category,
                                                                                    bool isBooleanParameter)
{
    // All parameters must be created before giving this manager a ValueTree state!
    jassert (! state.isValid());

    Parameter* p = new Parameter (*this, paramID, paramName, labelText, r,
                                  defaultVal, valueToTextFunction, textToValueFunction,
                                  isMetaParameter, isAutomatableParameter,
                                  isDiscreteParameter, category, isBooleanParameter);
    processor.addParameter (p);
    return p;
}

void AudioProcessorValueTreeState::addParameterListener (StringRef paramID, Listener* listener)
{
    if (auto* p = Parameter::getParameterForID (processor, paramID))
        p->listeners.add (listener);
}

void AudioProcessorValueTreeState::removeParameterListener (StringRef paramID, Listener* listener)
{
    if (auto* p = Parameter::getParameterForID (processor, paramID))
        p->listeners.remove (listener);
}

Value AudioProcessorValueTreeState::getParameterAsValue (StringRef paramID) const
{
    if (auto* p = Parameter::getParameterForID (processor, paramID))
        return p->state.getPropertyAsValue (valuePropertyID, undoManager);

    return {};
}

NormalisableRange<float> AudioProcessorValueTreeState::getParameterRange (StringRef paramID) const noexcept
{
    if (auto* p = Parameter::getParameterForID (processor, paramID))
        return p->range;

    return NormalisableRange<float>();
}

AudioProcessorParameterWithID* AudioProcessorValueTreeState::getParameter (StringRef paramID) const noexcept
{
    return Parameter::getParameterForID (processor, paramID);
}

float* AudioProcessorValueTreeState::getRawParameterValue (StringRef paramID) const noexcept
{
    if (auto* p = Parameter::getParameterForID (processor, paramID))
        return &(p->value);

    return nullptr;
}

ValueTree AudioProcessorValueTreeState::copyState()
{
    ScopedLock lock (valueTreeChanging);

    flushParameterValuesToValueTree();

    return state.createCopy();
}

void AudioProcessorValueTreeState::replaceState (const ValueTree& newState)
{
    ScopedLock lock (valueTreeChanging);

    state = newState;

    if (undoManager != nullptr)
        undoManager->clearUndoHistory();
}

ValueTree AudioProcessorValueTreeState::getOrCreateChildValueTree (const String& paramID)
{
    ValueTree v (state.getChildWithProperty (idPropertyID, paramID));

    if (! v.isValid())
    {
        v = ValueTree (valueType);
        v.setProperty (idPropertyID, paramID, nullptr);
        state.appendChild (v, nullptr);
    }

    return v;
}

void AudioProcessorValueTreeState::updateParameterConnectionsToChildTrees()
{
    ScopedLock lock (valueTreeChanging);

    for (auto* param : processor.getParameters())
    {
        jassert (dynamic_cast<Parameter*> (param) != nullptr);
        auto* p = static_cast<Parameter*> (param);

        p->setNewState (getOrCreateChildValueTree (p->paramID));
    }
}

void AudioProcessorValueTreeState::valueTreePropertyChanged (ValueTree& tree, const Identifier& property)
{
    if (property == idPropertyID && tree.hasType (valueType) && tree.getParent() == state)
        updateParameterConnectionsToChildTrees();
}

void AudioProcessorValueTreeState::valueTreeChildAdded (ValueTree& parent, ValueTree& tree)
{
    if (parent == state && tree.hasType (valueType))
        if (auto* param = Parameter::getParameterForID (processor, tree.getProperty (idPropertyID).toString()))
            param->setNewState (getOrCreateChildValueTree (param->paramID));
}

void AudioProcessorValueTreeState::valueTreeChildRemoved (ValueTree& parent, ValueTree& tree, int)
{
    if (parent == state && tree.hasType (valueType))
        if (auto* param = Parameter::getParameterForID (processor, tree.getProperty (idPropertyID).toString()))
            param->setNewState (getOrCreateChildValueTree (param->paramID));
}

void AudioProcessorValueTreeState::valueTreeRedirected (ValueTree& v)
{
    if (v == state)
        updateParameterConnectionsToChildTrees();
}

void AudioProcessorValueTreeState::valueTreeChildOrderChanged (ValueTree&, int, int) {}
void AudioProcessorValueTreeState::valueTreeParentChanged (ValueTree&) {}

bool AudioProcessorValueTreeState::flushParameterValuesToValueTree()
{
    ScopedLock lock (valueTreeChanging);

    bool anythingUpdated = false;

    for (auto* ap : processor.getParameters())
    {
        jassert (dynamic_cast<Parameter*> (ap) != nullptr);
        auto* p = static_cast<Parameter*> (ap);

        bool needsUpdateTestValue = true;

        if (p->needsUpdate.compare_exchange_strong (needsUpdateTestValue, false))
        {
            p->copyValueToValueTree();
            anythingUpdated = true;
        }
    }

    return anythingUpdated;
}

void AudioProcessorValueTreeState::timerCallback()
{
    auto anythingUpdated = flushParameterValuesToValueTree();

    startTimer (anythingUpdated ? 1000 / 50
                                : jlimit (50, 500, getTimerInterval() + 20));
}

AudioProcessorValueTreeState::Listener::Listener() {}
AudioProcessorValueTreeState::Listener::~Listener() {}

//==============================================================================
struct AttachedControlBase  : public AudioProcessorValueTreeState::Listener,
                              public AsyncUpdater
{
    AttachedControlBase (AudioProcessorValueTreeState& s, const String& p)
        : state (s), paramID (p), lastValue (0)
    {
        state.addParameterListener (paramID, this);
    }

    void removeListener()
    {
        state.removeParameterListener (paramID, this);
    }

    void setNewUnnormalisedValue (float newUnnormalisedValue)
    {
        if (auto* p = state.getParameter (paramID))
        {
            const float newValue = state.getParameterRange (paramID)
                                        .convertTo0to1 (newUnnormalisedValue);

            if (p->getValue() != newValue)
                p->setValueNotifyingHost (newValue);
        }
    }

    void sendInitialUpdate()
    {
        if (auto* v = state.getRawParameterValue (paramID))
            parameterChanged (paramID, *v);
    }

    void parameterChanged (const String&, float newValue) override
    {
        lastValue = newValue;

        if (MessageManager::getInstance()->isThisTheMessageThread())
        {
            cancelPendingUpdate();
            setValue (newValue);
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void beginParameterChange()
    {
        if (auto* p = state.getParameter (paramID))
        {
            if (state.undoManager != nullptr)
                state.undoManager->beginNewTransaction();

            p->beginChangeGesture();
        }
    }

    void endParameterChange()
    {
        if (AudioProcessorParameter* p = state.getParameter (paramID))
            p->endChangeGesture();
    }

    void handleAsyncUpdate() override
    {
        setValue (lastValue);
    }

    virtual void setValue (float) = 0;

    AudioProcessorValueTreeState& state;
    String paramID;
    float lastValue;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AttachedControlBase)
};

//==============================================================================
struct AudioProcessorValueTreeState::SliderAttachment::Pimpl  : private AttachedControlBase,
                                                                private Slider::Listener
{
    Pimpl (AudioProcessorValueTreeState& s, const String& p, Slider& sl)
        : AttachedControlBase (s, p), slider (sl), ignoreCallbacks (false)
    {
        NormalisableRange<float> range (state.getParameterRange (paramID));

        if (range.interval != 0 || range.skew != 0)
        {
            slider.setRange (range.start, range.end, range.interval);
            slider.setSkewFactor (range.skew, range.symmetricSkew);
        }
        else
        {
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
                                                     double valueToSnap) mutable
            {
                range.start = (float) currentRangeStart;
                range.end = (float) currentRangeEnd;
                return (double) range.snapToLegalValue ((float) valueToSnap);
            };

            slider.setNormalisableRange ({ (double) range.start, (double) range.end,
                                           convertFrom0To1Function,
                                           convertTo0To1Function,
                                           snapToLegalValueFunction });
        }

        if (auto* param = dynamic_cast<AudioProcessorValueTreeState::Parameter*> (state.getParameter (paramID)))
        {
            if (param->textToValueFunction != nullptr)
                slider.valueFromTextFunction = [param] (const String& text) { return (double) param->textToValueFunction (text); };

            if (param->valueToTextFunction != nullptr)
                slider.textFromValueFunction = [param] (double value)       { return param->valueToTextFunction ((float) value); };

            slider.setDoubleClickReturnValue (true, range.convertFrom0to1 (param->getDefaultValue()));
        }

        sendInitialUpdate();
        slider.addListener (this);
    }

    ~Pimpl()
    {
        slider.removeListener (this);
        removeListener();
    }

    void setValue (float newValue) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        {
            ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            slider.setValue (newValue, sendNotificationSync);
        }
    }

    void sliderValueChanged (Slider* s) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if ((! ignoreCallbacks) && (! ModifierKeys::currentModifiers.isRightButtonDown()))
            setNewUnnormalisedValue ((float) s->getValue());
    }

    void sliderDragStarted (Slider*) override { beginParameterChange(); }
    void sliderDragEnded   (Slider*) override { endParameterChange();   }

    Slider& slider;
    bool ignoreCallbacks;
    CriticalSection selfCallbackMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

AudioProcessorValueTreeState::SliderAttachment::SliderAttachment (AudioProcessorValueTreeState& s, const String& p, Slider& sl)
    : pimpl (new Pimpl (s, p, sl))
{
}

AudioProcessorValueTreeState::SliderAttachment::~SliderAttachment() {}

//==============================================================================
struct AudioProcessorValueTreeState::ComboBoxAttachment::Pimpl  : private AttachedControlBase,
                                                                  private ComboBox::Listener
{
    Pimpl (AudioProcessorValueTreeState& s, const String& p, ComboBox& c)
        : AttachedControlBase (s, p), combo (c), ignoreCallbacks (false)
    {
        sendInitialUpdate();
        combo.addListener (this);
    }

    ~Pimpl()
    {
        combo.removeListener (this);
        removeListener();
    }

    void setValue (float newValue) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (state.getParameter (paramID) != nullptr)
        {
            auto normValue = state.getParameterRange (paramID)
                                  .convertTo0to1 (newValue);
            auto index = roundToInt (normValue * (combo.getNumItems() - 1));

            if (index != combo.getSelectedItemIndex())
            {
                ScopedValueSetter<bool> svs (ignoreCallbacks, true);
                combo.setSelectedItemIndex (index, sendNotificationSync);
            }
        }
    }

    void comboBoxChanged (ComboBox*) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            if (auto* p = state.getParameter (paramID))
            {
                auto newValue = (float) combo.getSelectedItemIndex() / (combo.getNumItems() - 1);

                if (p->getValue() != newValue)
                {
                    beginParameterChange();
                    p->setValueNotifyingHost (newValue);
                    endParameterChange();
                }
            }
        }
    }

    ComboBox& combo;
    bool ignoreCallbacks;
    CriticalSection selfCallbackMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

AudioProcessorValueTreeState::ComboBoxAttachment::ComboBoxAttachment (AudioProcessorValueTreeState& s, const String& p, ComboBox& c)
    : pimpl (new Pimpl (s, p, c))
{
}

AudioProcessorValueTreeState::ComboBoxAttachment::~ComboBoxAttachment() {}

//==============================================================================
struct AudioProcessorValueTreeState::ButtonAttachment::Pimpl  : private AttachedControlBase,
                                                                private Button::Listener
{
    Pimpl (AudioProcessorValueTreeState& s, const String& p, Button& b)
        : AttachedControlBase (s, p), button (b), ignoreCallbacks (false)
    {
        sendInitialUpdate();
        button.addListener (this);
    }

    ~Pimpl()
    {
        button.removeListener (this);
        removeListener();
    }

    void setValue (float newValue) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        {
            ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            button.setToggleState (newValue >= 0.5f, sendNotificationSync);
        }
    }

    void buttonClicked (Button* b) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            beginParameterChange();
            setNewUnnormalisedValue (b->getToggleState() ? 1.0f : 0.0f);
            endParameterChange();
        }
    }

    Button& button;
    bool ignoreCallbacks;
    CriticalSection selfCallbackMutex;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pimpl)
};

AudioProcessorValueTreeState::ButtonAttachment::ButtonAttachment (AudioProcessorValueTreeState& s, const String& p, Button& b)
    : pimpl (new Pimpl (s, p, b))
{
}

AudioProcessorValueTreeState::ButtonAttachment::~ButtonAttachment() {}

} // namespace juce
