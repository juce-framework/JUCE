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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ValueTree.h"


//==============================================================================
class ValueTreeSetPropertyAction  : public UndoableAction
{
public:
    ValueTreeSetPropertyAction (const ValueTree::SharedObjectPtr& target_, const var::identifier& name_,
                                const var& newValue_, const bool isAddingNewProperty_, const bool isDeletingProperty_)
        : target (target_), name (name_), newValue (newValue_),
          isAddingNewProperty (isAddingNewProperty_),
          isDeletingProperty (isDeletingProperty_)
    {
        if (! isAddingNewProperty)
            oldValue = target_->getProperty (name_);
    }

    ~ValueTreeSetPropertyAction() {}

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

private:
    const ValueTree::SharedObjectPtr target;
    const var::identifier name;
    const var newValue;
    var oldValue;
    const bool isAddingNewProperty, isDeletingProperty;

    ValueTreeSetPropertyAction (const ValueTreeSetPropertyAction&);
    const ValueTreeSetPropertyAction& operator= (const ValueTreeSetPropertyAction&);
};

//==============================================================================
class ValueTreeChildChangeAction  : public UndoableAction
{
public:
    ValueTreeChildChangeAction (const ValueTree::SharedObjectPtr& target_, const int childIndex_,
                                const ValueTree::SharedObjectPtr& newChild_)
        : target (target_),
          child (newChild_ != 0 ? newChild_ : target_->children [childIndex_]),
          childIndex (childIndex_),
          isDeleting (newChild_ == 0)
    {
        jassert (child != 0);
    }

    ~ValueTreeChildChangeAction() {}

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
            target->addChild (child, childIndex, 0);
        else
            target->removeChild (childIndex, 0);

        return true;
    }

    int getSizeInUnits()
    {
        return (int) sizeof (*this); //xxx should be more accurate
    }

private:
    const ValueTree::SharedObjectPtr target, child;
    const int childIndex;
    const bool isDeleting;

    ValueTreeChildChangeAction (const ValueTreeChildChangeAction&);
    const ValueTreeChildChangeAction& operator= (const ValueTreeChildChangeAction&);
};


//==============================================================================
ValueTree::SharedObject::SharedObject (const String& type_)
    : type (type_), parent (0)
{
}

ValueTree::SharedObject::SharedObject (const SharedObject& other)
    : type (other.type), parent (0)
{
    int i;
    for (i = 0; i < other.properties.size(); ++i)
    {
        const Property* const p = other.properties.getUnchecked(i);
        properties.add (new Property (p->name, p->value));
    }

    for (i = 0; i < other.children.size(); ++i)
        children.add (new SharedObject (*other.children.getUnchecked(i)));
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

ValueTree::SharedObject::Property::Property (const var::identifier& name_, const var& value_)
    : name (name_), value (value_)
{
}

//==============================================================================
void ValueTree::deliverPropertyChangeMessage (ValueTree& tree, const var::identifier& property)
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ValueTree::Listener* const l = listeners[i];
        if (l != 0)
            l->valueTreePropertyChanged (tree, property);
    }
}

void ValueTree::SharedObject::sendPropertyChangeMessage (ValueTree& tree, const var::identifier& property)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != 0)
            v->deliverPropertyChangeMessage (tree, property);
    }
}

void ValueTree::SharedObject::sendPropertyChangeMessage (const var::identifier& property)
{
    ValueTree tree (this);
    ValueTree::SharedObject* t = this;

    while (t != 0)
    {
        t->sendPropertyChangeMessage (tree, property);
        t = t->parent;
    }
}

void ValueTree::deliverChildChangeMessage (ValueTree& tree)
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ValueTree::Listener* const l = listeners[i];
        if (l != 0)
            l->valueTreeChildrenChanged (tree);
    }
}

void ValueTree::SharedObject::sendChildChangeMessage (ValueTree& tree)
{
    for (int i = valueTreesWithListeners.size(); --i >= 0;)
    {
        ValueTree* const v = valueTreesWithListeners[i];
        if (v != 0)
            v->deliverChildChangeMessage (tree);
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

void ValueTree::deliverParentChangeMessage (ValueTree& tree)
{
    for (int i = listeners.size(); --i >= 0;)
    {
        ValueTree::Listener* const l = listeners[i];
        if (l != 0)
            l->valueTreeParentChanged (tree);
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
            v->deliverParentChangeMessage (tree);
    }
}

//==============================================================================
const var ValueTree::SharedObject::getProperty (const var::identifier& name) const
{
    for (int i = properties.size(); --i >= 0;)
    {
        const Property* const p = properties.getUnchecked(i);
        if (p->name == name)
            return p->value;
    }

    return var();
}

void ValueTree::SharedObject::setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager)
{
    for (int i = properties.size(); --i >= 0;)
    {
        Property* const p = properties.getUnchecked(i);

        if (p->name == name)
        {
            if (p->value != newValue)
            {
                if (undoManager == 0)
                {
                    p->value = newValue;
                    sendPropertyChangeMessage (name);
                }
                else
                {
                    undoManager->perform (new ValueTreeSetPropertyAction (this, name, newValue, false, false));
                }
            }

            return;
        }
    }

    if (undoManager == 0)
    {
        properties.add (new Property (name, newValue));
        sendPropertyChangeMessage (name);
    }
    else
    {
        undoManager->perform (new ValueTreeSetPropertyAction (this, name, newValue, true, false));
    }
}

bool ValueTree::SharedObject::hasProperty (const var::identifier& name) const
{
    for (int i = properties.size(); --i >= 0;)
        if (properties.getUnchecked(i)->name == name)
            return true;

    return false;
}

void ValueTree::SharedObject::removeProperty (const var::identifier& name, UndoManager* const undoManager)
{
    for (int i = properties.size(); --i >= 0;)
    {
        Property* const p = properties.getUnchecked(i);

        if (p->name == name)
        {
            if (undoManager == 0)
            {
                properties.remove (i);
                sendPropertyChangeMessage (name);
            }
            else
            {
                undoManager->perform (new ValueTreeSetPropertyAction (this, name, var(), false, true));
            }

            break;
        }
    }
}

void ValueTree::SharedObject::removeAllProperties (UndoManager* const undoManager)
{
    if (undoManager == 0)
    {
        while (properties.size() > 0)
        {
            const var::identifier name (properties.getLast()->name);
            properties.removeLast();
            sendPropertyChangeMessage (name);
        }
    }
    else
    {
        for (int i = properties.size(); --i >= 0;)
            undoManager->perform (new ValueTreeSetPropertyAction (this, properties.getUnchecked(i)->name, var(), false, true));
    }
}

ValueTree ValueTree::SharedObject::getChildWithName (const String& typeToMatch) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->type == typeToMatch)
            return (SharedObject*) children.getUnchecked(i);

    return (SharedObject*) 0;
}

ValueTree ValueTree::SharedObject::getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i)->getProperty (propertyName) == propertyValue)
            return (SharedObject*) children.getUnchecked(i);

    return (SharedObject*) 0;
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
                undoManager->perform (new ValueTreeChildChangeAction (this, index, child));
            }
        }
        else
        {
            // You're attempting to create a recursive loop! A node
            // can't be a child of one of its own children!
            jassertfalse
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
            undoManager->perform (new ValueTreeChildChangeAction (this, childIndex, 0));
        }
    }
}

void ValueTree::SharedObject::removeAllChildren (UndoManager* const undoManager)
{
    while (children.size() > 0)
        removeChild (children.size() - 1, undoManager);
}


//==============================================================================
ValueTree::ValueTree (const String& type_)
    : object (new ValueTree::SharedObject (type_))
{
    jassert (type_.isNotEmpty()); // All objects should be given a sensible type name!
}

ValueTree::ValueTree (SharedObject* const object_)
    : object (object_)
{
}

ValueTree::ValueTree (const ValueTree& other)
    : object (other.object)
{
}

const ValueTree& ValueTree::operator= (const ValueTree& other)
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

bool ValueTree::operator== (const ValueTree& other) const
{
    return object == other.object;
}

bool ValueTree::operator!= (const ValueTree& other) const
{
    return object != other.object;
}

ValueTree ValueTree::createCopy() const
{
    return ValueTree (object != 0 ? new SharedObject (*object) : 0);
}

bool ValueTree::hasType (const String& typeName) const
{
    return object != 0 && object->type == typeName;
}

const String ValueTree::getType() const
{
    return object != 0 ? object->type : String::empty;
}

ValueTree ValueTree::getParent() const
{
    return object != 0 ? ValueTree (object->parent) : ValueTree ((SharedObject*) 0);
}

const var ValueTree::operator[] (const var::identifier& name) const
{
    return object == 0 ? var() : object->getProperty (name);
}

const var ValueTree::getProperty (const var::identifier& name) const
{
    return object == 0 ? var() : object->getProperty (name);
}

void ValueTree::setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager)
{
    jassert (name.name.isNotEmpty());

    if (object != 0 && name.name.isNotEmpty())
        object->setProperty (name, newValue, undoManager);
}

bool ValueTree::hasProperty (const var::identifier& name) const
{
    return object != 0 && object->hasProperty (name);
}

void ValueTree::removeProperty (const var::identifier& name, UndoManager* const undoManager)
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

const var::identifier ValueTree::getPropertyName (int index) const
{
    const SharedObject::Property* const p = (object == 0) ? 0 : object->properties [index];
    return p != 0 ? p->name : var::identifier (String::empty);
}

//==============================================================================
class ValueTreePropertyValueSource  : public Value::ValueSource,
                                      public ValueTree::Listener
{
public:
    ValueTreePropertyValueSource (const ValueTree& tree_,
                                  const var::identifier& property_,
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

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& changedProperty)
    {
        if (tree == treeWhosePropertyHasChanged && property == changedProperty)
            sendChangeMessage (false);
    }

    void valueTreeChildrenChanged (ValueTree&) {}
    void valueTreeParentChanged (ValueTree&)   {}

private:
    ValueTree tree;
    const var::identifier property;
    UndoManager* const undoManager;

    const ValueTreePropertyValueSource& operator= (const ValueTreePropertyValueSource&);
};

Value ValueTree::getPropertyAsValue (const var::identifier& name, UndoManager* const undoManager) const
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
    return object != 0 ? (SharedObject*) object->children [index] : ValueTree ((SharedObject*) 0);
}

ValueTree ValueTree::getChildWithName (const String& type) const
{
    return object != 0 ? object->getChildWithName (type) : ValueTree ((SharedObject*) 0);
}

ValueTree ValueTree::getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const
{
    return object != 0 ? object->getChildWithProperty (propertyName, propertyValue) : ValueTree ((SharedObject*) 0);
}

bool ValueTree::isAChildOf (const ValueTree& possibleParent) const
{
    return object != 0 && object->isAChildOf (possibleParent.object);
}

void ValueTree::addChild (ValueTree child, int index, UndoManager* const undoManager)
{
    if (object != 0)
        object->addChild (child.object, index, undoManager);
}

void ValueTree::removeChild (const int childIndex, UndoManager* const undoManager)
{
    if (object != 0)
        object->removeChild (childIndex, undoManager);
}

void ValueTree::removeChild (ValueTree& child, UndoManager* const undoManager)
{
    if (object != 0)
        object->removeChild (object->children.indexOf (child.object), undoManager);
}

void ValueTree::removeAllChildren (UndoManager* const undoManager)
{
    if (object != 0)
        object->removeAllChildren (undoManager);
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
    listeners.removeValue (listener);

    if (listeners.size() == 0 && object != 0)
        object->valueTreesWithListeners.removeValue (this);
}

//==============================================================================
XmlElement* ValueTree::SharedObject::createXml() const
{
    XmlElement* xml = new XmlElement (type);

    int i;
    for (i = 0; i < properties.size(); ++i)
    {
        const Property* const p = properties.getUnchecked(i);

        jassert (! p->value.isObject()); // DynamicObjects can't be stored as XML!

        xml->setAttribute (p->name.name, p->value.toString());
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
        v.setProperty (xml.getAttributeName (i), xml.getAttributeValue (i), 0);

    forEachXmlChildElement (xml, e)
    {
        v.addChild (fromXml (*e), -1, 0);
    }

    return v;
}

//==============================================================================
void ValueTree::writeToStream (OutputStream& output)
{
    output.writeString (getType());

    const int numProps = getNumProperties();
    output.writeCompressedInt (numProps);

    int i;
    for (i = 0; i < numProps; ++i)
    {
        const var::identifier name (getPropertyName(i));
        output.writeString (name.name);
        getProperty(name).writeToStream (output);
    }

    const int numChildren = getNumChildren();
    output.writeCompressedInt (numChildren);

    for (i = 0; i < numChildren; ++i)
        getChild (i).writeToStream (output);
}

ValueTree ValueTree::readFromStream (InputStream& input)
{
    String type (input.readString());

    if (type.isEmpty())
        return ValueTree ((SharedObject*) 0);

    ValueTree v (type);

    const int numProps = input.readCompressedInt();

    if (numProps < 0)
    {
        jassertfalse  // trying to read corrupted data!
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

END_JUCE_NAMESPACE
