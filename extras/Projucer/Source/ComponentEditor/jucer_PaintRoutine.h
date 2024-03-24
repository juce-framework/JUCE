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

#include "PaintElements/jucer_PaintElement.h"
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
    PaintElement* getElement (int index) const noexcept                     { return elements [index]; }
    int indexOfElement (PaintElement* e) const noexcept                     { return elements.indexOf (e); }
    bool containsElement (PaintElement* e) const noexcept                   { return elements.contains (e); }

    //==============================================================================
    void clear();
    PaintElement* addElementFromXml (const XmlElement& xml, int index, bool undoable);
    PaintElement* addNewElement (PaintElement* elementToCopy, int index, bool undoable);
    void removeElement (PaintElement* element, bool undoable);

    void elementToFront (PaintElement* element, bool undoable);
    void elementToBack (PaintElement* element, bool undoable);

    Colour getBackgroundColour() const noexcept                       { return backgroundColour; }
    void setBackgroundColour (Colour newColour) noexcept;

    void fillWithBackground (Graphics& g, bool drawOpaqueBackground);
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
    void moveElementZOrder (int oldIndex, int newIndex);

private:
    OwnedArray<PaintElement> elements;
    SelectedItemSet <PaintElement*> selectedElements;
    SelectedItemSet <PathPoint*> selectedPoints;
    JucerDocument* document;

    Colour backgroundColour;
};
