/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_ITEMPREVIEWCOMPONENT_JUCEHEADER__
#define __JUCER_ITEMPREVIEWCOMPONENT_JUCEHEADER__


//==============================================================================
/**
*/
class ItemPreviewComponent  : public Component
{
public:
    //==============================================================================
    // This will delete the stream
    ItemPreviewComponent (const File& file);
    ~ItemPreviewComponent();

    void paint (Graphics& g);
    void resized();


private:
    StringArray facts;
    File file;
    Image image;

    void tryToLoadImage();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemPreviewComponent);
};


#endif   // __JUCER_ITEMPREVIEWCOMPONENT_JUCEHEADER__
