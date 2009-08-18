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

#ifndef __JUCER_JUCERCOMPONENTHANDLER_JUCEHEADER__
#define __JUCER_JUCERCOMPONENTHANDLER_JUCEHEADER__

#include "../../ui/jucer_TestComponent.h"
#include "../../properties/jucer_FilePropertyComponent.h"
#include "../../properties/jucer_ComponentTextProperty.h"
#include "../../ui/jucer_MainWindow.h"
#include "jucer_ComponentUndoableAction.h"


//==============================================================================
/**
*/
class JucerComponentHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    JucerComponentHandler()
        : ComponentTypeHandler ("Jucer Component", "xxx", typeid (TestComponent), 300, 200)
    {}

    //==============================================================================
    Component* createNewComponent (JucerDocument* doc)
    {
        return new TestComponent (doc, 0, false);
    }

    const String getXmlTagName() const throw()                  { return T("JUCERCOMP"); }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        TestComponent* const tc = dynamic_cast <TestComponent*> (comp);

        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute (T("sourceFile"), tc->getFilename());
        e->setAttribute (T("constructorParams"), tc->getConstructorParams());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        TestComponent* const tc = dynamic_cast <TestComponent*> (comp);

        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        tc->setFilename (xml.getStringAttribute (T("sourceFile"), tc->getFilename()));
        tc->setConstructorParams (xml.getStringAttribute (T("constructorParams")));

        return true;
    }

    const String getClassName (Component* comp) const
    {
        TestComponent* const tc = dynamic_cast <TestComponent*> (comp);

        String jucerCompClassName;

        if (tc->getDocument() != 0)
            jucerCompClassName = tc->getDocument()->getClassName();

        if (jucerCompClassName.isEmpty())
            jucerCompClassName = T("Component");

        return jucerCompClassName;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        TestComponent* const tc = dynamic_cast <TestComponent*> (component);

        ComponentTypeHandler::getEditableProperties (component, document, properties);

        properties.add (new JucerCompFileProperty (tc, document));
        properties.add (new ConstructorParamsProperty (tc, document));
        properties.add (new JucerCompOpenDocProperty (tc));
    }

    const String getCreationParameters (Component* component)
    {
        TestComponent* const tc = dynamic_cast <TestComponent*> (component);

        return tc->getConstructorParams().trim();
    }
    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        TestComponent* const tc = dynamic_cast <TestComponent*> (component);

        code.includeFilesH.add (tc->getFilename().replace (T(".cpp"), T(".h")));
    }

    //==============================================================================
    class JucerCompFileChangeAction  : public ComponentUndoableAction <TestComponent>
    {
    public:
        JucerCompFileChangeAction (TestComponent* const comp, ComponentLayout& layout, const String& newState_)
            : ComponentUndoableAction <TestComponent> (comp, layout),
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
        jassert (comp != 0);

        if (comp != 0)
            document.perform (new JucerCompFileChangeAction (comp, *document.getComponentLayout(), newFilename),
                              T("Change Jucer component file"));
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:

    //==============================================================================
    class JucerCompFileProperty  : public FilePropertyComponent,
                                   public ChangeListener
    {
    public:
        JucerCompFileProperty (TestComponent* const component_, JucerDocument& document_)
            : FilePropertyComponent (T("Jucer file"), false, true),
              component (component_),
              document (document_)
        {
            document.addChangeListener (this);
        }

        ~JucerCompFileProperty()
        {
            document.removeChangeListener (this);
        }

        //==============================================================================
        void setFile (const File& newFile)
        {
            setJucerComponentFile (document, component,
                                   newFile.getRelativePathFrom (document.getFile().getParentDirectory())
                                          .replaceCharacter (T('\\'), T('/')));
        }

        const File getFile() const
        {
            return component->findFile();
        }

        void changeListenerCallback (void*)
        {
            refresh();
        }

    private:
        TestComponent* const component;
        JucerDocument& document;
    };

    //==============================================================================
    class JucerCompOpenDocProperty  : public ButtonPropertyComponent
    {
    public:
        JucerCompOpenDocProperty (TestComponent* const component_)
            : ButtonPropertyComponent (T("edit"), false),
              component (component_)
        {
        }

        void buttonClicked()
        {
            MainWindow* const mw = findParentComponentOfClass ((MainWindow*) 0);

            jassert (mw != 0);
            if (mw != 0)
                mw->openFile (component->findFile());
        }

        const String getButtonText() const
        {
            return T("Open file for editing");
        }

    private:
        TestComponent* const component;
    };

    //==============================================================================
    class ConstructorParamsProperty   : public ComponentTextProperty <TestComponent>
    {
    public:
        ConstructorParamsProperty (TestComponent* comp, JucerDocument& document)
            : ComponentTextProperty <TestComponent> (T("constructor params"), 512, false, comp, document)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new ConstructorParamChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change Viewport content constructor params"));
        }

        const String getText() const
        {
            return component->getConstructorParams();
        }

    private:
        int tabIndex;

        class ConstructorParamChangeAction  : public ComponentUndoableAction <TestComponent>
        {
        public:
            ConstructorParamChangeAction (TestComponent* const comp, ComponentLayout& layout, const String& newValue_)
                : ComponentUndoableAction <TestComponent> (comp, layout),
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


#endif   // __JUCER_JUCERCOMPONENTHANDLER_JUCEHEADER__
