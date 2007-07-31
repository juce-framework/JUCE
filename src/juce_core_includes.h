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

#ifndef __JUCE_JUCE_CORE_INCLUDES_INCLUDEFILES__
#define __JUCE_JUCE_CORE_INCLUDES_INCLUDEFILES__

#ifndef __JUCE_ATOMIC_JUCEHEADER__
 #include "juce_core/basics/juce_Atomic.h"
#endif
#ifndef __JUCE_DATACONVERSIONS_JUCEHEADER__
 #include "juce_core/basics/juce_DataConversions.h"
#endif
#ifndef __JUCE_FILELOGGER_JUCEHEADER__
 #include "juce_core/basics/juce_FileLogger.h"
#endif
#ifndef __JUCE_INITIALISATION_JUCEHEADER__
 #include "juce_core/basics/juce_Initialisation.h"
#endif
#ifndef __JUCE_LOGGER_JUCEHEADER__
 #include "juce_core/basics/juce_Logger.h"
#endif
#ifndef __JUCE_MATHSFUNCTIONS_JUCEHEADER__
 #include "juce_core/basics/juce_MathsFunctions.h"
#endif
#ifndef __JUCE_MEMORY_JUCEHEADER__
 #include "juce_core/basics/juce_Memory.h"
#endif
#ifndef __JUCE_PLATFORMDEFS_JUCEHEADER__
 #include "juce_core/basics/juce_PlatformDefs.h"
#endif
#ifndef __JUCE_RANDOM_JUCEHEADER__
 #include "juce_core/basics/juce_Random.h"
#endif
#ifndef __JUCE_RELATIVETIME_JUCEHEADER__
 #include "juce_core/basics/juce_RelativeTime.h"
#endif
#ifndef __JUCE_SINGLETON_JUCEHEADER__
 #include "juce_core/basics/juce_Singleton.h"
#endif
#ifndef __JUCE_STANDARDHEADER_JUCEHEADER__
 #include "juce_core/basics/juce_StandardHeader.h"
#endif
#ifndef __JUCE_SYSTEMSTATS_JUCEHEADER__
 #include "juce_core/basics/juce_SystemStats.h"
#endif
#ifndef __JUCE_TIME_JUCEHEADER__
 #include "juce_core/basics/juce_Time.h"
#endif
#ifndef __JUCE_ARRAY_JUCEHEADER__
 #include "juce_core/containers/juce_Array.h"
#endif
#ifndef __JUCE_ARRAYALLOCATIONBASE_JUCEHEADER__
 #include "juce_core/containers/juce_ArrayAllocationBase.h"
#endif
#ifndef __JUCE_BITARRAY_JUCEHEADER__
 #include "juce_core/containers/juce_BitArray.h"
#endif
#ifndef __JUCE_ELEMENTCOMPARATOR_JUCEHEADER__
 #include "juce_core/containers/juce_ElementComparator.h"
#endif
#ifndef __JUCE_MEMORYBLOCK_JUCEHEADER__
 #include "juce_core/containers/juce_MemoryBlock.h"
#endif
#ifndef __JUCE_OWNEDARRAY_JUCEHEADER__
 #include "juce_core/containers/juce_OwnedArray.h"
#endif
#ifndef __JUCE_PROPERTYSET_JUCEHEADER__
 #include "juce_core/containers/juce_PropertySet.h"
#endif
#ifndef __JUCE_REFERENCECOUNTEDARRAY_JUCEHEADER__
 #include "juce_core/containers/juce_ReferenceCountedArray.h"
#endif
#ifndef __JUCE_REFERENCECOUNTEDOBJECT_JUCEHEADER__
 #include "juce_core/containers/juce_ReferenceCountedObject.h"
#endif
#ifndef __JUCE_SORTEDSET_JUCEHEADER__
 #include "juce_core/containers/juce_SortedSet.h"
#endif
#ifndef __JUCE_SPARSESET_JUCEHEADER__
 #include "juce_core/containers/juce_SparseSet.h"
#endif
#ifndef __JUCE_VOIDARRAY_JUCEHEADER__
 #include "juce_core/containers/juce_VoidArray.h"
#endif
#ifndef __JUCE_INPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/juce_InputStream.h"
#endif
#ifndef __JUCE_OUTPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/juce_OutputStream.h"
#endif
#ifndef __JUCE_DIRECTORYITERATOR_JUCEHEADER__
 #include "juce_core/io/files/juce_DirectoryIterator.h"
#endif
#ifndef __JUCE_FILE_JUCEHEADER__
 #include "juce_core/io/files/juce_File.h"
#endif
#ifndef __JUCE_FILEINPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/files/juce_FileInputStream.h"
#endif
#ifndef __JUCE_FILEOUTPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/files/juce_FileOutputStream.h"
#endif
#ifndef __JUCE_FILESEARCHPATH_JUCEHEADER__
 #include "juce_core/io/files/juce_FileSearchPath.h"
#endif
#ifndef __JUCE_NAMEDPIPE_JUCEHEADER__
 #include "juce_core/io/files/juce_NamedPipe.h"
#endif
#ifndef __JUCE_BLOWFISH_JUCEHEADER__
 #include "juce_core/cryptography/juce_BlowFish.h"
#endif
#ifndef __JUCE_MD5_JUCEHEADER__
 #include "juce_core/cryptography/juce_MD5.h"
#endif
#ifndef __JUCE_PRIMES_JUCEHEADER__
 #include "juce_core/cryptography/juce_Primes.h"
#endif
#ifndef __JUCE_RSAKEY_JUCEHEADER__
 #include "juce_core/cryptography/juce_RSAKey.h"
#endif
#ifndef __JUCE_SOCKET_JUCEHEADER__
 #include "juce_core/io/network/juce_Socket.h"
#endif
#ifndef __JUCE_URL_JUCEHEADER__
 #include "juce_core/io/network/juce_URL.h"
#endif
#ifndef __JUCE_BUFFEREDINPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_BufferedInputStream.h"
#endif
#ifndef __JUCE_GZIPCOMPRESSOROUTPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_GZIPCompressorOutputStream.h"
#endif
#ifndef __JUCE_GZIPDECOMPRESSORINPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_GZIPDecompressorInputStream.h"
#endif
#ifndef __JUCE_MEMORYINPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_MemoryInputStream.h"
#endif
#ifndef __JUCE_MEMORYOUTPUTSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_MemoryOutputStream.h"
#endif
#ifndef __JUCE_SUBREGIONSTREAM_JUCEHEADER__
 #include "juce_core/io/streams/juce_SubregionStream.h"
#endif
#ifndef __JUCE_PERFORMANCECOUNTER_JUCEHEADER__
 #include "juce_core/misc/juce_PerformanceCounter.h"
#endif
#ifndef __JUCE_PLATFORMUTILITIES_JUCEHEADER__
 #include "juce_core/misc/juce_PlatformUtilities.h"
#endif
#ifndef __JUCE_UUID_JUCEHEADER__
 #include "juce_core/misc/juce_Uuid.h"
#endif
#ifndef __JUCE_ZIPFILE_JUCEHEADER__
 #include "juce_core/misc/juce_ZipFile.h"
#endif
#ifndef __JUCE_CHARACTERFUNCTIONS_JUCEHEADER__
 #include "juce_core/text/juce_CharacterFunctions.h"
#endif
#ifndef __JUCE_LOCALISEDSTRINGS_JUCEHEADER__
 #include "juce_core/text/juce_LocalisedStrings.h"
#endif
#ifndef __JUCE_STRING_JUCEHEADER__
 #include "juce_core/text/juce_String.h"
#endif
#ifndef __JUCE_STRINGARRAY_JUCEHEADER__
 #include "juce_core/text/juce_StringArray.h"
#endif
#ifndef __JUCE_STRINGPAIRARRAY_JUCEHEADER__
 #include "juce_core/text/juce_StringPairArray.h"
#endif
#ifndef __JUCE_XMLDOCUMENT_JUCEHEADER__
 #include "juce_core/text/juce_XmlDocument.h"
#endif
#ifndef __JUCE_XMLELEMENT_JUCEHEADER__
 #include "juce_core/text/juce_XmlElement.h"
#endif
#ifndef __JUCE_CRITICALSECTION_JUCEHEADER__
 #include "juce_core/threads/juce_CriticalSection.h"
#endif
#ifndef __JUCE_INTERPROCESSLOCK_JUCEHEADER__
 #include "juce_core/threads/juce_InterProcessLock.h"
#endif
#ifndef __JUCE_PROCESS_JUCEHEADER__
 #include "juce_core/threads/juce_Process.h"
#endif
#ifndef __JUCE_READWRITELOCK_JUCEHEADER__
 #include "juce_core/threads/juce_ReadWriteLock.h"
#endif
#ifndef __JUCE_SCOPEDLOCK_JUCEHEADER__
 #include "juce_core/threads/juce_ScopedLock.h"
#endif
#ifndef __JUCE_SCOPEDREADLOCK_JUCEHEADER__
 #include "juce_core/threads/juce_ScopedReadLock.h"
#endif
#ifndef __JUCE_SCOPEDWRITELOCK_JUCEHEADER__
 #include "juce_core/threads/juce_ScopedWriteLock.h"
#endif
#ifndef __JUCE_THREAD_JUCEHEADER__
 #include "juce_core/threads/juce_Thread.h"
#endif
#ifndef __JUCE_THREADPOOL_JUCEHEADER__
 #include "juce_core/threads/juce_ThreadPool.h"
#endif
#ifndef __JUCE_TIMESLICETHREAD_JUCEHEADER__
 #include "juce_core/threads/juce_TimeSliceThread.h"
#endif
#ifndef __JUCE_WAITABLEEVENT_JUCEHEADER__
 #include "juce_core/threads/juce_WaitableEvent.h"
#endif

#endif
