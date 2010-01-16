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

#ifdef _MSC_VER
  #pragma warning (disable: 4514)
  #pragma warning (push)
#endif

#include "../../core/juce_StandardHeader.h"

#if ! JUCE_WINDOWS
  #include <pwd.h>
#endif

BEGIN_JUCE_NAMESPACE


#include "juce_File.h"
#include "juce_FileInputStream.h"
#include "juce_TemporaryFile.h"
#include "../../core/juce_SystemStats.h"
#include "../../core/juce_Random.h"

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

//==============================================================================
void* juce_fileOpen (const String& path, bool forWriting);
void juce_fileClose (void* handle);
int juce_fileWrite (void* handle, const void* buffer, int size);
int64 juce_fileGetPosition (void* handle);
int64 juce_fileSetPosition (void* handle, int64 pos);
void juce_fileFlush (void* handle);

bool juce_fileExists (const String& fileName, const bool dontCountDirectories);
bool juce_isDirectory (const String& fileName);
int64 juce_getFileSize (const String& fileName);
bool juce_canWriteToFile (const String& fileName);
bool juce_setFileReadOnly (const String& fileName, bool isReadOnly);

void juce_getFileTimes (const String& fileName, int64& modificationTime, int64& accessTime, int64& creationTime);
bool juce_setFileTimes (const String& fileName, int64 modificationTime, int64 accessTime, int64 creationTime);

bool juce_deleteFile (const String& fileName);
bool juce_copyFile (const String& source, const String& dest);
bool juce_moveFile (const String& source, const String& dest);

// this must also create all paths involved in the directory.
void juce_createDirectory (const String& fileName);

bool juce_launchFile (const String& fileName, const String& parameters);

const StringArray juce_getFileSystemRoots();
const String juce_getVolumeLabel (const String& filenameOnVolume, int& volumeSerialNumber);

// starts a directory search operation with a wildcard, returning a handle for
// use in calls to juce_findFileNext.
// juce_firstResultFile gets the name of the file (not the whole pathname) and
// the other pointers, if non-null, are set based on the properties of the file.
void* juce_findFileStart (const String& directory, const String& wildCard, String& firstResultFile,
                          bool* isDirectory, bool* isHidden, int64* fileSize, Time* modTime,
                          Time* creationTime, bool* isReadOnly);

// returns false when no more files are found
bool juce_findFileNext (void* handle, String& resultFile,
                        bool* isDirectory, bool* isHidden, int64* fileSize,
                        Time* modTime, Time* creationTime, bool* isReadOnly);

void juce_findFileClose (void* handle);

//==============================================================================
static const String juce_addTrailingSeparator (const String& path)
{
    return path.endsWithChar (File::separator) ? path
                                               : path + File::separator;
}

//==============================================================================
static const String parseAbsolutePath (String path)
{
    if (path.isEmpty())
        return String::empty;

#if JUCE_WINDOWS
    // Windows..
    path = path.replaceCharacter (T('/'), T('\\'));

    if (path.startsWithChar (File::separator))
    {
        if (path[1] != File::separator)
        {
            jassertfalse // using a filename that starts with a slash is a bit dodgy on
                         // Windows, because it needs a drive letter, which in this case
                         // we'll take from the CWD.. but this is a bit of an assumption that
                         // could be wrong..

            path = File::getCurrentWorkingDirectory().getFullPathName().substring (0, 2) + path;
        }
    }
    else if (path.indexOfChar (T(':')) < 0)
    {
        if (path.isEmpty())
            return String::empty;

        jassertfalse // using a partial filename is a bad way to initialise a file, because
                     // we don't know what directory to put it in.
                     // Here we'll assume it's in the CWD, but this might not be what was
                     // intended..

        return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
    }
#else
    // Mac or Linux..
    path = path.replaceCharacter (T('\\'), T('/'));

    if (path.startsWithChar (T('~')))
    {
        const char* homeDir = 0;

        if (path[1] == File::separator || path[1] == 0)
        {
            // expand a name of the form "~/abc"
            path = File::getSpecialLocation (File::userHomeDirectory).getFullPathName()
                    + path.substring (1);
        }
        else
        {
            // expand a name of type "~dave/abc"
            const String userName (path.substring (1)
                                       .upToFirstOccurrenceOf (T("/"), false, false));

            struct passwd* const pw = getpwnam (userName);
            if (pw != 0)
            {
                String home (homeDir);

                if (home.endsWithChar (File::separator))
                    home [home.length() - 1] = 0;

                path = String (pw->pw_dir)
                      + path.substring (userName.length());
            }
        }
    }
    else if (! path.startsWithChar (File::separator))
    {
        while (path.startsWith (T("./")))
            path = path.substring (2);

        if (path.isEmpty())
            return String::empty;

        jassertfalse // using a partial filename is a bad way to initialise a file, because
                     // we don't know what directory to put it in.
                     // Here we'll assume it's in the CWD, but this might not be what was
                     // intended..

        return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
    }
#endif

    int len = path.length();
    while (--len > 0 && path [len] == File::separator)
        path [len] = 0;

    return path;
}


//==============================================================================
const File File::nonexistent;


//==============================================================================
File::File (const String& fullPathName)
    : fullPath (parseAbsolutePath (fullPathName))
{
}

File::File (const String& path, int)
    : fullPath (path)
{
}

File::File (const File& other)
    : fullPath (other.fullPath)
{
}

const File& File::operator= (const String& newPath)
{
    fullPath = parseAbsolutePath (newPath);
    return *this;
}

const File& File::operator= (const File& other)
{
    fullPath = other.fullPath;
    return *this;
}

//==============================================================================
#if JUCE_LINUX
  #define NAMES_ARE_CASE_SENSITIVE 1
#endif

bool File::areFileNamesCaseSensitive()
{
#if NAMES_ARE_CASE_SENSITIVE
    return true;
#else
    return false;
#endif
}

bool File::operator== (const File& other) const
{
    // case-insensitive on Windows, but not on linux.
#if NAMES_ARE_CASE_SENSITIVE
    return fullPath == other.fullPath;
#else
    return fullPath.equalsIgnoreCase (other.fullPath);
#endif
}

bool File::operator!= (const File& other) const
{
    return ! operator== (other);
}

//==============================================================================
bool File::exists() const
{
    return juce_fileExists (fullPath, false);
}

bool File::existsAsFile() const
{
    return juce_fileExists (fullPath, true);
}

bool File::isDirectory() const
{
    return juce_isDirectory (fullPath);
}

bool File::hasWriteAccess() const
{
    if (exists())
        return juce_canWriteToFile (fullPath);

#if ! JUCE_WINDOWS
    else if ((! isDirectory()) && fullPath.containsChar (separator))
        return getParentDirectory().hasWriteAccess();
    else
        return false;
#else
    // on windows, it seems that even read-only directories can still be written into,
    // so checking the parent directory's permissions would return the wrong result..
    else
        return true;
#endif
}

bool File::setReadOnly (const bool shouldBeReadOnly,
                        const bool applyRecursively) const
{
    bool worked = true;

    if (applyRecursively && isDirectory())
    {
        OwnedArray <File> subFiles;
        findChildFiles (subFiles, File::findFilesAndDirectories, false);

        for (int i = subFiles.size(); --i >= 0;)
            worked = subFiles[i]->setReadOnly (shouldBeReadOnly, true) && worked;
    }

    return juce_setFileReadOnly (fullPath, shouldBeReadOnly) && worked;
}

bool File::deleteFile() const
{
    return (! exists())
            || juce_deleteFile (fullPath);
}

bool File::deleteRecursively() const
{
    bool worked = true;

    if (isDirectory())
    {
        OwnedArray<File> subFiles;
        findChildFiles (subFiles, File::findFilesAndDirectories, false);

        for (int i = subFiles.size(); --i >= 0;)
            worked = subFiles[i]->deleteRecursively() && worked;
    }

    return deleteFile() && worked;
}

bool File::moveFileTo (const File& newFile) const
{
    if (newFile.fullPath == fullPath)
        return true;

#if ! NAMES_ARE_CASE_SENSITIVE
    if (*this != newFile)
#endif
        if (! newFile.deleteFile())
            return false;

    return juce_moveFile (fullPath, newFile.fullPath);
}

bool File::copyFileTo (const File& newFile) const
{
    if (*this == newFile)
        return true;

    if (! newFile.deleteFile())
        return false;

    return juce_copyFile (fullPath, newFile.fullPath);
}

bool File::copyDirectoryTo (const File& newDirectory) const
{
    if (isDirectory() && newDirectory.createDirectory())
    {
        OwnedArray<File> subFiles;
        findChildFiles (subFiles, File::findFiles, false);

        int i;
        for (i = 0; i < subFiles.size(); ++i)
            if (! subFiles[i]->copyFileTo (newDirectory.getChildFile (subFiles[i]->getFileName())))
                return false;

        subFiles.clear();
        findChildFiles (subFiles, File::findDirectories, false);

        for (i = 0; i < subFiles.size(); ++i)
            if (! subFiles[i]->copyDirectoryTo (newDirectory.getChildFile (subFiles[i]->getFileName())))
                return false;

        return true;
    }

    return false;
}

//==============================================================================
const String File::getPathUpToLastSlash() const
{
    const int lastSlash = fullPath.lastIndexOfChar (separator);

    if (lastSlash > 0)
        return fullPath.substring (0, lastSlash);
    else if (lastSlash == 0)
        return separatorString;
    else
        return fullPath;
}

const File File::getParentDirectory() const
{
    return File (getPathUpToLastSlash());
}

//==============================================================================
const String File::getFileName() const
{
    return fullPath.substring (fullPath.lastIndexOfChar (separator) + 1);
}

int File::hashCode() const
{
    return fullPath.hashCode();
}

int64 File::hashCode64() const
{
    return fullPath.hashCode64();
}

const String File::getFileNameWithoutExtension() const
{
    const int lastSlash = fullPath.lastIndexOfChar (separator) + 1;
    const int lastDot = fullPath.lastIndexOfChar (T('.'));

    if (lastDot > lastSlash)
        return fullPath.substring (lastSlash, lastDot);
    else
        return fullPath.substring (lastSlash);
}

bool File::isAChildOf (const File& potentialParent) const
{
    const String ourPath (getPathUpToLastSlash());

#if NAMES_ARE_CASE_SENSITIVE
    if (potentialParent.fullPath == ourPath)
#else
    if (potentialParent.fullPath.equalsIgnoreCase (ourPath))
#endif
    {
        return true;
    }
    else if (potentialParent.fullPath.length() >= ourPath.length())
    {
        return false;
    }
    else
    {
        return getParentDirectory().isAChildOf (potentialParent);
    }
}

//==============================================================================
bool File::isAbsolutePath (const String& path)
{
    return path.startsWithChar (T('/')) || path.startsWithChar (T('\\'))
#if JUCE_WINDOWS
            || (path.isNotEmpty() && ((const String&) path)[1] == T(':'));
#else
            || path.startsWithChar (T('~'));
#endif
}

const File File::getChildFile (String relativePath) const
{
    if (isAbsolutePath (relativePath))
    {
        // the path is really absolute..
        return File (relativePath);
    }
    else
    {
        // it's relative, so remove any ../ or ./ bits at the start.
        String path (fullPath);

        if (relativePath[0] == T('.'))
        {
#if JUCE_WINDOWS
            relativePath = relativePath.replaceCharacter (T('/'), T('\\')).trimStart();
#else
            relativePath = relativePath.replaceCharacter (T('\\'), T('/')).trimStart();
#endif
            while (relativePath[0] == T('.'))
            {
                if (relativePath[1] == T('.'))
                {
                    if (relativePath [2] == 0 || relativePath[2] == separator)
                    {
                        const int lastSlash = path.lastIndexOfChar (separator);
                        if (lastSlash >= 0)
                            path = path.substring (0, lastSlash);

                        relativePath = relativePath.substring (3);
                    }
                    else
                    {
                        break;
                    }
                }
                else if (relativePath[1] == separator)
                {
                    relativePath = relativePath.substring (2);
                }
                else
                {
                    break;
                }
            }
        }

        return File (juce_addTrailingSeparator (path) + relativePath);
    }
}

const File File::getSiblingFile (const String& fileName) const
{
    return getParentDirectory().getChildFile (fileName);
}

//==============================================================================
int64 File::getSize() const
{
    return juce_getFileSize (fullPath);
}

const String File::descriptionOfSizeInBytes (const int64 bytes)
{
    if (bytes == 1)
    {
        return "1 byte";
    }
    else if (bytes < 1024)
    {
        return String ((int) bytes) + " bytes";
    }
    else if (bytes < 1024 * 1024)
    {
        return String (bytes / 1024.0, 1) + " KB";
    }
    else if (bytes < 1024 * 1024 * 1024)
    {
        return String (bytes / (1024.0 * 1024.0), 1) + " MB";
    }
    else
    {
        return String (bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }
}

//==============================================================================
bool File::create() const
{
    if (! exists())
    {
        const File parentDir (getParentDirectory());

        if (parentDir == *this || ! parentDir.createDirectory())
            return false;

        void* const fh = juce_fileOpen (fullPath, true);

        if (fh == 0)
            return false;

        juce_fileClose (fh);
    }

    return true;
}

bool File::createDirectory() const
{
    if (! isDirectory())
    {
        const File parentDir (getParentDirectory());

        if (parentDir == *this || ! parentDir.createDirectory())
            return false;

        String dir (fullPath);

        while (dir.endsWithChar (separator))
            dir [dir.length() - 1] = 0;

        juce_createDirectory (dir);

        return isDirectory();
    }

    return true;
}

//==============================================================================
const Time File::getCreationTime() const
{
    int64 m, a, c;
    juce_getFileTimes (fullPath, m, a, c);
    return Time (c);
}

bool File::setCreationTime (const Time& t) const
{
    return juce_setFileTimes (fullPath, 0, 0, t.toMilliseconds());
}

const Time File::getLastModificationTime() const
{
    int64 m, a, c;
    juce_getFileTimes (fullPath, m, a, c);
    return Time (m);
}

bool File::setLastModificationTime (const Time& t) const
{
    return juce_setFileTimes (fullPath, t.toMilliseconds(), 0, 0);
}

const Time File::getLastAccessTime() const
{
    int64 m, a, c;
    juce_getFileTimes (fullPath, m, a, c);
    return Time (a);
}

bool File::setLastAccessTime (const Time& t) const
{
    return juce_setFileTimes (fullPath, 0, t.toMilliseconds(), 0);
}

//==============================================================================
bool File::loadFileAsData (MemoryBlock& destBlock) const
{
    if (! existsAsFile())
        return false;

    FileInputStream in (*this);
    return getSize() == in.readIntoMemoryBlock (destBlock);
}

const String File::loadFileAsString() const
{
    if (! existsAsFile())
        return String::empty;

    FileInputStream in (*this);
    return in.readEntireStreamAsString();
}

//==============================================================================
static inline bool fileTypeMatches (const int whatToLookFor,
                                    const bool isDir,
                                    const bool isHidden)
{
    return (whatToLookFor & (isDir ? File::findDirectories
                                   : File::findFiles)) != 0
             && ((! isHidden)
                  || (whatToLookFor & File::ignoreHiddenFiles) == 0);
}

int File::findChildFiles (OwnedArray<File>& results,
                          const int whatToLookFor,
                          const bool searchRecursively,
                          const String& wildCardPattern) const
{
    // you have to specify the type of files you're looking for!
    jassert ((whatToLookFor & (findFiles | findDirectories)) != 0);

    int total = 0;

    // find child files or directories in this directory first..
    if (isDirectory())
    {
        const String path (juce_addTrailingSeparator (fullPath));

        String filename;
        bool itemIsDirectory, itemIsHidden;

        void* const handle = juce_findFileStart (path, wildCardPattern, filename,
                                                 &itemIsDirectory, &itemIsHidden,
                                                 0, 0, 0, 0);

        if (handle != 0)
        {
            do
            {
                if (fileTypeMatches (whatToLookFor, itemIsDirectory, itemIsHidden)
                     && ! filename.containsOnly (T(".")))
                {
                    results.add (new File (path + filename, 0));
                    ++total;
                }

            } while (juce_findFileNext (handle, filename, &itemIsDirectory, &itemIsHidden, 0, 0, 0, 0));

            juce_findFileClose (handle);
        }
    }
    else
    {
        // trying to search for files inside a non-directory?
        //jassertfalse
    }

    // and recurse down if required.
    if (searchRecursively)
    {
        OwnedArray <File> subDirectories;
        findChildFiles (subDirectories, File::findDirectories, false);

        for (int i = 0; i < subDirectories.size(); ++i)
        {
            total += subDirectories.getUnchecked(i)
                        ->findChildFiles (results,
                                          whatToLookFor,
                                          true,
                                          wildCardPattern);
        }
    }

    return total;
}

int File::getNumberOfChildFiles (const int whatToLookFor,
                                 const String& wildCardPattern) const
{
    // you have to specify the type of files you're looking for!
    jassert (whatToLookFor > 0 && whatToLookFor <= 3);

    int count = 0;

    if (isDirectory())
    {
        String filename;
        bool itemIsDirectory, itemIsHidden;

        void* const handle = juce_findFileStart (fullPath, wildCardPattern, filename,
                                                 &itemIsDirectory, &itemIsHidden,
                                                 0, 0, 0, 0);

        if (handle != 0)
        {
            do
            {
                if (fileTypeMatches (whatToLookFor, itemIsDirectory, itemIsHidden)
                     && ! filename.containsOnly (T(".")))
                {
                    ++count;
                }

            } while (juce_findFileNext (handle, filename, &itemIsDirectory, &itemIsHidden, 0, 0, 0, 0));

            juce_findFileClose (handle);
        }
    }
    else
    {
        // trying to search for files inside a non-directory?
        jassertfalse
    }

    return count;
}

bool File::containsSubDirectories() const
{
    bool result = false;

    if (isDirectory())
    {
        String filename;
        bool itemIsDirectory, itemIsHidden;
        void* const handle = juce_findFileStart (juce_addTrailingSeparator (fullPath),
                                                 T("*"), filename,
                                                 &itemIsDirectory, &itemIsHidden, 0, 0, 0, 0);

        if (handle != 0)
        {
            do
            {
                if (itemIsDirectory)
                {
                    result = true;
                    break;
                }

            } while (juce_findFileNext (handle, filename, &itemIsDirectory, &itemIsHidden, 0, 0, 0, 0));

            juce_findFileClose (handle);
        }
    }

    return result;
}

//==============================================================================
const File File::getNonexistentChildFile (const String& prefix_,
                                          const String& suffix,
                                          bool putNumbersInBrackets) const
{
    File f (getChildFile (prefix_ + suffix));

    if (f.exists())
    {
        int num = 2;
        String prefix (prefix_);

        // remove any bracketed numbers that may already be on the end..
        if (prefix.trim().endsWithChar (T(')')))
        {
            putNumbersInBrackets = true;

            const int openBracks = prefix.lastIndexOfChar (T('('));
            const int closeBracks = prefix.lastIndexOfChar (T(')'));

            if (openBracks > 0
                 && closeBracks > openBracks
                 && prefix.substring (openBracks + 1, closeBracks).containsOnly (T("0123456789")))
            {
                num = prefix.substring (openBracks + 1, closeBracks).getIntValue() + 1;
                prefix = prefix.substring (0, openBracks);
            }
        }

        // also use brackets if it ends in a digit.
        putNumbersInBrackets = putNumbersInBrackets
                                || CharacterFunctions::isDigit (prefix.getLastCharacter());

        do
        {
            if (putNumbersInBrackets)
                f = getChildFile (prefix + T('(') + String (num++) + T(')') + suffix);
            else
                f = getChildFile (prefix + String (num++) + suffix);

        } while (f.exists());
    }

    return f;
}

const File File::getNonexistentSibling (const bool putNumbersInBrackets) const
{
    if (exists())
    {
        return getParentDirectory()
                .getNonexistentChildFile (getFileNameWithoutExtension(),
                                          getFileExtension(),
                                          putNumbersInBrackets);
    }
    else
    {
        return *this;
    }
}

//==============================================================================
const String File::getFileExtension() const
{
    String ext;

    if (! isDirectory())
    {
        const int indexOfDot = fullPath.lastIndexOfChar (T('.'));

        if (indexOfDot > fullPath.lastIndexOfChar (separator))
            ext = fullPath.substring (indexOfDot);
    }

    return ext;
}

bool File::hasFileExtension (const String& possibleSuffix) const
{
    if (possibleSuffix.isEmpty())
        return fullPath.lastIndexOfChar (T('.')) <= fullPath.lastIndexOfChar (separator);

    const int semicolon = possibleSuffix.indexOfChar (0, T(';'));

    if (semicolon >= 0)
    {
        return hasFileExtension (possibleSuffix.substring (0, semicolon).trimEnd())
                || hasFileExtension (possibleSuffix.substring (semicolon + 1).trimStart());
    }
    else
    {
        if (fullPath.endsWithIgnoreCase (possibleSuffix))
        {
            if (possibleSuffix.startsWithChar (T('.')))
                return true;

            const int dotPos = fullPath.length() - possibleSuffix.length() - 1;

            if (dotPos >= 0)
                return fullPath [dotPos] == T('.');
        }
    }

    return false;
}

const File File::withFileExtension (const String& newExtension) const
{
    if (fullPath.isEmpty())
        return File::nonexistent;

    String filePart (getFileName());

    int i = filePart.lastIndexOfChar (T('.'));
    if (i < 0)
        i = filePart.length();

    String newExt (newExtension);

    if (newExt.isNotEmpty() && ! newExt.startsWithChar (T('.')))
        newExt = T(".") + newExt;

    return getSiblingFile (filePart.substring (0, i) + newExt);
}

//==============================================================================
bool File::startAsProcess (const String& parameters) const
{
    return exists()
            && juce_launchFile (fullPath, parameters);
}

//==============================================================================
FileInputStream* File::createInputStream() const
{
    if (existsAsFile())
        return new FileInputStream (*this);
    else
        return 0;
}

FileOutputStream* File::createOutputStream (const int bufferSize) const
{
    ScopedPointer <FileOutputStream> out (new FileOutputStream (*this, bufferSize));

    if (out->failedToOpen())
        return 0;

    return out.release();
}

//==============================================================================
bool File::appendData (const void* const dataToAppend,
                       const int numberOfBytes) const
{
    if (numberOfBytes > 0)
    {
        const ScopedPointer <FileOutputStream> out (createOutputStream());

        if (out == 0)
            return false;

        out->write (dataToAppend, numberOfBytes);
    }

    return true;
}

bool File::replaceWithData (const void* const dataToWrite,
                            const int numberOfBytes) const
{
    jassert (numberOfBytes >= 0); // a negative number of bytes??

    if (numberOfBytes <= 0)
        return deleteFile();

    TemporaryFile tempFile (*this, TemporaryFile::useHiddenFile);
    tempFile.getFile().appendData (dataToWrite, numberOfBytes);
    return tempFile.overwriteTargetFileWithTemporary();
}

bool File::appendText (const String& text,
                       const bool asUnicode,
                       const bool writeUnicodeHeaderBytes) const
{
    const ScopedPointer <FileOutputStream> out (createOutputStream());

    if (out != 0)
    {
        out->writeText (text, asUnicode, writeUnicodeHeaderBytes);
        return true;
    }

    return false;
}

bool File::replaceWithText (const String& textToWrite,
                            const bool asUnicode,
                            const bool writeUnicodeHeaderBytes) const
{
    TemporaryFile tempFile (*this, TemporaryFile::useHiddenFile);
    tempFile.getFile().appendText (textToWrite, asUnicode, writeUnicodeHeaderBytes);
    return tempFile.overwriteTargetFileWithTemporary();
}

//==============================================================================
const String File::createLegalPathName (const String& original)
{
    String s (original);
    String start;

    if (s[1] == T(':'))
    {
        start = s.substring (0, 2);
        s = s.substring (2);
    }

    return start + s.removeCharacters (T("\"#@,;:<>*^|?"))
                    .substring (0, 1024);
}

const String File::createLegalFileName (const String& original)
{
    String s (original.removeCharacters (T("\"#@,;:<>*^|?\\/")));

    const int maxLength = 128; // only the length of the filename, not the whole path
    const int len = s.length();

    if (len > maxLength)
    {
        const int lastDot = s.lastIndexOfChar (T('.'));

        if (lastDot > jmax (0, len - 12))
        {
            s = s.substring (0, maxLength - (len - lastDot))
                 + s.substring (lastDot);
        }
        else
        {
            s = s.substring (0, maxLength);
        }
    }

    return s;
}

//==============================================================================
const String File::getRelativePathFrom (const File& dir)  const
{
    String thisPath (fullPath);

    {
        int len = thisPath.length();
        while (--len >= 0 && thisPath [len] == File::separator)
            thisPath [len] = 0;
    }

    String dirPath (juce_addTrailingSeparator ((dir.existsAsFile()) ? dir.getParentDirectory().getFullPathName()
                                                                    : dir.fullPath));

    const int len = jmin (thisPath.length(), dirPath.length());
    int commonBitLength = 0;

    for (int i = 0; i < len; ++i)
    {
#if NAMES_ARE_CASE_SENSITIVE
        if (thisPath[i] != dirPath[i])
#else
        if (CharacterFunctions::toLowerCase (thisPath[i])
             != CharacterFunctions::toLowerCase (dirPath[i]))
#endif
        {
            break;
        }

        ++commonBitLength;
    }

    while (commonBitLength > 0 && thisPath [commonBitLength - 1] != File::separator)
        --commonBitLength;

    // if the only common bit is the root, then just return the full path..
    if (commonBitLength <= 0
         || (commonBitLength == 1 && thisPath [1] == File::separator))
        return fullPath;

    thisPath = thisPath.substring (commonBitLength);
    dirPath  = dirPath.substring (commonBitLength);

    while (dirPath.isNotEmpty())
    {
#if JUCE_WINDOWS
        thisPath = T("..\\") + thisPath;
#else
        thisPath = T("../") + thisPath;
#endif

        const int sep = dirPath.indexOfChar (separator);

        if (sep >= 0)
            dirPath = dirPath.substring (sep + 1);
        else
            dirPath = String::empty;
    }

    return thisPath;
}

//==============================================================================
void File::findFileSystemRoots (OwnedArray<File>& destArray)
{
    const StringArray roots (juce_getFileSystemRoots());

    for (int i = 0; i < roots.size(); ++i)
        destArray.add (new File (roots[i]));
}

const String File::getVolumeLabel() const
{
    int serialNum;
    return juce_getVolumeLabel (fullPath, serialNum);
}

int File::getVolumeSerialNumber() const
{
    int serialNum;
    juce_getVolumeLabel (fullPath, serialNum);

    return serialNum;
}

//==============================================================================
const File File::createTempFile (const String& fileNameEnding)
{
    const File tempFile (getSpecialLocation (tempDirectory)
                            .getChildFile (T("temp_") + String (Random::getSystemRandom().nextInt()))
                            .withFileExtension (fileNameEnding));

    if (tempFile.exists())
        return createTempFile (fileNameEnding);
    else
        return tempFile;
}


END_JUCE_NAMESPACE
