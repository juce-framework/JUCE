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

#include "../Project/jucer_Project.h"


//==============================================================================
class ResourceFile
{
public:
    //==============================================================================
    ResourceFile (Project& project);
    ~ResourceFile();

    //==============================================================================
    void setClassName (const String& className);
    String getClassName() const       { return className; }

    void addFile (const File& file);
    String getDataVariableFor (const File& file) const;
    String getSizeVariableFor (const File& file) const;

    int getNumFiles() const                 { return files.size(); }
    const File& getFile (int index) const   { return files.getReference (index); }

    int64 getTotalDataSize() const;

    Result write (Array<File>& filesCreated, int maxFileSize);

    //==============================================================================
private:
    Array<File> files;
    StringArray variableNames;
    Project& project;
    String className;

    Result writeHeader (MemoryOutputStream&);
    Result writeCpp (MemoryOutputStream&, const File& headerFile, int& index, int maxFileSize);
    void addResourcesFromProjectItem (const Project::Item& node);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResourceFile)
};
