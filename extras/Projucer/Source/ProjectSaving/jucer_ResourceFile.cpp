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

#include "../Application/jucer_Headers.h"
#include "jucer_ResourceFile.h"

static const char* resourceFileIdentifierString = "JUCER_BINARY_RESOURCE";

//==============================================================================
ResourceFile::ResourceFile (Project& p)    : project (p)
{
    addResourcesFromProjectItem (project.getMainGroup());
}

ResourceFile::~ResourceFile()
{
}

//==============================================================================
void ResourceFile::addResourcesFromProjectItem (const Project::Item& projectItem)
{
    if (projectItem.isGroup())
    {
        for (int i = 0; i < projectItem.getNumChildren(); ++i)
            addResourcesFromProjectItem (projectItem.getChild(i));
    }
    else
    {
        if (projectItem.shouldBeAddedToBinaryResources())
            addFile (projectItem.getFile());
    }
}

//==============================================================================
void ResourceFile::setClassName (const String& name)
{
    className = name;
}

void ResourceFile::addFile (const File& file)
{
    files.add (file);

    auto variableNameRoot = CodeHelpers::makeBinaryDataIdentifierName (file);
    auto variableName = variableNameRoot;

    int suffix = 2;
    while (variableNames.contains (variableName))
        variableName = variableNameRoot + String (suffix++);

    variableNames.add (variableName);
}

String ResourceFile::getDataVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)];
}

String ResourceFile::getSizeVariableFor (const File& file) const
{
    jassert (files.indexOf (file) >= 0);
    return variableNames [files.indexOf (file)] + "Size";
}

int64 ResourceFile::getTotalDataSize() const
{
    int64 total = 0;

    for (int i = 0; i < files.size(); ++i)
        total += files.getReference(i).getSize();

    return total;
}

static void writeComment (MemoryOutputStream& mo)
{
    mo << newLine << newLine
       << "   This is an auto-generated file: Any edits you make may be overwritten!" << newLine
       << newLine
       << "*/" << newLine
       << newLine;
}

Result ResourceFile::writeHeader (MemoryOutputStream& header)
{
    header << "/* =========================================================================================";
    writeComment (header);
    header << "#pragma once" << newLine
           << newLine
           << "namespace " << className << newLine
           << "{" << newLine;

    for (int i = 0; i < files.size(); ++i)
    {
        auto& file = files.getReference(i);

        if (! file.existsAsFile())
            return Result::fail ("Can't open resource file: " + file.getFullPathName());

        auto dataSize = file.getSize();

        auto variableName = variableNames[i];

        FileInputStream fileStream (file);

        if (fileStream.openedOk())
        {
            header << "    extern const char*   " << variableName << ";" << newLine;
            header << "    const int            " << variableName << "Size = " << (int) dataSize << ";" << newLine << newLine;
        }
    }

    header << "    // Number of elements in the namedResourceList and originalFileNames arrays."                             << newLine
           << "    const int namedResourceListSize = " << files.size() <<  ";"                                               << newLine
           << newLine
           << "    // Points to the start of a list of resource names."                                                      << newLine
           << "    extern const char* namedResourceList[];"                                                                  << newLine
           << newLine
           << "    // Points to the start of a list of resource filenames."                                                  << newLine
           << "    extern const char* originalFilenames[];"                                                                  << newLine
           << newLine
           << "    // If you provide the name of one of the binary resource variables above, this function will"             << newLine
           << "    // return the corresponding data and its size (or a null pointer if the name isn't found)."               << newLine
           << "    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);"                       << newLine
           << newLine
           << "    // If you provide the name of one of the binary resource variables above, this function will"             << newLine
           << "    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found)."  << newLine
           << "    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);"                             << newLine
           << "}" << newLine;

    return Result::ok();
}

Result ResourceFile::writeCpp (MemoryOutputStream& cpp, const File& headerFile, int& i, const int maxFileSize)
{
    bool isFirstFile = (i == 0);

    cpp << "/* ==================================== " << resourceFileIdentifierString << " ====================================";
    writeComment (cpp);
    cpp << "namespace " << className << newLine
        << "{" << newLine;

    while (i < files.size())
    {
        auto& file = files.getReference(i);
        auto variableName = variableNames[i];

        FileInputStream fileStream (file);

        if (fileStream.openedOk())
        {
            auto tempVariable = "temp_binary_data_" + String (i);

            cpp  << newLine << "//================== " << file.getFileName() << " ==================" << newLine
                 << "static const unsigned char " << tempVariable << "[] =" << newLine;

            {
                MemoryBlock data;
                fileStream.readIntoMemoryBlock (data);
                CodeHelpers::writeDataAsCppLiteral (data, cpp, true, true);
            }

            cpp << newLine << newLine
                << "const char* " << variableName << " = (const char*) " << tempVariable << ";" << newLine;
        }

        ++i;

        if (cpp.getPosition() > maxFileSize)
            break;
    }

    if (isFirstFile)
    {
        if (i < files.size())
        {
            cpp << newLine
                << "}" << newLine
                << newLine
                << "#include \"" << headerFile.getFileName() << "\"" << newLine
                << newLine
                << "namespace " << className << newLine
                << "{";
        }

        cpp << newLine
            << newLine
            << "const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)" << newLine
            << "{" << newLine;

        StringArray returnCodes;
        for (auto& file : files)
        {
            auto dataSize = file.getSize();
            returnCodes.add ("numBytes = " + String (dataSize) + "; return " + variableNames[files.indexOf (file)] + ";");
        }

        CodeHelpers::createStringMatcher (cpp, "resourceNameUTF8", variableNames, returnCodes, 4);

        cpp << "    numBytes = 0;" << newLine
            << "    return nullptr;" << newLine
            << "}" << newLine
            << newLine;

        cpp << "const char* namedResourceList[] =" << newLine
            << "{" << newLine;

        for (int j = 0; j < files.size(); ++j)
            cpp << "    " << variableNames[j].quoted() << (j < files.size() - 1 ? "," : "") << newLine;

        cpp << "};" << newLine << newLine;

        cpp << "const char* originalFilenames[] =" << newLine
            << "{" << newLine;

        for (auto& f : files)
            cpp << "    " << f.getFileName().quoted() << (files.indexOf (f) < files.size() - 1 ? "," : "") << newLine;

        cpp << "};" << newLine << newLine;

        cpp << "const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)"                         << newLine
            << "{"                                                                                                   << newLine
            << "    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)" << newLine
            << "    {"                                                                                               << newLine
            << "        if (namedResourceList[i] == resourceNameUTF8)"                                               << newLine
            << "            return originalFilenames[i];"                                                            << newLine
            << "    }"                                                                                               << newLine
            <<                                                                                                          newLine
            << "    return nullptr;"                                                                                 << newLine
            << "}"                                                                                                   << newLine
            <<                                                                                                          newLine;
    }

    cpp << "}" << newLine;

    return Result::ok();
}

Result ResourceFile::write (Array<File>& filesCreated, const int maxFileSize)
{
    auto projectLineFeed = project.getProjectLineFeed();

    auto headerFile = project.getBinaryDataHeaderFile();

    {
        MemoryOutputStream mo;
        mo.setNewLineString (projectLineFeed);

        auto r = writeHeader (mo);

        if (r.failed())
            return r;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (headerFile, mo))
            return Result::fail ("Can't write to file: " + headerFile.getFullPathName());

        filesCreated.add (headerFile);
    }

    int i = 0;
    int fileIndex = 0;

    for (;;)
    {
        auto cpp = project.getBinaryDataCppFile (fileIndex);

        MemoryOutputStream mo;
        mo.setNewLineString (projectLineFeed);

        auto r = writeCpp (mo, headerFile, i, maxFileSize);

        if (r.failed())
            return r;

        if (! FileHelpers::overwriteFileWithNewDataIfDifferent (cpp, mo))
            return Result::fail ("Can't write to file: " + cpp.getFullPathName());

        filesCreated.add (cpp);
        ++fileIndex;

        if (i >= files.size())
            break;
    }

    return Result::ok();
}
