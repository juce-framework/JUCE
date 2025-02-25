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

#ifdef JUCE_CORE_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#define JUCE_CORE_INCLUDE_OBJC_HELPERS 1
#define JUCE_CORE_INCLUDE_COM_SMART_PTR 1
#define JUCE_CORE_INCLUDE_NATIVE_HEADERS 1
#define JUCE_CORE_INCLUDE_JNI_HELPERS 1

#include "juce_core.h"

#include <cctype>
#include <cstdarg>
#include <locale>
#include <thread>

#if ! (JUCE_ANDROID || JUCE_BSD)
 #include <sys/timeb.h>
 #include <cwctype>
#endif

#if JUCE_WINDOWS
 #include <ctime>

 JUCE_BEGIN_IGNORE_WARNINGS_MSVC (4091)
 #include <Dbghelp.h>
 JUCE_END_IGNORE_WARNINGS_MSVC

 #if ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
  #pragma comment (lib, "DbgHelp.lib")
 #endif

#else
 #if JUCE_LINUX || JUCE_BSD || JUCE_ANDROID
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
 #endif

 #if JUCE_WASM
  #include <stdio.h>
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
  #include <sys/stat.h>
 #endif

 #if JUCE_LINUX || JUCE_BSD
  #include <stdio.h>
  #include <langinfo.h>
  #include <ifaddrs.h>
  #include <sys/resource.h>

  #if JUCE_USE_CURL
   #include <curl/curl.h>
  #endif
 #endif

 #include <pwd.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <netinet/tcp.h>
 #include <sys/time.h>
 #include <net/if.h>
 #include <sys/ioctl.h>

 #if ! (JUCE_ANDROID || JUCE_WASM)
  #include <execinfo.h>
 #endif
#endif

#if JUCE_MAC || JUCE_IOS
 #include <xlocale.h>
 #include <mach/mach.h>
#endif

#if JUCE_ANDROID
 #include <ifaddrs.h>
 #include <android/log.h>
#endif

#undef check

//==============================================================================
#include "containers/juce_AbstractFifo.cpp"
#include "containers/juce_ArrayBase.cpp"
#include "containers/juce_NamedValueSet.cpp"
#include "containers/juce_OwnedArray.cpp"
#include "containers/juce_PropertySet.cpp"
#include "containers/juce_ReferenceCountedArray.cpp"
#include "containers/juce_SparseSet.cpp"
#include "files/juce_DirectoryIterator.cpp"
#include "files/juce_RangedDirectoryIterator.cpp"
#include "files/juce_File.cpp"
#include "files/juce_FileInputStream.cpp"
#include "files/juce_FileOutputStream.cpp"
#include "files/juce_FileSearchPath.cpp"
#include "files/juce_TemporaryFile.cpp"
#include "logging/juce_FileLogger.cpp"
#include "logging/juce_Logger.cpp"
#include "maths/juce_BigInteger.cpp"
#include "maths/juce_Expression.cpp"
#include "maths/juce_Random.cpp"
#include "memory/juce_MemoryBlock.cpp"
#include "memory/juce_AllocationHooks.cpp"
#include "misc/juce_RuntimePermissions.cpp"
#include "misc/juce_Result.cpp"
#include "misc/juce_Uuid.cpp"
#include "misc/juce_ConsoleApplication.cpp"
#include "misc/juce_ScopeGuard.cpp"
#include "network/juce_MACAddress.cpp"

#if ! JUCE_WINDOWS
 #include "native/juce_SharedCode_posix.h"
 #include "native/juce_NamedPipe_posix.cpp"
#else
 #include "native/juce_Files_windows.cpp"
#endif

#include "zip/juce_zlib.h"
#include "network/juce_NamedPipe.cpp"
#include "network/juce_Socket.cpp"
#include "network/juce_IPAddress.cpp"
#include "streams/juce_BufferedInputStream.cpp"
#include "streams/juce_FileInputSource.cpp"
#include "streams/juce_InputStream.cpp"
#include "streams/juce_MemoryInputStream.cpp"
#include "streams/juce_MemoryOutputStream.cpp"
#include "streams/juce_SubregionStream.cpp"
#include "system/juce_SystemStats.cpp"
#include "text/juce_CharacterFunctions.cpp"
#include "text/juce_Identifier.cpp"
#include "text/juce_LocalisedStrings.cpp"
#include "text/juce_String.cpp"
#include "streams/juce_OutputStream.cpp"
#include "text/juce_StringArray.cpp"
#include "text/juce_StringPairArray.cpp"
#include "text/juce_StringPool.cpp"
#include "text/juce_TextDiff.cpp"
#include "text/juce_Base64.cpp"
#include "threads/juce_ReadWriteLock.cpp"
#include "threads/juce_Thread.cpp"
#include "threads/juce_ThreadPool.cpp"
#include "threads/juce_TimeSliceThread.cpp"
#include "time/juce_PerformanceCounter.cpp"
#include "time/juce_RelativeTime.cpp"
#include "time/juce_Time.cpp"
#include "unit_tests/juce_UnitTest.cpp"
#include "containers/juce_Variant.cpp"
#include "json/juce_JSON.cpp"
#include "json/juce_JSONUtils.cpp"
#include "containers/juce_DynamicObject.cpp"
#include "xml/juce_XmlDocument.cpp"
#include "xml/juce_XmlElement.cpp"
#include "zip/juce_GZIPDecompressorInputStream.cpp"
#include "zip/juce_GZIPCompressorOutputStream.cpp"
#include "zip/juce_ZipFile.cpp"
#include "files/juce_FileFilter.cpp"
#include "files/juce_WildcardFileFilter.cpp"
#include "native/juce_ThreadPriorities_native.h"
#include "native/juce_PlatformTimerListener.h"

//==============================================================================
#if ! JUCE_WINDOWS && (! JUCE_ANDROID || __ANDROID_API__ >= 24)
 #include "native/juce_IPAddress_posix.h"
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS
 #include "native/juce_Files_mac.mm"
 #include "native/juce_Network_mac.mm"
 #include "native/juce_Strings_mac.mm"
 #include "native/juce_SharedCode_intel.h"
 #include "native/juce_SystemStats_mac.mm"
 #include "native/juce_Threads_mac.mm"
 #include "native/juce_PlatformTimer_generic.cpp"
 #include "native/juce_Process_mac.mm"

//==============================================================================
#elif JUCE_WINDOWS
 #include "native/juce_Network_windows.cpp"
 #include "native/juce_Registry_windows.cpp"
 #include "native/juce_SystemStats_windows.cpp"
 #include "native/juce_Threads_windows.cpp"
 #include "native/juce_PlatformTimer_generic.cpp"
 #include "native/juce_PlatformTimer_windows.cpp"

//==============================================================================
#elif JUCE_LINUX
 #include "native/juce_CommonFile_linux.cpp"
 #include "native/juce_Files_linux.cpp"
 #include "native/juce_Network_linux.cpp"
 #if JUCE_USE_CURL
  #include "native/juce_Network_curl.cpp"
 #endif
 #include "native/juce_SystemStats_linux.cpp"
 #include "native/juce_Threads_linux.cpp"
 #include "native/juce_PlatformTimer_generic.cpp"

//==============================================================================
#elif JUCE_BSD
 #include "native/juce_CommonFile_linux.cpp"
 #include "native/juce_Files_linux.cpp"
 #include "native/juce_Network_linux.cpp"
 #if JUCE_USE_CURL
  #include "native/juce_Network_curl.cpp"
 #endif
 #include "native/juce_SharedCode_intel.h"
 #include "native/juce_SystemStats_linux.cpp"
 #include "native/juce_Threads_linux.cpp"
 #include "native/juce_PlatformTimer_generic.cpp"

//==============================================================================
#elif JUCE_ANDROID
 #include "native/juce_CommonFile_linux.cpp"
 #include "native/juce_JNIHelpers_android.cpp"
 #include "native/juce_Files_android.cpp"
 #include "native/juce_Misc_android.cpp"
 #include "native/juce_Network_android.cpp"
 #include "native/juce_SystemStats_android.cpp"
 #include "native/juce_Threads_android.cpp"
 #include "native/juce_RuntimePermissions_android.cpp"
 #include "native/juce_PlatformTimer_generic.cpp"

//==============================================================================
#elif JUCE_WASM
 #include "native/juce_SystemStats_wasm.cpp"
 #include "native/juce_PlatformTimer_generic.cpp"
#endif

#include "files/juce_common_MimeTypes.h"
#include "files/juce_common_MimeTypes.cpp"
#include "native/juce_AndroidDocument_android.cpp"
#include "threads/juce_HighResolutionTimer.cpp"
#include "threads/juce_WaitableEvent.cpp"
#include "network/juce_URL.cpp"

#if ! JUCE_WASM
 #include "threads/juce_ChildProcess.cpp"
 #include "network/juce_WebInputStream.cpp"
 #include "streams/juce_URLInputSource.cpp"
#endif

//==============================================================================
#if JUCE_UNIT_TESTS
 #include "containers/juce_HashMap_test.cpp"
 #include "containers/juce_Optional_test.cpp"
 #include "containers/juce_Enumerate_test.cpp"
 #include "containers/juce_ListenerList_test.cpp"
 #include "maths/juce_MathsFunctions_test.cpp"
 #include "misc/juce_EnumHelpers_test.cpp"
 #include "containers/juce_FixedSizeFunction_test.cpp"
 #include "json/juce_JSONSerialisation_test.cpp"
 #include "memory/juce_SharedResourcePointer_test.cpp"
 #include "text/juce_CharPointer_UTF8_test.cpp"
 #include "text/juce_CharPointer_UTF16_test.cpp"
 #include "text/juce_CharPointer_UTF32_test.cpp"
 #if JUCE_MAC || JUCE_IOS
  #include "native/juce_ObjCHelpers_mac_test.mm"
 #endif
#endif

//==============================================================================
namespace juce
{
/*
    As the very long class names here try to explain, the purpose of this code is to cause
    a linker error if not all of your compile units are consistent in the options that they
    enable before including JUCE headers. The reason this is important is that if you have
    two cpp files, and one includes the juce headers with debug enabled, and the other doesn't,
    then each will be generating code with different memory layouts for the classes, and
    you'll get subtle and hard-to-track-down memory corruption bugs!
*/
#if JUCE_DEBUG
 this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode
    ::this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_debug_mode() noexcept {}
#else
 this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode
    ::this_will_fail_to_link_if_some_of_your_compile_units_are_built_in_release_mode() noexcept {}
#endif
}
