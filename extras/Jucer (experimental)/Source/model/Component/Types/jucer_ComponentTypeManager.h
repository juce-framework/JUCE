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

#ifndef __JUCER_COMPONENTTYPEMANAGER_H_734EBF1__
#define __JUCER_COMPONENTTYPEMANAGER_H_734EBF1__

#include "../../../jucer_Headers.h"
#include "../jucer_ComponentDocument.h"
#include "../../../utility/jucer_ColourEditorComponent.h"

class ComponentTypeHandler;


//==============================================================================
/** Temporary wrapper around a document and a component's ValueTree, providing lots of useful
    functions that can be performed on the component.
*/
class ComponentTypeInstance
{
public:
    ComponentTypeInstance (ComponentDocument& document_, const ValueTree& state_);

    //==============================================================================
    ComponentDocument& getDocument() throw()            { return document; }
    ValueTree& getState() throw()                       { return state; }

    Value getValue (const Identifier& name) const;
    void set (const Identifier& name, const var& value);
    const var operator[] (const Identifier& name) const    { return state [name] ;}

    const String getMemberName() const                  { return state [ComponentDocument::memberNameProperty]; }
    const String getComponentName() const               { return state [ComponentDocument::compNameProperty]; }

    //==============================================================================
    void initialiseNewItemBasics();
    void updateComponentBasics (Component* comp);

    void addMemberNameProperty (Array <PropertyComponent*>& props);
    void addBoundsProperties (Array <PropertyComponent*>& props);
    void addTooltipProperty (Array <PropertyComponent*>& props);
    void addFocusOrderProperty (Array <PropertyComponent*>& props);
    void addColourProperty (Array <PropertyComponent*>& props, int colourId, const String& name, const Identifier& propertyName);
    void addFontProperties (Array <PropertyComponent*>& props, const Identifier& name);
    void addJustificationProperty (Array <PropertyComponent*>& props, const String& name, const Value& value, bool onlyHorizontal);

    //==============================================================================
    const String createConstructorStatement (const String& params);

    //==============================================================================
    ComponentTypeHandler* getHandler() const;
    void updateComponent (Component* comp);
    void createProperties (Array <PropertyComponent*>& props);
    void createCode (CodeGenerator& code);

private:
    //==============================================================================
    ComponentDocument& document;
    ValueTree state;
};


//==============================================================================
class ComponentTypeHandler
{
public:
    //==============================================================================
    ComponentTypeHandler (const String& displayName_, const String& className_, const String& xmlTag_, const String& memberNameRoot_);
    virtual ~ComponentTypeHandler();

    const String& getDisplayName() const        { return displayName; }
    const Identifier& getXmlTag() const    { return xmlTag; }
    const String& getMemberNameRoot() const     { return memberNameRoot; }

    virtual Component* createComponent() = 0;
    virtual const Rectangle<int> getDefaultSize() = 0;

    virtual void initialiseNewItem (ComponentTypeInstance& item) = 0;
    virtual void updateComponent (ComponentTypeInstance& item, Component* comp) = 0;
    virtual void createPropertyEditors (ComponentTypeInstance& item, Array <PropertyComponent*>& props) = 0;
    virtual void itemDoubleClicked (const MouseEvent& e, ComponentTypeInstance& item) = 0;
    virtual void createCode (ComponentTypeInstance& item, CodeGenerator& code) = 0;
    virtual const String getClassName (ComponentTypeInstance& item) const     { return className; }

protected:
    //==============================================================================
    const String displayName, className, memberNameRoot;
    const Identifier xmlTag;

private:
    ComponentTypeHandler (const ComponentTypeHandler&);
    ComponentTypeHandler& operator= (const ComponentTypeHandler&);
};


//==============================================================================
class ComponentTypeManager  : public DeletedAtShutdown
{
public:
    //==============================================================================
    ComponentTypeManager();
    ~ComponentTypeManager();

    juce_DeclareSingleton_SingleThreaded_Minimal (ComponentTypeManager);

    //==============================================================================
    Component* createFromStoredType (ComponentDocument& document, const ValueTree& value);

    int getNumHandlers() const                                      { return handlers.size(); }
    ComponentTypeHandler* getHandler (const int index) const        { return handlers[index]; }

    ComponentTypeHandler* getHandlerFor (const Identifier& type);
    const StringArray getDisplayNames() const;

private:
    //==============================================================================
    OwnedArray <ComponentTypeHandler> handlers;
};


//==============================================================================
template <class ComponentClass>
class ComponentTypeHelper   : public ComponentTypeHandler
{
public:
    //==============================================================================
    ComponentTypeHelper (const String& displayName_, const String& className_, const String& xmlTag_, const String& memberNameRoot_)
        : ComponentTypeHandler (displayName_, className_, xmlTag_, memberNameRoot_)
    {
    }

    virtual void initialiseNew (ComponentTypeInstance& item) = 0;
    virtual void update (ComponentTypeInstance& item, ComponentClass* comp) = 0;
    virtual void createProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props) = 0;

    void initialiseNewItem (ComponentTypeInstance& item)
    {
        item.initialiseNewItemBasics();
        initialiseNew (item);
    }

    void updateComponent (ComponentTypeInstance& item, Component* comp)
    {
        item.updateComponentBasics (comp);

        ComponentClass* const c = dynamic_cast <ComponentClass*> (comp);
        jassert (c != 0);
        updateComponentColours (item, c);

        update (item, c);
    }

    void createPropertyEditors (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        item.addMemberNameProperty (props);
        item.addBoundsProperties (props);

        createProperties (item, props);
    }

    void itemDoubleClicked (const MouseEvent&, ComponentTypeInstance&)
    {
    }

protected:
    //==============================================================================
    struct EditableColour
    {
        int colourId;
        String name;
        Identifier propertyName;
    };

    Array <EditableColour> editableColours;

    void addEditableColour (int colourId, const String& displayName, const Identifier& propertyName)
    {
        EditableColour ec;
        ec.colourId = colourId;
        ec.name = displayName;
        ec.propertyName = propertyName;
        editableColours.add (ec);
    }

    void addEditableColourProperties (ComponentTypeInstance& item, Array <PropertyComponent*>& props)
    {
        for (int i = 0; i < editableColours.size(); ++i)
        {
            const EditableColour& ec = editableColours.getReference(i);
            item.addColourProperty (props, ec.colourId, ec.name, ec.propertyName);
        }
    }

    void updateComponentColours (ComponentTypeInstance& item, Component* component)
    {
        for (int i = 0; i < editableColours.size(); ++i)
        {
            const EditableColour& ec = editableColours.getReference(i);

            const String colour (item.getState() [ec.propertyName].toString());

            if (colour.isNotEmpty())
                component->setColour (ec.colourId, Colour::fromString (colour));
            else
                component->removeColour (ec.colourId);
        }
    }
};


#endif  // __JUCER_COMPONENTTYPEMANAGER_H_734EBF1__
