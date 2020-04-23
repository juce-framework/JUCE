/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace build_tools
{
    class ResourceFile
    {
    public:
        ResourceFile() = default;

        void setClassName (const String& className);

        String getClassName() const { return className; }

        void addFile (const File& file);

        String getDataVariableFor (const File& file) const;

        String getSizeVariableFor (const File& file) const;

        int getNumFiles() const { return files.size (); }

        const File& getFile (int index) const { return files.getReference (index); }

        int64 getTotalDataSize() const;

        struct WriteResult
        {
            Result result;
            Array<File> filesCreated;
        };

        WriteResult write (int maxFileSize,
                           String projectLineFeed,
                           File headerFile,
                           std::function<File (int)> getCppFile);

    private:
        Array<File> files;
        StringArray variableNames;
        String className { "BinaryData" };

        Result writeHeader (MemoryOutputStream&);

        Result writeCpp (MemoryOutputStream&, const File&, int&, int);

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResourceFile)
    };
}
}
