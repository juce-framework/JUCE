/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__


class FilePropertyComponent  : public PropertyComponent,
                               public FilenameComponentListener
{
public:
    FilePropertyComponent (const String& name,
                           const bool isDirectory,
                           const bool allowEditingOfFilename,
                           const String& fileBrowserWildcard = "*")
        : PropertyComponent (name),
          filenameComp (name, File::nonexistent, allowEditingOfFilename,
                        isDirectory, false, fileBrowserWildcard,
                        String::empty, String::empty)
    {
        addAndMakeVisible (&filenameComp);
        filenameComp.addListener (this);
    }

    virtual void setFile (const File& newFile) = 0;
    virtual File getFile() const = 0;

    void refresh()
    {
        filenameComp.setCurrentFile (getFile(), false);
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        if (getFile() != filenameComp.getCurrentFile())
            setFile (filenameComp.getCurrentFile());
    }

private:
    FilenameComponent filenameComp;
};


#endif   // __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__
