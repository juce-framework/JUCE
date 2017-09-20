/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../JuceDemoHeader.h"

#if JUCE_MAC || (JUCE_WINDOWS && ! JUCE_MINGW)

//==============================================================================
// so that we can easily have two video windows each with a file browser, wrap this up as a class..
class MovieComponentWithFileBrowser  : public Component,
                                       public DragAndDropTarget,
                                       private FilenameComponentListener
{
public:
    MovieComponentWithFileBrowser()
        : isDragOver (false),
          fileChooser ("movie", File(), true, false, false,
                       "*", String(), "(choose a video file to play)")
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

    bool isDragOver;
    FilenameComponent fileChooser;

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
                    private Button::Listener,
                    private FileBrowserListener
{
public:
    VideoDemo()
        : moviesWildcardFilter ("*", "*", "Movies File Filter"),
          directoryThread ("Movie File Scanner Thread"),
          movieList (&moviesWildcardFilter, directoryThread),
          fileTree (movieList),
          resizerBar (&stretchableManager, 1, false)
    {
        setOpaque (true);

        movieList.setDirectory (File::getSpecialLocation (File::userMoviesDirectory), true, true);
        directoryThread.startThread (1);

        fileTree.addListener (this);
        fileTree.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        loadLeftButton.setButtonText ("Load Left");
        loadRightButton.setButtonText ("Load Right");

        loadLeftButton.addListener (this);
        loadRightButton.addListener (this);

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
    }

    ~VideoDemo()
    {
        loadLeftButton.removeListener (this);
        loadRightButton.removeListener (this);
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
        Rectangle<int> area (getLocalBounds().removeFromBottom (getHeight() - resizerBar.getBottom()));

        {
            Rectangle<int> buttonArea (area.removeFromTop (30));
            loadLeftButton.setBounds (buttonArea.removeFromLeft (buttonArea.getWidth() / 2).reduced (5));
            loadRightButton.setBounds (buttonArea.reduced (5));
        }

        movieCompLeft.setBounds (area.removeFromLeft (area.getWidth() / 2).reduced (5));
        movieCompRight.setBounds (area.reduced (5));
    }

private:
    WildcardFileFilter moviesWildcardFilter;
    TimeSliceThread directoryThread;
    DirectoryContentsList movieList;
    FileTreeComponent fileTree;

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar;

    TextButton loadLeftButton, loadRightButton;
    MovieComponentWithFileBrowser movieCompLeft, movieCompRight;

    void buttonClicked (Button* button) override
    {
        if (button == &loadLeftButton)
            movieCompLeft.setFile (fileTree.getSelectedFile (0));
        else if (button == &loadRightButton)
            movieCompRight.setFile (fileTree.getSelectedFile (0));
    }

    void selectionChanged() override
    {
        // we're just going to update the drag description of out tree so that rows can be dragged onto the file players
        fileTree.setDragAndDropDescription (fileTree.getSelectedFile().getFullPathName());
    }

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&) override {}
    void browserRootChanged (const File&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<VideoDemo> demo ("29 Graphics: Video Playback");

#endif
