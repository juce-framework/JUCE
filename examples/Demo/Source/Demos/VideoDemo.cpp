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

#include "../JuceDemoHeader.h"

#if JUCE_QUICKTIME || JUCE_DIRECTSHOW

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

    void paintOverChildren (Graphics& g)
    {
        if (isDragOver)
        {
            g.setColour (Colours::red);
            g.drawRect (fileChooser.getBounds(), 2);
        }
    }

    void resized()
    {
        videoComp.setBoundsWithCorrectAspectRatio (Rectangle<int> (0, 0, getWidth(), getHeight() - 30),
                                                   Justification::centred);
        fileChooser.setBounds (0, getHeight() - 24, getWidth(), 24);
    }

    bool isInterestedInDragSource (const SourceDetails&)  { return true; }

    void itemDragEnter (const SourceDetails&)
    {
        isDragOver = true;
        repaint();
    }

    void itemDragExit (const SourceDetails&)
    {
        isDragOver = false;
        repaint();
    }

    void itemDropped (const SourceDetails& dragSourceDetails)
    {
        setFile (dragSourceDetails.description.toString());
        isDragOver = false;
        repaint();
    }

private:
   #if JUCE_QUICKTIME
    QuickTimeMovieComponent videoComp;
   #elif JUCE_DIRECTSHOW
    DirectShowComponent videoComp;
   #endif

    bool isDragOver;
    FilenameComponent fileChooser;

    void filenameComponentChanged (FilenameComponent*) override
    {
        // this is called when the user changes the filename in the file chooser box
       #if JUCE_QUICKTIME
        if (videoComp.loadMovie (fileChooser.getCurrentFile(), true))
       #elif JUCE_DIRECTSHOW
        if (videoComp.loadMovie (fileChooser.getCurrentFile()))
       #endif
        {
            // loaded the file ok, so let's start it playing..

            videoComp.play();
            resized(); // update to reflect the video's aspect ratio
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Couldn't load the file!",
                                              r.getErrorMessage());
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
        fileTree.setColour (TreeView::backgroundColourId, Colours::lightgrey);
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
        fillStandardDemoBackground (g);
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
