/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

CriticalSection::CriticalSection() noexcept
{
    pthread_mutexattr_t atts;
    pthread_mutexattr_init (&atts);
    pthread_mutexattr_settype (&atts, PTHREAD_MUTEX_RECURSIVE);
   #if ! JUCE_ANDROID
    pthread_mutexattr_setprotocol (&atts, PTHREAD_PRIO_INHERIT);
   #endif
    pthread_mutex_init (&lock, &atts);
    pthread_mutexattr_destroy (&atts);
}

CriticalSection::~CriticalSection() noexcept        { pthread_mutex_destroy (&lock); }
void CriticalSection::enter() const noexcept        { pthread_mutex_lock (&lock); }
bool CriticalSection::tryEnter() const noexcept     { return pthread_mutex_trylock (&lock) == 0; }
void CriticalSection::exit() const noexcept         { pthread_mutex_unlock (&lock); }

//==============================================================================
void JUCE_CALLTYPE Thread::sleep (int millisecs)
{
    struct timespec time;
    time.tv_sec = millisecs / 1000;
    time.tv_nsec = (millisecs % 1000) * 1000000;
    nanosleep (&time, nullptr);
}

void JUCE_CALLTYPE Process::terminate()
{
   #if JUCE_ANDROID
    _exit (EXIT_FAILURE);
   #else
    std::_Exit (EXIT_FAILURE);
   #endif
}


#if JUCE_MAC || JUCE_LINUX || JUCE_BSD
bool Process::setMaxNumberOfFileHandles (int newMaxNumber) noexcept
{
    rlimit lim;

    if (getrlimit (RLIMIT_NOFILE, &lim) == 0)
    {
        if (newMaxNumber <= 0 && lim.rlim_cur == RLIM_INFINITY && lim.rlim_max == RLIM_INFINITY)
            return true;

        if (newMaxNumber > 0 && lim.rlim_cur >= (rlim_t) newMaxNumber)
            return true;
    }

    lim.rlim_cur = lim.rlim_max = newMaxNumber <= 0 ? RLIM_INFINITY : (rlim_t) newMaxNumber;
    return setrlimit (RLIMIT_NOFILE, &lim) == 0;
}

struct MaxNumFileHandlesInitialiser
{
    MaxNumFileHandlesInitialiser() noexcept
    {
       #ifndef JUCE_PREFERRED_MAX_FILE_HANDLES
        enum { JUCE_PREFERRED_MAX_FILE_HANDLES = 8192 };
       #endif

        // Try to give our app a decent number of file handles by default
        if (! Process::setMaxNumberOfFileHandles (0))
        {
            for (int num = JUCE_PREFERRED_MAX_FILE_HANDLES; num > 256; num -= 1024)
                if (Process::setMaxNumberOfFileHandles (num))
                    break;
        }
    }
};

static MaxNumFileHandlesInitialiser maxNumFileHandlesInitialiser;
#endif

//==============================================================================
#if JUCE_ALLOW_STATIC_NULL_VARIABLES

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")

const juce_wchar File::separator = '/';
const StringRef File::separatorString ("/");

JUCE_END_IGNORE_WARNINGS_GCC_LIKE

#endif

juce_wchar File::getSeparatorChar()    { return '/'; }
StringRef File::getSeparatorString()   { return "/"; }


//==============================================================================
File File::getCurrentWorkingDirectory()
{
    HeapBlock<char> heapBuffer;

    char localBuffer[1024];
    auto cwd = getcwd (localBuffer, sizeof (localBuffer) - 1);
    size_t bufferSize = 4096;

    while (cwd == nullptr && errno == ERANGE)
    {
        heapBuffer.malloc (bufferSize);
        cwd = getcwd (heapBuffer, bufferSize - 1);
        bufferSize += 1024;
    }

    return File (CharPointer_UTF8 (cwd));
}

bool File::setAsCurrentWorkingDirectory() const
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
// The unix siginterrupt function is deprecated - this does the same job.
int juce_siginterrupt ([[maybe_unused]] int sig, [[maybe_unused]] int flag)
{
   #if JUCE_WASM
    return 0;
   #else
    #if JUCE_ANDROID
     using juce_sigactionflags_type = unsigned long;
    #else
     using juce_sigactionflags_type = int;
    #endif

    struct ::sigaction act;
    (void) ::sigaction (sig, nullptr, &act);

    if (flag != 0)
        act.sa_flags &= static_cast<juce_sigactionflags_type> (~SA_RESTART);
    else
        act.sa_flags |= static_cast<juce_sigactionflags_type> (SA_RESTART);

    return ::sigaction (sig, &act, nullptr);
   #endif
}

//==============================================================================
namespace
{
   #if JUCE_LINUX || (JUCE_IOS && (! TARGET_OS_MACCATALYST) && (! __DARWIN_ONLY_64_BIT_INO_T)) // (this iOS stuff is to avoid a simulator bug)
    using juce_statStruct = struct stat64;
    #define JUCE_STAT  stat64
   #else
    using juce_statStruct = struct stat;
    #define JUCE_STAT  stat
   #endif

    bool juce_stat (const String& fileName, juce_statStruct& info)
    {
        return fileName.isNotEmpty()
                 && JUCE_STAT (fileName.toUTF8(), &info) == 0;
    }

   #if ! JUCE_WASM
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

   #if JUCE_MAC || JUCE_IOS
    static int64 getCreationTime (const juce_statStruct& s) noexcept     { return (int64) s.st_birthtime; }
   #else
    static int64 getCreationTime (const juce_statStruct& s) noexcept     { return (int64) s.st_ctime; }
   #endif

    void updateStatInfoForFile (const String& path, bool* isDir, int64* fileSize,
                                Time* modTime, Time* creationTime, bool* isReadOnly)
    {
        if (isDir != nullptr || fileSize != nullptr || modTime != nullptr || creationTime != nullptr)
        {
            juce_statStruct info;
            const bool statOk = juce_stat (path, info);

            if (isDir != nullptr)         *isDir        = statOk && ((info.st_mode & S_IFDIR) != 0);
            if (fileSize != nullptr)      *fileSize     = statOk ? (int64) info.st_size : 0;
            if (modTime != nullptr)       *modTime      = Time (statOk ? (int64) info.st_mtime  * 1000 : 0);
            if (creationTime != nullptr)  *creationTime = Time (statOk ? getCreationTime (info) * 1000 : 0);
        }

        if (isReadOnly != nullptr)
            *isReadOnly = access (path.toUTF8(), W_OK) != 0;
    }
   #endif

    Result getResultForErrno()
    {
        return Result::fail (String (strerror (errno)));
    }

    Result getResultForReturnValue (int value)
    {
        return value == -1 ? getResultForErrno() : Result::ok();
    }

    int getFD (void* handle) noexcept        { return (int) (pointer_sized_int) handle; }
    void* fdToVoidPointer (int fd) noexcept  { return (void*) (pointer_sized_int) fd; }
}

bool File::isDirectory() const
{
    juce_statStruct info;

    return fullPath.isNotEmpty()
             && (juce_stat (fullPath, info) && ((info.st_mode & S_IFDIR) != 0));
}

bool File::exists() const
{
    return fullPath.isNotEmpty()
             && access (fullPath.toUTF8(), F_OK) == 0;
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

uint64 File::getFileIdentifier() const
{
    juce_statStruct info;
    return juce_stat (fullPath, info) ? (uint64) info.st_ino : 0;
}

static bool hasEffectiveRootFilePermissions()
{
   #if JUCE_LINUX || JUCE_BSD
    return geteuid() == 0;
   #else
    return false;
   #endif
}

//==============================================================================
bool File::hasWriteAccess() const
{
    if (exists())
        return (hasEffectiveRootFilePermissions()
             || access (fullPath.toUTF8(), W_OK) == 0);

    if ((! isDirectory()) && fullPath.containsChar (getSeparatorChar()))
        return getParentDirectory().hasWriteAccess();

    return false;
}

bool File::hasReadAccess() const
{
    return fullPath.isNotEmpty()
           && access (fullPath.toUTF8(), R_OK) == 0;
}

static bool setFileModeFlags (const String& fullPath, mode_t flags, bool shouldSet) noexcept
{
    juce_statStruct info;

    if (! juce_stat (fullPath, info))
        return false;

    info.st_mode &= 0777;

    if (shouldSet)
        info.st_mode |= flags;
    else
        info.st_mode &= ~flags;

    return chmod (fullPath.toUTF8(), (mode_t) info.st_mode) == 0;
}

bool File::setFileReadOnlyInternal (bool shouldBeReadOnly) const
{
    // Hmm.. should we give global write permission or just the current user?
    return setFileModeFlags (fullPath, S_IWUSR | S_IWGRP | S_IWOTH, ! shouldBeReadOnly);
}

bool File::setFileExecutableInternal (bool shouldBeExecutable) const
{
    return setFileModeFlags (fullPath, S_IXUSR | S_IXGRP | S_IXOTH, shouldBeExecutable);
}

void File::getFileTimesInternal (int64& modificationTime, int64& accessTime, int64& creationTime) const
{
    modificationTime = 0;
    accessTime = 0;
    creationTime = 0;

    juce_statStruct info;

    if (juce_stat (fullPath, info))
    {
      #if JUCE_MAC || (JUCE_IOS && __DARWIN_ONLY_64_BIT_INO_T)
        modificationTime  = (int64) info.st_mtimespec.tv_sec * 1000 + info.st_mtimespec.tv_nsec / 1000000;
        accessTime        = (int64) info.st_atimespec.tv_sec * 1000 + info.st_atimespec.tv_nsec / 1000000;
        creationTime      = (int64) info.st_birthtimespec.tv_sec * 1000 + info.st_birthtimespec.tv_nsec / 1000000;
      #else
        modificationTime  = (int64) info.st_mtime * 1000;
        accessTime        = (int64) info.st_atime * 1000;
       #if JUCE_IOS
        creationTime      = (int64) info.st_birthtime * 1000;
       #else
        creationTime      = (int64) info.st_ctime * 1000;
       #endif
      #endif
    }
}

bool File::setFileTimesInternal (int64 modificationTime, int64 accessTime, int64 /*creationTime*/) const
{
   #if ! JUCE_WASM
    juce_statStruct info;

    if ((modificationTime != 0 || accessTime != 0) && juce_stat (fullPath, info))
    {
       #if JUCE_MAC || (JUCE_IOS && __DARWIN_ONLY_64_BIT_INO_T)
        struct timeval times[2];

        bool setModificationTime = (modificationTime != 0);
        bool setAccessTime       = (accessTime != 0);

        times[0].tv_sec  = setAccessTime ? static_cast<__darwin_time_t> (accessTime / 1000)
                                         : info.st_atimespec.tv_sec;

        times[0].tv_usec = setAccessTime ? static_cast<__darwin_suseconds_t> ((accessTime % 1000) * 1000)
                                         : static_cast<__darwin_suseconds_t> (info.st_atimespec.tv_nsec / 1000);

        times[1].tv_sec  = setModificationTime ? static_cast<__darwin_time_t> (modificationTime / 1000)
                                               : info.st_mtimespec.tv_sec;

        times[1].tv_usec = setModificationTime ? static_cast<__darwin_suseconds_t> ((modificationTime % 1000) * 1000)
                                               : static_cast<__darwin_suseconds_t> (info.st_mtimespec.tv_nsec / 1000);

        return utimes (fullPath.toUTF8(), times) == 0;
       #else
        struct utimbuf times;
        times.actime  = accessTime != 0       ? static_cast<time_t> (accessTime / 1000)       : static_cast<time_t> (info.st_atime);
        times.modtime = modificationTime != 0 ? static_cast<time_t> (modificationTime / 1000) : static_cast<time_t> (info.st_mtime);

        return utime (fullPath.toUTF8(), &times) == 0;
       #endif
    }
   #endif

    return false;
}

bool File::deleteFile() const
{
    if (! isSymbolicLink())
    {
        if (! exists())
            return true;

        if (isDirectory())
            return rmdir (fullPath.toUTF8()) == 0;
    }

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

bool File::replaceInternal (const File& dest) const
{
    return moveInternal (dest);
}

Result File::createDirectoryInternal (const String& fileName) const
{
    return getResultForReturnValue (mkdir (fileName.toUTF8(), 0777));
}

//==============================================================================
int64 juce_fileSetPosition (void* handle, int64 pos)
{
    if (handle != nullptr && lseek (getFD (handle), (off_t) pos, SEEK_SET) == pos)
        return pos;

    return -1;
}

void FileInputStream::openHandle()
{
    auto f = open (file.getFullPathName().toUTF8(), O_RDONLY);

    if (f != -1)
        fileHandle = fdToVoidPointer (f);
    else
        status = getResultForErrno();
}

FileInputStream::~FileInputStream()
{
    if (fileHandle != nullptr)
        close (getFD (fileHandle));
}

size_t FileInputStream::readInternal (void* buffer, size_t numBytes)
{
    ssize_t result = 0;

    if (fileHandle != nullptr)
    {
        result = ::read (getFD (fileHandle), buffer, numBytes);

        if (result < 0)
        {
            status = getResultForErrno();
            result = 0;
        }
    }

    return (size_t) result;
}

//==============================================================================
void FileOutputStream::openHandle()
{
    if (file.exists())
    {
        auto f = open (file.getFullPathName().toUTF8(), O_RDWR);

        if (f != -1)
        {
            currentPosition = lseek (f, 0, SEEK_END);

            if (currentPosition >= 0)
            {
                fileHandle = fdToVoidPointer (f);
            }
            else
            {
                status = getResultForErrno();
                close (f);
            }
        }
        else
        {
            status = getResultForErrno();
        }
    }
    else
    {
        auto f = open (file.getFullPathName().toUTF8(), O_RDWR | O_CREAT, 00644);

        if (f != -1)
            fileHandle = fdToVoidPointer (f);
        else
            status = getResultForErrno();
    }
}

void FileOutputStream::closeHandle()
{
    if (fileHandle != nullptr)
    {
        close (getFD (fileHandle));
        fileHandle = nullptr;
    }
}

ssize_t FileOutputStream::writeInternal (const void* data, size_t numBytes)
{
    if (fileHandle == nullptr)
        return 0;

    auto result = ::write (getFD (fileHandle), data, numBytes);

    if (result == -1)
        status = getResultForErrno();

    return (ssize_t) result;
}

#ifndef JUCE_ANDROID
void FileOutputStream::flushInternal()
{
    if (fileHandle != nullptr && fsync (getFD (fileHandle)) == -1)
        status = getResultForErrno();
}
#endif

Result FileOutputStream::truncate()
{
    if (fileHandle == nullptr)
        return status;

    flush();
    return getResultForReturnValue (ftruncate (getFD (fileHandle), (off_t) currentPosition));
}

//==============================================================================
String SystemStats::getEnvironmentVariable (const String& name, const String& defaultValue)
{
    if (auto s = ::getenv (name.toUTF8()))
        return String::fromUTF8 (s);

    return defaultValue;
}

//==============================================================================
#if ! JUCE_WASM
void MemoryMappedFile::openInternal (const File& file, AccessMode mode, bool exclusive)
{
    jassert (mode == readOnly || mode == readWrite);

    if (range.getStart() > 0)
    {
        auto pageSize = sysconf (_SC_PAGE_SIZE);
        range.setStart (range.getStart() - (range.getStart() % pageSize));
    }

    auto filename = file.getFullPathName().toUTF8();

    if (mode == readWrite)
        fileHandle = open (filename, O_CREAT | O_RDWR, 00644);
    else
        fileHandle = open (filename, O_RDONLY);

    if (fileHandle != -1)
    {
        auto m = mmap (nullptr, (size_t) range.getLength(),
                       mode == readWrite ? (PROT_READ | PROT_WRITE) : PROT_READ,
                       exclusive ? MAP_PRIVATE : MAP_SHARED, fileHandle,
                       (off_t) range.getStart());

        if (m != MAP_FAILED)
        {
            address = m;
            madvise (m, (size_t) range.getLength(), MADV_SEQUENTIAL);
        }
        else
        {
            range = Range<int64>();
        }

        close (fileHandle);
        fileHandle = 0;
    }
}

MemoryMappedFile::~MemoryMappedFile()
{
    if (address != nullptr)
        munmap (address, (size_t) range.getLength());

    if (fileHandle != 0)
        close (fileHandle);
}

//==============================================================================
File juce_getExecutableFile();
File juce_getExecutableFile()
{
    struct DLAddrReader
    {
        static String getFilename()
        {
            Dl_info exeInfo;

            auto localSymbol = (void*) juce_getExecutableFile;
            dladdr (localSymbol, &exeInfo);
            return CharPointer_UTF8 (exeInfo.dli_fname);
        }
    };

    static String filename = DLAddrReader::getFilename();
    return File::getCurrentWorkingDirectory().getChildFile (filename);
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

String File::getVolumeLabel() const
{
   #if JUCE_MAC
    struct VolAttrBuf
    {
        u_int32_t       length;
        attrreference_t mountPointRef;
        char            mountPointSpace[MAXPATHLEN];
    } attrBuf;

    struct attrlist attrList;
    zerostruct (attrList); // (can't use "= {}" on this object because it's a C struct)
    attrList.bitmapcount = ATTR_BIT_MAP_COUNT;
    attrList.volattr = ATTR_VOL_INFO | ATTR_VOL_NAME;

    File f (*this);

    for (;;)
    {
        if (getattrlist (f.getFullPathName().toUTF8(), &attrList, &attrBuf, sizeof (attrBuf), 0) == 0)
            return String::fromUTF8 (((const char*) &attrBuf.mountPointRef) + attrBuf.mountPointRef.attr_dataoffset,
                                     (int) attrBuf.mountPointRef.attr_length);

        auto parent = f.getParentDirectory();

        if (f == parent)
            break;

        f = parent;
    }
   #endif

    return {};
}

int File::getVolumeSerialNumber() const
{
    return 0;
}

#endif

//==============================================================================
#if ! JUCE_IOS
void juce_runSystemCommand (const String&);
void juce_runSystemCommand (const String& command)
{
    [[maybe_unused]] int result = system (command.toUTF8());
}

String juce_getOutputFromCommand (const String&);
String juce_getOutputFromCommand (const String& command)
{
    // slight bodge here, as we just pipe the output into a temp file and read it...
    auto tempFile = File::getSpecialLocation (File::tempDirectory)
                      .getNonexistentChildFile (String::toHexString (Random::getSystemRandom().nextInt()), ".tmp", false);

    juce_runSystemCommand (command + " > " + tempFile.getFullPathName());

    auto result = tempFile.loadFileAsString();
    tempFile.deleteFile();
    return result;
}
#endif

//==============================================================================
#if JUCE_IOS
class InterProcessLock::Pimpl
{
public:
    Pimpl (const String&, int)  {}

    int handle = 1, refCount = 1;  // On iOS just fake success..
};

#else

class InterProcessLock::Pimpl
{
public:
    Pimpl (const String& lockName, int timeOutMillisecs)
    {
       #if JUCE_MAC
        if (! createLockFile (File ("~/Library/Caches/com.juce.locks").getChildFile (lockName), timeOutMillisecs))
            // Fallback if the user's home folder is on a network drive with no ability to lock..
            createLockFile (File ("/tmp/com.juce.locks").getChildFile (lockName), timeOutMillisecs);

       #else
        File tempFolder ("/var/tmp");

        if (! tempFolder.isDirectory())
            tempFolder = "/tmp";

        createLockFile (tempFolder.getChildFile (lockName), timeOutMillisecs);
       #endif
    }

    ~Pimpl()
    {
        closeFile();
    }

    bool createLockFile (const File& file, int timeOutMillisecs)
    {
        file.create();
        handle = open (file.getFullPathName().toUTF8(), O_RDWR);

        if (handle != 0)
        {
            struct flock fl;
            zerostruct (fl);

            fl.l_whence = SEEK_SET;
            fl.l_type = F_WRLCK;

            auto endTime = Time::currentTimeMillis() + timeOutMillisecs;

            for (;;)
            {
                auto result = fcntl (handle, F_SETLK, &fl);

                if (result >= 0)
                    return true;

                auto error = errno;

                if (error != EINTR)
                {
                    if (error == EBADF || error == ENOTSUP)
                        return false;

                    if (timeOutMillisecs == 0
                         || (timeOutMillisecs > 0 && Time::currentTimeMillis() >= endTime))
                        break;

                    Thread::sleep (10);
                }
            }
        }

        closeFile();
        return true; // only false if there's a file system error. Failure to lock still returns true.
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

    int handle = 0, refCount = 1;
};
#endif

InterProcessLock::InterProcessLock (const String& nm)  : name (nm)
{
}

InterProcessLock::~InterProcessLock()
{
}

bool InterProcessLock::enter (int timeOutMillisecs)
{
    const ScopedLock sl (lock);

    if (pimpl == nullptr)
    {
        pimpl.reset (new Pimpl (name, timeOutMillisecs));

        if (pimpl->handle == 0)
            pimpl.reset();
    }
    else
    {
        pimpl->refCount++;
    }

    return pimpl != nullptr;
}

void InterProcessLock::exit()
{
    const ScopedLock sl (lock);

    // Trying to release the lock too many times!
    jassert (pimpl != nullptr);

    if (pimpl != nullptr && --(pimpl->refCount) == 0)
        pimpl.reset();
}

class PosixThreadAttribute
{
public:
    explicit PosixThreadAttribute (size_t stackSize)
    {
        if (valid && stackSize != 0)
            pthread_attr_setstacksize (&attr, stackSize);
    }

    ~PosixThreadAttribute()
    {
        if (valid)
            pthread_attr_destroy (&attr);
    }

    auto* get() { return valid ? &attr : nullptr; }

private:
    pthread_attr_t attr;
    bool valid { pthread_attr_init (&attr) == 0 };
};

class PosixSchedulerPriority
{
public:
    static PosixSchedulerPriority findCurrentSchedulerAndPriority()
    {
        int scheduler{};
        sched_param param{};
        pthread_getschedparam (pthread_self(), &scheduler, &param);
        return { scheduler, param.sched_priority };
    }

    static PosixSchedulerPriority getNativeSchedulerAndPriority (const Optional<Thread::RealtimeOptions>& rt,
                                                                 [[maybe_unused]] Thread::Priority prio)
    {
        const auto isRealtime = rt.hasValue();

        const auto priority = [&]
        {
            if (isRealtime)
            {
                const auto min = jmax (0, sched_get_priority_min (SCHED_RR));
                const auto max = jmax (1, sched_get_priority_max (SCHED_RR));

                return jmap (rt->getPriority(), 0, 10, min, max);
            }

            // We only use this helper if we're on an old macos/ios platform that might
            // still respect legacy pthread priorities for SCHED_OTHER.
            #if JUCE_MAC || JUCE_IOS
             const auto min = jmax (0, sched_get_priority_min (SCHED_OTHER));
             const auto max = jmax (0, sched_get_priority_max (SCHED_OTHER));

             const auto p = [prio]
             {
                 switch (prio)
                 {
                     case Thread::Priority::highest:    return 4;
                     case Thread::Priority::high:       return 3;
                     case Thread::Priority::normal:     return 2;
                     case Thread::Priority::low:        return 1;
                     case Thread::Priority::background: return 0;
                 }

                 return 3;
             }();

             if (min != 0 && max != 0)
                 return jmap (p, 0, 4, min, max);
            #endif

            return 0;
        }();

        #if JUCE_MAC || JUCE_IOS || JUCE_BSD
         const auto scheduler = SCHED_OTHER;
        #elif JUCE_LINUX
         const auto backgroundSched = prio == Thread::Priority::background ? SCHED_IDLE
                                                                           : SCHED_OTHER;
         const auto scheduler = isRealtime ? SCHED_RR : backgroundSched;
        #else
         const auto scheduler = 0;
        #endif

         return { scheduler, priority };
    }

    void apply ([[maybe_unused]] PosixThreadAttribute& attr) const
    {
        #if JUCE_LINUX || JUCE_BSD
         const struct sched_param param { getPriority() };

         pthread_attr_setinheritsched (attr.get(), PTHREAD_EXPLICIT_SCHED);
         pthread_attr_setschedpolicy (attr.get(), getScheduler());
         pthread_attr_setschedparam (attr.get(), &param);
        #endif
    }

    constexpr int getScheduler() const { return scheduler; }
    constexpr int  getPriority() const { return priority; }

private:
    constexpr PosixSchedulerPriority (int schedulerIn, int priorityIn)
        : scheduler (schedulerIn), priority (priorityIn) {}

    int scheduler;
    int priority;
};

static void* makeThreadHandle (PosixThreadAttribute& attr, void* userData, void* (*threadEntryProc) (void*))
{
    pthread_t handle = {};

    const auto status = pthread_create (&handle, attr.get(), threadEntryProc, userData);

    if (status != 0)
        return nullptr;

    pthread_detach (handle);
    return (void*) handle;
}

void Thread::closeThreadHandle()
{
    threadId = {};
    threadHandle = nullptr;
}

void JUCE_CALLTYPE Thread::setCurrentThreadName (const String& name)
{
   #if JUCE_IOS || JUCE_MAC
    JUCE_AUTORELEASEPOOL
    {
        [[NSThread currentThread] setName: juceStringToNS (name)];
    }
   #elif JUCE_LINUX || JUCE_BSD || JUCE_ANDROID
    #if (JUCE_BSD \
          || (JUCE_LINUX && (__GLIBC__ * 1000 + __GLIBC_MINOR__) >= 2012) \
          || (JUCE_ANDROID && __ANDROID_API__ >= 9))
     pthread_setname_np (pthread_self(), name.toRawUTF8());
    #else
     prctl (PR_SET_NAME, name.toRawUTF8(), 0, 0, 0);
    #endif
   #endif
}

Thread::ThreadID JUCE_CALLTYPE Thread::getCurrentThreadId()
{
    return (ThreadID) pthread_self();
}

void JUCE_CALLTYPE Thread::yield()
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

void JUCE_CALLTYPE Thread::setCurrentThreadAffinityMask ([[maybe_unused]] uint32 affinityMask)
{
   #if SUPPORT_AFFINITIES
    cpu_set_t affinity;
    CPU_ZERO (&affinity);

    for (int i = 0; i < 32; ++i)
    {
        if ((affinityMask & (uint32) (1 << i)) != 0)
        {
            // GCC 12 on FreeBSD complains about CPU_SET irrespective of
            // the type of the first argument
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsign-conversion")
            CPU_SET ((size_t) i, &affinity);
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
        }
    }

   #if (! JUCE_ANDROID) && ((! (JUCE_LINUX || JUCE_BSD)) || ((__GLIBC__ * 1000 + __GLIBC_MINOR__) >= 2004))
    pthread_setaffinity_np (pthread_self(), sizeof (cpu_set_t), &affinity);
   #elif JUCE_ANDROID
    sched_setaffinity (gettid(), sizeof (cpu_set_t), &affinity);
   #else
    // NB: this call isn't really correct because it sets the affinity of the process,
    // (getpid) not the thread (not gettid). But it's included here as a fallback for
    // people who are using ridiculously old versions of glibc
    sched_setaffinity (getpid(), sizeof (cpu_set_t), &affinity);
   #endif

    sched_yield();

   #else
    // affinities aren't supported because either the appropriate header files weren't found,
    // or the SUPPORT_AFFINITIES macro was turned off
    jassertfalse;
   #endif
}

//==============================================================================
#if ! JUCE_WASM
bool DynamicLibrary::open (const String& name)
{
    close();
    handle = dlopen (name.isEmpty() ? nullptr : name.toUTF8().getAddress(), RTLD_LOCAL | RTLD_NOW);
    return handle != nullptr;
}

void DynamicLibrary::close()
{
    if (handle != nullptr)
    {
        dlclose (handle);
        handle = nullptr;
    }
}

void* DynamicLibrary::getFunction (const String& functionName) noexcept
{
    return handle != nullptr ? dlsym (handle, functionName.toUTF8()) : nullptr;
}

//==============================================================================
#if JUCE_LINUX || JUCE_ANDROID
static String readPosixConfigFileValue (const char* file, const char* key)
{
    StringArray lines;
    File (file).readLines (lines);

    for (int i = lines.size(); --i >= 0;) // (NB - it's important that this runs in reverse order)
        if (lines[i].upToFirstOccurrenceOf (":", false, false).trim().equalsIgnoreCase (key))
            return lines[i].fromFirstOccurrenceOf (":", false, false).trim();

    return {};
}
#endif


//==============================================================================
class ChildProcess::ActiveProcess
{
public:
    ActiveProcess (const StringArray& arguments, int streamFlags)
    {
        auto exe = arguments[0].unquoted();

        // Looks like you're trying to launch a non-existent exe or a folder (perhaps on OSX
        // you're trying to launch the .app folder rather than the actual binary inside it?)
        jassert (File::getCurrentWorkingDirectory().getChildFile (exe).existsAsFile()
                  || ! exe.containsChar (File::getSeparatorChar()));

        int pipeHandles[2] = {};

        if (pipe (pipeHandles) == 0)
        {
            auto result = fork();

            if (result < 0)
            {
                close (pipeHandles[0]);
                close (pipeHandles[1]);
            }
            else if (result == 0)
            {
                // we're the child process..
                close (pipeHandles[0]);   // close the read handle

                if ((streamFlags & wantStdOut) != 0)
                    dup2 (pipeHandles[1], STDOUT_FILENO); // turns the pipe into stdout
                else
                    dup2 (open ("/dev/null", O_WRONLY), STDOUT_FILENO);

                if ((streamFlags & wantStdErr) != 0)
                    dup2 (pipeHandles[1], STDERR_FILENO);
                else
                    dup2 (open ("/dev/null", O_WRONLY), STDERR_FILENO);

                close (pipeHandles[1]);

                Array<char*> argv;

                for (auto& arg : arguments)
                    if (arg.isNotEmpty())
                        argv.add (const_cast<char*> (arg.toRawUTF8()));

                argv.add (nullptr);

                execvp (exe.toRawUTF8(), argv.getRawDataPointer());
                _exit (-1);
            }
            else
            {
                // we're the parent process..
                childPID = result;
                pipeHandle = pipeHandles[0];
                close (pipeHandles[1]); // close the write handle
            }
        }
    }

    ~ActiveProcess()
    {
        if (readHandle != nullptr)
            fclose (readHandle);

        if (pipeHandle != 0)
            close (pipeHandle);
    }

    bool isRunning() noexcept
    {
        if (childPID == 0)
            return false;

        int childState = 0;
        auto pid = waitpid (childPID, &childState, WNOHANG);

        if (pid == 0)
            return true;

        if (WIFEXITED (childState))
        {
            exitCode = WEXITSTATUS (childState);
            return false;
        }

        return ! WIFSIGNALED (childState);
    }

    int read (void* dest, int numBytes) noexcept
    {
        jassert (dest != nullptr && numBytes > 0);

        #ifdef fdopen
         #error // some crazy 3rd party headers (e.g. zlib) define this function as NULL!
        #endif

        if (readHandle == nullptr && childPID != 0)
            readHandle = fdopen (pipeHandle, "r");

        if (readHandle != nullptr)
        {
            for (;;)
            {
                auto numBytesRead = (int) fread (dest, 1, (size_t) numBytes, readHandle);

                if (numBytesRead > 0 || feof (readHandle))
                    return numBytesRead;

                // signal occurred during fread() so try again
                if (ferror (readHandle) && errno == EINTR)
                    continue;

                break;
            }
        }

        return 0;
    }

    bool killProcess() const noexcept
    {
        return ::kill (childPID, SIGKILL) == 0;
    }

    uint32 getExitCode() noexcept
    {
        if (exitCode >= 0)
            return (uint32) exitCode;

        if (childPID != 0)
        {
            int childState = 0;
            auto pid = waitpid (childPID, &childState, WNOHANG);

            if (pid >= 0 && WIFEXITED (childState))
            {
                exitCode = WEXITSTATUS (childState);
                return (uint32) exitCode;
            }
        }

        return 0;
    }

    int childPID = 0;
    int pipeHandle = 0;
    int exitCode = -1;
    FILE* readHandle = {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ActiveProcess)
};

bool ChildProcess::start (const String& command, int streamFlags)
{
    return start (StringArray::fromTokens (command, true), streamFlags);
}

bool ChildProcess::start (const StringArray& args, int streamFlags)
{
    if (args.size() == 0)
        return false;

    activeProcess.reset (new ActiveProcess (args, streamFlags));

    if (activeProcess->childPID == 0)
        activeProcess.reset();

    return activeProcess != nullptr;
}

#endif

} // namespace juce
