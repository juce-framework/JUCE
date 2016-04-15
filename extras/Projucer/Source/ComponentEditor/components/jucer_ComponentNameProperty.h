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

#ifndef JUCER_COMPONENTNAMEPROPERTY_H_INCLUDED
#define JUCER_COMPONENTNAMEPROPERTY_H_INCLUDED

#include "jucer_ComponentTypeHandler.h"
#include "jucer_ComponentUndoableAction.h"
#include "../properties/jucer_ComponentTextProperty.h"


//==============================================================================
class ComponentNameProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentNameProperty (Component* comp, JucerDocument& doc)
        : ComponentTextProperty <Component> ("name", 40, false, comp, doc)
    {
    }


    void setText (const String& newText) override
    {
        document.perform (new CompNameChangeAction (component, *document.getComponentLayout(), newText),
                          "Change component name");
    }

    String getText() const override
    {
        return component->getName();
    }

private:
    class CompNameChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompNameChangeAction (Component* const comp, ComponentLayout& l, const String& nm)
            : ComponentUndoableAction <Component> (comp, l),
              newName (nm), oldName (comp->getName())
        {
        }

        bool perform()
        {
            showCorrectTab();
            getComponent()->setName (newName);
            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getComponent()->setName (oldName);
            changed();
            return true;
        }

        String newName, oldName;
    };
};

//==============================================================================
class ComponentMemberNameProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentMemberNameProperty (Component* comp, JucerDocument& doc)
        : ComponentTextProperty <Component> ("member name", 40, false, comp, doc)
    {
    }

    void setText (const String& newText) override
    {
        document.perform (new CompMemberNameChangeAction (component, *document.getComponentLayout(), newText),
                          "Change component member name");
    }

    String getText() const override
    {
        return document.getComponentLayout()->getComponentMemberVariableName (component);
    }

private:
    class CompMemberNameChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompMemberNameChangeAction (Component* const comp, ComponentLayout& l, const String& nm)
            : ComponentUndoableAction <Component> (comp, l),
              newName (nm), oldName (layout.getComponentMemberVariableName (comp))
        {
        }

        bool perform()
        {
            showCorrectTab();
            layout.setComponentMemberVariableName (getComponent(), newName);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            layout.setComponentMemberVariableName (getComponent(), oldName);
            return true;
        }

        String newName, oldName;
    };
};


//==============================================================================
class ComponentVirtualClassProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentVirtualClassProperty (Component* comp, JucerDocument& doc)
        : ComponentTextProperty <Component> ("virtual class", 40, false, comp, doc)
    {
    }

    void setText (const String& newText) override
    {
        document.perform (new CompVirtualClassChangeAction (component, *document.getComponentLayout(), newText),
                          "Change component virtual class name");
    }

    String getText() const override
    {
        return document.getComponentLayout()->getComponentVirtualClassName (component);
    }

private:
    class CompVirtualClassChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompVirtualClassChangeAction (Component* const comp, ComponentLayout& l, const String& nm)
            : ComponentUndoableAction <Component> (comp, l),
              newName (nm), oldName (layout.getComponentVirtualClassName (comp))
        {
        }

        bool perform()
        {
            showCorrectTab();
            layout.setComponentVirtualClassName (getComponent(), newName);
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            layout.setComponentVirtualClassName (getComponent(), oldName);
            return true;
        }

        String newName, oldName;
    };
};



#endif   // JUCER_COMPONENTNAMEPROPERTY_H_INCLUDED
