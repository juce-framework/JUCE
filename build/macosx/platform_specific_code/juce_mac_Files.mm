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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#ifdef JUCE_INCLUDED_FILE

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
static File executableFile;

const unsigned int macTimeToUnixTimeDiff = 0x7c25be90;

static uint64 utcDateTimeToUnixTime (const UTCDateTime& d) throw()
{
    if (d.highSeconds == 0 && d.lowSeconds == 0 && d.fraction == 0)
        return  0;

    return (((((uint64) d.highSeconds) << 32) | (uint64) d.lowSeconds) * 1000)
            + ((d.fraction * 1000) >> 16)
            - 2082844800000ll;
}

static void unixTimeToUtcDateTime (uint64 t, UTCDateTime& d) throw()
{
    if (t != 0)
        t += 2082844800000ll;

    d.highSeconds = (t / 1000) >> 32;
    d.lowSeconds = (t / 1000) & (uint64) 0xffffffff;
    d.fraction = ((t % 1000) << 16) / 1000;
}

void juce_getFileTimes (const String& fileName,
                        int64& modificationTime,
                        int64& accessTime,
                        int64& creationTime) throw()
{
    modificationTime = 0;
    accessTime = 0;
    creationTime = 0;

    FSRef fileRef;
    if (PlatformUtilities::makeFSRefFromPath (&fileRef, fileName))
    {
        FSRefParam info;
        zerostruct (info);

        info.ref = &fileRef;
        info.whichInfo = kFSCatInfoAllDates;

        FSCatalogInfo catInfo;
        info.catInfo = &catInfo;

        if (PBGetCatalogInfoSync (&info) == noErr)
        {
            creationTime = utcDateTimeToUnixTime (catInfo.createDate);
            accessTime = utcDateTimeToUnixTime (catInfo.accessDate);
            modificationTime = utcDateTimeToUnixTime (catInfo.contentModDate);
        }
    }
}

bool juce_setFileTimes (const String& fileName,
                        int64 modificationTime,
                        int64 accessTime,
                        int64 creationTime) throw()
{
    FSRef fileRef;
    if (PlatformUtilities::makeFSRefFromPath (&fileRef, fileName))
    {
        FSRefParam info;
        zerostruct (info);

        info.ref = &fileRef;
        info.whichInfo = kFSCatInfoAllDates;

        FSCatalogInfo catInfo;
        info.catInfo = &catInfo;

        if (PBGetCatalogInfoSync (&info) == noErr)
        {
            if (creationTime != 0)
                unixTimeToUtcDateTime (creationTime, catInfo.createDate);

            if (modificationTime != 0)
                unixTimeToUtcDateTime (modificationTime, catInfo.contentModDate);

            if (accessTime != 0)
                unixTimeToUtcDateTime (accessTime, catInfo.accessDate);

            return PBSetCatalogInfoSync (&info) == noErr;
        }
    }

    return false;
}

bool juce_setFileReadOnly (const String& fileName, bool isReadOnly) throw()
{
    const char* const fileNameUTF8 = fileName.toUTF8();

    struct stat info;
    const int res = stat (fileNameUTF8, &info);

    bool ok = false;

    if (res == 0)
    {
        info.st_mode &= 0777;   // Just permissions

        if (isReadOnly)
            info.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
        else
            // Give everybody write permission?
            info.st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

        ok = chmod (fileNameUTF8, info.st_mode) == 0;
    }

    return ok;
}

bool juce_copyFile (const String& src, const String& dst) throw()
{
    const ScopedAutoReleasePool pool;
    NSFileManager* fm = [NSFileManager defaultManager];

    return [fm fileExistsAtPath: juceStringToNS (src)]
            && [fm copyPath: juceStringToNS (src)
                     toPath: juceStringToNS (dst)
                    handler: nil];
}

const StringArray juce_getFileSystemRoots() throw()
{
    StringArray s;
    s.add (T("/"));
    return s;
}

//==============================================================================
static bool isFileOnDriveType (const File* const f, const char** types) throw()
{
    struct statfs buf;

    if (doStatFS (f, buf))
    {
        const String type (buf.f_fstypename);

        while (*types != 0)
            if (type.equalsIgnoreCase (*types++))
                return true;
    }

    return false;
}

bool File::isOnCDRomDrive() const throw()
{
    static const char* const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", 0 };

    return isFileOnDriveType (this, (const char**) cdTypes);
}

bool File::isOnHardDisk() const throw()
{
    static const char* const nonHDTypes[] = { "nfs", "smbfs", "ramfs", 0 };

    return ! (isOnCDRomDrive() || isFileOnDriveType (this, (const char**) nonHDTypes));
}

bool File::isOnRemovableDrive() const throw()
{
    jassertfalse // xxx not implemented for mac!
    return false;
}

static bool juce_isHiddenFile (const String& path) throw()
{
    FSRef ref;
    if (! PlatformUtilities::makeFSRefFromPath (&ref, path))
        return false;

    FSCatalogInfo info;
    FSGetCatalogInfo (&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &info, 0, 0, 0);

    if ((info.nodeFlags & kFSNodeIsDirectoryBit) != 0)
        return (((FolderInfo*) &info.finderInfo)->finderFlags & kIsInvisible) != 0;

    return (((FileInfo*) &info.finderInfo)->finderFlags & kIsInvisible) != 0;
}

bool File::isHidden() const throw()
{
    return juce_isHiddenFile (getFullPathName());
}

//==============================================================================
const File File::getSpecialLocation (const SpecialLocationType type)
{
    const ScopedAutoReleasePool pool;

    const char* resultPath = 0;

    switch (type)
    {
    case userHomeDirectory:
        resultPath = nsStringToJuce (NSHomeDirectory());
        break;

    case userDocumentsDirectory:
        resultPath = "~/Documents";
        break;

    case userDesktopDirectory:
        resultPath = "~/Desktop";
        break;

    case userApplicationDataDirectory:
        resultPath = "~/Library";
        break;

    case commonApplicationDataDirectory:
        resultPath = "/Library";
        break;

    case globalApplicationsDirectory:
        resultPath = "/Applications";
        break;

    case userMusicDirectory:
        resultPath = "~/Music";
        break;

    case userMoviesDirectory:
        resultPath = "~/Movies";
        break;

    case tempDirectory:
    {
        File tmp (T("~/Library/Caches/") + executableFile.getFileNameWithoutExtension());

        tmp.createDirectory();
        return tmp.getFullPathName();
    }

    case currentExecutableFile:
        return executableFile;

    case currentApplicationFile:
    {
        const File parent (executableFile.getParentDirectory());

        return parent.getFullPathName().endsWithIgnoreCase (T("Contents/MacOS"))
                ? parent.getParentDirectory().getParentDirectory()
                : executableFile;
    }

    default:
        jassertfalse // unknown type?
        break;
    }

    if (resultPath != 0)
        return File (PlatformUtilities::convertToPrecomposedUnicode (resultPath));

    return File::nonexistent;
}

void juce_setCurrentExecutableFileName (const String& filename) throw()
{
    executableFile = File::getCurrentWorkingDirectory()
                        .getChildFile (PlatformUtilities::convertToPrecomposedUnicode (filename));
}

void juce_setCurrentExecutableFileNameFromBundleId (const String& bundleId) throw()
{
    const ScopedAutoReleasePool pool;

    NSBundle* b = [NSBundle bundleWithIdentifier: juceStringToNS (bundleId)];

    if (b != nil)
        executableFile = nsStringToJuce ([b executablePath]);
}

//==============================================================================
const File File::getCurrentWorkingDirectory() throw()
{
    char buf [2048];
    getcwd (buf, sizeof(buf));

    return File (PlatformUtilities::convertToPrecomposedUnicode (buf));
}

bool File::setAsCurrentWorkingDirectory() const throw()
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
struct FindFileStruct
{
    String parentDir, wildCard;
    DIR* dir;

    bool getNextMatch (String& result, bool* const isDir, bool* const isHidden, int64* const fileSize,
                       Time* const modTime, Time* const creationTime, bool* const isReadOnly) throw()
    {
        const char* const wildCardUTF8 = wildCard.toUTF8();

        for (;;)
        {
            struct dirent* const de = readdir (dir);

            if (de == 0)
                break;

            if (fnmatch (wildCardUTF8, de->d_name, 0) == 0)
            {
                result = String::fromUTF8 ((const uint8*) de->d_name);

                const String path (parentDir + result);

                if (isDir != 0 || fileSize != 0)
                {
                    struct stat info;
                    const bool statOk = juce_stat (path, info);

                    if (isDir != 0)
                        *isDir = path.isEmpty() || (statOk && ((info.st_mode & S_IFDIR) != 0));

                    if (isHidden != 0)
                        *isHidden = (de->d_name[0] == '.')
                                      || juce_isHiddenFile (path);

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
    DIR* const d = opendir (directory.toUTF8());

    if (d != 0)
    {
        FindFileStruct* const ff = new FindFileStruct();
        ff->parentDir = directory;

        if (!ff->parentDir.endsWithChar (File::separator))
            ff->parentDir += File::separator;

        ff->wildCard = wildCard;
        ff->dir = d;

        if (ff->getNextMatch (firstResultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly))
        {
            return ff;
        }
        else
        {
            firstResultFile = String::empty;
            isDir = false;
            closedir (d);
            delete ff;
        }
    }

    return 0;
}

bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDir,  bool* isHidden, int64* fileSize, Time* modTime, Time* creationTime, bool* isReadOnly) throw()
{
    FindFileStruct* const ff = (FindFileStruct*) handle;

    if (ff != 0)
        return ff->getNextMatch (resultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);

    return false;
}

void juce_findFileClose (void* handle) throw()
{
    FindFileStruct* const ff = (FindFileStruct*)handle;

    if (ff != 0)
    {
        closedir (ff->dir);
        delete ff;
    }
}

//==============================================================================
bool juce_launchExecutable (const String& pathAndArguments) throw()
{
    char* const argv[4] = { "/bin/sh", "-c", (char*) (const char*) pathAndArguments, 0 };

    const int cpid = fork();

    if (cpid == 0)
    {
        // Child process
        if (execve (argv[0], argv, 0) < 0)
            exit (0);
    }
    else
    {
        if (cpid < 0)
            return false;
    }

    return true;
}

bool juce_launchFile (const String& fileName,
                      const String& parameters) throw()
{
    const ScopedAutoReleasePool pool;

    if (parameters.isEmpty())
        return [[NSWorkspace sharedWorkspace] openFile: juceStringToNS (fileName)];

    bool ok = false;

    FSRef ref;
    if (PlatformUtilities::makeFSRefFromPath (&ref, fileName))
    {
        if (PlatformUtilities::isBundle (fileName))
        {
            NSMutableArray* urls = [NSMutableArray array];

            StringArray docs;
            docs.addTokens (parameters, true);
            for (int i = 0; i < docs.size(); ++i)
                [urls addObject: juceStringToNS (docs[i])];

            ok = [[NSWorkspace sharedWorkspace] openURLs: urls
                                 withAppBundleIdentifier: [[NSBundle bundleWithPath: juceStringToNS (fileName)] bundleIdentifier]
                                                 options: nil
                          additionalEventParamDescriptor: nil
                                       launchIdentifiers: nil];
        }
        else
        {
            ok = juce_launchExecutable (T("\"") + fileName + T("\" ") + parameters);
        }
    }

    return ok;
}

//==============================================================================
bool PlatformUtilities::makeFSRefFromPath (FSRef* destFSRef, const String& path)
{
    return FSPathMakeRef ((const UInt8*) path.toUTF8(), destFSRef, 0) == noErr;
}

const String PlatformUtilities::makePathFromFSRef (FSRef* file)
{
    uint8 path [2048];
    zeromem (path, sizeof (path));

    String result;

    if (FSRefMakePath (file, (UInt8*) path, sizeof (path) - 1) == noErr)
        result = String::fromUTF8 (path);

    return PlatformUtilities::convertToPrecomposedUnicode (result);
}

//==============================================================================
OSType PlatformUtilities::getTypeOfFile (const String& filename)
{
    const ScopedAutoReleasePool pool;
    return NSHFSTypeCodeFromFileType (NSHFSTypeOfFile (juceStringToNS (filename)));
}

bool PlatformUtilities::isBundle (const String& filename)
{
    const ScopedAutoReleasePool pool;
    return [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (filename)];
}

#endif
