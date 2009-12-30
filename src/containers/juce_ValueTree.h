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

#ifndef __JUCE_VALUETREE_JUCEHEADER__
#define __JUCE_VALUETREE_JUCEHEADER__

#include "juce_Variant.h"
#include "../utilities/juce_UndoManager.h"
#include "../text/juce_XmlElement.h"
#include "juce_ReferenceCountedArray.h"


//==============================================================================
/**
    A powerful tree structure that can be used to hold free-form data, and which can
    handle its own undo and redo behaviour.

    A ValueTree contains a list of named properties as var objects, and also holds
    any number of sub-trees.

    Create ValueTree objects on the stack, and don't be afraid to copy them around, as
    they're simply a lightweight reference to a shared data container. Creating a copy
    of another ValueTree simply creates a new reference to the same underlying object - to
    make a separate, deep copy of a tree you should explicitly call createCopy().

    Each ValueTree has a type name, in much the same way as an XmlElement has a tag name,
    and much of the structure of a ValueTree is similar to an XmlElement tree.
    You can convert a ValueTree to and from an XmlElement, and as long as the XML doesn't
    contain text elements, the conversion works well and makes a good serialisation
    format. They can also be serialised to a binary format, which is very fast and compact.

    All the methods that change data take an optional UndoManager, which will be used
    to track any changes to the object. For this to work, you have to be careful to
    consistently always use the same UndoManager for all operations to any node inside
    the tree.

    A ValueTree can only be a child of one parent at a time, so if you're moving one from
    one tree to another, be careful to always remove it first, before adding it. This
    could also mess up your undo/redo chain, so be wary! In a debug build you should hit
    assertions if you try to do anything dangerous, but there are still plenty of ways it
    could go wrong.

    Listeners can be added to a ValueTree to be told when properies change and when
    nodes are added or removed.

    @see var, XmlElement
*/
class JUCE_API  ValueTree
{
public:
    //==============================================================================
    /** Creates an empty ValueTree with the given type name.
        Like an XmlElement, each ValueTree node has a type, which you can access with
        getType() and hasType().
    */
    ValueTree (const String& type) throw();

    /** Creates a reference to another ValueTree. */
    ValueTree (const ValueTree& other) throw();

    /** Makes this object reference another node. */
    const ValueTree& operator= (const ValueTree& other) throw();

    /** Destructor. */
    ~ValueTree() throw();

    /** Returns true if both this and the other tree node refer to the same underlying structure.
        Note that this isn't a value comparison - two independently-created trees which
        contain identical data are not considered equal.
    */
    bool operator== (const ValueTree& other) const throw();

    /** Returns true if this and the other node refer to different underlying structures.
        Note that this isn't a value comparison - two independently-created trees which
        contain identical data are not considered equal.
    */
    bool operator!= (const ValueTree& other) const throw();

    //==============================================================================
    /** Returns true if this node refers to some valid data.
        It's hard to create an invalid node, but you might get one returned, e.g. by an out-of-range
        call to getChild().
    */
    bool isValid() const throw()                    { return object != 0; }

    /** Returns a deep copy of this tree and all its sub-nodes. */
    ValueTree createCopy() const throw();

    //==============================================================================
    /** Returns the type of this node.
        The type is specified when the ValueTree is created.
        @see hasType
    */
    const String getType() const throw();

    /** Returns true if the node has this type.
        The comparison is case-sensitive.
    */
    bool hasType (const String& typeName) const throw();

    //==============================================================================
    /** Returns the value of a named property.
        If no such property has been set, this will return a void variant.
        You can also use operator[] to get a property.
        @see var, setProperty, hasProperty
    */
    const var getProperty (const var::identifier& name) const throw();

    /** Returns the value of a named property.
        If no such property has been set, this will return a void variant. This is the same as
        calling getProperty().
        @see getProperty
    */
    const var operator[] (const var::identifier& name) const throw();

    /** Changes a named property of the node.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
        @see var, getProperty, removeProperty
    */
    void setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager) throw();

    /** Returns true if the node contains a named property. */
    bool hasProperty (const var::identifier& name) const throw();

    /** Removes a property from the node.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void removeProperty (const var::identifier& name, UndoManager* const undoManager) throw();

    /** Removes all properties from the node.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void removeAllProperties (UndoManager* const undoManager) throw();

    /** Returns the total number of properties that the node contains.
        @see getProperty.
    */
    int getNumProperties() const throw();

    /** Returns the identifier of the property with a given index.
        @see getNumProperties
    */
    const var::identifier getPropertyName (int index) const throw();

    //==============================================================================
    /** Returns the number of child nodes belonging to this one.
        @see getChild
    */
    int getNumChildren() const throw();

    /** Returns one of this node's child nodes.
        If the index is out of range, it'll return an invalid node. (See isValid() to find out
        whether a node is valid).
    */
    ValueTree getChild (int index) const throw();

    /** Looks for a child node with the speficied type name.
        If no such node is found, it'll return an invalid node. (See isValid() to find out
        whether a node is valid).
    */
    ValueTree getChildWithName (const String& type) const throw();

    /** Looks for the first child node that has the speficied property value.

        This will scan the child nodes in order, until it finds one that has property that matches
        the specified value.

        If no such node is found, it'll return an invalid node. (See isValid() to find out
        whether a node is valid).
    */
    ValueTree getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const throw();

    /** Adds a child to this node.

        Make sure that the child is removed from any former parent node before calling this, or
        you'll hit an assertion.

        If the index is < 0 or greater than the current number of child nodes, the new node will
        be added at the end of the list.

        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void addChild (ValueTree child, int index, UndoManager* const undoManager) throw();

    /** Removes the specified child from this node's child-list.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void removeChild (ValueTree& child, UndoManager* const undoManager) throw();

    /** Removes a child from this node's child-list.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void removeChild (const int childIndex, UndoManager* const undoManager) throw();

    /** Removes all child-nodes from this node.
        If the undoManager parameter is non-null, its UndoManager::perform() method will be used,
        so that this change can be undone.
    */
    void removeAllChildren (UndoManager* const undoManager) throw();

    /** Returns true if this node is anywhere below the specified parent node.
        This returns true if the node is a child-of-a-child, as well as a direct child.
    */
    bool isAChildOf (const ValueTree& possibleParent) const throw();

    /** Returns the parent node that contains this one.
        If the node has no parent, this will return an invalid node. (See isValid() to find out
        whether a node is valid).
    */
    ValueTree getParent() const throw();

    //==============================================================================
    /** Creates an XmlElement that holds a complete image of this node and all its children.

        If this node is invalid, this may return 0. Otherwise, the XML that is produced can
        be used to recreate a similar node by calling fromXml()
        @see fromXml
    */
    XmlElement* createXml() const throw();

    /** Tries to recreate a node from its XML representation.

        This isn't designed to cope with random XML data - for a sensible result, it should only
        be fed XML that was created by the createXml() method.
    */
    static ValueTree fromXml (const XmlElement& xml) throw();

    //==============================================================================
    /** Stores this tree (and all its children) in a binary format.

        Once written, the data can be read back with readFromStream().

        It's much faster to load/save your tree in binary form than as XML, but
        obviously isn't human-readable.
    */
    void writeToStream (OutputStream& output) throw();

    /** Reloads a tree from a stream that was written with writeToStream().
    */
    static ValueTree readFromStream (InputStream& input) throw();

    //==============================================================================
    /** Listener class for events that happen to a ValueTree.

        To get events from a ValueTree, make your class implement this interface, and use
        ValueTree::addListener() and ValueTree::removeListener() to register it.
    */
    class JUCE_API  Listener
    {
    public:
        /** Destructor. */
        virtual ~Listener() {}

        /** This method is called when one or more of the properties of this node have changed. */
        virtual void valueTreePropertyChanged (ValueTree& tree) = 0;

        /** This method is called when one or more of the children of this node have been added or removed. */
        virtual void valueTreeChildrenChanged (ValueTree& tree) = 0;

        /** This method is called when this node has been added or removed from a parent node. */
        virtual void valueTreeParentChanged() = 0;
    };

    /** Adds a listener to receive callbacks when this node is changed. */
    void addListener (Listener* listener) throw();

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* listener) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    friend class ValueTreeSetPropertyAction;
    friend class ValueTreeChildChangeAction;

    class SharedObject    : public ReferenceCountedObject
    {
    public:
        SharedObject (const String& type) throw();
        SharedObject (const SharedObject& other) throw();
        ~SharedObject() throw();

        struct Property
        {
            Property (const var::identifier& name, const var& value) throw();

            var::identifier name;
            var value;
        };

        const String type;
        OwnedArray <Property> properties;
        ReferenceCountedArray <SharedObject> children;
        SortedSet <Listener*> listeners;
        SharedObject* parent;

        void sendPropertyChangeMessage();
        void sendChildChangeMessage();
        void sendParentChangeMessage();
        const var getProperty (const var::identifier& name) const throw();
        void setProperty (const var::identifier& name, const var& newValue, UndoManager* const undoManager) throw();
        bool hasProperty (const var::identifier& name) const throw();
        void removeProperty (const var::identifier& name, UndoManager* const undoManager) throw();
        void removeAllProperties (UndoManager* const undoManager) throw();
        bool isAChildOf (const SharedObject* const possibleParent) const throw();
        ValueTree getChildWithName (const String& type) const throw();
        ValueTree getChildWithProperty (const var::identifier& propertyName, const var& propertyValue) const throw();
        void addChild (SharedObject* child, int index, UndoManager* const undoManager) throw();
        void removeChild (const int childIndex, UndoManager* const undoManager) throw();
        void removeAllChildren (UndoManager* const undoManager) throw();
        XmlElement* createXml() const throw();
    };

    friend class SharedObject;

    typedef ReferenceCountedObjectPtr <SharedObject> SharedObjectPtr;

    ReferenceCountedObjectPtr <SharedObject> object;

    ValueTree (SharedObject* const object_) throw();
};


#endif   // __JUCE_VALUETREE_JUCEHEADER__
