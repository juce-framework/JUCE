/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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

        bool perform() override
        {
            showCorrectTab();
            getComponent()->setName (newName);
            changed();
            return true;
        }

        bool undo() override
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

        bool perform() override
        {
            showCorrectTab();
            layout.setComponentMemberVariableName (getComponent(), newName);
            return true;
        }

        bool undo() override
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

        bool perform() override
        {
            showCorrectTab();
            layout.setComponentVirtualClassName (getComponent(), newName);
            return true;
        }

        bool undo() override
        {
            showCorrectTab();
            layout.setComponentVirtualClassName (getComponent(), oldName);
            return true;
        }

        String newName, oldName;
    };
};
