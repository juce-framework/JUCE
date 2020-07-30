/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class GenericComponent  : public Component
{
public:
    GenericComponent()
        : Component ("new component"),
          actualClassName ("juce::Component")
    {
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::white.withAlpha (0.25f));

        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRect (getLocalBounds());
        g.drawLine (0.0f, 0.0f, (float) getWidth(), (float) getHeight());
        g.drawLine (0.0f, (float) getHeight(), (float) getWidth(), 0.0f);

        g.setFont (14.0f);
        g.drawText (actualClassName, 0, 0, getWidth(), getHeight() / 2, Justification::centred, true);
    }

    void setClassName (const String& newName)
    {
        if (actualClassName != newName)
        {
            actualClassName = newName;
            repaint();
        }
    }

    void setParams (const String& newParams)
    {
        if (constructorParams != newParams)
        {
            constructorParams = newParams;
            repaint();
        }
    }

    String actualClassName, constructorParams;
};

//==============================================================================
class GenericComponentHandler  : public ComponentTypeHandler
{
public:
    GenericComponentHandler()
        : ComponentTypeHandler ("Generic Component", "GenericComponent", typeid (GenericComponent), 150, 24)
    {}

    Component* createNewComponent (JucerDocument*) override
    {
        return new GenericComponent();
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("class", ((GenericComponent*) comp)->actualClassName);
        e->setAttribute ("params", ((GenericComponent*) comp)->constructorParams);

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        ((GenericComponent*) comp)->actualClassName = xml.getStringAttribute ("class", "juce::Component");
        ((GenericComponent*) comp)->constructorParams = xml.getStringAttribute ("params", String());
        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        props.add (new GenericCompClassProperty (dynamic_cast<GenericComponent*> (component), document));
        props.add (new GenericCompParamsProperty (dynamic_cast<GenericComponent*> (component), document));
    }

    String getClassName (Component* comp) const override
    {
        return static_cast<GenericComponent*> (comp)->actualClassName;
    }

    String getCreationParameters (GeneratedCode&, Component* comp) override
    {
        return static_cast<GenericComponent*> (comp)->constructorParams;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (component->getName().isNotEmpty())
            code.constructorCode
                << memberVariableName << "->setName ("
                << quotedString (component->getName(), false)
                << ");\n\n";
        else
            code.constructorCode << "\n";
    }

private:
    class GenericCompClassProperty  : public ComponentTextProperty <GenericComponent>
    {
    public:
        GenericCompClassProperty (GenericComponent* comp, JucerDocument& doc)
            : ComponentTextProperty <GenericComponent> ("class", 300, false, comp, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new GenericCompClassChangeAction (component, *document.getComponentLayout(),
                                                                build_tools::makeValidIdentifier (newText, false, false, true)),
                              "Change generic component class");
        }

        String getText() const override
        {
            return component->actualClassName;
        }

    private:
        class GenericCompClassChangeAction  : public ComponentUndoableAction <GenericComponent>
        {
        public:
            GenericCompClassChangeAction (GenericComponent* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <GenericComponent> (comp, l),
                  newState (newState_)
            {
                oldState = comp->actualClassName;
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setClassName (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setClassName (oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    class GenericCompParamsProperty  : public ComponentTextProperty <GenericComponent>
    {
    public:
        GenericCompParamsProperty (GenericComponent* comp, JucerDocument& doc)
            : ComponentTextProperty <GenericComponent> ("constructor params", 1024, true, comp, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new GenericCompParamsChangeAction (component, *document.getComponentLayout(), newText),
                              "Change generic component class");
        }

        String getText() const override
        {
            return component->constructorParams;
        }

    private:
        class GenericCompParamsChangeAction  : public ComponentUndoableAction <GenericComponent>
        {
        public:
            GenericCompParamsChangeAction (GenericComponent* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <GenericComponent> (comp, l),
                  newState (newState_)
            {
                oldState = comp->constructorParams;
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setParams (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setParams (oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };
};
