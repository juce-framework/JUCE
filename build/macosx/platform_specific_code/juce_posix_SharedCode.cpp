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

    bool ok = true;
    pthread_mutex_lock (&es->mutex);

    if (! es->triggered)
    {
        if (timeOutMillisecs < 0)
        {
            pthread_cond_wait (&es->condition, &es->mutex);
        }
        else
        {
            struct timespec time;

#if JUCE_MAC
            time.tv_sec = timeOutMillisecs / 1000;
            time.tv_nsec = (timeOutMillisecs % 1000) * 1000000;
            pthread_cond_timedwait_relative_np (&es->condition, &es->mutex, &time);
#else
            struct timeval t;
            int timeout = 0;

            gettimeofday (&t, 0);

            time.tv_sec  = t.tv_sec  + (timeOutMillisecs / 1000);
            time.tv_nsec = (t.tv_usec + ((timeOutMillisecs % 1000) * 1000)) * 1000;

            while (time.tv_nsec >= 1000000000)
            {
                time.tv_nsec -= 1000000000;
                time.tv_sec++;
            }

            while (! timeout)
            {
                timeout = pthread_cond_timedwait (&es->condition, &es->mutex, &time);

                if (! timeout)
                    // Success
                    break;

                if (timeout == EINTR)
                    // Go round again
                    timeout = 0;
            }
#endif
        }

        ok = es->triggered;
    }

    es->triggered = false;

    pthread_mutex_unlock (&es->mutex);
    return ok;
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
void JUCE_CALLTYPE Thread::sleep (int millisecs) throw()
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
bool juce_copyFile (const String& s, const String& d) throw();

static bool juce_stat (const String& fileName, struct stat& info) throw()
{
    return fileName.isNotEmpty()
            && (stat (fileName.toUTF8(), &info) == 0);
}

bool juce_isDirectory (const String& fileName) throw()
{
    struct stat info;

    return fileName.isEmpty()
            || (juce_stat (fileName, info)
                  && ((info.st_mode & S_IFDIR) != 0));
}

bool juce_fileExists (const String& fileName, const bool dontCountDirectories) throw()
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

int64 juce_getFileSize (const String& fileName) throw()
{
    struct stat info;
    return juce_stat (fileName, info) ? info.st_size : 0;
}

//==============================================================================
bool juce_canWriteToFile (const String& fileName) throw()
{
    return access (fileName.toUTF8(), W_OK) == 0;
}

bool juce_deleteFile (const String& fileName) throw()
{
    const char* const fileNameUTF8 = fileName.toUTF8();

    if (juce_isDirectory (fileName))
        return rmdir (fileNameUTF8) == 0;
    else
        return remove (fileNameUTF8) == 0;
}

bool juce_moveFile (const String& source, const String& dest) throw()
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

void juce_createDirectory (const String& fileName) throw()
{
    mkdir (fileName.toUTF8(), 0777);
}

void* juce_fileOpen (const String& fileName, bool forWriting) throw()
{
    const char* const fileNameUTF8 = fileName.toUTF8();
    int flags = O_RDONLY;

    if (forWriting)
    {
        if (juce_fileExists (fileName, false))
        {
            const int f = open (fileNameUTF8, O_RDWR, 00644);

            if (f != -1)
                lseek (f, 0, SEEK_END);

            return (void*) f;
        }
        else
        {
            flags = O_RDWR + O_CREAT;
        }
    }

    return (void*) open (fileNameUTF8, flags, 00644);
}

void juce_fileClose (void* handle) throw()
{
    if (handle != 0)
        close ((int) handle);
}

int juce_fileRead (void* handle, void* buffer, int size) throw()
{
    if (handle != 0)
        return read ((int) handle, buffer, size);

    return 0;
}

int juce_fileWrite (void* handle, const void* buffer, int size) throw()
{
    if (handle != 0)
        return write ((int) handle, buffer, size);

    return 0;
}

int64 juce_fileSetPosition (void* handle, int64 pos) throw()
{
    if (handle != 0 && lseek ((int) handle, pos, SEEK_SET) == pos)
        return pos;

    return -1;
}

int64 juce_fileGetPosition (void* handle) throw()
{
    if (handle != 0)
        return lseek ((int) handle, 0, SEEK_CUR);
    else
        return -1;
}

void juce_fileFlush (void* handle) throw()
{
    if (handle != 0)
        fsync ((int) handle);
}

//==============================================================================
// if this file doesn't exist, find a parent of it that does..
static bool doStatFS (const File* file, struct statfs& result) throw()
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

int64 File::getBytesFreeOnVolume() const throw()
{
    int64 free_space = 0;

    struct statfs buf;
    if (doStatFS (this, buf))
        // Note: this returns space available to non-super user
        free_space = (int64) buf.f_bsize * (int64) buf.f_bavail;

    return free_space;
}

const String juce_getVolumeLabel (const String& filenameOnVolume,
                                  int& volumeSerialNumber) throw()
{
    // There is no equivalent on Linux
    volumeSerialNumber = 0;
    return String::empty;
}

//==============================================================================
#if JUCE_64BIT
  #define filedesc ((long long) internal)
#else
  #define filedesc ((int) internal)
#endif

InterProcessLock::InterProcessLock (const String& name_) throw()
    : internal (0),
      name (name_),
      reentrancyLevel (0)
{
    const File tempDir (File::getSpecialLocation (File::tempDirectory));
    const File temp (tempDir.getChildFile (name));
    temp.create();

    internal = (void*) open (temp.getFullPathName().toUTF8(), O_RDWR);
}

InterProcessLock::~InterProcessLock() throw()
{
    while (reentrancyLevel > 0)
        this->exit();

    close (filedesc);
}

bool InterProcessLock::enter (const int timeOutMillisecs) throw()
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
        const int result = fcntl (filedesc, F_SETLK, &fl);

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

void InterProcessLock::exit() throw()
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
            const int result = fcntl (filedesc, F_SETLKW, &fl);

            if (result >= 0 || errno != EINTR)
                break;
        }
    }
}

