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
class TreeViewHandler  : public ComponentTypeHandler
{
public:
    TreeViewHandler()
        : ComponentTypeHandler ("TreeView", "juce::TreeView", typeid (DemoTreeView), 150, 150)
    {
        registerColour (juce::TreeView::backgroundColourId, "background", "backgroundColour");
        registerColour (juce::TreeView::linesColourId, "lines", "linecol");
    }

    Component* createNewComponent (JucerDocument*) override
    {
        return new DemoTreeView();
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout) override
    {
        TreeView* const t = dynamic_cast<TreeView*> (comp);
        XmlElement* const e = ComponentTypeHandler::createXmlFor (comp, layout);

        e->setAttribute ("rootVisible", t->isRootItemVisible());
        e->setAttribute ("openByDefault", t->areItemsOpenByDefault());

        return e;
    }

    bool restoreFromXml (const XmlElement& xml, Component* comp, const ComponentLayout* layout) override
    {
        if (! ComponentTypeHandler::restoreFromXml (xml, comp, layout))
            return false;

        TreeView defaultTreeView;
        TreeView* const t = dynamic_cast<TreeView*> (comp);

        t->setRootItemVisible (xml.getBoolAttribute ("rootVisible", defaultTreeView.isRootItemVisible()));
        t->setDefaultOpenness (xml.getBoolAttribute ("openByDefault", defaultTreeView.areItemsOpenByDefault()));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document,
                                Array<PropertyComponent*>& props, bool multipleSelected) override
    {
        ComponentTypeHandler::getEditableProperties (component, document, props, multipleSelected);

        if (multipleSelected)
            return;

        auto* t = dynamic_cast<TreeView*> (component);

        props.add (new TreeViewRootItemProperty (t, document));
        props.add (new TreeViewRootOpennessProperty (t, document));

        addColourProperties (t, document, props);
    }

    String getCreationParameters (GeneratedCode&, Component* comp) override
    {
        return quotedString (comp->getName(), false);
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName) override
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
                g.setFont ((float) height * 0.7f);
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
