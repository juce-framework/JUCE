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

#ifndef __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__
#define __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__

#include "../jucer_Headers.h"
#include "../Utility/jucer_JucerTreeViewBase.h"
#include "jucer_Project.h"


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
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property) override;
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree& childWhichHasBeenAdded) override;
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved) override;
    void valueTreeChildOrderChanged (ValueTree& parentTree) override;
    void valueTreeParentChanged (ValueTree& tree) override;

    //==============================================================================
    bool mightContainSubItems() override;
    String getUniqueName() const override;
    void itemOpennessChanged (bool isNowOpen) override;
    bool canBeSelected() const override                  { return true; }
    String getTooltip() override;
    var getDragSourceDescription() override;
    void addSubItems() override;

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files) override;
    void filesDropped (const StringArray& files, int insertIndex) override;
    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override;
    int getMillisecsAllowedForDragGesture() override;

    static void getAllSelectedNodesInTree (Component* componentInTree, OwnedArray <Project::Item>& selectedNodes);

    File getDraggableFile() const override      { return getFile(); }

    //==============================================================================
    Project::Item item;

protected:
    bool isFileMissing;

    //==============================================================================
    void treeChildrenChanged (const ValueTree& parentTree);
    virtual ProjectTreeViewBase* createSubItem (const Project::Item& node) = 0;

    Icon getIcon() const override           { return item.getIcon().withContrastingColourTo (getBackgroundColour()); }
    bool isIconCrossedOut() const override  { return item.isIconCrossedOut(); }

    //==============================================================================
    void triggerAsyncRename (const Project::Item& itemToRename);
    static void moveItems (OwnedArray <Project::Item>& selectedNodes,
                           Project::Item destNode, int insertIndex);

    ProjectTreeViewBase* getParentProjectItem() const;
};


#endif   // __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__
