/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

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
  #define JUCE_MAIN_FUNCTION_ARGS  argc, (const char**) argv, nullptr
 #endif

 #define START_JUCE_APPLICATION(AppClass) \
    static juce::JUCEApplicationBase* juce_CreateApplication() { return new AppClass(); } \
    extern "C" JUCE_MAIN_FUNCTION \
    { \
        juce::JUCEApplicationBase::createInstance = &juce_CreateApplication; \
        return juce::JUCEApplicationBase::main (JUCE_MAIN_FUNCTION_ARGS); \
    }

 #if JUCE_IOS
  /**
      You can instruct JUCE to use a custom iOS app delegate class instaed of JUCE's default
      app delegate. For JUCE to work you must pass all messages to JUCE's internal app delegate.
      Below is an example of minimal forwarding custom delegate. Note that you are at your own
      risk if you decide to use your own delegate an subtle, hard to debug bugs may occur.

      @interface MyCustomDelegate : NSObject <UIApplicationDelegate> { NSObject<UIApplicationDelegate>* juceDelegate; } @end
      @implementation MyCustomDelegate
      -(id) init
      {
          self = [super init];
          juceDelegate = reinterpret_cast<NSObject<UIApplicationDelegate>*> ([[NSClassFromString (@"JuceAppStartupDelegate") alloc] init]);

          return self;
      }

      -(void)dealloc
      {
          [juceDelegate release];
          [super dealloc];
      }

      - (void)forwardInvocation:(NSInvocation *)anInvocation
      {
         if (juceDelegate != nullptr && [juceDelegate respondsToSelector:[anInvocation selector]])
             [anInvocation invokeWithTarget:juceDelegate];
        else
             [super forwardInvocation:anInvocation];
      }

      -(BOOL)respondsToSelector:(SEL)aSelector
      {
          if (juceDelegate != nullptr && [juceDelegate respondsToSelector:aSelector])
             return YES;

          return [super respondsToSelector:aSelector];
      }
      @end
  */
  #define START_JUCE_APPLICATION_WITH_CUSTOM_DELEGATE(AppClass, DelegateClass) \
     static juce::JUCEApplicationBase* juce_CreateApplication() { return new AppClass(); } \
     extern "C" JUCE_MAIN_FUNCTION \
     { \
         juce::JUCEApplicationBase::createInstance = &juce_CreateApplication; \
         return juce::JUCEApplicationBase::main (argc, (const char**) argv, [DelegateClass class]); \
     }
 #endif

#endif

#endif   // JUCE_INITIALISATION_H_INCLUDED
