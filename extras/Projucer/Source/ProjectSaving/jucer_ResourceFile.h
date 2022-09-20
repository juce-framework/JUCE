/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
class JucerResourceFile
{
public:
    //==============================================================================
    explicit JucerResourceFile (Project& project);
    ~JucerResourceFile();

    //==============================================================================
    void setClassName (const String& className)         { resourceFile.setClassName (className); }
    String getClassName() const                         { return resourceFile.getClassName(); }

    void addFile (const File& file)                     { resourceFile.addFile (file); }
    String getDataVariableFor (const File& file) const  { return resourceFile.getDataVariableFor (file); }
    String getSizeVariableFor (const File& file) const  { return resourceFile.getSizeVariableFor (file); }

    int getNumFiles() const                             { return resourceFile.getNumFiles(); }
    const File& getFile (int index) const               { return resourceFile.getFile (index); }

    int64 getTotalDataSize() const                      { return resourceFile.getTotalDataSize(); }

    build_tools::ResourceFile::WriteResult write (int maxFileSize)
    {
        return resourceFile.write (maxFileSize,
                                   project.getProjectLineFeed(),
                                   project.getBinaryDataHeaderFile(),
                                   [this] (int index) { return project.getBinaryDataCppFile (index); });
    }

    //==============================================================================
private:
    void addResourcesFromProjectItem (const Project::Item& node);

    Project& project;
    build_tools::ResourceFile resourceFile;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JucerResourceFile)
};
