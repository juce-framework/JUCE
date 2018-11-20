/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             ImagesDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Displays image files.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        ImagesDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class ImagesDemo  : public Component,
                    public FileBrowserListener
{
public:
    ImagesDemo()
    {
        setOpaque (true);
        imageList.setDirectory (File::getSpecialLocation (File::userPicturesDirectory), true, true);
        directoryThread.startThread (1);

        fileTree.addListener (this);
        fileTree.setColour (TreeView::backgroundColourId, Colours::grey);
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        addAndMakeVisible (imagePreview);

        // we have to set up our StretchableLayoutManager so it know the limits and preferred sizes of it's contents
        stretchableManager.setItemLayout (0,            // for the fileTree
                                          -0.1, -0.9,   // must be between 50 pixels and 90% of the available space
                                          -0.3);        // and its preferred size is 30% of the total available space

        stretchableManager.setItemLayout (1,            // for the resize bar
                                          5, 5, 5);     // hard limit to 5 pixels

        stretchableManager.setItemLayout (2,            // for the imagePreview
                                          -0.1, -0.9,   // size must be between 50 pixels and 90% of the available space
                                          -0.7);        // and its preferred size is 70% of the total available space

        setSize (500, 500);
    }

    ~ImagesDemo()
    {
        fileTree.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);

        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, &imagePreview };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                             true, true);
    }

private:
    WildcardFileFilter imagesWildcardFilter  { "*.jpeg;*.jpg;*.png;*.gif", "*", "Image File Filter"};
    TimeSliceThread directoryThread          { "Image File Scanner Thread" };
    DirectoryContentsList imageList          { &imagesWildcardFilter, directoryThread };
    FileTreeComponent fileTree               { imageList };

    ImageComponent imagePreview;

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar { &stretchableManager, 1, false };

    void selectionChanged() override
    {
        // we're only really interested in when the selection changes, regardless of if it was
        // clicked or not so we'll only override this method
        auto selectedFile = fileTree.getSelectedFile();

        if (selectedFile.existsAsFile())
            imagePreview.setImage (ImageCache::getFromFile (selectedFile));

        // the image cahce is a handly way to load images from files or directly from memory and
        // will keep them hanging around for a few seconds in case they are requested elsewhere
    }

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&)              override {}
    void browserRootChanged (const File&)             override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagesDemo)
};
