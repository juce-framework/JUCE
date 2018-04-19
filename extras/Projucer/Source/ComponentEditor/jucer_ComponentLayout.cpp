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

#include "../Application/jucer_Headers.h"
#include "jucer_JucerDocument.h"
#include "jucer_ObjectTypes.h"
#include "UI/jucer_JucerDocumentEditor.h"
#include "Components/jucer_ComponentUndoableAction.h"

//==============================================================================
ComponentLayout::ComponentLayout()
    : document (nullptr),
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
    if (document != nullptr)
        document->changed();
}

void ComponentLayout::perform (UndoableAction* action, const String& actionName)
{
    jassert (document != nullptr);

    if (document != nullptr)
    {
        document->getUndoManager().perform (action, actionName);
    }
    else
    {
        std::unique_ptr<UndoableAction> deleter (action);
        action->perform();
    }
}

//==============================================================================
void ComponentLayout::clearComponents()
{
    selected.deselectAll();
    selected.dispatchPendingMessages();
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

    bool perform()
    {
        showCorrectTab();
        Component* const newComp = layout.addComponentFromXml (*xml, false);
        jassert (newComp != nullptr);

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
    std::unique_ptr<XmlElement> xml;
    ComponentLayout& layout;

    static void showCorrectTab()
    {
        if (JucerDocumentEditor* const ed = JucerDocumentEditor::getActiveDocumentHolder())
            ed->showLayout();
    }

    AddCompAction (const AddCompAction&);
    AddCompAction& operator= (const AddCompAction&);
};

//==============================================================================
class DeleteCompAction  : public ComponentUndoableAction <Component>
{
public:
    DeleteCompAction (Component* const comp, ComponentLayout& l)
       : ComponentUndoableAction <Component> (comp, l),
         oldIndex (-1)
    {
        if (ComponentTypeHandler* const h = ComponentTypeHandler::getHandlerFor (*comp))
            xml.reset (h->createXmlFor (comp, &layout));
        else
            jassertfalse;

        oldIndex = l.indexOfComponent (comp);
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
        jassert (c != nullptr);

        layout.moveComponentZOrder (layout.indexOfComponent (c), oldIndex);

        showCorrectTab();
        return c != nullptr;
    }

private:
    std::unique_ptr<XmlElement> xml;
    int oldIndex;
};

void ComponentLayout::removeComponent (Component* comp, const bool undoable)
{
    if (comp != nullptr && components.contains (comp))
    {
        if (undoable)
        {
            perform (new DeleteCompAction (comp, *this), "Delete components");
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
    FrontBackCompAction (Component* const comp, ComponentLayout& l, int newIndex_)
       : ComponentUndoableAction <Component> (comp, l),
         newIndex (newIndex_)
    {
        oldIndex = l.indexOfComponent (comp);
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
    jassert (components [oldIndex] != nullptr);

    if (oldIndex != newIndex && components [oldIndex] != nullptr)
    {
        components.move (oldIndex, newIndex);
        changed();
    }
}

void ComponentLayout::componentToFront (Component* comp, const bool undoable)
{
    if (comp != nullptr && components.contains (comp))
    {
        if (undoable)
            perform (new FrontBackCompAction (comp, *this, -1), "Move components to front");
        else
            moveComponentZOrder (components.indexOf (comp), -1);
    }
}

void ComponentLayout::componentToBack (Component* comp, const bool undoable)
{
    if (comp != nullptr && components.contains (comp))
    {
        if (undoable)
            perform (new FrontBackCompAction (comp, *this, 0), "Move components to back");
        else
            moveComponentZOrder (components.indexOf (comp), 0);
    }
}

//==============================================================================
const char* const ComponentLayout::clipboardXmlTag = "COMPONENTS";

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
            if (ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*c))
            {
                XmlElement* const e = type->createXmlFor (c, this);
                clip.addChildElement (e);
            }
        }
    }

    SystemClipboard::copyTextToClipboard (clip.createDocument ("", false, false));
}

void ComponentLayout::paste()
{
    XmlDocument clip (SystemClipboard::getTextFromClipboard());
    std::unique_ptr<XmlElement> doc (clip.getDocumentElement());

    if (doc != nullptr && doc->hasTagName (clipboardXmlTag))
    {
        selected.deselectAll();

        forEachXmlChildElement (*doc, e)
            if (Component* newComp = addComponentFromXml (*e, true))
                selected.addToSelection (newComp);

        startDragging();
        dragSelectedComps (Random::getSystemRandom().nextInt (40),
                           Random::getSystemRandom().nextInt (40));
        endDragging();
    }
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

        if (document != nullptr)
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

void ComponentLayout::alignTop()
{
    if (selected.getNumSelected() > 1)
    {
        auto* main = selected.getSelectedItem (0);
        auto yPos = main->getY();

        for (auto* other : selected)
        {
            if (other != main)
                setComponentBoundsAndProperties (other,
                                                 other->getBounds().withPosition (other->getX(), yPos),
                                                 main, true);
        }
    }
}

void ComponentLayout::alignRight()
{
    if (selected.getNumSelected() > 1)
    {
        auto* main = selected.getSelectedItem (0);
        auto rightPos = main->getRight();

        for (auto* other : selected)
        {
            if (other != main)
                setComponentBoundsAndProperties (other, other->getBounds().withPosition (rightPos - other->getWidth(), other->getY()), main, true);
        }
    }
}

void ComponentLayout::alignBottom()
{
    if (selected.getNumSelected() > 1)
    {
        auto* main = selected.getSelectedItem (0);
        auto bottomPos = main->getBottom();

        for (auto* other : selected)
        {
            if (other != main)
                setComponentBoundsAndProperties (other, other->getBounds().withPosition (other->getX(), bottomPos - other->getHeight()), main, true);
        }
    }
}

void ComponentLayout::alignLeft()
{
    if (selected.getNumSelected() > 1)
    {
        auto* main = selected.getSelectedItem (0);
        auto xPos = main->getX();

        for (auto* other : selected)
        {
            if (other != main)
                setComponentBoundsAndProperties (other, other->getBounds().withPosition (xPos, other->getY()), main, true);
        }
    }
}

void ComponentLayout::bringLostItemsBackOnScreen (int width, int height)
{
    for (int i = components.size(); --i >= 0;)
    {
        Component* const c = components[i];

        if (! c->getBounds().intersects (Rectangle<int> (0, 0, width, height)))
        {
            c->setTopLeftPosition (width / 2, height / 2);
            updateStoredComponentPosition (c, false);
        }
    }
}

Component* ComponentLayout::addNewComponent (ComponentTypeHandler* const type, int x, int y)
{
    std::unique_ptr<Component> c (type->createNewComponent (getDocument()));
    jassert (c != nullptr);

    if (c != nullptr)
    {
        c->setSize (type->getDefaultWidth(), type->getDefaultHeight());
        c->setCentrePosition (x, y);
        updateStoredComponentPosition (c.get(), false);

        c->getProperties().set ("id", nextCompUID++);

        std::unique_ptr<XmlElement> xml (type->createXmlFor (c.get(), this));
        c.reset (addComponentFromXml (*xml, true));

        String memberName (CodeHelpers::makeValidIdentifier (type->getClassName (c.get()), true, true, false));
        setComponentMemberVariableName (c.get(), memberName);

        selected.selectOnly (c.get());
    }

    return c.release();
}


Component* ComponentLayout::addComponentFromXml (const XmlElement& xml, const bool undoable)
{
    if (undoable)
    {
        AddCompAction* const action = new AddCompAction (new XmlElement (xml), *this);
        perform (action, "Add new components");
        return components [action->indexAdded];
    }

    if (ComponentTypeHandler* const type
           = ComponentTypeHandler::getHandlerForXmlTag (xml.getTagName()))
    {
        std::unique_ptr<Component> newComp (type->createNewComponent (getDocument()));

        if (type->restoreFromXml (xml, newComp.get(), this))
        {
            // ensure that the new comp's name is unique
            setComponentMemberVariableName (newComp.get(), getComponentMemberVariableName (newComp.get()));

            // check for duped IDs..
            while (findComponentWithId (ComponentTypeHandler::getComponentId (newComp.get())) != nullptr)
                ComponentTypeHandler::setComponentId (newComp.get(), Random::getSystemRandom().nextInt64());

            components.add (newComp.get());
            changed();
            return newComp.release();
        }
    }

    return nullptr;
}

Component* ComponentLayout::findComponentWithId (const int64 componentId) const
{
    for (int i = 0; i < components.size(); ++i)
        if (ComponentTypeHandler::getComponentId (components.getUnchecked(i)) == componentId)
            return components.getUnchecked(i);

    return nullptr;
}

//==============================================================================
static const char* const dimensionSuffixes[] = { "X", "Y", "W", "H" };

Component* ComponentLayout::getComponentRelativePosTarget (Component* comp, int whichDimension) const
{
    jassert (comp != nullptr);

    if (PaintElement* const pe = dynamic_cast<PaintElement*> (comp))
    {
        int64 compId;

        if (whichDimension == 0)        compId = pe->getPosition().relativeToX;
        else if (whichDimension == 1)   compId = pe->getPosition().relativeToY;
        else if (whichDimension == 2)   compId = pe->getPosition().relativeToW;
        else                            compId = pe->getPosition().relativeToH;

        return findComponentWithId (compId);
    }

    return findComponentWithId (comp->getProperties() [String ("relativeTo") + dimensionSuffixes [whichDimension]]
                                    .toString().getHexValue64());
}

void ComponentLayout::setComponentRelativeTarget (Component* comp, int whichDimension, Component* compToBeRelativeTo)
{
    PaintElement* const pe = dynamic_cast<PaintElement*> (comp);

    jassert (comp != nullptr);
    jassert (pe != nullptr || components.contains (comp));
    jassert (compToBeRelativeTo == 0 || components.contains (compToBeRelativeTo));
    jassert (compToBeRelativeTo == 0 || ! dependsOnComponentForRelativePos (compToBeRelativeTo, comp));

    if (compToBeRelativeTo != getComponentRelativePosTarget (comp, whichDimension)
         && (compToBeRelativeTo == 0 || ! dependsOnComponentForRelativePos (compToBeRelativeTo, comp)))
    {
        const int64 compId = ComponentTypeHandler::getComponentId (compToBeRelativeTo);

        Rectangle<int> oldBounds (comp->getBounds());
        RelativePositionedRectangle pos;

        if (pe != nullptr)
        {
            oldBounds = pe->getCurrentBounds (dynamic_cast<PaintRoutineEditor*> (pe->getParentComponent())->getComponentArea());
            pos = pe->getPosition();
        }
        else
        {
            pos = ComponentTypeHandler::getComponentPosition (comp);
        }

        if (whichDimension == 0)       pos.relativeToX = compId;
        else if (whichDimension == 1)  pos.relativeToY = compId;
        else if (whichDimension == 2)  pos.relativeToW = compId;
        else if (whichDimension == 3)  pos.relativeToH = compId;

        if (pe != nullptr)
        {
            pe->setPosition (pos, true);
            pe->setCurrentBounds (oldBounds, dynamic_cast<PaintRoutineEditor*> (pe->getParentComponent())->getComponentArea(), true);
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
        if (Component* const c = getComponentRelativePosTarget (comp, i))
            if (c == possibleDependee || dependsOnComponentForRelativePos (c, possibleDependee))
                return true;

    return false;
}

const int menuIdBase = 0x63240000;

PopupMenu ComponentLayout::getRelativeTargetMenu (Component* comp, int whichDimension) const
{
    PopupMenu m;

    Component* const current = getComponentRelativePosTarget (comp, whichDimension);

    m.addItem (menuIdBase, "Relative to parent component", true, current == 0);
    m.addSeparator();

    for (int i = 0; i < components.size(); ++i)
    {
        Component* const c = components.getUnchecked(i);

        if (c != comp)
            m.addItem (menuIdBase + i + 1,
                       "Relative to " + getComponentMemberVariableName (c)
                        + " (class: " + ComponentTypeHandler::getHandlerFor (*c)->getClassName (c) + ")",
                       ! dependsOnComponentForRelativePos (c, comp),
                       current == c);
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
    ChangeCompPositionAction (Component* const comp, ComponentLayout& l,
                              const RelativePositionedRectangle& newPos_)
       : ComponentUndoableAction <Component> (comp, l),
         newPos (newPos_), oldPos (ComponentTypeHandler::getComponentPosition (comp))
    {
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

class ChangeCompBoundsAndPropertiesAction    : public ComponentUndoableAction<Component>
{
public:
    ChangeCompBoundsAndPropertiesAction (Component* const comp, ComponentLayout& l,
                                         const Rectangle<int>& bounds, const NamedValueSet& props)
        : ComponentUndoableAction <Component> (comp, l),
          newBounds (bounds),
          oldBounds (comp->getBounds()),
          newProps (props),
          oldProps(comp->getProperties())
    {
    }

    bool perform()
    {
        showCorrectTab();
        getComponent()->setBounds (newBounds);
        getComponent()->getProperties() = newProps;
        layout.updateStoredComponentPosition (getComponent(), false);
        return true;
    }

    bool undo()
    {
        showCorrectTab();
        getComponent()->setBounds (oldBounds);
        getComponent()->getProperties() = oldProps;
        layout.updateStoredComponentPosition (getComponent(), false);
        return true;
    }

private:
    Rectangle<int> newBounds, oldBounds;
    NamedValueSet newProps, oldProps;
};

void ComponentLayout::setComponentPosition (Component* comp,
                                            const RelativePositionedRectangle& newPos,
                                            const bool undoable)
{
    if (ComponentTypeHandler::getComponentPosition (comp) != newPos)
    {
        if (undoable)
        {
            perform (new ChangeCompPositionAction (comp, *this, newPos), "Move components");
        }
        else
        {
            ComponentTypeHandler::setComponentPosition (comp, newPos, this);
            changed();
        }
    }
}

void ComponentLayout::setComponentBoundsAndProperties (Component* componentToPosition, const Rectangle<int>& newBounds,
                                                       Component* referenceComponent, const bool undoable)
{
    auto props = NamedValueSet (componentToPosition->getProperties());

    auto rect = ComponentTypeHandler::getComponentPosition (componentToPosition).rect;
    auto referenceComponentPosition = ComponentTypeHandler::getComponentPosition (referenceComponent);
    auto referenceComponentRect = referenceComponentPosition.rect;

    rect.setModes (referenceComponentRect.getAnchorPointX(), referenceComponentRect.getPositionModeX(),
                   referenceComponentRect.getAnchorPointY(), referenceComponentRect.getPositionModeY(),
                   referenceComponentRect.getWidthMode(),    referenceComponentRect.getHeightMode(),
                   componentToPosition->getBounds());

    props.set ("pos",         rect.toString());
    props.set ("relativeToX", String::toHexString (referenceComponentPosition.relativeToX));
    props.set ("relativeToY", String::toHexString (referenceComponentPosition.relativeToY));
    props.set ("relativeToW", String::toHexString (referenceComponentPosition.relativeToW));
    props.set ("relativeToH", String::toHexString (referenceComponentPosition.relativeToH));

    if (componentToPosition->getBounds() != newBounds || componentToPosition->getProperties() != props)
    {
        if (undoable)
        {
            perform (new ChangeCompBoundsAndPropertiesAction (componentToPosition, *this, newBounds, props), "Change component bounds");
        }
        else
        {
            componentToPosition->setBounds (newBounds);
            componentToPosition->getProperties() = props;
            updateStoredComponentPosition (componentToPosition, false);
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
        c->getProperties().set ("xDragStart", c->getX());
        c->getProperties().set ("yDragStart", c->getY());
    }

    jassert (document != nullptr);
    document->beginTransaction();
}

void ComponentLayout::dragSelectedComps (int dx, int dy, const bool allowSnap)
{
    if (allowSnap && document != nullptr && selected.getNumSelected() > 1)
    {
        dx = document->snapPosition (dx);
        dy = document->snapPosition (dy);
    }

    for (int i = 0; i < selected.getNumSelected(); ++i)
    {
        Component* const c = selected.getSelectedItem (i);

        const int startX = c->getProperties() ["xDragStart"];
        const int startY = c->getProperties() ["yDragStart"];

        if (allowSnap && document != nullptr && selected.getNumSelected() == 1)
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
    document->beginTransaction();

    for (int i = 0; i < selected.getNumSelected(); ++i)
    {
        Component* const c = selected.getSelectedItem (i);

        const int newX = c->getX();
        const int newY = c->getY();

        const int startX = c->getProperties() ["xDragStart"];
        const int startY = c->getProperties() ["yDragStart"];

        c->setTopLeftPosition (startX, startY);
        updateStoredComponentPosition (c, false);

        c->setTopLeftPosition (newX, newY);
        updateStoredComponentPosition (c, true);
    }

    document->beginTransaction();
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

    if (document != nullptr && selected.getNumSelected() == 1)
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
        if (Component* const comp = components.getUnchecked(i))
            if (ComponentTypeHandler* const type = ComponentTypeHandler::getHandlerFor (*comp))
                type->fillInGeneratedCode (comp, code);
}

//==============================================================================
String ComponentLayout::getComponentMemberVariableName (Component* comp) const
{
    if (comp == nullptr)
        return {};

    String name (comp->getProperties() ["memberName"].toString());

    if (name.isEmpty())
        name = getUnusedMemberName (CodeHelpers::makeValidIdentifier (comp->getName(), true, true, false), comp);

    return name;
}

void ComponentLayout::setComponentMemberVariableName (Component* comp, const String& newName)
{
    jassert (comp != nullptr);
    const String oldName (getComponentMemberVariableName (comp));

    comp->getProperties().set ("memberName", String());

    const String n (getUnusedMemberName (CodeHelpers::makeValidIdentifier (newName, false, true, false), comp));
    comp->getProperties().set ("memberName", n);

    if (n != oldName)
        changed();
}

String ComponentLayout::getUnusedMemberName (String nameRoot, Component* comp) const
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
                 && components[i]->getProperties() ["memberName"] == n)
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
String ComponentLayout::getComponentVirtualClassName (Component* comp) const
{
    if (comp == nullptr)
        return {};

    return comp->getProperties() ["virtualName"];
}

void ComponentLayout::setComponentVirtualClassName (Component* comp, const String& newName)
{
    jassert (comp != nullptr);
    const String name (CodeHelpers::makeValidIdentifier (newName, false, false, true));

    if (name != getComponentVirtualClassName (comp))
    {
        comp->getProperties().set ("virtualName", name);
        changed();
    }
}

//==============================================================================
void ComponentLayout::addToXml (XmlElement& xml) const
{
    for (int i = 0; i < components.size(); ++i)
        if (ComponentTypeHandler* h = ComponentTypeHandler::getHandlerFor (*components [i]))
            xml.addChildElement (h->createXmlFor (components [i], this));
}

static String bracketIfNeeded (const String& s)
{
    return s.containsAnyOf ("+-*/%") ? "(" + s + ")" : s;
}

//==============================================================================
void positionToCode (const RelativePositionedRectangle& position,
                     const ComponentLayout* layout,
                     String& x, String& y, String& w, String& h)
{
    // these are the code sections for the positions of the relative comps
    String xrx, xry, xrw, xrh;
    if (Component* const relCompX = layout != nullptr ? layout->findComponentWithId (position.relativeToX) : nullptr)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompX), layout, xrx, xry, xrw, xrh);

    String yrx, yry, yrw, yrh;

    if (Component* const relCompY = layout != nullptr ? layout->findComponentWithId (position.relativeToY) : nullptr)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompY), layout, yrx, yry, yrw, yrh);

    String wrx, wry, wrw, wrh;

    if (Component* const relCompW = (layout != nullptr && position.rect.getWidthMode() != PositionedRectangle::absoluteSize)
                                        ? layout->findComponentWithId (position.relativeToW) : nullptr)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompW), layout, wrx, wry, wrw, wrh);

    String hrx, hry, hrw, hrh;

    if (Component* const relCompH = (layout != nullptr && position.rect.getHeightMode() != PositionedRectangle::absoluteSize)
                                        ? layout->findComponentWithId (position.relativeToH) : nullptr)
        positionToCode (ComponentTypeHandler::getComponentPosition (relCompH), layout, hrx, hry, hrw, hrh);

    // width
    if (position.rect.getWidthMode() == PositionedRectangle::proportionalSize)
    {
        if (wrw.isNotEmpty())
            w << "roundToInt (" << bracketIfNeeded (wrw) << " * " << CodeHelpers::floatLiteral (position.rect.getWidth(), 4) << ")";
        else
            w << "proportionOfWidth (" << CodeHelpers::floatLiteral (position.rect.getWidth(), 4) << ")";
    }
    else if (position.rect.getWidthMode() == PositionedRectangle::parentSizeMinusAbsolute)
    {
        if (wrw.isNotEmpty())
            w << bracketIfNeeded (wrw) << " - " << roundToInt (position.rect.getWidth());
        else
            w << "getWidth() - " << roundToInt (position.rect.getWidth());
    }
    else
    {
        if (wrw.isNotEmpty())
            w << bracketIfNeeded (wrw) << " + ";

        w << roundToInt (position.rect.getWidth());
    }

    // height
    if (position.rect.getHeightMode() == PositionedRectangle::proportionalSize)
    {
        if (hrh.isNotEmpty())
            h << "roundToInt (" << bracketIfNeeded (hrh) << " * " << CodeHelpers::floatLiteral (position.rect.getHeight(), 4) << ")";
        else
            h << "proportionOfHeight (" << CodeHelpers::floatLiteral (position.rect.getHeight(), 4) << ")";
    }
    else if (position.rect.getHeightMode() == PositionedRectangle::parentSizeMinusAbsolute)
    {
        if (hrh.isNotEmpty())
            h << bracketIfNeeded (hrh) << " - " << roundToInt (position.rect.getHeight());
        else
            h << "getHeight() - " << roundToInt (position.rect.getHeight());
    }
    else
    {
        if (hrh.isNotEmpty())
            h << bracketIfNeeded (hrh) << " + ";

        h << roundToInt (position.rect.getHeight());
    }

    // x-pos
    if (position.rect.getPositionModeX() == PositionedRectangle::proportionOfParentSize)
    {
        if (xrx.isNotEmpty() && xrw.isNotEmpty())
            x << bracketIfNeeded (xrx) << " + roundToInt (" << bracketIfNeeded (xrw) << " * " << CodeHelpers::floatLiteral (position.rect.getX(), 4) << ")";
        else
            x << "proportionOfWidth (" << CodeHelpers::floatLiteral (position.rect.getX(), 4) << ")";
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentTopLeft)
    {
        if (xrx.isNotEmpty())
            x << bracketIfNeeded (xrx) << " + ";

        x << roundToInt (position.rect.getX());
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentBottomRight)
    {
        if (xrx.isNotEmpty())
            x << bracketIfNeeded (xrx) << " + " << bracketIfNeeded (xrw);
        else
            x << "getWidth()";

        const int d = roundToInt (position.rect.getX());
        if (d != 0)
            x << " - " << d;
    }
    else if (position.rect.getPositionModeX() == PositionedRectangle::absoluteFromParentCentre)
    {
        if (xrx.isNotEmpty())
            x << bracketIfNeeded (xrx) << " + " << bracketIfNeeded (xrw) << " / 2";
        else
            x << "(getWidth() / 2)";

        const int d = roundToInt (position.rect.getX());
        if (d != 0)
            x << " + " << d;
    }

    if (w != "0")
    {
        if (position.rect.getAnchorPointX() == PositionedRectangle::anchorAtRightOrBottom)
            x << " - " << bracketIfNeeded (w);
        else if (position.rect.getAnchorPointX() == PositionedRectangle::anchorAtCentre)
            x << " - (" << bracketIfNeeded (w) << " / 2)";
    }

    // y-pos
    if (position.rect.getPositionModeY() == PositionedRectangle::proportionOfParentSize)
    {
        if (yry.isNotEmpty() && yrh.isNotEmpty())
            y << bracketIfNeeded (yry) << " + roundToInt (" << bracketIfNeeded (yrh) << " * " << CodeHelpers::floatLiteral (position.rect.getY(), 4) << ")";
        else
            y << "proportionOfHeight (" << CodeHelpers::floatLiteral (position.rect.getY(), 4) << ")";
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentTopLeft)
    {
        if (yry.isNotEmpty())
            y << bracketIfNeeded (yry) << " + ";

        y << roundToInt (position.rect.getY());
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentBottomRight)
    {
        if (yry.isNotEmpty())
            y << bracketIfNeeded (yry) << " + " << bracketIfNeeded (yrh);
        else
            y << "getHeight()";

        const int d = roundToInt (position.rect.getY());
        if (d != 0)
            y << " - " << d;
    }
    else if (position.rect.getPositionModeY() == PositionedRectangle::absoluteFromParentCentre)
    {
        if (yry.isNotEmpty())
            y << bracketIfNeeded (yry) << " + " << bracketIfNeeded (yrh) << " / 2";
        else
            y << "(getHeight() / 2)";

        const int d = roundToInt (position.rect.getY());
        if (d != 0)
            y << " + " << d;
    }

    if (h != "0")
    {
        if (position.rect.getAnchorPointY() == PositionedRectangle::anchorAtRightOrBottom)
            y << " - " << bracketIfNeeded (h);
        else if (position.rect.getAnchorPointY() == PositionedRectangle::anchorAtCentre)
            y << " - (" << bracketIfNeeded (h) << " / 2)";
    }
}
