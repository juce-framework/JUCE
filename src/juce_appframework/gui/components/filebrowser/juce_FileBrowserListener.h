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
};


#endif   // __JUCE_FILEBROWSERLISTENER_JUCEHEADER__
