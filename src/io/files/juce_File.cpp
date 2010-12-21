/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../../core/juce_StandardHeader.h"

#if ! JUCE_WINDOWS
  #include <pwd.h>
#endif

BEGIN_JUCE_NAMESPACE

#include "juce_File.h"
#include "juce_FileInputStream.h"
#include "juce_DirectoryIterator.h"
#include "juce_TemporaryFile.h"
#include "../../core/juce_SystemStats.h"
#include "../../maths/juce_Random.h"
#include "../../core/juce_PlatformUtilities.h"
#include "../../memory/juce_ScopedPointer.h"


//==============================================================================
File::File (const String& fullPathName)
    : fullPath (parseAbsolutePath (fullPathName))
{
}

File::File (const String& path, int)
    : fullPath (path)
{
}

const File File::createFileWithoutCheckingPath (const String& path)
{
    return File (path, 0);
}

File::File (const File& other)
    : fullPath (other.fullPath)
{
}

File& File::operator= (const String& newPath)
{
    fullPath = parseAbsolutePath (newPath);
    return *this;
}

File& File::operator= (const File& other)
{
    fullPath = other.fullPath;
    return *this;
}

const File File::nonexistent;


//==============================================================================
const String File::parseAbsolutePath (const String& p)
{
    if (p.isEmpty())
        return String::empty;

#if JUCE_WINDOWS
    // Windows..
    String path (p.replaceCharacter ('/', '\\'));

    if (path.startsWithChar (File::separator))
    {
        if (path[1] != File::separator)
        {
            /*  When you supply a raw string to the File object constructor, it must be an absolute path.
                If you're trying to parse a string that may be either a relative path or an absolute path,
                you MUST provide a context against which the partial path can be evaluated - you can do
                this by simply using File::getChildFile() instead of the File constructor. E.g. saying
                "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
                path if that's what was supplied, or would evaluate a partial path relative to the CWD.
            */
            jassertfalse;

            path = File::getCurrentWorkingDirectory().getFullPathName().substring (0, 2) + path;
        }
    }
    else if (! path.containsChar (':'))
    {
        /*  When you supply a raw string to the File object constructor, it must be an absolute path.
            If you're trying to parse a string that may be either a relative path or an absolute path,
            you MUST provide a context against which the partial path can be evaluated - you can do
            this by simply using File::getChildFile() instead of the File constructor. E.g. saying
            "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
            path if that's what was supplied, or would evaluate a partial path relative to the CWD.
        */
        jassertfalse;

        return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
    }
#else
    // Mac or Linux..
    String path (p.replaceCharacter ('\\', '/'));

    if (path.startsWithChar ('~'))
    {
        if (path[1] == File::separator || path[1] == 0)
        {
            // expand a name of the form "~/abc"
            path = File::getSpecialLocation (File::userHomeDirectory).getFullPathName()
                    + path.substring (1);
        }
        else
        {
            // expand a name of type "~dave/abc"
            const String userName (path.substring (1).upToFirstOccurrenceOf ("/", false, false));

            struct passwd* const pw = getpwnam (userName.toUTF8());
            if (pw != 0)
                path = addTrailingSeparator (pw->pw_dir) + path.fromFirstOccurrenceOf ("/", false, false);
        }
    }
    else if (! path.startsWithChar (File::separator))
    {
        /*  When you supply a raw string to the File object constructor, it must be an absolute path.
            If you're trying to parse a string that may be either a relative path or an absolute path,
            you MUST provide a context against which the partial path can be evaluated - you can do
            this by simply using File::getChildFile() instead of the File constructor. E.g. saying
            "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
            path if that's what was supplied, or would evaluate a partial path relative to the CWD.
        */
        jassert (path.startsWith ("./") || path.startsWith ("../")); // (assume that a path "./xyz" is deliberately intended to be relative to the CWD)

        return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
    }
#endif

    while (path.endsWithChar (separator) && path != separatorString) // careful not to turn a single "/" into an empty string.
        path = path.dropLastCharacters (1);

    return path;
}

const String File::addTrailingSeparator (const String& path)
{
    return path.endsWithChar (File::separator) ? path
                                               : path + File::separator;
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

bool File::operator< (const File& other) const
{
#if NAMES_ARE_CASE_SENSITIVE
    return fullPath < other.fullPath;
#else
    return fullPath.compareIgnoreCase (other.fullPath) < 0;
#endif
}

bool File::operator> (const File& other) const
{
#if NAMES_ARE_CASE_SENSITIVE
    return fullPath > other.fullPath;
#else
    return fullPath.compareIgnoreCase (other.fullPath) > 0;
#endif
}

//==============================================================================
bool File::setReadOnly (const bool shouldBeReadOnly,
                        const bool applyRecursively) const
{
    bool worked = true;

    if (applyRecursively && isDirectory())
    {
        Array <File> subFiles;
        findChildFiles (subFiles, File::findFilesAndDirectories, false);

        for (int i = subFiles.size(); --i >= 0;)
            worked = subFiles.getReference(i).setReadOnly (shouldBeReadOnly, true) && worked;
    }

    return setFileReadOnlyInternal (shouldBeReadOnly) && worked;
}

bool File::deleteRecursively() const
{
    bool worked = true;

    if (isDirectory())
    {
        Array<File> subFiles;
        findChildFiles (subFiles, File::findFilesAndDirectories, false);

        for (int i = subFiles.size(); --i >= 0;)
            worked = subFiles.getReference(i).deleteRecursively() && worked;
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

    return moveInternal (newFile);
}

bool File::copyFileTo (const File& newFile) const
{
    return (*this == newFile)
            || (exists() && newFile.deleteFile() && copyInternal (newFile));
}

bool File::copyDirectoryTo (const File& newDirectory) const
{
    if (isDirectory() && newDirectory.createDirectory())
    {
        Array<File> subFiles;
        findChildFiles (subFiles, File::findFiles, false);

        int i;
        for (i = 0; i < subFiles.size(); ++i)
            if (! subFiles.getReference(i).copyFileTo (newDirectory.getChildFile (subFiles.getReference(i).getFileName())))
                return false;

        subFiles.clear();
        findChildFiles (subFiles, File::findDirectories, false);

        for (i = 0; i < subFiles.size(); ++i)
            if (! subFiles.getReference(i).copyDirectoryTo (newDirectory.getChildFile (subFiles.getReference(i).getFileName())))
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
    return File (getPathUpToLastSlash(), (int) 0);
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
    const int lastDot = fullPath.lastIndexOfChar ('.');

    if (lastDot > lastSlash)
        return fullPath.substring (lastSlash, lastDot);
    else
        return fullPath.substring (lastSlash);
}

bool File::isAChildOf (const File& potentialParent) const
{
    if (potentialParent == File::nonexistent)
        return false;

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
    return path.startsWithChar ('/') || path.startsWithChar ('\\')
#if JUCE_WINDOWS
            || (path.isNotEmpty() && path[1] == ':');
#else
            || path.startsWithChar ('~');
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

        if (relativePath[0] == '.')
        {
#if JUCE_WINDOWS
            relativePath = relativePath.replaceCharacter ('/', '\\').trimStart();
#else
            relativePath = relativePath.replaceCharacter ('\\', '/').trimStart();
#endif
            while (relativePath[0] == '.')
            {
                if (relativePath[1] == '.')
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

        return File (addTrailingSeparator (path) + relativePath);
    }
}

const File File::getSiblingFile (const String& fileName) const
{
    return getParentDirectory().getChildFile (fileName);
}

//==============================================================================
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
    if (exists())
        return true;

    {
        const File parentDir (getParentDirectory());

        if (parentDir == *this || ! parentDir.createDirectory())
            return false;

        FileOutputStream fo (*this, 8);
    }

    return exists();
}

bool File::createDirectory() const
{
    if (! isDirectory())
    {
        const File parentDir (getParentDirectory());

        if (parentDir == *this || ! parentDir.createDirectory())
            return false;

        createDirectoryInternal (fullPath.trimCharactersAtEnd (separatorString));
        return isDirectory();
    }

    return true;
}

//==============================================================================
const Time File::getCreationTime() const
{
    int64 m, a, c;
    getFileTimesInternal (m, a, c);
    return Time (c);
}

const Time File::getLastModificationTime() const
{
    int64 m, a, c;
    getFileTimesInternal (m, a, c);
    return Time (m);
}

const Time File::getLastAccessTime() const
{
    int64 m, a, c;
    getFileTimesInternal (m, a, c);
    return Time (a);
}

bool File::setLastModificationTime (const Time& t) const    { return setFileTimesInternal (t.toMilliseconds(), 0, 0); }
bool File::setLastAccessTime (const Time& t) const          { return setFileTimesInternal (0, t.toMilliseconds(), 0); }
bool File::setCreationTime (const Time& t) const            { return setFileTimesInternal (0, 0, t.toMilliseconds()); }

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
int File::findChildFiles (Array<File>& results,
                          const int whatToLookFor,
                          const bool searchRecursively,
                          const String& wildCardPattern) const
{
    DirectoryIterator di (*this, searchRecursively, wildCardPattern, whatToLookFor);

    int total = 0;
    while (di.next())
    {
        results.add (di.getFile());
        ++total;
    }

    return total;
}

int File::getNumberOfChildFiles (const int whatToLookFor, const String& wildCardPattern) const
{
    DirectoryIterator di (*this, false, wildCardPattern, whatToLookFor);

    int total = 0;
    while (di.next())
        ++total;

    return total;
}

bool File::containsSubDirectories() const
{
    if (isDirectory())
    {
        DirectoryIterator di (*this, false, "*", findDirectories);
        return di.next();
    }

    return false;
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
        if (prefix.trim().endsWithChar (')'))
        {
            putNumbersInBrackets = true;

            const int openBracks = prefix.lastIndexOfChar ('(');
            const int closeBracks = prefix.lastIndexOfChar (')');

            if (openBracks > 0
                 && closeBracks > openBracks
                 && prefix.substring (openBracks + 1, closeBracks).containsOnly ("0123456789"))
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
                f = getChildFile (prefix + '(' + String (num++) + ')' + suffix);
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
        const int indexOfDot = fullPath.lastIndexOfChar ('.');

        if (indexOfDot > fullPath.lastIndexOfChar (separator))
            ext = fullPath.substring (indexOfDot);
    }

    return ext;
}

bool File::hasFileExtension (const String& possibleSuffix) const
{
    if (possibleSuffix.isEmpty())
        return fullPath.lastIndexOfChar ('.') <= fullPath.lastIndexOfChar (separator);

    const int semicolon = possibleSuffix.indexOfChar (0, ';');

    if (semicolon >= 0)
    {
        return hasFileExtension (possibleSuffix.substring (0, semicolon).trimEnd())
                || hasFileExtension (possibleSuffix.substring (semicolon + 1).trimStart());
    }
    else
    {
        if (fullPath.endsWithIgnoreCase (possibleSuffix))
        {
            if (possibleSuffix.startsWithChar ('.'))
                return true;

            const int dotPos = fullPath.length() - possibleSuffix.length() - 1;

            if (dotPos >= 0)
                return fullPath [dotPos] == '.';
        }
    }

    return false;
}

const File File::withFileExtension (const String& newExtension) const
{
    if (fullPath.isEmpty())
        return File::nonexistent;

    String filePart (getFileName());

    int i = filePart.lastIndexOfChar ('.');
    if (i >= 0)
        filePart = filePart.substring (0, i);

    if (newExtension.isNotEmpty() && ! newExtension.startsWithChar ('.'))
        filePart << '.';

    return getSiblingFile (filePart + newExtension);
}

//==============================================================================
bool File::startAsProcess (const String& parameters) const
{
    return exists() && PlatformUtilities::openDocument (fullPath, parameters);
}

//==============================================================================
FileInputStream* File::createInputStream() const
{
    if (existsAsFile())
        return new FileInputStream (*this);

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

bool File::hasIdenticalContentTo (const File& other) const
{
    if (other == *this)
        return true;

    if (getSize() == other.getSize() && existsAsFile() && other.existsAsFile())
    {
        FileInputStream in1 (*this), in2 (other);

        const int bufferSize = 4096;
        HeapBlock <char> buffer1, buffer2;
        buffer1.malloc (bufferSize);
        buffer2.malloc (bufferSize);

        for (;;)
        {
            const int num1 = in1.read (buffer1, bufferSize);
            const int num2 = in2.read (buffer2, bufferSize);

            if (num1 != num2)
                break;

            if (num1 <= 0)
                return true;

            if (memcmp (buffer1, buffer2, num1) != 0)
                break;
        }
    }

    return false;
}

//==============================================================================
const String File::createLegalPathName (const String& original)
{
    String s (original);
    String start;

    if (s[1] == ':')
    {
        start = s.substring (0, 2);
        s = s.substring (2);
    }

    return start + s.removeCharacters ("\"#@,;:<>*^|?")
                    .substring (0, 1024);
}

const String File::createLegalFileName (const String& original)
{
    String s (original.removeCharacters ("\"#@,;:<>*^|?\\/"));

    const int maxLength = 128; // only the length of the filename, not the whole path
    const int len = s.length();

    if (len > maxLength)
    {
        const int lastDot = s.lastIndexOfChar ('.');

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

    String dirPath (addTrailingSeparator (dir.existsAsFile() ? dir.getParentDirectory().getFullPathName()
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
        thisPath = "..\\" + thisPath;
#else
        thisPath = "../" + thisPath;
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
const File File::createTempFile (const String& fileNameEnding)
{
    const File tempFile (getSpecialLocation (tempDirectory)
                            .getChildFile ("temp_" + String (Random::getSystemRandom().nextInt()))
                            .withFileExtension (fileNameEnding));

    if (tempFile.exists())
        return createTempFile (fileNameEnding);
    else
        return tempFile;
}

#if JUCE_UNIT_TESTS

#include "../../utilities/juce_UnitTest.h"
#include "../../maths/juce_Random.h"

class FileTests  : public UnitTest
{
public:
    FileTests() : UnitTest ("Files") {}

    void runTest()
    {
        beginTest ("Reading");

        const File home (File::getSpecialLocation (File::userHomeDirectory));
        const File temp (File::getSpecialLocation (File::tempDirectory));

        expect (! File::nonexistent.exists());
        expect (home.isDirectory());
        expect (home.exists());
        expect (! home.existsAsFile());
        expect (File::getSpecialLocation (File::userDocumentsDirectory).isDirectory());
        expect (File::getSpecialLocation (File::userApplicationDataDirectory).isDirectory());
        expect (File::getSpecialLocation (File::currentExecutableFile).exists());
        expect (File::getSpecialLocation (File::currentApplicationFile).exists());
        expect (File::getSpecialLocation (File::invokedExecutableFile).exists());
        expect (home.getVolumeTotalSize() > 1024 * 1024);
        expect (home.getBytesFreeOnVolume() > 0);
        expect (! home.isHidden());
        expect (home.isOnHardDisk());
        expect (! home.isOnCDRomDrive());
        expect (File::getCurrentWorkingDirectory().exists());
        expect (home.setAsCurrentWorkingDirectory());
        expect (File::getCurrentWorkingDirectory() == home);

        {
            Array<File> roots;
            File::findFileSystemRoots (roots);
            expect (roots.size() > 0);

            int numRootsExisting = 0;
            for (int i = 0; i < roots.size(); ++i)
                if (roots[i].exists())
                    ++numRootsExisting;

            // (on windows, some of the drives may not contain media, so as long as at least one is ok..)
            expect (numRootsExisting > 0);
        }

        beginTest ("Writing");

        File demoFolder (temp.getChildFile ("Juce UnitTests Temp Folder.folder"));
        expect (demoFolder.deleteRecursively());
        expect (demoFolder.createDirectory());
        expect (demoFolder.isDirectory());
        expect (demoFolder.getParentDirectory() == temp);
        expect (temp.isDirectory());

        {
            Array<File> files;
            temp.findChildFiles (files, File::findFilesAndDirectories, false, "*");
            expect (files.contains (demoFolder));
        }

        {
            Array<File> files;
            temp.findChildFiles (files, File::findDirectories, true, "*.folder");
            expect (files.contains (demoFolder));
        }

        File tempFile (demoFolder.getNonexistentChildFile ("test", ".txt", false));

        expect (tempFile.getFileExtension() == ".txt");
        expect (tempFile.hasFileExtension (".txt"));
        expect (tempFile.hasFileExtension ("txt"));
        expect (tempFile.withFileExtension ("xyz").hasFileExtension (".xyz"));
        expect (tempFile.getSiblingFile ("foo").isAChildOf (temp));
        expect (tempFile.hasWriteAccess());

        {
            FileOutputStream fo (tempFile);
            fo.write ("0123456789", 10);
        }

        expect (tempFile.exists());
        expect (tempFile.getSize() == 10);
        expect (std::abs ((int) (tempFile.getLastModificationTime().toMilliseconds() - Time::getCurrentTime().toMilliseconds())) < 3000);
        expect (tempFile.loadFileAsString() == "0123456789");
        expect (! demoFolder.containsSubDirectories());

        expect (demoFolder.getNumberOfChildFiles (File::findFiles) == 1);
        expect (demoFolder.getNumberOfChildFiles (File::findFilesAndDirectories) == 1);
        expect (demoFolder.getNumberOfChildFiles (File::findDirectories) == 0);
        demoFolder.getNonexistentChildFile ("tempFolder", "", false).createDirectory();
        expect (demoFolder.getNumberOfChildFiles (File::findDirectories) == 1);
        expect (demoFolder.getNumberOfChildFiles (File::findFilesAndDirectories) == 2);
        expect (demoFolder.containsSubDirectories());

        expect (tempFile.hasWriteAccess());
        tempFile.setReadOnly (true);
        expect (! tempFile.hasWriteAccess());
        tempFile.setReadOnly (false);
        expect (tempFile.hasWriteAccess());

        Time t (Time::getCurrentTime());
        tempFile.setLastModificationTime (t);
        Time t2 = tempFile.getLastModificationTime();
        expect (std::abs ((int) (t2.toMilliseconds() - t.toMilliseconds())) <= 1000);

        {
            MemoryBlock mb;
            tempFile.loadFileAsData (mb);
            expect (mb.getSize() == 10);
            expect (mb[0] == '0');
        }

        expect (tempFile.appendData ("abcdefghij", 10));
        expect (tempFile.getSize() == 20);
        expect (tempFile.replaceWithData ("abcdefghij", 10));
        expect (tempFile.getSize() == 10);

        File tempFile2 (tempFile.getNonexistentSibling (false));
        expect (tempFile.copyFileTo (tempFile2));
        expect (tempFile2.exists());
        expect (tempFile2.hasIdenticalContentTo (tempFile));
        expect (tempFile.deleteFile());
        expect (! tempFile.exists());
        expect (tempFile2.moveFileTo (tempFile));
        expect (tempFile.exists());
        expect (! tempFile2.exists());

        expect (demoFolder.deleteRecursively());
        expect (! demoFolder.exists());
    }
};

static FileTests fileUnitTests;

#endif

END_JUCE_NAMESPACE
