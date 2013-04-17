/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
    Note that a lot of methods that you'd expect to find in this file actually
    live in juce_posix_SharedCode.h!
*/

//==============================================================================
bool File::copyInternal (const File& dest) const
{
    JUCE_AUTORELEASEPOOL
    {
        NSFileManager* fm = [NSFileManager defaultManager];

        return [fm fileExistsAtPath: juceStringToNS (fullPath)]
               #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_6
                && [fm copyItemAtPath: juceStringToNS (fullPath)
                               toPath: juceStringToNS (dest.getFullPathName())
                                error: nil];
               #else
                && [fm copyPath: juceStringToNS (fullPath)
                         toPath: juceStringToNS (dest.getFullPathName())
                        handler: nil];
               #endif
    }
}

void File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}


//==============================================================================
namespace FileHelpers
{
    static bool isFileOnDriveType (const File& f, const char* const* types)
    {
        struct statfs buf;

        if (juce_doStatFS (f, buf))
        {
            const String type (buf.f_fstypename);

            while (*types != 0)
                if (type.equalsIgnoreCase (*types++))
                    return true;
        }

        return false;
    }

    static bool isHiddenFile (const String& path)
    {
       #if defined (MAC_OS_X_VERSION_10_6) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_6
        JUCE_AUTORELEASEPOOL
        {
            NSNumber* hidden = nil;
            NSError* err = nil;

            return [[NSURL fileURLWithPath: juceStringToNS (path)]
                        getResourceValue: &hidden forKey: NSURLIsHiddenKey error: &err]
                    && [hidden boolValue];
        }
       #elif JUCE_IOS
        return File (path).getFileName().startsWithChar ('.');
       #else
        FSRef ref;
        LSItemInfoRecord info;

        return FSPathMakeRefWithOptions ((const UInt8*) path.toRawUTF8(), kFSPathMakeRefDoNotFollowLeafSymlink, &ref, 0) == noErr
                 && LSCopyItemInfoForRef (&ref, kLSRequestBasicFlagsOnly, &info) == noErr
                 && (info.flags & kLSItemInfoIsInvisible) != 0;
       #endif
    }

   #if JUCE_IOS
    String getIOSSystemLocation (NSSearchPathDirectory type)
    {
        return nsStringToJuce ([NSSearchPathForDirectoriesInDomains (type, NSUserDomainMask, YES)
                                objectAtIndex: 0]);
    }
   #endif

    static bool launchExecutable (const String& pathAndArguments)
    {
        const char* const argv[4] = { "/bin/sh", "-c", pathAndArguments.toUTF8(), 0 };

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
}

bool File::isOnCDRomDrive() const
{
    const char* const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", 0 };

    return FileHelpers::isFileOnDriveType (*this, cdTypes);
}

bool File::isOnHardDisk() const
{
    const char* const nonHDTypes[] = { "nfs", "smbfs", "ramfs", 0 };

    return ! (isOnCDRomDrive() || FileHelpers::isFileOnDriveType (*this, nonHDTypes));
}

bool File::isOnRemovableDrive() const
{
   #if JUCE_IOS
    return false; // xxx is this possible?
   #else
    JUCE_AUTORELEASEPOOL
    {
        BOOL removable = false;

        [[NSWorkspace sharedWorkspace]
               getFileSystemInfoForPath: juceStringToNS (getFullPathName())
                            isRemovable: &removable
                             isWritable: nil
                          isUnmountable: nil
                            description: nil
                                   type: nil];

        return removable;
    }
   #endif
}

bool File::isHidden() const
{
    return FileHelpers::isHiddenFile (getFullPathName());
}

//==============================================================================
const char* const* juce_argv = nullptr;
int juce_argc = 0;

File File::getSpecialLocation (const SpecialLocationType type)
{
    JUCE_AUTORELEASEPOOL
    {
        String resultPath;

        switch (type)
        {
            case userHomeDirectory:                 resultPath = nsStringToJuce (NSHomeDirectory()); break;

          #if JUCE_IOS
            case userDocumentsDirectory:            resultPath = FileHelpers::getIOSSystemLocation (NSDocumentDirectory); break;
            case userDesktopDirectory:              resultPath = FileHelpers::getIOSSystemLocation (NSDesktopDirectory); break;

            case tempDirectory:
            {
                File tmp (FileHelpers::getIOSSystemLocation (NSCachesDirectory));
                tmp = tmp.getChildFile (juce_getExecutableFile().getFileNameWithoutExtension());
                tmp.createDirectory();
                return tmp.getFullPathName();
            }

          #else
            case userDocumentsDirectory:            resultPath = "~/Documents"; break;
            case userDesktopDirectory:              resultPath = "~/Desktop"; break;

            case tempDirectory:
            {
                File tmp ("~/Library/Caches/" + juce_getExecutableFile().getFileNameWithoutExtension());
                tmp.createDirectory();
                return tmp.getFullPathName();
            }
          #endif
            case userMusicDirectory:                resultPath = "~/Music"; break;
            case userMoviesDirectory:               resultPath = "~/Movies"; break;
            case userPicturesDirectory:             resultPath = "~/Pictures"; break;
            case userApplicationDataDirectory:      resultPath = "~/Library"; break;
            case commonApplicationDataDirectory:    resultPath = "/Library"; break;
            case globalApplicationsDirectory:       resultPath = "/Applications"; break;

            case invokedExecutableFile:
                if (juce_argv != nullptr && juce_argc > 0)
                    return File (CharPointer_UTF8 (juce_argv[0]));
                // deliberate fall-through...

            case currentExecutableFile:
                return juce_getExecutableFile();

            case currentApplicationFile:
            {
                const File exe (juce_getExecutableFile());
                const File parent (exe.getParentDirectory());

              #if JUCE_IOS
                return parent;
              #else
                return parent.getFullPathName().endsWithIgnoreCase ("Contents/MacOS")
                        ? parent.getParentDirectory().getParentDirectory()
                        : exe;
              #endif
            }

            case hostApplicationPath:
            {
                unsigned int size = 8192;
                HeapBlock<char> buffer;
                buffer.calloc (size + 8);

                _NSGetExecutablePath (buffer.getData(), &size);
                return String::fromUTF8 (buffer, (int) size);
            }

            default:
                jassertfalse; // unknown type?
                break;
        }

        if (resultPath.isNotEmpty())
            return File (resultPath.convertToPrecomposedUnicode());
    }

    return File::nonexistent;
}

//==============================================================================
String File::getVersion() const
{
    JUCE_AUTORELEASEPOOL
    {
        if (NSBundle* bundle = [NSBundle bundleWithPath: juceStringToNS (getFullPathName())])
            if (NSDictionary* info = [bundle infoDictionary])
                if (NSString* name = [info valueForKey: nsStringLiteral ("CFBundleShortVersionString")])
                    return nsStringToJuce (name);
    }

    return String::empty;
}

//==============================================================================
File File::getLinkedTarget() const
{
   #if JUCE_IOS || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
    NSString* dest = [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath: juceStringToNS (getFullPathName()) error: nil];
   #else
    // (the cast here avoids a deprecation warning)
    NSString* dest = [((id) [NSFileManager defaultManager]) pathContentOfSymbolicLinkAtPath: juceStringToNS (getFullPathName())];
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

   #if JUCE_IOS
    return deleteFile(); //xxx is there a trashcan on the iOS?
   #else
    JUCE_AUTORELEASEPOOL
    {
        NSString* p = juceStringToNS (getFullPathName());

        return [[NSWorkspace sharedWorkspace]
                    performFileOperation: NSWorkspaceRecycleOperation
                                  source: [p stringByDeletingLastPathComponent]
                             destination: nsEmptyString()
                                   files: [NSArray arrayWithObject: [p lastPathComponent]]
                                     tag: nil ];
    }
   #endif
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildCard_)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildCard_),
          enumerator (nil)
    {
        JUCE_AUTORELEASEPOOL
        {
            enumerator = [[[NSFileManager defaultManager] enumeratorAtPath: juceStringToNS (directory.getFullPathName())] retain];
        }
    }

    ~Pimpl()
    {
        [enumerator release];
    }

    bool next (String& filenameFound,
               bool* const isDir, bool* const isHidden, int64* const fileSize,
               Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        JUCE_AUTORELEASEPOOL
        {
            const char* wildcardUTF8 = nullptr;

            for (;;)
            {
                NSString* file;
                if (enumerator == nil || (file = [enumerator nextObject]) == nil)
                    return false;

                [enumerator skipDescendents];
                filenameFound = nsStringToJuce (file);

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, filenameFound.toUTF8(), FNM_CASEFOLD) != 0)
                    continue;

                const String fullPath (parentDir + filenameFound);
                updateStatInfoForFile (fullPath, isDir, fileSize, modTime, creationTime, isReadOnly);

                if (isHidden != nullptr)
                    *isHidden = FileHelpers::isHiddenFile (fullPath);

                return true;
            }
        }
    }

private:
    String parentDir, wildCard;
    NSDirectoryEnumerator* enumerator;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildcard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildcard))
{
}

DirectoryIterator::NativeIterator::~NativeIterator()
{
}

bool DirectoryIterator::NativeIterator::next (String& filenameFound,
                                              bool* const isDir, bool* const isHidden, int64* const fileSize,
                                              Time* const modTime, Time* const creationTime, bool* const isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}


//==============================================================================
bool Process::openDocument (const String& fileName, const String& parameters)
{
  #if JUCE_IOS
    return [[UIApplication sharedApplication] openURL: [NSURL URLWithString: juceStringToNS (fileName)]];
  #else
    JUCE_AUTORELEASEPOOL
    {
        if (parameters.isEmpty())
        {
            return [[NSWorkspace sharedWorkspace] openFile: juceStringToNS (fileName)]
                || [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString: juceStringToNS (fileName)]];
        }

        bool ok = false;
        const File file (fileName);

        if (file.isBundle())
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
        else if (file.exists())
        {
            ok = FileHelpers::launchExecutable ("\"" + fileName + "\" " + parameters);
        }

        return ok;
    }
  #endif
}

void File::revealToUser() const
{
   #if ! JUCE_IOS
    if (exists())
        [[NSWorkspace sharedWorkspace] selectFile: juceStringToNS (getFullPathName()) inFileViewerRootedAtPath: nsEmptyString()];
    else if (getParentDirectory().exists())
        getParentDirectory().revealToUser();
   #endif
}

//==============================================================================
OSType File::getMacOSType() const
{
    JUCE_AUTORELEASEPOOL
    {
       #if JUCE_IOS || (defined (MAC_OS_X_VERSION_10_5) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5)
        NSDictionary* fileDict = [[NSFileManager defaultManager] attributesOfItemAtPath: juceStringToNS (getFullPathName()) error: nil];
       #else
        // (the cast here avoids a deprecation warning)
        NSDictionary* fileDict = [((id) [NSFileManager defaultManager]) fileAttributesAtPath: juceStringToNS (getFullPathName()) traverseLink: NO];
       #endif

        return [fileDict fileHFSTypeCode];
    }
}

bool File::isBundle() const
{
   #if JUCE_IOS
    return false; // xxx can't find a sensible way to do this without trying to open the bundle..
   #else
    JUCE_AUTORELEASEPOOL
    {
        return [[NSWorkspace sharedWorkspace] isFilePackageAtPath: juceStringToNS (getFullPathName())];
    }
   #endif
}

#if JUCE_MAC
void File::addToDock() const
{
    // check that it's not already there...
    if (! juce_getOutputFromCommand ("defaults read com.apple.dock persistent-apps").containsIgnoreCase (getFullPathName()))
    {
        juce_runSystemCommand ("defaults write com.apple.dock persistent-apps -array-add \"<dict><key>tile-data</key><dict><key>file-data</key><dict><key>_CFURLString</key><string>"
                                 + getFullPathName() + "</string><key>_CFURLStringType</key><integer>0</integer></dict></dict></dict>\"");

        juce_runSystemCommand ("osascript -e \"tell application \\\"Dock\\\" to quit\"");
    }
}
#endif
