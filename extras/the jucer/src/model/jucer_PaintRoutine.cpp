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

#include "../jucer_Headers.h"
#include "jucer_PaintRoutine.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "paintelements/jucer_PaintElementUndoableAction.h"
#include "paintelements/jucer_PaintElementPath.h"
#include "paintelements/jucer_PaintElementImage.h"
#include "paintelements/jucer_PaintElementGroup.h"
#include "../ui/jucer_JucerDocumentHolder.h"


//==============================================================================
PaintRoutine::PaintRoutine()
    : document (0),
      backgroundColour (Colours::white)
{
    clear();
}

PaintRoutine::~PaintRoutine()
{
    elements.clear(); // do this explicitly before the scalar destructor because these
                      // objects will be listeners on this object
}

//==============================================================================
void PaintRoutine::changed()
{
    if (document != 0)
        document->changed();
}

bool PaintRoutine::perform (UndoableAction* action, const String& actionName)
{
    jassert (document != 0);

    if (document != 0)
    {
        return document->getUndoManager().perform (action, actionName);
    }
    else
    {
        action->perform();
        delete action;
        return false;
    }
}

void PaintRoutine::setBackgroundColour (const Colour& newColour) throw()
{
    backgroundColour = newColour;
    changed();
}

void PaintRoutine::clear()
{
    if (elements.size() > 0)
    {
        elements.clear();
        changed();
    }
}

//==============================================================================
class AddXmlElementAction   : public UndoableAction
{
public:
    AddXmlElementAction (PaintRoutine& routine_, XmlElement* xml_)
        : routine (routine_),
          xml (xml_)
    {
    }

    ~AddXmlElementAction()
    {
        delete xml;
    }

    bool perform()
    {
        showCorrectTab();
        PaintElement* newElement = routine.addElementFromXml (*xml, -1, false);
        jassert (newElement != 0);

        indexAdded = routine.indexOfElement (newElement);
        jassert (indexAdded >= 0);
        return indexAdded >= 0;
    }

    bool undo()
    {
        showCorrectTab();
        routine.removeElement (routine.getElement (indexAdded), false);
        return true;
    }

    int getSizeInUnits()    { return 10; }

    int indexAdded;

private:
    PaintRoutine& routine;
    XmlElement* xml;

    void showCorrectTab() const
    {
        JucerDocumentHolder* const docHolder = JucerDocumentHolder::getActiveDocumentHolder();

        if (docHolder != 0)
            docHolder->showGraphics (&routine);
    }

    AddXmlElementAction (const AddXmlElementAction&);
    const AddXmlElementAction& operator= (const AddXmlElementAction&);
};

PaintElement* PaintRoutine::addElementFromXml (const XmlElement& xml, const int index, const bool undoable)
{
    selectedPoints.deselectAll();

    if (undoable)
    {
        AddXmlElementAction* action = new AddXmlElementAction (*this, new XmlElement (xml));
        perform (action, T("Add new element"));

        return elements [action->indexAdded];
    }
    else
    {
        PaintElement* const newElement = ObjectTypes::createElementForXml (&xml, this);

        if (newElement != 0)
        {
            elements.insert (index, newElement);
            changed();

            return newElement;
        }
    }

    return 0;
}

PaintElement* PaintRoutine::addNewElement (PaintElement* e, const int index, const bool undoable)
{
    if (e != 0)
    {
        XmlElement* const xml = e->createXml();
        delete e;

        e = addElementFromXml (*xml, index, undoable);

        delete xml;
    }

    return e;
}

//==============================================================================
class DeleteElementAction   : public PaintElementUndoableAction <PaintElement>
{
public:
    DeleteElementAction (PaintElement* const element)
        : PaintElementUndoableAction <PaintElement> (element),
          oldIndex (-1)
    {
        xml = element->createXml();
        oldIndex = routine.indexOfElement (element);
    }

    ~DeleteElementAction()
    {
        delete xml;
    }

    bool perform()
    {
        showCorrectTab();
        routine.removeElement (getElement(), false);
        return true;
    }

    bool undo()
    {
        PaintElement* newElement = routine.addElementFromXml (*xml, oldIndex, false);
        showCorrectTab();
        return newElement != 0;
    }

    int getSizeInUnits()    { return 10; }

private:
    XmlElement* xml;
    int oldIndex;
};


void PaintRoutine::removeElement (PaintElement* element, const bool undoable)
{
    if (elements.contains (element))
    {
        if (undoable)
        {
            perform (new DeleteElementAction (element),
                     T("Delete ") + element->getTypeName());
        }
        else
        {
            selectedElements.deselect (element);
            selectedPoints.deselectAll();

            selectedPoints.changed (true);
            selectedElements.changed (true);

            elements.removeObject (element);
            changed();
        }
    }
}

//==============================================================================
class FrontOrBackElementAction  : public PaintElementUndoableAction <PaintElement>
{
public:
    FrontOrBackElementAction (PaintElement* const element, int newIndex_)
        : PaintElementUndoableAction <PaintElement> (element),
          newIndex (newIndex_)
    {
        oldIndex = routine.indexOfElement (element);
    }

    bool perform()
    {
        showCorrectTab();

        PaintElement* e = routine.getElement (oldIndex);
        routine.moveElementZOrder (oldIndex, newIndex);
        newIndex = routine.indexOfElement (e);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        routine.moveElementZOrder (newIndex, oldIndex);
        return true;
    }

private:
    int newIndex, oldIndex;
};

void PaintRoutine::moveElementZOrder (int oldIndex, int newIndex)
{
    jassert (elements [oldIndex] != 0);

    if (oldIndex != newIndex && elements [oldIndex] != 0)
    {
        elements.move (oldIndex, newIndex);
        changed();
    }
}

void PaintRoutine::elementToFront (PaintElement* element, const bool undoable)
{
    if (element != 0 && elements.contains (element))
    {
        if (undoable)
            perform (new FrontOrBackElementAction (element, -1), T("Move elements to front"));
        else
            moveElementZOrder (elements.indexOf (element), -1);
    }
}

void PaintRoutine::elementToBack (PaintElement* element, const bool undoable)
{
    if (element != 0 && elements.contains (element))
    {
        if (undoable)
            perform (new FrontOrBackElementAction (element, 0), T("Move elements to back"));
        else
            moveElementZOrder (elements.indexOf (element), 0);
    }
}

//==============================================================================
const tchar* const PaintRoutine::clipboardXmlTag = T("PAINTELEMENTS");

void PaintRoutine::copySelectedToClipboard()
{
    if (selectedElements.getNumSelected() == 0)
        return;

    XmlElement clip (clipboardXmlTag);

    for (int i = 0; i < elements.size(); ++i)
    {
        PaintElement* const pe = elements.getUnchecked(i);

        if (selectedElements.isSelected (pe))
        {
            XmlElement* const e = pe->createXml();
            clip.addChildElement (e);
        }
    }

    SystemClipboard::copyTextToClipboard (clip.createDocument (String::empty, false, false));
}

void PaintRoutine::paste()
{
    XmlDocument clip (SystemClipboard::getTextFromClipboard());
    XmlElement* const doc = clip.getDocumentElement();

    if (doc != 0 && doc->hasTagName (clipboardXmlTag))
    {
        selectedElements.deselectAll();
        selectedPoints.deselectAll();

        forEachXmlChildElement (*doc, e)
        {
            PaintElement* newElement = addElementFromXml (*e, -1, true);

            if (newElement != 0)
                selectedElements.addToSelection (newElement);
        }
    }

    delete doc;
}

void PaintRoutine::deleteSelected()
{
    const SelectedItemSet <PaintElement*> temp1 (selectedElements);
    const SelectedItemSet <PathPoint*> temp2 (selectedPoints);

    if (temp2.getNumSelected() > 0)
    {
        selectedPoints.deselectAll();
        selectedPoints.changed (true); // synchronous message to get rid of any property components

        // if any points are selected, just delete them, and not the element, which may
        // also be selected..
        for (int i = temp2.getNumSelected(); --i >= 0;)
            temp2.getSelectedItem (i)->deleteFromPath();

        changed();
    }
    else if (temp1.getNumSelected() > 0)
    {
        selectedElements.deselectAll();
        selectedElements.changed (true);

        for (int i = temp1.getNumSelected(); --i >= 0;)
            removeElement (temp1.getSelectedItem (i), true);

        changed();
    }
}

void PaintRoutine::selectAll()
{
    if (selectedPoints.getNumSelected() > 0)
    {
        PaintElementPath* path = selectedPoints.getSelectedItem (0)->owner;

        if (path != 0)
        {
            for (int i = 0; i < path->getNumPoints(); ++i)
                selectedPoints.addToSelection (path->getPoint (i));
        }
    }
    else
    {
        for (int i = 0; i < elements.size(); ++i)
            selectedElements.addToSelection (elements.getUnchecked (i));
    }
}

void PaintRoutine::selectedToFront()
{
    const SelectedItemSet <PaintElement*> temp (selectedElements);

    for (int i = temp.getNumSelected(); --i >= 0;)
        elementToFront (temp.getSelectedItem(i), true);
}

void PaintRoutine::selectedToBack()
{
    const SelectedItemSet <PaintElement*> temp (selectedElements);

    for (int i = 0; i < temp.getNumSelected(); ++i)
        elementToBack (temp.getSelectedItem(i), true);
}

void PaintRoutine::groupSelected()
{
    PaintElementGroup::groupSelected (this);
}

void PaintRoutine::ungroupSelected()
{
    const SelectedItemSet <PaintElement*> temp (selectedElements);

    for (int i = 0; i < temp.getNumSelected(); ++i)
    {
        PaintElementGroup* const pg = dynamic_cast <PaintElementGroup*> (temp.getSelectedItem (i));

        if (pg != 0)
            pg->ungroup (true);
    }
}

void PaintRoutine::bringLostItemsBackOnScreen (const Rectangle& parentArea)
{
    for (int i = 0; i < elements.size(); ++i)
    {
        PaintElement* const c = elements[i];

        Rectangle r (c->getCurrentBounds (parentArea));

        if (! r.intersects (parentArea))
        {
            r.setPosition (parentArea.getCentreX(), parentArea.getCentreY());
            c->setCurrentBounds (r, parentArea, true);
        }
    }
}

void PaintRoutine::startDragging (const Rectangle& parentArea)
{
    for (int i = 0; i < elements.size(); ++i)
    {
        PaintElement* const c = elements[i];

        Rectangle r (c->getCurrentBounds (parentArea));

        c->setComponentProperty (T("xDragStart"), r.getX());
        c->setComponentProperty (T("yDragStart"), r.getY());
    }

    getDocument()->getUndoManager().beginNewTransaction();
}

void PaintRoutine::dragSelectedComps (int dx, int dy, const Rectangle& parentArea)
{
    getDocument()->getUndoManager().undoCurrentTransactionOnly();

    if (document != 0 && selectedElements.getNumSelected() > 1)
    {
        dx = document->snapPosition (dx);
        dy = document->snapPosition (dy);
    }

    for (int i = 0; i < selectedElements.getNumSelected(); ++i)
    {
        PaintElement* const c = selectedElements.getSelectedItem (i);

        const int startX = c->getComponentPropertyInt (T("xDragStart"), false);
        const int startY = c->getComponentPropertyInt (T("yDragStart"), false);

        Rectangle r (c->getCurrentBounds (parentArea));

        if (document != 0 && selectedElements.getNumSelected() == 1)
        {
            r.setPosition (document->snapPosition (startX + dx),
                           document->snapPosition (startY + dy));
        }
        else
        {
            r.setPosition (startX + dx,
                           startY + dy);
        }

        c->setCurrentBounds (r, parentArea, true);
    }

    changed();
}

void PaintRoutine::endDragging()
{
    getDocument()->getUndoManager().beginNewTransaction();
}

//==============================================================================
void PaintRoutine::fillWithBackground (Graphics& g, const bool drawOpaqueBackground)
{
    if ((! backgroundColour.isOpaque()) && drawOpaqueBackground)
    {
        g.fillCheckerBoard (0, 0, g.getClipBounds().getRight(), g.getClipBounds().getBottom(),
                            50, 50,
                            Colour (0xffdddddd).overlaidWith (backgroundColour),
                            Colour (0xffffffff).overlaidWith (backgroundColour));
    }
    else
    {
        g.fillAll (backgroundColour);
    }
}

void PaintRoutine::drawElements (Graphics& g, const Rectangle& relativeTo)
{
    Component temp;
    temp.setBounds (relativeTo);

    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked (i)->draw (g, getDocument()->getComponentLayout(), relativeTo);
}

//==============================================================================
void PaintRoutine::dropImageAt (const File& f, int x, int y)
{
    Drawable* d = Drawable::createFromImageFile (f);

    if (d != 0)
    {
        float ix, iy, iw, ih;
        d->getBounds (ix, iy, iw, ih);
        delete d;

        PaintElement* newElement
            = addNewElement (ObjectTypes::createNewImageElement (this), -1, true);

        PaintElementImage* pei = dynamic_cast <PaintElementImage*> (newElement);

        if (pei != 0)
        {
            String resourceName (getDocument()->getResources().findUniqueName (f.getFileName()));

            const BinaryResources::BinaryResource* existingResource = getDocument()->getResources().getResourceForFile (f);

            if (existingResource != 0)
            {
                resourceName = existingResource->name;
            }
            else
            {
                MemoryBlock data;
                f.loadFileAsData (data);

                getDocument()->getResources().add (resourceName, f.getFullPathName(), data);
            }

            pei->setResource (resourceName, true);

            const int imageW = (int) (ix + iw + 1.0f);
            const int imageH = (int) (iy + ih + 1.0f);

            RelativePositionedRectangle pr;
            pr.rect.setX (x - imageW / 2);
            pr.rect.setY (y - imageH / 2);
            pr.rect.setWidth (imageW);
            pr.rect.setHeight (imageH);

            pei->setPosition (pr, true);

            getSelectedElements().selectOnly (pei);
        }
    }
}

//==============================================================================
const tchar* PaintRoutine::xmlTagName = T("BACKGROUND");

XmlElement* PaintRoutine::createXml() const
{
    XmlElement* const xml = new XmlElement (xmlTagName);

    xml->setAttribute (T("backgroundColour"), colourToHex (backgroundColour));

    for (int i = 0; i < elements.size(); ++i)
        xml->addChildElement (elements.getUnchecked (i)->createXml());

    return xml;
}

bool PaintRoutine::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (xmlTagName))
    {
        backgroundColour = Colour (xml.getStringAttribute (T("backgroundColour"), colourToHex (Colours::white)).getHexValue32());

        clear();

        forEachXmlChildElement (xml, e)
        {
            PaintElement* const newElement = ObjectTypes::createElementForXml (e, this);

            if (newElement != 0)
                elements.add (newElement);
        }

        return true;
    }

    return false;
}

void PaintRoutine::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const
{
    if (! backgroundColour.isTransparent())
        paintMethodCode << "g.fillAll (" << colourToCode (backgroundColour) << ");\n\n";

    for (int i = 0; i < elements.size(); ++i)
        elements[i]->fillInGeneratedCode (code, paintMethodCode);
}
