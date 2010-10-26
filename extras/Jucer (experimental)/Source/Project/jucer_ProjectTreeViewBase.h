/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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
#include "jucer_ResourceFile.h"
#include "jucer_ProjectContentComponent.h"


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
    virtual bool acceptsFileDrop (const StringArray& files) const         = 0;
    virtual bool acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes) = 0;

    //==============================================================================
    virtual const String getDisplayName() const;
    virtual const String getRenamingName() const        { return getDisplayName(); }
    virtual void setName (const String& newName);
    virtual bool isMissing()                            { return isFileMissing; }
    virtual const File getFile() const;

    virtual void deleteItem();
    virtual void deleteAllSelectedItems();
    virtual void revealInFinder() const;
    virtual void showDocument() = 0;
    virtual void browseToAddExistingFiles();
    virtual void checkFileStatus();  // (recursive)

    virtual void addFiles (const StringArray& files, int insertIndex);
    virtual void moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex);
    virtual void showMultiSelectionPopupMenu();

    virtual ProjectTreeViewBase* findTreeViewItem (const Project::Item& itemToFind);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const Identifier& property);
    void valueTreeChildrenChanged (ValueTree& tree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    // TreeViewItem stuff..
    bool mightContainSubItems();
    const String getUniqueName() const;
    void itemOpennessChanged (bool isNowOpen);
    void refreshSubItems();
    bool canBeSelected() const                  { return true; }
    void itemDoubleClicked (const MouseEvent& e);
    void itemSelectionChanged (bool isNowSelected);
    const String getTooltip();
    const String getDragSourceDescription();

    //==============================================================================
    // Drag-and-drop stuff..
    bool isInterestedInFileDrag (const StringArray& files);
    void filesDropped (const StringArray& files, int insertIndex);
    bool isInterestedInDragSource (const String& sourceDescription, Component* sourceComponent);
    void itemDropped (const String& sourceDescription, Component* sourceComponent, int insertIndex);

    //==============================================================================
    Project::Item item;

protected:
    bool isFileMissing;

    //==============================================================================
    virtual void addSubItems();
    virtual ProjectTreeViewBase* createSubItem (const Project::Item& node) = 0;
    const Drawable* getIcon() const         { return item.getIcon(); }

    //==============================================================================
    void triggerAsyncRename (const Project::Item& itemToRename);
    static void moveItems (OwnedArray <Project::Item>& selectedNodes,
                           Project::Item destNode, int insertIndex);

    ProjectContentComponent* getProjectContentComponent() const;
    ProjectTreeViewBase* getParentProjectItem() const;
};


#endif   // __JUCER_PROJECTTREEVIEWBASE_JUCEHEADER__
