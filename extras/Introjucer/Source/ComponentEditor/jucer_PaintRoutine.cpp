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

#include "../jucer_Headers.h"
#include "jucer_PaintRoutine.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "paintelements/jucer_PaintElementUndoableAction.h"
#include "paintelements/jucer_PaintElementPath.h"
#include "paintelements/jucer_PaintElementImage.h"
#include "paintelements/jucer_PaintElementGroup.h"
#include "ui/jucer_JucerDocumentEditor.h"


//==============================================================================
PaintRoutine::PaintRoutine()
    : document (nullptr),
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
    if (document != nullptr)
        document->changed();
}

bool PaintRoutine::perform (UndoableAction* action, const String& actionName)
{
    if (document != nullptr)
        return document->getUndoManager().perform (action, actionName);

    ScopedPointer<UndoableAction> deleter (action);
    action->perform();
    return false;
}

void PaintRoutine::setBackgroundColour (Colour newColour) noexcept
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
        : routine (routine_), xml (xml_)
    {
    }

    bool perform()
    {
        showCorrectTab();
        PaintElement* newElement = routine.addElementFromXml (*xml, -1, false);
        jassert (newElement != nullptr);

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
    ScopedPointer<XmlElement> xml;

    void showCorrectTab() const
    {
        if (JucerDocumentEditor* const ed = JucerDocumentEditor::getActiveDocumentHolder())
            ed->showGraphics (&routine);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AddXmlElementAction)
};

PaintElement* PaintRoutine::addElementFromXml (const XmlElement& xml, const int index, const bool undoable)
{
    selectedPoints.deselectAll();

    if (undoable && document != nullptr)
    {
        AddXmlElementAction* action = new AddXmlElementAction (*this, new XmlElement (xml));
        document->getUndoManager().perform (action, "Add new element");

        return elements [action->indexAdded];
    }

    if (PaintElement* const newElement = ObjectTypes::createElementForXml (&xml, this))
    {
        elements.insert (index, newElement);
        changed();

        return newElement;
    }

    return nullptr;
}

PaintElement* PaintRoutine::addNewElement (PaintElement* e, const int index, const bool undoable)
{
    if (e != nullptr)
    {
        ScopedPointer<PaintElement> deleter (e);
        ScopedPointer<XmlElement> xml (e->createXml());

        e = addElementFromXml (*xml, index, undoable);
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
        return newElement != nullptr;
    }

    int getSizeInUnits()    { return 10; }

private:
    ScopedPointer<XmlElement> xml;
    int oldIndex;
};


void PaintRoutine::removeElement (PaintElement* element, const bool undoable)
{
    if (elements.contains (element))
    {
        if (undoable)
        {
            perform (new DeleteElementAction (element),
                     "Delete " + element->getTypeName());
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
    jassert (elements [oldIndex] != nullptr);

    if (oldIndex != newIndex && elements [oldIndex] != nullptr)
    {
        elements.move (oldIndex, newIndex);
        changed();
    }
}

void PaintRoutine::elementToFront (PaintElement* element, const bool undoable)
{
    if (element != nullptr && elements.contains (element))
    {
        if (undoable)
            perform (new FrontOrBackElementAction (element, -1), "Move elements to front");
        else
            moveElementZOrder (elements.indexOf (element), -1);
    }
}

void PaintRoutine::elementToBack (PaintElement* element, const bool undoable)
{
    if (element != nullptr && elements.contains (element))
    {
        if (undoable)
            perform (new FrontOrBackElementAction (element, 0), "Move elements to back");
        else
            moveElementZOrder (elements.indexOf (element), 0);
    }
}

//==============================================================================
const char* const PaintRoutine::clipboardXmlTag = "PAINTELEMENTS";

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

    SystemClipboard::copyTextToClipboard (clip.createDocument ("", false, false));
}

void PaintRoutine::paste()
{
    XmlDocument clip (SystemClipboard::getTextFromClipboard());
    ScopedPointer<XmlElement> doc (clip.getDocumentElement());

    if (doc != nullptr && doc->hasTagName (clipboardXmlTag))
    {
        selectedElements.deselectAll();
        selectedPoints.deselectAll();

        forEachXmlChildElement (*doc, e)
            if (PaintElement* newElement = addElementFromXml (*e, -1, true))
                selectedElements.addToSelection (newElement);
    }
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
        if (const PaintElementPath* path = selectedPoints.getSelectedItem (0)->owner)
            for (int i = 0; i < path->getNumPoints(); ++i)
                selectedPoints.addToSelection (path->getPoint (i));
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
    const SelectedItemSet<PaintElement*> temp (selectedElements);

    for (int i = 0; i < temp.getNumSelected(); ++i)
        if (PaintElementGroup* const pg = dynamic_cast<PaintElementGroup*> (temp.getSelectedItem (i)))
            pg->ungroup (true);
}

void PaintRoutine::bringLostItemsBackOnScreen (const Rectangle<int>& parentArea)
{
    for (int i = 0; i < elements.size(); ++i)
    {
        PaintElement* const c = elements[i];

        Rectangle<int> r (c->getCurrentBounds (parentArea));

        if (! r.intersects (parentArea))
        {
            r.setPosition (parentArea.getCentreX(), parentArea.getCentreY());
            c->setCurrentBounds (r, parentArea, true);
        }
    }
}

void PaintRoutine::startDragging (const Rectangle<int>& parentArea)
{
    for (int i = 0; i < elements.size(); ++i)
    {
        PaintElement* const c = elements[i];

        Rectangle<int> r (c->getCurrentBounds (parentArea));

        c->getProperties().set ("xDragStart", r.getX());
        c->getProperties().set ("yDragStart", r.getY());
    }

    getDocument()->beginTransaction();
}

void PaintRoutine::dragSelectedComps (int dx, int dy, const Rectangle<int>& parentArea)
{
    getDocument()->getUndoManager().undoCurrentTransactionOnly();

    if (document != nullptr && selectedElements.getNumSelected() > 1)
    {
        dx = document->snapPosition (dx);
        dy = document->snapPosition (dy);
    }

    for (int i = 0; i < selectedElements.getNumSelected(); ++i)
    {
        PaintElement* const c = selectedElements.getSelectedItem (i);

        const int startX = c->getProperties() ["xDragStart"];
        const int startY = c->getProperties() ["yDragStart"];

        Rectangle<int> r (c->getCurrentBounds (parentArea));

        if (document != nullptr && selectedElements.getNumSelected() == 1)
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
    getDocument()->beginTransaction();
}

//==============================================================================
void PaintRoutine::fillWithBackground (Graphics& g, const bool drawOpaqueBackground)
{
    if ((! backgroundColour.isOpaque()) && drawOpaqueBackground)
    {
        g.fillCheckerBoard (Rectangle<int> (0, 0, g.getClipBounds().getRight(), g.getClipBounds().getBottom()),
                            50, 50,
                            Colour (0xffdddddd).overlaidWith (backgroundColour),
                            Colour (0xffffffff).overlaidWith (backgroundColour));
    }
    else
    {
        g.fillAll (backgroundColour);
    }
}

void PaintRoutine::drawElements (Graphics& g, const Rectangle<int>& relativeTo)
{
    Component temp;
    temp.setBounds (relativeTo);

    for (int i = 0; i < elements.size(); ++i)
        elements.getUnchecked (i)->draw (g, getDocument()->getComponentLayout(), relativeTo);
}

//==============================================================================
void PaintRoutine::dropImageAt (const File& f, int x, int y)
{
    ScopedPointer<Drawable> d (Drawable::createFromImageFile (f));

    if (d != nullptr)
    {
        Rectangle<float> bounds (d->getDrawableBounds());
        d = nullptr;

        PaintElement* newElement
            = addNewElement (ObjectTypes::createNewImageElement (this), -1, true);

        if (PaintElementImage* pei = dynamic_cast<PaintElementImage*> (newElement))
        {
            String resourceName (getDocument()->getResources().findUniqueName (f.getFileName()));

            if (const BinaryResources::BinaryResource* existingResource = getDocument()->getResources().getResourceForFile (f))
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

            const int imageW = (int) (bounds.getRight() + 0.999f);
            const int imageH = (int) (bounds.getBottom() + 0.999f);

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
const char* PaintRoutine::xmlTagName = "BACKGROUND";

XmlElement* PaintRoutine::createXml() const
{
    XmlElement* const xml = new XmlElement (xmlTagName);

    xml->setAttribute ("backgroundColour", backgroundColour.toString());

    for (int i = 0; i < elements.size(); ++i)
        xml->addChildElement (elements.getUnchecked (i)->createXml());

    return xml;
}

bool PaintRoutine::loadFromXml (const XmlElement& xml)
{
    if (xml.hasTagName (xmlTagName))
    {
        backgroundColour = Colour::fromString (xml.getStringAttribute ("backgroundColour", Colours::white.toString()));

        clear();

        forEachXmlChildElement (xml, e)
            if (PaintElement* const newElement = ObjectTypes::createElementForXml (e, this))
                elements.add (newElement);

        return true;
    }

    return false;
}

void PaintRoutine::fillInGeneratedCode (GeneratedCode& code, String& paintMethodCode) const
{
    if (! backgroundColour.isTransparent())
        paintMethodCode << "g.fillAll (" << CodeHelpers::colourToCode (backgroundColour) << ");\n\n";

    for (int i = 0; i < elements.size(); ++i)
        elements[i]->fillInGeneratedCode (code, paintMethodCode);
}
