/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    Holds a set of objects and can invoke a member function callback on each
    object in the set with a single call.

    Use a ListenerList to manage a set of objects which need a callback, and you
    can invoke a member function by simply calling call(), callChecked(), or
    callExcluding().

    E.g.
    @code
    class MyListenerType
    {
    public:
        void myCallbackMethod (int foo, bool bar);
    };

    ListenerList<MyListenerType> listeners;
    listeners.add (someCallbackObjects...);

    // This will invoke myCallbackMethod (1234, true) on each of the objects
    // in the list...
    listeners.call ([] (MyListenerType& l) { l.myCallbackMethod (1234, true); });
    @endcode

    It is safe to add listeners, remove listeners, clear the listeners, and
    delete the ListenerList itself during any listener callback.

    If a Listener is added during a callback, it is guaranteed not to be called
    in the same iteration.

    If a Listener is removed during a callback, it is guaranteed not to be
    called if it hasn't already been called.

    If the ListenerList is cleared or deleted during a callback, it is
    guaranteed that no more listeners will be called.

    By default a ListenerList is not thread safe. If thread-safety is required,
    you can provide a thread-safe Array type as the second type parameter e.g.
    @code
    using ThreadSafeList = ListenerList<MyListenerType, Array<MyListenerType*, CriticalSection>>;
    @endcode

    When calling listeners the iteration can be escaped early by using a
    "BailOutChecker". A BailOutChecker is a type that has a public member function
    with the following signature:
    @code bool shouldBailOut() const @endcode
    This function will be called before making a call to each listener.
    For an example see the DummyBailOutChecker.

    @tags{Core}
*/
template <class ListenerClass,
          class ArrayType = Array<ListenerClass*>>
class ListenerList
{
public:
    //==============================================================================
    /** Creates an empty list. */
    ListenerList() = default;

    /** Destructor. */
    ~ListenerList() { clear(); }

    //==============================================================================
    /** Adds a listener to the list.
        A listener can only be added once, so if the listener is already in the list,
        this method has no effect.

        If a Listener is added during a callback, it is guaranteed not to be called
        in the same iteration.

        @see remove
    */
    void add (ListenerClass* listenerToAdd)
    {
        if (listenerToAdd != nullptr)
            listeners->addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse; // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.

        If a Listener is removed during a callback, it is guaranteed not to be
        called if it hasn't already been called.
    */
    void remove (ListenerClass* listenerToRemove)
    {
        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        const ScopedLockType lock (listeners->getLock());

        if (const auto index = listeners->removeFirstMatchingValue (listenerToRemove); index >= 0)
        {
            for (auto* it : *iterators)
            {
                --it->end;

                if (index <= it->index)
                    --it->index;
            }
        }
    }

    /** Adds a listener that will be automatically removed again when the Guard is destroyed.

        Be very careful to ensure that the ErasedScopeGuard is destroyed or released before the
        ListenerList is destroyed, otherwise the ErasedScopeGuard may attempt to dereference a
        dangling pointer when it is destroyed, which will result in a crash.
    */
    ErasedScopeGuard addScoped (ListenerClass& listenerToAdd)
    {
        add (&listenerToAdd);
        return ErasedScopeGuard { [this, &listenerToAdd] { remove (&listenerToAdd); } };
    }

    /** Returns the number of registered listeners. */
    int size() const noexcept                                { return listeners->size(); }

    /** Returns true if no listeners are registered, false otherwise. */
    bool isEmpty() const noexcept                            { return listeners->isEmpty(); }

    /** Clears the list.

        If the ListenerList is cleared during a callback, it is guaranteed that
        no more listeners will be called.
    */
    void clear()
    {
        const ScopedLockType lock (listeners->getLock());

        listeners->clear();

        for (auto* it : *iterators)
            it->end = 0;
    }

    /** Returns true if the specified listener has been added to the list. */
    bool contains (ListenerClass* listener) const noexcept   { return listeners->contains (listener); }

    /** Returns the raw array of listeners.

        Any attempt to mutate the array may result in undefined behaviour.

        If the array uses a mutex/CriticalSection, reading from the array without first
        obtaining the lock may potentially result in undefined behaviour.

        @see add, remove, clear, contains
    */
    const ArrayType& getListeners() const noexcept           { return *listeners; }

    //==============================================================================
    /** Calls an invokable object for each listener in the list. */
    template <typename Callback>
    void call (Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the
        listener specified by listenerToExclude.
    */
    template <typename Callback>
    void callExcluding (ListenerClass* listenerToExclude, Callback&& callback)
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));

    }

    /** Calls an invokable object for each listener in the list, additionally
        checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the
        listener specified by listenerToExclude, additionally checking the
        bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback)
    {
        const auto localListeners = listeners;
        const ScopedLockType lock { localListeners->getLock() };

        Iterator it{};
        it.end = localListeners->size();

        iterators->push_back (&it);

        const ScopeGuard scope { [i = iterators, &it]
        {
            i->erase (std::remove (i->begin(), i->end(), &it), i->end());
        } };

        for (; it.index < it.end; ++it.index)
        {
            if (bailOutChecker.shouldBailOut())
                return;

            auto* listener = localListeners->getUnchecked (it.index);

            if (listener == listenerToExclude)
                continue;

            callback (*listener);
        }
    }

    //==============================================================================
    /** Calls a specific listener method for each listener in the list. */
    template <typename... MethodArgs, typename... Args>
    void call (void (ListenerClass::*callbackFunction) (MethodArgs...), Args&&... args)
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except
        for the listener specified by listenerToExclude.
    */
    template <typename... MethodArgs, typename... Args>
    void callExcluding (ListenerClass* listenerToExclude,
                        void (ListenerClass::*callbackFunction) (MethodArgs...),
                        Args&&... args)
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list,
        additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callChecked (const BailOutCheckerType& bailOutChecker,
                      void (ListenerClass::*callbackFunction) (MethodArgs...),
                      Args&&... args)
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except
        for the listener specified by listenerToExclude, additionally checking
        the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               void (ListenerClass::*callbackFunction) (MethodArgs...),
                               Args&&... args)
    {
        callCheckedExcluding (listenerToExclude, bailOutChecker, [&] (ListenerClass& l)
        {
            (l.*callbackFunction) (args...);
        });
    }

    //==============================================================================
    /** A dummy bail-out checker that always returns false.
        See the class description for info about writing a bail-out checker.
    */
    struct DummyBailOutChecker
    {
        constexpr bool shouldBailOut() const noexcept { return false; }
    };

    //==============================================================================
    using ThisType      = ListenerList<ListenerClass, ArrayType>;
    using ListenerType  = ListenerClass;

private:
    //==============================================================================
    using ScopedLockType = typename ArrayType::ScopedLockType;

    //==============================================================================
    using SharedListeners = std::shared_ptr<ArrayType>;
    const SharedListeners listeners = std::make_shared<ArrayType>();

    struct Iterator
    {
        int index{};
        int end{};
    };

    using SafeIterators = std::vector<Iterator*>;
    using SharedIterators = std::shared_ptr<SafeIterators>;
    const SharedIterators iterators = std::make_shared<SafeIterators>();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (ListenerList)
};

} // namespace juce
