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

//==============================================================================
/**
    Represents a shared variant value.

    A Value object contains a reference to a var object, and can get and set its value.
    Listeners can be attached to be told when the value is changed.

    The Value class is a wrapper around a shared, reference-counted underlying data
    object - this means that multiple Value objects can all refer to the same piece of
    data, allowing all of them to be notified when any of them changes it.

    When you create a Value with its default constructor, it acts as a wrapper around a
    simple var object, but by creating a Value that refers to a custom subclass of ValueSource,
    you can map the Value onto any kind of underlying data.

    Important note! The Value class is not thread-safe! If you're accessing one from
    multiple threads, then you'll need to use your own synchronisation around any code
    that accesses it.

    @tags{DataStructures}
*/
class JUCE_API  Value  final
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
    explicit Value (const var& initialValue);

    /** Move constructor */
    Value (Value&&) noexcept;

    /** Destructor. */
    ~Value();

    //==============================================================================
    /** Returns the current value. */
    var getValue() const;

    /** Returns the current value. */
    operator var() const;

    /** Returns the value as a string.
        This is a shortcut for "myValue.getValue().toString()".
    */
    String toString() const;

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
    Value& operator= (const var& newValue);

    /** Move assignment operator */
    Value& operator= (Value&&) noexcept;

    /** Makes this object refer to the same underlying ValueSource as another one.

        Once this object has been connected to another one, changing either one
        will update the other.

        Existing listeners will still be registered after you call this method, and
        they'll continue to receive messages when the new value changes.
    */
    void referTo (const Value& valueToReferTo, bool notifyListeners = true);

    /** Returns true if this object and the other one use the same underlying
        ValueSource object.
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
        Listener() = default;
        virtual ~Listener() = default;

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
        adding a listener, make sure that you add it to a Value instance that will last
        for as long as you need the listener. In general, you'd never want to add a listener
        to a local stack-based Value, but more likely to one that's a member variable.

        @see removeListener
    */
    void addListener (Listener* listener);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* listener);


    //==============================================================================
    /**
        Used internally by the Value class as the base class for its shared value objects.

        The Value class is essentially a reference-counted pointer to a shared instance
        of a ValueSource object. If you're feeling adventurous, you can create your own custom
        ValueSource classes to allow Value objects to represent your own custom data items.
    */
    class JUCE_API  ValueSource   : public ReferenceCountedObject,
                                    private AsyncUpdater
    {
    public:
        ValueSource();
        ~ValueSource() override;

        /** Returns the current value of this object. */
        virtual var getValue() const = 0;

        /** Changes the current value.
            This must also trigger a change message if the value actually changes.
        */
        virtual void setValue (const var& newValue) = 0;

        /** Delivers a change message to all the listeners that are registered with
            this value.

            If dispatchSynchronously is true, the method will call all the listeners
            before returning; otherwise it'll dispatch a message and make the call later.
        */
        void sendChangeMessage (bool dispatchSynchronously);

    protected:
        //==============================================================================
        friend class Value;
        SortedSet<Value*> valuesWithListeners;

    private:
        void handleAsyncUpdate() override;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueSource)
    };


    //==============================================================================
    /** Creates a Value object that uses this valueSource object as its underlying data. */
    explicit Value (ValueSource* valueSource);

    /** Returns the ValueSource that this value is referring to. */
    ValueSource& getValueSource() noexcept          { return *value; }


private:
    //==============================================================================
    friend class ValueSource;
    ReferenceCountedObjectPtr<ValueSource> value;
    ListenerList<Listener> listeners;

    void callListeners();
    void removeFromListenerList();

    // This is disallowed to avoid confusion about whether it should
    // do a by-value or by-reference copy.
    Value& operator= (const Value&) = delete;

    // This declaration prevents accidental construction from an integer of 0,
    // which is possible in some compilers via an implicit cast to a pointer.
    explicit Value (void*) = delete;
};

/** Writes a Value to an OutputStream as a UTF8 string. */
OutputStream& JUCE_CALLTYPE operator<< (OutputStream&, const Value&);


} // namespace juce
