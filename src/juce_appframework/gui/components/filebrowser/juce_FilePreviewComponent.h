/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__

#include "../../../../juce_core/io/files/juce_File.h"
#include "../juce_Component.h"


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


    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    FilePreviewComponent (const FilePreviewComponent&);
    const FilePreviewComponent& operator= (const FilePreviewComponent&);
};


#endif   // __JUCE_FILEPREVIEWCOMPONENT_JUCEHEADER__
