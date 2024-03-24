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
