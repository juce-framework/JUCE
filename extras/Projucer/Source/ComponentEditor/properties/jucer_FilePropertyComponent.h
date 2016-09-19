/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_FILEPROPERTYCOMPONENT_H_INCLUDED
#define JUCER_FILEPROPERTYCOMPONENT_H_INCLUDED


class FilePropertyComponent  : public PropertyComponent,
                               public FilenameComponentListener
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


#endif   // JUCER_FILEPROPERTYCOMPONENT_H_INCLUDED
