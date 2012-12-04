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

#ifndef __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__

#include "juce_FilePreviewComponent.h"


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

private:
    File fileToLoad;
    Image currentThumbnail;
    String currentDetails;

    void getThumbSize (int& w, int& h) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagePreviewComponent)
};


#endif   // __JUCE_IMAGEPREVIEWCOMPONENT_JUCEHEADER__
