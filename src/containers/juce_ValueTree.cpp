/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ValueTree.h"
#include "../io/streams/juce_MemoryInputStream.h"


//==============================================================================
class ValueTree::SetPropertyAction  : public UndoableAction
{
public:
    SetPropertyAction (const SharedObjectPtr& target_, const Identifier& name_,
                       const var& newValue_, const var& oldValue_,
                       const bool isAddingNewProperty_, const bool isDeletingProperty_)
        : target (target_), name (name_), newValue (newValue_), oldValue (oldValue_),
          isAddingNewProperty (isAddingNewProperty_), isDeletingProperty (isDeletingProperty_)
    {
    }

    bool perform()
    {
        jassert (! (isAddingNewProperty && target->hasProperty (name)));

        if (isDeletingProperty)
            target->removeProperty (name, nullptr);
        else
            target->setProperty (name, newValue, nullptr);

        return true;
    }

    bool undo()
    {
        if (isAddingNewProperty)
            target->removeProperty (name, nullptr);
        else
            target->setProperty (name, oldValue, nullptr);

        return true;
    }

    int getSizeInUnits()
    {
        return (int) sizeof (*this); //xxx should be more accurate
    }

    UndoableAction* createCoalescedAction (UndoableAction* nextAction)
    {
        if (! (isAddingNewProperty || isDeletingProperty))
        {
            SetPropertyAction* next = dynamic_cast <SetPropertyAction*> (nextAction);

            if (next != nullptr && next->target == target && next->name == name
                 && ! (next->isAddingNewProperty || next->isDeletingProperty))
            {
                return new SetPropertyAction (target, name, next->newValue, oldValue, false, false);
            }
        }

        return nullptr;
    }

private:
    const SharedObjectPtr target;
    const Identifier name;
    const var newValue;
    var oldValue;
    const bool isAddingNewProperty : 1, isDeletingProperty : 1;

    JUCE_DECLARE_NON_COPYABLE (SetPropertyAction);
};

//==============================================================================
class ValueTree::AddOrRemoveChildAction  : public UndoableAction
{
public:
    AddOrRemoveChildAction (const SharedObjectPtr& target_, const int childIndex_,
                            const SharedObjectPtr& newChild_)
        : target (target_),
          child (newChild_ != nullptr ? newChild_ : target_->children [childIndex_]),
          childIndex (childIndex_),
          isDeleting (newChild_ == nullptr)
    {
        jassert (child != nullptr);
    }

    bool perform()
    {
        if (isDeleting)
            target->removeChild (childIndex, nullptr);
        else
            target->addChild (child, childIndex, nullptr);

        return true;
    }

    bool undo()
    {
        if (isDeleting)
        {
            target->addChild (child, childIndex, nullptr);
        }
        else
        {
            // If you hit this, it seems that your object's state is getting confused - probably
            // because you've interleaved some undoable and non-undoable operations?
            jassert (childIndex < target->children.size());
            target->removeChild (childIndex, nullptr);
        }

        return true;
    }

    int getSizeInUnits()
    {
        return (int) sizeof (*this); //xxx should be more accurate
    }

private:
    const SharedObjectPtr target, child;
    const int childIndex;
    const bool isDeleting;

    JUCE_DECLARE_NON_COPYABLE (AddOrRemoveChildAction);
};

//==============================================================================
class ValueTree::MoveChildAction  : public UndoableAction
{
public:
    MoveChildAction (const SharedObjectPtr& parent_,
                     const int startIndex_, const int endIndex_)
        : parent (parent_),
          startIndex (startIndex_),
          endIndex (endIndex_)
    {
    }

    bool perform()
    {
        parent->moveChild (startIndex, endIndex, nullptr);
        return true;
    }

    bool undo()
    {
        parent->moveChild (endIndex, startIndex, nullptr);
        return true;
    }

    int getSizeInUnits()
    {
        return (int) sizeof (*this); //xxx should be more accurate
    }

    UndoableAction* createCoalescedAction (UndoableAction* nextAction)
    {
        MoveChildAction* next = dynamic_cast <MoveChildAction*> (nextAction);

        if (next != nullptr && next->parent == parent && next->startIndex == endIndex)
            return new MoveChildAction (parent, startIndex, next->endIndex);

        return nullptr;
    }

private:
    const SharedObjectPtr parent;
    const int startIndex, endIndex;

    JUCE_DECLARE_NON_COPYABLE (MoveChildAction);
};


//==============================================================================
ValueTree::SharedObject::SharedObject (const Identifier& type_)
    : type (type_), parent (nullptr)
{
}

ValueTree::SharedObject::SharedObject (const SharedObject& other)
    : type (other.type), properties (other.properties), parent (nullptr)
{
    for (int i = 0; i < other.children.size(); ++i)
    {
        SharedObject* const child = new SharedObject (*other.children.getUnchecked(i));
        child->parent = this;
        children.add (child);
    }
}

ValueTree::SharedObject::~SharedObject()
{
    jassert (parent == nullptr); // this should never happen unless something isn't obeying the ref-counting!

    for (int i = children.size(); --i >= 0;)
    {
        const SharedObjectPtr c (children.getUnchecked(i));
        c->parent = nullptr;
        children.remove (i);
        c->sendParentChangeMessage();
    }
}

//==============================================================================
void ValueTree::SharedObject::sendPropertyChangeMessage (ValueTree& tree, const Identifier& property)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != nullptr)
            v->listeners.call (&ValueTree::Listener::valueTreePropertyChanged, tree, property);
    }
}

void ValueTree::SharedObject::sendPropertyChangeMessage (const Identifier& property)
{
    ValueTree tree (this);

    for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
        t->sendPropertyChangeMessage (tree, property);
}

void ValueTree::SharedObject::sendChildAddedMessage (ValueTree& tree, ValueTree& child)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != nullptr)
            v->listeners.call (&ValueTree::Listener::valueTreeChildAdded, tree, child);
    }
}

void ValueTree::SharedObject::sendChildAddedMessage (ValueTree child)
{
    ValueTree tree (this);

    for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
        t->sendChildAddedMessage (tree, child);
}

void ValueTree::SharedObject::sendChildRemovedMessage (ValueTree& tree, ValueTree& child)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != nullptr)
            v->listeners.call (&ValueTree::Listener::valueTreeChildRemoved, tree, child);
    }
}

void ValueTree::SharedObject::sendChildRemovedMessage (ValueTree child)
{
    ValueTree tree (this);

    for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
        t->sendChildRemovedMessage (tree, child);
}

void ValueTree::SharedObject::sendChildOrderChangedMessage (ValueTree& tree)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != nullptr)
            v->listeners.call (&ValueTree::Listener::valueTreeChildOrderChanged, tree);
    }
}

void ValueTree::SharedObject::sendChildOrderChangedMessage()
{
    ValueTree tree (this);

    for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
        t->sendChildOrderChangedMessage (tree);
}

void ValueTree::SharedObject::sendParentChangeMessage()
{
    ValueTree tree (this);

    int i;
    for (i = children.size(); --i >= 0;)
    {
        SharedObject* const t = children[i];
        if (t != nullptr)
            t->sendParentChangeMessage();
    }

    for (i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != nullptr)
            v->listeners.call (&ValueTree::Listener::valueTreeParentChanged, tree);
    }
}

//==============================================================================
const var& ValueTree::SharedObject::getProperty (const Identifier& name) const
{
    return properties [name];
}

var ValueTree::SharedObject::getProperty (const Identifier& name, const var& defaultReturnValue) const
{
    return properties.getWithDefault (name, defaultReturnValue);
}

void ValueTree::SharedObject::setProperty (const Identifier& name, const var& newValue, UndoManager* const undoManager)
{
    if (undoManager == nullptr)
    {
        if (properties.set (name, newValue))
            sendPropertyChangeMessage (name);
    }
    else
    {
        const var* const existingValue = properties.getVarPointer (name);

        if (existingValue != nullptr)
        {
            if (*existingValue != newValue)
                undoManager->perform (new SetPropertyAction (this, name, newValue, *existingValue, false, false));
        }
        else
        {
            undoManager->perform (new SetPropertyAction (this, name, newValue, var::null, true, false));
        }
    }
}

bool ValueTree::SharedObject::hasProperty (const Identifier& name) const
{
    return properties.contains (name);
}

void ValueTree::SharedObject::removeProperty (const Identifier& name, UndoManager* const undoManager)
{
    if (undoManager == nullptr)
    {
        if (properties.remove (name))
            sendPropertyChangeMessage (name);
    }
    else
    {
        if (properties.contains (name))
            undoManager->perform (new SetPropertyAction (this, name, var::null, properties [name], false, true));
    }
}

void ValueTree::SharedObject::removeAllProperties (UndoManager* const undoManager)
{
    if (undoManager == nullptr)
    {
        while (properties.size() > 0)
        {
            const Identifier name (properties.getName (properties.size() - 1));
            properties.remove (name);
            sendPropertyChangeMessage (name);
        }
    }
    else
    {
        for (int i = properties.size(); --i >= 0;)
            undoManager->perform (new SetPropertyAction (this, properties.getName(i), var::null, properties.getValueAt(i), false, true));
    }
}

ValueTree ValueTree::SharedObject::getChildWithName (const Identifier& typeToMatch) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->type == typeToMatch)
            return ValueTree (children.getUnchecked(i).getObject());

    return ValueTree::invalid;
}

ValueTree ValueTree::SharedObject::getOrCreateChildWithName (const Identifier& typeToMatch, UndoManager* undoManager)
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->type == typeToMatch)
            return ValueTree (children.getUnchecked(i).getObject());

    SharedObject* const newObject = new SharedObject (typeToMatch);
    addChild (newObject, -1, undoManager);
    return ValueTree (newObject);

}

ValueTree ValueTree::SharedObject::getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->getProperty (propertyName) == propertyValue)
            return ValueTree (children.getUnchecked(i).getObject());

    return ValueTree::invalid;
}

bool ValueTree::SharedObject::isAChildOf (const SharedObject* const possibleParent) const
{
    const SharedObject* p = parent;

    while (p != nullptr)
    {
        if (p == possibleParent)
            return true;

        p = p->parent;
    }

    return false;
}

int ValueTree::SharedObject::indexOf (const ValueTree& child) const
{
    return children.indexOf (child.object);
}

void ValueTree::SharedObject::addChild (SharedObject* child, int index, UndoManager* const undoManager)
{
    if (child != nullptr && child->parent != this)
    {
        if (child != this && ! isAChildOf (child))
        {
            // You should always make sure that a child is removed from its previous parent before
            // adding it somewhere else - otherwise, it's ambiguous as to whether a different
            // undomanager should be used when removing it from its current parent..
            jassert (child->parent == nullptr);

            if (child->parent != nullptr)
            {
                jassert (child->parent->children.indexOf (child) >= 0);
                child->parent->removeChild (child->parent->children.indexOf (child), undoManager);
            }

            if (undoManager == nullptr)
            {
                children.insert (index, child);
                child->parent = this;
                sendChildAddedMessage (ValueTree (child));
                child->sendParentChangeMessage();
            }
            else
            {
                if (index < 0)
                    index = children.size();

                undoManager->perform (new AddOrRemoveChildAction (this, index, child));
            }
        }
        else
        {
            // You're attempting to create a recursive loop! A node
            // can't be a child of one of its own children!
            jassertfalse;
        }
    }
}

void ValueTree::SharedObject::removeChild (const int childIndex, UndoManager* const undoManager)
{
    const SharedObjectPtr child (children [childIndex]);

    if (child != nullptr)
    {
        if (undoManager == nullptr)
        {
            children.remove (childIndex);
            child->parent = nullptr;
            sendChildRemovedMessage (ValueTree (child));
            child->sendParentChangeMessage();
        }
        else
        {
            undoManager->perform (new AddOrRemoveChildAction (this, childIndex, nullptr));
        }
    }
}

void ValueTree::SharedObject::removeAllChildren (UndoManager* const undoManager)
{
    while (children.size() > 0)
        removeChild (children.size() - 1, undoManager);
}

void ValueTree::SharedObject::moveChild (int currentIndex, int newIndex, UndoManager* undoManager)
{
    // The source index must be a valid index!
    jassert (isPositiveAndBelow (currentIndex, children.size()));

    if (currentIndex != newIndex
         && isPositiveAndBelow (currentIndex, children.size()))
    {
        if (undoManager == nullptr)
        {
            children.move (currentIndex, newIndex);
            sendChildOrderChangedMessage();
        }
        else
        {
            if (! isPositiveAndBelow (newIndex, children.size()))
                newIndex = children.size() - 1;

            undoManager->perform (new MoveChildAction (this, currentIndex, newIndex));
        }
    }
}

void ValueTree::SharedObject::reorderChildren (const ReferenceCountedArray <SharedObject>& newOrder, UndoManager* undoManager)
{
    jassert (newOrder.size() == children.size());

    if (undoManager == nullptr)
    {
        children = newOrder;
        sendChildOrderChangedMessage();
    }
    else
    {
        for (int i = 0; i < children.size(); ++i)
        {
            const SharedObjectPtr child (newOrder.getUnchecked(i));

            if (children.getUnchecked(i) != child)
            {
                const int oldIndex = children.indexOf (child);
                jassert (oldIndex >= 0);
                moveChild (oldIndex, i, undoManager);
            }
        }
    }
}

bool ValueTree::SharedObject::isEquivalentTo (const SharedObject& other) const
{
    if (type != other.type
         || properties.size() != other.properties.size()
         || children.size() != other.children.size()
         || properties != other.properties)
        return false;

    for (int i = 0; i < children.size(); ++i)
        if (! children.getUnchecked(i)->isEquivalentTo (*other.children.getUnchecked(i)))
            return false;

    return true;
}


//==============================================================================
ValueTree::ValueTree() noexcept
{
}

const ValueTree ValueTree::invalid;

ValueTree::ValueTree (const Identifier& type_)
    : object (new ValueTree::SharedObject (type_))
{
    jassert (type_.toString().isNotEmpty()); // All objects should be given a sensible type name!
}

ValueTree::ValueTree (SharedObject* const object_)
    : object (object_)
{
}

ValueTree::ValueTree (const ValueTree& other)
    : object (other.object)
{
}

ValueTree& ValueTree::operator= (const ValueTree& other)
{
    if (listeners.size() > 0)
    {
        if (object != nullptr)
            object->valueTreesWithListeners.removeValue (this);

        if (other.object != nullptr)
            other.object->valueTreesWithListeners.add (this);
    }

    object = other.object;

    return *this;
}

ValueTree::~ValueTree()
{
    if (listeners.size() > 0 && object != nullptr)
        object->valueTreesWithListeners.removeValue (this);
}

bool ValueTree::operator== (const ValueTree& other) const noexcept
{
    return object == other.object;
}

bool ValueTree::operator!= (const ValueTree& other) const noexcept
{
    return object != other.object;
}

bool ValueTree::isEquivalentTo (const ValueTree& other) const
{
    return object == other.object
            || (object != nullptr && other.object != nullptr && object->isEquivalentTo (*other.object));
}

ValueTree ValueTree::createCopy() const
{
    return ValueTree (object != nullptr ? new SharedObject (*object) : nullptr);
}

bool ValueTree::hasType (const Identifier& typeName) const
{
    return object != nullptr && object->type == typeName;
}

Identifier ValueTree::getType() const
{
    return object != nullptr ? object->type : Identifier();
}

ValueTree ValueTree::getParent() const
{
    return ValueTree (object != nullptr ? object->parent : (SharedObject*) nullptr);
}

ValueTree ValueTree::getSibling (const int delta) const
{
    if (object == nullptr || object->parent == nullptr)
        return invalid;

    const int index = object->parent->indexOf (*this) + delta;
    return ValueTree (object->parent->children [index].getObject());
}

const var& ValueTree::operator[] (const Identifier& name) const
{
    return object == nullptr ? var::null : object->getProperty (name);
}

const var& ValueTree::getProperty (const Identifier& name) const
{
    return object == nullptr ? var::null : object->getProperty (name);
}

var ValueTree::getProperty (const Identifier& name, const var& defaultReturnValue) const
{
    return object == nullptr ? defaultReturnValue : object->getProperty (name, defaultReturnValue);
}

void ValueTree::setProperty (const Identifier& name, const var& newValue, UndoManager* const undoManager)
{
    jassert (name.toString().isNotEmpty());

    if (object != nullptr && name.toString().isNotEmpty())
        object->setProperty (name, newValue, undoManager);
}

bool ValueTree::hasProperty (const Identifier& name) const
{
    return object != nullptr && object->hasProperty (name);
}

void ValueTree::removeProperty (const Identifier& name, UndoManager* const undoManager)
{
    if (object != nullptr)
        object->removeProperty (name, undoManager);
}

void ValueTree::removeAllProperties (UndoManager* const undoManager)
{
    if (object != nullptr)
        object->removeAllProperties (undoManager);
}

int ValueTree::getNumProperties() const
{
    return object == nullptr ? 0 : object->properties.size();
}

Identifier ValueTree::getPropertyName (const int index) const
{
    return object == nullptr ? Identifier()
                             : object->properties.getName (index);
}

//==============================================================================
class ValueTreePropertyValueSource  : public Value::ValueSource,
                                      public ValueTree::Listener
{
public:
    ValueTreePropertyValueSource (const ValueTree& tree_,
                                  const Identifier& property_,
                                  UndoManager* const undoManager_)
        : tree (tree_),
          property (property_),
          undoManager (undoManager_)
    {
        tree.addListener (this);
    }

    ~ValueTreePropertyValueSource()
    {
        tree.removeListener (this);
    }

    var getValue() const
    {
        return tree [property];
    }

    void setValue (const var& newValue)
    {
        tree.setProperty (property, newValue, undoManager);
    }

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const Identifier& changedProperty)
    {
        if (tree == treeWhosePropertyHasChanged && property == changedProperty)
            sendChangeMessage (false);
    }

    void valueTreeChildAdded (ValueTree&, ValueTree&) {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&) {}
    void valueTreeChildOrderChanged (ValueTree&) {}
    void valueTreeParentChanged (ValueTree&) {}

private:
    ValueTree tree;
    const Identifier property;
    UndoManager* const undoManager;

    ValueTreePropertyValueSource& operator= (const ValueTreePropertyValueSource&);
};

Value ValueTree::getPropertyAsValue (const Identifier& name, UndoManager* const undoManager) const
{
    return Value (new ValueTreePropertyValueSource (*this, name, undoManager));
}

//==============================================================================
int ValueTree::getNumChildren() const
{
    return object == nullptr ? 0 : object->children.size();
}

ValueTree ValueTree::getChild (int index) const
{
    return ValueTree (object != nullptr ? (SharedObject*) object->children [index] : (SharedObject*) nullptr);
}

ValueTree ValueTree::getChildWithName (const Identifier& type) const
{
    return object != nullptr ? object->getChildWithName (type) : ValueTree::invalid;
}

ValueTree ValueTree::getOrCreateChildWithName (const Identifier& type, UndoManager* undoManager)
{
    return object != nullptr ? object->getOrCreateChildWithName (type, undoManager) : ValueTree::invalid;
}

ValueTree ValueTree::getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
{
    return object != nullptr ? object->getChildWithProperty (propertyName, propertyValue) : ValueTree::invalid;
}

bool ValueTree::isAChildOf (const ValueTree& possibleParent) const
{
    return object != nullptr && object->isAChildOf (possibleParent.object);
}

int ValueTree::indexOf (const ValueTree& child) const
{
    return object != nullptr ? object->indexOf (child) : -1;
}

void ValueTree::addChild (const ValueTree& child, int index, UndoManager* const undoManager)
{
    if (object != nullptr)
        object->addChild (child.object, index, undoManager);
}

void ValueTree::removeChild (const int childIndex, UndoManager* const undoManager)
{
    if (object != nullptr)
        object->removeChild (childIndex, undoManager);
}

void ValueTree::removeChild (const ValueTree& child, UndoManager* const undoManager)
{
    if (object != nullptr)
        object->removeChild (object->children.indexOf (child.object), undoManager);
}

void ValueTree::removeAllChildren (UndoManager* const undoManager)
{
    if (object != nullptr)
        object->removeAllChildren (undoManager);
}

void ValueTree::moveChild (int currentIndex, int newIndex, UndoManager* undoManager)
{
    if (object != nullptr)
        object->moveChild (currentIndex, newIndex, undoManager);
}

//==============================================================================
void ValueTree::addListener (Listener* listener)
{
    if (listener != nullptr)
    {
        if (listeners.size() == 0 && object != nullptr)
            object->valueTreesWithListeners.add (this);

        listeners.add (listener);
    }
}

void ValueTree::removeListener (Listener* listener)
{
    listeners.remove (listener);

    if (listeners.size() == 0 && object != nullptr)
        object->valueTreesWithListeners.removeValue (this);
}

//==============================================================================
XmlElement* ValueTree::SharedObject::createXml() const
{
    XmlElement* const xml = new XmlElement (type.toString());
    properties.copyToXmlAttributes (*xml);

    for (int i = 0; i < children.size(); ++i)
        xml->addChildElement (children.getUnchecked(i)->createXml());

    return xml;
}

XmlElement* ValueTree::createXml() const
{
    return object != nullptr ? object->createXml() : nullptr;
}

ValueTree ValueTree::fromXml (const XmlElement& xml)
{
    ValueTree v (xml.getTagName());
    v.object->properties.setFromXmlAttributes (xml);

    forEachXmlChildElement (xml, e)
        v.addChild (fromXml (*e), -1, nullptr);

    return v;
}

//==============================================================================
void ValueTree::writeToStream (OutputStream& output)
{
    output.writeString (getType().toString());

    const int numProps = getNumProperties();
    output.writeCompressedInt (numProps);

    int i;
    for (i = 0; i < numProps; ++i)
    {
        const Identifier name (getPropertyName(i));
        output.writeString (name.toString());
        getProperty(name).writeToStream (output);
    }

    const int numChildren = getNumChildren();
    output.writeCompressedInt (numChildren);

    for (i = 0; i < numChildren; ++i)
        getChild (i).writeToStream (output);
}

ValueTree ValueTree::readFromStream (InputStream& input)
{
    const String type (input.readString());

    if (type.isEmpty())
        return ValueTree::invalid;

    ValueTree v (type);

    const int numProps = input.readCompressedInt();

    if (numProps < 0)
    {
        jassertfalse;  // trying to read corrupted data!
        return v;
    }

    int i;
    for (i = 0; i < numProps; ++i)
    {
        const String name (input.readString());
        jassert (name.isNotEmpty());
        const var value (var::readFromStream (input));
        v.object->properties.set (name, value);
    }

    const int numChildren = input.readCompressedInt();

    for (i = 0; i < numChildren; ++i)
    {
        ValueTree child (readFromStream (input));

        v.object->children.add (child.object);
        child.object->parent = v.object;
    }

    return v;
}

ValueTree ValueTree::readFromData (const void* const data, const size_t numBytes)
{
    MemoryInputStream in (data, numBytes, false);
    return readFromStream (in);
}

END_JUCE_NAMESPACE
