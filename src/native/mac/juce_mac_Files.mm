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

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
void juce_getFileTimes (const String& fileName,
                        int64& modificationTime,
                        int64& accessTime,
                        int64& creationTime)
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
                        int64 creationTime)
{
    struct utimbuf times;
    times.actime = (time_t) (accessTime / 1000);
    times.modtime = (time_t) (modificationTime / 1000);

    return utime (fileName.toUTF8(), &times) == 0;
}

bool juce_setFileReadOnly (const String& fileName, bool isReadOnly)
{
    struct stat info;
    const int res = stat (fileName.toUTF8(), &info);
    if (res != 0)
        return false;

    info.st_mode &= 0777;   // Just permissions

    if (isReadOnly)
        info.st_mode &= ~(S_IWUSR | S_IWGRP | S_IWOTH);
    else
        // Give everybody write permission?
        info.st_mode |= S_IWUSR | S_IWGRP | S_IWOTH;

    return chmod (fileName.toUTF8(), info.st_mode) == 0;
}

bool juce_copyFile (const String& src, const String& dst)
{
    const ScopedAutoReleasePool pool;
    NSFileManager* fm = [NSFileManager defaultManager];

    return [fm fileExistsAtPath: juceStringToNS (src)]
#if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
            && [fm copyItemAtPath: juceStringToNS (src)
                           toPath: juceStringToNS (dst)
                            error: nil];
#else
            && [fm copyPath: juceStringToNS (src)
                     toPath: juceStringToNS (dst)
                    handler: nil];
#endif
}

const StringArray juce_getFileSystemRoots()
{
    StringArray s;
    s.add (T("/"));
    return s;
}

//==============================================================================
static bool isFileOnDriveType (const File* const f, const char** types)
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

bool File::isOnCDRomDrive() const
{
    static const char* const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", 0 };

    return isFileOnDriveType (this, (const char**) cdTypes);
}

bool File::isOnHardDisk() const
{
    static const char* const nonHDTypes[] = { "nfs", "smbfs", "ramfs", 0 };

    return ! (isOnCDRomDrive() || isFileOnDriveType (this, (const char**) nonHDTypes));
}

bool File::isOnRemovableDrive() const
{
#if JUCE_IPHONE
    return false; // xxx is this possible?
#else
    const ScopedAutoReleasePool pool;
    BOOL removable = false;

    [[NSWorkspace sharedWorkspace]
           getFileSystemInfoForPath: juceStringToNS (getFullPathName())
                        isRemovable: &removable
                         isWritable: nil
                      isUnmountable: nil
                        description: nil
                               type: nil];

    return removable;
#endif
}

static bool juce_isHiddenFile (const String& path)
{
#if JUCE_IPHONE
    return File (path).getFileName().startsWithChar (T('.'));
#else
    FSRef ref;
    if (! PlatformUtilities::makeFSRefFromPath (&ref, path))
        return false;

    FSCatalogInfo info;
    FSGetCatalogInfo (&ref, kFSCatInfoNodeFlags | kFSCatInfoFinderInfo, &info, 0, 0, 0);

    if ((info.nodeFlags & kFSNodeIsDirectoryBit) != 0)
        return (((FolderInfo*) &info.finderInfo)->finderFlags & kIsInvisible) != 0;

    return (((FileInfo*) &info.finderInfo)->finderFlags & kIsInvisible) != 0;
#endif
}

bool File::isHidden() const
{
    return juce_isHiddenFile (getFullPathName());
}

//==============================================================================
const char* juce_Argv0 = 0;  // referenced from juce_Application.cpp

const File File::getSpecialLocation (const SpecialLocationType type)
{
    const ScopedAutoReleasePool pool;

    String resultPath;

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
        File tmp (T("~/Library/Caches/") + juce_getExecutableFile().getFileNameWithoutExtension());

        tmp.createDirectory();
        return tmp.getFullPathName();
    }

    case invokedExecutableFile:
        if (juce_Argv0 != 0)
            return File (String::fromUTF8 ((const uint8*) juce_Argv0));
        // deliberate fall-through...

    case currentExecutableFile:
        return juce_getExecutableFile();

    case currentApplicationFile:
    {
        const File exe (juce_getExecutableFile());
        const File parent (exe.getParentDirectory());

        return parent.getFullPathName().endsWithIgnoreCase (T("Contents/MacOS"))
                ? parent.getParentDirectory().getParentDirectory()
                : exe;
    }

    default:
        jassertfalse // unknown type?
        break;
    }

    if (resultPath != 0)
        return File (PlatformUtilities::convertToPrecomposedUnicode (resultPath));

    return File::nonexistent;
}

//==============================================================================
const File File::getCurrentWorkingDirectory()
{
    char buf [2048];
    getcwd (buf, sizeof(buf));

    return File (PlatformUtilities::convertToPrecomposedUnicode (buf));
}

bool File::setAsCurrentWorkingDirectory() const
{
    return chdir (getFullPathName().toUTF8()) == 0;
}

//==============================================================================
const String File::getVersion() const
{
    const ScopedAutoReleasePool pool;
    String result;

    NSBundle* bundle = [NSBundle bundleWithPath: juceStringToNS (getFullPathName())];

    if (bundle != 0)
    {
        NSDictionary* info = [bundle infoDictionary];

        if (info != 0)
        {
            NSString* name = [info valueForKey: @"CFBundleShortVersionString"];

            if (name != nil)
                result = nsStringToJuce (name);
        }
    }

    return result;
}

//==============================================================================
const File File::getLinkedTarget() const
{
#if JUCE_IPHONE || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
    NSString* dest = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath: juceStringToNS (getFullPathName()) error: nil];

#else
    NSString* dest = [[NSFileManager defaultManager] pathContentOfSymbolicLinkAtPath: juceStringToNS (getFullPathName())];
#endif

    if (dest != nil)
        return File (nsStringToJuce (dest));

    return *this;
}

//==============================================================================
bool File::moveToTrash() const
{
    if (! exists())
        return true;

#if JUCE_IPHONE
    return deleteFile(); //xxx is there a trashcan on the iPhone?
#else
    const ScopedAutoReleasePool pool;

    NSString* p = juceStringToNS (getFullPathName());

    return [[NSWorkspace sharedWorkspace]
                performFileOperation: NSWorkspaceRecycleOperation
                              source: [p stringByDeletingLastPathComponent]
                         destination: @""
                               files: [NSArray arrayWithObject: [p lastPathComponent]]
                                 tag: nil ];
#endif
}

//==============================================================================
struct FindFileStruct
{
    NSDirectoryEnumerator* enumerator;
    String parentDir, wildCard;
};

bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDir,  bool* isHidden, int64* fileSize, Time* modTime, Time* creationTime, bool* isReadOnly)
{
    FindFileStruct* ff = (FindFileStruct*) handle;
    NSString* file;
    const char* const wildcardUTF8 = ff->wildCard.toUTF8();

    for (;;)
    {
        if (ff == 0 || (file = [ff->enumerator nextObject]) == 0)
            return false;

        [ff->enumerator skipDescendents];
        resultFile = nsStringToJuce (file);

        if (fnmatch (wildcardUTF8, resultFile.toUTF8(), FNM_CASEFOLD) != 0)
            continue;

        const String path (ff->parentDir + resultFile);

        if (isDir != 0 || fileSize != 0)
        {
            struct stat info;
            const bool statOk = juce_stat (path, info);

            if (isDir != 0)
                *isDir = statOk && ((info.st_mode & S_IFDIR) != 0);

            if (isHidden != 0)
                *isHidden = juce_isHiddenFile (path);

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

void* juce_findFileStart (const String& directory, const String& wildCard, String& firstResultFile,
                          bool* isDir, bool* isHidden, int64* fileSize, Time* modTime,
                          Time* creationTime, bool* isReadOnly)
{
    NSDirectoryEnumerator* e = [[NSFileManager defaultManager] enumeratorAtPath: juceStringToNS (directory)];

    if (e != 0)
    {
        ScopedPointer <FindFileStruct> ff (new FindFileStruct());
        ff->enumerator = [e retain];
        ff->parentDir = directory;
        ff->wildCard = wildCard;

        if (! ff->parentDir.endsWithChar (File::separator))
            ff->parentDir += File::separator;

        if (juce_findFileNext (ff, firstResultFile, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly))
            return ff.release();

        [e release];
    }

    return 0;
}

void juce_findFileClose (void* handle)
{
    ScopedPointer <FindFileStruct> ff ((FindFileStruct*) handle);
    [ff->enumerator release];
}

//==============================================================================
bool juce_launchExecutable (const String& pathAndArguments)
{
    const char* const argv[4] = { "/bin/sh", "-c", (const char*) pathAndArguments, 0 };

    const int cpid = fork();

    if (cpid == 0)
    {
        // Child process
        if (execve (argv[0], (char**) argv, 0) < 0)
            exit (0);
    }
    else
    {
        if (cpid < 0)
            return false;
    }

    return true;
}

bool juce_launchFile (const String& fileName, const String& parameters)
{
#if JUCE_IPHONE
    return [[UIApplication sharedApplication] openURL: [NSURL fileURLWithPath: juceStringToNS (fileName)]];
#else
    const ScopedAutoReleasePool pool;

    if (parameters.isEmpty())
    {
        return [[NSWorkspace sharedWorkspace] openFile: juceStringToNS (fileName)]
            || [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: juceStringToNS (fileName)]];
    }

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
                                                 options: 0
                          additionalEventParamDescriptor: nil
                                       launchIdentifiers: nil];
        }
        else
        {
            ok = juce_launchExecutable (T("\"") + fileName + T("\" ") + parameters);
        }
    }

    return ok;
#endif
}

void File::revealToUser() const
{
#if ! JUCE_IPHONE
    if (exists())
        [[NSWorkspace sharedWorkspace] selectFile: juceStringToNS (getFullPathName()) inFileViewerRootedAtPath: @""];
    else if (getParentDirectory().exists())
        getParentDirectory().revealToUser();
#endif
}

//==============================================================================
#if ! JUCE_IPHONE
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
#endif

//==============================================================================
OSType PlatformUtilities::getTypeOfFile (const String& filename)
{
    const ScopedAutoReleasePool pool;

#if JUCE_IPHONE || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5)
    NSDictionary* fileDict = [[NSFileManager defaultManager] attributesOfItemAtPath: juceStringToNS (filename) error: nil];
#else
    NSDictionary* fileDict = [[NSFileManager defaultManager] fileAttributesAtPath: juceStringToNS (filename) traverseLink: NO];
#endif
    //return (OSType) [fileDict objectForKey: NSFileHFSTypeCode];
    return [fileDict fileHFSTypeCode];
}

bool PlatformUtilities::isBundle (const String& filename)
{
#if JUCE_IPHONE
    return false; // xxx can't find a sensible way to do this without trying to open the bundle..
#else
    const ScopedAutoReleasePool pool;
    return [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (filename)];
#endif
}

#endif
