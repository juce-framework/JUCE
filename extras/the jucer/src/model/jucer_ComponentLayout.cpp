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
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "../ui/jucer_JucerDocumentHolder.h"
#include "components/jucer_ComponentUndoableAction.h"


//==============================================================================
ComponentLayout::ComponentLayout()
    : document (0),
      nextCompUID (1)
{
}

ComponentLayout::~ComponentLayout()
{
    components.clear();
}

//==============================================================================
void ComponentLayout::changed()
{
    if (document != 0)
        document->changed();
}

void ComponentLayout::perform (UndoableAction* action, const String& actionName)
{
    jassert (document != 0);

    if (document != 0)
    {
        document->getUndoManager().perform (action, actionName);
    }
    else
    {
        action->perform();
        delete action;
    }
}

//==============================================================================
void ComponentLayout::clearComponents()
{
    selected.deselectAll();
    components.clear();
    changed();
}

//==============================================================================
class AddCompAction  : public UndoableAction
{
public:
    AddCompAction (XmlElement* const xml_, ComponentLayout& layout_)
       : indexAdded (-1),
         xml (xml_),
         layout (layout_)
    {
    }

    ~AddCompAction()
    {
        delete xml;
    }

    bool perform()
    {
        showCorrectTab();
        Component* const newComp = layout.addComponentFromXml (*xml, false);
        jassert (newComp != 0);

        indexAdded = layout.indexOfComponent (newComp);
        jassert (indexAdded >= 0);
        return indexAdded >= 0;
    }

    bool undo()
    {
        showCorrectTab();
        layout.removeComponent (layout.getComponent (indexAdded), false);
        return true;
    }

    int getSizeInUnits()    { return 10; }

    int indexAdded;

private:
    XmlElement* xml;
    ComponentLayout& layout;

    static void showCorrectTab()
    {
        JucerDocumentHolder* const docHolder = JucerDocumentHolder::getActiveDocumentHolder();

        if (docHolder != 0)
            docHolder->showLayout();
    }

    AddCompAction (const AddCompAction&);
    const AddCompAction& operator= (const AddCompAction&);
};

//==============================================================================
class DeleteCompAction  : public ComponentUndoableAction <Component>
{
public:
    DeleteCompAction (Component* const comp, ComponentLayout& layout)
       : ComponentUndoableAction <Component> (comp, layout),
         oldIndex (-1)
    {
        ComponentTypeHandler* const h = ComponentTypeHandler::getHandlerFor (*comp);
        jassert (h != 0);
        if (h != 0)
            xml = h->createXmlFor (comp, &layout);
        else
            xml = 0;

        oldIndex = layout.indexOfComponent (comp);
    }

    ~DeleteCompAction()
    {
        delete xml;
    }

    bool perform()
    {
        showCorrectTab();
        layout.removeComponent (getComponent(), false);
        return true;
    }

    bool undo()
    {
        Component* c = layout.addComponentFromXml (*xml, false);
        jassert (c != 0);

        layout.moveComponentZOrder (layout.indexOfComponent (c), oldIndex);

        showCorrectTab();
        return c != 0;
    }

private:
    XmlElement* xml;
    int oldIndex;
};

void ComponentLayout::removeComponent (Component* comp, const bool undoable)
{
    if (comp != 0 && components.contains (comp))
    {
        if (undoable)
        {
            perform (new DeleteCompAction (comp, *this), T("Delete components"));
        }
        else
        {
            selected.deselect (comp);
            selected.changed (true); // synchronous message to get rid of any property components

            components.removeObject (comp);
            changed();
        }
    }
}

//==============================================================================
class FrontBackCompAction  : public ComponentUndoableAction <Component>
{
public:
    FrontBackCompAction (Component* const comp, ComponentLayout& layout, int newIndex_)
       : ComponentUndoableAction <Component> (comp, layout),
         newIndex (newIndex_)
    {
        oldIndex = layout.indexOfComponent (comp);
    }

    bool perform()
    {
        showCorrectTab();
        Component* comp = layout.getComponent (oldIndex);
        layout.moveComponentZOrder (oldIndex, newIndex);
        newIndex = layout.indexOfComponent (comp);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        layout.moveComponentZOrder (newIndex, oldIndex);
        return true;
    }

private:
    int newIndex, oldIndex;
};

void ComponentLayout::moveComponentZOrder (int oldIndex, int newIndex)
{
    jassert (components [oldIndex] != 0);

    if (oldIndex != newIndex && components [oldIndex] != 0)
    {
        components.move (oldIndex, newIndex);
        changed();
    }
}

void ComponentLayout::componentToFront (Component* comp, const bool undoable)
{
    if (comp != 0 && components.contains (comp))
    {
        if (undoable)
            perform (new FrontBackCompAction (comp, *this, -1), T("Move components to front"));
        else
            moveComponentZOrder (components.indexOf (comp), -1);
    }
}

void ComponentLayout::componentToBack (Component* comp, const bool undoable)
{
    if (comp != 0 && components.contains (comp))
    {
        if (undoable)
            perform (new FrontBackCompAction (comp, *this, 0), T("Move components to back"));
        else
            moveComponentZOrder (components.indexOf (comp), 0);
    }
}


//==============================================================================
const tchar* const ComponentLayout::clipboardXmlTag = T("COMPONENTS");

void ComponentLayout::copySelectedToClipboard()
{
    if (selected.getNumSelected() == 0)
        return;

    XmlElement clip (clipboardXmlTag);

    for (int i = 0; i < components.size(); ++i)
    {
        Component* const c = components.getUnchecked(i);

        if (selected.isSelected (c))
        {
            ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*c);

            jassert (type != 0);
            if (type != 0)
            {
                XmlElement* const e = type->createXmlFor (c, this);
                clip.addChildElement (e);
            }
        }
    }

    SystemClipboard::copyTextToClipboard (clip.createDocument (String::empty, false, false));
}

void ComponentLayout::paste()
{
    XmlDocument clip (SystemClipboard::getTextFromClipboard());
    XmlElement* const doc = clip.getDocumentElement();

    if (doc != 0 && doc->hasTagName (clipboardXmlTag))
    {
        selected.deselectAll();

        forEachXmlChildElement (*doc, e)
        {
            Component* newComp = addComponentFromXml (*e, true);

            if (newComp != 0)
                selected.addToSelection (newComp);
        }

        startDragging();
        dragSelectedComps (Random::getSystemRandom().nextInt (40),
                           Random::getSystemRandom().nextInt (40));
        endDragging();
    }

    delete doc;
}

void ComponentLayout::deleteSelected()
{
    const SelectedItemSet <Component*> temp (selected);
    selected.deselectAll();
    selected.changed (true); // synchronous message to get rid of any property components

    if (temp.getNumSelected() > 0)
    {
        for (int i = temp.getNumSelected(); --i >= 0;)
            removeComponent (temp.getSelectedItem (i), true);

        changed();

        if (document != 0)
            document->dispatchPendingMessages(); // forces the change to propagate before a paint() callback can happen,
                                                 // in case there are components floating around that are now dangling pointers
    }
}

void ComponentLayout::selectAll()
{
    for (int i = 0; i < components.size(); ++i)
        selected.addToSelection (components.getUnchecked (i));
}

void ComponentLayout::selectedToFront()
{
    const SelectedItemSet <Component*> temp (selected);

    for (int i = temp.getNumSelected(); --i >= 0;)
        componentToFront (temp.getSelectedItem(i), true);
}

void ComponentLayout::selectedToBack()
{
    const SelectedItemSet <Component*> temp (selected);

    for (int i = 0; i < temp.getNumSelected(); ++i)
        componentToBack (temp.getSelectedItem(i), true);
}

void ComponentLayout::bringLostItemsBackOnScreen (int width, int height)
{
    for (int i = components.size(); --i >= 0;)
    {
        Component* const c = components[i];

        if (! c->getBounds().intersects (Rectangle (0, 0, width, height)))
        {
            c->setTopLeftPosition (width / 2, height / 2);
            updateStoredComponentPosition (c, false);
        }
    }
}

Component* ComponentLayout::addNewComponent (ComponentTypeHandler* const type, int x, int y)
{
    Component* c = type->createNewComponent (getDocument());
    jassert (c != 0);

    if (c != 0)
    {
        c->setSize (type->getDefaultWidth(), type->getDefaultHeight());
        c->setCentrePosition (x, y);
        updateStoredComponentPosition (c, false);

        c->setComponentProperty (T("id"), nextCompUID++);

        XmlElement* xml = type->createXmlFor (c, this);
        delete c;

        c = addComponentFromXml (*xml, true);
        delete xml;

        String memberName (makeValidCppIdentifier (type->getClassName (c), true, true, false));
        setComponentMemberVariableName (c, memberName);

        selected.selectOnly (c);
    }

    return c;
}


Component* ComponentLayout::addComponentFromXml (const XmlElement& xml, const bool undoable)
{
    if (undoable)
    {
        AddCompAction* const action = new AddCompAction (new XmlElement (xml), *this);
        perform (action, T("Add new components"));

        return components [action->indexAdded];
    }
    else
    {
        ComponentTypeHandler* const type
            = ComponentTypeHandler::getHandlerForXmlTag (xml.getTagName());

        if (type != 0)
        {
            Component* const newComp = type->createNewComponent (getDocument());

            if (type->restoreFromXml (xml, newComp, this))
            {
                // ensure that the new comp's name is unique
                setComponentMemberVariableName (newComp, getComponentMemberVariableName (newComp));

                // check for duped IDs..
                while (findComponentWithId (ComponentTypeHandler::getComponentId (newComp)) != 0)
                    ComponentTypeHandler::setComponentId (newComp, Random::getSystemRandom().nextInt64());

                components.add (newComp);
                changed();
                return newComp;
            }
            else
            {
                delete newComp;
            }
        }

        return 0;
    }
}

Component* ComponentLayout::findComponentWithId (const int64 componentId) const
{
    for (int i = 0; i < components.size(); ++i)
        if (ComponentTypeHandler::getComponentId (components.getUnchecked(i)) == componentId)
            return components.getUnchecked(i);

    return 0;
}

//==============================================================================
static const tchar* const dimensionSuffixes[] = { T("X"), T("Y"), T("W"), T("H") };

Component* ComponentLayout::getComponentRelativePosTarget (Component* comp, int whichDimension) const
{
    jassert (comp != 0);

    PaintElement* const pe = dynamic_cast <PaintElement*> (comp);

    if (pe != 0)
    {
        int64 compId;

        if (whichDimension == 0)
            compId = pe->getPosition().relativeToX;
        else if (whichDimension == 1)
            compId = pe->getPosition().relativeToY;
        else if (whichDimension == 2)
            compId = pe->getPosition().relativeToW;
        else
            compId = pe->getPosition().relativeToH;

        return findComponentWithId (compId);
    }
    else
    {
        return findComponentWithId (comp->getComponentProperty (String (T("relativeTo"))
                                      + dimensionSuffixes [whichDimension], false).getHexValue64());
    }
}

void ComponentLayout::setComponentRelativeTarget (Component* comp, int whichDimension, Component* compToBeRelativeTo)
{
    PaintElement* const pe = dynamic_cast <PaintElement*> (comp);

    jassert (comp != 0);
    jassert (pe != 0 || components.contains (comp));
    jassert (compToBeRelativeTo == 0 || components.contains (compToBeRelativeTo));
    jassert (compToBeRelativeTo == 0 || ! dependsOnComponentForRelativePos (compToBeRelativeTo, comp));

    if (compToBeRelativeTo != getComponentRelativePosTarget (comp, whichDimension)
         && (compToBeRelativeTo == 0 || ! dependsOnComponentForRelativePos (compToBeRelativeTo, comp)))
    {
        const int64 compId = ComponentTypeHandler::getComponentId (compToBeRelativeTo);

        Rectangle oldBounds (comp->getBounds());
        RelativePositionedRectangle pos;

        if (pe != 0)
        {
            oldBounds = pe->getCurrentBounds (dynamic_cast <PaintRoutineEditor*> (pe->getParentComponent())->getComponentArea());
            pos = pe->getPosition();
        }
        else
        {
            pos = ComponentTypeHandler::getComponentPosition (comp);
        }

        if (whichDimension == 0)
            pos.relativeToX = compId;
        else if (whichDimension == 1)
            pos.relativeToY = compId;
        else if (whichDimension == 2)
            pos.relativeToW = compId;
        else if (whichDimension == 3)
            pos.relativeToH = compId;

        if (pe != 0)
        {
            pe->setPosition (pos, true);
            pe->setCurrentBounds (oldBounds, dynamic_cast <PaintRoutineEditor*> (pe->getParentComponent())->getComponentArea(), true);
        }
        else
        {
            setComponentPosition (comp, pos, true);
            comp->setBounds (oldBounds);
            updateStoredComponentPosition (comp, false);
        }

        changed();
    }
}

bool ComponentLayout::dependsOnComponentForRelativePos (Component* comp, Component* possibleDependee) const
{
    for (int i = 0; i < 4; ++i)
    {
        Component* const c = getComponentRelativePosTarget (comp, i);
        if (c != 0 && (c == possibleDependee || dependsOnComponentForRelativePos (c, possibleDependee)))
            return true;
    }

    return false;
}

const int menuIdBase = 0x63240000;

PopupMenu ComponentLayout::getRelativeTargetMenu (Component* comp, int whichDimension) const
{
    PopupMenu m;

    Component* const current = getComponentRelativePosTarget (comp, whichDimension);

    m.addItem (menuIdBase, T("Relative to parent component"), true, current == 0);
    m.addSeparator();

    for (int i = 0; i < components.size(); ++i)
    {
        Component* const c = components.getUnchecked(i);

        if (c->getComponentUID() != comp->getComponentUID())
        {
            m.addItem (menuIdBase + i + 1,
                       T("Relative to ") + getComponentMemberVariableName (c)
                        + T(" (class: ") + ComponentTypeHandler::getHandlerFor (*c)->getClassName (c) + T(")"),
                       ! dependsOnComponentForRelativePos (c, comp),
                       current == c);
        }
    }

    return m;
}

void ComponentLayout::processRelativeTargetMenuResult (Component* comp, int whichDimension, int menuResultID)
{
    if (menuResultID != 0)
    {
        Component* const newTarget = components [menuResultID - menuIdBase - 1];
        setComponentRelativeTarget (comp, whichDimension, newTarget);
    }
}

//==============================================================================
class ChangeCompPositionAction  : public ComponentUndoableAction <Component>
{
public:
    ChangeCompPositionAction (Component* const comp, ComponentLayout& layout,
                              const RelativePositionedRectangle& newPos_)
       : ComponentUndoableAction <Component> (comp, layout),
         newPos (newPos_)
    {
        oldPos = ComponentTypeHandler::getComponentPosition (comp);
    }

    bool perform()
    {
        showCorrectTab();
        layout.setComponentPosition (getComponent(), newPos, false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        layout.setComponentPosition (getComponent(), oldPos, false);
        return true;
    }

private:
    RelativePositionedRectangle newPos, oldPos;
};

void ComponentLayout::setComponentPosition (Component* comp,
                                            const RelativePositionedRectangle& newPos,
                                            const bool undoable)
{
    if (ComponentTypeHandler::getComponentPosition (comp) != newPos)
    {
        if (undoable)
        {
            perform (new ChangeCompPositionAction (comp, *this, newPos), T("Move components"));
        }
        else
        {
            ComponentTypeHandler::setComponentPosition (comp, newPos, this);
            changed();
        }
    }
}

void ComponentLayout::updateStoredComponentPosition (Component* comp, const bool undoable)
{
    RelativePositionedRectangle newPos (ComponentTypeHandler::getComponentPosition (comp));

    newPos.updateFromComponent (*comp, this);

    setComponentPosition (comp, newPos, undoable);
}

//==============================================================================
void ComponentLayout::startDragging()
{
    for (int i = 0; i < components.size(); ++i)
    {
        Component* const c = components[i];
        c->setComponentProperty (T("xDragStart"), c->getX());
        c->setComponentProperty (T("yDragStart"), c->getY());
    }

    jassert (document != 0);
    document->getUndoManager().beginNewTransaction();
}

void ComponentLayout::dragSelectedComps (int dx, int dy, const bool allowSnap)
{
    if (allowSnap && document != 0 && selected.getNumSelected() > 1)
    {
        dx = document->snapPosition (dx);
        dy = document->snapPosition (dy);
    }

    for (int i = 0; i < selected.getNumSelected(); ++i)
    {
        Component* const c = selected.getSelectedItem (i);

        const int startX = c->getComponentPropertyInt (T("xDragStart"), false);
        const int startY = c->getComponentPropertyInt (T("yDragStart"), false);

        if (allowSnap && document != 0 && selected.getNumSelected() == 1)
        {
            c->setTopLeftPosition (document->snapPosition (startX + dx),
                                   document->snapPosition (startY + dy));
        }
        else
        {
            c->setTopLeftPosition (startX + dx,
                                   startY + dy);
        }

        updateStoredComponentPosition (c, false);
    }
}

void ComponentLayout::endDragging()
{
    // after the drag, roll back all the comps to their start position, then
    // back to their finish positions using an undoable command.
    document->getUndoManager().beginNewTransaction();

    for (int i = 0; i < selected.getNumSelected(); ++i)
    {
        Component* const c = selected.getSelectedItem (i);

        const int newX = c->getX();
        const int newY = c->getY();

        const int startX = c->getComponentPropertyInt (T("xDragStart"), false);
        const int startY = c->getComponentPropertyInt (T("yDragStart"), false);

        c->setTopLeftPosition (startX, startY);
        updateStoredComponentPosition (c, false);

        c->setTopLeftPosition (newX, newY);
        updateStoredComponentPosition (c, true);
    }

    document->getUndoManager().beginNewTransaction();
}

void ComponentLayout::moveSelectedComps (int dx, int dy, bool snap)
{
    startDragging();
    dragSelectedComps (dx, dy, snap);
    endDragging();
}

void ComponentLayout::stretchSelectedComps (int dw, int dh, bool allowSnap)
{
    int neww, newh;

    if (document != 0 && selected.getNumSelected() == 1)
    {
        Component* const c = selected.getSelectedItem (0);

        if (allowSnap)
        {
            int bot = c->getBottom() + dh;
            int right = c->getRight() + dw;
            bot = (dh != 0) ? document->snapPosition (bot) : bot;
            right = (dw != 0) ? document->snapPosition (right) : right;
            newh = bot - c->getY();
            neww = right - c->getX();
        }
        else
        {
            newh = c->getHeight() + dh;
            neww = c->getWidth() + dw;
        }

        c->setSize (neww, newh);

        updateStoredComponentPosition (c, true);
    }
    else
    {
        for (int i = 0; i < selected.getNumSelected(); ++i)
        {
            Component* const c = selected.getSelectedItem (i);

            neww = c->getWidth() + dw;
            newh = c->getHeight() + dh;
            c->setSize (neww, newh);

            updateStoredComponentPosition (c, true);
        }
    }
}


//==============================================================================
void ComponentLayout::fillInGeneratedCode (GeneratedCode& code) const
{
    for (int i = 0; i < components.size(); ++i)
    {
        Component* const comp = components[i];
        ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp);

        if (type != 0)
            type->fillInGeneratedCode (comp, code);
    }
}

//==============================================================================
const String ComponentLayout::getComponentMemberVariableName (Component* comp) const
{
    if (comp == 0)
        return String::empty;

    String name (comp->getComponentProperty (T("memberName"), false));

    if (name.isEmpty())
        name = getUnusedMemberName (makeValidCppIdentifier (comp->getName(), true, true, false), comp);

    return name;
}

void ComponentLayout::setComponentMemberVariableName (Component* comp, const String& newName)
{
    const String oldName (getComponentMemberVariableName (comp));

    comp->setComponentProperty (T("memberName"), String::empty);

    const String n (getUnusedMemberName (makeValidCppIdentifier (newName, false, true, false), comp));
    comp->setComponentProperty (T("memberName"), n);

    if (n != oldName)
        changed();
}

const String ComponentLayout::getUnusedMemberName (String nameRoot, Component* comp) const
{
    String n (nameRoot);

    while (CharacterFunctions::isDigit (nameRoot.getLastCharacter()))
        nameRoot = nameRoot.dropLastCharacters (1);

    int suffix = 2;

    for (;;)
    {
        bool alreadyUsed = false;

        for (int i = 0; i < components.size(); ++i)
        {
            if (components[i] != comp
                 && components[i]->getComponentProperty (T("memberName"), false) == n)
            {
                alreadyUsed = true;
                break;
            }
        }

        if (! alreadyUsed)
            break;

        n = nameRoot + String (suffix++);
    }

    return n;
}

//==============================================================================
const String ComponentLayout::getComponentVirtualClassName (Component* comp) const
{
    if (comp == 0)
        return String::empty;

    return comp->getComponentProperty (T("virtualName"), false);
}

void ComponentLayout::setComponentVirtualClassName (Component* comp, const String& newName)
{
    const String name (makeValidCppIdentifier (newName, false, false, true));

    if (name != getComponentVirtualClassName (comp))
    {
        comp->setComponentProperty (T("virtualName"), name);
        changed();
    }
}

//==============================================================================
void ComponentLayout::addToXml (XmlElement& xml) const
{
    for (int i = 0; i < components.size(); ++i)
    {
        ComponentTypeHandler* h = ComponentTypeHandler::getHandlerFor (*components [i]);

        if (h != 0)
            xml.addChildElement (h->createXmlFor (components [i], this));
    }
}
