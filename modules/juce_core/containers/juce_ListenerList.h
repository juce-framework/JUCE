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
    Holds a set of objects and can invoke a member function callback on each object in the set with
    a single call.

    It is safe to add listeners, remove listeners, clear the listeners, and even delete the
    ListenerList itself during any listener callback. If you don't need these extra guarantees
    consider using a LightweightListenerList instead.

    If a Listener is added during a callback, it is guaranteed not to be called in the same
    iteration.

    If a Listener is removed during a callback, it is guaranteed not to be called if it hasn't
    already been called.

    If the ListenerList is cleared or deleted during a callback, it is guaranteed that no more
    listeners will be called.

    It is NOT safe to make concurrent calls to the listeners without a mutex. If you need this
    functionality, either use a LightweightListenerList or a ThreadSafeListenerList.

    When calling listeners the iteration can be escaped early by using a "BailOutChecker".
    A BailOutChecker is a type that has a public member function with the following signature:
    @code bool shouldBailOut() const @endcode
    This function will be called before making a call to each listener.
    For an example see the DummyBailOutChecker.

    @see LightweightListenerList, ThreadSafeListenerList

    @tags{Core}
*/
template <typename ListenerClass,
          typename ArrayType = Array<ListenerClass*>>
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
        A listener can only be added once, so if the listener is already in the list, this method
        has no effect.

        If a Listener is added during a callback, it is guaranteed not to be called in the same
        iteration.

        @see remove
    */
    void add (ListenerClass* listenerToAdd)
    {
        initialiseIfNeeded();

        if (listenerToAdd != nullptr)
            listeners->addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse; // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.

        If a Listener is removed during a callback, it is guaranteed not to be called if it hasn't
        already been called.
    */
    void remove (ListenerClass* listenerToRemove)
    {
        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        if (! initialised())
            return;

        const ScopedLockType lock (listeners->getLock());

        if (const auto index = listeners->removeFirstMatchingValue (listenerToRemove); index >= 0)
        {
            for (auto* it : *iterators)
            {
                if (index < it->end)
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
    [[nodiscard]] ErasedScopeGuard addScoped (ListenerClass& listenerToAdd)
    {
        add (&listenerToAdd);
        return ErasedScopeGuard { [this, &listenerToAdd] { remove (&listenerToAdd); } };
    }

    /** Returns the number of registered listeners. */
    [[nodiscard]] int size() const noexcept                                { return ! initialised() ? 0 : listeners->size(); }

    /** Returns true if no listeners are registered, false otherwise. */
    [[nodiscard]] bool isEmpty() const noexcept                            { return ! initialised() || listeners->isEmpty(); }

    /** Clears the list.

        If the ListenerList is cleared during a callback, it is guaranteed that no more
        listeners will be called.
    */
    void clear()
    {
        if (! initialised())
            return;

        const ScopedLockType lock { listeners->getLock() };

        listeners->clear();

        for (auto* it : *iterators)
            it->end = 0;
    }

    /** Returns true if the specified listener has been added to the list. */
    [[nodiscard]] bool contains (ListenerClass* listener) const noexcept
    {
        return initialised()
            && listeners->contains (listener);
    }

    /** Returns the raw array of listeners.

        Any attempt to mutate the array may result in undefined behaviour.

        If the array uses a mutex/CriticalSection, reading from the array without first obtaining
        the lock may potentially result in undefined behaviour.

        @see add, remove, clear, contains
    */
    [[nodiscard]] const ArrayType& getListeners() const noexcept
    {
        const_cast<ListenerList*> (this)->initialiseIfNeeded();
        return *listeners;
    }

    //==============================================================================
    /** Calls an invokable object for each listener in the list. */
    template <typename Callback>
    void call (Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude.
    */
    template <typename Callback>
    void callExcluding (ListenerClass* listenerToExclude, Callback&& callback)
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));

    }

    /** Calls an invokable object for each listener in the list, additionally checking the bail-out
        checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback)
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback)
    {
       #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
        const ScopedTryLock callCheckedExcludingLock (*callCheckedExcludingMutex);

        // If you hit this assertion it means you're trying to call the listeners from multiple
        // threads concurrently. If you need to do this either use a LightweightListenerList, for a
        // lock free option, or a ThreadSafeListenerList if you also need the extra guarantees
        // provided by ListenerList. See the class descriptions for more details.
        jassert (callCheckedExcludingLock.isLocked());
       #endif

        if (! initialised())
            return;

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

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude.
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
    SharedListeners listeners;

    struct Iterator
    {
        int index{};
        int end{};
    };

    using SafeIterators = std::vector<Iterator*>;
    using SharedIterators = std::shared_ptr<SafeIterators>;
    SharedIterators iterators;

    enum class State
    {
        uninitialised,
        initialising,
        initialised
    };

    std::atomic<State> state { State::uninitialised };

    inline bool initialised() const noexcept { return state == State::initialised; }

    inline void initialiseIfNeeded() noexcept
    {
        if (initialised())
            return;

        auto expected = State::uninitialised;

        if (state.compare_exchange_strong (expected, State::initialising))
        {
            static_assert (std::is_nothrow_constructible_v<ArrayType>,
                           "Any ListenerList ArrayType must have a noexcept default constructor");

            static_assert (std::is_nothrow_constructible_v<SafeIterators>,
                           "Please notify the JUCE team if you encounter this assertion");

            listeners = std::make_shared<ArrayType>();
            iterators = std::make_shared<SafeIterators>();
            state = State::initialised;
            return;
        }

        while (! initialised())
            std::this_thread::yield();
    }

   #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
    // using a unique_ptr helps keep the size of this class down to prevent excessive stack sizes
    // due to objects that contain a ListenerList being created on the stack
    std::unique_ptr<CriticalSection> callCheckedExcludingMutex = std::make_unique<CriticalSection>();
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (ListenerList)
};

//==============================================================================
/**
    A thread safe version of the ListenerList class.

    @see ListenerList, LightweightListenerList

    @tags{Core}
*/
template <typename ListenerClass>
using ThreadSafeListenerList = ListenerList<ListenerClass, Array<ListenerClass*, CriticalSection>>;

//==============================================================================
/**
    A lightweight version of the ListenerList that doesn't provide any guarantees when mutating the
    list from a callback, but allows callbacks to be triggered concurrently without a mutex.

    @see ListenerList, ThreadSafeListenerList

    @tags{Core}
*/
template <typename ListenerClass>
class LightweightListenerList
{
public:
    //==============================================================================
    /** Creates an empty list. */
    LightweightListenerList() = default;

    /** Destructor. */
    ~LightweightListenerList()
    {
        // If you hit this jassert it means you're trying to delete the list while iterating through
        // the listeners! If you need to handle this situation gracefully use a ListenerList or
        // ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);
    }

    //==============================================================================
    /** Adds a listener to the list.
        A listener can only be added once, so if the listener is already in the list, this method
        has no effect.

        If you need to add a Listener during a callback, use the ListenerList type.

        @see remove
    */
    void add (ListenerClass* listenerToAdd)
    {
        // If you hit this jassert it means you're trying to add a listener while iterating through
        // the listeners! If you need to handle this situation gracefully use a ListenerList or
        // ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        if (listenerToAdd != nullptr)
            listeners.addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse; // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.

        If you need to remove a Listener during a callback, use the ListenerList type.
    */
    void remove (ListenerClass* listenerToRemove)
    {
        // If you hit this jassert it means you're trying to remove a listener while iterating
        // through the listeners! If you need to handle this situation gracefully use a ListenerList
        // or ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        listeners.removeFirstMatchingValue (listenerToRemove);
    }

    /** Adds a listener that will be automatically removed when the Guard is destroyed.

        Be very careful to ensure that the ErasedScopeGuard is destroyed or released before the
        ListenerList is destroyed, otherwise the ErasedScopeGuard may attempt to dereference a
        dangling pointer when it is destroyed, which will result in a crash.
    */
    [[nodiscard]] ErasedScopeGuard addScoped (ListenerClass& listenerToAdd)
    {
        add (&listenerToAdd);
        return ErasedScopeGuard { [this, &listenerToAdd] { remove (&listenerToAdd); } };
    }

    /** Returns the number of registered listeners. */
    [[nodiscard]] int size() const noexcept { return listeners.size(); }

    /** Returns true if no listeners are registered, false otherwise. */
    [[nodiscard]] bool isEmpty() const noexcept { return listeners.isEmpty(); }

    /** Clears the list.

        If you need to clear the list during a callback, use the ListenerList type.
    */
    void clear()
    {
        // If you hit this jassert it means you're trying to clear the listener list while iterating
        // through the listeners! If you need to handle this situation gracefully use a ListenerList
        // or ThreadSafeListenerList.
        jassert (numCallsInProgress == 0);

        listeners.clear();
    }

    /** Returns true if the specified listener has been added to the list. */
    [[nodiscard]] bool contains (ListenerClass* listener) const noexcept
    {
        return listeners.contains (listener);
    }

    //==============================================================================
    /** Calls an invokable object for each listener in the list. */
    template <typename Callback>
    void call (Callback&& callback) const
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude.
    */
    template <typename Callback>
    void callExcluding (ListenerClass* listenerToExclude, Callback&& callback) const
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              std::forward<Callback> (callback));

    }

    /** Calls an invokable object for each listener in the list, additionally checking the bail-out
        checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback) const
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              std::forward<Callback> (callback));
    }

    /** Calls an invokable object for each listener in the list, except for the listener specified
        by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback) const
    {
       #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
        ++numCallsInProgress;
        const ScopeGuard decrementPerformingCallbackCount { [&] { --numCallsInProgress; }};
       #endif

        for (auto* listener : listeners)
        {
            if (bailOutChecker.shouldBailOut())
                return;

            if (listener == listenerToExclude)
                continue;

            callback (*listener);
        }
    }

    //==============================================================================
    /** Calls a specific listener method for each listener in the list. */
    template <typename... MethodArgs, typename... Args>
    void call (void (ListenerClass::*callbackFunction) (MethodArgs...), Args&&... args) const
    {
        callCheckedExcluding (nullptr,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude.
    */
    template <typename... MethodArgs, typename... Args>
    void callExcluding (ListenerClass* listenerToExclude,
                        void (ListenerClass::*callbackFunction) (MethodArgs...),
                        Args&&... args) const
    {
        callCheckedExcluding (listenerToExclude,
                              DummyBailOutChecker{},
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, additionally checking the
        bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callChecked (const BailOutCheckerType& bailOutChecker,
                      void (ListenerClass::*callbackFunction) (MethodArgs...),
                      Args&&... args) const
    {
        callCheckedExcluding (nullptr,
                              bailOutChecker,
                              callbackFunction,
                              std::forward<Args> (args)...);
    }

    /** Calls a specific listener method for each listener in the list, except for the listener
        specified by listenerToExclude, additionally checking the bail-out checker before each call.

        See the class description for info about writing a bail-out checker.
    */
    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               void (ListenerClass::*callbackFunction) (MethodArgs...),
                               Args&&... args) const
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
    using DummyBailOutChecker = typename ListenerList<ListenerClass>::DummyBailOutChecker;

    //==============================================================================
    using ThisType      = LightweightListenerList<ListenerClass>;
    using ListenerType  = ListenerClass;

private:
   #if JUCE_ASSERTIONS_ENABLED_OR_LOGGED
    mutable std::atomic<int> numCallsInProgress { 0 };
   #endif

    Array<ListenerClass*> listeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (LightweightListenerList)
};

} // namespace juce
