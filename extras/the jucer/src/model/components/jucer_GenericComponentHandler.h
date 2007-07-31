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

#ifndef __JUCER_GENERICCOMPONENTHANDLER_JUCEHEADER__
#define __JUCER_GENERICCOMPONENTHANDLER_JUCEHEADER__


//==============================================================================
/**
*/
class GenericComponent  : public Component
{
public:
    GenericComponent()
        : Component (T("new component")),
          actualClassName (T("Component"))
    {
    }

    ~GenericComponent()
    {
    }

    void paint (Graphics& g)
    {
        g.fillAll (Colours::white.withAlpha (0.25f));

        g.setColour (Colours::black.withAlpha (0.5f));
        g.drawRect (0, 0, getWidth(), getHeight());
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
/**
*/
class GenericComponentHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    GenericComponentHandler()
        : ComponentTypeHandler ("Generic Component", "GenericComponent", typeid (GenericComponent), 150, 24)
    {}

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new GenericComponent();
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        XmlElement* e = ComponentTypeHandler::createXmlFor (comp, layout);
        e->setAttribute (T("class"), ((GenericComponent*) comp)->actualClassName);
        e->setAttribute (T("params"), ((GenericComponent*) comp)->constructorParams);

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        ((GenericComponent*) comp)->actualClassName = xml.getStringAttribute (T("class"), T("Component"));
        ((GenericComponent*) comp)->constructorParams = xml.getStringAttribute (T("params"), String::empty);
        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        properties.add (new GenericCompClassProperty (dynamic_cast <GenericComponent*> (component), document));
        properties.add (new GenericCompParamsProperty (dynamic_cast <GenericComponent*> (component), document));
    }

    const String getClassName (Component* comp) const
    {
        return ((GenericComponent*) comp)->actualClassName;
    }

    const String getCreationParameters (Component* comp)
    {
        return ((GenericComponent*) comp)->constructorParams;
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (component->getName().isNotEmpty())
            code.constructorCode
                << memberVariableName << "->setName ("
                << quotedString (component->getName())
                << ");\n\n";
        else
            code.constructorCode << "\n";
    }

    juce_UseDebuggingNewOperator

private:
    class GenericCompClassProperty  : public ComponentTextProperty <GenericComponent>
    {
    public:
        GenericCompClassProperty (GenericComponent* comp, JucerDocument& document)
            : ComponentTextProperty <GenericComponent> (T("class"), 300, false, comp, document)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new GenericCompClassChangeAction (component, *document.getComponentLayout(),
                                                                makeValidCppIdentifier (newText, false, false, true)),
                              T("Change generic component class"));
        }

        const String getText() const
        {
            return component->actualClassName;
        }

    private:
        class GenericCompClassChangeAction  : public ComponentUndoableAction <GenericComponent>
        {
        public:
            GenericCompClassChangeAction (GenericComponent* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <GenericComponent> (comp, layout),
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
        GenericCompParamsProperty (GenericComponent* comp, JucerDocument& document)
            : ComponentTextProperty <GenericComponent> (T("constructor params"), 1024, true, comp, document)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new GenericCompParamsChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change generic component class"));
        }

        const String getText() const
        {
            return component->constructorParams;
        }

    private:
        class GenericCompParamsChangeAction  : public ComponentUndoableAction <GenericComponent>
        {
        public:
            GenericCompParamsChangeAction (GenericComponent* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <GenericComponent> (comp, layout),
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


#endif   // __JUCER_GENERICCOMPONENTHANDLER_JUCEHEADER__
