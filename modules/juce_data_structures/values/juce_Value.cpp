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

Value::ValueSource::ValueSource()
{
}

Value::ValueSource::~ValueSource()
{
    cancelPendingUpdate();
}

void Value::ValueSource::handleAsyncUpdate()
{
    sendChangeMessage (true);
}

void Value::ValueSource::sendChangeMessage (const bool synchronous)
{
    const int numListeners = valuesWithListeners.size();

    if (numListeners > 0)
    {
        if (synchronous)
        {
            const ReferenceCountedObjectPtr<ValueSource> localRef (this);

            cancelPendingUpdate();

            for (int i = numListeners; --i >= 0;)
                if (Value* const v = valuesWithListeners[i])
                    v->callListeners();
        }
        else
        {
            triggerAsyncUpdate();
        }
    }
}

//==============================================================================
class SimpleValueSource final : public Value::ValueSource
{
public:
    SimpleValueSource()
    {
    }

    SimpleValueSource (const var& initialValue)
        : value (initialValue)
    {
    }

    var getValue() const override
    {
        return value;
    }

    void setValue (const var& newValue) override
    {
        if (! newValue.equalsWithSameType (value))
        {
            value = newValue;
            sendChangeMessage (false);
        }
    }

private:
    var value;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SimpleValueSource)
};


//==============================================================================
Value::Value()  : value (new SimpleValueSource())
{
}

Value::Value (ValueSource* const v)  : value (v)
{
    jassert (v != nullptr);
}

Value::Value (const var& initialValue)  : value (new SimpleValueSource (initialValue))
{
}

Value::Value (const Value& other)  : value (other.value)
{
}

Value::Value (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = std::move (other.value);
}

Value& Value::operator= (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = std::move (other.value);
    return *this;
}

Value::~Value()
{
    removeFromListenerList();
}

void Value::removeFromListenerList()
{
    if (listeners.size() > 0 && value != nullptr) // may be nullptr after a move operation
        value->valuesWithListeners.removeValue (this);
}

//==============================================================================
var Value::getValue() const
{
    return value->getValue();
}

Value::operator var() const
{
    return value->getValue();
}

void Value::setValue (const var& newValue)
{
    value->setValue (newValue);
}

String Value::toString() const
{
    return value->getValue().toString();
}

Value& Value::operator= (const var& newValue)
{
    value->setValue (newValue);
    return *this;
}

void Value::referTo (const Value& valueToReferTo, bool notifyListeners)
{
    if (valueToReferTo.value != value)
    {
        if (listeners.size() > 0)
        {
            value->valuesWithListeners.removeValue (this);
            valueToReferTo.value->valuesWithListeners.add (this);
        }

        value = valueToReferTo.value;
        if(notifyListeners) callListeners();
    }
}

bool Value::refersToSameSourceAs (const Value& other) const
{
    return value == other.value;
}

bool Value::operator== (const Value& other) const
{
    return value == other.value || value->getValue() == other.getValue();
}

bool Value::operator!= (const Value& other) const
{
    return value != other.value && value->getValue() != other.getValue();
}

//==============================================================================
void Value::addListener (Value::Listener* listener)
{
    if (listener != nullptr)
    {
        if (listeners.size() == 0)
            value->valuesWithListeners.add (this);

        listeners.add (listener);
    }
}

void Value::removeListener (Value::Listener* listener)
{
    listeners.remove (listener);

    if (listeners.size() == 0)
        value->valuesWithListeners.removeValue (this);
}

void Value::callListeners()
{
    if (listeners.size() > 0)
    {
        Value v (*this); // (create a copy in case this gets deleted by a callback)
        listeners.call ([&] (Value::Listener& l) { l.valueChanged (v); });
    }
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const Value& value)
{
    return stream << value.toString();
}

} // namespace juce
