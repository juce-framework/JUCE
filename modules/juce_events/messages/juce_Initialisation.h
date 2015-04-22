/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_INITIALISATION_H_INCLUDED
#define JUCE_INITIALISATION_H_INCLUDED


//==============================================================================
/** Initialises Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function before making any
    Juce calls, to make sure things are initialised correctly.

    Note that if you're creating a Juce DLL for Windows, you may also need to call the
    Process::setCurrentModuleInstanceHandle() method.

    @see shutdownJuce_GUI()
*/
JUCE_API void JUCE_CALLTYPE  initialiseJuce_GUI();

/** Clears up any static data being used by Juce's GUI classes.

    If you're embedding Juce into an application that uses its own event-loop rather
    than using the START_JUCE_APPLICATION macro, call this function in your shutdown
    code to clean up any juce objects that might be lying around.

    @see initialiseJuce_GUI()
*/
JUCE_API void JUCE_CALLTYPE  shutdownJuce_GUI();


//==============================================================================
/** A utility object that helps you initialise and shutdown Juce correctly
    using an RAII pattern.

    When the first instance of this class is created, it calls initialiseJuce_GUI(),
    and when the last instance is deleted, it calls shutdownJuce_GUI(), so that you
    can easily be sure that as long as at least one instance of the class exists, the
    library will be initialised.

    This class is particularly handy to use at the beginning of a console app's
    main() function, because it'll take care of shutting down whenever you return
    from the main() call.

    Be careful with your threading though - to be safe, you should always make sure
    that these objects are created and deleted on the message thread.
*/
class JUCE_API  ScopedJuceInitialiser_GUI
{
public:
    /** The constructor simply calls initialiseJuce_GUI(). */
    ScopedJuceInitialiser_GUI();

    /** The destructor simply calls shutdownJuce_GUI(). */
    ~ScopedJuceInitialiser_GUI();
};


//==============================================================================
/**
    To start a JUCE app, use this macro: START_JUCE_APPLICATION (AppSubClass) where
    AppSubClass is the name of a class derived from JUCEApplication or JUCEApplicationBase.

    See the JUCEApplication and JUCEApplicationBase class documentation for more details.
*/
#ifdef DOXYGEN
 #define START_JUCE_APPLICATION(AppClass)
#elif JUCE_ANDROID
 #define START_JUCE_APPLICATION(AppClass) \
   juce::JUCEApplicationBase* juce_CreateApplication() { return new AppClass(); }

#else
 #if JUCE_WINDOWS && ! defined (_CONSOLE)
  #define JUCE_MAIN_FUNCTION       int __stdcall WinMain (struct HINSTANCE__*, struct HINSTANCE__*, char*, int)
  #define JUCE_MAIN_FUNCTION_ARGS
 #else
  #define JUCE_MAIN_FUNCTION       int main (int argc, char* argv[])
  #define JUCE_MAIN_FUNCTION_ARGS  argc, (const char**) argv
 #endif

 #define START_JUCE_APPLICATION(AppClass) \
    static juce::JUCEApplicationBase* juce_CreateApplication() { return new AppClass(); } \
    extern "C" JUCE_MAIN_FUNCTION \
    { \
        juce::JUCEApplicationBase::createInstance = &juce_CreateApplication; \
        return juce::JUCEApplicationBase::main (JUCE_MAIN_FUNCTION_ARGS); \
    }
#endif

#endif   // JUCE_INITIALISATION_H_INCLUDED
