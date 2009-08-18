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

#ifndef __JUCER_PAINTROUTINE_JUCEHEADER__
#define __JUCER_PAINTROUTINE_JUCEHEADER__

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
    int getNumElements() const throw()                                      { return elements.size(); }
    PaintElement* getElement (const int index) const throw()                { return elements [index]; }
    int indexOfElement (PaintElement* e) const throw()                      { return elements.indexOf (e); }
    bool containsElement (PaintElement* e) const throw()                    { return elements.contains (e); }

    //==============================================================================
    void clear();
    PaintElement* addElementFromXml (const XmlElement& xml, const int index, const bool undoable);
    PaintElement* addNewElement (PaintElement* elementToCopy, const int index, const bool undoable);
    void removeElement (PaintElement* element, const bool undoable);

    void elementToFront (PaintElement* element, const bool undoable);
    void elementToBack (PaintElement* element, const bool undoable);

    const Colour getBackgroundColour() const throw()                        { return backgroundColour; }
    void setBackgroundColour (const Colour& newColour) throw();

    void fillWithBackground (Graphics& g, const bool drawOpaqueBackground);
    void drawElements (Graphics& g, const Rectangle& relativeTo);

    void dropImageAt (const File& f, int x, int y);

    //==============================================================================
    SelectedItemSet <PaintElement*>& getSelectedElements() throw()          { return selectedElements; }
    SelectedItemSet <PathPoint*>& getSelectedPoints() throw()               { return selectedPoints; }

    static const tchar* const clipboardXmlTag;
    void copySelectedToClipboard();
    void paste();
    void deleteSelected();
    void selectAll();

    void selectedToFront();
    void selectedToBack();

    void groupSelected();
    void ungroupSelected();

    void startDragging (const Rectangle& parentArea);
    void dragSelectedComps (int dxFromDragStart, int dyFromDragStart, const Rectangle& parentArea);
    void endDragging();

    void bringLostItemsBackOnScreen (const Rectangle& parentArea);

    //==============================================================================
    void setDocument (JucerDocument* const document_)                       { document = document_; }
    JucerDocument* getDocument() const throw()                              { return document; }

    //==============================================================================
    static const tchar* xmlTagName;
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

#endif   // __JUCER_PAINTROUTINE_JUCEHEADER__
