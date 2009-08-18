/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_FilePropertyComponent.h"


//==============================================================================
FilePropertyComponent::FilePropertyComponent (const String& name,
                                              const bool isDirectory,
                                              const bool allowEditingOfFilename,
                                              const String& fileBrowserWildcard)
    : PropertyComponent (name)
{
    addAndMakeVisible (filenameComp = new FilenameComponent (name,
                                                             File::nonexistent,
                                                             allowEditingOfFilename,
                                                             isDirectory,
                                                             false,
                                                             fileBrowserWildcard,
                                                             String::empty,
                                                             String::empty));

    filenameComp->addListener (this);
}

FilePropertyComponent::~FilePropertyComponent()
{
    deleteAllChildren();
}

void FilePropertyComponent::refresh()
{
    filenameComp->setCurrentFile (getFile(), false);
}

void FilePropertyComponent::filenameComponentChanged (FilenameComponent*)
{
    if (getFile() != filenameComp->getCurrentFile())
        setFile (filenameComp->getCurrentFile());
}
