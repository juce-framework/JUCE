/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

    ~SetPropertyAction() {}

    bool perform()
    {
        jassert (! (isAddingNewProperty && target->hasProperty (name)));

        if (isDeletingProperty)
            target->removeProperty (name, 0);
        else
            target->setProperty (name, newValue, 0);

        return true;
    }

    bool undo()
    {
        if (isAddingNewProperty)
            target->removeProperty (name, 0);
        else
            target->setProperty (name, oldValue, 0);

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

            if (next != 0 && next->target == target && next->name == name
                 && ! (next->isAddingNewProperty || next->isDeletingProperty))
            {
                return new SetPropertyAction (target, name, next->newValue, oldValue, false, false);
            }
        }

        return 0;
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
          child (newChild_ != 0 ? newChild_ : target_->children [childIndex_]),
          childIndex (childIndex_),
          isDeleting (newChild_ == 0)
    {
        jassert (child != 0);
    }

    ~AddOrRemoveChildAction() {}

    bool perform()
    {
        if (isDeleting)
            target->removeChild (childIndex, 0);
        else
            target->addChild (child, childIndex, 0);

        return true;
    }

    bool undo()
    {
        if (isDeleting)
        {
            target->addChild (child, childIndex, 0);
        }
        else
        {
            // If you hit this, it seems that your object's state is getting confused - probably
            // because you've interleaved some undoable and non-undoable operations?
            jassert (childIndex < target->children.size());
            target->removeChild (childIndex, 0);
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

    ~MoveChildAction() {}

    bool perform()
    {
        parent->moveChild (startIndex, endIndex, 0);
        return true;
    }

    bool undo()
    {
        parent->moveChild (endIndex, startIndex, 0);
        return true;
    }

    int getSizeInUnits()
    {
        return (int) sizeof (*this); //xxx should be more accurate
    }

    UndoableAction* createCoalescedAction (UndoableAction* nextAction)
    {
        MoveChildAction* next = dynamic_cast <MoveChildAction*> (nextAction);

        if (next != 0 && next->parent == parent && next->startIndex == endIndex)
            return new MoveChildAction (parent, startIndex, next->endIndex);

        return 0;
    }

private:
    const SharedObjectPtr parent;
    const int startIndex, endIndex;

    JUCE_DECLARE_NON_COPYABLE (MoveChildAction);
};


//==============================================================================
ValueTree::SharedObject::SharedObject (const Identifier& type_)
    : type (type_), parent (0)
{
}

ValueTree::SharedObject::SharedObject (const SharedObject& other)
    : type (other.type), properties (other.properties), parent (0)
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
    jassert (parent == 0); // this should never happen unless something isn't obeying the ref-counting!

    for (int i = children.size(); --i >= 0;)
    {
        const SharedObjectPtr c (children.getUnchecked(i));
        c->parent = 0;
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
        if (v != 0)
            v->listeners.call (&ValueTree::Listener::valueTreePropertyChanged, tree, property);
    }
}

void ValueTree::SharedObject::sendPropertyChangeMessage (const Identifier& property)
{
    ValueTree tree (this);
    ValueTree::SharedObject* t = this;

    while (t != 0)
    {
        t->sendPropertyChangeMessage (tree, property);
        t = t->parent;
    }
}

void ValueTree::SharedObject::sendChildChangeMessage (ValueTree& tree)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != 0)
            v->listeners.call (&ValueTree::Listener::valueTreeChildrenChanged, tree);
    }
}

void ValueTree::SharedObject::sendChildChangeMessage()
{
    ValueTree tree (this);
    ValueTree::SharedObject* t = this;

    while (t != 0)
    {
        t->sendChildChangeMessage (tree);
        t = t->parent;
    }
}

void ValueTree::SharedObject::sendParentChangeMessage()
{
    ValueTree tree (this);

    int i;
    for (i = children.size(); --i >= 0;)
    {
        SharedObject* const t = children[i];
        if (t != 0)
            t->sendParentChangeMessage();
    }

    for (i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != 0)
            v->listeners.call (&ValueTree::Listener::valueTreeParentChanged, tree);
    }
}

//==============================================================================
const var& ValueTree::SharedObject::getProperty (const Identifier& name) const
{
    return properties [name];
}

const var ValueTree::SharedObject::getProperty (const Identifier& name, const var& defaultReturnValue) const
{
    return properties.getWithDefault (name, defaultReturnValue);
}

void ValueTree::SharedObject::setProperty (const Identifier& name, const var& newValue, UndoManager* const undoManager)
{
    if (undoManager == 0)
    {
        if (properties.set (name, newValue))
            sendPropertyChangeMessage (name);
    }
    else
    {
        var* const existingValue = properties.getVarPointer (name);

        if (existingValue != 0)
        {
            if (*existingValue != newValue)
                undoManager->perform (new SetPropertyAction (this, name, newValue, properties [name], false, false));
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
    if (undoManager == 0)
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
    if (undoManager == 0)
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
            return ValueTree (static_cast <SharedObject*> (children.getUnchecked(i)));

    return ValueTree::invalid;
}

ValueTree ValueTree::SharedObject::getOrCreateChildWithName (const Identifier& typeToMatch, UndoManager* undoManager)
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->type == typeToMatch)
            return ValueTree (static_cast <SharedObject*> (children.getUnchecked(i)));

    SharedObject* const newObject = new SharedObject (typeToMatch);
    addChild (newObject, -1, undoManager);
    return ValueTree (newObject);

}

ValueTree ValueTree::SharedObject::getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->getProperty (propertyName) == propertyValue)
            return ValueTree (static_cast <SharedObject*> (children.getUnchecked(i)));

    return ValueTree::invalid;
}

bool ValueTree::SharedObject::isAChildOf (const SharedObject* const possibleParent) const
{
    const SharedObject* p = parent;

    while (p != 0)
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
    if (child != 0 && child->parent != this)
    {
        if (child != this && ! isAChildOf (child))
        {
            // You should always make sure that a child is removed from its previous parent before
            // adding it somewhere else - otherwise, it's ambiguous as to whether a different
            // undomanager should be used when removing it from its current parent..
            jassert (child->parent == 0);

            if (child->parent != 0)
            {
                jassert (child->parent->children.indexOf (child) >= 0);
                child->parent->removeChild (child->parent->children.indexOf (child), undoManager);
            }

            if (undoManager == 0)
            {
                children.insert (index, child);
                child->parent = this;
                sendChildChangeMessage();
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

    if (child != 0)
    {
        if (undoManager == 0)
        {
            children.remove (childIndex);
            child->parent = 0;
            sendChildChangeMessage();
            child->sendParentChangeMessage();
        }
        else
        {
            undoManager->perform (new AddOrRemoveChildAction (this, childIndex, 0));
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
        if (undoManager == 0)
        {
            children.move (currentIndex, newIndex);
            sendChildChangeMessage();
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

    if (undoManager == 0)
    {
        children = newOrder;
        sendChildChangeMessage();
    }
    else
    {
        for (int i = 0; i < children.size(); ++i)
        {
            if (children.getUnchecked(i) != newOrder.getUnchecked(i))
            {
                jassert (children.contains (newOrder.getUnchecked(i)));
                moveChild (children.indexOf (newOrder.getUnchecked(i)), i, undoManager);
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
ValueTree::ValueTree() throw()
    : object (0)
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
        if (object != 0)
            object->valueTreesWithListeners.removeValue (this);

        if (other.object != 0)
            other.object->valueTreesWithListeners.add (this);
    }

    object = other.object;

    return *this;
}

ValueTree::~ValueTree()
{
    if (listeners.size() > 0 && object != 0)
        object->valueTreesWithListeners.removeValue (this);
}

bool ValueTree::operator== (const ValueTree& other) const throw()
{
    return object == other.object;
}

bool ValueTree::operator!= (const ValueTree& other) const throw()
{
    return object != other.object;
}

bool ValueTree::isEquivalentTo (const ValueTree& other) const
{
    return object == other.object
            || (object != 0 && other.object != 0 && object->isEquivalentTo (*other.object));
}

ValueTree ValueTree::createCopy() const
{
    return ValueTree (object != 0 ? new SharedObject (*object) : 0);
}

bool ValueTree::hasType (const Identifier& typeName) const
{
    return object != 0 && object->type == typeName;
}

const Identifier ValueTree::getType() const
{
    return object != 0 ? object->type : Identifier();
}

ValueTree ValueTree::getParent() const
{
    return ValueTree (object != 0 ? object->parent : (SharedObject*) 0);
}

ValueTree ValueTree::getSibling (const int delta) const
{
    if (object == 0 || object->parent == 0)
        return invalid;

    const int index = object->parent->indexOf (*this) + delta;
    return ValueTree (static_cast <SharedObject*> (object->parent->children [index]));
}

const var& ValueTree::operator[] (const Identifier& name) const
{
    return object == 0 ? var::null : object->getProperty (name);
}

const var& ValueTree::getProperty (const Identifier& name) const
{
    return object == 0 ? var::null : object->getProperty (name);
}

const var ValueTree::getProperty (const Identifier& name, const var& defaultReturnValue) const
{
    return object == 0 ? defaultReturnValue : object->getProperty (name, defaultReturnValue);
}

void ValueTree::setProperty (const Identifier& name, const var& newValue, UndoManager* const undoManager)
{
    jassert (name.toString().isNotEmpty());

    if (object != 0 && name.toString().isNotEmpty())
        object->setProperty (name, newValue, undoManager);
}

bool ValueTree::hasProperty (const Identifier& name) const
{
    return object != 0 && object->hasProperty (name);
}

void ValueTree::removeProperty (const Identifier& name, UndoManager* const undoManager)
{
    if (object != 0)
        object->removeProperty (name, undoManager);
}

void ValueTree::removeAllProperties (UndoManager* const undoManager)
{
    if (object != 0)
        object->removeAllProperties (undoManager);
}

int ValueTree::getNumProperties() const
{
    return object == 0 ? 0 : object->properties.size();
}

const Identifier ValueTree::getPropertyName (const int index) const
{
    return object == 0 ? Identifier()
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

    const var getValue() const
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

    void valueTreeChildrenChanged (ValueTree&) {}
    void valueTreeParentChanged (ValueTree&)   {}

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
    return object == 0 ? 0 : object->children.size();
}

ValueTree ValueTree::getChild (int index) const
{
    return ValueTree (object != 0 ? (SharedObject*) object->children [index] : (SharedObject*) 0);
}

ValueTree ValueTree::getChildWithName (const Identifier& type) const
{
    return object != 0 ? object->getChildWithName (type) : ValueTree::invalid;
}

ValueTree ValueTree::getOrCreateChildWithName (const Identifier& type, UndoManager* undoManager)
{
    return object != 0 ? object->getOrCreateChildWithName (type, undoManager) : ValueTree::invalid;
}

ValueTree ValueTree::getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
{
    return object != 0 ? object->getChildWithProperty (propertyName, propertyValue) : ValueTree::invalid;
}

bool ValueTree::isAChildOf (const ValueTree& possibleParent) const
{
    return object != 0 && object->isAChildOf (possibleParent.object);
}

int ValueTree::indexOf (const ValueTree& child) const
{
    return object != 0 ? object->indexOf (child) : -1;
}

void ValueTree::addChild (const ValueTree& child, int index, UndoManager* const undoManager)
{
    if (object != 0)
        object->addChild (child.object, index, undoManager);
}

void ValueTree::removeChild (const int childIndex, UndoManager* const undoManager)
{
    if (object != 0)
        object->removeChild (childIndex, undoManager);
}

void ValueTree::removeChild (const ValueTree& child, UndoManager* const undoManager)
{
    if (object != 0)
        object->removeChild (object->children.indexOf (child.object), undoManager);
}

void ValueTree::removeAllChildren (UndoManager* const undoManager)
{
    if (object != 0)
        object->removeAllChildren (undoManager);
}

void ValueTree::moveChild (int currentIndex, int newIndex, UndoManager* undoManager)
{
    if (object != 0)
        object->moveChild (currentIndex, newIndex, undoManager);
}

//==============================================================================
void ValueTree::addListener (Listener* listener)
{
    if (listener != 0)
    {
        if (listeners.size() == 0 && object != 0)
            object->valueTreesWithListeners.add (this);

        listeners.add (listener);
    }
}

void ValueTree::removeListener (Listener* listener)
{
    listeners.remove (listener);

    if (listeners.size() == 0 && object != 0)
        object->valueTreesWithListeners.removeValue (this);
}

//==============================================================================
XmlElement* ValueTree::SharedObject::createXml() const
{
    XmlElement* xml = new XmlElement (type.toString());

    int i;
    for (i = 0; i < properties.size(); ++i)
    {
        Identifier name (properties.getName(i));
        const var& v = properties [name];

        jassert (! v.isObject()); // DynamicObjects can't be stored as XML!

        xml->setAttribute (name.toString(), v.toString());
    }

    for (i = 0; i < children.size(); ++i)
        xml->addChildElement (children.getUnchecked(i)->createXml());

    return xml;
}

XmlElement* ValueTree::createXml() const
{
    return object != 0 ? object->createXml() : 0;
}

ValueTree ValueTree::fromXml (const XmlElement& xml)
{
    ValueTree v (xml.getTagName());

    const int numAtts = xml.getNumAttributes(); // xxx inefficient - should write an att iterator..

    for (int i = 0; i < numAtts; ++i)
        v.setProperty (xml.getAttributeName (i), var (xml.getAttributeValue (i)), 0);

    forEachXmlChildElement (xml, e)
    {
        v.addChild (fromXml (*e), -1, 0);
    }

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
        v.setProperty (name, value, 0);
    }

    const int numChildren = input.readCompressedInt();

    for (i = 0; i < numChildren; ++i)
        v.addChild (readFromStream (input), -1, 0);

    return v;
}

ValueTree ValueTree::readFromData (const void* const data, const size_t numBytes)
{
    MemoryInputStream in (data, numBytes, false);
    return readFromStream (in);
}

END_JUCE_NAMESPACE
