/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCER_COMPONENTNAMEPROPERTY_JUCEHEADER__
#define __JUCER_COMPONENTNAMEPROPERTY_JUCEHEADER__

#include "jucer_ComponentTypeHandler.h"
#include "jucer_ComponentUndoableAction.h"
#include "../../properties/jucer_ComponentTextProperty.h"


//==============================================================================
/**
*/
class ComponentNameProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentNameProperty (Component* component_, JucerDocument& document_)
        : ComponentTextProperty <Component> (T("name"), 40, false, component_, document_)
    {
    }

    ~ComponentNameProperty()
    {
    }

    //==============================================================================
    void setText (const String& newText)
    {
        document.perform (new CompNameChangeAction (component, *document.getComponentLayout(), newText),
                          T("Change component name"));
    }

    const String getText() const
    {
        return component->getName();
    }

    juce_UseDebuggingNewOperator

private:
    class CompNameChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompNameChangeAction (Component* const comp, ComponentLayout& layout, const String& newName_)
            : ComponentUndoableAction <Component> (comp, layout),
              newName (newName_)
        {
            oldName = comp->getName();
        }

        ~CompNameChangeAction() {}

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
/**
*/
class ComponentMemberNameProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentMemberNameProperty (Component* component_, JucerDocument& document_)
        : ComponentTextProperty <Component> (T("member name"), 40, false, component_, document_)
    {
    }

    ~ComponentMemberNameProperty()
    {
    }

    //==============================================================================
    void setText (const String& newText)
    {
        document.perform (new CompMemberNameChangeAction (component, *document.getComponentLayout(), newText),
                          T("Change component member name"));
    }

    const String getText() const
    {
        return document.getComponentLayout()->getComponentMemberVariableName (component);
    }

    juce_UseDebuggingNewOperator

private:
    class CompMemberNameChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompMemberNameChangeAction (Component* const comp, ComponentLayout& layout, const String& newName_)
            : ComponentUndoableAction <Component> (comp, layout),
              newName (newName_)
        {
            oldName = layout.getComponentMemberVariableName (comp);
        }

        ~CompMemberNameChangeAction() {}

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
/**
*/
class ComponentVirtualClassProperty  : public ComponentTextProperty <Component>
{
public:
    ComponentVirtualClassProperty (Component* component_, JucerDocument& document_)
        : ComponentTextProperty <Component> (T("virtual class"), 40, false, component_, document_)
    {
    }

    ~ComponentVirtualClassProperty()
    {
    }

    //==============================================================================
    void setText (const String& newText)
    {
        document.perform (new CompVirtualClassChangeAction (component, *document.getComponentLayout(), newText),
                          T("Change component virtual class name"));
    }

    const String getText() const
    {
        return document.getComponentLayout()->getComponentVirtualClassName (component);
    }

    juce_UseDebuggingNewOperator

private:
    class CompVirtualClassChangeAction  : public ComponentUndoableAction <Component>
    {
    public:
        CompVirtualClassChangeAction (Component* const comp, ComponentLayout& layout, const String& newName_)
            : ComponentUndoableAction <Component> (comp, layout),
              newName (newName_)
        {
            oldName = layout.getComponentVirtualClassName (comp);
        }

        ~CompVirtualClassChangeAction() {}

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



#endif   // __JUCER_COMPONENTNAMEPROPERTY_JUCEHEADER__
