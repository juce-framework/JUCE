/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/

File::File (const String& fullPathName)
    : fullPath (parseAbsolutePath (fullPathName))
{
}

File File::createFileWithoutCheckingPath (const String& path) noexcept
{
    File f;
    f.fullPath = path;
    return f;
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

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
File::File (File&& other) noexcept
    : fullPath (static_cast <String&&> (other.fullPath))
{
}

File& File::operator= (File&& other) noexcept
{
    fullPath = static_cast <String&&> (other.fullPath);
    return *this;
}
#endif

const File File::nonexistent;


//==============================================================================
String File::parseAbsolutePath (const String& p)
{
    if (p.isEmpty())
        return String();

#if JUCE_WINDOWS
    // Windows..
    String path (p.replaceCharacter ('/', '\\'));

    if (path.startsWithChar (separator))
    {
        if (path[1] != separator)
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

    // Yes, I know it's legal for a unix pathname to contain a backslash, but this assertion is here
    // to catch anyone who's trying to run code that was written on Windows with hard-coded path names.
    // If that's why you've ended up here, use File::getChildFile() to build your paths instead.
    jassert ((! p.containsChar ('\\')) || (p.indexOfChar ('/') >= 0 && p.indexOfChar ('/') < p.indexOfChar ('\\')));

    String path (p);

    if (path.startsWithChar ('~'))
    {
        if (path[1] == separator || path[1] == 0)
        {
            // expand a name of the form "~/abc"
            path = File::getSpecialLocation (File::userHomeDirectory).getFullPathName()
                    + path.substring (1);
        }
        else
        {
            // expand a name of type "~dave/abc"
            const String userName (path.substring (1).upToFirstOccurrenceOf ("/", false, false));

            if (struct passwd* const pw = getpwnam (userName.toUTF8()))
                path = addTrailingSeparator (pw->pw_dir) + path.fromFirstOccurrenceOf ("/", false, false);
        }
    }
    else if (! path.startsWithChar (separator))
    {
       #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
        if (! (path.startsWith ("./") || path.startsWith ("../")))
        {
            /*  When you supply a raw string to the File object constructor, it must be an absolute path.
                If you're trying to parse a string that may be either a relative path or an absolute path,
                you MUST provide a context against which the partial path can be evaluated - you can do
                this by simply using File::getChildFile() instead of the File constructor. E.g. saying
                "File::getCurrentWorkingDirectory().getChildFile (myUnknownPath)" would return an absolute
                path if that's what was supplied, or would evaluate a partial path relative to the CWD.
            */
            jassertfalse;

           #if JUCE_LOG_ASSERTIONS
            Logger::writeToLog ("Illegal absolute path: " + path);
           #endif
        }
       #endif

        return File::getCurrentWorkingDirectory().getChildFile (path).getFullPathName();
    }
#endif

    while (path.endsWithChar (separator) && path != separatorString) // careful not to turn a single "/" into an empty string.
        path = path.dropLastCharacters (1);

    return path;
}

String File::addTrailingSeparator (const String& path)
{
    return path.endsWithChar (separator) ? path
                                         : path + separator;
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

static int compareFilenames (const String& name1, const String& name2) noexcept
{
   #if NAMES_ARE_CASE_SENSITIVE
    return name1.compare (name2);
   #else
    return name1.compareIgnoreCase (name2);
   #endif
}

bool File::operator== (const File& other) const     { return compareFilenames (fullPath, other.fullPath) == 0; }
bool File::operator!= (const File& other) const     { return compareFilenames (fullPath, other.fullPath) != 0; }
bool File::operator<  (const File& other) const     { return compareFilenames (fullPath, other.fullPath) <  0; }
bool File::operator>  (const File& other) const     { return compareFilenames (fullPath, other.fullPath) >  0; }

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

bool File::setExecutePermission (bool shouldBeExecutable) const
{
    return setFileExecutableInternal (shouldBeExecutable);
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

    if (! exists())
        return false;

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

        for (int i = 0; i < subFiles.size(); ++i)
            if (! subFiles.getReference(i).copyFileTo (newDirectory.getChildFile (subFiles.getReference(i).getFileName())))
                return false;

        subFiles.clear();
        findChildFiles (subFiles, File::findDirectories, false);

        for (int i = 0; i < subFiles.size(); ++i)
            if (! subFiles.getReference(i).copyDirectoryTo (newDirectory.getChildFile (subFiles.getReference(i).getFileName())))
                return false;

        return true;
    }

    return false;
}

//==============================================================================
String File::getPathUpToLastSlash() const
{
    const int lastSlash = fullPath.lastIndexOfChar (separator);

    if (lastSlash > 0)
        return fullPath.substring (0, lastSlash);

    if (lastSlash == 0)
        return separatorString;

    return fullPath;
}

File File::getParentDirectory() const
{
    File f;
    f.fullPath = getPathUpToLastSlash();
    return f;
}

//==============================================================================
String File::getFileName() const
{
    return fullPath.substring (fullPath.lastIndexOfChar (separator) + 1);
}

String File::getFileNameWithoutExtension() const
{
    const int lastSlash = fullPath.lastIndexOfChar (separator) + 1;
    const int lastDot   = fullPath.lastIndexOfChar ('.');

    if (lastDot > lastSlash)
        return fullPath.substring (lastSlash, lastDot);

    return fullPath.substring (lastSlash);
}

bool File::isAChildOf (const File& potentialParent) const
{
    if (potentialParent.fullPath.isEmpty())
        return false;

    const String ourPath (getPathUpToLastSlash());

    if (compareFilenames (potentialParent.fullPath, ourPath) == 0)
        return true;

    if (potentialParent.fullPath.length() >= ourPath.length())
        return false;

    return getParentDirectory().isAChildOf (potentialParent);
}

int   File::hashCode() const    { return fullPath.hashCode(); }
int64 File::hashCode64() const  { return fullPath.hashCode64(); }

//==============================================================================
bool File::isAbsolutePath (StringRef path)
{
    return path.text[0] == separator
           #if JUCE_WINDOWS
            || (path.isNotEmpty() && path.text[1] == ':');
           #else
            || path.text[0] == '~';
           #endif
}

File File::getChildFile (StringRef relativePath) const
{
    if (isAbsolutePath (relativePath))
        return File (String (relativePath.text));

    if (relativePath[0] != '.')
        return File (addTrailingSeparator (fullPath) + relativePath);

    String path (fullPath);

    // It's relative, so remove any ../ or ./ bits at the start..
   #if JUCE_WINDOWS
    if (relativePath.text.indexOf ((juce_wchar) '/') >= 0)
        return getChildFile (String (relativePath.text).replaceCharacter ('/', '\\'));
   #endif

    while (relativePath[0] == '.')
    {
        const juce_wchar secondChar = relativePath[1];

        if (secondChar == '.')
        {
            const juce_wchar thirdChar = relativePath[2];

            if (thirdChar == 0 || thirdChar == separator)
            {
                const int lastSlash = path.lastIndexOfChar (separator);
                if (lastSlash >= 0)
                    path = path.substring (0, lastSlash);

                relativePath = relativePath.text + (thirdChar == 0 ? 2 : 3);
            }
            else
            {
                break;
            }
        }
        else if (secondChar == separator)
        {
            relativePath = relativePath.text + 2;
        }
        else
        {
            break;
        }
    }

    return File (addTrailingSeparator (path) + relativePath);
}

File File::getSiblingFile (StringRef fileName) const
{
    return getParentDirectory().getChildFile (fileName);
}

//==============================================================================
String File::descriptionOfSizeInBytes (const int64 bytes)
{
    const char* suffix;
    double divisor = 0;

    if (bytes == 1)                       { suffix = " byte"; }
    else if (bytes < 1024)                { suffix = " bytes"; }
    else if (bytes < 1024 * 1024)         { suffix = " KB"; divisor = 1024.0; }
    else if (bytes < 1024 * 1024 * 1024)  { suffix = " MB"; divisor = 1024.0 * 1024.0; }
    else                                  { suffix = " GB"; divisor = 1024.0 * 1024.0 * 1024.0; }

    return (divisor > 0 ? String (bytes / divisor, 1) : String (bytes)) + suffix;
}

//==============================================================================
Result File::create() const
{
    if (exists())
        return Result::ok();

    const File parentDir (getParentDirectory());

    if (parentDir == *this)
        return Result::fail ("Cannot create parent directory");

    Result r (parentDir.createDirectory());

    if (r.wasOk())
    {
        FileOutputStream fo (*this, 8);
        r = fo.getStatus();
    }

    return r;
}

Result File::createDirectory() const
{
    if (isDirectory())
        return Result::ok();

    const File parentDir (getParentDirectory());

    if (parentDir == *this)
        return Result::fail ("Cannot create parent directory");

    Result r (parentDir.createDirectory());

    if (r.wasOk())
        r = createDirectoryInternal (fullPath.trimCharactersAtEnd (separatorString));

    return r;
}

//==============================================================================
Time File::getLastModificationTime() const           { int64 m, a, c; getFileTimesInternal (m, a, c); return Time (m); }
Time File::getLastAccessTime() const                 { int64 m, a, c; getFileTimesInternal (m, a, c); return Time (a); }
Time File::getCreationTime() const                   { int64 m, a, c; getFileTimesInternal (m, a, c); return Time (c); }

bool File::setLastModificationTime (Time t) const    { return setFileTimesInternal (t.toMilliseconds(), 0, 0); }
bool File::setLastAccessTime (Time t) const          { return setFileTimesInternal (0, t.toMilliseconds(), 0); }
bool File::setCreationTime (Time t) const            { return setFileTimesInternal (0, 0, t.toMilliseconds()); }

//==============================================================================
bool File::loadFileAsData (MemoryBlock& destBlock) const
{
    if (! existsAsFile())
        return false;

    FileInputStream in (*this);
    return in.openedOk() && getSize() == (int64) in.readIntoMemoryBlock (destBlock);
}

String File::loadFileAsString() const
{
    if (! existsAsFile())
        return String();

    FileInputStream in (*this);
    return in.openedOk() ? in.readEntireStreamAsString()
                         : String();
}

void File::readLines (StringArray& destLines) const
{
    destLines.addLines (loadFileAsString());
}

//==============================================================================
int File::findChildFiles (Array<File>& results,
                          const int whatToLookFor,
                          const bool searchRecursively,
                          const String& wildCardPattern) const
{
    int total = 0;

    for (DirectoryIterator di (*this, searchRecursively, wildCardPattern, whatToLookFor); di.next();)
    {
        results.add (di.getFile());
        ++total;
    }

    return total;
}

int File::getNumberOfChildFiles (const int whatToLookFor, const String& wildCardPattern) const
{
    int total = 0;

    for (DirectoryIterator di (*this, false, wildCardPattern, whatToLookFor); di.next();)
        ++total;

    return total;
}

bool File::containsSubDirectories() const
{
    if (! isDirectory())
        return false;

    DirectoryIterator di (*this, false, "*", findDirectories);
    return di.next();
}

//==============================================================================
File File::getNonexistentChildFile (const String& suggestedPrefix,
                                    const String& suffix,
                                    bool putNumbersInBrackets) const
{
    File f (getChildFile (suggestedPrefix + suffix));

    if (f.exists())
    {
        int number = 1;
        String prefix (suggestedPrefix);

        // remove any bracketed numbers that may already be on the end..
        if (prefix.trim().endsWithChar (')'))
        {
            putNumbersInBrackets = true;

            const int openBracks  = prefix.lastIndexOfChar ('(');
            const int closeBracks = prefix.lastIndexOfChar (')');

            if (openBracks > 0
                 && closeBracks > openBracks
                 && prefix.substring (openBracks + 1, closeBracks).containsOnly ("0123456789"))
            {
                number = prefix.substring (openBracks + 1, closeBracks).getIntValue();
                prefix = prefix.substring (0, openBracks);
            }
        }

        // also use brackets if it ends in a digit.
        putNumbersInBrackets = putNumbersInBrackets
                                 || CharacterFunctions::isDigit (prefix.getLastCharacter());

        do
        {
            String newName (prefix);

            if (putNumbersInBrackets)
                newName << '(' << ++number << ')';
            else
                newName << ++number;

            f = getChildFile (newName + suffix);

        } while (f.exists());
    }

    return f;
}

File File::getNonexistentSibling (const bool putNumbersInBrackets) const
{
    if (! exists())
        return *this;

    return getParentDirectory().getNonexistentChildFile (getFileNameWithoutExtension(),
                                                         getFileExtension(),
                                                         putNumbersInBrackets);
}

//==============================================================================
String File::getFileExtension() const
{
    const int indexOfDot = fullPath.lastIndexOfChar ('.');

    if (indexOfDot > fullPath.lastIndexOfChar (separator))
        return fullPath.substring (indexOfDot);

    return String();
}

bool File::hasFileExtension (StringRef possibleSuffix) const
{
    if (possibleSuffix.isEmpty())
        return fullPath.lastIndexOfChar ('.') <= fullPath.lastIndexOfChar (separator);

    const int semicolon = possibleSuffix.text.indexOf ((juce_wchar) ';');

    if (semicolon >= 0)
        return hasFileExtension (String (possibleSuffix.text).substring (0, semicolon).trimEnd())
                || hasFileExtension ((possibleSuffix.text + (semicolon + 1)).findEndOfWhitespace());

    if (fullPath.endsWithIgnoreCase (possibleSuffix))
    {
        if (possibleSuffix.text[0] == '.')
            return true;

        const int dotPos = fullPath.length() - possibleSuffix.length() - 1;

        if (dotPos >= 0)
            return fullPath [dotPos] == '.';
    }

    return false;
}

File File::withFileExtension (StringRef newExtension) const
{
    if (fullPath.isEmpty())
        return File();

    String filePart (getFileName());

    const int i = filePart.lastIndexOfChar ('.');
    if (i >= 0)
        filePart = filePart.substring (0, i);

    if (newExtension.isNotEmpty() && newExtension.text[0] != '.')
        filePart << '.';

    return getSiblingFile (filePart + newExtension);
}

//==============================================================================
bool File::startAsProcess (const String& parameters) const
{
    return exists() && Process::openDocument (fullPath, parameters);
}

//==============================================================================
FileInputStream* File::createInputStream() const
{
    ScopedPointer<FileInputStream> fin (new FileInputStream (*this));

    if (fin->openedOk())
        return fin.release();

    return nullptr;
}

FileOutputStream* File::createOutputStream (const size_t bufferSize) const
{
    ScopedPointer<FileOutputStream> out (new FileOutputStream (*this, bufferSize));

    return out->failedToOpen() ? nullptr
                               : out.release();
}

//==============================================================================
bool File::appendData (const void* const dataToAppend,
                       const size_t numberOfBytes) const
{
    jassert (((ssize_t) numberOfBytes) >= 0);

    if (numberOfBytes == 0)
        return true;

    FileOutputStream out (*this, 8192);
    return out.openedOk() && out.write (dataToAppend, numberOfBytes);
}

bool File::replaceWithData (const void* const dataToWrite,
                            const size_t numberOfBytes) const
{
    if (numberOfBytes == 0)
        return deleteFile();

    TemporaryFile tempFile (*this, TemporaryFile::useHiddenFile);
    tempFile.getFile().appendData (dataToWrite, numberOfBytes);
    return tempFile.overwriteTargetFileWithTemporary();
}

bool File::appendText (const String& text,
                       const bool asUnicode,
                       const bool writeUnicodeHeaderBytes) const
{
    FileOutputStream out (*this);

    if (out.failedToOpen())
        return false;

    out.writeText (text, asUnicode, writeUnicodeHeaderBytes);
    return true;
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

        if (in1.openedOk() && in2.openedOk())
        {
            const int bufferSize = 4096;
            HeapBlock<char> buffer1 (bufferSize), buffer2 (bufferSize);

            for (;;)
            {
                const int num1 = in1.read (buffer1, bufferSize);
                const int num2 = in2.read (buffer2, bufferSize);

                if (num1 != num2)
                    break;

                if (num1 <= 0)
                    return true;

                if (memcmp (buffer1, buffer2, (size_t) num1) != 0)
                    break;
            }
        }
    }

    return false;
}

//==============================================================================
String File::createLegalPathName (const String& original)
{
    String s (original);
    String start;

    if (s.isNotEmpty() && s[1] == ':')
    {
        start = s.substring (0, 2);
        s = s.substring (2);
    }

    return start + s.removeCharacters ("\"#@,;:<>*^|?")
                    .substring (0, 1024);
}

String File::createLegalFileName (const String& original)
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
static int countNumberOfSeparators (String::CharPointerType s)
{
    int num = 0;

    for (;;)
    {
        const juce_wchar c = s.getAndAdvance();

        if (c == 0)
            break;

        if (c == File::separator)
            ++num;
    }

    return num;
}

String File::getRelativePathFrom (const File& dir)  const
{
    String thisPath (fullPath);

    while (thisPath.endsWithChar (separator))
        thisPath = thisPath.dropLastCharacters (1);

    String dirPath (addTrailingSeparator (dir.existsAsFile() ? dir.getParentDirectory().getFullPathName()
                                                             : dir.fullPath));

    int commonBitLength = 0;
    String::CharPointerType thisPathAfterCommon (thisPath.getCharPointer());
    String::CharPointerType dirPathAfterCommon  (dirPath.getCharPointer());

    {
        String::CharPointerType thisPathIter (thisPath.getCharPointer());
        String::CharPointerType dirPathIter  (dirPath.getCharPointer());

        for (int i = 0;;)
        {
            const juce_wchar c1 = thisPathIter.getAndAdvance();
            const juce_wchar c2 = dirPathIter.getAndAdvance();

           #if NAMES_ARE_CASE_SENSITIVE
            if (c1 != c2
           #else
            if ((c1 != c2 && CharacterFunctions::toLowerCase (c1) != CharacterFunctions::toLowerCase (c2))
           #endif
                 || c1 == 0)
                break;

            ++i;

            if (c1 == separator)
            {
                thisPathAfterCommon = thisPathIter;
                dirPathAfterCommon  = dirPathIter;
                commonBitLength = i;
            }
        }
    }

    // if the only common bit is the root, then just return the full path..
    if (commonBitLength == 0 || (commonBitLength == 1 && thisPath[1] == separator))
        return fullPath;

    const int numUpDirectoriesNeeded = countNumberOfSeparators (dirPathAfterCommon);

    if (numUpDirectoriesNeeded == 0)
        return thisPathAfterCommon;

   #if JUCE_WINDOWS
    String s (String::repeatedString ("..\\", numUpDirectoriesNeeded));
   #else
    String s (String::repeatedString ("../",  numUpDirectoriesNeeded));
   #endif
    s.appendCharPointer (thisPathAfterCommon);
    return s;
}

//==============================================================================
File File::createTempFile (StringRef fileNameEnding)
{
    const File tempFile (getSpecialLocation (tempDirectory)
                            .getChildFile ("temp_" + String::toHexString (Random::getSystemRandom().nextInt()))
                            .withFileExtension (fileNameEnding));

    if (tempFile.exists())
        return createTempFile (fileNameEnding);

    return tempFile;
}

//==============================================================================
MemoryMappedFile::MemoryMappedFile (const File& file, MemoryMappedFile::AccessMode mode)
    : address (nullptr), range (0, file.getSize()), fileHandle (0)
{
    openInternal (file, mode);
}

MemoryMappedFile::MemoryMappedFile (const File& file, const Range<int64>& fileRange, AccessMode mode)
    : address (nullptr), range (fileRange.getIntersectionWith (Range<int64> (0, file.getSize()))), fileHandle (0)
{
    openInternal (file, mode);
}


//==============================================================================
#if JUCE_UNIT_TESTS

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
        expect (! File::nonexistent.existsAsFile());
        expect (! File::nonexistent.isDirectory());
       #if ! JUCE_WINDOWS
        expect (File("/").isDirectory());
       #endif
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
        expect (tempFile.withFileExtension ("xyz").hasFileExtension ("abc;xyz;foo"));
        expect (tempFile.withFileExtension ("xyz").hasFileExtension ("xyz;foo"));
        expect (! tempFile.withFileExtension ("h").hasFileExtension ("bar;foo;xx"));
        expect (tempFile.getSiblingFile ("foo").isAChildOf (temp));
        expect (tempFile.hasWriteAccess());

        {
            FileOutputStream fo (tempFile);
            fo.write ("0123456789", 10);
        }

        expect (tempFile.exists());
        expect (tempFile.getSize() == 10);
        expect (std::abs ((int) (tempFile.getLastModificationTime().toMilliseconds() - Time::getCurrentTime().toMilliseconds())) < 3000);
        expectEquals (tempFile.loadFileAsString(), String ("0123456789"));
        expect (! demoFolder.containsSubDirectories());

        expectEquals (tempFile.getRelativePathFrom (demoFolder.getParentDirectory()), demoFolder.getFileName() + File::separatorString + tempFile.getFileName());
        expectEquals (demoFolder.getParentDirectory().getRelativePathFrom (tempFile), ".." + File::separatorString + ".." + File::separatorString + demoFolder.getParentDirectory().getFileName());

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

        {
            expect (tempFile.getSize() == 10);
            FileOutputStream fo (tempFile);
            expect (fo.openedOk());

            expect (fo.setPosition  (7));
            expect (fo.truncate().wasOk());
            expect (tempFile.getSize() == 7);
            fo.write ("789", 3);
            fo.flush();
            expect (tempFile.getSize() == 10);
        }

        beginTest ("Memory-mapped files");

        {
            MemoryMappedFile mmf (tempFile, MemoryMappedFile::readOnly);
            expect (mmf.getSize() == 10);
            expect (mmf.getData() != nullptr);
            expect (memcmp (mmf.getData(), "0123456789", 10) == 0);
        }

        {
            const File tempFile2 (tempFile.getNonexistentSibling (false));
            expect (tempFile2.create());
            expect (tempFile2.appendData ("xxxxxxxxxx", 10));

            {
                MemoryMappedFile mmf (tempFile2, MemoryMappedFile::readWrite);
                expect (mmf.getSize() == 10);
                expect (mmf.getData() != nullptr);
                memcpy (mmf.getData(), "abcdefghij", 10);
            }

            {
                MemoryMappedFile mmf (tempFile2, MemoryMappedFile::readWrite);
                expect (mmf.getSize() == 10);
                expect (mmf.getData() != nullptr);
                expect (memcmp (mmf.getData(), "abcdefghij", 10) == 0);
            }

            expect (tempFile2.deleteFile());
        }

        beginTest ("More writing");

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
