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

#ifndef JUCER_JUCERCOMPONENTHANDLER_H_INCLUDED
#define JUCER_JUCERCOMPONENTHANDLER_H_INCLUDED

#include "../ui/jucer_TestComponent.h"
#include "../properties/jucer_FilePropertyComponent.h"
#include "../properties/jucer_ComponentTextProperty.h"
#include "jucer_ComponentUndoableAction.h"
#include "../../Project/jucer_ProjectContentComponent.h"


//==============================================================================
class JucerComponentHandler  : public ComponentTypeHandler
{
public:
    JucerComponentHandler()
        : ComponentTypeHandler ("Projucer Component", "xxx",
                                typeid (TestComponent), 300, 200)
    {}

    Component* createNewComponent (JucerDocument* doc)
    {
        return new TestComponent (doc, 0, false);
    }

    String getXmlTagName() const noexcept                 { return "JUCERCOMP"; }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute ("sourceFile", tc->getFilename());
        e->setAttribute ("constructorParams", tc->getConstructorParams());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        tc->setFilename (xml.getStringAttribute ("sourceFile", tc->getFilename()));
        tc->setConstructorParams (xml.getStringAttribute ("constructorParams"));

        return true;
    }

    String getClassName (Component* comp) const
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (comp);

        String jucerCompClassName;

        if (tc->getDocument() != 0)
            jucerCompClassName = tc->getDocument()->getClassName();

        if (jucerCompClassName.isEmpty())
            jucerCompClassName = "Component";

        return jucerCompClassName;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        TestComponent* const tc = dynamic_cast<TestComponent*> (component);

        ComponentTypeHandler::getEditableProperties (component, document, props);

        props.add (new JucerCompFileProperty (tc, document));
        props.add (new ConstructorParamsProperty (tc, document));
        props.add (new JucerCompOpenDocProperty (tc));
    }

    String getCreationParameters (GeneratedCode&, Component* component)
    {
        return dynamic_cast<TestComponent*> (component)->getConstructorParams().trim();
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
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


#endif   // JUCER_JUCERCOMPONENTHANDLER_H_INCLUDED
