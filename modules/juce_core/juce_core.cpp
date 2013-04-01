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

#if defined (__JUCE_CORE_JUCEHEADER__) && ! JUCE_AMALGAMATED_INCLUDE
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

// Your project must contain an AppConfig.h file with your project-specific settings in it,
// and your header search path must make it accessible to the module's files.
#include "AppConfig.h"

//==============================================================================
#include "native/juce_BasicNativeHeaders.h"
#include "juce_core.h"

#include <locale>
#include <cctype>
#include <sys/timeb.h>

#if ! JUCE_ANDROID
 #include <cwctype>
#endif

#if JUCE_WINDOWS
 #include <ctime>
 #include <winsock2.h>
 #include <ws2tcpip.h>

 #if ! JUCE_MINGW
  #include <Dbghelp.h>

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
 #endif

 #include <pwd.h>
 #include <fcntl.h>
 #include <netdb.h>
 #include <arpa/inet.h>
 #include <netinet/tcp.h>
 #include <sys/time.h>

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


//==============================================================================
namespace juce
{

#include "containers/juce_AbstractFifo.cpp"
#include "containers/juce_DynamicObject.cpp"
#include "containers/juce_NamedValueSet.cpp"
#include "containers/juce_PropertySet.cpp"
#include "containers/juce_Variant.cpp"
#include "files/juce_DirectoryIterator.cpp"
#include "files/juce_File.cpp"
#include "files/juce_FileInputStream.cpp"
#include "files/juce_FileOutputStream.cpp"
#include "files/juce_FileSearchPath.cpp"
#include "files/juce_TemporaryFile.cpp"
#include "json/juce_JSON.cpp"
#include "logging/juce_FileLogger.cpp"
#include "logging/juce_Logger.cpp"
#include "maths/juce_BigInteger.cpp"
#include "maths/juce_Expression.cpp"
#include "maths/juce_Random.cpp"
#include "memory/juce_MemoryBlock.cpp"
#include "misc/juce_Result.cpp"
#include "misc/juce_Uuid.cpp"
#include "network/juce_MACAddress.cpp"
#include "network/juce_NamedPipe.cpp"
#include "network/juce_Socket.cpp"
#include "network/juce_URL.cpp"
#include "streams/juce_BufferedInputStream.cpp"
#include "streams/juce_FileInputSource.cpp"
#include "streams/juce_InputStream.cpp"
#include "streams/juce_MemoryInputStream.cpp"
#include "streams/juce_MemoryOutputStream.cpp"
#include "streams/juce_OutputStream.cpp"
#include "streams/juce_SubregionStream.cpp"
#include "system/juce_SystemStats.cpp"
#include "text/juce_CharacterFunctions.cpp"
#include "text/juce_Identifier.cpp"
#include "text/juce_LocalisedStrings.cpp"
#include "text/juce_String.cpp"
#include "text/juce_StringArray.cpp"
#include "text/juce_StringPairArray.cpp"
#include "text/juce_StringPool.cpp"
#include "text/juce_TextDiff.cpp"
#include "threads/juce_ChildProcess.cpp"
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

//==============================================================================
#if JUCE_MAC || JUCE_IOS
#include "native/juce_osx_ObjCHelpers.h"
#endif

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
#include "native/juce_win32_ComSmartPtr.h"
#include "native/juce_win32_Files.cpp"
#include "native/juce_win32_Network.cpp"
#include "native/juce_win32_Registry.cpp"
#include "native/juce_win32_SystemStats.cpp"
#include "native/juce_win32_Threads.cpp"

//==============================================================================
#elif JUCE_LINUX
#include "native/juce_linux_Files.cpp"
#include "native/juce_linux_Network.cpp"
#include "native/juce_linux_SystemStats.cpp"
#include "native/juce_linux_Threads.cpp"

//==============================================================================
#elif JUCE_ANDROID
#include "native/juce_android_Files.cpp"
#include "native/juce_android_Misc.cpp"
#include "native/juce_android_Network.cpp"
#include "native/juce_android_SystemStats.cpp"
#include "native/juce_android_Threads.cpp"

#endif

#include "threads/juce_HighResolutionTimer.cpp"

}
