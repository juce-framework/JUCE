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

#ifndef JUCE_CORE_H_INCLUDED
#define JUCE_CORE_H_INCLUDED

#ifndef JUCE_MODULE_AVAILABLE_juce_core
 /* If you fail to make sure that all your compile units are building JUCE with the same set of
    option flags, then there's a risk that different compile units will treat the classes as having
    different memory layouts, leading to very nasty memory corruption errors when they all get
    linked together. That's why it's best to always include the Introjucer-generated AppConfig.h
    file before any juce headers.

    Note that if you do have an AppConfig.h file and hit this warning, it means that it doesn't
    contain the JUCE_MODULE_AVAILABLE_xxx flags, which are necessary for some inter-module
    functionality to work correctly. In that case, you should either rebuild your AppConfig.h with
    the latest introjucer, or fix it manually to contain these flags.
 */
 #ifdef _MSC_VER
  #pragma message ("Have you included your AppConfig.h file before including the JUCE headers?")
 #else
  #warning "Have you included your AppConfig.h file before including the JUCE headers?"
 #endif
#endif

#ifdef _MSC_VER
 #pragma warning (push)
 // Disable warnings for long class names, padding, and undefined preprocessor definitions.
 #pragma warning (disable: 4251 4786 4668 4820)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125)
 #endif
#endif

//==============================================================================
#include "system/juce_TargetPlatform.h"

//=============================================================================
/** Config: JUCE_FORCE_DEBUG

    Normally, JUCE_DEBUG is set to 1 or 0 based on compiler and project settings,
    but if you define this value, you can override this to force it to be true or false.
*/
#ifndef JUCE_FORCE_DEBUG
 //#define JUCE_FORCE_DEBUG 0
#endif

//=============================================================================
/** Config: JUCE_LOG_ASSERTIONS

    If this flag is enabled, the jassert and jassertfalse macros will always use Logger::writeToLog()
    to write a message when an assertion happens.

    Enabling it will also leave this turned on in release builds. When it's disabled,
    however, the jassert and jassertfalse macros will not be compiled in a
    release build.

    @see jassert, jassertfalse, Logger
*/
#ifndef JUCE_LOG_ASSERTIONS
 #if JUCE_ANDROID
  #define JUCE_LOG_ASSERTIONS 1
 #else
  #define JUCE_LOG_ASSERTIONS 0
 #endif
#endif

//=============================================================================
/** Config: JUCE_CHECK_MEMORY_LEAKS

    Enables a memory-leak check for certain objects when the app terminates. See the LeakedObjectDetector
    class and the JUCE_LEAK_DETECTOR macro for more details about enabling leak checking for specific classes.
*/
#if JUCE_DEBUG && ! defined (JUCE_CHECK_MEMORY_LEAKS)
 #define JUCE_CHECK_MEMORY_LEAKS 1
#endif

//=============================================================================
/** Config: JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES

    In a Visual C++  build, this can be used to stop the required system libs being
    automatically added to the link stage.
*/
#ifndef JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
 #define JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES 0
#endif

/** Config: JUCE_INCLUDE_ZLIB_CODE
    This can be used to disable Juce's embedded 3rd-party zlib code.
    You might need to tweak this if you're linking to an external zlib library in your app,
    but for normal apps, this option should be left alone.

    If you disable this, you might also want to set a value for JUCE_ZLIB_INCLUDE_PATH, to
    specify the path where your zlib headers live.
*/
#ifndef JUCE_INCLUDE_ZLIB_CODE
 #define JUCE_INCLUDE_ZLIB_CODE 1
#endif

#ifndef JUCE_ZLIB_INCLUDE_PATH
 #define JUCE_ZLIB_INCLUDE_PATH <zlib.h>
#endif

/*  Config: JUCE_CATCH_UNHANDLED_EXCEPTIONS
    If enabled, this will add some exception-catching code to forward unhandled exceptions
    to your JUCEApplicationBase::unhandledException() callback.
*/
#ifndef JUCE_CATCH_UNHANDLED_EXCEPTIONS
 //#define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1
#endif

#ifndef JUCE_STRING_UTF_TYPE
 #define JUCE_STRING_UTF_TYPE 8
#endif

//=============================================================================
//=============================================================================

#include "system/juce_StandardHeader.h"

namespace juce
{

class StringRef;
class MemoryBlock;
class File;
class InputStream;
class OutputStream;
class DynamicObject;
class FileInputStream;
class FileOutputStream;
class XmlElement;
class JSONFormatter;

extern JUCE_API bool JUCE_CALLTYPE juce_isRunningUnderDebugger();
extern JUCE_API void JUCE_CALLTYPE logAssertion (const char* file, int line) noexcept;

#include "memory/juce_Memory.h"
#include "maths/juce_MathsFunctions.h"
#include "memory/juce_ByteOrder.h"
#include "memory/juce_Atomic.h"
#include "text/juce_CharacterFunctions.h"

#if JUCE_MSVC
 #pragma warning (push)
 #pragma warning (disable: 4514 4996)
#endif

#include "text/juce_CharPointer_UTF8.h"
#include "text/juce_CharPointer_UTF16.h"
#include "text/juce_CharPointer_UTF32.h"
#include "text/juce_CharPointer_ASCII.h"

#if JUCE_MSVC
 #pragma warning (pop)
#endif

#include "text/juce_String.h"
#include "text/juce_StringRef.h"
#include "logging/juce_Logger.h"
#include "memory/juce_LeakedObjectDetector.h"
#include "memory/juce_ContainerDeletePolicy.h"
#include "memory/juce_HeapBlock.h"
#include "memory/juce_MemoryBlock.h"
#include "memory/juce_ReferenceCountedObject.h"
#include "memory/juce_ScopedPointer.h"
#include "memory/juce_OptionalScopedPointer.h"
#include "memory/juce_Singleton.h"
#include "memory/juce_WeakReference.h"
#include "threads/juce_ScopedLock.h"
#include "threads/juce_CriticalSection.h"
#include "maths/juce_Range.h"
#include "maths/juce_NormalisableRange.h"
#include "containers/juce_ElementComparator.h"
#include "containers/juce_ArrayAllocationBase.h"
#include "containers/juce_Array.h"
#include "containers/juce_LinkedListPointer.h"
#include "containers/juce_OwnedArray.h"
#include "containers/juce_ReferenceCountedArray.h"
#include "containers/juce_ScopedValueSetter.h"
#include "containers/juce_SortedSet.h"
#include "containers/juce_SparseSet.h"
#include "containers/juce_AbstractFifo.h"
#include "text/juce_NewLine.h"
#include "text/juce_StringPool.h"
#include "text/juce_Identifier.h"
#include "text/juce_StringArray.h"
#include "text/juce_StringPairArray.h"
#include "text/juce_TextDiff.h"
#include "text/juce_LocalisedStrings.h"
#include "misc/juce_Result.h"
#include "containers/juce_Variant.h"
#include "containers/juce_NamedValueSet.h"
#include "containers/juce_DynamicObject.h"
#include "containers/juce_HashMap.h"
#include "time/juce_RelativeTime.h"
#include "time/juce_Time.h"
#include "streams/juce_InputStream.h"
#include "streams/juce_OutputStream.h"
#include "streams/juce_BufferedInputStream.h"
#include "streams/juce_MemoryInputStream.h"
#include "streams/juce_MemoryOutputStream.h"
#include "streams/juce_SubregionStream.h"
#include "streams/juce_InputSource.h"
#include "files/juce_File.h"
#include "files/juce_DirectoryIterator.h"
#include "files/juce_FileInputStream.h"
#include "files/juce_FileOutputStream.h"
#include "files/juce_FileSearchPath.h"
#include "files/juce_MemoryMappedFile.h"
#include "files/juce_TemporaryFile.h"
#include "files/juce_FileFilter.h"
#include "files/juce_WildcardFileFilter.h"
#include "streams/juce_FileInputSource.h"
#include "logging/juce_FileLogger.h"
#include "javascript/juce_JSON.h"
#include "javascript/juce_Javascript.h"
#include "maths/juce_BigInteger.h"
#include "maths/juce_Expression.h"
#include "maths/juce_Random.h"
#include "misc/juce_Uuid.h"
#include "misc/juce_WindowsRegistry.h"
#include "system/juce_SystemStats.h"
#include "threads/juce_ChildProcess.h"
#include "threads/juce_DynamicLibrary.h"
#include "threads/juce_HighResolutionTimer.h"
#include "threads/juce_InterProcessLock.h"
#include "threads/juce_Process.h"
#include "threads/juce_SpinLock.h"
#include "threads/juce_WaitableEvent.h"
#include "threads/juce_Thread.h"
#include "threads/juce_ThreadLocalValue.h"
#include "threads/juce_ThreadPool.h"
#include "threads/juce_TimeSliceThread.h"
#include "threads/juce_ReadWriteLock.h"
#include "threads/juce_ScopedReadLock.h"
#include "threads/juce_ScopedWriteLock.h"
#include "network/juce_IPAddress.h"
#include "network/juce_MACAddress.h"
#include "network/juce_NamedPipe.h"
#include "network/juce_Socket.h"
#include "network/juce_URL.h"
#include "time/juce_PerformanceCounter.h"
#include "unit_tests/juce_UnitTest.h"
#include "xml/juce_XmlDocument.h"
#include "xml/juce_XmlElement.h"
#include "zip/juce_GZIPCompressorOutputStream.h"
#include "zip/juce_GZIPDecompressorInputStream.h"
#include "zip/juce_ZipFile.h"
#include "containers/juce_PropertySet.h"
#include "memory/juce_SharedResourcePointer.h"

}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

#endif   // JUCE_CORE_H_INCLUDED
