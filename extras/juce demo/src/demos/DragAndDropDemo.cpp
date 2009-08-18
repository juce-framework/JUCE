/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"

//==============================================================================
// this is the listbox containing the draggable source components..

class DragAndDropDemoSource  : public ListBox,
                               public ListBoxModel
{
public:
    //==============================================================================
    DragAndDropDemoSource()
        : ListBox (T("d+d source"), 0)
    {
        // tells the ListBox that this object supplies the info about
        // its rows.
        setModel (this);

        setMultipleSelectionEnabled (true);
    }

    ~DragAndDropDemoSource()
    {
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

        g.drawText (T("Row Number ") + String (rowNumber + 1),
                    5, 0, width, height,
                    Justification::centredLeft, true);
    }

    const String getDragSourceDescription (const SparseSet<int>& selectedRows)
    {
        // for our drag desctription, we'll just make a list of the selected
        // row numbers - this will be picked up by the drag target and displayed in
        // its box.
        String desc;

        for (int i = 0; i < selectedRows.size(); ++i)
            desc << (selectedRows [i] + 1) << T(" ");

        return desc.trim();
    }

    //==============================================================================
    // this just fills in the background of the listbox
    void paint (Graphics& g)
    {
        g.fillAll (Colours::white.withAlpha (0.7f));
    }

    /*void listBoxItemClicked (int row, const MouseEvent& e)
    {
        PopupMenu m;
        m.addItem (1, "sdfsdfs");

        m.show();

        //AlertWindow::showMessageBox (AlertWindow::InfoIcon, "asdfsadfads", "srdfsdfa");
        DocumentWindow* dw = new DocumentWindow ("sfdsd", Colours::white, DocumentWindow::allButtons, true);
        dw->setBounds (100, 100, 500, 500);
        dw->setVisible (true);
    }*/
};


//==============================================================================
// and this is a component that can have things dropped onto it..

class DragAndDropDemoTarget  : public Component,
                               public DragAndDropTarget
{
    bool somethingIsBeingDraggedOver;
    String message;

public:
    //==============================================================================
    DragAndDropDemoTarget()
    {
        somethingIsBeingDraggedOver = false;

        message = T("Drag-and-drop some rows from the top-left box onto this component!");
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
        g.drawFittedText (message, 10, 0, getWidth() - 20, getHeight(), Justification::centred, 4);
    }

    //==============================================================================
    bool isInterestedInDragSource (const String& sourceDescription,
                                   Component* sourceComponent)
    {
        // normally you'd check the sourceDescription value to see if it's the
        // sort of object that you're interested in before returning true, but for
        // the demo, we'll say yes to anything..
        return true;
    }

    void itemDragEnter (const String& sourceDescription,
                        Component* sourceComponent,
                        int x, int y)
    {
        somethingIsBeingDraggedOver = true;
        repaint();
    }

    void itemDragMove (const String& sourceDescription,
                       Component* sourceComponent,
                       int x, int y)
    {
    }

    void itemDragExit (const String& sourceDescription,
                       Component* sourceComponent)
    {
        somethingIsBeingDraggedOver = false;
        repaint();
    }

    void itemDropped (const String& sourceDescription,
                      Component* sourceComponent,
                      int x, int y)
    {
        message = T("last rows dropped: ") + sourceDescription;

        somethingIsBeingDraggedOver = false;
        repaint();
    }
};


//==============================================================================
class DragAndDropDemo  : public Component,
                         public DragAndDropContainer
{
    //==============================================================================
    DragAndDropDemoSource* source;
    DragAndDropDemoTarget* target;

public:
    //==============================================================================
    DragAndDropDemo()
    {
        setName (T("Drag-and-Drop"));

        source = new DragAndDropDemoSource();
        addAndMakeVisible (source);

        target = new DragAndDropDemoTarget();
        addAndMakeVisible (target);
    }

    ~DragAndDropDemo()
    {
        deleteAllChildren();
    }

    void resized()
    {
        source->setBounds (10, 10, 250, 150);
        target->setBounds (getWidth() - 260, getHeight() - 160, 250, 150);
    }

    //==============================================================================
    // (need to put this in to disambiguate the new/delete operators used in the
    // two base classes).
    juce_UseDebuggingNewOperator
};


//==============================================================================
Component* createDragAndDropDemo()
{
    return new DragAndDropDemo();
}
