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
    Holds a set of objects and can invoke a member function callback on each object
    in the set with a single call.

    Use a ListenerList to manage a set of objects which need a callback, and you
    can invoke a member function by simply calling call() or callChecked().

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

    It is guaranteed that every Listener is called during an iteration if it's inside the
    ListenerList before the iteration starts and isn't removed until its end. This guarantee
    holds even if some Listeners are removed or new ones are added during the iteration.

    Listeners added during an iteration are guaranteed to be not called in that iteration.

    Sometimes, there's a chance that invoking one of the callbacks might result in the
    list itself being deleted while it's still iterating - to survive this situation, you can
    use callChecked() instead of call(), passing it a local object to act as a "BailOutChecker".
    The BailOutChecker must implement a method of the form "bool shouldBailOut()", and
    the list will check this after each callback to determine whether it should abort the
    operation. For an example of a bail-out checker, see the Component::BailOutChecker class,
    which can be used to check when a Component has been deleted. See also
    ListenerList::DummyBailOutChecker, which is a dummy checker that always returns false.

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
    ~ListenerList()
    {
        WrappedIterator::forEach (activeIterators, [&] (auto& iter)
        {
            iter.invalidate();
        });
    }

    //==============================================================================
    /** Adds a listener to the list.
        A listener can only be added once, so if the listener is already in the list,
        this method has no effect.
        @see remove
    */
    void add (ListenerClass* listenerToAdd)
    {
        if (listenerToAdd != nullptr)
            listeners.addIfNotAlreadyThere (listenerToAdd);
        else
            jassertfalse;  // Listeners can't be null pointers!
    }

    /** Removes a listener from the list.
        If the listener wasn't in the list, this has no effect.
    */
    void remove (ListenerClass* listenerToRemove)
    {
        jassert (listenerToRemove != nullptr); // Listeners can't be null pointers!

        typename ArrayType::ScopedLockType lock (listeners.getLock());

        const auto index = listeners.removeFirstMatchingValue (listenerToRemove);

        WrappedIterator::forEach (activeIterators, [&] (auto& iter)
        {
            if (0 <= index && index < iter.get().index)
                --iter.get().index;
        });
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
    int size() const noexcept                                { return listeners.size(); }

    /** Returns true if no listeners are registered, false otherwise. */
    bool isEmpty() const noexcept                            { return listeners.isEmpty(); }

    /** Clears the list. */
    void clear()                                             { listeners.clear(); }

    /** Returns true if the specified listener has been added to the list. */
    bool contains (ListenerClass* listener) const noexcept   { return listeners.contains (listener); }

    /** Returns the raw array of listeners. */
    const ArrayType& getListeners() const noexcept           { return listeners; }

    //==============================================================================
    /** Calls a member function on each listener in the list, with multiple parameters. */
    template <typename Callback>
    void call (Callback&& callback)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (WrappedIterator iter (*this, activeIterators); iter.get().next();)
            callback (*iter.get().getListener());
    }

    /** Calls a member function with 1 parameter, on all but the specified listener in the list.
        This can be useful if the caller is also a listener and needs to exclude itself.
    */
    template <typename Callback>
    void callExcluding (ListenerClass* listenerToExclude, Callback&& callback)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (WrappedIterator iter (*this, activeIterators); iter.get().next();)
        {
            auto* l = iter.get().getListener();

            if (l != listenerToExclude)
                callback (*l);
        }
    }

    /** Calls a member function on each listener in the list, with 1 parameter and a bail-out-checker.
        See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callChecked (const BailOutCheckerType& bailOutChecker, Callback&& callback)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (WrappedIterator iter (*this, activeIterators); iter.get().next (bailOutChecker);)
        {
            callback (*iter.get().getListener());
        }
    }

    /** Calls a member function, with 1 parameter, on all but the specified listener in the list
        with a bail-out-checker. This can be useful if the caller is also a listener and needs to
        exclude itself. See the class description for info about writing a bail-out checker.
    */
    template <typename Callback, typename BailOutCheckerType>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               Callback&& callback)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (WrappedIterator iter (*this, activeIterators); iter.get().next (bailOutChecker);)
        {
            auto* l = iter.get().getListener();

            if (l != listenerToExclude)
                callback (*l);
        }
    }

    //==============================================================================
    /** A dummy bail-out checker that always returns false.
        See the ListenerList notes for more info about bail-out checkers.
    */
    struct DummyBailOutChecker
    {
        bool shouldBailOut() const noexcept                 { return false; }
    };

    using ThisType      = ListenerList<ListenerClass, ArrayType>;
    using ListenerType  = ListenerClass;

    //==============================================================================
    /** Iterates the listeners in a ListenerList. */
    struct Iterator
    {
        explicit Iterator (const ListenerList& listToIterate) noexcept
            : list (listToIterate), index (listToIterate.size())
        {}

        //==============================================================================
        bool next() noexcept
        {
            if (index <= 0)
                return false;

            auto listSize = list.size();

            if (--index < listSize)
                return true;

            index = listSize - 1;
            return index >= 0;
        }

        template <class BailOutCheckerType>
        bool next (const BailOutCheckerType& bailOutChecker) noexcept
        {
            return (! bailOutChecker.shouldBailOut()) && next();
        }

        ListenerClass* getListener() const noexcept
        {
            return list.getListeners().getUnchecked (index);
        }

    private:
        const ListenerList& list;
        int index;

        friend ListenerList;

        JUCE_DECLARE_NON_COPYABLE (Iterator)
    };

    //==============================================================================
   #ifndef DOXYGEN
    void call (void (ListenerClass::*callbackFunction)())
    {
        call ([=] (ListenerClass& l) { (l.*callbackFunction)(); });
    }

    void callExcluding (ListenerClass* listenerToExclude, void (ListenerClass::*callbackFunction)())
    {
        callExcluding (listenerToExclude, [=] (ListenerClass& l) { (l.*callbackFunction)(); });
    }

    template <class BailOutCheckerType>
    void callChecked (const BailOutCheckerType& bailOutChecker, void (ListenerClass::*callbackFunction)())
    {
        callChecked (bailOutChecker, [=] (ListenerClass& l) { (l.*callbackFunction)(); });
    }

    template <class BailOutCheckerType>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               void (ListenerClass::*callbackFunction)())
    {
        callCheckedExcluding (listenerToExclude, bailOutChecker, [=] (ListenerClass& l) { (l.*callbackFunction)(); });
    }

    template <typename... MethodArgs, typename... Args>
    void call (void (ListenerClass::*callbackFunction) (MethodArgs...), Args&&... args)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (Iterator iter (*this); iter.next();)
            (iter.getListener()->*callbackFunction) (static_cast<typename TypeHelpers::ParameterType<Args>::type> (args)...);
    }

    template <typename... MethodArgs, typename... Args>
    void callExcluding (ListenerClass* listenerToExclude,
                        void (ListenerClass::*callbackFunction) (MethodArgs...),
                        Args&&... args)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (Iterator iter (*this); iter.next();)
            if (iter.getListener() != listenerToExclude)
                (iter.getListener()->*callbackFunction) (static_cast<typename TypeHelpers::ParameterType<Args>::type> (args)...);
    }

    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callChecked (const BailOutCheckerType& bailOutChecker,
                      void (ListenerClass::*callbackFunction) (MethodArgs...),
                      Args&&... args)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (Iterator iter (*this); iter.next (bailOutChecker);)
            (iter.getListener()->*callbackFunction) (static_cast<typename TypeHelpers::ParameterType<Args>::type> (args)...);
    }

    template <typename BailOutCheckerType, typename... MethodArgs, typename... Args>
    void callCheckedExcluding (ListenerClass* listenerToExclude,
                               const BailOutCheckerType& bailOutChecker,
                               void (ListenerClass::*callbackFunction) (MethodArgs...),
                               Args&&... args)
    {
        typename ArrayType::ScopedLockType lock (listeners.getLock());

        for (Iterator iter (*this); iter.next (bailOutChecker);)
            if (iter.getListener() != listenerToExclude)
                (iter.getListener()->*callbackFunction) (static_cast<typename TypeHelpers::ParameterType<Args>::type> (args)...);
    }
   #endif

private:
    class WrappedIterator
    {
    public:
        WrappedIterator (const ListenerList& listToIterate, WrappedIterator*& listHeadIn)
            : it (listToIterate), listHead (listHeadIn), next (listHead)
        {
            // GCC 12.2 with O1 and above gets confused here
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdangling-pointer")
            listHead = this;
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }

        ~WrappedIterator()
        {
            if (valid)
                listHead = next;
        }

        auto& get() noexcept { return it; }

        template <typename Callback>
        static void forEach (WrappedIterator* wrapped, Callback&& cb)
        {
            for (auto* p = wrapped; p != nullptr; p = p->next)
                cb (*p);
        }

        void invalidate() noexcept { valid = false; }

    private:
        Iterator it;
        WrappedIterator*& listHead;
        WrappedIterator* next = nullptr;
        bool valid = true;
    };

    //==============================================================================
    ArrayType listeners;
    WrappedIterator* activeIterators = nullptr;

    JUCE_DECLARE_NON_COPYABLE (ListenerList)
};

} // namespace juce
