/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__
#define __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__

#include "../jucer_Headers.h"
class ProjectContentComponent;
class Project;

//==============================================================================
class JucerTreeViewBase   : public TreeViewItem
{
public:
    JucerTreeViewBase();

    int getItemWidth() const                                { return -1; }
    int getItemHeight() const                               { return 20; }

    void paintItem (Graphics& g, int width, int height);
    void paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver);
    Component* createItemComponent();
    void itemClicked (const MouseEvent& e);
    void itemSelectionChanged (bool isNowSelected);
    void itemDoubleClicked (const MouseEvent&);

    void cancelDelayedSelectionTimer();

    //==============================================================================
    virtual Font getFont() const;
    virtual String getRenamingName() const = 0;
    virtual String getDisplayName() const = 0;
    virtual void setName (const String& newName) = 0;
    virtual bool isMissing() = 0;
    virtual Icon getIcon() const = 0;
    virtual float getIconSize() const;
    virtual void paintContent (Graphics& g, const Rectangle<int>& area);
    virtual int getMillisecsAllowedForDragGesture()    { return 120; };

    void refreshSubItems();
    virtual void deleteItem();
    virtual void deleteAllSelectedItems();
    virtual void showDocument();
    virtual void showPopupMenu();
    virtual void showMultiSelectionPopupMenu();
    virtual void showRenameBox();

    void launchPopupMenu (PopupMenu&); // runs asynchronously, and produces a callback to handlePopupMenuResult().
    virtual void handlePopupMenuResult (int resultCode);

    //==============================================================================
    // To handle situations where an item gets deleted before openness is
    // restored for it, this OpennessRestorer keeps only a pointer to the
    // topmost tree item.
    struct WholeTreeOpennessRestorer   : public OpennessRestorer
    {
        WholeTreeOpennessRestorer (TreeViewItem& item)  : OpennessRestorer (getTopLevelItem (item))
        {}

    private:
        static TreeViewItem& getTopLevelItem (TreeViewItem& item)
        {
            TreeViewItem* const p = item.getParentItem();
            return p != nullptr ? getTopLevelItem (*p) : item;
        }
    };

    int textX;

protected:
    ProjectContentComponent* getProjectContentComponent() const;
    virtual void addSubItems() {}

    Colour getBackgroundColour() const;
    Colour getContrastingColour (float contrast) const;
    Colour getContrastingColour (const Colour& targetColour, float minContrast) const;

private:
    class ItemSelectionTimer;
    friend class ItemSelectionTimer;
    ScopedPointer<Timer> delayedSelectionTimer;

    void invokeShowDocument();
};

//==============================================================================
class TreePanelBase   : public Component
{
public:
    TreePanelBase (const Project* p, const String& treeviewID)
        : project (p), opennessStateKey (treeviewID)
    {
        addAndMakeVisible (&tree);
        tree.setRootItemVisible (true);
        tree.setDefaultOpenness (true);
        tree.setColour (TreeView::backgroundColourId, Colours::transparentBlack);
        tree.setIndentSize (14);
        tree.getViewport()->setScrollBarThickness (14);
    }

    ~TreePanelBase()
    {
        tree.setRootItem (nullptr);
    }

    void setRoot (JucerTreeViewBase* root);
    void saveOpenness();

    void deleteSelectedItems()
    {
        if (rootItem != nullptr)
            rootItem->deleteAllSelectedItems();
    }

    void setEmptyTreeMessage (const String& newMessage)
    {
        if (emptyTreeMessage != newMessage)
        {
            emptyTreeMessage = newMessage;
            repaint();
        }
    }

    static void drawEmptyPanelMessage (Component& comp, Graphics& g, const String& message)
    {
        const int fontHeight = 13;
        const Rectangle<int> area (comp.getLocalBounds());
        g.setColour (comp.findColour (mainBackgroundColourId).contrasting (0.7f));
        g.setFont ((float) fontHeight);
        g.drawFittedText (message, area.reduced (4, 2), Justification::centred, area.getHeight() / fontHeight);
    }

    void paint (Graphics& g)
    {
        if (emptyTreeMessage.isNotEmpty() && (rootItem == nullptr || rootItem->getNumSubItems() == 0))
            drawEmptyPanelMessage (*this, g, emptyTreeMessage);
    }

    void resized()
    {
        tree.setBounds (getAvailableBounds());
    }

    Rectangle<int> getAvailableBounds() const
    {
        return Rectangle<int> (0, 2, getWidth() - 2, getHeight() - 2);
    }

    const Project* project;
    TreeView tree;
    ScopedPointer<JucerTreeViewBase> rootItem;

private:
    String opennessStateKey, emptyTreeMessage;
};

//==============================================================================
class TreeItemComponent   : public Component
{
public:
    TreeItemComponent (JucerTreeViewBase& item_)  : item (item_)
    {
        setInterceptsMouseClicks (false, true);
    }

    void paint (Graphics& g)
    {
        g.setColour (Colours::black);
        paintIcon (g);
        item.paintContent (g, Rectangle<int> (item.textX, 0, getWidth() - item.textX, getHeight()));
    }

    void paintIcon (Graphics& g)
    {
        item.getIcon().draw (g, Rectangle<float> (4.0f, 2.0f, item.getIconSize(), getHeight() - 4.0f));
    }

    void resized()
    {
        item.textX = (int) item.getIconSize() + 8;
    }

    JucerTreeViewBase& item;
};


#endif
