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

#ifndef __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
#define __JUCER_COMPONENTDOCUMENT_JUCEHEADER__

#include "../jucer_Headers.h"
#include "jucer_Project.h"


//==============================================================================
class ComponentDocument   : public ValueTree::Listener
{
public:
    //==============================================================================
    ComponentDocument (Project* project, const File& cppFile);
    ~ComponentDocument();

    static bool isComponentFile (const File& file);

    bool save();
    bool reload();
    bool hasChangedSinceLastSave();

    typedef SelectedItemSet<uint32> SelectedItems;

    //==============================================================================
    Value getClassName()            { return getRootValue ("className"); }
    Value getClassDescription()     { return getRootValue ("classDesc"); }
    const String getNonExistentMemberName (String suggestedName);

    //==============================================================================
    int getNumComponents() const;
    const ValueTree getComponent (int index) const;
    const ValueTree getComponentWithMemberName (const String& name) const;
    Component* createComponent (int index) const;
    void updateComponent (Component* comp) const;
    bool containsComponent (Component* comp) const;
    const ValueTree getComponentState (Component* comp) const;
    void getComponentProperties (Array <PropertyComponent*>& props, Component* comp);
    bool isStateForComponent (const ValueTree& storedState, Component* comp) const;

    void addNewComponentMenuItems (PopupMenu& menu) const;
    void performNewComponentMenuItem (int menuResultCode);

    //==============================================================================
    void beginDrag (const Array<Component*>& items, const MouseEvent& e,
                    const ResizableBorderComponent::Zone& zone);
    void continueDrag (const MouseEvent& e);
    void endDrag (const MouseEvent& e);

    //==============================================================================
    ValueTree& getRoot()                                { return root; }
    UndoManager* getUndoManager();
    void beginNewTransaction();

    void valueTreePropertyChanged (ValueTree& treeWhosePropertyHasChanged, const var::identifier& property);
    void valueTreeChildrenChanged (ValueTree& treeWhoseChildHasChanged);
    void valueTreeParentChanged (ValueTree& treeWhoseParentHasChanged);

private:
    Project* project;
    File cppFile;
    ValueTree root;
    UndoManager undoManager;
    bool changedSinceSaved;

    class DragHandler;
    ScopedPointer <DragHandler> dragger;

    void checkRootObject();
    ValueTree getComponentGroup() const;
    Value getRootValue (const var::identifier& name)    { return root.getPropertyAsValue (name, getUndoManager()); }

    void writeCode (OutputStream& cpp, OutputStream& header);
    void writeMetadata (OutputStream& out);
};


//==============================================================================
class ComponentTypeHandler
{
public:
    //==============================================================================
    ComponentTypeHandler (const String& name_, const String& xmlTag_, const String& memberNameRoot_);
    virtual ~ComponentTypeHandler();

    const String& getName() const               { return name; }
    const String& getXmlTag() const             { return xmlTag; }
    const String& getMemberNameRoot() const     { return memberNameRoot; }

    virtual Component* createComponent() = 0;
    virtual const Rectangle<int> getDefaultSize() = 0;

    virtual void updateComponent (Component* comp, const ValueTree& state);
    virtual void initialiseNewItem (ValueTree& state, ComponentDocument& document);
    virtual void createPropertyEditors (ValueTree& state, ComponentDocument& document, Array <PropertyComponent*>& props);

    Value getValue (const var::identifier& name, ValueTree& state, ComponentDocument& document) const;

    //==============================================================================
protected:
    const String name, xmlTag, memberNameRoot;
};


#endif   // __JUCER_COMPONENTDOCUMENT_JUCEHEADER__
