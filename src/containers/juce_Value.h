/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_VALUE_JUCEHEADER__
#define __JUCE_VALUE_JUCEHEADER__

#include "juce_Variant.h"
#include "../events/juce_AsyncUpdater.h"
#include "juce_ReferenceCountedObject.h"
#include "juce_SortedSet.h"


//==============================================================================
/**
    Represents a shared variant value.

    A Value object contains a reference to a var object, and can get and set its value.
    Listeners can be attached to be told when the value is changed.

    The Value class is a wrapper around a shared, reference-counted underlying data
    object - this means that multiple Value objects can all refer to the same piece of
    data, allowing all of them to be notified when any of them changes it.

    The base class of Value contains a simple var object, but subclasses can be
    created that map a Value onto any kind of underlying data, e.g.
    ValueTree::getPropertyAsValue() returns a Value object that is a wrapper
    for one of its properties.
*/
class JUCE_API  Value
{
public:
    //==============================================================================
    /** Creates an empty Value, containing a void var. */
    Value();

    /** Creates a Value that refers to the same value as another one.

        Note that this doesn't make a copy of the other value - both this and the other
        Value will share the same underlying value, so that when either one alters it, both
        will see it change.
    */
    Value (const Value& other);

    /** Creates a Value that is set to the specified value. */
    Value (const var& initialValue);

    /** Destructor. */
    ~Value();

    //==============================================================================
    /** Returns the current value. */
    const var getValue() const;

    /** Returns the current value. */
    operator const var() const;

    /** Returns the value as a string.

        This is alternative to writing things like "myValue.getValue().toString()".
    */
    const String toString() const;

    /** Sets the current value.

        You can also use operator= to set the value.

        If there are any listeners registered, they will be notified of the
        change asynchronously.
    */
    void setValue (const var& newValue);

    /** Sets the current value.

        This is the same as calling setValue().

        If there are any listeners registered, they will be notified of the
        change asynchronously.
    */
    const Value& operator= (const var& newValue);

    /** Makes this object refer to the same underlying ValueSource as another one.

        Once this object has been connected to another one, changing either one
        will update the other.

        Existing listeners will still be registered after you call this method, and
        they'll continue to receive messages when the new value changes.
    */
    void referTo (const Value& valueToReferTo);

    /** Returns true if this value and the other one are references to the same value.
    */
    bool refersToSameSourceAs (const Value& other) const;

    /** Compares two values.
        This is a compare-by-value comparison, so is effectively the same as
        saying (this->getValue() == other.getValue()).
    */
    bool operator== (const Value& other) const;

    /** Compares two values.
        This is a compare-by-value comparison, so is effectively the same as
        saying (this->getValue() != other.getValue()).
    */
    bool operator!= (const Value& other) const;

    //==============================================================================
    /** Receives callbacks when a Value object changes.
        @see Value::addListener
    */
    class JUCE_API  Listener
    {
    public:
        Listener()          {}
        virtual ~Listener() {}

        /** Called when a Value object is changed.

            Note that the Value object passed as a parameter may not be exactly the same
            object that you registered the listener with - it might be a copy that refers
            to the same underlying ValueSource. To find out, you can call Value::refersToSameSourceAs().
        */
        virtual void valueChanged (Value& value) = 0;
    };

    /** Adds a listener to receive callbacks when the value changes.

        The listener is added to this specific Value object, and not to the shared
        object that it refers to. When this object is deleted, all the listeners will
        be lost, even if other references to the same Value still exist. So when you're
        adding a listener, make sure that you add it to a ValueTree instance that will last
        for as long as you need the listener. In general, you'd never want to add a listener
        to a local stack-based ValueTree, but more likely to one that's a member variable.

        @see removeListener
    */
    void addListener (Listener* const listener);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* const listener);


    //==============================================================================
    /**
        Used internally by the Value class as the base class for its shared value objects.

        The Value class is essentially a reference-counted pointer to a shared instance
        of a ValueSource object. If you're feeling adventurous, you can create your own custom
        ValueSource classes to allow Value objects to represent your own custom data items.
    */
    class JUCE_API  ValueSource   : public ReferenceCountedObject,
                                    public AsyncUpdater
    {
    public:
        ValueSource();
        virtual ~ValueSource();

        /** Returns the current value of this object. */
        virtual const var getValue() const = 0;
        /** Changes the current value.
            This must also trigger a change message if the value actually changes.
        */
        virtual void setValue (const var& newValue) = 0;

        /** Delivers a change message to all the listeners that are registered with
            this value.

            If dispatchSynchronously is true, the method will call all the listeners
            before returning; otherwise it'll dispatch a message and make the call later.
        */
        void sendChangeMessage (const bool dispatchSynchronously);

        //==============================================================================
        juce_UseDebuggingNewOperator

    protected:
        friend class Value;
        SortedSet <Value*> valuesWithListeners;

        void handleAsyncUpdate();

        ValueSource (const ValueSource&);
        const ValueSource& operator= (const ValueSource&);
    };


    //==============================================================================
    /** @internal */
    explicit Value (ValueSource* const valueSource);
    /** @internal */
    ValueSource& getValueSource()       { return *value; }

    juce_UseDebuggingNewOperator

private:
    friend class ValueSource;
    ReferenceCountedObjectPtr <ValueSource> value;
    SortedSet <Listener*> listeners;

    void callListeners();

    // This is disallowed to avoid confusion about whether it should
    // do a by-value or by-reference copy.
    const Value& operator= (const Value& other);
};


#endif   // __JUCE_VALUE_JUCEHEADER__
