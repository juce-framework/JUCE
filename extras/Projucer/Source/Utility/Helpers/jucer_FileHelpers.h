/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
namespace FileHelpers
{
    uint64 calculateStreamHashCode (InputStream&);
    uint64 calculateFileHashCode (const File&);

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
const char* const sourceFileExtensions          = "cpp;mm;m;c;cc;cxx;swift;s;asm;r";
const char* const headerFileExtensions          = "h;hpp;hxx;hh;inl";
const char* const cOrCppFileExtensions          = "cpp;cc;cxx;c";
const char* const cppFileExtensions             = "cpp;cc;cxx";
const char* const objCFileExtensions            = "mm;m";
const char* const asmFileExtensions             = "s;S;asm";
const char* const sourceOrHeaderFileExtensions  = "cpp;mm;m;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl";
const char* const browseableFileExtensions      = "cpp;mm;m;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl;txt;md;rtf";
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
    uint64 fileHashCode = 0;
    int64 fileSize = -1;
};
