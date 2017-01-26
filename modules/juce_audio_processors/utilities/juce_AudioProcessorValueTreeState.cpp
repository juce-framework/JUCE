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

#if JUCE_COMPILER_SUPPORTS_LAMBDAS

//==============================================================================
struct AudioProcessorValueTreeState::Parameter   : public AudioProcessorParameterWithID,
                                                   private ValueTree::Listener
{
    Parameter (AudioProcessorValueTreeState& s,
               const String& parameterID, const String& paramName, const String& labelText,
               NormalisableRange<float> r, float defaultVal,
               std::function<String (float)> valueToText,
               std::function<float (const String&)> textToValue)
        : AudioProcessorParameterWithID (parameterID, paramName, labelText),
          owner (s), valueToTextFunction (valueToText), textToValueFunction (textToValue),
          range (r), value (defaultVal), defaultValue (defaultVal),
          listenersNeedCalling (true)
    {
        state.addListener (this);
        needsUpdate.set (1);
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

    String getText (float v, int length) const override
    {
        return valueToTextFunction != nullptr ? valueToTextFunction (range.convertFrom0to1 (v))
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

            listeners.call (&AudioProcessorValueTreeState::Listener::parameterChanged, paramID, value);
            listenersNeedCalling = false;

            needsUpdate.set (1);
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
        if (state.isValid())
            state.setPropertyExcludingListener (this, owner.valuePropertyID, value, owner.undoManager);
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier& property) override
    {
        if (property == owner.valuePropertyID)
            updateFromValueTree();
    }

    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    static Parameter* getParameterForID (AudioProcessor& processor, StringRef paramID) noexcept
    {
        const int numParams = processor.getParameters().size();

        for (int i = 0; i < numParams; ++i)
        {
            AudioProcessorParameter* const ap = processor.getParameters().getUnchecked(i);

            // When using this class, you must allow it to manage all the parameters in your AudioProcessor, and
            // not add any parameter objects of other types!
            jassert (dynamic_cast<Parameter*> (ap) != nullptr);

            Parameter* const p = static_cast<Parameter*> (ap);

            if (paramID == p->paramID)
                return p;
        }

        return nullptr;
    }

    AudioProcessorValueTreeState& owner;
    ValueTree state;
    ListenerList<AudioProcessorValueTreeState::Listener> listeners;
    std::function<String (float)> valueToTextFunction;
    std::function<float (const String&)> textToValueFunction;
    NormalisableRange<float> range;
    float value, defaultValue;
    Atomic<int> needsUpdate;
    bool listenersNeedCalling;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Parameter)
};

//==============================================================================
AudioProcessorValueTreeState::AudioProcessorValueTreeState (AudioProcessor& p, UndoManager* um)
    : processor (p),
      undoManager (um),
      valueType ("PARAM"),
      valuePropertyID ("value"),
      idPropertyID ("id"),
      updatingConnections (false)
{
    startTimerHz (10);
    state.addListener (this);
}

AudioProcessorValueTreeState::~AudioProcessorValueTreeState() {}

AudioProcessorParameter* AudioProcessorValueTreeState::createAndAddParameter (const String& paramID, const String& paramName,
                                                                              const String& labelText, NormalisableRange<float> r,
                                                                              float defaultVal, std::function<String (float)> valueToTextFunction,
                                                                              std::function<float (const String&)> textToValueFunction)
{
    // All parameters must be created before giving this manager a ValueTree state!
    jassert (! state.isValid());
   #if ! JUCE_LINUX
    jassert (MessageManager::getInstance()->isThisTheMessageThread());
   #endif

    Parameter* p = new Parameter (*this, paramID, paramName, labelText, r,
                                  defaultVal, valueToTextFunction, textToValueFunction);
    processor.addParameter (p);
    return p;
}

void AudioProcessorValueTreeState::addParameterListener (StringRef paramID, Listener* listener)
{
    if (Parameter* p = Parameter::getParameterForID (processor, paramID))
        p->listeners.add (listener);
}

void AudioProcessorValueTreeState::removeParameterListener (StringRef paramID, Listener* listener)
{
    if (Parameter* p = Parameter::getParameterForID (processor, paramID))
        p->listeners.remove (listener);
}

Value AudioProcessorValueTreeState::getParameterAsValue (StringRef paramID) const
{
    if (Parameter* p = Parameter::getParameterForID (processor, paramID))
        return p->state.getPropertyAsValue (valuePropertyID, undoManager);

    return Value();
}

NormalisableRange<float> AudioProcessorValueTreeState::getParameterRange (StringRef paramID) const noexcept
{
    if (Parameter* p = Parameter::getParameterForID (processor, paramID))
        return p->range;

    return NormalisableRange<float>();
}

AudioProcessorParameter* AudioProcessorValueTreeState::getParameter (StringRef paramID) const noexcept
{
    return Parameter::getParameterForID (processor, paramID);
}

float* AudioProcessorValueTreeState::getRawParameterValue (StringRef paramID) const noexcept
{
    if (Parameter* p = Parameter::getParameterForID (processor, paramID))
        return &(p->value);

    return nullptr;
}

ValueTree AudioProcessorValueTreeState::getOrCreateChildValueTree (const String& paramID)
{
    ValueTree v (state.getChildWithProperty (idPropertyID, paramID));

    if (! v.isValid())
    {
        v = ValueTree (valueType);
        v.setProperty (idPropertyID, paramID, undoManager);
        state.addChild (v, -1, undoManager);
    }

    return v;
}

void AudioProcessorValueTreeState::updateParameterConnectionsToChildTrees()
{
    if (! updatingConnections)
    {
        ScopedValueSetter<bool> svs (updatingConnections, true, false);

        const int numParams = processor.getParameters().size();

        for (int i = 0; i < numParams; ++i)
        {
            AudioProcessorParameter* const ap = processor.getParameters().getUnchecked(i);
            jassert (dynamic_cast<Parameter*> (ap) != nullptr);

            Parameter* p = static_cast<Parameter*> (ap);
            p->setNewState (getOrCreateChildValueTree (p->paramID));
        }
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
        updateParameterConnectionsToChildTrees();
}

void AudioProcessorValueTreeState::valueTreeChildRemoved (ValueTree& parent, ValueTree& tree, int)
{
    if (parent == state && tree.hasType (valueType))
        updateParameterConnectionsToChildTrees();
}

void AudioProcessorValueTreeState::valueTreeRedirected (ValueTree& v)
{
    if (v == state)
        updateParameterConnectionsToChildTrees();
}

void AudioProcessorValueTreeState::valueTreeChildOrderChanged (ValueTree&, int, int) {}
void AudioProcessorValueTreeState::valueTreeParentChanged (ValueTree&) {}

void AudioProcessorValueTreeState::timerCallback()
{
    const int numParams = processor.getParameters().size();
    bool anythingUpdated = false;

    for (int i = 0; i < numParams; ++i)
    {
        AudioProcessorParameter* const ap = processor.getParameters().getUnchecked(i);
        jassert (dynamic_cast<Parameter*> (ap) != nullptr);

        Parameter* p = static_cast<Parameter*> (ap);

        if (p->needsUpdate.compareAndSetBool (0, 1))
        {
            p->copyValueToValueTree();
            anythingUpdated = true;
        }
    }

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
        if (AudioProcessorParameter* p = state.getParameter (paramID))
        {
            const float newValue = state.getParameterRange (paramID)
                                      .convertTo0to1 (newUnnormalisedValue);

            if (p->getValue() != newValue)
                p->setValueNotifyingHost (newValue);
        }
    }

    void sendInitialUpdate()
    {
        if (float* v = state.getRawParameterValue (paramID))
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
        if (AudioProcessorParameter* p = state.getParameter (paramID))
            p->beginChangeGesture();
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
        NormalisableRange<float> range (s.getParameterRange (paramID));
        slider.setRange (range.start, range.end, range.interval);
        slider.setSkewFactor (range.skew, range.symmetricSkew);

        if (AudioProcessorParameter* param = state.getParameter (paramID))
            slider.setDoubleClickReturnValue (true, range.convertFrom0to1 (param->getDefaultValue()));

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

        if ((! ignoreCallbacks) && (! ModifierKeys::getCurrentModifiers().isRightButtonDown()))
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

        {
            ScopedValueSetter<bool> svs (ignoreCallbacks, true);
            combo.setSelectedItemIndex (roundToInt (newValue), sendNotificationSync);
        }
    }

    void comboBoxChanged (ComboBox* comboBox) override
    {
        const ScopedLock selfCallbackLock (selfCallbackMutex);

        if (! ignoreCallbacks)
        {
            beginParameterChange();
            setNewUnnormalisedValue ((float) comboBox->getSelectedId() - 1.0f);
            endParameterChange();
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

#endif
