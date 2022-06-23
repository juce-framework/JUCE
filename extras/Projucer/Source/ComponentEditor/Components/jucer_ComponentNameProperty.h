/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "jucer_ComponentTypeHandler.h"
#include "jucer_ComponentUndoableAction.h"
#include "../Properties/jucer_ComponentTextProperty.h"

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
