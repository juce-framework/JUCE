/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_RESOURCEFILE_JUCEHEADER__
#define __JUCER_RESOURCEFILE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class ResourceFile
{
public:
    //==============================================================================
    ResourceFile (Project& project);
    ~ResourceFile();

    //==============================================================================
    static bool isResourceFile (const File& file);

    //==============================================================================
    void setJuceHeaderToInclude (const File& header);
    void setClassName (const String& className);
    void addFile (const File& file);

    int getNumFiles() const                 { return files.size(); }
    int64 getTotalDataSize() const;

    bool write (const File& cppFile);
    bool write (const File& cppFile, OutputStream& cpp, OutputStream& header);

    //==============================================================================
private:
    OwnedArray <File> files;
    Project& project;
    File juceHeader;
    String className;

    void addResourcesFromProjectItem (const Project::Item& node);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ResourceFile);
};


#endif   // __JUCER_RESOURCEFILE_JUCEHEADER__
