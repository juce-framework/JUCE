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

bool File::copyInternal (const File& dest) const
{
    FileInputStream in (*this);

    if (dest.deleteFile())
    {
        {
            FileOutputStream out (dest);

            if (out.failedToOpen())
                return false;

            if (out.writeFromInputStream (in, -1) == getSize())
                return true;
        }

        dest.deleteFile();
    }

    return false;
}

void File::findFileSystemRoots (Array<File>& destArray)
{
    destArray.add (File ("/"));
}

//==============================================================================
bool File::isOnCDRomDrive() const
{
    return false;
}

bool File::isOnHardDisk() const
{
    return true;
}

bool File::isOnRemovableDrive() const
{
    return false;
}

bool File::isHidden() const
{
    return getFileName().startsWithChar ('.');
}

//==============================================================================
namespace
{
    File juce_readlink (const String& file, const File& defaultFile)
    {
        const int size = 8192;
        HeapBlock<char> buffer;
        buffer.malloc (size + 4);

        const size_t numBytes = readlink (file.toUTF8(), buffer, size);

        if (numBytes > 0 && numBytes <= size)
            return File (file).getSiblingFile (String::fromUTF8 (buffer, (int) numBytes));

        return defaultFile;
    }
}

File File::getLinkedTarget() const
{
    return juce_readlink (getFullPathName().toUTF8(), *this);
}

//==============================================================================
File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        case userDocumentsDirectory:
        case userMusicDirectory:
        case userMoviesDirectory:
        case userPicturesDirectory:
        case userApplicationDataDirectory:
        case userDesktopDirectory:
            return File (android.appDataDir);

        case commonApplicationDataDirectory:
            return File (android.appDataDir);

        case globalApplicationsDirectory:
            return File ("/system/app");

        case tempDirectory:
            //return File (AndroidStatsHelpers::getSystemProperty ("java.io.tmpdir"));
            return File (android.appDataDir).getChildFile (".temp");

        case invokedExecutableFile:
        case currentExecutableFile:
        case currentApplicationFile:
        case hostApplicationPath:
            return juce_getExecutableFile();

        default:
            jassertfalse; // unknown type?
            break;
    }

    return File::nonexistent;
}

//==============================================================================
String File::getVersion() const
{
    return String::empty;
}

//==============================================================================
bool File::moveToTrash() const
{
    if (! exists())
        return true;

    // TODO

    return false;
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildCard_)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildCard_),
          dir (opendir (directory.getFullPathName().toUTF8()))
    {
    }

    ~Pimpl()
    {
        if (dir != 0)
            closedir (dir);
    }

    bool next (String& filenameFound,
               bool* const isDir, bool* const isHidden, int64* const fileSize,
               Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        if (dir != 0)
        {
            const char* wildcardUTF8 = nullptr;

            for (;;)
            {
                struct dirent* const de = readdir (dir);

                if (de == nullptr)
                    break;

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, de->d_name, FNM_CASEFOLD) == 0)
                {
                    filenameFound = CharPointer_UTF8 (de->d_name);

                    updateStatInfoForFile (parentDir + filenameFound, isDir, fileSize, modTime, creationTime, isReadOnly);

                    if (isHidden != 0)
                        *isHidden = filenameFound.startsWithChar ('.');

                    return true;
                }
            }
        }

        return false;
    }

private:
    String parentDir, wildCard;
    DIR* dir;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};


DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildCard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCard))
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
    const LocalRef<jstring> t (javaString (fileName));
    android.activity.callVoidMethod (JuceAppActivity.launchURL, t.get());
}

void File::revealToUser() const
{
}
