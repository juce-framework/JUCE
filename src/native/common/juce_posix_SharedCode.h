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

    WaitableEventImpl (const WaitableEventImpl&);
    WaitableEventImpl& operator= (const WaitableEventImpl&);
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
static bool juce_stat (const String& fileName, struct stat& info)
{
    return fileName.isNotEmpty()
            && (stat (fileName.toUTF8(), &info) == 0);
}

bool File::isDirectory() const
{
    struct stat info;

    return fullPath.isEmpty()
            || (juce_stat (fullPath, info) && ((info.st_mode & S_IFDIR) != 0));
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
    struct stat info;
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
    struct stat info;
    const int res = stat (fullPath.toUTF8(), &info);
    if (res != 0)
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

    struct stat info;
    const int res = stat (fullPath.toUTF8(), &info);
    if (res == 0)
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

void* juce_fileOpen (const File& file, bool forWriting)
{
    int flags = O_RDONLY;

    if (forWriting)
    {
        if (file.exists())
        {
            const int f = open (file.getFullPathName().toUTF8(), O_RDWR, 00644);

            if (f != -1)
                lseek (f, 0, SEEK_END);

            return (void*) f;
        }
        else
        {
            flags = O_RDWR + O_CREAT;
        }
    }

    return (void*) open (file.getFullPathName().toUTF8(), flags, 00644);
}

void juce_fileClose (void* handle)
{
    if (handle != 0)
        close ((int) (pointer_sized_int) handle);
}

int juce_fileRead (void* handle, void* buffer, int size)
{
    if (handle != 0)
        return jmax (0, (int) read ((int) (pointer_sized_int) handle, buffer, size));

    return 0;
}

int juce_fileWrite (void* handle, const void* buffer, int size)
{
    if (handle != 0)
        return (int) write ((int) (pointer_sized_int) handle, buffer, size);

    return 0;
}

int64 juce_fileSetPosition (void* handle, int64 pos)
{
    if (handle != 0 && lseek ((int) (pointer_sized_int) handle, pos, SEEK_SET) == pos)
        return pos;

    return -1;
}

int64 FileOutputStream::getPositionInternal() const
{
    if (fileHandle != 0)
        return lseek ((int) (pointer_sized_int) fileHandle, 0, SEEK_CUR);

    return -1;
}

void FileOutputStream::flushInternal()
{
    if (fileHandle != 0)
        fsync ((int) (pointer_sized_int) fileHandle);
}

const File juce_getExecutableFile()
{
    Dl_info exeInfo;
    dladdr ((const void*) juce_getExecutableFile, &exeInfo);
    return File::getCurrentWorkingDirectory().getChildFile (String::fromUTF8 (exeInfo.dli_fname));
}

//==============================================================================
// if this file doesn't exist, find a parent of it that does..
static bool juce_doStatFS (File f, struct statfs& result)
{
    for (int i = 5; --i >= 0;)
    {
        if (f.exists())
            break;

        f = f.getParentDirectory();
    }

    return statfs (f.getFullPathName().toUTF8(), &result) == 0;
}

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
