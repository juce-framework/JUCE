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
class TreeViewDemoItem  : public TreeViewItem
{
public:
    TreeViewDemoItem (XmlElement& xml_)
        : xml (xml_)
    {
    }

    int getItemWidth() const
    {
        return xml.getIntAttribute ("width", -1);
    }

    String getUniqueName() const
    {
        return xml.getTagName();
    }

    bool mightContainSubItems()
    {
        return xml.getFirstChildElement() != 0;
    }

    void paintItem (Graphics& g, int width, int height)
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colours::blue.withAlpha (0.3f));

        // use a "colour" attribute in the xml tag for this node to set the text colour..
        g.setColour (Colour::fromString (xml.getStringAttribute ("colour", "ff000000")));

        g.setFont (height * 0.7f);

        // draw the xml element's tag name..
        g.drawText (xml.getTagName(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
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

                forEachXmlChildElement (xml, child)
                {
                    jassert (child != 0);
                    addSubItem (new TreeViewDemoItem (*child));
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

    var getDragSourceDescription()
    {
        return "TreeView Items";
    }

private:
    XmlElement& xml;
};

//==============================================================================
class TreeViewDemo  : public Component,
                      public DragAndDropContainer,
                      public ButtonListener
{
public:
    //==============================================================================
    TreeViewDemo()
        : treeView (0),
          thread ("Demo file tree thread"),
          typeButton ("Type of treeview...")
    {
        setName ("Tree Views");

        {
            const String treeXmlString (BinaryData::treedemo_xml);
            XmlDocument parser (treeXmlString);
            treeXml = parser.getDocumentElement();
            jassert (treeXml != nullptr);
        }

        rootItem = new TreeViewDemoItem (*treeXml);
        rootItem->setOpen (true);

        // find the root of the user's home drive, and set that as our root..
        File folder (File::getSpecialLocation (File::userHomeDirectory));
        while (folder.getParentDirectory() != folder)
            folder = folder.getParentDirectory();

        directoryList = new DirectoryContentsList (0, thread);
        directoryList->setDirectory (folder, true, true);
        thread.startThread (3);

        addAndMakeVisible (&typeButton);
        typeButton.addListener (this);
        typeButton.setAlwaysOnTop (true);
        typeButton.setTriggeredOnMouseDown (true);

        showCustomTreeView();
    }

    ~TreeViewDemo()
    {
        treeView = nullptr;
        fileTreeComp = nullptr;
        directoryList = nullptr; // (need to make sure this is deleted before the TimeSliceThread)
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::grey);

        if (treeView != nullptr)
            g.drawRect (treeView->getX(), treeView->getY(),
                        treeView->getWidth(), treeView->getHeight());

        if (fileTreeComp != nullptr)
            g.drawRect (fileTreeComp->getX(), fileTreeComp->getY(),
                        fileTreeComp->getWidth(), fileTreeComp->getHeight());
    }

    void resized()
    {
        if (treeView != nullptr)
            treeView->setBoundsInset (BorderSize<int> (40, 10, 10, 10));
        else if (fileTreeComp != nullptr)
            fileTreeComp->setBoundsInset (BorderSize<int> (40, 10, 10, 10));

        typeButton.changeWidthToFitText (22);
        typeButton.setTopLeftPosition (10, 10);
    }

    void showCustomTreeView()
    {
        treeView = nullptr;
        fileTreeComp = nullptr;

        addAndMakeVisible (treeView = new TreeView());
        treeView->setRootItem (rootItem);
        treeView->setMultiSelectEnabled (true);

        resized();
    }

    void showFileTreeComp()
    {
        treeView = nullptr;
        fileTreeComp = nullptr;

        addAndMakeVisible (fileTreeComp = new FileTreeComponent (*directoryList));
        resized();
    }

    void buttonClicked (Button*)
    {
        PopupMenu m;
        m.addItem (1, "Custom treeview showing an XML tree");
        m.addItem (2, "FileTreeComponent showing the file system");
        m.addSeparator();
        m.addItem (3, "Show root item", true,
                   treeView != nullptr ? treeView->isRootItemVisible()
                                       : fileTreeComp->isRootItemVisible());
        m.addItem (4, "Show open/close buttons", true,
                   treeView != nullptr ? treeView->areOpenCloseButtonsVisible()
                                       : fileTreeComp->areOpenCloseButtonsVisible());

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (&typeButton),
                         ModalCallbackFunction::forComponent (menuItemChosenCallback, this));
    }

    static void menuItemChosenCallback (int result, TreeViewDemo* demoComponent)
    {
        if (demoComponent != nullptr)
            demoComponent->menuItemChosenCallback (result);
    }

    void menuItemChosenCallback (int result)
    {
        if (result == 1)
        {
            showCustomTreeView();
        }
        else if (result == 2)
        {
            showFileTreeComp();
        }
        else if (result == 3)
        {
            if (treeView != nullptr)
                treeView->setRootItemVisible (! treeView->isRootItemVisible());
            else if (fileTreeComp != nullptr)
                fileTreeComp->setRootItemVisible (! fileTreeComp->isRootItemVisible());
        }
        else if (result == 4)
        {
            if (treeView != nullptr)
                treeView->setOpenCloseButtonsVisible (! treeView->areOpenCloseButtonsVisible());
            else if (fileTreeComp != nullptr)
                fileTreeComp->setOpenCloseButtonsVisible (! fileTreeComp->areOpenCloseButtonsVisible());
        }
    }

private:
    ScopedPointer <XmlElement> treeXml;

    ScopedPointer <TreeViewItem> rootItem;
    ScopedPointer <TreeView> treeView;

    ScopedPointer <FileTreeComponent> fileTreeComp;
    ScopedPointer <DirectoryContentsList> directoryList;
    TimeSliceThread thread;

    TextButton typeButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TreeViewDemo)
};


//==============================================================================
Component* createTreeViewDemo()
{
    return new TreeViewDemo();
}
