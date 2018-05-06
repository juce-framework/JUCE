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

 name:             VideoDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays video files.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_video
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        VideoDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
// so that we can easily have two video windows each with a file browser, wrap this up as a class..
class MovieComponentWithFileBrowser  : public Component,
                                       public DragAndDropTarget,
                                       private FilenameComponentListener
{
public:
    MovieComponentWithFileBrowser()
    {
        addAndMakeVisible (videoComp);

        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    void setFile (const File& file)
    {
        fileChooser.setCurrentFile (file, true);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (isDragOver)
        {
            g.setColour (Colours::red);
            g.drawRect (fileChooser.getBounds(), 2);
        }
    }

    void resized() override
    {
        videoComp.setBounds (getLocalBounds().reduced (10));
    }

    bool isInterestedInDragSource (const SourceDetails&) override   { return true; }

    void itemDragEnter (const SourceDetails&) override
    {
        isDragOver = true;
        repaint();
    }

    void itemDragExit (const SourceDetails&) override
    {
        isDragOver = false;
        repaint();
    }

    void itemDropped (const SourceDetails& dragSourceDetails) override
    {
        setFile (dragSourceDetails.description.toString());
        isDragOver = false;
        repaint();
    }

private:
    VideoComponent videoComp;

    bool isDragOver = false;
    FilenameComponent fileChooser  { "movie", {}, true, false, false, "*", {}, "(choose a video file to play)"};

    void filenameComponentChanged (FilenameComponent*) override
    {
        // this is called when the user changes the filename in the file chooser box
        auto result = videoComp.load (fileChooser.getCurrentFile());

        if (result.wasOk())
        {
            // loaded the file ok, so let's start it playing..

            videoComp.play();
            resized(); // update to reflect the video's aspect ratio
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Couldn't load the file!",
                                              result.getErrorMessage());
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MovieComponentWithFileBrowser)
};

//==============================================================================
class VideoDemo   : public Component,
                    public DragAndDropContainer,
                    private FileBrowserListener
{
public:
    VideoDemo()
    {
        setOpaque (true);

        movieList.setDirectory (File::getSpecialLocation (File::userMoviesDirectory), true, true);
        directoryThread.startThread (1);

        fileTree.addListener (this);
        fileTree.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        loadLeftButton .onClick = [this] { movieCompLeft .setFile (fileTree.getSelectedFile (0)); };
        loadRightButton.onClick = [this] { movieCompRight.setFile (fileTree.getSelectedFile (0)); };

        addAndMakeVisible (loadLeftButton);
        addAndMakeVisible (loadRightButton);

        addAndMakeVisible (movieCompLeft);
        addAndMakeVisible (movieCompRight);

        // we have to set up our StretchableLayoutManager so it know the limits and preferred sizes of it's contents
        stretchableManager.setItemLayout (0,            // for the fileTree
                                          -0.1, -0.9,   // must be between 50 pixels and 90% of the available space
                                          -0.3);        // and its preferred size is 30% of the total available space

        stretchableManager.setItemLayout (1,            // for the resize bar
                                          5, 5, 5);     // hard limit to 5 pixels

        stretchableManager.setItemLayout (2,            // for the movie components
                                          -0.1, -0.9,   // size must be between 50 pixels and 90% of the available space
                                          -0.7);        // and its preferred size is 70% of the total available space

        setSize (500, 500);
    }

    ~VideoDemo()
    {
        fileTree.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, nullptr };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             0, 0, getWidth(), getHeight(),
                                             true, true);

        // now position out two video components in the space that's left
        auto area = getLocalBounds().removeFromBottom (getHeight() - resizerBar.getBottom());

        {
            auto buttonArea = area.removeFromTop (30);
            loadLeftButton .setBounds (buttonArea.removeFromLeft (buttonArea.getWidth() / 2).reduced (5));
            loadRightButton.setBounds (buttonArea.reduced (5));
        }

        movieCompLeft .setBounds (area.removeFromLeft (area.getWidth() / 2).reduced (5));
        movieCompRight.setBounds (area.reduced (5));
    }

private:
    WildcardFileFilter moviesWildcardFilter  { "*", "*", "Movies File Filter" };
    TimeSliceThread directoryThread          { "Movie File Scanner Thread" };
    DirectoryContentsList movieList          { &moviesWildcardFilter, directoryThread };
    FileTreeComponent fileTree               { movieList };

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar   { &stretchableManager, 1, false };

    TextButton loadLeftButton   { "Load Left" },
               loadRightButton  { "Load Right" };
    MovieComponentWithFileBrowser movieCompLeft, movieCompRight;

    void selectionChanged() override
    {
        // we're just going to update the drag description of out tree so that rows can be dragged onto the file players
        fileTree.setDragAndDropDescription (fileTree.getSelectedFile().getFullPathName());
    }

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&)              override {}
    void browserRootChanged (const File&)             override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
};
