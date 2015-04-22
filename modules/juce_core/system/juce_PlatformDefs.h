/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

#ifndef JUCE_PLATFORMDEFS_H_INCLUDED
#define JUCE_PLATFORMDEFS_H_INCLUDED

//==============================================================================
/*  This file defines miscellaneous macros for debugging, assertions, etc.
*/

//==============================================================================
#ifdef JUCE_FORCE_DEBUG
 #undef JUCE_DEBUG

 #if JUCE_FORCE_DEBUG
  #define JUCE_DEBUG 1
 #endif
#endif

/** This macro defines the C calling convention used as the standard for Juce calls. */
#if JUCE_MSVC
 #define JUCE_CALLTYPE   __stdcall
 #define JUCE_CDECL      __cdecl
#else
 #define JUCE_CALLTYPE
 #define JUCE_CDECL
#endif

//==============================================================================
// Debugging and assertion macros

#if JUCE_LOG_ASSERTIONS || JUCE_DEBUG
 #define juce_LogCurrentAssertion    juce::logAssertion (__FILE__, __LINE__);
#else
 #define juce_LogCurrentAssertion
#endif

//==============================================================================
#if JUCE_IOS || JUCE_LINUX || JUCE_ANDROID || JUCE_PPC
  /** This will try to break into the debugger if the app is currently being debugged.
      If called by an app that's not being debugged, the behaiour isn't defined - it may crash or not, depending
      on the platform.
      @see jassert()
  */
  #define juce_breakDebugger        { ::kill (0, SIGTRAP); }
#elif JUCE_USE_MSVC_INTRINSICS
  #ifndef __INTEL_COMPILER
    #pragma intrinsic (__debugbreak)
  #endif
  #define juce_breakDebugger        { __debugbreak(); }
#elif JUCE_GCC || JUCE_MAC
  #if JUCE_NO_INLINE_ASM
   #define juce_breakDebugger       { }
  #else
   #define juce_breakDebugger       { asm ("int $3"); }
  #endif
#else
  #define juce_breakDebugger        { __asm int 3 }
#endif

#if JUCE_CLANG && defined (__has_feature) && ! defined (JUCE_ANALYZER_NORETURN)
 #if __has_feature (attribute_analyzer_noreturn)
  inline void __attribute__((analyzer_noreturn)) juce_assert_noreturn() {}
  #define JUCE_ANALYZER_NORETURN juce_assert_noreturn();
 #endif
#endif

#ifndef JUCE_ANALYZER_NORETURN
 #define JUCE_ANALYZER_NORETURN
#endif

//==============================================================================
#if JUCE_MSVC && ! DOXYGEN
 #define MACRO_WITH_FORCED_SEMICOLON(x) \
   __pragma(warning(push)) \
   __pragma(warning(disable:4127)) \
   do { x } while (false) \
   __pragma(warning(pop))
#else
 /** This is the good old C++ trick for creating a macro that forces the user to put
    a semicolon after it when they use it.
 */
 #define MACRO_WITH_FORCED_SEMICOLON(x) do { x } while (false)
#endif

//==============================================================================
#if JUCE_DEBUG || DOXYGEN
  /** Writes a string to the standard error stream.
      This is only compiled in a debug build.
      @see Logger::outputDebugString
  */
  #define DBG(dbgtext)              MACRO_WITH_FORCED_SEMICOLON (juce::String tempDbgBuf; tempDbgBuf << dbgtext; juce::Logger::outputDebugString (tempDbgBuf);)

  //==============================================================================
  /** This will always cause an assertion failure.
      It is only compiled in a debug build, (unless JUCE_LOG_ASSERTIONS is enabled for your build).
      @see jassert
  */
  #define jassertfalse              MACRO_WITH_FORCED_SEMICOLON (juce_LogCurrentAssertion; if (juce::juce_isRunningUnderDebugger()) juce_breakDebugger; JUCE_ANALYZER_NORETURN)

  //==============================================================================
  /** Platform-independent assertion macro.

      This macro gets turned into a no-op when you're building with debugging turned off, so be
      careful that the expression you pass to it doesn't perform any actions that are vital for the
      correct behaviour of your program!
      @see jassertfalse
  */
  #define jassert(expression)       MACRO_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)

#else
  //==============================================================================
  // If debugging is disabled, these dummy debug and assertion macros are used..

  #define DBG(dbgtext)
  #define jassertfalse              MACRO_WITH_FORCED_SEMICOLON (juce_LogCurrentAssertion)

  #if JUCE_LOG_ASSERTIONS
   #define jassert(expression)      MACRO_WITH_FORCED_SEMICOLON (if (! (expression)) jassertfalse;)
  #else
   #define jassert(a)               MACRO_WITH_FORCED_SEMICOLON ( ; )
  #endif

#endif

//==============================================================================
#ifndef DOXYGEN
namespace juce
{
    template <bool b> struct JuceStaticAssert;
    template <>       struct JuceStaticAssert<true> { static void dummy() {} };
}
#endif

/** A compile-time assertion macro.
    If the expression parameter is false, the macro will cause a compile error. (The actual error
    message that the compiler generates may be completely bizarre and seem to have no relation to
    the place where you put the static_assert though!)
*/
#define static_jassert(expression)      juce::JuceStaticAssert<expression>::dummy();

/** This is a shorthand macro for declaring stubs for a class's copy constructor and operator=.

    For example, instead of
    @code
    class MyClass
    {
        etc..

    private:
        MyClass (const MyClass&);
        MyClass& operator= (const MyClass&);
    };@endcode

    ..you can just write:

    @code
    class MyClass
    {
        etc..

    private:
        JUCE_DECLARE_NON_COPYABLE (MyClass)
    };@endcode
*/
#define JUCE_DECLARE_NON_COPYABLE(className) \
    className (const className&) JUCE_DELETED_FUNCTION;\
    className& operator= (const className&) JUCE_DELETED_FUNCTION;

/** This is a shorthand way of writing both a JUCE_DECLARE_NON_COPYABLE and
    JUCE_LEAK_DETECTOR macro for a class.
*/
#define JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(className) \
    JUCE_DECLARE_NON_COPYABLE(className) \
    JUCE_LEAK_DETECTOR(className)

/** This macro can be added to class definitions to disable the use of new/delete to
    allocate the object on the heap, forcing it to only be used as a stack or member variable.
*/
#define JUCE_PREVENT_HEAP_ALLOCATION \
   private: \
    static void* operator new (size_t) JUCE_DELETED_FUNCTION; \
    static void operator delete (void*) JUCE_DELETED_FUNCTION;


//==============================================================================
#if ! DOXYGEN
 #define JUCE_JOIN_MACRO_HELPER(a, b) a ## b
 #define JUCE_STRINGIFY_MACRO_HELPER(a) #a
#endif

/** A good old-fashioned C macro concatenation helper.
    This combines two items (which may themselves be macros) into a single string,
    avoiding the pitfalls of the ## macro operator.
*/
#define JUCE_JOIN_MACRO(item1, item2)  JUCE_JOIN_MACRO_HELPER (item1, item2)

/** A handy C macro for stringifying any symbol, rather than just a macro parameter.
*/
#define JUCE_STRINGIFY(item)  JUCE_STRINGIFY_MACRO_HELPER (item)


//==============================================================================
#if JUCE_MSVC && ! defined (DOXYGEN)
 #define JUCE_WARNING_HELPER(file, line, mess) message(file "(" JUCE_STRINGIFY (line) ") : Warning: " #mess)
 #define JUCE_COMPILER_WARNING(message)  __pragma(JUCE_WARNING_HELPER (__FILE__, __LINE__, message));
#else
 #ifndef DOXYGEN
  #define JUCE_WARNING_HELPER(mess) message(#mess)
 #endif

 /** This macro allows you to emit a custom compiler warning message.
     Very handy for marking bits of code as "to-do" items, or for shaming
     code written by your co-workers in a way that's hard to ignore.

     GCC and Clang provide the \#warning directive, but MSVC doesn't, so this macro
     is a cross-compiler way to get the same functionality as \#warning.
 */
 #define JUCE_COMPILER_WARNING(message)  _Pragma(JUCE_STRINGIFY (JUCE_WARNING_HELPER (message)));
#endif


//==============================================================================
#if JUCE_CATCH_UNHANDLED_EXCEPTIONS

  #define JUCE_TRY try

  #define JUCE_CATCH_ALL            catch (...) {}
  #define JUCE_CATCH_ALL_ASSERT     catch (...) { jassertfalse; }

  #if ! JUCE_MODULE_AVAILABLE_juce_gui_basics
    #define JUCE_CATCH_EXCEPTION    JUCE_CATCH_ALL
  #else
    /** Used in try-catch blocks, this macro will send exceptions to the JUCEApplicationBase
        object so they can be logged by the application if it wants to.
    */
    #define JUCE_CATCH_EXCEPTION \
      catch (const std::exception& e)  \
      { \
          juce::JUCEApplicationBase::sendUnhandledException (&e, __FILE__, __LINE__); \
      } \
      catch (...) \
      { \
          juce::JUCEApplicationBase::sendUnhandledException (nullptr, __FILE__, __LINE__); \
      }
  #endif

#else

  #define JUCE_TRY
  #define JUCE_CATCH_EXCEPTION
  #define JUCE_CATCH_ALL
  #define JUCE_CATCH_ALL_ASSERT

#endif

//==============================================================================
#if JUCE_DEBUG || DOXYGEN
  /** A platform-independent way of forcing an inline function.
      Use the syntax: @code
      forcedinline void myfunction (int x)
      @endcode
  */
  #define forcedinline  inline
#else
  #if JUCE_MSVC
   #define forcedinline       __forceinline
  #else
   #define forcedinline       inline __attribute__((always_inline))
  #endif
#endif

#if JUCE_MSVC || DOXYGEN
  /** This can be placed before a stack or member variable declaration to tell the compiler
      to align it to the specified number of bytes. */
  #define JUCE_ALIGN(bytes)   __declspec (align (bytes))
#else
  #define JUCE_ALIGN(bytes)   __attribute__ ((aligned (bytes)))
#endif

//==============================================================================
// Cross-compiler deprecation macros..
#ifdef DOXYGEN
 /** This macro can be used to wrap a function which has been deprecated. */
 #define JUCE_DEPRECATED(functionDef)
 #define JUCE_DEPRECATED_WITH_BODY(functionDef, body)
#elif JUCE_MSVC && ! JUCE_NO_DEPRECATION_WARNINGS
 #define JUCE_DEPRECATED(functionDef)                   __declspec(deprecated) functionDef
 #define JUCE_DEPRECATED_WITH_BODY(functionDef, body)   __declspec(deprecated) functionDef body
#elif JUCE_GCC && ! JUCE_NO_DEPRECATION_WARNINGS
 #define JUCE_DEPRECATED(functionDef)                   functionDef __attribute__ ((deprecated))
 #define JUCE_DEPRECATED_WITH_BODY(functionDef, body)   functionDef __attribute__ ((deprecated)) body
#else
 #define JUCE_DEPRECATED(functionDef)                   functionDef
 #define JUCE_DEPRECATED_WITH_BODY(functionDef, body)   functionDef body
#endif

//==============================================================================
#if JUCE_ANDROID && ! DOXYGEN
 #define JUCE_MODAL_LOOPS_PERMITTED 0
#elif ! defined (JUCE_MODAL_LOOPS_PERMITTED)
 /** Some operating environments don't provide a modal loop mechanism, so this flag can be
     used to disable any functions that try to run a modal loop. */
 #define JUCE_MODAL_LOOPS_PERMITTED 1
#endif

//==============================================================================
#if JUCE_GCC
 #define JUCE_PACKED __attribute__((packed))
#elif ! DOXYGEN
 #define JUCE_PACKED
#endif

#endif   // JUCE_PLATFORMDEFS_H_INCLUDED
