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

#ifndef __JUCER_VIEWPORTHANDLER_JUCEHEADER__
#define __JUCER_VIEWPORTHANDLER_JUCEHEADER__


//==============================================================================
/**
*/
class ViewportHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    ViewportHandler()
        : ComponentTypeHandler ("Viewport", "Viewport", typeid (UpdatingViewport), 150, 150)
    {}

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        Viewport* const v = new UpdatingViewport (T("new viewport"));
        v->setViewedComponent (new ViewportDemoContentComp());

        return v;
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        Viewport* const v = dynamic_cast <Viewport*> (comp);
        XmlElement* const e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute (T("vscroll"), v->isVerticalScrollBarShown());
        e->setAttribute (T("hscroll"), v->isHorizontalScrollBarShown());
        e->setAttribute (T("scrollbarThickness"), v->getScrollBarThickness());

        e->setAttribute (T("contentType"), getViewportContentType (v));
        e->setAttribute (T("jucerFile"), getViewportJucerComponentFile (v));
        e->setAttribute (T("contentClass"), getViewportGenericComponentClass (v));
        e->setAttribute (T("constructorParams"), getViewportConstructorParams (v));

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Viewport defaultViewport;
        Viewport* const v = dynamic_cast <Viewport*> (comp);
        v->setScrollBarsShown (xml.getBoolAttribute (T("vscroll"), defaultViewport.isVerticalScrollBarShown()),
                               xml.getBoolAttribute (T("hscroll"), defaultViewport.isHorizontalScrollBarShown()));

        v->setScrollBarThickness (xml.getIntAttribute (T("scrollbarThickness"), defaultViewport.getScrollBarThickness()));

        setViewportJucerComponentFile (v, xml.getStringAttribute (T("jucerFile"), String::empty));
        setViewportGenericComponentClass (v, xml.getStringAttribute (T("contentClass")));
        setViewportContentType (v, xml.getIntAttribute (T("contentType"), 0));
        setViewportConstructorParams (v, xml.getStringAttribute (T("constructorParams")));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);

        Viewport* const v = dynamic_cast <Viewport*> (component);

        properties.add (new ViewportScrollbarShownProperty (v, document, true));
        properties.add (new ViewportScrollbarShownProperty (v, document, false));
        properties.add (new ViewportScrollbarSizeProperty (v, document));
        properties.add (new ViewportContentTypeProperty (v, document));

        if (getViewportContentType (v) == 1)
        {
            properties.add (new ViewportJucerFileProperty (v, document));
            properties.add (new ConstructorParamsProperty (v, document));
        }
        else if (getViewportContentType (v) == 2)
        {
            properties.add (new ViewportContentClassProperty (v, document));
            properties.add (new ConstructorParamsProperty (v, document));
        }
    }

    const String getCreationParameters (Component* comp)
    {
        return quotedString (comp->getName());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        Viewport defaultViewport;
        Viewport* const v = dynamic_cast <Viewport*> (component);

        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (defaultViewport.isVerticalScrollBarShown() != v->isVerticalScrollBarShown()
             || defaultViewport.isHorizontalScrollBarShown() != v->isHorizontalScrollBarShown())
        {
            code.constructorCode
                << memberVariableName << "->setScrollBarsShown ("
                << boolToString (v->isVerticalScrollBarShown()) << ", "
                << boolToString (v->isHorizontalScrollBarShown()) << ");\n";
        }

        if (defaultViewport.getScrollBarThickness() != v->getScrollBarThickness())
        {
            code.constructorCode
                << memberVariableName << "->setScrollBarThickness ("
                << v->getScrollBarThickness() << ");\n";
        }

        if (getViewportContentType (v) != 0)
        {
            String className (getViewportGenericComponentClass (v));

            if (getViewportContentType (v) == 1)
            {
                File file;

                const String filename (getViewportJucerComponentFile (v));

                if (filename.isNotEmpty())
                    file = code.document->getFile().getSiblingFile (filename);

                JucerDocument* doc = ObjectTypes::loadDocumentFromFile (file, false);

                if (doc != 0)
                {
                    code.includeFilesCPP.add (doc->getFile().withFileExtension (T("h"))
                                                .getRelativePathFrom (code.document->getFile().getParentDirectory())
                                                .replaceCharacter (T('\\'), T('/')));

                    className = doc->getClassName();
                    delete doc;
                }
                else
                {
                    className = String::empty;
                }
            }

            if (className.isNotEmpty())
            {
                code.constructorCode
                    << memberVariableName << "->setViewedComponent (new "
                    << className;

                if (getViewportConstructorParams (v).trim().isNotEmpty())
                {
                    code.constructorCode << " (" << getViewportConstructorParams (v).trim() << "));\n";
                }
                else
                {
                    code.constructorCode << "());\n";
                }
            }
        }

        code.constructorCode << "\n";
    }

    static void updateViewportContentComp (Viewport* vp)
    {
        if (getViewportContentType (vp) == 1)
        {
            JucerDocument* doc = findParentDocument (vp);
            TestComponent* tc = new TestComponent (doc, 0, false);

            tc->setFilename (getViewportJucerComponentFile (vp));
            tc->setToInitialSize();

            vp->setViewedComponent (tc);
        }
        else
        {
            vp->setViewedComponent (new ViewportDemoContentComp());
        }
    }

    static int getViewportContentType (Viewport* vp)
    {
        return vp->getComponentPropertyInt (T("contentType"), false, 0);
    }

    static void setViewportContentType (Viewport* vp, int newValue)
    {
        if (newValue != getViewportContentType (vp))
        {
            vp->setComponentProperty (T("contentType"), newValue);
            updateViewportContentComp (vp);
        }
    }

    static const String getViewportJucerComponentFile (Viewport* vp)
    {
        return vp->getComponentProperty (T("jucerFile"), false);
    }

    static void setViewportJucerComponentFile (Viewport* vp, const String& file)
    {
        if (file != getViewportJucerComponentFile (vp))
        {
            vp->setComponentProperty (T("jucerFile"), file);
            updateViewportContentComp (vp);
        }
    }

    static const String getViewportGenericComponentClass (Viewport* vp)
    {
        return vp->getComponentProperty (T("contentClass"), false);
    }

    static void setViewportGenericComponentClass (Viewport* vp, const String& className)
    {
        if (className != getViewportGenericComponentClass (vp))
        {
            vp->setComponentProperty (T("contentClass"), className);
            updateViewportContentComp (vp);
        }
    }

    static const String getViewportConstructorParams (Viewport* vp)
    {
        return vp->getComponentProperty (T("constructorParams"), false);
    }

    static void setViewportConstructorParams (Viewport* vp, const String& newParams)
    {
        if (newParams != getViewportConstructorParams (vp))
        {
            vp->setComponentProperty (T("constructorParams"), newParams);
            updateViewportContentComp (vp);
        }
    }

    juce_UseDebuggingNewOperator

private:
    //==============================================================================
    class UpdatingViewport  : public Viewport
    {
    public:
        UpdatingViewport (const String& name)
            : Viewport (name)
        {
        }

        ~UpdatingViewport()
        {
        }

        void parentHierarchyChanged()
        {
            Viewport::parentHierarchyChanged();
            updateViewportContentComp (this);
        }
    };

    //==============================================================================
    class ViewportDemoContentComp : public Component
    {
    public:
        ViewportDemoContentComp()
        {
            setSize (2048, 2048);
        }

        ~ViewportDemoContentComp()
        {
        }

        void paint (Graphics& g)
        {
            g.fillCheckerBoard (0, 0, getWidth(), getHeight(),
                                50, 50,
                                Colours::lightgrey.withAlpha (0.5f),
                                Colours::darkgrey.withAlpha (0.5f));
        }
    };

    //==============================================================================
    class ViewportScrollbarShownProperty  : public ComponentBooleanProperty <Viewport>
    {
    public:
        ViewportScrollbarShownProperty (Viewport* comp, JucerDocument& document, const bool vertical_)
            : ComponentBooleanProperty <Viewport> (vertical_ ? T("V scrollbar") : T("H scrollbar"),
                                                   T("enabled"), T("enabled"),
                                                   comp, document),
               vertical (vertical_)
        {
        }

        void setState (const bool newState)
        {
            document.perform (new ViewportScrollbarChangeAction (component, *document.getComponentLayout(), vertical, newState),
                              T("Change Viewport scrollbar"));
        }

        bool getState() const
        {
            return vertical ? component->isVerticalScrollBarShown()
                            : component->isHorizontalScrollBarShown();
        }

        const bool vertical;

    private:
        class ViewportScrollbarChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportScrollbarChangeAction (Viewport* const comp, ComponentLayout& layout, const bool vertical_, const bool newState_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  vertical (vertical_),
                  newState (newState_)
            {
                oldState = vertical ? comp->isVerticalScrollBarShown()
                                    : comp->isHorizontalScrollBarShown();
            }

            bool perform()
            {
                showCorrectTab();
                if (vertical)
                    getComponent()->setScrollBarsShown (newState, getComponent()->isHorizontalScrollBarShown());
                else
                    getComponent()->setScrollBarsShown (getComponent()->isVerticalScrollBarShown(), newState);

                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                if (vertical)
                    getComponent()->setScrollBarsShown (oldState, getComponent()->isHorizontalScrollBarShown());
                else
                    getComponent()->setScrollBarsShown (getComponent()->isVerticalScrollBarShown(), oldState);
                changed();
                return true;
            }

            bool vertical, newState, oldState;
        };
    };

    //==============================================================================
    class ViewportScrollbarSizeProperty  : public SliderPropertyComponent,
                                           public ChangeListener
    {
    public:
        ViewportScrollbarSizeProperty (Viewport* comp, JucerDocument& document_)
            : SliderPropertyComponent (T("scrollbar size"), 3.0, 30.0, 1.0, 1.0),
              component (comp),
              document (document_)
        {
            document.addChangeListener (this);
        }

        ~ViewportScrollbarSizeProperty()
        {
            document.removeChangeListener (this);
        }

        void setValue (const double newValue)
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new ViewportScrollbarSizeChangeAction (component, *document.getComponentLayout(), roundDoubleToInt (newValue)),
                              T("Change Viewport scrollbar size"));
        }

        const double getValue() const
        {
            return component->getScrollBarThickness();
        }

        void changeListenerCallback (void*)
        {
            refresh();
        }

        Viewport* component;
        JucerDocument& document;

    private:
        class ViewportScrollbarSizeChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportScrollbarSizeChangeAction (Viewport* const comp, ComponentLayout& layout, const int newState_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  newState (newState_)
            {
                oldState = comp->getScrollBarThickness();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setScrollBarThickness (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setScrollBarThickness (newState);
                changed();
                return true;
            }

            int newState, oldState;
        };
    };

    //==============================================================================
    class ViewportContentTypeProperty  : public ComponentChoiceProperty <Viewport>
    {
    public:
        ViewportContentTypeProperty (Viewport* comp, JucerDocument& document)
            : ComponentChoiceProperty <Viewport> (T("content"), comp, document)
        {
            choices.add (T("No content component"));
            choices.add (T("Jucer content component"));
            choices.add (T("Named content component"));
        }

        void setIndex (const int newIndex)
        {
            document.perform (new ViewportContentTypeChangeAction (component, *document.getComponentLayout(), newIndex),
                              T("Change Viewport content type"));
        }

        int getIndex() const
        {
            return getViewportContentType (component);
        }

    private:
        class ViewportContentTypeChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportContentTypeChangeAction (Viewport* const comp, ComponentLayout& layout, const int newValue_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  newValue (newValue_)
            {
                oldValue = getViewportContentType (comp);
            }

            bool perform()
            {
                showCorrectTab();
                setViewportContentType (getComponent(), newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setViewportContentType (getComponent(), oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            int newValue, oldValue;
        };
    };

    //==============================================================================
    class ViewportJucerFileProperty   : public FilePropertyComponent,
                                        public ChangeListener
    {
    public:
        ViewportJucerFileProperty (Viewport* const component_, JucerDocument& document_)
            : FilePropertyComponent (T("Jucer file"), false, true),
              component (component_),
              document (document_)
        {
            document.addChangeListener (this);
        }

        ~ViewportJucerFileProperty()
        {
            document.removeChangeListener (this);
        }

        //==============================================================================
        void setFile (const File& newFile)
        {
            document.perform (new JucerCompFileChangeAction (component, *document.getComponentLayout(),
                                                             newFile.getRelativePathFrom (document.getFile().getParentDirectory())
                                                                    .replaceCharacter (T('\\'), T('/'))
),
                              T("Change Jucer component file"));
        }

        const File getFile() const
        {
            const String filename (getViewportJucerComponentFile (component));

            if (filename.isEmpty())
                return File::nonexistent;

            return document.getFile().getSiblingFile (filename);
        }

        void changeListenerCallback (void*)
        {
            refresh();
        }

    private:
        Viewport* const component;
        JucerDocument& document;

        class JucerCompFileChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            JucerCompFileChangeAction (Viewport* const comp, ComponentLayout& layout, const String& newState_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  newState (newState_)
            {
                oldState = getViewportJucerComponentFile (comp);
            }

            bool perform()
            {
                showCorrectTab();
                setViewportJucerComponentFile (getComponent(), newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setViewportJucerComponentFile (getComponent(), oldState);
                changed();
                return true;
            }

            String newState, oldState;
        };
    };

    //==============================================================================
    class ViewportContentClassProperty   : public ComponentTextProperty <Viewport>
    {
    public:
        ViewportContentClassProperty (Viewport* comp, JucerDocument& document)
            : ComponentTextProperty <Viewport> (T("content class"), 256, false, comp, document)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new ViewportClassNameChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change Viewport content class"));
        }

        const String getText() const
        {
            return getViewportGenericComponentClass (component);
        }

    private:
        class ViewportClassNameChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportClassNameChangeAction (Viewport* const comp, ComponentLayout& layout, const String& newValue_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  newValue (newValue_)
            {
                oldValue = getViewportGenericComponentClass (comp);
            }

            bool perform()
            {
                showCorrectTab();
                setViewportGenericComponentClass (getComponent(), newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setViewportGenericComponentClass (getComponent(), oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            String newValue, oldValue;
        };
    };

    //==============================================================================
    class ConstructorParamsProperty   : public ComponentTextProperty <Viewport>
    {
    public:
        ConstructorParamsProperty (Viewport* comp, JucerDocument& document)
            : ComponentTextProperty <Viewport> (T("constructor params"), 512, false, comp, document)
        {
        }

        void setText (const String& newText)
        {
            document.perform (new ConstructorParamChangeAction (component, *document.getComponentLayout(), newText),
                              T("Change Viewport content constructor params"));
        }

        const String getText() const
        {
            return getViewportConstructorParams (component);
        }

    private:
        int tabIndex;

        class ConstructorParamChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ConstructorParamChangeAction (Viewport* const comp, ComponentLayout& layout, const String& newValue_)
                : ComponentUndoableAction <Viewport> (comp, layout),
                  newValue (newValue_)
            {
                oldValue = getViewportConstructorParams (comp);
            }

            bool perform()
            {
                showCorrectTab();
                setViewportConstructorParams (getComponent(), newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setViewportConstructorParams (getComponent(), oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            String newValue, oldValue;
        };
    };
};


#endif   // __JUCER_VIEWPORTHANDLER_JUCEHEADER__
