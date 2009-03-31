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

#include "linuxincludes.h"
#include "../../../src/juce_core/basics/juce_StandardHeader.h"

#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/ptrace.h>
#include <sys/vfs.h>        // for statfs
#include <sys/wait.h>
#include <unistd.h>
#include <fnmatch.h>
#include <utime.h>
#include <pwd.h>
#include <fcntl.h>

#define U_ISOFS_SUPER_MAGIC     (short) 0x9660   // linux/iso_fs.h
#define U_MSDOS_SUPER_MAGIC     (short) 0x4d44   // linux/msdos_fs.h
#define U_NFS_SUPER_MAGIC       (short) 0x6969   // linux/nfs_fs.h
#define U_SMB_SUPER_MAGIC       (short) 0x517B   // linux/smb_fs.h


BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_FileInputStream.h"
#include "../../../src/juce_core/io/files/juce_FileOutputStream.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/basics/juce_Time.h"
#include "../../../src/juce_core/basics/juce_Random.h" 
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/threads/juce_Thread.h"

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/
#include "../../macosx/platform_specific_code/juce_posix_SharedCode.h"


//==============================================================================
static File executableFile;


//==============================================================================
void juce_getFileTimes (const String& fileName,
                        int64& modificationTime,
                        int64& accessTime,
                        int64& creationTime) throw()
{
    modificationTime = 0;
    accessTime = 0;
    creationTime = 0;

    struct stat info;
    const int res = stat (fileName.toUTF8(), &info);
    if (res == 0)
    {
        modificationTime = (int64) info.st_mtime * 1000;
        accessTime = (int64) info.st_atime * 1000;
        creationTime = (int64) info.st_ctime * 1000;
    }
}

bool juce_setFileTimes (const String& fileName,
                        int64 modificationTime,
                        int64 accessTime,
                        int64 creationTime) throw()
{
    struct utimbuf times;
    times.actime = (time_t) (accessTime / 1000);
    times.modtime = (time_t) (modificationTime / 1000);

    return utime (fileName.toUTF8(), &times) == 0;
}

bool juce_setFileReadOnly (const String& fileName, bool isReadOnly) throw()
{
    struct stat info;
    const int res = stat (fileName.toUTF8(), &info);
    if (res != 0)
        return false;

    info.st_mode &= 0777;   // Just permissions

    if( isReadOnly )
        info.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    else
        // Give everybody write permission?
        info.st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

    return chmod (fileName.toUTF8(), info.st_mode) == 0;
}

bool juce_copyFile (const String& s, const String& d) throw()
{
    const File source (s), dest (d);

    FileInputStream* in = source.createInputStream();
    bool ok = false;

    if (in != 0)
    {
        if (dest.deleteFile())
        {
            FileOutputStream* const out = dest.createOutputStream();

            if (out != 0)
            {
                const int bytesCopied = out->writeFromInputStream (*in, -1);
                delete out;

                ok = (bytesCopied == source.getSize());

                if (! ok)
                    dest.deleteFile();
            }
        }

        delete in;
    }

    return ok;
}

const StringArray juce_getFileSystemRoots() throw()
{
    StringArray s;
    s.add (T("/"));
    return s;
}

//==============================================================================
bool File::isOnCDRomDrive() const throw()
{
    struct statfs buf;

    if (statfs (getFullPathName().toUTF8(), &buf) == 0)
        return (buf.f_type == U_ISOFS_SUPER_MAGIC);

    // Assume not if this fails for some reason
    return false;
}

bool File::isOnHardDisk() const throw()
{
    struct statfs buf;

    if (statfs (getFullPathName().toUTF8(), &buf) == 0)
    {
        switch (buf.f_type)
        {
            case U_ISOFS_SUPER_MAGIC:   // CD-ROM
            case U_MSDOS_SUPER_MAGIC:   // Probably floppy (but could be mounted FAT filesystem)
            case U_NFS_SUPER_MAGIC:     // Network NFS
            case U_SMB_SUPER_MAGIC:     // Network Samba
                return false;

            default:
                // Assume anything else is a hard-disk (but note it could
                // be a RAM disk.  There isn't a good way of determining
                // this for sure)
                return true;
        }
    }

    // Assume so if this fails for some reason
    return true;
}

bool File::isOnRemovableDrive() const throw()
{
    jassertfalse // xxx not implemented for linux!
    return false;
}

bool File::isHidden() const throw()
{
    return getFileName().startsWithChar (T('.'));
}

//==============================================================================
const File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
    case userHomeDirectory:
    {
        const char* homeDir = getenv ("HOME");

        if (homeDir == 0)
        {
            struct passwd* const pw = getpwuid (getuid());
            if (pw != 0)
                homeDir = pw->pw_dir;
        }

        return File (String::fromUTF8 ((const uint8*) homeDir));
    }

    case userDocumentsDirectory:
    case userMusicDirectory:
    case userMoviesDirectory:
    case userApplicationDataDirectory:
        return File ("~");

    case userDesktopDirectory:
        return File ("~/Desktop");

    case commonApplicationDataDirectory:
        return File ("/var");

    case globalApplicationsDirectory:
        return File ("/usr");

    case tempDirectory:
    {
        File tmp ("/var/tmp");

        if (! tmp.isDirectory())
        {
            tmp = T("/tmp");

            if (! tmp.isDirectory())
                tmp = File::getCurrentWorkingDirectory();
        }

        return tmp;
    }

    case currentExecutableFile:
    case currentApplicationFile:
        // if this fails, it's probably because juce_setCurrentExecutableFileName()
        // was never called to set the filename - this should be done by the juce
        // main() function, so maybe you've hacked it to use your own custom main()?
        jassert (executableFile.exists());

        return executableFile;

    default:
        jassertfalse // unknown type?
        break;
    }

    return File::nonexistent;
}

//==============================================================================
void juce_setCurrentExecutableFileName (const String& filename) throw()
{
    executableFile = File::getCurrentWorkingDirectory().getChildFile (filename);
}

//==============================================================================
const File File::getCurrentWorkingDirectory() throw()
{
    char buf [2048];
    return File (String::fromUTF8 ((const uint8*) getcwd (buf, sizeof (buf))));
}

bool File::setAsCurrentWorkingDirectory() const throw()
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
const String File::getVersion() const throw()
{
    return String::empty; // xxx not yet implemented
}

//==============================================================================
const File File::getLinkedTarget() const throw()
{
    char buffer [4096];
    size_t numChars = readlink ((const char*) getFullPathName().toUTF8(),
                                buffer, sizeof (buffer));

    if (numChars > 0 && numChars <= sizeof (buffer))
        return File (String::fromUTF8 ((const uint8*) buffer, (int) numChars));

    return *this;
}

//==============================================================================
bool File::moveToTrash() const throw()
{
    if (! exists())
        return true;

    File trashCan (T("~/.Trash"));

    if (! trashCan.isDirectory())
        trashCan = T("~/.local/share/Trash/files");

    if (! trashCan.isDirectory())
        return false;

    return moveFileTo (trashCan.getNonexistentChildFile (getFileNameWithoutExtension(),
                                                         getFileExtension()));
}

//==============================================================================
struct FindFileStruct
{
    String parentDir, wildCard;
    DIR* dir;

    bool getNextMatch (String& result, bool* const isDir, bool* const isHidden, int64* const fileSize,
                       Time* const modTime, Time* const creationTime, bool* const isReadOnly) throw()
    {
        const char* const wildcardUTF8 = wildCard.toUTF8();

        for (;;)
        {
            struct dirent* const de = readdir (dir);

            if (de == 0)
                break;

            if (fnmatch (wildcardUTF8, de->d_name, FNM_CASEFOLD) == 0)
            {
                result = String::fromUTF8 ((const uint8*) de->d_name);

                const String path (parentDir + result);

                if (isDir != 0 || fileSize != 0)
                {
                    struct stat info;
                    const bool statOk = (stat (path.toUTF8(), &info) == 0);

                    if (isDir != 0)
                        *isDir = path.isEmpty() || (statOk && ((info.st_mode & S_IFDIR) != 0));

                    if (isHidden != 0)
                        *isHidden = (de->d_name[0] == '.');

                    if (fileSize != 0)
                        *fileSize = statOk ? info.st_size : 0;
                }

                if (modTime != 0 || creationTime != 0)
                {
                    int64 m, a, c;
                    juce_getFileTimes (path, m, a, c);

                    if (modTime != 0)
                        *modTime = m;

                    if (creationTime != 0)
                        *creationTime = c;
                }

                if (isReadOnly != 0)
                    *isReadOnly = ! juce_canWriteToFile (path);

                return true;
            }
        }

        return false;
    }
};

// returns 0 on failure
void* juce_findFileStart (const String& directory, const String& wildCard, String& firstResultFile,
                          bool* isDir, bool* isHidden, int64* fileSize, Time* modTime,
                          Time* creationTime, bool* isReadOnly) throw()
{
    DIR* d = opendir (directory.toUTF8());

    if (d != 0)
    {
        FindFileStruct* ff = new FindFileStruct();
        ff->parentDir = directory;

        if (!ff->parentDir.endsWithChar (File::separator))
            ff->parentDir += File::separator;

        ff->wildCard = wildCard;
        if (wildCard == T("*.*"))
            ff->wildCard = T("*");

        ff->dir = d;

        if (ff->getNextMatch (firstResultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly))
        {
            return ff;
        }
        else
        {
            firstResultFile = String::empty;
            isDir = false;
            isHidden = false;
            closedir (d);
            delete ff;
        }
    }

    return 0;
}

bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDir, bool* isHidden, int64* fileSize, Time* modTime, Time* creationTime, bool* isReadOnly) throw()
{
    FindFileStruct* const ff = (FindFileStruct*) handle;

    if (ff != 0)
        return ff->getNextMatch (resultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);

    return false;
}

void juce_findFileClose (void* handle) throw()
{
    FindFileStruct* const ff = (FindFileStruct*) handle;

    if (ff != 0)
    {
        closedir (ff->dir);
        delete ff;
    }
}

bool juce_launchFile (const String& fileName,
                      const String& parameters) throw()
{
    String cmdString (fileName);
    cmdString << " " << parameters;

    if (URL::isProbablyAWebsiteURL (fileName)
         || URL::isProbablyAnEmailAddress (fileName))
    {
        // create a command that tries to launch a bunch of likely browsers
        const char* const browserNames[] = { "/etc/alternatives/x-www-browser", "firefox", "mozilla", "konqueror", "opera" };

        StringArray cmdLines;

        for (int i = 0; i < numElementsInArray (browserNames); ++i)
            cmdLines.add (String (browserNames[i]) + T(" ") + cmdString.trim().quoted());

        cmdString = cmdLines.joinIntoString (T(" || "));
    }

    if (cmdString.startsWithIgnoreCase (T("file:")))
        cmdString = cmdString.substring (5);

    const char* const argv[4] = { "/bin/sh", "-c", (const char*) cmdString.toUTF8(), 0 };

    const int cpid = fork();

    if (cpid == 0)
    {
        setsid();

        // Child process
        execve (argv[0], (char**) argv, environ);
        exit (0);
    }

    return cpid >= 0;
}


END_JUCE_NAMESPACE
