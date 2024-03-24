/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "Components/jucer_ComponentTypeHandler.h"
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

    int getNumComponents() const noexcept                                { return components.size(); }
    Component* getComponent (const int index) const noexcept             { return components [index]; }
    int indexOfComponent (Component* const comp) const noexcept          { return components.indexOf (comp); }
    bool containsComponent (Component* const comp) const noexcept        { return components.contains (comp); }

    //==============================================================================
    void clearComponents();
    void removeComponent (Component* comp, bool undoable);

    Component* addNewComponent (ComponentTypeHandler* const type, int x, int y);
    Component* addComponentFromXml (const XmlElement& xml, bool undoable);

    Component* findComponentWithId (int64 componentId) const;

    //==============================================================================
    void componentToFront (Component* comp, bool undoable);
    void componentToBack (Component* comp, bool undoable);

    void setComponentPosition (Component* comp, const RelativePositionedRectangle& newPos, bool undoable);
    void setComponentBoundsAndProperties (Component* comp, const Rectangle<int>& newBounds, Component* referenceComponent, bool undoable);
    void updateStoredComponentPosition (Component* comp, bool undoable);

    //==============================================================================
    Component* getComponentRelativePosTarget (Component* comp, int whichDimension) const;
    void setComponentRelativeTarget (Component* comp, int whichDimension, Component* compToBeRelativeTo);
    // checks recursively whether the comp depends on the given comp for its position
    bool dependsOnComponentForRelativePos (Component* comp, Component* possibleDependee) const;
    bool isComponentPositionRelative (Component* comp) const;

    PopupMenu getRelativeTargetMenu (Component* comp, int whichDimension) const;
    void processRelativeTargetMenuResult (Component* comp, int whichDimension, int menuResultID);

    //==============================================================================
    void setComponentMemberVariableName (Component* comp, const String& newName);
    String getComponentMemberVariableName (Component* comp) const;

    //==============================================================================
    void setComponentVirtualClassName (Component* comp, const String& newName);
    String getComponentVirtualClassName (Component* comp) const;

    //==============================================================================
    SelectedItemSet <Component*>& getSelectedSet()                      { return selected; }

    static const char* const clipboardXmlTag;
    void copySelectedToClipboard();
    void paste();
    void deleteSelected();
    void selectAll();

    void selectedToFront();
    void selectedToBack();

    void alignTop();
    void alignRight();
    void alignBottom();
    void alignLeft();

    void startDragging();
    void dragSelectedComps (int dxFromDragStart, int dyFromDragStart, const bool allowSnap = true);
    void endDragging();

    void moveSelectedComps (int dx, int dy, bool snap);
    void stretchSelectedComps (int dw, int dh, bool allowSnap);

    void bringLostItemsBackOnScreen (int width, int height);

    //==============================================================================
    void setDocument (JucerDocument* const doc)                   { document = doc; }
    JucerDocument* getDocument() const noexcept                   { return document; }

    //==============================================================================
    void addToXml (XmlElement& xml) const;

    void fillInGeneratedCode (GeneratedCode& code) const;

    void perform (std::unique_ptr<UndoableAction> action, const String& actionName);

    void moveComponentZOrder (int oldIndex, int newIndex);

private:
    JucerDocument* document;
    OwnedArray<Component> components;
    SelectedItemSet <Component*> selected;
    int nextCompUID;

    int addComponentIndexAdded = 0;

    String getUnusedMemberName (String nameRoot, Component* comp) const;
};

void positionToCode (const RelativePositionedRectangle& position,
                     const ComponentLayout* layout,
                     String& x, String& y, String& w, String& h);
