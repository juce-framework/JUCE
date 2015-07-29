/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#ifndef JUCER_PAINTROUTINE_H_INCLUDED
#define JUCER_PAINTROUTINE_H_INCLUDED

#include "paintelements/jucer_PaintElement.h"
class JucerDocument;
class PathPoint;


//==============================================================================
/**
    Contains a set of PaintElements that constitute some kind of paint() method.

*/
class PaintRoutine
{
public:
    //==============================================================================
    PaintRoutine();
    ~PaintRoutine();

    //==============================================================================
    void changed();
    bool perform (UndoableAction* action, const String& actionName);

    //==============================================================================
    int getNumElements() const noexcept                                     { return elements.size(); }
    PaintElement* getElement (const int index) const noexcept               { return elements [index]; }
    int indexOfElement (PaintElement* e) const noexcept                     { return elements.indexOf (e); }
    bool containsElement (PaintElement* e) const noexcept                   { return elements.contains (e); }

    //==============================================================================
    void clear();
    PaintElement* addElementFromXml (const XmlElement& xml, const int index, const bool undoable);
    PaintElement* addNewElement (PaintElement* elementToCopy, const int index, const bool undoable);
    void removeElement (PaintElement* element, const bool undoable);

    void elementToFront (PaintElement* element, const bool undoable);
    void elementToBack (PaintElement* element, const bool undoable);

    const Colour getBackgroundColour() const noexcept                       { return backgroundColour; }
    void setBackgroundColour (Colour newColour) noexcept;

    void fillWithBackground (Graphics& g, const bool drawOpaqueBackground);
    void drawElements (Graphics& g, const Rectangle<int>& relativeTo);

    void dropImageAt (const File& f, int x, int y);

    //==============================================================================
    SelectedItemSet <PaintElement*>& getSelectedElements() noexcept         { return selectedElements; }
    SelectedItemSet <PathPoint*>& getSelectedPoints() noexcept              { return selectedPoints; }

    static const char* const clipboardXmlTag;
    void copySelectedToClipboard();
    void paste();
    void deleteSelected();
    void selectAll();

    void selectedToFront();
    void selectedToBack();

    void groupSelected();
    void ungroupSelected();

    void startDragging (const Rectangle<int>& parentArea);
    void dragSelectedComps (int dxFromDragStart, int dyFromDragStart, const Rectangle<int>& parentArea);
    void endDragging();

    void bringLostItemsBackOnScreen (const Rectangle<int>& parentArea);

    //==============================================================================
    void setDocument (JucerDocument* const doc)                       { document = doc; }
    JucerDocument* getDocument() const noexcept                       { return document; }

    //==============================================================================
    static const char* xmlTagName;
    XmlElement* createXml() const;
    bool loadFromXml (const XmlElement& xml);

    void fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const;

    //==============================================================================
private:
    OwnedArray <PaintElement> elements;
    SelectedItemSet <PaintElement*> selectedElements;
    SelectedItemSet <PathPoint*> selectedPoints;
    JucerDocument* document;

    Colour backgroundColour;

    friend class DeleteElementAction;
    friend class FrontOrBackElementAction;
    void moveElementZOrder (int oldIndex, int newIndex);
};

#endif   // JUCER_PAINTROUTINE_H_INCLUDED
