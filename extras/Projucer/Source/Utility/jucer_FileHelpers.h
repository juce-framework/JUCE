/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_FILEHELPERS_H_INCLUDED
#define JUCER_FILEHELPERS_H_INCLUDED


//==============================================================================
namespace FileHelpers
{
    int64 calculateStreamHashCode (InputStream& stream);
    int64 calculateFileHashCode (const File& file);

    bool overwriteFileWithNewDataIfDifferent (const File& file, const void* data, size_t numBytes);
    bool overwriteFileWithNewDataIfDifferent (const File& file, const MemoryOutputStream& newData);
    bool overwriteFileWithNewDataIfDifferent (const File& file, const String& newData);

    bool containsAnyNonHiddenFiles (const File& folder);

    String unixStylePath (const String& path);
    String windowsStylePath (const String& path);
    String currentOSStylePath (const String& path);

    bool shouldPathsBeRelative (String path1, String path2);
    bool isAbsolutePath (const String& path);

    // A windows-aware version of File::getRelativePath()
    String getRelativePathFrom (const File& file, const File& sourceFolder);

    // removes "/../" bits from the middle of the path
    String simplifyPath (String::CharPointerType path);
    String simplifyPath (const String& path);
}

//==============================================================================
const char* const sourceFileExtensions          = "cpp;mm;m;c;cc;cxx;swift;s;asm";
const char* const headerFileExtensions          = "h;hpp;hxx;hh;inl";
const char* const cOrCppFileExtensions          = "cpp;cc;cxx;c";
const char* const cppFileExtensions             = "cpp;cc;cxx";
const char* const objCFileExtensions            = "mm;m";
const char* const asmFileExtensions             = "s;S;asm";
const char* const sourceOrHeaderFileExtensions  = "cpp;mm;m;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl";
const char* const browseableFileExtensions      = "cpp;mm;m;c;cc;cxx;swift;s;S;asm;r;h;hpp;hxx;hh;inl;txt;md;rtf";
const char* const fileTypesToCompileByDefault   = "cpp;mm;c;m;cc;cxx;swift;s;S;asm;r";

//==============================================================================
struct FileModificationDetector
{
    FileModificationDetector (const File& f)  : file (f) {}

    const File& getFile() const                     { return file; }
    void fileHasBeenRenamed (const File& newFile)   { file = newFile; }

    bool hasBeenModified() const
    {
        return fileModificationTime != file.getLastModificationTime()
                 && (fileSize != file.getSize()
                      || FileHelpers::calculateFileHashCode (file) != fileHashCode);
    }

    void updateHash()
    {
        fileModificationTime = file.getLastModificationTime();
        fileSize = file.getSize();
        fileHashCode = FileHelpers::calculateFileHashCode (file);
    }

private:
    File file;
    Time fileModificationTime;
    int64 fileHashCode, fileSize;
};


#endif   // JUCER_FILEHELPERS_H_INCLUDED
