/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "juce_core.h"

#include <locale>
#include <cctype>
#include <cstdarg>

#if ! JUCE_ANDROID
 #include <sys/timeb.h>
 #include <cwctype>
#endif

#if JUCE_WINDOWS
 #include <ctime>

 #define _WINSOCK_DEPRECATED_NO_WARNINGS 1
 #include <winsock2.h>
 #include <ws2tcpip.h>

 #if ! JUCE_MINGW
  #pragma warning (push)
  #pragma warning (disable: 4091)
  #include <Dbghelp.h>
  #pragma warning (pop)

  #if ! JUCE_DONT_AUTOLINK_TO_WIN32_LIBRARIES
   #pragma comment (lib, "DbgHelp.lib")
  #endif
 #endif

 #if JUCE_MINGW
  #include <ws2spi.h>
 #endif

#else
 #if JUCE_LINUX || JUCE_ANDROID
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <sys/errno.h>
  #include <unistd.h>
  #include <netinet/in.h>
 #endif

 #if JUCE_LINUX
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

 #if ! JUCE_ANDROID
  #include <execinfo.h>
 #endif
#endif

#if JUCE_MAC || JUCE_IOS
 #include <xlocale.h>
 #include <mach/mach.h>
#endif

#if JUCE_ANDROID
 #include <android/log.h>
#endif

#undef check

//==============================================================================
#ifndef    JUCE_STANDALONE_APPLICATION
 JUCE_COMPILER_WARNING ("Please re-save your project with the latest Projucer version to avoid this warning")
 #define   JUCE_STANDALONE_APPLICATION 0
#endif

//==============================================================================
namespace juce
{

#include "containers/juce_AbstractFifo.cpp"
#include "containers/juce_NamedValueSet.cpp"
#include "containers/juce_PropertySet.cpp"
#include "containers/juce_Variant.cpp"
#include "files/juce_DirectoryIterator.cpp"
#include "files/juce_File.cpp"
#include "files/juce_FileInputStream.cpp"
#include "files/juce_FileOutputStream.cpp"
#include "files/juce_FileSearchPath.cpp"
#include "files/juce_TemporaryFile.cpp"
#include "javascript/juce_JSON.cpp"
#include "javascript/juce_Javascript.cpp"
#include "containers/juce_DynamicObject.cpp"
#include "logging/juce_FileLogger.cpp"
#include "logging/juce_Logger.cpp"
#include "maths/juce_BigInteger.cpp"
#include "maths/juce_Expression.cpp"
#include "maths/juce_Random.cpp"
#include "memory/juce_MemoryBlock.cpp"
#include "misc/juce_RuntimePermissions.cpp"
#include "misc/juce_Result.cpp"
#include "misc/juce_Uuid.cpp"
#include "network/juce_MACAddress.cpp"
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
#include "xml/juce_XmlDocument.cpp"
#include "xml/juce_XmlElement.cpp"
#include "zip/juce_GZIPDecompressorInputStream.cpp"
#include "zip/juce_GZIPCompressorOutputStream.cpp"
#include "zip/juce_ZipFile.cpp"
#include "files/juce_FileFilter.cpp"
#include "files/juce_WildcardFileFilter.cpp"

//==============================================================================
#if JUCE_ANDROID
#include "native/juce_android_JNIHelpers.h"
#endif

#if ! JUCE_WINDOWS
#include "native/juce_posix_SharedCode.h"
#include "native/juce_posix_NamedPipe.cpp"
#endif

//==============================================================================
#if JUCE_MAC || JUCE_IOS
#include "native/juce_mac_Files.mm"
#include "native/juce_mac_Network.mm"
#include "native/juce_mac_Strings.mm"
#include "native/juce_mac_SystemStats.mm"
#include "native/juce_mac_Threads.mm"

//==============================================================================
#elif JUCE_WINDOWS
#include "native/juce_win32_Files.cpp"
#include "native/juce_win32_Network.cpp"
#include "native/juce_win32_Registry.cpp"
#include "native/juce_win32_SystemStats.cpp"
#include "native/juce_win32_Threads.cpp"

//==============================================================================
#elif JUCE_LINUX
#include "native/juce_linux_CommonFile.cpp"
#include "native/juce_linux_Files.cpp"
#include "native/juce_linux_Network.cpp"
#if JUCE_USE_CURL
 #include "native/juce_curl_Network.cpp"
#endif
#include "native/juce_linux_SystemStats.cpp"
#include "native/juce_linux_Threads.cpp"

//==============================================================================
#elif JUCE_ANDROID
#include "native/juce_linux_CommonFile.cpp"
#include "native/juce_android_Files.cpp"
#include "native/juce_android_Misc.cpp"
#include "native/juce_android_Network.cpp"
#include "native/juce_android_SystemStats.cpp"
#include "native/juce_android_Threads.cpp"
#include "native/juce_android_RuntimePermissions.cpp"

#endif

#include "threads/juce_ChildProcess.cpp"
#include "threads/juce_HighResolutionTimer.cpp"
#include "network/juce_URL.cpp"

//==============================================================================
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
