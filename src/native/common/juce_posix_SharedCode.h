/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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
struct EventStruct
{
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    bool triggered;
};

WaitableEvent::WaitableEvent() throw()
{
    EventStruct* const es = new EventStruct();
    es->triggered = false;

    pthread_cond_init (&es->condition, 0);
    pthread_mutex_init (&es->mutex, 0);

    internal = es;
}

WaitableEvent::~WaitableEvent() throw()
{
    EventStruct* const es = (EventStruct*) internal;

    pthread_cond_destroy (&es->condition);
    pthread_mutex_destroy (&es->mutex);

    delete es;
}

bool WaitableEvent::wait (const int timeOutMillisecs) const throw()
{
    EventStruct* const es = (EventStruct*) internal;
    pthread_mutex_lock (&es->mutex);

    if (timeOutMillisecs < 0)
    {
        while (! es->triggered)
            pthread_cond_wait (&es->condition, &es->mutex);
    }
    else
    {
        while (! es->triggered)
        {
            struct timeval t;
            gettimeofday (&t, 0);

            struct timespec time;
            time.tv_sec  = t.tv_sec  + (timeOutMillisecs / 1000);
            time.tv_nsec = (t.tv_usec + ((timeOutMillisecs % 1000) * 1000)) * 1000;

            if (time.tv_nsec >= 1000000000)
            {
                time.tv_nsec -= 1000000000;
                time.tv_sec++;
            }

            if (pthread_cond_timedwait (&es->condition, &es->mutex, &time) == ETIMEDOUT)
            {
                pthread_mutex_unlock (&es->mutex);
                return false;
            }
        }
    }

    es->triggered = false;
    pthread_mutex_unlock (&es->mutex);
    return true;
}

void WaitableEvent::signal() const throw()
{
    EventStruct* const es = (EventStruct*) internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = true;
    pthread_cond_broadcast (&es->condition);
    pthread_mutex_unlock (&es->mutex);
}

void WaitableEvent::reset() const throw()
{
    EventStruct* const es = (EventStruct*) internal;

    pthread_mutex_lock (&es->mutex);
    es->triggered = false;
    pthread_mutex_unlock (&es->mutex);
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
const tchar File::separator = T('/');
const tchar* File::separatorString = T("/");


//==============================================================================
bool juce_copyFile (const String& s, const String& d);

static bool juce_stat (const String& fileName, struct stat& info)
{
    return fileName.isNotEmpty()
            && (stat (fileName.toUTF8(), &info) == 0);
}

bool juce_isDirectory (const String& fileName)
{
    struct stat info;

    return fileName.isEmpty()
            || (juce_stat (fileName, info)
                  && ((info.st_mode & S_IFDIR) != 0));
}

bool juce_fileExists (const String& fileName, const bool dontCountDirectories)
{
    if (fileName.isEmpty())
        return false;

    const char* const fileNameUTF8 = fileName.toUTF8();
    bool exists = access (fileNameUTF8, F_OK) == 0;

    if (exists && dontCountDirectories)
    {
        struct stat info;
        const int res = stat (fileNameUTF8, &info);

        if (res == 0 && (info.st_mode & S_IFDIR) != 0)
            exists = false;
    }

    return exists;
}

int64 juce_getFileSize (const String& fileName)
{
    struct stat info;
    return juce_stat (fileName, info) ? info.st_size : 0;
}

//==============================================================================
bool juce_canWriteToFile (const String& fileName)
{
    return access (fileName.toUTF8(), W_OK) == 0;
}

bool juce_deleteFile (const String& fileName)
{
    if (juce_isDirectory (fileName))
        return rmdir ((const char*) fileName.toUTF8()) == 0;
    else
        return remove ((const char*) fileName.toUTF8()) == 0;
}

bool juce_moveFile (const String& source, const String& dest)
{
    if (rename (source.toUTF8(), dest.toUTF8()) == 0)
        return true;

    if (juce_canWriteToFile (source)
         && juce_copyFile (source, dest))
    {
        if (juce_deleteFile (source))
            return true;

        juce_deleteFile (dest);
    }

    return false;
}

void juce_createDirectory (const String& fileName)
{
    mkdir (fileName.toUTF8(), 0777);
}

void* juce_fileOpen (const String& fileName, bool forWriting)
{
    int flags = O_RDONLY;

    if (forWriting)
    {
        if (juce_fileExists (fileName, false))
        {
            const int f = open ((const char*) fileName.toUTF8(), O_RDWR, 00644);

            if (f != -1)
                lseek (f, 0, SEEK_END);

            return (void*) f;
        }
        else
        {
            flags = O_RDWR + O_CREAT;
        }
    }

    return (void*) open ((const char*) fileName.toUTF8(), flags, 00644);
}

void juce_fileClose (void* handle)
{
    if (handle != 0)
        close ((int) (pointer_sized_int) handle);
}

int juce_fileRead (void* handle, void* buffer, int size)
{
    if (handle != 0)
        return (int) read ((int) (pointer_sized_int) handle, buffer, size);

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

int64 juce_fileGetPosition (void* handle)
{
    if (handle != 0)
        return lseek ((int) (pointer_sized_int) handle, 0, SEEK_CUR);
    else
        return -1;
}

void juce_fileFlush (void* handle)
{
    if (handle != 0)
        fsync ((int) (pointer_sized_int) handle);
}

const File juce_getExecutableFile()
{
    Dl_info exeInfo;
    dladdr ((const void*) juce_getExecutableFile, &exeInfo);
    return File (exeInfo.dli_fname);
}

//==============================================================================
// if this file doesn't exist, find a parent of it that does..
static bool doStatFS (const File* file, struct statfs& result)
{
    File f (*file);

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
    if (doStatFS (this, buf))
        return (int64) buf.f_bsize * (int64) buf.f_bavail; // Note: this returns space available to non-super user

    return 0;
}

int64 File::getVolumeTotalSize() const
{
    struct statfs buf;
    if (doStatFS (this, buf))
        return (int64) buf.f_bsize * (int64) buf.f_blocks;

    return 0;
}

const String juce_getVolumeLabel (const String& filenameOnVolume,
                                  int& volumeSerialNumber)
{
    volumeSerialNumber = 0;

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

    File f (filenameOnVolume);

    for (;;)
    {
        if (getattrlist ((const char*) f.getFullPathName().toUTF8(),
                         &attrList, &attrBuf, sizeof(attrBuf), 0) == 0)
        {
            return String::fromUTF8 (((const uint8*) &attrBuf.mountPointRef) + attrBuf.mountPointRef.attr_dataoffset,
                                     (int) attrBuf.mountPointRef.attr_length);
        }

        const File parent (f.getParentDirectory());

        if (f == parent)
            break;

        f = parent;
    }
#endif

    return String::empty;
}


//==============================================================================
void juce_runSystemCommand (const String& command)
{
    int result = system ((const char*) command.toUTF8());
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
InterProcessLock::InterProcessLock (const String& name_)
    : internal (0),
      name (name_),
      reentrancyLevel (0)
{
#if JUCE_MAC
    // (don't use getSpecialLocation() to avoid the temp folder being different for each app)
    const File temp (File (T("~/Library/Caches/Juce")).getChildFile (name));
#else
    const File temp (File::getSpecialLocation (File::tempDirectory).getChildFile (name));
#endif

    temp.create();

    internal = open (temp.getFullPathName().toUTF8(), O_RDWR);
}

InterProcessLock::~InterProcessLock()
{
    while (reentrancyLevel > 0)
        this->exit();

    close (internal);
}

bool InterProcessLock::enter (const int timeOutMillisecs)
{
    if (internal == 0)
        return false;

    if (reentrancyLevel != 0)
        return true;

    const int64 endTime = Time::currentTimeMillis() + timeOutMillisecs;

    struct flock fl;
    zerostruct (fl);
    fl.l_whence = SEEK_SET;
    fl.l_type = F_WRLCK;

    for (;;)
    {
        const int result = fcntl (internal, F_SETLK, &fl);

        if (result >= 0)
        {
            ++reentrancyLevel;
            return true;
        }

        if (errno != EINTR)
        {
            if (timeOutMillisecs == 0
                 || (timeOutMillisecs > 0 && Time::currentTimeMillis() >= endTime))
                break;

            Thread::sleep (10);
        }
    }

    return false;
}

void InterProcessLock::exit()
{
    if (reentrancyLevel > 0 && internal != 0)
    {
        --reentrancyLevel;

        struct flock fl;
        zerostruct (fl);
        fl.l_whence = SEEK_SET;
        fl.l_type = F_UNLCK;

        for (;;)
        {
            const int result = fcntl (internal, F_SETLKW, &fl);

            if (result >= 0 || errno != EINTR)
                break;
        }
    }
}
