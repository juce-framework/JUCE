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

class SharedValueSourceUpdater  : public ReferenceCountedObject,
                                  private AsyncUpdater
{
public:
    SharedValueSourceUpdater() : insideCallback (false) {}

    typedef ReferenceCountedObjectPtr<SharedValueSourceUpdater> Ptr;

    void update (Value::ValueSource* source)
    {
        sourcesToUpdate.add (source);

        if (! insideCallback)
            triggerAsyncUpdate();
    }

    static SharedValueSourceUpdater* getOrCreateSharedUpdater()
    {
        Ptr& p = getSharedUpdater();

        if (p == nullptr)
            p = new SharedValueSourceUpdater();

        return p;
    }

    static void releaseIfUnused()
    {
        if (Ptr& p = getSharedUpdater())
            if (p->getReferenceCount() == 1)
                p = nullptr;
    }

private:
    ReferenceCountedArray<Value::ValueSource> sourcesToUpdate;
    bool insideCallback;

    static Ptr& getSharedUpdater()
    {
        static Ptr updater;
        return updater;
    }

    void handleAsyncUpdate()
    {
        int maxLoops = 10;
        const ScopedValueSetter<bool> inside (insideCallback, true, false);
        const Ptr localRef (this);

        while (sourcesToUpdate.size() > 0 && --maxLoops >= 0)
        {
            ReferenceCountedArray<Value::ValueSource> sources;
            sources.swapWithArray (sourcesToUpdate);

            for (int i = 0; i < sources.size(); ++i)
                sources.getObjectPointerUnchecked(i)->sendChangeMessage (true);
        }
    }

    JUCE_DECLARE_NON_COPYABLE (SharedValueSourceUpdater);
};

Value::ValueSource::ValueSource()
{
}

Value::ValueSource::~ValueSource()
{
    asyncUpdater = nullptr;
    SharedValueSourceUpdater::releaseIfUnused();
}

void Value::ValueSource::sendChangeMessage (const bool synchronous)
{
    const int numListeners = valuesWithListeners.size();

    if (numListeners > 0)
    {
        if (synchronous)
        {
            asyncUpdater = nullptr;
            const ReferenceCountedObjectPtr<ValueSource> localRef (this);

            for (int i = numListeners; --i >= 0;)
                if (Value* const v = valuesWithListeners[i])
                    v->callListeners();
        }
        else if (asyncUpdater == nullptr)
        {
            SharedValueSourceUpdater* const updater = SharedValueSourceUpdater::getOrCreateSharedUpdater();
            asyncUpdater = updater;
            updater->update (this);
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

    var getValue() const
    {
        return value;
    }

    void setValue (const var& newValue)
    {
        if (! newValue.equalsWithSameType (value))
        {
            value = newValue;
            sendChangeMessage (false);
        }
    }

private:
    var value;

    JUCE_DECLARE_NON_COPYABLE (SimpleValueSource);
};


//==============================================================================
Value::Value()
    : value (new SimpleValueSource())
{
}

Value::Value (ValueSource* const value_)
    : value (value_)
{
    jassert (value_ != nullptr);
}

Value::Value (const var& initialValue)
    : value (new SimpleValueSource (initialValue))
{
}

Value::Value (const Value& other)
    : value (other.value)
{
}

Value& Value::operator= (const Value& other)
{
    value = other.value;
    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
Value::Value (Value&& other) noexcept
    : value (static_cast <ReferenceCountedObjectPtr <ValueSource>&&> (other.value))
{
}

Value& Value::operator= (Value&& other) noexcept
{
    value = static_cast <ReferenceCountedObjectPtr <ValueSource>&&> (other.value);
    return *this;
}
#endif

Value::~Value()
{
    if (listeners.size() > 0)
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
    Value v (*this); // (create a copy in case this gets deleted by a callback)
    listeners.call (&ValueListener::valueChanged, v);
}

OutputStream& JUCE_CALLTYPE operator<< (OutputStream& stream, const Value& value)
{
    return stream << value.toString();
}
