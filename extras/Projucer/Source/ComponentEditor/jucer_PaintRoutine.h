/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

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

    void alignTop();
    void alignRight();
    void alignBottom();
    void alignLeft();

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

    void applyCustomPaintSnippets (StringArray&);

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
