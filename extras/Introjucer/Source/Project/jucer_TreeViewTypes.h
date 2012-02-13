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

#ifndef __JUCER_TREEVIEWTYPES_JUCEHEADER__
#define __JUCER_TREEVIEWTYPES_JUCEHEADER__

#include "jucer_ProjectTreeViewBase.h"


//==============================================================================
class SourceFileTreeViewItem   : public ProjectTreeViewBase
{
public:
    SourceFileTreeViewItem (const Project::Item& item);
    ~SourceFileTreeViewItem();

    bool acceptsFileDrop (const StringArray& files) const                   { return false; }
    bool acceptsDragItems (const OwnedArray <Project::Item>& selectedNodes) { return false; }
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
    bool acceptsFileDrop (const StringArray& files) const               { return true; }
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
