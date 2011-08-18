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

#ifndef __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
#define __JUCE_FILEBROWSERLISTENER_JUCEHEADER__

#include "../mouse/juce_MouseEvent.h"


//==============================================================================
/**
    A listener for user selection events in a file browser.

    This is used by a FileBrowserComponent or FileListComponent.
*/
class JUCE_API  FileBrowserListener
{
public:
    //==============================================================================
    /** Destructor. */
    virtual ~FileBrowserListener();

    //==============================================================================
    /** Callback when the user selects a different file in the browser. */
    virtual void selectionChanged() = 0;

    /** Callback when the user clicks on a file in the browser. */
    virtual void fileClicked (const File& file, const MouseEvent& e) = 0;

    /** Callback when the user double-clicks on a file in the browser. */
    virtual void fileDoubleClicked (const File& file) = 0;

    /** Callback when the browser's root folder changes. */
    virtual void browserRootChanged (const File& newRoot) = 0;
};


#endif   // __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
