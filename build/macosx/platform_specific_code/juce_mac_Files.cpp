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

#include "../../../src/juce_core/basics/juce_StandardHeader.h"
#include <ApplicationServices/ApplicationServices.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fnmatch.h>
#include <utime.h>
#include <pwd.h>
#include <fcntl.h>

BEGIN_JUCE_NAMESPACE

#include "../../../src/juce_core/io/files/juce_FileInputStream.h"
#include "../../../src/juce_core/io/files/juce_FileOutputStream.h"
#include "../../../src/juce_core/io/network/juce_URL.h"
#include "../../../src/juce_core/basics/juce_SystemStats.h"
#include "../../../src/juce_core/misc/juce_PlatformUtilities.h"
#include "../../../src/juce_core/io/files/juce_NamedPipe.h"
#include "../../../src/juce_core/threads/juce_InterProcessLock.h"
#include "../../../src/juce_core/threads/juce_Thread.h"

//==============================================================================
/*
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.cpp!
*/
#include "juce_posix_SharedCode.cpp"


//==============================================================================
static File executableFile;


//==============================================================================
void PlatformUtilities::copyToStr255 (Str255& d, const String& s)
{
    unsigned char* t = (unsigned char*) d;
    t[0] = jmin (254, s.length());
    s.copyToBuffer ((char*) t + 1, 254);
}

void PlatformUtilities::copyToStr63 (Str63& d, const String& s)
{
    unsigned char* t = (unsigned char*) d;
    t[0] = jmin (62, s.length());
    s.copyToBuffer ((char*) t + 1, 62);
}

const String PlatformUtilities::cfStringToJuceString (CFStringRef cfString)
{
    String result;

    if (cfString != 0)
    {
#if JUCE_STRINGS_ARE_UNICODE
        CFRange range = { 0, CFStringGetLength (cfString) };
        UniChar* const u = (UniChar*) juce_malloc (sizeof (UniChar) * (range.length + 1));

        CFStringGetCharacters (cfString, range, u);
        u[range.length] = 0;

        result = convertUTF16ToString (u);

        juce_free (u);
#else
        const int len = CFStringGetLength (cfString);
        char* buffer = (char*) juce_malloc (len + 1);
        CFStringGetCString (cfString, buffer, len + 1, CFStringGetSystemEncoding());
        result = buffer;
        juce_free (buffer);
#endif
    }

    return result;
}

CFStringRef PlatformUtilities::juceStringToCFString (const String& s)
{
#if JUCE_STRINGS_ARE_UNICODE
    const int len = s.length();
    const juce_wchar* t = (const juce_wchar*) s;

    UniChar* temp = (UniChar*) juce_malloc (sizeof (UniChar) * len + 4);

    for (int i = 0; i <= len; ++i)
        temp[i] = t[i];

    CFStringRef result = CFStringCreateWithCharacters (kCFAllocatorDefault, temp, len);
    juce_free (temp);

    return result;

#else
    return CFStringCreateWithCString (kCFAllocatorDefault,
                                      (const char*) s,
                                      CFStringGetSystemEncoding());
#endif
}

const String PlatformUtilities::convertUTF16ToString (const UniChar* utf16)
{
    String s;

    while (*utf16 != 0)
        s += (juce_wchar) *utf16++;

    return s;
}

const String PlatformUtilities::convertToPrecomposedUnicode (const String& s)
{
    UnicodeMapping map;

    map.unicodeEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                              kUnicodeNoSubset,
                                              kTextEncodingDefaultFormat);

    map.otherEncoding = CreateTextEncoding (kTextEncodingUnicodeDefault,
                                            kUnicodeCanonicalCompVariant,
                                            kTextEncodingDefaultFormat);

    map.mappingVersion = kUnicodeUseLatestMapping;

    UnicodeToTextInfo conversionInfo = 0;
    String result;

    if (CreateUnicodeToTextInfo (&map, &conversionInfo) == noErr)
    {
        const int len = s.length();

        UniChar* const tempIn = (UniChar*) juce_calloc (sizeof (UniChar) * len + 4);
        UniChar* const tempOut = (UniChar*) juce_calloc (sizeof (UniChar) * len + 4);

        for (int i = 0; i <= len; ++i)
            tempIn[i] = s[i];

        ByteCount bytesRead = 0;
        ByteCount outputBufferSize = 0;

        if (ConvertFromUnicodeToText (conversionInfo,
                                      len * sizeof (UniChar), tempIn,
                                      kUnicodeDefaultDirectionMask,
                                      0, 0, 0, 0,
                                      len * sizeof (UniChar), &bytesRead,
                                      &outputBufferSize, tempOut) == noErr)
        {
            result.preallocateStorage (bytesRead / sizeof (UniChar) + 2);

            tchar* t = const_cast <tchar*> ((const tchar*) result);

            int i;
            for (i = 0; i < bytesRead / sizeof (UniChar); ++i)
                t[i] = (tchar) tempOut[i];

            t[i] = 0;
        }

        juce_free (tempIn);
        juce_free (tempOut);

        DisposeUnicodeToTextInfo (&conversionInfo);
    }

    return result;
}

//==============================================================================
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
    const File destFile (dst);

    if (! destFile.create())
        return false;

    FSRef srcRef, dstRef;

    if (! (PlatformUtilities::makeFSRefFromPath (&srcRef, src)
            && PlatformUtilities::makeFSRefFromPath (&dstRef, dst)))
    {
        return false;
    }

    int okForks = 0;

    CatPositionRec iter;
    iter.initialize = 0;
    HFSUniStr255 forkName;

    // can't just copy the data because this is a bloody Mac, so we need to copy each
    // fork separately...
    while (FSIterateForks (&srcRef, &iter, &forkName, 0, 0) == noErr)
    {
        SInt16 srcForkNum = 0, dstForkNum = 0;
        OSErr err = FSOpenFork (&srcRef, forkName.length, forkName.unicode, fsRdPerm, &srcForkNum);

        if (err == noErr)
        {
            err = FSOpenFork (&dstRef, forkName.length, forkName.unicode, fsRdWrPerm, &dstForkNum);

            if (err == noErr)
            {
                MemoryBlock buf (32768);
                SInt64 pos = 0;

                for (;;)
                {
                    ByteCount bytesRead = 0;
                    err = FSReadFork (srcForkNum, fsFromStart, pos, buf.getSize(), (char*) buf, &bytesRead);

                    if (bytesRead > 0)
                    {
                        err = FSWriteFork (dstForkNum, fsFromStart, pos, bytesRead, (const char*) buf, &bytesRead);
                        pos += bytesRead;
                    }

                    if (err != noErr)
                    {
                        if (err == eofErr)
                            ++okForks;

                        break;
                    }
                }

                FSFlushFork (dstForkNum);
                FSCloseFork (dstForkNum);
            }

            FSCloseFork (srcForkNum);
        }
    }

    if (okForks > 0) // some files seem to be ok even if not all their forks get copied..
    {
        // copy permissions..
        struct stat info;
        if (juce_stat (src, info))
            chmod (dst.toUTF8(), info.st_mode & 0777);

        return true;
    }

    return false;
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
    const char* resultPath = 0;

    switch (type)
    {
    case userHomeDirectory:
        resultPath = getenv ("HOME");

        if (resultPath == 0)
        {
            struct passwd* const pw = getpwuid (getuid());
            if (pw != 0)
                resultPath = pw->pw_dir;
        }

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
    CFStringRef bundleIdStringRef = PlatformUtilities::juceStringToCFString (bundleId);
    CFBundleRef bundleRef = CFBundleGetBundleWithIdentifier (bundleIdStringRef);
    CFRelease (bundleIdStringRef);

    if (bundleRef != 0)
    {
        CFURLRef exeURLRef = CFBundleCopyExecutableURL (bundleRef);

        if (exeURLRef != 0)
        {
            CFStringRef pathStringRef = CFURLCopyFileSystemPath (exeURLRef, kCFURLPOSIXPathStyle);
            CFRelease (exeURLRef);

            if (pathStringRef != 0)
            {
                juce_setCurrentExecutableFileName (PlatformUtilities::cfStringToJuceString (pathStringRef));
                CFRelease (pathStringRef);
            }
        }
    }
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
                result = PlatformUtilities::convertToPrecomposedUnicode (String::fromUTF8 ((const uint8*) de->d_name));

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
    bool ok = false;

    if (fileName.startsWithIgnoreCase (T("http:"))
        || fileName.startsWithIgnoreCase (T("https:"))
        || fileName.startsWithIgnoreCase (T("ftp:"))
        || fileName.startsWithIgnoreCase (T("file:")))
    {
        CFStringRef urlString = PlatformUtilities::juceStringToCFString (fileName);

        if (urlString != 0)
        {
            CFURLRef url = CFURLCreateWithString (kCFAllocatorDefault,
                                                  urlString, 0);
            CFRelease (urlString);

            if (url != 0)
            {
                ok = (LSOpenCFURLRef (url, 0) == noErr);
                CFRelease (url);
            }
        }
    }
    else
    {
        FSRef ref;
        if (PlatformUtilities::makeFSRefFromPath (&ref, fileName))
        {
            if (juce_isDirectory (fileName) && parameters.isNotEmpty())
            {
                // if we're launching a bundled app with a document..
                StringArray docs;
                docs.addTokens (parameters, true);
                FSRef* docRefs = new FSRef [docs.size()];

                for (int i = 0; i < docs.size(); ++i)
                    PlatformUtilities::makeFSRefFromPath (docRefs + i, docs[i]);

                LSLaunchFSRefSpec ors;
                ors.appRef = &ref;
                ors.numDocs = docs.size();
                ors.itemRefs = docRefs;
                ors.passThruParams = 0;
                ors.launchFlags = kLSLaunchDefaults;
                ors.asyncRefCon = 0;

                FSRef actual;
                ok = (LSOpenFromRefSpec (&ors, &actual) == noErr);

                delete docRefs;
            }
            else
            {
                if (parameters.isNotEmpty())
                    ok = juce_launchExecutable (T("\"") + fileName + T("\" ") + parameters);
                else
                    ok = (LSOpenFSRef (&ref, 0) == noErr);
            }
        }
    }

    return ok;
}

//==============================================================================
bool PlatformUtilities::makeFSSpecFromPath (FSSpec* fs, const String& path)
{
    FSRef ref;

    return makeFSRefFromPath (&ref, path)
            && FSGetCatalogInfo (&ref, kFSCatInfoNone, 0, 0, fs, 0) == noErr;
}

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
    FSRef fs;
    if (makeFSRefFromPath (&fs, filename))
    {
        LSItemInfoRecord info;

        if (LSCopyItemInfoForRef (&fs, kLSRequestTypeCreator, &info) == noErr)
            return info.filetype;
    }

    return 0;
}

bool PlatformUtilities::isBundle (const String& filename)
{
    FSRef fs;
    if (makeFSRefFromPath (&fs, filename))
    {
        LSItemInfoRecord info;

        if (LSCopyItemInfoForRef (&fs, kLSItemInfoIsPackage, &info) == noErr)
            return (info.flags & kLSItemInfoIsPackage) != 0;
    }

    return false;
}


END_JUCE_NAMESPACE
