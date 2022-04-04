/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class FilePropertyComponent  : public PropertyComponent,
                               private FilenameComponentListener
{
public:
    FilePropertyComponent (const String& name,
                           const bool isDirectory,
                           const bool allowEditingOfFilename,
                           const String& fileBrowserWildcard = "*")
        : PropertyComponent (name),
          filenameComp (name, File(), allowEditingOfFilename,
                        isDirectory, false, fileBrowserWildcard,
                        String(), String())
    {
        addAndMakeVisible (filenameComp);
        filenameComp.addListener (this);
    }

    virtual void setFile (const File& newFile) = 0;
    virtual File getFile() const = 0;

    void refresh() override
    {
        filenameComp.setCurrentFile (getFile(), false);
    }

private:
    void filenameComponentChanged (FilenameComponent*) override
    {
        if (getFile() != filenameComp.getCurrentFile())
            setFile (filenameComp.getCurrentFile());
    }

    FilenameComponent filenameComp;
};
