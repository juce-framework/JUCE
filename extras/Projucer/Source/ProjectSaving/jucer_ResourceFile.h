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
