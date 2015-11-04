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

//==============================================================================
class ImagesDemo  : public Component,
                    public FileBrowserListener
{
public:
    ImagesDemo()
        : imagesWildcardFilter ("*.jpeg;*.jpg;*.png;*.gif", "*", "Image File Filter"),
          directoryThread ("Image File Scanner Thread"),
          imageList (&imagesWildcardFilter, directoryThread),
          fileTree (imageList),
          resizerBar (&stretchableManager, 1, false)
    {
        setOpaque (true);
        imageList.setDirectory (File::getSpecialLocation (File::userPicturesDirectory), true, true);
        directoryThread.startThread (1);

        fileTree.addListener (this);
        fileTree.setColour (TreeView::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
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
    }

    ~ImagesDemo()
    {
        fileTree.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (4));

        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, &imagePreview };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             r.getX(), r.getY(), r.getWidth(), r.getHeight(),
                                             true, true);
    }

private:
    WildcardFileFilter imagesWildcardFilter;
    TimeSliceThread directoryThread;
    DirectoryContentsList imageList;
    FileTreeComponent fileTree;

    ImageComponent imagePreview;

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar;

    void selectionChanged() override
    {
        // we're only really interested in when the selection changes, regardless of if it was
        // clicked or not so we'll only override this method
        const File selectedFile (fileTree.getSelectedFile());

        if (selectedFile.existsAsFile())
            imagePreview.setImage (ImageCache::getFromFile (selectedFile));

        // the image cahce is a handly way to load images from files or directly from memory and
        // will keep them hanging around for a few seconds in case they are requested elsewhere
    }

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&) override {}
    void browserRootChanged (const File&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ImagesDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<ImagesDemo> demo ("20 Graphics: Image formats");
