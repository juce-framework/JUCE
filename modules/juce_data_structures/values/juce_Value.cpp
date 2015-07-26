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
class SimpleValueSource  : public Value::ValueSource
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

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Value::Value (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = static_cast<ReferenceCountedObjectPtr<ValueSource>&&> (other.value);
}

Value& Value::operator= (Value&& other) noexcept
{
    // moving a Value with listeners will lose those listeners, which
    // probably isn't what you wanted to happen!
    jassert (other.listeners.size() == 0);

    other.removeFromListenerList();
    value = static_cast<ReferenceCountedObjectPtr<ValueSource>&&> (other.value);
    return *this;
}
#endif

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

void Value::referTo (const Value& valueToReferTo)
{
    if (valueToReferTo.value != value)
    {
        if (listeners.size() > 0)
        {
            value->valuesWithListeners.removeValue (this);
            valueToReferTo.value->valuesWithListeners.add (this);
        }

        value = valueToReferTo.value;
        callListeners();
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
void Value::addListener (ValueListener* const listener)
{
    if (listener != nullptr)
    {
        if (listeners.size() == 0)
            value->valuesWithListeners.add (this);

        listeners.add (listener);
    }
}

void Value::removeListener (ValueListener* const listener)
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
        listeners.call (&ValueListener::valueChanged, v);
    }
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const Value& value)
{
    return stream << value.toString();
}
