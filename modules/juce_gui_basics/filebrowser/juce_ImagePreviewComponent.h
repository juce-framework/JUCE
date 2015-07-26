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

#ifndef JUCE_IMAGEPREVIEWCOMPONENT_H_INCLUDED
#define JUCE_IMAGEPREVIEWCOMPONENT_H_INCLUDED


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
    void selectedFileChanged (const File& newSelectedFile) override;
    /** @internal */
    void paint (Graphics&) override;
    /** @internal */
    void timerCallback() override;

private:
    File fileToLoad;
    Image currentThumbnail;
    String currentDetails;

    void getThumbSize (int& w, int& h) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagePreviewComponent)
};


#endif   // JUCE_IMAGEPREVIEWCOMPONENT_H_INCLUDED
