/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

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

        OwnedArray <File> roots;
        File::findFileSystemRoots (roots);
        directoryList = new DirectoryContentsList (0, thread);
        directoryList->setDirectory (*roots[0], true, true);
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
            treeView->setBoundsRelative (0.05f, 0.07f, 0.9f, 0.9f);
        else if (fileTreeComp != 0)
            fileTreeComp->setBoundsRelative (0.05f, 0.07f, 0.9f, 0.9f);

        typeButton->changeWidthToFitText (20);
        typeButton->setTopLeftPosition (40, 10);
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

        const int r = m.showAt (typeButton);

        if (r == 1)
        {
            showCustomTreeView();
        }
        else if (r == 2)
        {
            showFileTreeComp();
        }
    }

    juce_UseDebuggingNewOperator
};


//==============================================================================
Component* createTreeViewDemo()
{
    return new TreeViewDemo();
}
