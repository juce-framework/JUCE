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
class TreeViewDemoItem  : public TreeViewItem
{
    XmlElement* xml;

public:
    TreeViewDemoItem (XmlElement* const xml_)
        : xml (xml_)
    {
    }

    ~TreeViewDemoItem()
    {
    }

    int getItemWidth() const
    {
        return xml->getIntAttribute (T("width"), -1);
    }

    const String getUniqueName() const
    {
        if (xml != 0)
            return xml->getTagName();
        else
            return String::empty;
    }

    bool mightContainSubItems()
    {
        return xml != 0
                && xml->getFirstChildElement() != 0;
    }

    void paintItem (Graphics& g, int width, int height)
    {
        if (xml != 0)
        {
            // if this item is selected, fill it with a background colour..
            if (isSelected())
                g.fillAll (Colours::blue.withAlpha (0.3f));

            // use a "colour" attribute in the xml tag for this node to set the text colour..
            g.setColour (Colour (xml->getStringAttribute (T("colour"), T("ff000000")).getHexValue32()));

            g.setFont (height * 0.7f);

            // draw the xml element's tag name..
            g.drawText (xml->getTagName(),
                        4, 0, width - 4, height,
                        Justification::centredLeft, true);
        }
    }

    void itemOpennessChanged (bool isNowOpen)
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                // create and add sub-items to this node of the tree, corresponding to
                // each sub-element in the XML..

                if (xml != 0)
                {
                    forEachXmlChildElement (*xml, child)
                    {
                        addSubItem (new TreeViewDemoItem (child));
                    }
                }
            }
        }
        else
        {
            // in this case, we'll leave any sub-items in the tree when the node gets closed,
            // though you could choose to delete them if that's more appropriate for
            // your application.
        }
    }

    const String getDragSourceDescription()
    {
        return T("TreeView Items");
    }
};

//==============================================================================
class TreeViewDemo  : public Component,
                      public DragAndDropContainer,
                      public ButtonListener
{
    XmlElement* treeXml;

    TreeViewItem* rootItem;
    TreeView* treeView;

    FileTreeComponent* fileTreeComp;
    DirectoryContentsList* directoryList;
    TimeSliceThread thread;

    TextButton* typeButton;

public:
    //==============================================================================
    TreeViewDemo()
        : treeView (0),
          rootItem (0),
          fileTreeComp (0),
          directoryList (0),
          thread ("Demo file tree thread")
    {
        setName (T("Tree Views"));

        const String treeXmlString (BinaryData::treedemo_xml);
        XmlDocument parser (treeXmlString);
        treeXml = parser.getDocumentElement();

        rootItem = new TreeViewDemoItem (treeXml);
        rootItem->setOpen (true);

        // find the root of the user's home drive, and set that as our root..
        File folder (File::getSpecialLocation (File::userHomeDirectory));
        while (folder.getParentDirectory() != folder)
            folder = folder.getParentDirectory();

        directoryList = new DirectoryContentsList (0, thread);
        directoryList->setDirectory (folder, true, true);
        thread.startThread (3);

        addAndMakeVisible (typeButton = new TextButton (T("Type of treeview...")));
        typeButton->addButtonListener (this);
        typeButton->setAlwaysOnTop (true);
        typeButton->setTriggeredOnMouseDown (true);

        showCustomTreeView();
    }

    ~TreeViewDemo()
    {
        deleteAllChildren();

        delete rootItem;
        delete treeXml;
        delete directoryList;
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::grey);

        if (treeView != 0)
            g.drawRect (treeView->getX(), treeView->getY(),
                        treeView->getWidth(), treeView->getHeight());

        if (fileTreeComp != 0)
            g.drawRect (fileTreeComp->getX(), fileTreeComp->getY(),
                        fileTreeComp->getWidth(), fileTreeComp->getHeight());
    }

    void resized()
    {
        if (treeView != 0)
            treeView->setBoundsInset (BorderSize (40, 10, 10, 10));
        else if (fileTreeComp != 0)
            fileTreeComp->setBoundsInset (BorderSize (40, 10, 10, 10));

        typeButton->changeWidthToFitText (22);
        typeButton->setTopLeftPosition (10, 10);
    }

    void showCustomTreeView()
    {
        deleteAndZero (treeView);
        deleteAndZero (fileTreeComp);

        addAndMakeVisible (treeView = new TreeView());
        treeView->setRootItem (rootItem);
        treeView->setMultiSelectEnabled (true);

        resized();
    }

    void showFileTreeComp()
    {
        deleteAndZero (treeView);
        deleteAndZero (fileTreeComp);

        addAndMakeVisible (fileTreeComp = new FileTreeComponent (*directoryList));

        resized();
    }

    void buttonClicked (Button*)
    {
        PopupMenu m;
        m.addItem (1, T("Custom treeview showing an XML tree"));
        m.addItem (2, T("FileTreeComponent showing the file system"));
        m.addSeparator();
        m.addItem (3, T("Show root item"), true,
                   treeView != 0 ? treeView->isRootItemVisible()
                                 : fileTreeComp->isRootItemVisible());
        m.addItem (4, T("Show open/close buttons"), true,
                   treeView != 0 ? treeView->areOpenCloseButtonsVisible()
                                 : fileTreeComp->areOpenCloseButtonsVisible());

        const int r = m.showAt (typeButton);

        if (r == 1)
        {
            showCustomTreeView();
        }
        else if (r == 2)
        {
            showFileTreeComp();
        }
        else if (r == 3)
        {
            if (treeView != 0)
                treeView->setRootItemVisible (! treeView->isRootItemVisible());
            else
                fileTreeComp->setRootItemVisible (! fileTreeComp->isRootItemVisible());
        }
        else if (r == 4)
        {
            if (treeView != 0)
                treeView->setOpenCloseButtonsVisible (! treeView->areOpenCloseButtonsVisible());
            else
                fileTreeComp->setOpenCloseButtonsVisible (! fileTreeComp->areOpenCloseButtonsVisible());
        }
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
Component* createTreeViewDemo()
{
    return new TreeViewDemo();
}
