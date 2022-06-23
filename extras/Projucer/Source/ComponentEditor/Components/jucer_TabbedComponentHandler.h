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


//==============================================================================
class TabbedComponentHandler  : public ComponentTypeHandler
{
public:
    TabbedComponentHandler()
        : ComponentTypeHandler ("Tabbed Component", "juce::TabbedComponent", typeid (TabbedComponent), 200, 150)
    {}

    Component* createNewComponent (JucerDocument*) override
    {
        TabbedComponent* const t = new TabbedComponent (TabbedButtonBar::TabsAtTop);
        t->setName ("new tabbed component");

        for (int i = 3; --i >= 0;)
            addNewTab (t);

        return t;
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        TabbedComponent* const t = dynamic_cast<TabbedComponent*> (comp);
        XmlElement* const e = ComponentTypeHandler::createXmlFor (comp, layout);

        if (t->getOrientation() == TabbedButtonBar::TabsAtTop)           e->setAttribute ("orientation", "top");
        else if (t->getOrientation() == TabbedButtonBar::TabsAtBottom)   e->setAttribute ("orientation", "bottom");
        else if (t->getOrientation() == TabbedButtonBar::TabsAtLeft)     e->setAttribute ("orientation", "left");
        else if (t->getOrientation() == TabbedButtonBar::TabsAtRight)    e->setAttribute ("orientation", "right");

        e->setAttribute ("tabBarDepth", t->getTabBarDepth());
        e->setAttribute ("initialTab", t->getCurrentTabIndex());

        for (int i = 0; i < t->getNumTabs(); ++i)
            e->addChildElement (getTabState (t, i).release());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        TabbedComponent* const t = dynamic_cast<TabbedComponent*> (comp);

        if (xml.getStringAttribute ("orientation") == "top")          t->setOrientation (TabbedButtonBar::TabsAtTop);
        else if (xml.getStringAttribute ("orientation") == "bottom")  t->setOrientation (TabbedButtonBar::TabsAtBottom);
        else if (xml.getStringAttribute ("orientation") == "left")    t->setOrientation (TabbedButtonBar::TabsAtLeft);
        else if (xml.getStringAttribute ("orientation") == "right")   t->setOrientation (TabbedButtonBar::TabsAtRight);

        TabbedComponent defaultTabComp (TabbedButtonBar::TabsAtTop);

        t->setTabBarDepth (xml.getIntAttribute ("tabBarDepth", defaultTabComp.getTabBarDepth()));

        t->clearTabs();

        for (auto* e : xml.getChildIterator())
        {
            addNewTab (t);
            restoreTabState (t, t->getNumTabs() - 1, *e);
        }

        t->setCurrentTabIndex (xml.getIntAttribute ("initialTab", 0));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& doc,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, doc, props, multipleSelected);

        if (multipleSelected)
            return;

        if (auto* t = dynamic_cast<TabbedComponent*> (component))
        {
            props.add (new TabOrientationProperty (t, doc));
            props.add (new TabDepthProperty (t, doc));

            if (t->getNumTabs() > 0)
                props.add (new TabInitialTabProperty (t, doc));

            props.add (new TabAddTabProperty (t, doc));

            if (t->getNumTabs() > 0)
                props.add (new TabRemoveTabProperty (t, doc));
        }
    }

    void addPropertiesToPropertyPanel (Component* comp, JucerDocument& doc,
                                       PropertyPanel& panel, bool multipleSelected) override
    {
        ComponentTypeHandler::addPropertiesToPropertyPanel (comp, doc, panel, multipleSelected);

        TabbedComponent* const t = dynamic_cast<TabbedComponent*> (comp);

        for (int i = 0; i < t->getNumTabs(); ++i)
        {
            Array<PropertyComponent*> properties;

            properties.add (new TabNameProperty (t, doc, i));
            properties.add (new TabColourProperty (t, doc, i));

            properties.add (new TabContentTypeProperty (t, doc, i));

            if (isTabUsingJucerComp (t, i))
                properties.add (new TabJucerFileProperty (t, doc, i));
            else
                properties.add (new TabContentClassProperty (t, doc, i));

            properties.add (new TabContentConstructorParamsProperty (t, doc, i));

            properties.add (new TabMoveProperty (t, doc, i, t->getNumTabs()));

            panel.addSection ("Tab " + String (i), properties);
        }
    }

    String getCreationParameters (GeneratedCode&, Component* comp) override
    {
        TabbedComponent* const t = dynamic_cast<TabbedComponent*> (comp);

        switch (t->getOrientation())
        {
            case TabbedButtonBar::TabsAtTop:        return "juce::TabbedButtonBar::TabsAtTop";
            case TabbedButtonBar::TabsAtBottom:     return "juce::TabbedButtonBar::TabsAtBottom";
            case TabbedButtonBar::TabsAtLeft:       return "juce::TabbedButtonBar::TabsAtLeft";
            case TabbedButtonBar::TabsAtRight:      return "juce::TabbedButtonBar::TabsAtRight";
            default:                                jassertfalse; break;
        }

        return {};
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
    {
        TabbedComponent* const t = dynamic_cast<TabbedComponent*> (component);

        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        code.constructorCode
            << memberVariableName << "->setTabBarDepth (" << t->getTabBarDepth() << ");\n";

        for (int i = 0; i < t->getNumTabs(); ++i)
        {
            String contentClassName;

            if (isTabUsingJucerComp (t, i))
            {
                File jucerCpp = code.document->getCppFile().getSiblingFile (getTabJucerFile (t, i));

                std::unique_ptr<JucerDocument> doc (JucerDocument::createForCppFile (nullptr, jucerCpp));

                if (doc != nullptr)
                {
                    code.includeFilesCPP.add (jucerCpp.withFileExtension (".h"));
                    contentClassName = doc->getClassName();
                }
            }
            else
            {
                contentClassName = getTabClassName (t, i);
            }

            code.constructorCode
                << memberVariableName
                << "->addTab ("
                << quotedString (t->getTabNames() [i], code.shouldUseTransMacro())
                << ", "
                << CodeHelpers::colourToCode (t->getTabBackgroundColour (i));

            if (contentClassName.isNotEmpty())
            {
                code.constructorCode << ", new " << contentClassName;

                if (getTabConstructorParams (t, i).trim().isNotEmpty())
                    code.constructorCode << " ";

                code.constructorCode << "(" << getTabConstructorParams (t, i).trim() << "), true);\n";
            }
            else
            {
                code.constructorCode << ", 0, false);\n";
            }
        }

        code.constructorCode
            << memberVariableName << "->setCurrentTabIndex (" << t->getCurrentTabIndex() << ");\n";

        code.constructorCode << "\n";
    }

    //==============================================================================
    static void addNewTab (TabbedComponent* tc, const int insertIndex = -1)
    {
        tc->addTab ("Tab " + String (tc->getNumTabs()), Colours::lightgrey,
                    new TabDemoContentComp(), true, insertIndex);
    }

    //==============================================================================
    static std::unique_ptr<XmlElement> getTabState (TabbedComponent* tc, int tabIndex)
    {
        auto xml = std::make_unique<XmlElement> ("TAB");
        xml->setAttribute ("name", tc->getTabNames() [tabIndex]);
        xml->setAttribute ("colour", tc->getTabBackgroundColour (tabIndex).toString());

        if (TabDemoContentComp* const tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex)))
        {
            xml->setAttribute ("useJucerComp", tdc->isUsingJucerComp);
            xml->setAttribute ("contentClassName", tdc->contentClassName);
            xml->setAttribute ("constructorParams", tdc->constructorParams);
            xml->setAttribute ("jucerComponentFile", tdc->jucerComponentFile);
        }

        return xml;
    }

    static void restoreTabState (TabbedComponent* tc, int tabIndex, const XmlElement& xml)
    {
        tc->setTabName (tabIndex, xml.getStringAttribute ("name", "Tab"));
        tc->setTabBackgroundColour (tabIndex, Colour::fromString (xml.getStringAttribute ("colour", Colours::lightgrey.toString())));

        if (TabDemoContentComp* const tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex)))
        {
            tdc->isUsingJucerComp   = xml.getBoolAttribute ("useJucerComp", false);
            tdc->contentClassName   = xml.getStringAttribute ("contentClassName");
            tdc->constructorParams  = xml.getStringAttribute ("constructorParams");
            tdc->jucerComponentFile = xml.getStringAttribute ("jucerComponentFile");

            tdc->updateContent();
        }
    }

    //==============================================================================
    static bool isTabUsingJucerComp (TabbedComponent* tc, int tabIndex)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        return tdc != nullptr && tdc->isUsingJucerComp;
    }

    static void setTabUsingJucerComp (TabbedComponent* tc, int tabIndex, const bool b)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        if (tdc != nullptr)
        {
            tdc->isUsingJucerComp = b;
            tdc->updateContent();
        }
    }

    static String getTabClassName (TabbedComponent* tc, int tabIndex)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        return tdc != nullptr ? tdc->contentClassName : String();
    }

    static void setTabClassName (TabbedComponent* tc, int tabIndex, const String& newName)
    {
        TabDemoContentComp* const tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        if (tdc != nullptr)
        {
            tdc->contentClassName = newName;
            tdc->updateContent();
        }
    }

    static String getTabConstructorParams (TabbedComponent* tc, int tabIndex)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        return tdc != nullptr ? tdc->constructorParams : String();
    }

    static void setTabConstructorParams (TabbedComponent* tc, int tabIndex, const String& newParams)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        if (tdc != nullptr)
        {
            tdc->constructorParams = newParams;
            tdc->updateContent();
        }
    }

    static String getTabJucerFile (TabbedComponent* tc, int tabIndex)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        return tdc != nullptr ? tdc->jucerComponentFile : String();
    }

    static void setTabJucerFile (TabbedComponent* tc, int tabIndex, const String& newFile)
    {
        auto tdc = dynamic_cast<TabDemoContentComp*> (tc->getTabContentComponent (tabIndex));
        jassert (tdc != nullptr);

        if (tdc != nullptr)
        {
            tdc->jucerComponentFile = newFile;
            tdc->updateContent();
        }
    }

private:
    //==============================================================================
    class TabDemoContentComp : public Component
    {
    public:
        TabDemoContentComp()
            : isUsingJucerComp (false)
        {
            setSize (2048, 2048);
        }

        void paint (Graphics& g) override
        {
            if (jucerComp == nullptr)
                g.fillCheckerBoard (getLocalBounds().toFloat(), 50.0f, 50.0f,
                                    Colour::greyLevel (0.9f).withAlpha (0.4f),
                                    Colour::greyLevel (0.8f).withAlpha (0.4f));
        }

        void resized() override
        {
            if (jucerComp != nullptr)
            {
                jucerComp->setBounds (getLocalBounds());
                setOpaque (jucerComp->isOpaque());
            }
        }

        void updateContent()
        {
            if (isUsingJucerComp)
            {
                if (jucerComp == nullptr
                    || jucerComp->getOwnerDocument() == nullptr
                    || jucerComp->getFilename() != jucerComponentFile)
                {
                    jucerComp.reset();

                    jucerComp.reset (new TestComponent (ComponentTypeHandler::findParentDocument (this), nullptr, false));
                    jucerComp->setFilename (jucerComponentFile);
                    jucerComp->setToInitialSize();

                    addAndMakeVisible (jucerComp.get());
                }
            }
            else
            {
                jucerComp.reset();
            }

            resized();
        }

        void parentHierarchyChanged() override
        {
            updateContent();
        }

        bool isUsingJucerComp;
        String contentClassName, constructorParams;
        String jucerComponentFile;
        std::unique_ptr<TestComponent> jucerComp;
    };

    //==============================================================================
    class TabOrientationProperty  : public ComponentChoiceProperty<TabbedComponent>
    {
    public:
        TabOrientationProperty (TabbedComponent* comp, JucerDocument& doc)
            : ComponentChoiceProperty<TabbedComponent> ("tab position", comp, doc)
        {
            choices.add ("Tabs at top");
            choices.add ("Tabs at bottom");
            choices.add ("Tabs at left");
            choices.add ("Tabs at right");
        }

        void setIndex (int newIndex)
        {
            const TabbedButtonBar::Orientation orientations[] = { TabbedButtonBar::TabsAtTop,
                                                                  TabbedButtonBar::TabsAtBottom,
                                                                  TabbedButtonBar::TabsAtLeft,
                                                                  TabbedButtonBar::TabsAtRight };

            document.perform (new TabOrienationChangeAction (component, *document.getComponentLayout(), orientations [newIndex]),
                              "Change TabComponent orientation");
        }

        int getIndex() const
        {
            switch (component->getOrientation())
            {
                case TabbedButtonBar::TabsAtTop:    return 0;
                case TabbedButtonBar::TabsAtBottom: return 1;
                case TabbedButtonBar::TabsAtLeft:   return 2;
                case TabbedButtonBar::TabsAtRight:  return 3;
                default:                            jassertfalse; break;
            }

            return 0;
        }

    private:
        class TabOrienationChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabOrienationChangeAction (TabbedComponent* const comp, ComponentLayout& l, const TabbedButtonBar::Orientation newState_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getOrientation();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setOrientation (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setOrientation (oldState);
                changed();
                return true;
            }

            TabbedButtonBar::Orientation newState, oldState;
        };
    };

    //==============================================================================
    class TabInitialTabProperty  : public ComponentChoiceProperty<TabbedComponent>
    {
    public:
        TabInitialTabProperty (TabbedComponent* comp, JucerDocument& doc)
            : ComponentChoiceProperty<TabbedComponent> ("initial tab", comp, doc)
        {
            for (int i = 0; i < comp->getNumTabs(); ++i)
                choices.add ("Tab " + String (i) + ": \"" + comp->getTabNames() [i] + "\"");
        }

        void setIndex (int newIndex)
        {
            document.perform (new InitialTabChangeAction (component, *document.getComponentLayout(), newIndex),
                              "Change initial tab");
        }

        int getIndex() const
        {
            return component->getCurrentTabIndex();
        }

    private:
        class InitialTabChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            InitialTabChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  newValue (newValue_)
            {
                oldValue = comp->getCurrentTabIndex();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setCurrentTabIndex (newValue);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setCurrentTabIndex (oldValue);
                changed();
                return true;
            }

        private:
            int newValue, oldValue;
        };
    };

    //==============================================================================
    class TabDepthProperty  : public SliderPropertyComponent,
                              private ChangeListener
    {
    public:
        TabDepthProperty (TabbedComponent* comp, JucerDocument& doc)
            : SliderPropertyComponent ("tab depth", 10.0, 80.0, 1.0, 1.0),
              component (comp),
              document (doc)
        {
            document.addChangeListener (this);
        }

        ~TabDepthProperty() override
        {
            document.removeChangeListener (this);
        }

        void setValue (double newValue) override
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new TabDepthChangeAction (component, *document.getComponentLayout(), roundToInt (newValue)),
                              "Change TabComponent tab depth");
        }

        double getValue() const override
        {
            return component->getTabBarDepth();
        }

        TabbedComponent* const component;
        JucerDocument& document;

    private:
        void changeListenerCallback (ChangeBroadcaster*) override
        {
            refresh();
        }

        class TabDepthChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabDepthChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int newState_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  newState (newState_)
            {
                oldState = comp->getTabBarDepth();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTabBarDepth (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTabBarDepth (oldState);
                changed();
                return true;
            }

            int newState, oldState;
        };
    };

    //==============================================================================
    class TabAddTabProperty  : public ButtonPropertyComponent
    {
    public:
        TabAddTabProperty (TabbedComponent* comp, JucerDocument& doc)
            : ButtonPropertyComponent ("add tab", false),
              component (comp),
              document (doc)
        {
        }

        void buttonClicked()
        {
            document.perform (new AddTabAction (component, *document.getComponentLayout()),
                              "Add a new tab");
        }

        String getButtonText() const
        {
            return "Create a new tab";
        }

        TabbedComponent* const component;
        JucerDocument& document;

    private:
        class AddTabAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            AddTabAction (TabbedComponent* const comp, ComponentLayout& l)
                : ComponentUndoableAction<TabbedComponent> (comp, l)
            {
            }

            bool perform()
            {
                showCorrectTab();
                addNewTab (getComponent());
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->removeTab (getComponent()->getNumTabs() - 1);
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }
        };
    };

    //==============================================================================
    class TabRemoveTabProperty  : public ButtonPropertyComponent
    {
    public:
        TabRemoveTabProperty (TabbedComponent* comp, JucerDocument& doc)
            : ButtonPropertyComponent ("remove tab", true),
              component (comp),
              document (doc)
        {
        }

        void buttonClicked()
        {
            const StringArray names (component->getTabNames());

            PopupMenu m;
            for (int i = 0; i < component->getNumTabs(); ++i)
                m.addItem (i + 1, "Delete tab " + String (i)
                                    + ": \"" + names[i] + "\"");

            PopupMenu::Options options{};
            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this), [this] (int r)
            {
                if (r > 0)
                    document.perform (new RemoveTabAction (component, *document.getComponentLayout(), r - 1),
                                      "Remove a tab");
            });
        }

        String getButtonText() const
        {
            return "Delete a tab...";
        }

        TabbedComponent* const component;
        JucerDocument& document;

    private:
        class RemoveTabAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            RemoveTabAction (TabbedComponent* const comp, ComponentLayout& l, int indexToRemove_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  indexToRemove (indexToRemove_)
            {
                previousState = getTabState (comp, indexToRemove);
            }

            bool perform()
            {
                showCorrectTab();

                getComponent()->removeTab (indexToRemove);
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                addNewTab (getComponent(), indexToRemove);
                restoreTabState (getComponent(), indexToRemove, *previousState);
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }

        private:
            int indexToRemove;
            std::unique_ptr<XmlElement> previousState;
        };
    };

    //==============================================================================
    class TabNameProperty  : public ComponentTextProperty<TabbedComponent>
    {
    public:
        TabNameProperty (TabbedComponent* comp, JucerDocument& doc, const int tabIndex_)
            : ComponentTextProperty<TabbedComponent> ("name", 200, false, comp, doc),
              tabIndex (tabIndex_)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new TabNameChangeAction (component, *document.getComponentLayout(), tabIndex, newText),
                              "Change tab name");
        }

        String getText() const override
        {
            return component->getTabNames() [tabIndex];
        }

    private:
        int tabIndex;

        class TabNameChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabNameChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int tabIndex_, const String& newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newValue (newValue_)
            {
                oldValue = comp->getTabNames() [tabIndex];
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTabName (tabIndex, newValue);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTabName (tabIndex, oldValue);
                changed();
                return true;
            }

        private:
            const int tabIndex;
            String newValue, oldValue;
        };
    };

    //==============================================================================
    class TabColourProperty  : public JucerColourPropertyComponent,
                               private ChangeListener
    {
    public:
        TabColourProperty (TabbedComponent* comp, JucerDocument& doc, const int tabIndex_)
            : JucerColourPropertyComponent ("colour", false),
              component (comp),
              document (doc),
              tabIndex (tabIndex_)
        {
            document.addChangeListener (this);
        }

        ~TabColourProperty() override
        {
            document.removeChangeListener (this);
        }

        void setColour (Colour newColour) override
        {
            document.getUndoManager().undoCurrentTransactionOnly();

            document.perform (new TabColourChangeAction (component, *document.getComponentLayout(), tabIndex, newColour),
                              "Change tab colour");
        }

        Colour getColour() const override
        {
            return component->getTabBackgroundColour (tabIndex);
        }

        void resetToDefault() override
        {
            jassertfalse; // shouldn't get called
        }

        void changeListenerCallback (ChangeBroadcaster*) override     { refresh(); }

    private:
        TabbedComponent* component;
        JucerDocument& document;
        int tabIndex;

        class TabColourChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabColourChangeAction (TabbedComponent* comp, ComponentLayout& l,
                                   int tabIndex_, Colour newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newValue (newValue_)
            {
                oldValue = comp->getTabBackgroundColour (tabIndex);
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setTabBackgroundColour (tabIndex, newValue);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setTabBackgroundColour (tabIndex, oldValue);
                changed();
                return true;
            }

        private:
            const int tabIndex;
            Colour newValue, oldValue;
        };
    };

    //==============================================================================
    class TabContentTypeProperty  : public ComponentChoiceProperty<TabbedComponent>
    {
    public:
        TabContentTypeProperty (TabbedComponent* comp, JucerDocument& doc, const int tabIndex_)
            : ComponentChoiceProperty<TabbedComponent> ("content type", comp, doc),
              tabIndex (tabIndex_)
        {
            choices.add ("Jucer content component");
            choices.add ("Named content component");
        }

        void setIndex (int newIndex)
        {
            document.perform (new TabContentTypeChangeAction (component, *document.getComponentLayout(), tabIndex, newIndex == 0),
                              "Change tab content type");
        }

        int getIndex() const
        {
            return isTabUsingJucerComp (component, tabIndex) ? 0 : 1;
        }

    private:
        int tabIndex;

        class TabContentTypeChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabContentTypeChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int tabIndex_, const bool newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newValue (newValue_)
            {
                oldValue = isTabUsingJucerComp (comp, tabIndex);
            }

            bool perform()
            {
                showCorrectTab();
                setTabUsingJucerComp (getComponent(), tabIndex, newValue);
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setTabUsingJucerComp (getComponent(), tabIndex, oldValue);
                layout.getDocument()->refreshAllPropertyComps();
                changed();
                return true;
            }

        private:
            int tabIndex;
            bool newValue, oldValue;
        };
    };

    //==============================================================================
    class TabJucerFileProperty   : public FilePropertyComponent,
                                   private ChangeListener
    {
    public:
        TabJucerFileProperty (TabbedComponent* const comp, JucerDocument& doc, const int tabIndex_)
            : FilePropertyComponent ("jucer file", false, true),
              component (comp),
              document (doc),
              tabIndex (tabIndex_)
        {
            document.addChangeListener (this);
        }

        ~TabJucerFileProperty() override
        {
            document.removeChangeListener (this);
        }

        //==============================================================================
        void setFile (const File& newFile) override
        {
            document.perform (new JucerCompFileChangeAction (component, *document.getComponentLayout(), tabIndex,
                                                             newFile.getRelativePathFrom (document.getCppFile().getParentDirectory())
                                                                    .replaceCharacter ('\\', '/')),
                              "Change tab component file");
        }

        File getFile() const override
        {
            return document.getCppFile().getSiblingFile (getTabJucerFile (component, tabIndex));
        }

    private:
        void changeListenerCallback (ChangeBroadcaster*) override { refresh(); }

        TabbedComponent* const component;
        JucerDocument& document;
        int tabIndex;

        class JucerCompFileChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            JucerCompFileChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int tabIndex_, const String& newState_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newState (newState_)
            {
                oldState = getTabJucerFile (comp, tabIndex);
            }

            bool perform()
            {
                showCorrectTab();
                setTabJucerFile (getComponent(), tabIndex, newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setTabJucerFile (getComponent(), tabIndex, oldState);
                changed();
                return true;
            }

            int tabIndex;
            String newState, oldState;
        };
    };

    //==============================================================================
    class TabContentClassProperty   : public ComponentTextProperty<TabbedComponent>
    {
    public:
        TabContentClassProperty (TabbedComponent* comp, JucerDocument& doc, const int tabIndex_)
            : ComponentTextProperty<TabbedComponent> ("content class", 256, false, comp, doc),
              tabIndex (tabIndex_)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new TabClassNameChangeAction (component, *document.getComponentLayout(), tabIndex, newText),
                              "Change TabbedComponent content class");
        }

        String getText() const override
        {
            return getTabClassName (component, tabIndex);
        }

    private:
        int tabIndex;

        class TabClassNameChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabClassNameChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int tabIndex_, const String& newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newValue (newValue_)
            {
                oldValue = getTabClassName (comp, tabIndex);
            }

            bool perform()
            {
                showCorrectTab();
                setTabClassName (getComponent(), tabIndex, newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setTabClassName (getComponent(), tabIndex, oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            int tabIndex;
            String newValue, oldValue;
        };
    };

    //==============================================================================
    class TabContentConstructorParamsProperty   : public ComponentTextProperty<TabbedComponent>
    {
    public:
        TabContentConstructorParamsProperty (TabbedComponent* comp, JucerDocument& doc, const int tabIndex_)
            : ComponentTextProperty<TabbedComponent> ("constructor params", 512, false, comp, doc),
              tabIndex (tabIndex_)
        {
        }

        void setText (const String& newText) override
        {
            document.perform (new TabConstructorParamChangeAction (component, *document.getComponentLayout(), tabIndex, newText),
                              "Change TabbedComponent content constructor param");
        }

        String getText() const override
        {
            return getTabConstructorParams (component, tabIndex);
        }

    private:
        int tabIndex;

        class TabConstructorParamChangeAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            TabConstructorParamChangeAction (TabbedComponent* const comp, ComponentLayout& l, const int tabIndex_, const String& newValue_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  tabIndex (tabIndex_),
                  newValue (newValue_)
            {
                oldValue = getTabConstructorParams (comp, tabIndex);
            }

            bool perform()
            {
                showCorrectTab();
                setTabConstructorParams (getComponent(), tabIndex, newValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                setTabConstructorParams (getComponent(), tabIndex, oldValue);
                changed();
                layout.getDocument()->refreshAllPropertyComps();
                return true;
            }

            int tabIndex;
            String newValue, oldValue;
        };
    };

    //==============================================================================
    class TabMoveProperty   : public ButtonPropertyComponent
    {
    public:
        TabMoveProperty (TabbedComponent* comp, JucerDocument& doc,
                         const int tabIndex_, const int totalNumTabs_)
            : ButtonPropertyComponent ("move tab", false),
              component (comp),
              document (doc),
              tabIndex (tabIndex_),
              totalNumTabs (totalNumTabs_)
        {

        }

        void buttonClicked()
        {
            PopupMenu m;
            m.addItem (1, "Move this tab up", tabIndex > 0);
            m.addItem (2, "Move this tab down", tabIndex < totalNumTabs - 1);

            PopupMenu::Options options{};
            m.showMenuAsync (PopupMenu::Options().withTargetComponent (this), [this] (int r)
            {
                if (r != 0)
                    document.perform (new MoveTabAction (component, *document.getComponentLayout(), tabIndex, tabIndex + (r == 2 ? 1 : -1)),
                                      "Move a tab");
            });
        }

        String getButtonText() const
        {
            return "Move this tab...";
        }

        TabbedComponent* const component;
        JucerDocument& document;
        const int tabIndex, totalNumTabs;

    private:
        class MoveTabAction  : public ComponentUndoableAction<TabbedComponent>
        {
        public:
            MoveTabAction (TabbedComponent* const comp, ComponentLayout& l,
                           const int oldIndex_, const int newIndex_)
                : ComponentUndoableAction<TabbedComponent> (comp, l),
                  oldIndex (oldIndex_),
                  newIndex (newIndex_)
            {
            }

            void move (int from, int to)
            {
                showCorrectTab();

                auto state = getTabState (getComponent(), from);

                getComponent()->removeTab (from);
                addNewTab (getComponent(), to);

                restoreTabState (getComponent(), to, *state);

                layout.getDocument()->refreshAllPropertyComps();
                changed();
            }

            bool perform()
            {
                move (oldIndex, newIndex);
                return true;
            }

            bool undo()
            {
                move (newIndex, oldIndex);
                return true;
            }

        private:
            const int oldIndex, newIndex;
        };
    };
};
