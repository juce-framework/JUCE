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
    Used by the JUCE_DECLARE_SINGLETON macros to manage a static pointer
    to a singleton instance.

    You generally won't use this directly, but see the macros JUCE_DECLARE_SINGLETON,
    JUCE_DECLARE_SINGLETON_SINGLETHREADED, JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL,
    and JUCE_IMPLEMENT_SINGLETON for how it is intended to be used.

    @tags{Core}
*/
template <typename Type, typename MutexType, bool onlyCreateOncePerRun>
struct SingletonHolder  : private MutexType // (inherited so we can use the empty-base-class optimisation)
{
    SingletonHolder() = default;

    ~SingletonHolder()
    {
        /* The static singleton holder is being deleted before the object that it holds
           has been deleted. This could mean that you've forgotten to call clearSingletonInstance()
           in the class's destructor, or have failed to delete it before your app shuts down.
           If you're having trouble cleaning up your singletons, perhaps consider using the
           SharedResourcePointer class instead.
        */
        jassert (instance.load() == nullptr);
    }

    /** Returns the current instance, or creates a new instance if there isn't one. */
    Type* get()
    {
        if (auto* ptr = instance.load())
            return ptr;

        typename MutexType::ScopedLockType sl (*this);

        if (auto* ptr = instance.load())
            return ptr;

        auto once = onlyCreateOncePerRun; // (local copy avoids VS compiler warning about this being constant)

        if (once)
        {
            static bool createdOnceAlready = false;

            if (createdOnceAlready)
            {
                // This means that the doNotRecreateAfterDeletion flag was set
                // and you tried to create the singleton more than once.
                jassertfalse;
                return nullptr;
            }

            createdOnceAlready = true;
        }

        static bool alreadyInside = false;

        if (alreadyInside)
        {
            // This means that your object's constructor has done something which has
            // ended up causing a recursive loop of singleton creation.
            jassertfalse;
            return nullptr;
        }

        const ScopedValueSetter<bool> scope (alreadyInside, true);
        return getWithoutChecking();
    }

    /** Returns the current instance, or creates a new instance if there isn't one, but doesn't do
        any locking, or checking for recursion or error conditions.
    */
    Type* getWithoutChecking()
    {
        if (auto* p = instance.load())
            return p;

        auto* newObject = new Type(); // (create into a local so that instance is still null during construction)
        instance.store (newObject);
        return newObject;
    }

    /** Deletes and resets the current instance, if there is one. */
    void deleteInstance()
    {
        typename MutexType::ScopedLockType sl (*this);
        delete instance.exchange (nullptr);
    }

    /** Called by the class's destructor to clear the pointer if it is currently set to the given object. */
    void clear (Type* expectedObject) noexcept
    {
        instance.compare_exchange_strong (expectedObject, nullptr);
    }

    // This must be atomic, otherwise a late call to get() may attempt to read instance while it is
    // being modified by the very first call to get().
    std::atomic<Type*> instance { nullptr };
};

/** @cond */
#define JUCE_PRIVATE_DECLARE_SINGLETON(Classname, mutex, doNotRecreate, inlineToken, getter) \
    static inlineToken juce::SingletonHolder<Classname, mutex, doNotRecreate> singletonHolder; \
    friend juce::SingletonHolder<Classname, mutex, doNotRecreate>; \
    static Classname* JUCE_CALLTYPE getInstance()                           { return singletonHolder.getter(); } \
    static Classname* JUCE_CALLTYPE getInstanceWithoutCreating() noexcept   { return singletonHolder.instance; } \
    static void JUCE_CALLTYPE deleteInstance() noexcept                     { singletonHolder.deleteInstance(); } \
    void clearSingletonInstance() noexcept                                  { singletonHolder.clear (this); }
/** @endcond */

//==============================================================================
/**
    Macro to generate the appropriate methods and boilerplate for a singleton class.

    To use this, add the line JUCE_DECLARE_SINGLETON (MyClass, doNotRecreateAfterDeletion)
    to the class's definition.

    Then put a macro JUCE_IMPLEMENT_SINGLETON (MyClass) along with the class's
    implementation code.

    It's also a very good idea to also add the call clearSingletonInstance() in your class's
    destructor, in case it is deleted by other means than deleteInstance()

    Clients can then call the static method MyClass::getInstance() to get a pointer
    to the singleton, or MyClass::getInstanceWithoutCreating() which will return nullptr if
    no instance currently exists.

    e.g. @code

        struct MySingleton
        {
            MySingleton() {}

            ~MySingleton()
            {
                // this ensures that no dangling pointers are left when the
                // singleton is deleted.
                clearSingletonInstance();
            }

            JUCE_DECLARE_SINGLETON (MySingleton, false)
        };

        // ..and this goes in a suitable .cpp file:
        JUCE_IMPLEMENT_SINGLETON (MySingleton)


        // example of usage:
        auto* m = MySingleton::getInstance(); // creates the singleton if there isn't already one.

        ...

        MySingleton::deleteInstance(); // safely deletes the singleton (if it's been created).

    @endcode

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    If you know that your object will only be created and deleted by a single thread, you
    can use the slightly more efficient JUCE_DECLARE_SINGLETON_SINGLETHREADED macro instead
    of this one.

    @see JUCE_IMPLEMENT_SINGLETON, JUCE_DECLARE_SINGLETON_SINGLETHREADED
*/
#define JUCE_DECLARE_SINGLETON(Classname, doNotRecreateAfterDeletion) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::CriticalSection, doNotRecreateAfterDeletion, , get)

/**
    The same as JUCE_DECLARE_SINGLETON, but does not require a matching
    JUCE_IMPLEMENT_SINGLETON definition.
*/
#define JUCE_DECLARE_SINGLETON_INLINE(Classname, doNotRecreateAfterDeletion) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::CriticalSection, doNotRecreateAfterDeletion, inline, get)

//==============================================================================
/** This is a counterpart to the JUCE_DECLARE_SINGLETON macros.

    After adding the JUCE_DECLARE_SINGLETON to the class definition, this macro has
    to be used in the cpp file.

    This macro is not required for singletons declared with the INLINE macros, specifically
    JUCE_DECLARE_SINGLETON_INLINE, JUCE_DECLARE_SINGLETON_SINGLETHREADED_INLINE, and
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE.
*/
#define JUCE_IMPLEMENT_SINGLETON(Classname) \
    decltype (Classname::singletonHolder) Classname::singletonHolder;

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is exactly the same as JUCE_DECLARE_SINGLETON, but doesn't use a critical
    section to make access to it thread-safe. If you know that your object will
    only ever be created or deleted by a single thread, then this is a
    more efficient version to use.

    If doNotRecreateAfterDeletion = true, it won't allow the object to be created more
    than once during the process's lifetime - i.e. after you've created and deleted the
    object, getInstance() will refuse to create another one. This can be useful to stop
    objects being accidentally re-created during your app's shutdown code.

    See the documentation for JUCE_DECLARE_SINGLETON for more information about
    how to use it. Just like JUCE_DECLARE_SINGLETON you need to also have a
    corresponding JUCE_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see JUCE_IMPLEMENT_SINGLETON, JUCE_DECLARE_SINGLETON, JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL
*/
#define JUCE_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreateAfterDeletion) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::DummyCriticalSection, doNotRecreateAfterDeletion, , get)

/**
    The same as JUCE_DECLARE_SINGLETON_SINGLETHREADED, but does not require a matching
    JUCE_IMPLEMENT_SINGLETON definition.
*/
#define JUCE_DECLARE_SINGLETON_SINGLETHREADED_INLINE(Classname, doNotRecreateAfterDeletion) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::DummyCriticalSection, doNotRecreateAfterDeletion, inline, get)

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is like JUCE_DECLARE_SINGLETON_SINGLETHREADED, but doesn't do any checking
    for recursion or repeated instantiation. It's intended for use as a lightweight
    version of a singleton, where you're using it in very straightforward
    circumstances and don't need the extra checking.

    See the documentation for JUCE_DECLARE_SINGLETON for more information about
    how to use it. Just like JUCE_DECLARE_SINGLETON you need to also have a
    corresponding JUCE_IMPLEMENT_SINGLETON statement somewhere in your code.

    @see JUCE_IMPLEMENT_SINGLETON, JUCE_DECLARE_SINGLETON
*/
#define JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::DummyCriticalSection, false, , getWithoutChecking)

/**
    The same as JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL, but does not require a matching
    JUCE_IMPLEMENT_SINGLETON definition.
*/
#define JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL_INLINE(Classname) \
    JUCE_PRIVATE_DECLARE_SINGLETON (Classname, juce::DummyCriticalSection, false, inline, getWithoutChecking)

//==============================================================================
/** @cond */
// These are ancient macros, and have now been updated with new names to match the JUCE style guide,
// so please update your code to use the newer versions!
#define juce_DeclareSingleton(Classname, doNotRecreate)                JUCE_DECLARE_SINGLETON(Classname, doNotRecreate)
#define juce_DeclareSingleton_SingleThreaded(Classname, doNotRecreate) JUCE_DECLARE_SINGLETON_SINGLETHREADED(Classname, doNotRecreate)
#define juce_DeclareSingleton_SingleThreaded_Minimal(Classname)        JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL(Classname)
#define juce_ImplementSingleton(Classname)                             JUCE_IMPLEMENT_SINGLETON(Classname)
#define juce_ImplementSingleton_SingleThreaded(Classname)              JUCE_IMPLEMENT_SINGLETON(Classname)
/** @endcond */

} // namespace juce
