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

#ifndef __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__
#define __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__

#include "../jucer_Headers.h"


//==============================================================================
class JucerTreeViewBase   : public TreeViewItem,
                            public TextEditorListener
{
protected:
    //==============================================================================
    JucerTreeViewBase();
    ~JucerTreeViewBase();

public:
    //==============================================================================
    int getItemWidth() const                                { return -1; }
    int getItemHeight() const                               { return 20; }

    void paintItem (Graphics& g, int width, int height);
    void paintOpenCloseButton (Graphics& g, int width, int height, bool isMouseOver);
    Component* createItemComponent();
    void itemClicked (const MouseEvent& e);

    //==============================================================================
    virtual const String getRenamingName() const = 0;
    virtual const String getDisplayName() const = 0;
    virtual void setName (const String& newName) = 0;
    virtual bool isMissing() = 0;
    virtual const Drawable* getIcon() const = 0;
    virtual void createLeftEdgeComponents (Array<Component*>& components) = 0;

    virtual void showPopupMenu();
    virtual void showMultiSelectionPopupMenu();

    virtual void showRenameBox();

    // Text editor listener for renaming..
    void textEditorTextChanged (TextEditor& editor)         {}
    void textEditorReturnKeyPressed (TextEditor& editor)    { editor.exitModalState (1); }
    void textEditorEscapeKeyPressed (TextEditor& editor)    { editor.exitModalState (0); }
    void textEditorFocusLost (TextEditor& editor)           { editor.exitModalState (0); }

    //==============================================================================
private:
    int numLeftHandComps;
    const Font getFont() const;
    int getTextX() const;
};


#endif   // __JUCER_JUCERTREEVIEWBASE_JUCEHEADER__
