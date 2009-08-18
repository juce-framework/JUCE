/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCER_COMPONENTLAYOUT_JUCEHEADER__
#define __JUCER_COMPONENTLAYOUT_JUCEHEADER__


#include "components/jucer_ComponentTypeHandler.h"
class JucerDocument;


//==============================================================================
/**
    Manages the set of sub-components for a JucerDocument.

*/
class ComponentLayout
{
public:
    //==============================================================================
    ComponentLayout();
    ~ComponentLayout();

    //==============================================================================
    void changed();

    int getNumComponents() const throw()                                { return components.size(); }
    Component* getComponent (const int index) const throw()             { return components [index]; }
    int indexOfComponent (Component* const comp) const throw()          { return components.indexOf (comp); }
    bool containsComponent (Component* const comp) const throw()        { return components.contains (comp); }

    //==============================================================================
    void clearComponents();
    void removeComponent (Component* comp, const bool undoable);

    Component* addNewComponent (ComponentTypeHandler* const type, int x, int y);
    Component* addComponentFromXml (const XmlElement& xml, const bool undoable);

    Component* findComponentWithId (const int64 componentId) const;

    //==============================================================================
    void componentToFront (Component* comp, const bool undoable);
    void componentToBack (Component* comp, const bool undoable);

    void setComponentPosition (Component* comp, const RelativePositionedRectangle& newPos, const bool undoable);
    void updateStoredComponentPosition (Component* comp, const bool undoable);

    //==============================================================================
    Component* getComponentRelativePosTarget (Component* comp, int whichDimension) const;
    void setComponentRelativeTarget (Component* comp, int whichDimension, Component* compToBeRelativeTo);
    // checks recursively whether the comp depends on the given comp for its position
    bool dependsOnComponentForRelativePos (Component* comp, Component* possibleDependee) const;

    PopupMenu getRelativeTargetMenu (Component* comp, int whichDimension) const;
    void processRelativeTargetMenuResult (Component* comp, int whichDimension, int menuResultID);

    //==============================================================================
    void setComponentMemberVariableName (Component* comp, const String& newName);
    const String getComponentMemberVariableName (Component* comp) const;

    //==============================================================================
    void setComponentVirtualClassName (Component* comp, const String& newName);
    const String getComponentVirtualClassName (Component* comp) const;

    //==============================================================================
    SelectedItemSet <Component*>& getSelectedSet()                      { return selected; }

    static const tchar* const clipboardXmlTag;
    void copySelectedToClipboard();
    void paste();
    void deleteSelected();
    void selectAll();

    void selectedToFront();
    void selectedToBack();

    void startDragging();
    void dragSelectedComps (int dxFromDragStart, int dyFromDragStart, const bool allowSnap = true);
    void endDragging();

    void moveSelectedComps (int dx, int dy, bool snap);
    void stretchSelectedComps (int dw, int dh, bool allowSnap);

    void bringLostItemsBackOnScreen (int width, int height);

    //==============================================================================
    void setDocument (JucerDocument* const document_)                   { document = document_; }
    JucerDocument* getDocument() const throw()                          { return document; }

    //==============================================================================
    void addToXml (XmlElement& xml) const;

    void fillInGeneratedCode (GeneratedCode& code) const;

    void perform (UndoableAction* action, const String& actionName);

private:
    JucerDocument* document;
    OwnedArray <Component> components;
    SelectedItemSet <Component*> selected;
    int nextCompUID;

    const String getUnusedMemberName (String nameRoot, Component* comp) const;

    friend class FrontBackCompAction;
    friend class DeleteCompAction;
    void moveComponentZOrder (int oldIndex, int newIndex);
};


#endif   // __JUCER_COMPONENTLAYOUT_JUCEHEADER__
