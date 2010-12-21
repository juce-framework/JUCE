/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

/*
    This file contains posix routines that are common to both the Linux and Mac builds.

    It gets included directly in the cpp files for these platforms.
*/


//==============================================================================
CriticalSection::CriticalSection() throw()
{
    pthread_mutexattr_t atts;
    pthread_mutexattr_init (&atts);
    pthread_mutexattr_settype (&atts, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutexattr_setprotocol (&atts, PTHREAD_PRIO_INHERIT);
    pthread_mutex_init (&internal, &atts);
}

CriticalSection::~CriticalSection() throw()
{
    pthread_mutex_destroy (&internal);
}

void CriticalSection::enter() const throw()
{
    pthread_mutex_lock (&internal);
}

bool CriticalSection::tryEnter() const throw()
{
    return pthread_mutex_trylock (&internal) == 0;
}

void CriticalSection::exit() const throw()
{
    pthread_mutex_unlock (&internal);
}


//==============================================================================
class WaitableEventImpl
{
public:
    WaitableEventImpl (const bool manualReset_)
        : triggered (false),
          manualReset (manualReset_)
    {
        pthread_cond_init (&condition, 0);

        pthread_mutexattr_t atts;
        pthread_mutexattr_init (&atts);
        pthread_mutexattr_setprotocol (&atts, PTHREAD_PRIO_INHERIT);
        pthread_mutex_init (&mutex, &atts);
    }

    ~WaitableEventImpl()
    {
        pthread_cond_destroy (&condition);
        pthread_mutex_destroy (&mutex);
    }

    bool wait (const int timeOutMillisecs) throw()
    {
        pthread_mutex_lock (&mutex);

        if (! triggered)
        {
            if (timeOutMillisecs < 0)
            {
                do
                {
                    pthread_cond_wait (&condition, &mutex);
                }
                while (! triggered);
            }
            else
            {
                struct timeval now;
                gettimeofday (&now, 0);

                struct timespec time;
                time.tv_sec  = now.tv_sec  + (timeOutMillisecs / 1000);
                time.tv_nsec = (now.tv_usec + ((timeOutMillisecs % 1000) * 1000)) * 1000;

                if (time.tv_nsec >= 1000000000)
                {
                    time.tv_nsec -= 1000000000;
                    time.tv_sec++;
                }

                do
                {
                    if (pthread_cond_timedwait (&condition, &mutex, &time) == ETIMEDOUT)
                    {
                        pthread_mutex_unlock (&mutex);
                        return false;
                    }
                }
                while (! triggered);
            }
        }

        if (! manualReset)
            triggered = false;

        pthread_mutex_unlock (&mutex);
        return true;
    }

    void signal() throw()
    {
        pthread_mutex_lock (&mutex);
        triggered = true;
        pthread_cond_broadcast (&condition);
        pthread_mutex_unlock (&mutex);
    }

    void reset() throw()
    {
        pthread_mutex_lock (&mutex);
        triggered = false;
        pthread_mutex_unlock (&mutex);
    }

private:
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool triggered;
    const bool manualReset;

    JUCE_DECLARE_NON_COPYABLE (WaitableEventImpl);
};

WaitableEvent::WaitableEvent (const bool manualReset) throw()
    : internal (new WaitableEventImpl (manualReset))
{
}

WaitableEvent::~WaitableEvent() throw()
{
    delete static_cast <WaitableEventImpl*> (internal);
}

bool WaitableEvent::wait (const int timeOutMillisecs) const throw()
{
    return static_cast <WaitableEventImpl*> (internal)->wait (timeOutMillisecs);
}

void WaitableEvent::signal() const throw()
{
    static_cast <WaitableEventImpl*> (internal)->signal();
}

void WaitableEvent::reset() const throw()
{
    static_cast <WaitableEventImpl*> (internal)->reset();
}

//==============================================================================
void JUCE_CALLTYPE Thread::sleep (int millisecs)
{
    struct timespec time;
    time.tv_sec = millisecs / 1000;
    time.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep (&time, 0);
}


//==============================================================================
const juce_wchar File::separator = '/';
const String File::separatorString ("/");

//==============================================================================
const File File::getCurrentWorkingDirectory()
{
    HeapBlock<char> heapBuffer;

    char localBuffer [1024];
    char* cwd = getcwd (localBuffer, sizeof (localBuffer) - 1);
    int bufferSize = 4096;

    while (cwd == 0 && errno == ERANGE)
    {
        heapBuffer.malloc (bufferSize);
        cwd = getcwd (heapBuffer, bufferSize - 1);
        bufferSize += 1024;
    }

    return File (String::fromUTF8 (cwd));
}

bool File::setAsCurrentWorkingDirectory() const
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
namespace
{
  #if JUCE_IOS && ! __DARWIN_ONLY_64_BIT_INO_T
   typedef struct stat64  juce_statStruct; // (need to use the 64-bit version to work around a simulator bug)
  #else
   typedef struct stat    juce_statStruct;
  #endif

    bool juce_stat (const String& fileName, juce_statStruct& info)
    {
        return fileName.isNotEmpty()
          #if JUCE_IOS && ! __DARWIN_ONLY_64_BIT_INO_T
                && (stat64 (fileName.toUTF8(), &info) == 0);
          #else
                && (stat (fileName.toUTF8(), &info) == 0);
          #endif
    }

    // if this file doesn't exist, find a parent of it that does..
    bool juce_doStatFS (File f, struct statfs& result)
    {
        for (int i = 5; --i >= 0;)
        {
            if (f.exists())
                break;

            f = f.getParentDirectory();
        }

        return statfs (f.getFullPathName().toUTF8(), &result) == 0;
    }

    void updateStatInfoForFile (const String& path, bool* const isDir, int64* const fileSize,
                                Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        if (isDir != 0 || fileSize != 0 || modTime != 0 || creationTime != 0)
        {
            juce_statStruct info;
            const bool statOk = juce_stat (path, info);

            if (isDir != 0)         *isDir = statOk && ((info.st_mode & S_IFDIR) != 0);
            if (fileSize != 0)      *fileSize = statOk ? info.st_size : 0;
            if (modTime != 0)       *modTime = Time (statOk ? (int64) info.st_mtime * 1000 : 0);
            if (creationTime != 0)  *creationTime = Time (statOk ? (int64) info.st_ctime * 1000 : 0);
        }

        if (isReadOnly != 0)
            *isReadOnly = access (path.toUTF8(), W_OK) != 0;
    }
}

bool File::isDirectory() const
{
    juce_statStruct info;

    return fullPath.isEmpty()
            || (juce_stat (fullPath, info) && ((info.st_mode & S_IFDIR) != 0));
}

bool File::exists() const
{
    juce_statStruct info;

    return fullPath.isNotEmpty()
      #if JUCE_IOS && ! __DARWIN_ONLY_64_BIT_INO_T
            && (lstat64 (fullPath.toUTF8(), &info) == 0);
      #else
            && (lstat (fullPath.toUTF8(), &info) == 0);
      #endif
}

bool File::existsAsFile() const
{
    return exists() && ! isDirectory();
}

int64 File::getSize() const
{
    juce_statStruct info;
    return juce_stat (fullPath, info) ? info.st_size : 0;
}

//==============================================================================
bool File::hasWriteAccess() const
{
    if (exists())
        return access (fullPath.toUTF8(), W_OK) == 0;

    if ((! isDirectory()) && fullPath.containsChar (separator))
        return getParentDirectory().hasWriteAccess();

    return false;
}

bool File::setFileReadOnlyInternal (const bool shouldBeReadOnly) const
{
    juce_statStruct info;
    if (! juce_stat (fullPath, info))
        return false;

    info.st_mode &= 0777;   // Just permissions

    if (shouldBeReadOnly)
        info.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    else
        // Give everybody write permission?
        info.st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

    return chmod (fullPath.toUTF8(), info.st_mode) == 0;
}

void File::getFileTimesInternal (int64& modificationTime, int64& accessTime, int64& creationTime) const
{
    modificationTime = 0;
    accessTime = 0;
    creationTime = 0;

    juce_statStruct info;
    if (juce_stat (fullPath, info))
    {
        modificationTime = (int64) info.st_mtime * 1000;
        accessTime = (int64) info.st_atime * 1000;
        creationTime = (int64) info.st_ctime * 1000;
    }
}

bool File::setFileTimesInternal (int64 modificationTime, int64 accessTime, int64 /*creationTime*/) const
{
    struct utimbuf times;
    times.actime = (time_t) (accessTime / 1000);
    times.modtime = (time_t) (modificationTime / 1000);

    return utime (fullPath.toUTF8(), &times) == 0;
}

bool File::deleteFile() const
{
    if (! exists())
        return true;
    else if (isDirectory())
        return rmdir (fullPath.toUTF8()) == 0;
    else
        return remove (fullPath.toUTF8()) == 0;
}

bool File::moveInternal (const File& dest) const
{
    if (rename (fullPath.toUTF8(), dest.getFullPathName().toUTF8()) == 0)
        return true;

    if (hasWriteAccess() && copyInternal (dest))
    {
        if (deleteFile())
            return true;

        dest.deleteFile();
    }

    return false;
}

void File::createDirectoryInternal (const String& fileName) const
{
    mkdir (fileName.toUTF8(), 0777);
}

//==============================================================================
int64 juce_fileSetPosition (void* handle, int64 pos)
{
    if (handle != 0 && lseek ((int) (pointer_sized_int) handle, pos, SEEK_SET) == pos)
        return pos;

    return -1;
}

void FileInputStream::openHandle()
{
    totalSize = file.getSize();

    const int f = open (file.getFullPathName().toUTF8(), O_RDONLY, 00644);

    if (f != -1)
        fileHandle = (void*) f;
}

void FileInputStream::closeHandle()
{
    if (fileHandle != 0)
    {
        close ((int) (pointer_sized_int) fileHandle);
        fileHandle = 0;
    }
}

size_t FileInputStream::readInternal (void* const buffer, const size_t numBytes)
{
    if (fileHandle != 0)
        return jmax ((ssize_t) 0, ::read ((int) (pointer_sized_int) fileHandle, buffer, numBytes));

    return 0;
}

//==============================================================================
void FileOutputStream::openHandle()
{
    if (file.exists())
    {
        const int f = open (file.getFullPathName().toUTF8(), O_RDWR, 00644);

        if (f != -1)
        {
            currentPosition = lseek (f, 0, SEEK_END);

            if (currentPosition >= 0)
                fileHandle = (void*) f;
            else
                close (f);
        }
    }
    else
    {
        const int f = open (file.getFullPathName().toUTF8(), O_RDWR + O_CREAT, 00644);

        if (f != -1)
            fileHandle = (void*) f;
    }
}

void FileOutputStream::closeHandle()
{
    if (fileHandle != 0)
    {
        close ((int) (pointer_sized_int) fileHandle);
        fileHandle = 0;
    }
}

int FileOutputStream::writeInternal (const void* const data, const int numBytes)
{
    if (fileHandle != 0)
        return (int) ::write ((int) (pointer_sized_int) fileHandle, data, numBytes);

    return 0;
}

void FileOutputStream::flushInternal()
{
    if (fileHandle != 0)
        fsync ((int) (pointer_sized_int) fileHandle);
}

//==============================================================================
const File juce_getExecutableFile()
{
    Dl_info exeInfo;
    dladdr ((const void*) juce_getExecutableFile, &exeInfo);
    return File::getCurrentWorkingDirectory().getChildFile (String::fromUTF8 (exeInfo.dli_fname));
}

//==============================================================================
int64 File::getBytesFreeOnVolume() const
{
    struct statfs buf;
    if (juce_doStatFS (*this, buf))
        return (int64) buf.f_bsize * (int64) buf.f_bavail; // Note: this returns space available to non-super user

    return 0;
}

int64 File::getVolumeTotalSize() const
{
    struct statfs buf;
    if (juce_doStatFS (*this, buf))
        return (int64) buf.f_bsize * (int64) buf.f_blocks;

    return 0;
}

const String File::getVolumeLabel() const
{
#if JUCE_MAC
    struct VolAttrBuf
    {
        u_int32_t       length;
        attrreference_t mountPointRef;
        char            mountPointSpace [MAXPATHLEN];
    } attrBuf;

    struct attrlist attrList;
    zerostruct (attrList);
    attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
    attrList.volattr = ATTR_VOL_INFO | ATTR_VOL_NAME;

    File f (*this);

    for (;;)
    {
        if (getattrlist (f.getFullPathName().toUTF8(), &attrList, &attrBuf, sizeof (attrBuf), 0) == 0)
            return String::fromUTF8 (((const char*) &attrBuf.mountPointRef) + attrBuf.mountPointRef.attr_dataoffset,
                                     (int) attrBuf.mountPointRef.attr_length);

        const File parent (f.getParentDirectory());

        if (f == parent)
            break;

        f = parent;
    }
#endif

    return String::empty;
}

int File::getVolumeSerialNumber() const
{
    return 0; // xxx
}

//==============================================================================
void juce_runSystemCommand (const String& command)
{
    int result = system (command.toUTF8());
    (void) result;
}

const String juce_getOutputFromCommand (const String& command)
{
    // slight bodge here, as we just pipe the output into a temp file and read it...
    const File tempFile (File::getSpecialLocation (File::tempDirectory)
                           .getNonexistentChildFile (String::toHexString (Random::getSystemRandom().nextInt()), ".tmp", false));

    juce_runSystemCommand (command + " > " + tempFile.getFullPathName());

    String result (tempFile.loadFileAsString());
    tempFile.deleteFile();
    return result;
}


//==============================================================================
class InterProcessLock::Pimpl
{
public:
    Pimpl (const String& name, const int timeOutMillisecs)
        : handle (0), refCount (1)
    {
      #if JUCE_MAC
        // (don't use getSpecialLocation() to avoid the temp folder being different for each app)
        const File temp (File ("~/Library/Caches/Juce").getChildFile (name));
      #else
        const File temp (File::getSpecialLocation (File::tempDirectory).getChildFile (name));
      #endif
        temp.create();
        handle = open (temp.getFullPathName().toUTF8(), O_RDWR);

        if (handle != 0)
        {
            struct flock fl;
            zerostruct (fl);
            fl.l_whence = SEEK_SET;
            fl.l_type = F_WRLCK;

            const int64 endTime = Time::currentTimeMillis() + timeOutMillisecs;

            for (;;)
            {
                const int result = fcntl (handle, F_SETLK, &fl);

                if (result >= 0)
                    return;

                if (errno != EINTR)
                {
                    if (timeOutMillisecs == 0
                         || (timeOutMillisecs > 0 && Time::currentTimeMillis() >= endTime))
                        break;

                    Thread::sleep (10);
                }
            }
        }

        closeFile();
    }

    ~Pimpl()
    {
        closeFile();
    }

    void closeFile()
    {
        if (handle != 0)
        {
            struct flock fl;
            zerostruct (fl);
            fl.l_whence = SEEK_SET;
            fl.l_type = F_UNLCK;

            while (! (fcntl (handle, F_SETLKW, &fl) >= 0 || errno != EINTR))
            {}

            close (handle);
            handle = 0;
        }
    }

    int handle, refCount;
};

InterProcessLock::InterProcessLock (const String& name_)
    : name (name_)
{
}

InterProcessLock::~InterProcessLock()
{
}

bool InterProcessLock::enter (const int timeOutMillisecs)
{
    const ScopedLock sl (lock);

    if (pimpl == 0)
    {
        pimpl = new Pimpl (name, timeOutMillisecs);

        if (pimpl->handle == 0)
            pimpl = 0;
    }
    else
    {
        pimpl->refCount++;
    }

    return pimpl != 0;
}

void InterProcessLock::exit()
{
    const ScopedLock sl (lock);

    // Trying to release the lock too many times!
    jassert (pimpl != 0);

    if (pimpl != 0 && --(pimpl->refCount) == 0)
        pimpl = 0;
}

//==============================================================================
void JUCE_API juce_threadEntryPoint (void*);

void* threadEntryProc (void* userData)
{
    JUCE_AUTORELEASEPOOL
    juce_threadEntryPoint (userData);
    return 0;
}

void Thread::launchThread()
{
    threadHandle_ = 0;
    pthread_t handle = 0;

    if (pthread_create (&handle, 0, threadEntryProc, this) == 0)
    {
        pthread_detach (handle);
        threadHandle_ = (void*) handle;
        threadId_ = (ThreadID) threadHandle_;
    }
}

void Thread::closeThreadHandle()
{
    threadId_ = 0;
    threadHandle_ = 0;
}

void Thread::killThread()
{
    if (threadHandle_ != 0)
        pthread_cancel ((pthread_t) threadHandle_);
}

void Thread::setCurrentThreadName (const String& /*name*/)
{
}

bool Thread::setThreadPriority (void* handle, int priority)
{
    struct sched_param param;
    int policy;
    priority = jlimit (0, 10, priority);

    if (handle == 0)
        handle = (void*) pthread_self();

    if (pthread_getschedparam ((pthread_t) handle, &policy, &param) != 0)
        return false;

    policy = priority == 0 ? SCHED_OTHER : SCHED_RR;

    const int minPriority = sched_get_priority_min (policy);
    const int maxPriority = sched_get_priority_max (policy);

    param.sched_priority = ((maxPriority - minPriority) * priority) / 10 + minPriority;
    return pthread_setschedparam ((pthread_t) handle, policy, &param) == 0;
}

Thread::ThreadID Thread::getCurrentThreadId()
{
    return (ThreadID) pthread_self();
}

void Thread::yield()
{
    sched_yield();
}

//==============================================================================
/* Remove this macro if you're having problems compiling the cpu affinity
   calls (the API for these has changed about quite a bit in various Linux
   versions, and a lot of distros seem to ship with obsolete versions)
*/
#if defined (CPU_ISSET) && ! defined (SUPPORT_AFFINITIES)
  #define SUPPORT_AFFINITIES 1
#endif

void Thread::setCurrentThreadAffinityMask (const uint32 affinityMask)
{
#if SUPPORT_AFFINITIES
    cpu_set_t affinity;
    CPU_ZERO (&affinity);

    for (int i = 0; i < 32; ++i)
        if ((affinityMask & (1 << i)) != 0)
            CPU_SET (i, &affinity);

    /*
       N.B. If this line causes a compile error, then you've probably not got the latest
       version of glibc installed.

       If you don't want to update your copy of glibc and don't care about cpu affinities,
       then you can just disable all this stuff by setting the SUPPORT_AFFINITIES macro to 0.
    */
    sched_setaffinity (getpid(), sizeof (cpu_set_t), &affinity);
    sched_yield();

#else
    /* affinities aren't supported because either the appropriate header files weren't found,
       or the SUPPORT_AFFINITIES macro was turned off
    */
    jassertfalse;
#endif
}
