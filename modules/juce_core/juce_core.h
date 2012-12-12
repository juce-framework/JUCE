/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCE_CORE_JUCEHEADER__
#define __JUCE_CORE_JUCEHEADER__

/* This line is here as a sanity-check to catch syntax errors caused by mistakes in 3rd-party
   header files that have been included prior to this one. If you hit an error at this line,
   there's probably some kind of syntax problem in whatever code immediately precedes this header.

   It also causes an error if you attempt to build using a C or obj-C compiler rather than a C++ one.
*/
namespace DummyNamespaceStatementToCatchSyntaxErrors {}

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

    If this flag is enabled, the the jassert and jassertfalse macros will always use Logger::writeToLog()
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

/*  Config: JUCE_INCLUDE_ZLIB_CODE
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
    to your JUCEApplication::unhandledException() callback.
*/
#ifndef JUCE_CATCH_UNHANDLED_EXCEPTIONS
 //#define JUCE_CATCH_UNHANDLED_EXCEPTIONS 1
#endif

//=============================================================================
//=============================================================================
#if JUCE_MSVC
 #pragma warning (disable: 4251) // (DLL build warning, must be disabled before pushing the warning state)
 #pragma warning (push)
 #pragma warning (disable: 4786) // (long class name warning)
 #ifdef __INTEL_COMPILER
  #pragma warning (disable: 1125)
 #endif
#endif

#include "system/juce_StandardHeader.h"

namespace juce
{

// START_AUTOINCLUDE containers, files, json, logging, maths, memory, misc, network,
// streams, system, text, threads, time, unit_tests, xml, zip
#ifndef __JUCE_ABSTRACTFIFO_JUCEHEADER__
 #include "containers/juce_AbstractFifo.h"
#endif
#ifndef __JUCE_ARRAY_JUCEHEADER__
 #include "containers/juce_Array.h"
#endif
#ifndef __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
 #include "containers/juce_ArrayAllocationBase.h"
#endif
#ifndef __JUCE_DYNAMICOBJECT_JUCEHEADER__
 #include "containers/juce_DynamicObject.h"
#endif
#ifndef __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
 #include "containers/juce_ElementComparator.h"
#endif
#ifndef __JUCE_HASHMAP_JUCEHEADER__
 #include "containers/juce_HashMap.h"
#endif
#ifndef __JUCE_LINKEDLISTPOINTER_JUCEHEADER__
 #include "containers/juce_LinkedListPointer.h"
#endif
#ifndef __JUCE_NAMEDVALUESET_JUCEHEADER__
 #include "containers/juce_NamedValueSet.h"
#endif
#ifndef __JUCE_OWNEDARRAY_JUCEHEADER__
 #include "containers/juce_OwnedArray.h"
#endif
#ifndef __JUCE_PROPERTYSET_JUCEHEADER__
 #include "containers/juce_PropertySet.h"
#endif
#ifndef __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
 #include "containers/juce_ReferenceCountedArray.h"
#endif
#ifndef __JUCE_SCOPEDVALUESETTER_JUCEHEADER__
 #include "containers/juce_ScopedValueSetter.h"
#endif
#ifndef __JUCE_SORTEDSET_JUCEHEADER__
 #include "containers/juce_SortedSet.h"
#endif
#ifndef __JUCE_SPARSESET_JUCEHEADER__
 #include "containers/juce_SparseSet.h"
#endif
#ifndef __JUCE_VARIANT_JUCEHEADER__
 #include "containers/juce_Variant.h"
#endif
#ifndef __JUCE_DIRECTORYITERATOR_JUCEHEADER__
 #include "files/juce_DirectoryIterator.h"
#endif
#ifndef __JUCE_FILE_JUCEHEADER__
 #include "files/juce_File.h"
#endif
#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__
 #include "files/juce_FileInputStream.h"
#endif
#ifndef __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
 #include "files/juce_FileOutputStream.h"
#endif
#ifndef __JUCE_FILESEARCHPATH_JUCEHEADER__
 #include "files/juce_FileSearchPath.h"
#endif
#ifndef __JUCE_MEMORYMAPPEDFILE_JUCEHEADER__
 #include "files/juce_MemoryMappedFile.h"
#endif
#ifndef __JUCE_TEMPORARYFILE_JUCEHEADER__
 #include "files/juce_TemporaryFile.h"
#endif
#ifndef __JUCE_JSON_JUCEHEADER__
 #include "json/juce_JSON.h"
#endif
#ifndef __JUCE_FILELOGGER_JUCEHEADER__
 #include "logging/juce_FileLogger.h"
#endif
#ifndef __JUCE_LOGGER_JUCEHEADER__
 #include "logging/juce_Logger.h"
#endif
#ifndef __JUCE_BIGINTEGER_JUCEHEADER__
 #include "maths/juce_BigInteger.h"
#endif
#ifndef __JUCE_EXPRESSION_JUCEHEADER__
 #include "maths/juce_Expression.h"
#endif
#ifndef __JUCE_MATHSFUNCTIONS_JUCEHEADER__
 #include "maths/juce_MathsFunctions.h"
#endif
#ifndef __JUCE_RANDOM_JUCEHEADER__
 #include "maths/juce_Random.h"
#endif
#ifndef __JUCE_RANGE_JUCEHEADER__
 #include "maths/juce_Range.h"
#endif
#ifndef __JUCE_ATOMIC_JUCEHEADER__
 #include "memory/juce_Atomic.h"
#endif
#ifndef __JUCE_BYTEORDER_JUCEHEADER__
 #include "memory/juce_ByteOrder.h"
#endif
#ifndef __JUCE_HEAPBLOCK_JUCEHEADER__
 #include "memory/juce_HeapBlock.h"
#endif
#ifndef __JUCE_LEAKEDOBJECTDETECTOR_JUCEHEADER__
 #include "memory/juce_LeakedObjectDetector.h"
#endif
#ifndef __JUCE_MEMORY_JUCEHEADER__
 #include "memory/juce_Memory.h"
#endif
#ifndef __JUCE_MEMORYBLOCK_JUCEHEADER__
 #include "memory/juce_MemoryBlock.h"
#endif
#ifndef __JUCE_OPTIONALSCOPEDPOINTER_JUCEHEADER__
 #include "memory/juce_OptionalScopedPointer.h"
#endif
#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
 #include "memory/juce_ReferenceCountedObject.h"
#endif
#ifndef __JUCE_SCOPEDPOINTER_JUCEHEADER__
 #include "memory/juce_ScopedPointer.h"
#endif
#ifndef __JUCE_SINGLETON_JUCEHEADER__
 #include "memory/juce_Singleton.h"
#endif
#ifndef __JUCE_WEAKREFERENCE_JUCEHEADER__
 #include "memory/juce_WeakReference.h"
#endif
#ifndef __JUCE_RESULT_JUCEHEADER__
 #include "misc/juce_Result.h"
#endif
#ifndef __JUCE_UUID_JUCEHEADER__
 #include "misc/juce_Uuid.h"
#endif
#ifndef __JUCE_WINDOWSREGISTRY_JUCEHEADER__
 #include "misc/juce_WindowsRegistry.h"
#endif
#ifndef __JUCE_MACADDRESS_JUCEHEADER__
 #include "network/juce_MACAddress.h"
#endif
#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__
 #include "network/juce_NamedPipe.h"
#endif
#ifndef __JUCE_SOCKET_JUCEHEADER__
 #include "network/juce_Socket.h"
#endif
#ifndef __JUCE_URL_JUCEHEADER__
 #include "network/juce_URL.h"
#endif
#ifndef __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
 #include "streams/juce_BufferedInputStream.h"
#endif
#ifndef __JUCE_FILEINPUTSOURCE_JUCEHEADER__
 #include "streams/juce_FileInputSource.h"
#endif
#ifndef __JUCE_INPUTSOURCE_JUCEHEADER__
 #include "streams/juce_InputSource.h"
#endif
#ifndef __JUCE_INPUTSTREAM_JUCEHEADER__
 #include "streams/juce_InputStream.h"
#endif
#ifndef __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
 #include "streams/juce_MemoryInputStream.h"
#endif
#ifndef __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
 #include "streams/juce_MemoryOutputStream.h"
#endif
#ifndef __JUCE_OUTPUTSTREAM_JUCEHEADER__
 #include "streams/juce_OutputStream.h"
#endif
#ifndef __JUCE_SUBREGIONSTREAM_JUCEHEADER__
 #include "streams/juce_SubregionStream.h"
#endif
#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__
 #include "system/juce_PlatformDefs.h"
#endif
#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__
 #include "system/juce_StandardHeader.h"
#endif
#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__
 #include "system/juce_SystemStats.h"
#endif
#ifndef __JUCE_TARGETPLATFORM_JUCEHEADER__
 #include "system/juce_TargetPlatform.h"
#endif
#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
 #include "text/juce_CharacterFunctions.h"
#endif
#ifndef __JUCE_CHARPOINTER_ASCII_JUCEHEADER__
 #include "text/juce_CharPointer_ASCII.h"
#endif
#ifndef __JUCE_CHARPOINTER_UTF16_JUCEHEADER__
 #include "text/juce_CharPointer_UTF16.h"
#endif
#ifndef __JUCE_CHARPOINTER_UTF32_JUCEHEADER__
 #include "text/juce_CharPointer_UTF32.h"
#endif
#ifndef __JUCE_CHARPOINTER_UTF8_JUCEHEADER__
 #include "text/juce_CharPointer_UTF8.h"
#endif
#ifndef __JUCE_IDENTIFIER_JUCEHEADER__
 #include "text/juce_Identifier.h"
#endif
#ifndef __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
 #include "text/juce_LocalisedStrings.h"
#endif
#ifndef __JUCE_NEWLINE_JUCEHEADER__
 #include "text/juce_NewLine.h"
#endif
#ifndef __JUCE_STRING_JUCEHEADER__
 #include "text/juce_String.h"
#endif
#ifndef __JUCE_STRINGARRAY_JUCEHEADER__
 #include "text/juce_StringArray.h"
#endif
#ifndef __JUCE_STRINGPAIRARRAY_JUCEHEADER__
 #include "text/juce_StringPairArray.h"
#endif
#ifndef __JUCE_STRINGPOOL_JUCEHEADER__
 #include "text/juce_StringPool.h"
#endif
#ifndef __JUCE_TEXTDIFF_JUCEHEADER__
 #include "text/juce_TextDiff.h"
#endif
#ifndef __JUCE_CHILDPROCESS_JUCEHEADER__
 #include "threads/juce_ChildProcess.h"
#endif
#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__
 #include "threads/juce_CriticalSection.h"
#endif
#ifndef __JUCE_DYNAMICLIBRARY_JUCEHEADER__
 #include "threads/juce_DynamicLibrary.h"
#endif
#ifndef __JUCE_INTERPROCESSLOCK_JUCEHEADER__
 #include "threads/juce_InterProcessLock.h"
#endif
#ifndef __JUCE_PROCESS_JUCEHEADER__
 #include "threads/juce_Process.h"
#endif
#ifndef __JUCE_READWRITELOCK_JUCEHEADER__
 #include "threads/juce_ReadWriteLock.h"
#endif
#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__
 #include "threads/juce_ScopedLock.h"
#endif
#ifndef __JUCE_SCOPEDREADLOCK_JUCEHEADER__
 #include "threads/juce_ScopedReadLock.h"
#endif
#ifndef __JUCE_SCOPEDWRITELOCK_JUCEHEADER__
 #include "threads/juce_ScopedWriteLock.h"
#endif
#ifndef __JUCE_SPINLOCK_JUCEHEADER__
 #include "threads/juce_SpinLock.h"
#endif
#ifndef __JUCE_THREAD_JUCEHEADER__
 #include "threads/juce_Thread.h"
#endif
#ifndef __JUCE_THREADLOCALVALUE_JUCEHEADER__
 #include "threads/juce_ThreadLocalValue.h"
#endif
#ifndef __JUCE_THREADPOOL_JUCEHEADER__
 #include "threads/juce_ThreadPool.h"
#endif
#ifndef __JUCE_TIMESLICETHREAD_JUCEHEADER__
 #include "threads/juce_TimeSliceThread.h"
#endif
#ifndef __JUCE_WAITABLEEVENT_JUCEHEADER__
 #include "threads/juce_WaitableEvent.h"
#endif
#ifndef __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
 #include "time/juce_PerformanceCounter.h"
#endif
#ifndef __JUCE_RELATIVETIME_JUCEHEADER__
 #include "time/juce_RelativeTime.h"
#endif
#ifndef __JUCE_TIME_JUCEHEADER__
 #include "time/juce_Time.h"
#endif
#ifndef __JUCE_UNITTEST_JUCEHEADER__
 #include "unit_tests/juce_UnitTest.h"
#endif
#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__
 #include "xml/juce_XmlDocument.h"
#endif
#ifndef __JUCE_XMLELEMENT_JUCEHEADER__
 #include "xml/juce_XmlElement.h"
#endif
#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
 #include "zip/juce_GZIPCompressorOutputStream.h"
#endif
#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
 #include "zip/juce_GZIPDecompressorInputStream.h"
#endif
#ifndef __JUCE_ZIPFILE_JUCEHEADER__
 #include "zip/juce_ZipFile.h"
#endif
// END_AUTOINCLUDE

}

#if JUCE_MSVC
 #pragma warning (pop)
#endif

#endif   // __JUCE_CORE_JUCEHEADER__
