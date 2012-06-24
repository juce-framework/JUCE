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

    //==============================================================================
    virtual Font getFont() const;
    virtual String getRenamingName() const = 0;
    virtual String getDisplayName() const = 0;
    virtual void setName (const String& newName) = 0;
    virtual bool isMissing() = 0;
    virtual const Drawable* getIcon() const = 0;
    virtual void createLeftEdgeComponents (OwnedArray<Component>&) {}
    virtual Component* createRightEdgeComponent()      { return nullptr; }
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
    void cancelDelayedSelectionTimer();
    virtual void addSubItems() {}

private:
    class ItemSelectionTimer;
    friend class ItemSelectionTimer;
    ScopedPointer<Timer> delayedSelectionTimer;

    void invokeShowDocument();
};


#endif   // __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__
