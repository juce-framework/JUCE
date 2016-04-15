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

class TreeViewHandler  : public ComponentTypeHandler
{
public:
    TreeViewHandler()
        : ComponentTypeHandler ("TreeView", "TreeView", typeid (DemoTreeView), 150, 150)
    {
        registerColour (TreeView::backgroundColourId, "background", "backgroundColour");
        registerColour (TreeView::linesColourId, "lines", "linecol");
    }

    Component* createNewComponent (JucerDocument*)
    {
        return new DemoTreeView();
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        TreeView* const t = dynamic_cast<TreeView*> (comp);
        XmlElement* const e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute ("rootVisible", t->isRootItemVisible());
        e->setAttribute ("openByDefault", t->areItemsOpenByDefault());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout)
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        TreeView defaultTreeView;
        TreeView* const t = dynamic_cast<TreeView*> (comp);

        t->setRootItemVisible (xml.getBoolAttribute ("rootVisible", defaultTreeView.isRootItemVisible()));
        t->setDefaultOpenness (xml.getBoolAttribute ("openByDefault", defaultTreeView.areItemsOpenByDefault()));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array<PropertyComponent*>& props)
    {
        ComponentTypeHandler::getEditableProperties (component, document, props);
        TreeView* const t = dynamic_cast<TreeView*> (component);

        props.add (new TreeViewRootItemProperty (t, document));
        props.add (new TreeViewRootOpennessProperty (t, document));

        addColourProperties (t, document, props);
    }

    String getCreationParameters (GeneratedCode&, Component* comp)
    {
        return quotedString (comp->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        TreeView defaultTreeView;
        TreeView* const t = dynamic_cast<TreeView*> (component);

        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (defaultTreeView.isRootItemVisible() != t->isRootItemVisible())
        {
            code.constructorCode
                << memberVariableName << "->setRootItemVisible ("
                << CodeHelpers::boolLiteral (t->isRootItemVisible()) << ");\n";
        }

        if (defaultTreeView.areItemsOpenByDefault() != t->areItemsOpenByDefault())
        {
            code.constructorCode
                << memberVariableName << "->setDefaultOpenness ("
                << CodeHelpers::boolLiteral (t->areItemsOpenByDefault()) << ");\n";
        }

        code.constructorCode << getColourIntialisationCode (component, memberVariableName);
        code.constructorCode << "\n";
    }

private:
    //==============================================================================
    class DemoTreeView : public TreeView
    {
    public:
        DemoTreeView()
            : TreeView ("new treeview")
        {
            setRootItem (new DemoTreeViewItem ("Demo root node", 4));
        }

        ~DemoTreeView()
        {
            deleteRootItem();
        }

    private:
        class DemoTreeViewItem  : public TreeViewItem
        {
        public:
            DemoTreeViewItem (const String& name_, const int numItems)
                : name (name_)
            {
                for (int i = 0; i < numItems; ++i)
                    addSubItem (new DemoTreeViewItem ("Demo sub-node " + String (i), numItems - 1));
            }

            void paintItem (Graphics& g, int width, int height) override
            {
                if (isSelected())
                    g.fillAll (Colours::lightblue);

                g.setColour (Colours::black);
                g.setFont (height * 0.7f);
                g.drawText (name, 4, 0, width - 4, height, Justification::centredLeft, true);
            }

            bool mightContainSubItems() override
            {
                return true;
            }

            const String name;
        };
    };

    //==============================================================================
    class TreeViewRootItemProperty  : public ComponentBooleanProperty <TreeView>
    {
    public:
        TreeViewRootItemProperty (TreeView* comp, JucerDocument& doc)
            : ComponentBooleanProperty <TreeView> ("show root item", "Root item visible", "Root item visible", comp, doc)
        {
        }

        void setState (bool newState)
        {
            document.perform (new TreeviewRootChangeAction (component, *document.getComponentLayout(), newState),
                              "Change TreeView root item");
        }

        bool getState() const
        {
            return component->isRootItemVisible();
        }

    private:
        class TreeviewRootChangeAction  : public ComponentUndoableAction <TreeView>
        {
        public:
            TreeviewRootChangeAction (TreeView* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TreeView> (comp, l),
                  newState (newState_)
            {
                oldState = comp->isRootItemVisible();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setRootItemVisible (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setRootItemVisible (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };

    //==============================================================================
    class TreeViewRootOpennessProperty  : public ComponentChoiceProperty <TreeView>
    {
    public:
        TreeViewRootOpennessProperty (TreeView* comp, JucerDocument& doc)
            : ComponentChoiceProperty <TreeView> ("default openness", comp, doc)
        {
            choices.add ("Items open by default");
            choices.add ("Items closed by default");
        }

        void setIndex (int newIndex)
        {
            document.perform (new TreeviewOpennessChangeAction (component, *document.getComponentLayout(), newIndex == 0),
                              "Change TreeView openness");
        }

        int getIndex() const
        {
            return component->areItemsOpenByDefault() ? 0 : 1;
        }

    private:
        class TreeviewOpennessChangeAction  : public ComponentUndoableAction <TreeView>
        {
        public:
            TreeviewOpennessChangeAction (TreeView* const comp, ComponentLayout& l, const bool newState_)
                : ComponentUndoableAction <TreeView> (comp, l),
                  newState (newState_)
            {
                oldState = comp->areItemsOpenByDefault();
            }

            bool perform()
            {
                showCorrectTab();
                getComponent()->setDefaultOpenness (newState);
                changed();
                return true;
            }

            bool undo()
            {
                showCorrectTab();
                getComponent()->setDefaultOpenness (oldState);
                changed();
                return true;
            }

            bool newState, oldState;
        };
    };
};
