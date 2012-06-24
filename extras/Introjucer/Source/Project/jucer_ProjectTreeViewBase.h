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

#ifndef __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__
#define __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Utility/jucer_JucerTreeViewBase.h"
#include "jucer_Project.h"
#include "../Project Saving/jucer_ResourceFile.h"


//==============================================================================
class ProjectTreeViewBase   : public JucerTreeViewBase,
                              public ValueTree::Listener
{
protected:
    //==============================================================================
    ProjectTreeViewBase (const Project::Item& item);
    ~ProjectTreeViewBase();

public:
    //==============================================================================
    virtual bool isRoot() const                         { return false; }
    virtual bool acceptsFileDrop (const StringArray& files) const = 0;
    virtual bool acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes) = 0;

    //==============================================================================
    virtual String getDisplayName() const;
    virtual String getRenamingName() const              { return getDisplayName(); }
    virtual void setName (const String& newName);
    virtual bool isMissing()                            { return isFileMissing; }
    virtual File getFile() const;

    virtual void deleteItem();
    virtual void deleteAllSelectedItems();
    virtual void revealInFinder() const;
    virtual void browseToAddExistingFiles();
    virtual void checkFileStatus();  // (recursive)

    virtual void addFiles (const StringArray& files, int insertIndex);
    virtual void moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex);
    virtual void showMultiSelectionPopupMenu();

    virtual ProjectTreeViewBase* findTreeViewItem (const Project::Item& itemToFind);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property);
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded);
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved);
    void valueTreeChildOrderChanged (ValueTree& parentTree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    bool mightContainSubItems();
    String getUniqueName() const;
    void itemOpennessChanged (bool isNowOpen);
    bool canBeSelected() const                  { return true; }
    String getTooltip();
    var getDragSourceDescription();
    void addSubItems();

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& files, int insertIndex);
    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails);
    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex);
    int getMillisecsAllowedForDragGesture();

    static void getAllSelectedNodesInTree (Component* componentInTree, OwnedArray <Project::Item>& selectedNodes);

    //==============================================================================
    Project::Item item;

protected:
    bool isFileMissing;

    //==============================================================================
    void treeChildrenChanged (const ValueTree& parentTree);
    virtual ProjectTreeViewBase* createSubItem (const Project::Item& node) = 0;
    const Drawable* getIcon() const         { return item.getIcon(); }

    //==============================================================================
    void triggerAsyncRename (const Project::Item& itemToRename);
    static void moveItems (OwnedArray <Project::Item>& selectedNodes,
                           Project::Item destNode, int insertIndex);

    ProjectTreeViewBase* getParentProjectItem() const;
};


#endif   // __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__
