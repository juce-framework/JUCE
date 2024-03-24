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

namespace juce::build_tools
{

    static const char* resourceFileIdentifierString = "JUCER_BINARY_RESOURCE";

    //==============================================================================
    void ResourceFile::setClassName (const String& name)
    {
        className = name;
    }

    void ResourceFile::addFile (const File& file)
    {
        files.add (file);

        auto variableNameRoot = makeBinaryDataIdentifierName (file);
        auto variableName = variableNameRoot;

        int suffix = 2;

        while (variableNames.contains (variableName))
            variableName = variableNameRoot + String (suffix++);

        variableNames.add (variableName);
    }

    String ResourceFile::getDataVariableFor (const File& file) const
    {
        const auto index = files.indexOf (file);
        jassert (index >= 0);
        return variableNames[index];
    }

    String ResourceFile::getSizeVariableFor (const File& file) const
    {
        return getDataVariableFor (file) + "Size";
    }

    int64 ResourceFile::getTotalDataSize() const
    {
        return std::accumulate (files.begin(),
                                files.end(),
                                int64 { 0 },
                                [] (int64 acc, const File& f) { return acc + f.getSize(); });
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
            auto& file = files.getReference (i);

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
        cpp << "#include <cstring>" << newLine
            << newLine
            << "namespace " << className << newLine
            << "{" << newLine;

        while (i < files.size())
        {
            auto& file = files.getReference (i);
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
                    writeDataAsCppLiteral (data, cpp, true, true);
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
                << "const char* getNamedResource (const char* resourceNameUTF8, int& numBytes);" << newLine
                << "const char* getNamedResource (const char* resourceNameUTF8, int& numBytes)"  << newLine
                << "{" << newLine;

            StringArray returnCodes;
            for (auto& file : files)
            {
                auto dataSize = file.getSize();
                returnCodes.add ("numBytes = " + String (dataSize) + "; return " + variableNames[files.indexOf (file)] + ";");
            }

            createStringMatcher (cpp, "resourceNameUTF8", variableNames, returnCodes, 4);

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

            cpp << "const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);"                        << newLine
                << "const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8)"                         << newLine
                << "{"                                                                                                   << newLine
                << "    for (unsigned int i = 0; i < (sizeof (namedResourceList) / sizeof (namedResourceList[0])); ++i)" << newLine
                << "        if (strcmp (namedResourceList[i], resourceNameUTF8) == 0)"                                   << newLine
                << "            return originalFilenames[i];"                                                            << newLine
                <<                                                                                                          newLine
                << "    return nullptr;"                                                                                 << newLine
                << "}"                                                                                                   << newLine
                <<                                                                                                          newLine;
        }

        cpp << "}" << newLine;

        return Result::ok();
    }

    ResourceFile::WriteResult ResourceFile::write (int maxFileSize,
                                                   String projectLineFeed,
                                                   File headerFile,
                                                   std::function<File (int)> getCppFile)
    {
        Array<File> filesCreated;

        {
            MemoryOutputStream mo;
            mo.setNewLineString (projectLineFeed);

            auto r = writeHeader (mo);

            if (r.failed())
                return { r, {} };

            if (! overwriteFileWithNewDataIfDifferent (headerFile, mo))
                return { Result::fail ("Can't write to file: " + headerFile.getFullPathName()), {} };

            filesCreated.add (headerFile);
        }

        int i = 0;
        int fileIndex = 0;

        for (;;)
        {
            auto cpp = getCppFile (fileIndex);

            MemoryOutputStream mo;
            mo.setNewLineString (projectLineFeed);

            auto r = writeCpp (mo, headerFile, i, maxFileSize);

            if (r.failed())
                return { r, std::move (filesCreated) };

            if (! overwriteFileWithNewDataIfDifferent (cpp, mo))
                return { Result::fail ("Can't write to file: " + cpp.getFullPathName()), std::move (filesCreated) };

            filesCreated.add (cpp);
            ++fileIndex;

            if (i >= files.size())
                break;
        }

        return { Result::ok(), std::move (filesCreated) };
    }

} // namespace juce::build_tools
