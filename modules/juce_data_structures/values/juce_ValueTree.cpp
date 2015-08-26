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

class ValueTree::SharedObject  : public ReferenceCountedObject
{
public:
    typedef ReferenceCountedObjectPtr<SharedObject> Ptr;

    explicit SharedObject (const Identifier& t) noexcept
        : type (t), parent (nullptr)
    {
    }

    SharedObject (const SharedObject& other)
        : ReferenceCountedObject(),
          type (other.type), properties (other.properties), parent (nullptr)
    {
        for (int i = 0; i < other.children.size(); ++i)
        {
            SharedObject* const child = new SharedObject (*other.children.getObjectPointerUnchecked(i));
            child->parent = this;
            children.add (child);
        }
    }

    ~SharedObject()
    {
        jassert (parent == nullptr); // this should never happen unless something isn't obeying the ref-counting!

        for (int i = children.size(); --i >= 0;)
        {
            const Ptr c (children.getObjectPointerUnchecked(i));
            c->parent = nullptr;
            children.remove (i);
            c->sendParentChangeMessage();
        }
    }

    template <typename Method>
    void callListeners (Method method, ValueTree& tree) const
    {
        const int numListeners = valueTreesWithListeners.size();

        if (numListeners == 1)
        {
            valueTreesWithListeners.getUnchecked(0)->listeners.call (method, tree);
        }
        else if (numListeners > 0)
        {
            const SortedSet<ValueTree*> listenersCopy (valueTreesWithListeners);

            for (int i = 0; i < numListeners; ++i)
            {
                ValueTree* const v = listenersCopy.getUnchecked(i);

                if (i == 0 || valueTreesWithListeners.contains (v))
                    v->listeners.call (method, tree);
            }
        }
    }

    template <typename Method, typename ParamType>
    void callListeners (Method method, ValueTree& tree, ParamType& param2) const
    {
        const int numListeners = valueTreesWithListeners.size();

        if (numListeners == 1)
        {
            valueTreesWithListeners.getUnchecked(0)->listeners.call (method, tree, param2);
        }
        else if (numListeners > 0)
        {
            const SortedSet<ValueTree*> listenersCopy (valueTreesWithListeners);

            for (int i = 0; i < numListeners; ++i)
            {
                ValueTree* const v = listenersCopy.getUnchecked(i);

                if (i == 0 || valueTreesWithListeners.contains (v))
                    v->listeners.call (method, tree, param2);
            }
        }
    }

    template <typename Method, typename ParamType1, typename ParamType2>
    void callListeners (Method method, ValueTree& tree, ParamType1& param2, ParamType2& param3) const
    {
        const int numListeners = valueTreesWithListeners.size();

        if (numListeners == 1)
        {
            valueTreesWithListeners.getUnchecked(0)->listeners.call (method, tree, param2, param3);
        }
        else if (numListeners > 0)
        {
            const SortedSet<ValueTree*> listenersCopy (valueTreesWithListeners);

            for (int i = 0; i < numListeners; ++i)
            {
                ValueTree* const v = listenersCopy.getUnchecked(i);

                if (i == 0 || valueTreesWithListeners.contains (v))
                    v->listeners.call (method, tree, param2, param3);
            }
        }
    }

    void sendPropertyChangeMessage (const Identifier& property)
    {
        ValueTree tree (this);

        for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
            t->callListeners (&ValueTree::Listener::valueTreePropertyChanged, tree, property);
    }

    void sendChildAddedMessage (ValueTree child)
    {
        ValueTree tree (this);

        for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
            t->callListeners (&ValueTree::Listener::valueTreeChildAdded, tree, child);
    }

    void sendChildRemovedMessage (ValueTree child, int index)
    {
        ValueTree tree (this);

        for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
            t->callListeners (&ValueTree::Listener::valueTreeChildRemoved, tree, child, index);
    }

    void sendChildOrderChangedMessage (int oldIndex, int newIndex)
    {
        ValueTree tree (this);

        for (ValueTree::SharedObject* t = this; t != nullptr; t = t->parent)
            t->callListeners (&ValueTree::Listener::valueTreeChildOrderChanged, tree, oldIndex, newIndex);
    }

    void sendParentChangeMessage()
    {
        ValueTree tree (this);

        for (int j = children.size(); --j >= 0;)
            if (SharedObject* const child = children.getObjectPointer (j))
                child->sendParentChangeMessage();

        callListeners (&ValueTree::Listener::valueTreeParentChanged, tree);
    }

    void setProperty (const Identifier& name, const var& newValue, UndoManager* const undoManager)
    {
        if (undoManager == nullptr)
        {
            if (properties.set (name, newValue))
                sendPropertyChangeMessage (name);
        }
        else
        {
            if (const var* const existingValue = properties.getVarPointer (name))
            {
                if (*existingValue != newValue)
                    undoManager->perform (new SetPropertyAction (this, name, newValue, *existingValue, false, false));
            }
            else
            {
                undoManager->perform (new SetPropertyAction (this, name, newValue, var(), true, false));
            }
        }
    }

    bool hasProperty (const Identifier& name) const noexcept
    {
        return properties.contains (name);
    }

    void removeProperty (const Identifier& name, UndoManager* const undoManager)
    {
        if (undoManager == nullptr)
        {
            if (properties.remove (name))
                sendPropertyChangeMessage (name);
        }
        else
        {
            if (properties.contains (name))
                undoManager->perform (new SetPropertyAction (this, name, var(), properties [name], false, true));
        }
    }

    void removeAllProperties (UndoManager* const undoManager)
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
                undoManager->perform (new SetPropertyAction (this, properties.getName(i), var(),
                                                             properties.getValueAt(i), false, true));
        }
    }

    void copyPropertiesFrom (const SharedObject& source, UndoManager* const undoManager)
    {
        for (int i = properties.size(); --i >= 0;)
            if (! source.properties.contains (properties.getName (i)))
                removeProperty (properties.getName (i), undoManager);

        for (int i = 0; i < source.properties.size(); ++i)
            setProperty (source.properties.getName(i), source.properties.getValueAt(i), undoManager);
    }

    ValueTree getChildWithName (const Identifier& typeToMatch) const
    {
        for (int i = 0; i < children.size(); ++i)
        {
            SharedObject* const s = children.getObjectPointerUnchecked (i);
            if (s->type == typeToMatch)
                return ValueTree (s);
        }

        return ValueTree();
    }

    ValueTree getOrCreateChildWithName (const Identifier& typeToMatch, UndoManager* undoManager)
    {
        for (int i = 0; i < children.size(); ++i)
        {
            SharedObject* const s = children.getObjectPointerUnchecked (i);
            if (s->type == typeToMatch)
                return ValueTree (s);
        }

        SharedObject* const newObject = new SharedObject (typeToMatch);
        addChild (newObject, -1, undoManager);
        return ValueTree (newObject);

    }

    ValueTree getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
    {
        for (int i = 0; i < children.size(); ++i)
        {
            SharedObject* const s = children.getObjectPointerUnchecked (i);
            if (s->properties[propertyName] == propertyValue)
                return ValueTree (s);
        }

        return ValueTree();
    }

    bool isAChildOf (const SharedObject* const possibleParent) const noexcept
    {
        for (const SharedObject* p = parent; p != nullptr; p = p->parent)
            if (p == possibleParent)
                return true;

        return false;
    }

    int indexOf (const ValueTree& child) const noexcept
    {
        return children.indexOf (child.object);
    }

    void addChild (SharedObject* child, int index, UndoManager* const undoManager)
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
                    if (! isPositiveAndBelow (index, children.size()))
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

    void removeChild (const int childIndex, UndoManager* const undoManager)
    {
        if (const Ptr child = children.getObjectPointer (childIndex))
        {
            if (undoManager == nullptr)
            {
                children.remove (childIndex);
                child->parent = nullptr;
                sendChildRemovedMessage (ValueTree (child), childIndex);
                child->sendParentChangeMessage();
            }
            else
            {
                undoManager->perform (new AddOrRemoveChildAction (this, childIndex, nullptr));
            }
        }
    }

    void removeAllChildren (UndoManager* const undoManager)
    {
        while (children.size() > 0)
            removeChild (children.size() - 1, undoManager);
    }

    void moveChild (int currentIndex, int newIndex, UndoManager* undoManager)
    {
        // The source index must be a valid index!
        jassert (isPositiveAndBelow (currentIndex, children.size()));

        if (currentIndex != newIndex
             && isPositiveAndBelow (currentIndex, children.size()))
        {
            if (undoManager == nullptr)
            {
                children.move (currentIndex, newIndex);
                sendChildOrderChangedMessage (currentIndex, newIndex);
            }
            else
            {
                if (! isPositiveAndBelow (newIndex, children.size()))
                    newIndex = children.size() - 1;

                undoManager->perform (new MoveChildAction (this, currentIndex, newIndex));
            }
        }
    }

    void reorderChildren (const OwnedArray<ValueTree>& newOrder, UndoManager* undoManager)
    {
        jassert (newOrder.size() == children.size());

        for (int i = 0; i < children.size(); ++i)
        {
            SharedObject* const child = newOrder.getUnchecked(i)->object;

            if (children.getObjectPointerUnchecked (i) != child)
            {
                const int oldIndex = children.indexOf (child);
                jassert (oldIndex >= 0);
                moveChild (oldIndex, i, undoManager);
            }
        }
    }

    bool isEquivalentTo (const SharedObject& other) const noexcept
    {
        if (type != other.type
             || properties.size() != other.properties.size()
             || children.size() != other.children.size()
             || properties != other.properties)
            return false;

        for (int i = 0; i < children.size(); ++i)
            if (! children.getObjectPointerUnchecked(i)->isEquivalentTo (*other.children.getObjectPointerUnchecked(i)))
                return false;

        return true;
    }

    XmlElement* createXml() const
    {
        XmlElement* const xml = new XmlElement (type);
        properties.copyToXmlAttributes (*xml);

        // (NB: it's faster to add nodes to XML elements in reverse order)
        for (int i = children.size(); --i >= 0;)
            xml->prependChildElement (children.getObjectPointerUnchecked(i)->createXml());

        return xml;
    }

    void writeToStream (OutputStream& output) const
    {
        output.writeString (type.toString());
        output.writeCompressedInt (properties.size());

        for (int j = 0; j < properties.size(); ++j)
        {
            output.writeString (properties.getName (j).toString());
            properties.getValueAt(j).writeToStream (output);
        }

        output.writeCompressedInt (children.size());

        for (int i = 0; i < children.size(); ++i)
            writeObjectToStream (output, children.getObjectPointerUnchecked(i));
    }

    static void writeObjectToStream (OutputStream& output, const SharedObject* const object)
    {
        if (object != nullptr)
        {
            object->writeToStream (output);
        }
        else
        {
            output.writeString (String());
            output.writeCompressedInt (0);
            output.writeCompressedInt (0);
        }
    }

    //==============================================================================
    class SetPropertyAction  : public UndoableAction
    {
    public:
        SetPropertyAction (SharedObject* const so, const Identifier& propertyName,
                           const var& newVal, const var& oldVal, bool isAdding, bool isDeleting)
            : target (so), name (propertyName), newValue (newVal), oldValue (oldVal),
              isAddingNewProperty (isAdding), isDeletingProperty (isDeleting)
        {
        }

        bool perform() override
        {
            jassert (! (isAddingNewProperty && target->hasProperty (name)));

            if (isDeletingProperty)
                target->removeProperty (name, nullptr);
            else
                target->setProperty (name, newValue, nullptr);

            return true;
        }

        bool undo() override
        {
            if (isAddingNewProperty)
                target->removeProperty (name, nullptr);
            else
                target->setProperty (name, oldValue, nullptr);

            return true;
        }

        int getSizeInUnits() override
        {
            return (int) sizeof (*this); //xxx should be more accurate
        }

        UndoableAction* createCoalescedAction (UndoableAction* nextAction) override
        {
            if (! (isAddingNewProperty || isDeletingProperty))
            {
                if (SetPropertyAction* const next = dynamic_cast<SetPropertyAction*> (nextAction))
                    if (next->target == target && next->name == name
                          && ! (next->isAddingNewProperty || next->isDeletingProperty))
                        return new SetPropertyAction (target, name, next->newValue, oldValue, false, false);
            }

            return nullptr;
        }

    private:
        const Ptr target;
        const Identifier name;
        const var newValue;
        var oldValue;
        const bool isAddingNewProperty : 1, isDeletingProperty : 1;

        JUCE_DECLARE_NON_COPYABLE (SetPropertyAction)
    };

    //==============================================================================
    class AddOrRemoveChildAction  : public UndoableAction
    {
    public:
        AddOrRemoveChildAction (SharedObject* parentObject, int index, SharedObject* newChild)
            : target (parentObject),
              child (newChild != nullptr ? newChild : parentObject->children.getObjectPointer (index)),
              childIndex (index),
              isDeleting (newChild == nullptr)
        {
            jassert (child != nullptr);
        }

        bool perform() override
        {
            if (isDeleting)
                target->removeChild (childIndex, nullptr);
            else
                target->addChild (child, childIndex, nullptr);

            return true;
        }

        bool undo() override
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

        int getSizeInUnits() override
        {
            return (int) sizeof (*this); //xxx should be more accurate
        }

    private:
        const Ptr target, child;
        const int childIndex;
        const bool isDeleting;

        JUCE_DECLARE_NON_COPYABLE (AddOrRemoveChildAction)
    };

    //==============================================================================
    class MoveChildAction  : public UndoableAction
    {
    public:
        MoveChildAction (SharedObject* parentObject, int fromIndex, int toIndex) noexcept
            : parent (parentObject), startIndex (fromIndex), endIndex (toIndex)
        {
        }

        bool perform() override
        {
            parent->moveChild (startIndex, endIndex, nullptr);
            return true;
        }

        bool undo() override
        {
            parent->moveChild (endIndex, startIndex, nullptr);
            return true;
        }

        int getSizeInUnits() override
        {
            return (int) sizeof (*this); //xxx should be more accurate
        }

        UndoableAction* createCoalescedAction (UndoableAction* nextAction) override
        {
            if (MoveChildAction* next = dynamic_cast<MoveChildAction*> (nextAction))
                if (next->parent == parent && next->startIndex == endIndex)
                    return new MoveChildAction (parent, startIndex, next->endIndex);

            return nullptr;
        }

    private:
        const Ptr parent;
        const int startIndex, endIndex;

        JUCE_DECLARE_NON_COPYABLE (MoveChildAction)
    };

    //==============================================================================
    const Identifier type;
    NamedValueSet properties;
    ReferenceCountedArray<SharedObject> children;
    SortedSet<ValueTree*> valueTreesWithListeners;
    SharedObject* parent;

private:
    SharedObject& operator= (const SharedObject&);
    JUCE_LEAK_DETECTOR (SharedObject)
};

//==============================================================================
ValueTree::ValueTree() noexcept
{
}

const ValueTree ValueTree::invalid;

ValueTree::ValueTree (const Identifier& type)  : object (new ValueTree::SharedObject (type))
{
    jassert (type.toString().isNotEmpty()); // All objects must be given a sensible type name!
}

ValueTree::ValueTree (SharedObject* so) noexcept  : object (so)
{
}

ValueTree::ValueTree (const ValueTree& other) noexcept  : object (other.object)
{
}

ValueTree& ValueTree::operator= (const ValueTree& other)
{
    if (object != other.object)
    {
        if (listeners.isEmpty())
        {
            object = other.object;
        }
        else
        {
            if (object != nullptr)
                object->valueTreesWithListeners.removeValue (this);

            if (other.object != nullptr)
                other.object->valueTreesWithListeners.add (this);

            object = other.object;

            listeners.call (&ValueTree::Listener::valueTreeRedirected, *this);
        }
    }

    return *this;
}

#if JUCE_COMPILER_SUPPORTS_MOVE_SEMANTICS
ValueTree::ValueTree (ValueTree&& other) noexcept
    : object (static_cast<SharedObject::Ptr&&> (other.object))
{
}
#endif

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
            || (object != nullptr && other.object != nullptr
                 && object->isEquivalentTo (*other.object));
}

ValueTree ValueTree::createCopy() const
{
    return ValueTree (createCopyIfNotNull (object.get()));
}

bool ValueTree::hasType (const Identifier& typeName) const noexcept
{
    return object != nullptr && object->type == typeName;
}

Identifier ValueTree::getType() const noexcept
{
    return object != nullptr ? object->type : Identifier();
}

ValueTree ValueTree::getParent() const noexcept
{
    return ValueTree (object != nullptr ? object->parent
                                        : static_cast<SharedObject*> (nullptr));
}

ValueTree ValueTree::getSibling (const int delta) const noexcept
{
    if (object == nullptr || object->parent == nullptr)
        return invalid;

    const int index = object->parent->indexOf (*this) + delta;
    return ValueTree (object->parent->children.getObjectPointer (index));
}

const var& ValueTree::operator[] (const Identifier& name) const noexcept
{
    return object == nullptr ? var::null : object->properties[name];
}

const var& ValueTree::getProperty (const Identifier& name) const noexcept
{
    return object == nullptr ? var::null : object->properties[name];
}

var ValueTree::getProperty (const Identifier& name, const var& defaultReturnValue) const
{
    return object == nullptr ? defaultReturnValue
                             : object->properties.getWithDefault (name, defaultReturnValue);
}

ValueTree& ValueTree::setProperty (const Identifier& name, const var& newValue, UndoManager* undoManager)
{
    jassert (name.toString().isNotEmpty()); // Must have a valid property name!
    jassert (object != nullptr); // Trying to add a property to a null ValueTree will fail!

    if (object != nullptr)
        object->setProperty (name, newValue, undoManager);

    return *this;
}

bool ValueTree::hasProperty (const Identifier& name) const noexcept
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

int ValueTree::getNumProperties() const noexcept
{
    return object == nullptr ? 0 : object->properties.size();
}

Identifier ValueTree::getPropertyName (const int index) const noexcept
{
    return object == nullptr ? Identifier()
                             : object->properties.getName (index);
}

void ValueTree::copyPropertiesFrom (const ValueTree& source, UndoManager* const undoManager)
{
    jassert (object != nullptr || source.object == nullptr); // Trying to add properties to a null ValueTree will fail!

    if (source.object == nullptr)
        removeAllProperties (undoManager);
    else if (object != nullptr)
        object->copyPropertiesFrom (*(source.object), undoManager);
}

int ValueTree::getReferenceCount() const noexcept
{
    return object != nullptr ? object->getReferenceCount() : 0;
}

//==============================================================================
class ValueTreePropertyValueSource  : public Value::ValueSource,
                                      private ValueTree::Listener
{
public:
    ValueTreePropertyValueSource (const ValueTree& vt, const Identifier& prop, UndoManager* um)
        : tree (vt), property (prop), undoManager (um)
    {
        tree.addListener (this);
    }

    ~ValueTreePropertyValueSource()
    {
        tree.removeListener (this);
    }

    var getValue() const override                 { return tree [property]; }
    void setValue (const var& newValue) override  { tree.setProperty (property, newValue, undoManager); }

private:
    ValueTree tree;
    const Identifier property;
    UndoManager* const undoManager;

    void valueTreePropertyChanged (ValueTree& changedTree, const Identifier& changedProperty) override
    {
        if (tree == changedTree && property == changedProperty)
            sendChangeMessage (false);
    }

    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ValueTreePropertyValueSource)
};

Value ValueTree::getPropertyAsValue (const Identifier& name, UndoManager* const undoManager)
{
    return Value (new ValueTreePropertyValueSource (*this, name, undoManager));
}

//==============================================================================
int ValueTree::getNumChildren() const noexcept
{
    return object == nullptr ? 0 : object->children.size();
}

ValueTree ValueTree::getChild (int index) const
{
    return ValueTree (object != nullptr ? object->children.getObjectPointer (index)
                                        : static_cast<SharedObject*> (nullptr));
}

ValueTree ValueTree::getChildWithName (const Identifier& type) const
{
    return object != nullptr ? object->getChildWithName (type) : ValueTree();
}

ValueTree ValueTree::getOrCreateChildWithName (const Identifier& type, UndoManager* undoManager)
{
    return object != nullptr ? object->getOrCreateChildWithName (type, undoManager) : ValueTree();
}

ValueTree ValueTree::getChildWithProperty (const Identifier& propertyName, const var& propertyValue) const
{
    return object != nullptr ? object->getChildWithProperty (propertyName, propertyValue) : ValueTree();
}

bool ValueTree::isAChildOf (const ValueTree& possibleParent) const noexcept
{
    return object != nullptr && object->isAChildOf (possibleParent.object);
}

int ValueTree::indexOf (const ValueTree& child) const noexcept
{
    return object != nullptr ? object->indexOf (child) : -1;
}

void ValueTree::addChild (const ValueTree& child, int index, UndoManager* const undoManager)
{
    jassert (object != nullptr); // Trying to add a child to a null ValueTree!

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
void ValueTree::createListOfChildren (OwnedArray<ValueTree>& list) const
{
    jassert (object != nullptr);

    for (int i = 0; i < object->children.size(); ++i)
        list.add (new ValueTree (object->children.getObjectPointerUnchecked(i)));
}

void ValueTree::reorderChildren (const OwnedArray<ValueTree>& newOrder, UndoManager* undoManager)
{
    jassert (object != nullptr);
    object->reorderChildren (newOrder, undoManager);
}

//==============================================================================
void ValueTree::addListener (Listener* listener)
{
    if (listener != nullptr)
    {
        if (listeners.isEmpty() && object != nullptr)
            object->valueTreesWithListeners.add (this);

        listeners.add (listener);
    }
}

void ValueTree::removeListener (Listener* listener)
{
    listeners.remove (listener);

    if (listeners.isEmpty() && object != nullptr)
        object->valueTreesWithListeners.removeValue (this);
}

void ValueTree::sendPropertyChangeMessage (const Identifier& property)
{
    if (object != nullptr)
        object->sendPropertyChangeMessage (property);
}

//==============================================================================
XmlElement* ValueTree::createXml() const
{
    return object != nullptr ? object->createXml() : nullptr;
}

ValueTree ValueTree::fromXml (const XmlElement& xml)
{
    // ValueTrees don't have any equivalent to XML text elements!
    jassert (! xml.isTextElement());

    ValueTree v (xml.getTagName());
    v.object->properties.setFromXmlAttributes (xml);

    forEachXmlChildElement (xml, e)
        v.addChild (fromXml (*e), -1, nullptr);

    return v;
}

String ValueTree::toXmlString() const
{
    const ScopedPointer<XmlElement> xml (createXml());
    return xml != nullptr ? xml->createDocument ("") : String();
}

//==============================================================================
void ValueTree::writeToStream (OutputStream& output) const
{
    SharedObject::writeObjectToStream (output, object);
}

ValueTree ValueTree::readFromStream (InputStream& input)
{
    const String type (input.readString());

    if (type.isEmpty())
        return ValueTree();

    ValueTree v (type);

    const int numProps = input.readCompressedInt();

    if (numProps < 0)
    {
        jassertfalse;  // trying to read corrupted data!
        return v;
    }

    for (int i = 0; i < numProps; ++i)
    {
        const String name (input.readString());

        if (name.isNotEmpty())
        {
            const var value (var::readFromStream (input));
            v.object->properties.set (name, value);
        }
        else
        {
            jassertfalse;  // trying to read corrupted data!
        }
    }

    const int numChildren = input.readCompressedInt();
    v.object->children.ensureStorageAllocated (numChildren);

    for (int i = 0; i < numChildren; ++i)
    {
        ValueTree child (readFromStream (input));

        if (! child.isValid())
            return v;

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

ValueTree ValueTree::readFromGZIPData (const void* const data, const size_t numBytes)
{
    MemoryInputStream in (data, numBytes, false);
    GZIPDecompressorInputStream gzipStream (in);
    return readFromStream (gzipStream);
}

void ValueTree::Listener::valueTreeRedirected (ValueTree&) {}

//==============================================================================
#if JUCE_UNIT_TESTS

class ValueTreeTests  : public UnitTest
{
public:
    ValueTreeTests() : UnitTest ("ValueTrees") {}

    static String createRandomIdentifier (Random& r)
    {
        char buffer[50] = { 0 };
        const char chars[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-:";

        for (int i = 1 + r.nextInt (numElementsInArray (buffer) - 2); --i >= 0;)
            buffer[i] = chars [r.nextInt (sizeof (chars) - 1)];

        return CharPointer_ASCII (buffer);
    }

    static String createRandomWideCharString (Random& r)
    {
        juce_wchar buffer[50] = { 0 };

        for (int i = r.nextInt (numElementsInArray (buffer) - 1); --i >= 0;)
        {
            if (r.nextBool())
            {
                do
                {
                    buffer[i] = (juce_wchar) (1 + r.nextInt (0x10ffff - 1));
                }
                while (! CharPointer_UTF16::canRepresent (buffer[i]));
            }
            else
                buffer[i] = (juce_wchar) (1 + r.nextInt (0x7e));
        }

        return CharPointer_UTF32 (buffer);
    }

    static ValueTree createRandomTree (UndoManager* undoManager, int depth, Random& r)
    {
        ValueTree v (createRandomIdentifier (r));

        for (int i = r.nextInt (10); --i >= 0;)
        {
            switch (r.nextInt (5))
            {
                case 0: v.setProperty (createRandomIdentifier (r), createRandomWideCharString (r), undoManager); break;
                case 1: v.setProperty (createRandomIdentifier (r), r.nextInt(), undoManager); break;
                case 2: if (depth < 5) v.addChild (createRandomTree (undoManager, depth + 1, r), r.nextInt (v.getNumChildren() + 1), undoManager); break;
                case 3: v.setProperty (createRandomIdentifier (r), r.nextBool(), undoManager); break;
                case 4: v.setProperty (createRandomIdentifier (r), r.nextDouble(), undoManager); break;
                default: break;
            }
        }

        return v;
    }

    void runTest() override
    {
        beginTest ("ValueTree");
        Random r = getRandom();

        for (int i = 10; --i >= 0;)
        {
            MemoryOutputStream mo;
            ValueTree v1 (createRandomTree (nullptr, 0, r));
            v1.writeToStream (mo);

            MemoryInputStream mi (mo.getData(), mo.getDataSize(), false);
            ValueTree v2 = ValueTree::readFromStream (mi);
            expect (v1.isEquivalentTo (v2));

            ScopedPointer<XmlElement> xml1 (v1.createXml());
            ScopedPointer<XmlElement> xml2 (v2.createCopy().createXml());
            expect (xml1->isEquivalentTo (xml2, false));

            ValueTree v4 = v2.createCopy();
            expect (v1.isEquivalentTo (v4));
        }
    }
};

static ValueTreeTests valueTreeTests;

#endif
