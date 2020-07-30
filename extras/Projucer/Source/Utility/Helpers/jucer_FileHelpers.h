/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    bool containsAnyNonHiddenFiles (const File& folder);

    bool shouldPathsBeRelative (String path1, String path2);

    // removes "/../" bits from the middle of the path
    String simplifyPath (String::CharPointerType path);
    String simplifyPath (const String& path);
}

//==============================================================================
const char* const sourceFileExtensions          = "cpp;mm;m;metal;c;cc;cxx;swift;s;asm;r";
const char* const headerFileExtensions          = "h;hpp;hxx;hh;inl";
const char* const cOrCppFileExtensions          = "cpp;cc;cxx;c";
const char* const cppFileExtensions             = "cpp;cc;cxx";
const char* const objCFileExtensions            = "mm;m";
const char* const asmFileExtensions             = "s;S;asm";
const char* const sourceOrHeaderFileExtensions  = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl";
const char* const browseableFileExtensions      = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;h;hpp;hxx;hh;inl;txt;md;rtf";
const char* const fileTypesToCompileByDefault   = "cpp;mm;m;metal;c;cc;cxx;swift;s;S;asm;r";

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
                      || build_tools::calculateFileHashCode (file) != fileHashCode);
    }

    void updateHash()
    {
        fileModificationTime = file.getLastModificationTime();
        fileSize = file.getSize();
        fileHashCode = build_tools::calculateFileHashCode (file);
    }

private:
    File file;
    Time fileModificationTime;
    uint64 fileHashCode = 0;
    int64 fileSize = -1;
};
