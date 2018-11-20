/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../UI/jucer_TestComponent.h"
#include "../Properties/jucer_FilePropertyComponent.h"
#include "../Properties/jucer_ComponentTextProperty.h"
#include "jucer_ComponentUndoableAction.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
class JucerComponentHandler  : public ComponentTypeHandler
{
public:
    JucerComponentHandler()
        : ComponentTypeHandler ("Projucer Component", "xxx",
                                typeid (TestComponent), 300, 200)
    {}

    Component* createNewComponent (JucerDocument* doc) override
    {
        return new TestComponent (doc, 0, false);
    }

    String getXmlTagName() const noexcept override               { return "JUCERCOMP"; }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("sourceFile", tc->getFilename());
        e->setAttribute ("constructorParams", tc->getConstructorParams());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        tc->setFilename (xml.getStringAttribute ("sourceFile", tc->getFilename()));
        tc->setConstructorParams (xml.getStringAttribute ("constructorParams"));

        return true;
    }

    String getClassName (Component* comp) const override
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        String jucerCompClassName;

        if (tc->getDocument() != 0)
            jucerCompClassName = tc->getDocument()->getClassName();

        if (jucerCompClassName.isEmpty())
            jucerCompClassName = "Component";

        return jucerCompClassName;
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* const tc = dynamic_cast<TestComponent*> (component))
        {
            props.add (new JucerCompFileProperty (tc, document));
            props.add (new ConstructorParamsProperty (tc, document));
            props.add (new JucerCompOpenDocProperty (tc));
        }
    }

    String getCreationParameters (GeneratedCode&, Component* component) override
    {
        return dynamic_cast<TestComponent*> (component)->getConstructorParams().trim();
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (TestComponent* const tc = dynamic_cast<TestComponent*> (component))
            code.includeFilesH.add (tc->findFile().withFileExtension (".h"));
        else
            jassertfalse;
    }

    //==============================================================================
    class JucerCompFileChangeAction  : public ComponentUndoableAction <TestComponent>
    {
    public:
        JucerCompFileChangeAction (TestComponent* const comp, ComponentLayout& l, const String& newState_)
            : ComponentUndoableAction <TestComponent> (comp, l),
              newState (newState_)
        {
            oldState = comp->getFilename();
        }

        bool perform()
        {
            showCorrectTab();
            getComponent()->setFilename (newState);
            changed();
            return true;
        }

        bool undo()
        {
            showCorrectTab();
            getComponent()->setFilename (oldState);
            changed();
            return true;
        }

        String newState, oldState;
    };

    static void setJucerComponentFile (JucerDocument& document, TestComponent* comp, const String& newFilename)
    {
        jassert (comp != nullptr);

        if (comp != nullptr)
            document.perform (new JucerCompFileChangeAction (comp, *document.getComponentLayout(), newFilename),
                              "Change Projucer component file");
    }

private:
    //==============================================================================
    class JucerCompFileProperty  : public FilePropertyComponent,
                                   public ChangeListener
    {
    public:
        JucerCompFileProperty (TestComponent* const comp, JucerDocument& doc)
            : FilePropertyComponent ("Jucer file", false, true),
              component (comp),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~JucerCompFileProperty()
        {
            document.removeChangeListener (this);
        }

        void setFile (const File& newFile)
        {
            setJucerComponentFile (document, component,
                                   newFile.getRelativePathFrom (document.getCppFile().getParentDirectory())
                                          .replaceCharacter ('\\', '/'));
        }

        File getFile() const
        {
            return component->findFile();
        }

        void changeListenerCallback (ChangeBroadcaster*)
        {
            refresh();
        }

    private:
        TestComponent* const component;
        JucerDocument& document;
    };

    //==============================================================================
    struct JucerCompOpenDocProperty  : public ButtonPropertyComponent
    {
        JucerCompOpenDocProperty (TestComponent* const c)
            : ButtonPropertyComponent ("edit", false),
              component (c)
        {
        }

        void buttonClicked()
        {
            if (ProjectContentComponent* const pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showEditorForFile (component->findFile(), true);
        }

        String getButtonText() const
        {
            return "Open file for editing";
        }

        TestComponent* const component;
    };

    //==============================================================================
    struct ConstructorParamsProperty   : public ComponentTextProperty <TestComponent>
    {
        ConstructorParamsProperty (TestComponent* comp, JucerDocument& doc)
            : ComponentTextProperty <TestComponent> ("constructor params", 512, false, comp, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new ConstructorParamChangeAction (component, *document.getComponentLayout(), newText),
                              "Change Viewport content constructor params");
        }

        String getText() const override
        {
            return component->getConstructorParams();
        }

    private:
        struct ConstructorParamChangeAction  : public ComponentUndoableAction <TestComponent>
        {
            ConstructorParamChangeAction (TestComponent* const comp, ComponentLayout& l, const String& newValue_)
                : ComponentUndoableAction <TestComponent> (comp, l),
                  newValue (newValue_)
            {
                oldValue = comp->getConstructorParams();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setConstructorParams (newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setConstructorParams (oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            String newValue, oldValue;
        };
    };
};
