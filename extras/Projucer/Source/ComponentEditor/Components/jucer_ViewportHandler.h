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
class ViewportHandler  : public ComponentTypeHandler
{
public:
    ViewportHandler()
        : ComponentTypeHandler ("Viewport", "juce::Viewport", typeid (UpdatingViewport), 150, 150)
    {}

    Component* createNewComponent (JucerDocument*) override
    {
        Viewport* const v = new UpdatingViewport ("new viewport");
        v->setViewedComponent (new ViewportDemoContentComp());

        return v;
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        Viewport* const v = dynamic_cast<Viewport*> (comp);
        XmlElement* const e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute ("vscroll", v->isVerticalScrollBarShown());
        e->setAttribute ("hscroll", v->isHorizontalScrollBarShown());
        e->setAttribute ("scrollbarThickness", v->getScrollBarThickness());

        e->setAttribute ("contentType", getViewportContentType (v));
        e->setAttribute ("jucerFile", getViewportJucerComponentFile (v));
        e->setAttribute ("contentClass", getViewportGenericComponentClass (v));
        e->setAttribute ("constructorParams", getViewportConstructorParams (v));

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        Viewport defaultViewport;
        Viewport* const v = dynamic_cast<Viewport*> (comp);
        v->setScrollBarsShown (xml.getBoolAttribute ("vscroll", defaultViewport.isVerticalScrollBarShown()),
                               xml.getBoolAttribute ("hscroll", defaultViewport.isHorizontalScrollBarShown()));

        v->setScrollBarThickness (xml.getIntAttribute ("scrollbarThickness", defaultViewport.getScrollBarThickness()));

        setViewportJucerComponentFile (v, xml.getStringAttribute ("jucerFile", String()));
        setViewportGenericComponentClass (v, xml.getStringAttribute ("contentClass"));
        setViewportContentType (v, xml.getIntAttribute ("contentType", 0));
        setViewportConstructorParams (v, xml.getStringAttribute ("constructorParams"));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        auto* v = dynamic_cast<Viewport*> (component);

        props.add (new ViewportScrollbarShownProperty (v, document, true));
        props.add (new ViewportScrollbarShownProperty (v, document, false));
        props.add (new ViewportScrollbarSizeProperty (v, document));
        props.add (new ViewportContentTypeProperty (v, document));

        if (getViewportContentType (v) == 1)
        {
            props.add (new ViewportJucerFileProperty (v, document));
            props.add (new ConstructorParamsProperty (v, document));
        }
        else if (getViewportContentType (v) == 2)
        {
            props.add (new ViewportContentClassProperty (v, document));
            props.add (new ConstructorParamsProperty (v, document));
        }
    }

    String getCreationParameters (GeneratedCode&, Component* comp) override
    {
        return quotedString (comp->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        Viewport defaultViewport;
        Viewport* const v = dynamic_cast<Viewport*> (component);

        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (defaultViewport.isVerticalScrollBarShown() != v->isVerticalScrollBarShown()
             || defaultViewport.isHorizontalScrollBarShown() != v->isHorizontalScrollBarShown())
        {
            code.constructorCode
                << memberVariableName << "->setScrollBarsShown ("
                << CodeHelpers::boolLiteral (v->isVerticalScrollBarShown()) << ", "
                << CodeHelpers::boolLiteral (v->isHorizontalScrollBarShown()) << ");\n";
        }

        if (defaultViewport.getScrollBarThickness() != v->getScrollBarThickness())
        {
            code.constructorCode
                << memberVariableName << "->setScrollBarThickness ("
                << v->getScrollBarThickness() << ");\n";
        }

        if (getViewportContentType (v) != 0)
        {
            String classNm (getViewportGenericComponentClass (v));

            if (getViewportContentType (v) == 1)
            {
                File file;

                const String filename (getViewportJucerComponentFile (v));

                if (filename.isNotEmpty())
                    file = code.document->getCppFile().getSiblingFile (filename);

                std::unique_ptr<JucerDocument> doc (JucerDocument::createForCppFile (nullptr, file));

                if (doc != nullptr)
                {
                    code.includeFilesCPP.add (File::createFileWithoutCheckingPath (doc->getHeaderFile()
                                                                                       .getRelativePathFrom (code.document->getCppFile().getParentDirectory())
                                                                                       .replaceCharacter ('\\', '/')));
                    classNm = doc->getClassName();
                }
                else
                {
                    classNm = String();
                }
            }

            if (classNm.isNotEmpty())
            {
                code.constructorCode
                    << memberVariableName << "->setViewedComponent (new "
                    << classNm;

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
            auto tc = new TestComponent (doc, nullptr, false);

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
        return vp->getProperties() ["contentType"];
    }

    static void setViewportContentType (Viewport* vp, int newValue)
    {
        if (newValue != getViewportContentType (vp))
        {
            vp->getProperties().set ("contentType", newValue);
            updateViewportContentComp (vp);
        }
    }

    static String getViewportJucerComponentFile (Viewport* vp)
    {
        return vp->getProperties() ["jucerFile"].toString();
    }

    static void setViewportJucerComponentFile (Viewport* vp, const String& file)
    {
        if (file != getViewportJucerComponentFile (vp))
        {
            vp->getProperties().set ("jucerFile", file);
            updateViewportContentComp (vp);
        }
    }

    static String getViewportGenericComponentClass (Viewport* vp)
    {
        return vp->getProperties() ["contentClass"].toString();
    }

    static void setViewportGenericComponentClass (Viewport* vp, const String& name)
    {
        if (name != getViewportGenericComponentClass (vp))
        {
            vp->getProperties().set ("contentClass", name);
            updateViewportContentComp (vp);
        }
    }

    static String getViewportConstructorParams (Viewport* vp)
    {
        return vp->getProperties() ["constructorParams"].toString();
    }

    static void setViewportConstructorParams (Viewport* vp, const String& newParams)
    {
        if (newParams != getViewportConstructorParams (vp))
        {
            vp->getProperties().set ("constructorParams", newParams);
            updateViewportContentComp (vp);
        }
    }

private:
    //==============================================================================
    class UpdatingViewport  : public Viewport
    {
    public:
        UpdatingViewport (const String& name)
            : Viewport (name)
        {
        }

        void parentHierarchyChanged()
        {
            Viewport::parentHierarchyChanged();
            updateViewportContentComp (this);
        }
    };

    //==============================================================================
    struct ViewportDemoContentComp : public Component
    {
        ViewportDemoContentComp()
        {
            setSize (2048, 2048);
        }

        void paint (Graphics& g) override
        {
            g.fillCheckerBoard (getLocalBounds().toFloat(), 50.0f, 50.0f,
                                Colours::lightgrey.withAlpha (0.5f),
                                Colours::darkgrey.withAlpha (0.5f));
        }
    };

    //==============================================================================
    class ViewportScrollbarShownProperty  : public ComponentBooleanProperty <Viewport>
    {
    public:
        ViewportScrollbarShownProperty (Viewport* comp, JucerDocument& doc, const bool vertical_)
            : ComponentBooleanProperty <Viewport> (vertical_ ? "V scrollbar" : "H scrollbar",
                                                   "enabled", "enabled",
                                                   comp, doc),
               vertical (vertical_)
        {
        }

        void setState (bool newState)
        {
            document.perform (new ViewportScrollbarChangeAction (component, *document.getComponentLayout(), vertical, newState),
                              "Change Viewport scrollbar");
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
            ViewportScrollbarChangeAction (Viewport* const comp, ComponentLayout& l, const bool vertical_, const bool newState_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
                                           private ChangeListener
    {
    public:
        ViewportScrollbarSizeProperty (Viewport* comp, JucerDocument& doc)
            : SliderPropertyComponent ("scrollbar size", 3.0, 30.0, 1.0, 1.0),
              component (comp),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~ViewportScrollbarSizeProperty() override
        {
            document.removeChangeListener (this);
        }

        void setValue (double newValue) override
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new ViewportScrollbarSizeChangeAction (component, *document.getComponentLayout(), roundToInt (newValue)),
                              "Change Viewport scrollbar size");
        }

        double getValue() const override
        {
            return component->getScrollBarThickness();
        }

    private:
        void changeListenerCallback (ChangeBroadcaster*) override
        {
            refresh();
        }

        Viewport* component;
        JucerDocument& document;

    private:
        class ViewportScrollbarSizeChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportScrollbarSizeChangeAction (Viewport* const comp, ComponentLayout& l, const int newState_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
        ViewportContentTypeProperty (Viewport* comp, JucerDocument& doc)
            : ComponentChoiceProperty <Viewport> ("content", comp, doc)
        {
            choices.add ("No content component");
            choices.add ("Jucer content component");
            choices.add ("Named content component");
        }

        void setIndex (int newIndex)
        {
            document.perform (new ViewportContentTypeChangeAction (component, *document.getComponentLayout(), newIndex),
                              "Change Viewport content type");
        }

        int getIndex() const
        {
            return getViewportContentType (component);
        }

    private:
        class ViewportContentTypeChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportContentTypeChangeAction (Viewport* const comp, ComponentLayout& l, const int newValue_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
                                        private ChangeListener
    {
    public:
        ViewportJucerFileProperty (Viewport* const comp, JucerDocument& doc)
            : FilePropertyComponent ("Jucer file", false, true),
              component (comp),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~ViewportJucerFileProperty() override
        {
            document.removeChangeListener (this);
        }

        void setFile (const File& newFile) override
        {
            document.perform (new JucerCompFileChangeAction (component, *document.getComponentLayout(),
                                                             newFile.getRelativePathFrom (document.getCppFile().getParentDirectory())
                                                                    .replaceCharacter ('\\', '/')),
                              "Change Projucer component file");
        }

        File getFile() const override
        {
            auto filename = getViewportJucerComponentFile (component);

            if (filename.isEmpty())
                return {};

            return document.getCppFile().getSiblingFile (filename);
        }

    private:
        void changeListenerCallback (ChangeBroadcaster*) override
        {
            refresh();
        }

        Viewport* const component;
        JucerDocument& document;

        class JucerCompFileChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            JucerCompFileChangeAction (Viewport* const comp, ComponentLayout& l, const String& newState_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
        ViewportContentClassProperty (Viewport* comp, JucerDocument& doc)
            : ComponentTextProperty <Viewport> ("content class", 256, false, comp, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new ViewportClassNameChangeAction (component, *document.getComponentLayout(), newText),
                              "Change Viewport content class");
        }

        String getText() const override
        {
            return getViewportGenericComponentClass (component);
        }

    private:
        class ViewportClassNameChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ViewportClassNameChangeAction (Viewport* const comp, ComponentLayout& l, const String& newValue_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
        ConstructorParamsProperty (Viewport* comp, JucerDocument& doc)
            : ComponentTextProperty <Viewport> ("constructor params", 512, false, comp, doc)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new ConstructorParamChangeAction (component, *document.getComponentLayout(), newText),
                              "Change Viewport content constructor params");
        }

        String getText() const override
        {
            return getViewportConstructorParams (component);
        }

    private:
        class ConstructorParamChangeAction  : public ComponentUndoableAction <Viewport>
        {
        public:
            ConstructorParamChangeAction (Viewport* const comp, ComponentLayout& l, const String& newValue_)
                : ComponentUndoableAction <Viewport> (comp, l),
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
