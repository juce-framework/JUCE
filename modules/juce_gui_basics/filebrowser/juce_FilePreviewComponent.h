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

#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__

#include "../components/juce_Component.h"


//==============================================================================
/**
    Base class for components that live inside a file chooser dialog box and
    show previews of the files that get selected.

    One of these allows special extra information to be displayed for files
    in a dialog box as the user selects them. Each time the current file or
    directory is changed, the selectedFileChanged() method will be called
    to allow it to update itself appropriately.

    @see FileChooser, ImagePreviewComponent
*/
class JUCE_API  FilePreviewComponent  : public Component
{
public:
    //==============================================================================
    /** Creates a FilePreviewComponent. */
    FilePreviewComponent();

    /** Destructor. */
    ~FilePreviewComponent();

    /** Called to indicate that the user's currently selected file has changed.

        @param newSelectedFile  the newly selected file or directory, which may be
                                File::nonexistent if none is selected.
    */
    virtual void selectedFileChanged (const File& newSelectedFile) = 0;


private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FilePreviewComponent)
};


#endif   // __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
