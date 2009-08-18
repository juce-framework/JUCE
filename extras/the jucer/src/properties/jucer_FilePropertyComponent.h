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

#ifndef __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__
#define __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__


//==============================================================================
/**
*/
class FilePropertyComponent  : public PropertyComponent,
                               public FilenameComponentListener
{
public:
    //==============================================================================
    FilePropertyComponent (const String& name,
                          const bool isDirectory,
                          const bool allowEditingOfFilename,
                          const String& fileBrowserWildcard = T("*"));

    ~FilePropertyComponent();


    //==============================================================================
    virtual void setFile (const File& newFile) = 0;
    virtual const File getFile() const = 0;

    //==============================================================================
    void refresh();
    void filenameComponentChanged (FilenameComponent*);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FilenameComponent* filenameComp;
};


#endif   // __JUCER_FILEPROPERTYCOMPONENT_JUCEHEADER__
