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

#include "jucer_ComponentDocument.h"


//==============================================================================
static const char* const componentDocumentTag   = "COMPONENT";
static const char* const componentGroupTag      = "COMPONENTS";

static const char* const idProperty             = "id";
static const char* const compBoundsProperty     = "position";

//==============================================================================
static const String componentBoundsToString (const Rectangle<int>& bounds)
{
    return bounds.toString();
}

static const Rectangle<int> stringToComponentBounds (const String& s)
{
    return Rectangle<int>::fromString (s);
}

//==============================================================================
class ComponentTypeHandler
{
public:
    //==============================================================================
    ComponentTypeHandler (const String& name_, const String& tag_)
        : name (name_), tag (tag_)
    {
    }

    virtual ~ComponentTypeHandler()
    {
    }

    const String& getName() const               { return name; }
    const String& getTag() const                { return tag; }

    virtual Component* createComponent() = 0;
    virtual const Rectangle<int> getDefaultSize() = 0;

    virtual void updateComponent (Component* comp, const ValueTree& state)
    {
        comp->setBounds (stringToComponentBounds (state [compBoundsProperty]));
    }

    virtual const ValueTree createNewItem()
    {
        ValueTree v (tag);
        v.setProperty (idProperty, createAlphaNumericUID(), 0);
        v.setProperty (compBoundsProperty,
                       componentBoundsToString (getDefaultSize().withPosition (Point<int> (Random::getSystemRandom().nextInt (100) + 100,
                                                                                           Random::getSystemRandom().nextInt (100) + 100))), 0);
        return v;
    }

    //==============================================================================
protected:
    const String name, tag;
};

//==============================================================================
class TextButtonHandler  : public ComponentTypeHandler
{
public:
    TextButtonHandler() : ComponentTypeHandler ("TextButton", "TEXTBUTTON")  {}
    ~TextButtonHandler()  {}

    Component* createComponent()                { return new TextButton (String::empty); }
    const Rectangle<int> getDefaultSize()       { return Rectangle<int> (0, 0, 150, 24); }

    void updateComponent (Component* comp, const ValueTree& state)
    {
        ComponentTypeHandler::updateComponent (comp, state);
    }

    const ValueTree createNewItem()
    {
        ValueTree v (ComponentTypeHandler::createNewItem());
        return v;
    }
};

//==============================================================================
class ComponentTypeManager  : public DeletedAtShutdown
{
public:
    ComponentTypeManager()
    {
        handlers.add (new TextButtonHandler());
    }

    ~ComponentTypeManager()
    {
    }

    juce_DeclareSingleton_SingleThreaded_Minimal (ComponentTypeManager);

    Component* createFromStoredType (const ValueTree& value)
    {
        ComponentTypeHandler* handler = getHandlerFor (value.getType());
        if (handler == 0)
            return 0;

        Component* c = handler->createComponent();
        if (c != 0)
            handler->updateComponent (c, value);

        return c;
    }

    ComponentTypeHandler* getHandlerFor (const String& type)
    {
        for (int i = handlers.size(); --i >= 0;)
            if (handlers.getUnchecked(i)->getTag() == type)
                return handlers.getUnchecked(i);

        return 0;
    }

    const StringArray getTypeNames() const
    {
        StringArray s;
        for (int i = 0; i < handlers.size(); ++i)
            s.add (handlers.getUnchecked(i)->getName());

        return s;
    }

    int getNumHandlers() const                                      { return handlers.size(); }
    ComponentTypeHandler* getHandler (const int index) const        { return handlers[index]; }

private:
    OwnedArray <ComponentTypeHandler> handlers;
};

juce_ImplementSingleton_SingleThreaded (ComponentTypeManager);


//==============================================================================
ComponentDocument::ComponentDocument (Project* project_, const File& cppFile_)
   : project (project_), cppFile (cppFile_), root (componentDocumentTag)
{
    checkRootObject();
}

ComponentDocument::~ComponentDocument()
{
}

bool ComponentDocument::isComponentFile (const File& file)
{
    if (! file.hasFileExtension (".cpp"))
        return false;

    ScopedPointer <InputStream> in (file.createInputStream());

    if (in == 0)
        return false;

    const int amountToRead = 1024;
    HeapBlock <char> initialData;
    initialData.calloc (amountToRead + 4);
    in->read (initialData, amountToRead);

    return String::createStringFromData (initialData, amountToRead)
                  .contains ("JUCER_" "COMPONENT"); // written like this to avoid thinking this file is a component!
}

bool ComponentDocument::save()
{

    //XXX
    return false;
}

bool ComponentDocument::reload()
{

    //XXX
    return true;
}

bool ComponentDocument::hasChangedSinceLastSave()
{

    //XXX
    return false;
}

void ComponentDocument::checkRootObject()
{
    jassert (root.hasType (componentDocumentTag));

    if (! getComponentGroup().isValid())
        root.addChild (ValueTree (componentGroupTag), -1, 0);
}

//==============================================================================
const int menuItemOffset = 0x63451fa4;

void ComponentDocument::addNewComponentMenuItems (PopupMenu& menu) const
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    for (int i = 0; i < typeNames.size(); ++i)
        menu.addItem (i + menuItemOffset, "New " + typeNames[i]);
}

void ComponentDocument::performNewComponentMenuItem (int menuResultCode)
{
    const StringArray typeNames (ComponentTypeManager::getInstance()->getTypeNames());

    if (menuResultCode >= menuItemOffset && menuResultCode < menuItemOffset + typeNames.size())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandler (menuResultCode - menuItemOffset);
        jassert (handler != 0);

        if (handler != 0)
            getComponentGroup().addChild (handler->createNewItem(), -1, getUndoManager());
    }
}

//==============================================================================
ValueTree ComponentDocument::getComponentGroup() const
{
    return root.getChildWithName (componentGroupTag);
}

int ComponentDocument::getNumComponents() const
{
    return getComponentGroup().getNumChildren();
}

const ValueTree ComponentDocument::getComponent (int index) const
{
    return getComponentGroup().getChild (index);
}

Component* ComponentDocument::createComponent (int index) const
{
    const ValueTree v (getComponentGroup().getChild (index));

    if (v.isValid())
    {
        Component* c = ComponentTypeManager::getInstance()->createFromStoredType (v);
        c->getProperties().set (idProperty, v[idProperty]);
        jassert (c->getProperties()[idProperty].toString().isNotEmpty());
        return c;
    }

    return 0;
}

void ComponentDocument::updateComponent (Component* comp) const
{
    const ValueTree v (getComponentState (comp));

    if (v.isValid())
    {
        ComponentTypeHandler* handler = ComponentTypeManager::getInstance()->getHandlerFor (v.getType());
        jassert (handler != 0);

        if (handler != 0)
            handler->updateComponent (comp, v);
    }
}

bool ComponentDocument::containsComponent (Component* comp) const
{
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return true;

    return false;
}

const ValueTree ComponentDocument::getComponentState (Component* comp) const
{
    jassert (comp != 0);
    const ValueTree comps (getComponentGroup());

    for (int i = 0; i < comps.getNumChildren(); ++i)
        if (isStateForComponent (comps.getChild(i), comp))
            return comps.getChild(i);

    jassertfalse;
    return ValueTree::invalid;
}

bool ComponentDocument::isStateForComponent (const ValueTree& storedState, Component* comp) const
{
    jassert (comp != 0);
    jassert (! storedState [idProperty].isVoid());
    return storedState [idProperty] == comp->getProperties() [idProperty];
}

//==============================================================================
class ComponentDocument::DragHandler
{
public:
    DragHandler (ComponentDocument& document_,
                 const Array<Component*>& items,
                 const MouseEvent& e,
                 const ResizableBorderComponent::Zone& zone_)
        : document (document_),
          zone (zone_)
    {
        for (int i = 0; i < items.size(); ++i)
        {
            jassert (items.getUnchecked(i) != 0);
            const ValueTree v (document.getComponentState (items.getUnchecked(i)));
            draggedComponents.add (v);
            originalPositions.add (stringToComponentBounds (v [compBoundsProperty]));
        }
    }

    ~DragHandler()
    {
    }

    void drag (const MouseEvent& e)
    {
        for (int i = 0; i < draggedComponents.size(); ++i)
            move (draggedComponents.getReference(i), e.getOffsetFromDragStart(), originalPositions.getReference(i));
    }

    void move (ValueTree& v, const Point<int>& distance, const Rectangle<int>& originalPos)
    {
        Rectangle<int> newBounds (zone.resizeRectangleBy (originalPos, distance));
        v.setProperty (compBoundsProperty, componentBoundsToString (newBounds), document.getUndoManager());
    }

private:
    ComponentDocument& document;
    Array <ValueTree> draggedComponents;
    Array <Rectangle<int> > originalPositions;
    const ResizableBorderComponent::Zone zone;
};

void ComponentDocument::beginDrag (const Array<Component*>& items, const MouseEvent& e, const ResizableBorderComponent::Zone& zone)
{
    dragger = new DragHandler (*this, items, e, zone);
}

void ComponentDocument::continueDrag (const MouseEvent& e)
{
    if (dragger != 0)
        dragger->drag (e);
}

void ComponentDocument::endDrag (const MouseEvent& e)
{
    if (dragger != 0)
    {
        dragger->drag (e);
        dragger = 0;
    }
}
