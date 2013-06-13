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

#ifndef __JUCER_TREEVIEWTYPES_JUCEHEADER__
#define __JUCER_TREEVIEWTYPES_JUCEHEADER__

#include "jucer_ProjectTreeViewBase.h"


//==============================================================================
class SourceFileTreeViewItem   : public ProjectTreeViewBase
{
public:
    SourceFileTreeViewItem (const Project::Item& item);
    ~SourceFileTreeViewItem();

    bool acceptsFileDrop (const StringArray&) const                     { return false; }
    bool acceptsDragItems (const OwnedArray <Project::Item>&)           { return false; }
    ProjectTreeViewBase* createSubItem (const Project::Item& child);
    void showDocument();
    void showPopupMenu();
    void handlePopupMenuResult (int resultCode);
    String getDisplayName() const;
    void setName (const String& newName);
};

//==============================================================================
class GroupTreeViewItem   : public ProjectTreeViewBase
{
public:
    GroupTreeViewItem (const Project::Item& item);
    ~GroupTreeViewItem();

    bool isRoot() const                                                 { return item.isMainGroup(); }
    bool acceptsFileDrop (const StringArray&) const                     { return true; }
    bool acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes);
    void checkFileStatus();
    void moveSelectedItemsTo (OwnedArray <Project::Item>& selectedNodes, int insertIndex);
    ProjectTreeViewBase* createSubItem (const Project::Item& child);
    void showDocument();
    void showPopupMenu();
    void handlePopupMenuResult (int resultCode);

    void addFiles (const StringArray& files, int insertIndex);
    void addNewGroup();

    void addCreateFileMenuItems (PopupMenu& m);
    void processCreateFileMenuItem (int item);
};


#endif   // __JUCER_TREEVIEWTYPES_JUCEHEADER__
