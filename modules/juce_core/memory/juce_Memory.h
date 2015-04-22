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

#ifndef JUCE_MEMORY_H_INCLUDED
#define JUCE_MEMORY_H_INCLUDED

//==============================================================================
/** Fills a block of memory with zeros. */
inline void zeromem (void* memory, size_t numBytes) noexcept        { memset (memory, 0, numBytes); }

/** Overwrites a structure or object with zeros. */
template <typename Type>
inline void zerostruct (Type& structure) noexcept                   { memset (&structure, 0, sizeof (structure)); }

/** Delete an object pointer, and sets the pointer to null.

    Remember that it's not good c++ practice to use delete directly - always try to use a ScopedPointer
    or other automatic lifetime-management system rather than resorting to deleting raw pointers!
*/
template <typename Type>
inline void deleteAndZero (Type& pointer)                           { delete pointer; pointer = nullptr; }

/** A handy function which adds a number of bytes to any type of pointer and returns the result.
    This can be useful to avoid casting pointers to a char* and back when you want to move them by
    a specific number of bytes,
*/
template <typename Type, typename IntegerType>
inline Type* addBytesToPointer (Type* pointer, IntegerType bytes) noexcept  { return (Type*) (((char*) pointer) + bytes); }

/** A handy function which returns the difference between any two pointers, in bytes.
    The address of the second pointer is subtracted from the first, and the difference in bytes is returned.
*/
template <typename Type1, typename Type2>
inline int getAddressDifference (Type1* pointer1, Type2* pointer2) noexcept  { return (int) (((const char*) pointer1) - (const char*) pointer2); }

/** If a pointer is non-null, this returns a new copy of the object that it points to, or safely returns
    nullptr if the pointer is null.
*/
template <class Type>
inline Type* createCopyIfNotNull (const Type* pointer)     { return pointer != nullptr ? new Type (*pointer) : nullptr; }

//==============================================================================
#if JUCE_MAC || JUCE_IOS || DOXYGEN

 /** A handy C++ wrapper that creates and deletes an NSAutoreleasePool object using RAII.
     You should use the JUCE_AUTORELEASEPOOL macro to create a local auto-release pool on the stack.
 */
 class JUCE_API  ScopedAutoReleasePool
 {
 public:
     ScopedAutoReleasePool();
     ~ScopedAutoReleasePool();

 private:
     void* pool;

     JUCE_DECLARE_NON_COPYABLE (ScopedAutoReleasePool)
 };

 /** A macro that can be used to easily declare a local ScopedAutoReleasePool
     object for RAII-based obj-C autoreleasing.
     Because this may use the \@autoreleasepool syntax, you must follow the macro with
     a set of braces to mark the scope of the pool.
 */
#if (JUCE_COMPILER_SUPPORTS_ARC && defined (__OBJC__)) || DOXYGEN
 #define JUCE_AUTORELEASEPOOL  @autoreleasepool
#else
 #define JUCE_AUTORELEASEPOOL  const juce::ScopedAutoReleasePool JUCE_JOIN_MACRO (autoReleasePool_, __LINE__);
#endif

#else
 #define JUCE_AUTORELEASEPOOL
#endif

//==============================================================================
/* In a Windows DLL build, we'll expose some malloc/free functions that live inside the DLL, and use these for
   allocating all the objects - that way all juce objects in the DLL and in the host will live in the same heap,
   avoiding problems when an object is created in one module and passed across to another where it is deleted.
   By piggy-backing on the JUCE_LEAK_DETECTOR macro, these allocators can be injected into most juce classes.
*/
#if JUCE_MSVC && (defined (JUCE_DLL) || defined (JUCE_DLL_BUILD)) && ! (JUCE_DISABLE_DLL_ALLOCATORS || DOXYGEN)
 extern JUCE_API void* juceDLL_malloc (size_t);
 extern JUCE_API void  juceDLL_free (void*);

 #define JUCE_LEAK_DETECTOR(OwnerClass)  public:\
    static void* operator new (size_t sz)           { return juce::juceDLL_malloc (sz); } \
    static void* operator new (size_t, void* p)     { return p; } \
    static void operator delete (void* p)           { juce::juceDLL_free (p); } \
    static void operator delete (void*, void*)      {}
#endif

//==============================================================================
/** (Deprecated) This was a Windows-specific way of checking for object leaks - now please
    use the JUCE_LEAK_DETECTOR instead.
*/
#ifndef juce_UseDebuggingNewOperator
 #define juce_UseDebuggingNewOperator
#endif


#endif   // JUCE_MEMORY_H_INCLUDED
