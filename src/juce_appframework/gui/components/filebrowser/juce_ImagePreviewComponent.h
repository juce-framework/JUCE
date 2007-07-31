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

#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__

#include "juce_FilePreviewComponent.h"
#include "../../../events/juce_Timer.h"


//==============================================================================
/**
    A simple preview component that shows thumbnails of image files.

    @see FileChooserDialogBox, FilePreviewComponent
*/
class JUCE_API  ImagePreviewComponent  : public FilePreviewComponent,
                                         private Timer
{
public:
    //==============================================================================
    /** Creates an ImagePreviewComponent. */
    ImagePreviewComponent();

    /** Destructor. */
    ~ImagePreviewComponent();


    //==============================================================================
    /** @internal */
    void selectedFileChanged (const File& newSelectedFile);
    /** @internal */
    void paint (Graphics& g);
    /** @internal */
    void timerCallback();

    juce_UseDebuggingNewOperator

private:
    File fileToLoad;
    Image* currentThumbnail;
    String currentDetails;

    void getThumbSize (int& w, int& h) const;

    ImagePreviewComponent (const ImagePreviewComponent&);
    const ImagePreviewComponent& operator= (const ImagePreviewComponent&);
};


#endif   // __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
