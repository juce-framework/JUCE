/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_SINGLETON_JUCEHEADER__
#define __JUCE_SINGLETON_JUCEHEADER__

#include "../threads/juce_ScopedLock.h"


//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    To use this, add the line juce_DeclareSingleton (MyClass, allowOnlyOneInstance)
    to the class's definition.

    If allowOnlyOneInstance == true, it won't allow the object to be created
    more than once in the process's lifetime.

    Then put a macro juce_ImplementSingleton (MyClass) along with the class's
    implementation code.

    Clients can then call the static MyClass::getInstance() to get a pointer to the
    singleton, or MyClass::getInstanceWithoutCreating() which may return 0 if no instance
    is currently extant

    it's a very good idea to also add the call clearSingletonInstance() to the
    destructor of the class, in case it is deleted by other means than deleteInstance()

    e.g. @code

        class MySingleton
        {
        public:
            MySingleton()
            {
            }

            ~MySingleton()
            {
                // this ensures that no dangling pointers are left when the
                // singleton is deleted.
                clearSingletonInstance();
            }

            juce_DeclareSingleton (MySingleton, false)
        };

        juce_ImplementSingleton (MySingleton)


        // example of usage:
        MySingleton* m = MySingleton::getInstance(); // creates the singleton if there isn't already one.

        ...

        MySingleton::deleteInstance(); // safely deletes the singleton (if it's been created).

    @endcode

    If you know that your object will only be created and deleted by a single thread, you
    can use the slightly more efficient juce_DeclareSingleton_SingleThreaded() macro instead
    of this one.

    @see juce_ImplementSingleton, juce_DeclareSingleton_SingleThreaded
*/
#define juce_DeclareSingleton(classname, allowOnlyOneInstance) \
\
    static classname* _singletonInstance;  \
    static CriticalSection _singletonLock; \
\
    static classname* getInstance() \
    { \
        if (_singletonInstance == 0) \
        {\
            const ScopedLock sl (_singletonLock); \
\
            if (_singletonInstance == 0) \
            { \
                static bool alreadyInside = false; \
                static bool createdOnceAlready = false; \
\
                const bool problem = alreadyInside || ((allowOnlyOneInstance) && createdOnceAlready); \
                jassert (! problem); \
                if (! problem) \
                { \
                    createdOnceAlready = true; \
                    alreadyInside = true; \
                    classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
                    alreadyInside = false; \
\
                    _singletonInstance = newObject; \
                } \
            } \
        } \
\
        return _singletonInstance; \
    } \
\
    static inline classname* getInstanceWithoutCreating() throw() \
    { \
        return _singletonInstance; \
    } \
\
    static void deleteInstance() \
    { \
        const ScopedLock sl (_singletonLock); \
        if (_singletonInstance != 0) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = 0; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() throw() \
    { \
        if (_singletonInstance == this) \
            _singletonInstance = 0; \
    }


//==============================================================================
/** This is a counterpart to the juce_DeclareSingleton macro.

    After adding the juce_DeclareSingleton to the class definition, this macro has
    to be used in the cpp file.
*/
#define juce_ImplementSingleton(classname) \
\
    classname* classname::_singletonInstance = 0; \
    CriticalSection classname::_singletonLock;


//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is exactly the same as juce_DeclareSingleton, but doesn't use a critical
    section to make access to it thread-safe. If you know that your object will
    only ever be created or deleted by a single thread, then this is a
    more efficient version to use.

    See the documentation for juce_DeclareSingleton for more information about
    how to use it, the only difference being that you have to use
    juce_ImplementSingleton_SingleThreaded instead of juce_ImplementSingleton.

    @see juce_ImplementSingleton_SingleThreaded, juce_DeclareSingleton, juce_DeclareSingleton_SingleThreaded_Minimal
*/
#define juce_DeclareSingleton_SingleThreaded(classname, allowOnlyOneInstance) \
\
    static classname* _singletonInstance;  \
\
    static classname* getInstance() \
    { \
        if (_singletonInstance == 0) \
        { \
            static bool alreadyInside = false; \
            static bool createdOnceAlready = false; \
\
            const bool problem = alreadyInside || ((allowOnlyOneInstance) && createdOnceAlready); \
            jassert (! problem); \
            if (! problem) \
            { \
                createdOnceAlready = true; \
                alreadyInside = true; \
                classname* newObject = new classname();  /* (use a stack variable to avoid setting the newObject value before the class has finished its constructor) */ \
                alreadyInside = false; \
\
                _singletonInstance = newObject; \
            } \
        } \
\
        return _singletonInstance; \
    } \
\
    static inline classname* getInstanceWithoutCreating() throw() \
    { \
        return _singletonInstance; \
    } \
\
    static void deleteInstance() \
    { \
        if (_singletonInstance != 0) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = 0; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() throw() \
    { \
        if (_singletonInstance == this) \
            _singletonInstance = 0; \
    }

//==============================================================================
/**
    Macro to declare member variables and methods for a singleton class.

    This is like juce_DeclareSingleton_SingleThreaded, but doesn't do any checking
    for recursion or repeated instantiation. It's intended for use as a lightweight
    version of a singleton, where you're using it in very straightforward
    circumstances and don't need the extra checking.

    Juce use the normal juce_ImplementSingleton_SingleThreaded as the counterpart
    to this declaration, as you would with juce_DeclareSingleton_SingleThreaded.

    See the documentation for juce_DeclareSingleton for more information about
    how to use it, the only difference being that you have to use
    juce_ImplementSingleton_SingleThreaded instead of juce_ImplementSingleton.

    @see juce_ImplementSingleton_SingleThreaded, juce_DeclareSingleton
*/
#define juce_DeclareSingleton_SingleThreaded_Minimal(classname) \
\
    static classname* _singletonInstance;  \
\
    static classname* getInstance() \
    { \
        if (_singletonInstance == 0) \
            _singletonInstance = new classname(); \
\
        return _singletonInstance; \
    } \
\
    static inline classname* getInstanceWithoutCreating() throw() \
    { \
        return _singletonInstance; \
    } \
\
    static void deleteInstance() \
    { \
        if (_singletonInstance != 0) \
        { \
            classname* const old = _singletonInstance; \
            _singletonInstance = 0; \
            delete old; \
        } \
    } \
\
    void clearSingletonInstance() throw() \
    { \
        if (_singletonInstance == this) \
            _singletonInstance = 0; \
    }


//==============================================================================
/** This is a counterpart to the juce_DeclareSingleton_SingleThreaded macro.

    After adding juce_DeclareSingleton_SingleThreaded or juce_DeclareSingleton_SingleThreaded_Minimal
    to the class definition, this macro has to be used somewhere in the cpp file.
*/
#define juce_ImplementSingleton_SingleThreaded(classname) \
\
    classname* classname::_singletonInstance = 0;



#endif   // __JUCE_SINGLETON_JUCEHEADER__
