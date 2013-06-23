/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"

//==============================================================================
// this is the listbox containing the draggable source components..

class DragAndDropDemoSource  : public ListBox,
                               public ListBoxModel
{
public:
    //==============================================================================
    DragAndDropDemoSource()
        : ListBox ("d+d source", 0)
    {
        // tells the ListBox that this object supplies the info about its rows.
        setModel (this);

        setMultipleSelectionEnabled (true);
    }

    //==============================================================================
    // The following methods implement the necessary virtual functions from ListBoxModel,
    // telling the listbox how many rows there are, painting them, etc.
    int getNumRows()
    {
        return 30;
    }

    void paintListBoxItem (int rowNumber,
                           Graphics& g,
                           int width, int height,
                           bool rowIsSelected)
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);

        g.setColour (Colours::black);
        g.setFont (height * 0.7f);

        g.drawText ("Row Number " + String (rowNumber + 1),
                    5, 0, width, height,
                    Justification::centredLeft, true);
    }

    var getDragSourceDescription (const SparseSet<int>& selectedRows)
    {
        // for our drag desctription, we'll just make a list of the selected
        // row numbers - this will be picked up by the drag target and displayed in
        // its box.
        String desc;

        for (int i = 0; i < selectedRows.size(); ++i)
            desc << (selectedRows [i] + 1) << " ";

        return desc.trim();
    }

    //==============================================================================
    // this just fills in the background of the listbox
    void paint (Graphics& g)
    {
        g.fillAll (Colours::white.withAlpha (0.7f));
    }
};


//==============================================================================
// and this is a component that can have things dropped onto it..

class DragAndDropDemoTarget  : public Component,
                               public DragAndDropTarget,
                               public FileDragAndDropTarget,
                               public TextDragAndDropTarget
{
public:
    //==============================================================================
    DragAndDropDemoTarget()
        : message ("Drag-and-drop some rows from the top-left box onto this component!\n\n"
                   "You can also drag-and-drop files here"),
          somethingIsBeingDraggedOver (false)
    {
    }

    ~DragAndDropDemoTarget()
    {
    }

    //==============================================================================
    void paint (Graphics& g)
    {
        g.fillAll (Colours::green.withAlpha (0.2f));

        // draw a red line around the comp if the user's currently dragging something over it..
        if (somethingIsBeingDraggedOver)
        {
            g.setColour (Colours::red);
            g.drawRect (0, 0, getWidth(), getHeight(), 3);
        }

        g.setColour (Colours::black);
        g.setFont (14.0f);
        g.drawFittedText (message, getLocalBounds().reduced (10, 0), Justification::centred, 4);
    }

    //==============================================================================
    // These methods implement the DragAndDropTarget interface, and allow our component
    // to accept drag-and-drop of objects from other Juce components..

    bool isInterestedInDragSource (const SourceDetails& /*dragSourceDetails*/)
    {
        // normally you'd check the sourceDescription value to see if it's the
        // sort of object that you're interested in before returning true, but for
        // the demo, we'll say yes to anything..
        return true;
    }

    void itemDragEnter (const SourceDetails& /*dragSourceDetails*/)
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }

    void itemDragMove (const SourceDetails& /*dragSourceDetails*/)
    {
    }

    void itemDragExit (const SourceDetails& /*dragSourceDetails*/)
    {
        somethingIsBeingDraggedOver = false;
        repaint();
    }

    void itemDropped (const SourceDetails& dragSourceDetails)
    {
        message = "last rows dropped: " + dragSourceDetails.description.toString();

        somethingIsBeingDraggedOver = false;
        repaint();
    }


    //==============================================================================
    // These methods implement the FileDragAndDropTarget interface, and allow our component
    // to accept drag-and-drop of files..

    bool isInterestedInFileDrag (const StringArray& /*files*/)
    {
        // normally you'd check these files to see if they're something that you're
        // interested in before returning true, but for the demo, we'll say yes to anything..
        return true;
    }

    void fileDragEnter (const StringArray& /*files*/, int /*x*/, int /*y*/)
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }

    void fileDragMove (const StringArray& /*files*/, int /*x*/, int /*y*/)
    {
    }

    void fileDragExit (const StringArray& /*files*/)
    {
        somethingIsBeingDraggedOver = false;
        repaint();
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/)
    {
        message = "files dropped: " + files.joinIntoString ("\n");

        somethingIsBeingDraggedOver = false;
        repaint();
    }

    // These methods implement the TextDragAndDropTarget interface, and allow our component
    // to accept drag-and-drop of text..

    bool isInterestedInTextDrag (const String& /*text*/)
    {
        return true;
    }

    void textDragEnter (const String& /*text*/, int /*x*/, int /*y*/)
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }

    void textDragMove (const String& /*text*/, int /*x*/, int /*y*/)
    {
    }

    void textDragExit (const String& /*text*/)
    {
        somethingIsBeingDraggedOver = false;
        repaint();
    }

    void textDropped (const String& text, int /*x*/, int /*y*/)
    {
        message = "text dropped:\n" + text;

        somethingIsBeingDraggedOver = false;
        repaint();
    }

private:
    String message;
    bool somethingIsBeingDraggedOver;
};


//==============================================================================
class DragAndDropDemo  : public Component,
                         public DragAndDropContainer
{
public:
    //==============================================================================
    DragAndDropDemo()
    {
        setName ("Drag-and-Drop");

        addAndMakeVisible (&source);
        addAndMakeVisible (&target);
    }

    ~DragAndDropDemo()
    {
    }

    void resized()
    {
        source.setBounds (10, 10, 250, 150);
        target.setBounds (getWidth() - 260, getHeight() - 160, 250, 150);
    }

private:
    DragAndDropDemoSource source;
    DragAndDropDemoTarget target;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DragAndDropDemo)
};


//==============================================================================
Component* createDragAndDropDemo()
{
    return new DragAndDropDemo();
}
