/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

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
                && [fm copyItemAtPath: juceStringToNS (fullPath)
                               toPath: juceStringToNS (dest.getFullPathName())
                                error: nil];
    }
}

void File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}


//==============================================================================
namespace MacFileHelpers
{
    static bool isFileOnDriveType (const File& f, const char* const* types)
    {
        struct statfs buf;

        if (juce_doStatFS (f, buf))
        {
            const String type (buf.f_fstypename);

            while (*types != nullptr)
                if (type.equalsIgnoreCase (*types++))
                    return true;
        }

        return false;
    }

    static bool isHiddenFile (const String& path)
    {
       #if JUCE_MAC
        JUCE_AUTORELEASEPOOL
        {
            NSNumber* hidden = nil;
            NSError* err = nil;

            return [createNSURLFromFile (path) getResourceValue: &hidden forKey: NSURLIsHiddenKey error: &err]
                     && [hidden boolValue];
        }
       #else
        return File (path).getFileName().startsWithChar ('.');
       #endif
    }

   #if JUCE_IOS
    static String getIOSSystemLocation (NSSearchPathDirectory type)
    {
        return nsStringToJuce ([NSSearchPathForDirectoriesInDomains (type, NSUserDomainMask, YES)
                                objectAtIndex: 0]);
    }
   #else
    static bool launchExecutable (const String& pathAndArguments)
    {
        auto cpid = fork();

        if (cpid == 0)
        {
            const char* const argv[4] = { "/bin/sh", "-c", pathAndArguments.toUTF8(), nullptr };

            // Child process
            if (execve (argv[0], (char**) argv, nullptr) < 0)
                exit (0);
        }
        else
        {
            if (cpid < 0)
                return false;
        }

        return true;
    }
   #endif
}

bool File::isOnCDRomDrive() const
{
    static const char* const cdTypes[] = { "cd9660", "cdfs", "cddafs", "udf", nullptr };

    return MacFileHelpers::isFileOnDriveType (*this, cdTypes);
}

bool File::isOnHardDisk() const
{
    static const char* const nonHDTypes[] = { "nfs", "smbfs", "ramfs", nullptr };

    return ! (isOnCDRomDrive() || MacFileHelpers::isFileOnDriveType (*this, nonHDTypes));
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
    return MacFileHelpers::isHiddenFile (getFullPathName());
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
            case userDocumentsDirectory:            resultPath = MacFileHelpers::getIOSSystemLocation (NSDocumentDirectory); break;
            case userDesktopDirectory:              resultPath = MacFileHelpers::getIOSSystemLocation (NSDesktopDirectory); break;

            case tempDirectory:
            {
                File tmp (MacFileHelpers::getIOSSystemLocation (NSCachesDirectory));
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
                return File (tmp.getFullPathName());
            }
          #endif
            case userMusicDirectory:                resultPath = "~/Music"; break;
            case userMoviesDirectory:               resultPath = "~/Movies"; break;
            case userPicturesDirectory:             resultPath = "~/Pictures"; break;
            case userApplicationDataDirectory:      resultPath = "~/Library"; break;
            case commonApplicationDataDirectory:    resultPath = "/Library"; break;
            case commonDocumentsDirectory:          resultPath = "/Users/Shared"; break;
            case globalApplicationsDirectory:       resultPath = "/Applications"; break;

            case invokedExecutableFile:
                if (juce_argv != nullptr && juce_argc > 0)
                    return File::getCurrentWorkingDirectory().getChildFile (String (CharPointer_UTF8 (juce_argv[0])));
                // deliberate fall-through...
                JUCE_FALLTHROUGH

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

                _NSGetExecutablePath (buffer.get(), &size);
                return File (String::fromUTF8 (buffer, (int) size));
            }

            default:
                jassertfalse; // unknown type?
                break;
        }

        if (resultPath.isNotEmpty())
            return File (resultPath.convertToPrecomposedUnicode());
    }

    return {};
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

    return {};
}

//==============================================================================
static NSString* getFileLink (const String& path)
{
    return [[NSFileManager defaultManager] destinationOfSymbolicLinkAtPath: juceStringToNS (path) error: nil];
}

bool File::isSymbolicLink() const
{
    return getFileLink (fullPath) != nil;
}

String File::getNativeLinkedTarget() const
{
    if (NSString* dest = getFileLink (fullPath))
        return nsStringToJuce (dest);

    return {};
}

//==============================================================================
bool File::moveToTrash() const
{
    if (! exists())
        return true;

    JUCE_AUTORELEASEPOOL
    {
        NSError* error = nil;
        return [[NSFileManager defaultManager] trashItemAtURL: createNSURLFromFile (*this)
                                             resultingItemURL: nil
                                                        error: &error];
    }
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildcard)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildcard)
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
                if (enumerator == nil)
                    return false;

                NSString* file = [enumerator nextObject];

                if (file == nil)
                    return false;

                [enumerator skipDescendents];
                filenameFound = nsStringToJuce (file).convertToPrecomposedUnicode();

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, filenameFound.toUTF8(), FNM_CASEFOLD) != 0)
                    continue;

                auto fullPath = parentDir + filenameFound;
                updateStatInfoForFile (fullPath, isDir, fileSize, modTime, creationTime, isReadOnly);

                if (isHidden != nullptr)
                    *isHidden = MacFileHelpers::isHiddenFile (fullPath);

                return true;
            }
        }
    }

private:
    String parentDir, wildCard;
    NSDirectoryEnumerator* enumerator = nil;

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
bool JUCE_CALLTYPE Process::openDocument (const String& fileName, [[maybe_unused]] const String& parameters)
{
    JUCE_AUTORELEASEPOOL
    {
        NSString* fileNameAsNS (juceStringToNS (fileName));
        NSURL* filenameAsURL = File::createFileWithoutCheckingPath (fileName).exists() ? [NSURL fileURLWithPath: fileNameAsNS]
                                                                                       : [NSURL URLWithString: fileNameAsNS];

      #if JUCE_IOS
        [[UIApplication sharedApplication] openURL: filenameAsURL
                                           options: @{}
                                 completionHandler: nil];

        return true;
      #else
        NSWorkspace* workspace = [NSWorkspace sharedWorkspace];

        if (parameters.isEmpty())
            return [workspace openURL: filenameAsURL];

        const File file (fileName);

        if (file.isBundle())
        {
            StringArray params;
            params.addTokens (parameters, true);

            NSMutableArray* paramArray = [[NSMutableArray new] autorelease];

            for (int i = 0; i < params.size(); ++i)
                [paramArray addObject: juceStringToNS (params[i])];

            if (@available (macOS 10.15, *))
            {
                auto config = [NSWorkspaceOpenConfiguration configuration];
                [config setCreatesNewApplicationInstance: YES];
                config.arguments = paramArray;

                [workspace openApplicationAtURL: filenameAsURL
                                  configuration: config
                              completionHandler: nil];

                return true;
            }

            JUCE_BEGIN_IGNORE_DEPRECATION_WARNINGS

            NSMutableDictionary* dict = [[NSMutableDictionary new] autorelease];

            [dict setObject: paramArray
                     forKey: nsStringLiteral ("NSWorkspaceLaunchConfigurationArguments")];

            return [workspace launchApplicationAtURL: filenameAsURL
                                             options: NSWorkspaceLaunchDefault | NSWorkspaceLaunchNewInstance
                                       configuration: dict
                                               error: nil];

            JUCE_END_IGNORE_DEPRECATION_WARNINGS
        }

        if (file.exists())
            return MacFileHelpers::launchExecutable ("\"" + fileName + "\" " + parameters);

        return false;
      #endif
    }
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
        NSDictionary* fileDict = [[NSFileManager defaultManager] attributesOfItemAtPath: juceStringToNS (getFullPathName()) error: nil];
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

File File::getContainerForSecurityApplicationGroupIdentifier (const String& appGroup)
{
    if (auto* url = [[NSFileManager defaultManager] containerURLForSecurityApplicationGroupIdentifier: juceStringToNS (appGroup)])
        return File (nsStringToJuce ([url path]));

    return File();
}

} // namespace juce
