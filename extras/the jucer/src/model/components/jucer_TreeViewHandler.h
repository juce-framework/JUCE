/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCER_TREEVIEWHANDLER_JUCEHEADER__
#define __JUCER_TREEVIEWHANDLER_JUCEHEADER__


//==============================================================================
/**
*/
class TreeViewHandler  : public ComponentTypeHandler
{
public:
    //==============================================================================
    TreeViewHandler()
        : ComponentTypeHandler ("TreeView", "TreeView", typeid (DemoTreeView), 150, 150)
    {
        registerColour (TreeView::backgroundColourId, "background", "backgroundColour");
        registerColour (TreeView::linesColourId, "lines", "linecol");
    }

    //==============================================================================
    Component* createNewComponent (JucerDocument*)
    {
        return new DemoTreeView();
    }

    XmlElement* createXmlFor (Component* comp, const ComponentLayout* layout)
    {
        TreeView* const t = dynamic_cast <TreeView*> (comp);
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
        TreeView* const t = dynamic_cast <TreeView*> (comp);

        t->setRootItemVisible (xml.getBoolAttribute ("rootVisible", defaultTreeView.isRootItemVisible()));
        t->setDefaultOpenness (xml.getBoolAttribute ("openByDefault", defaultTreeView.areItemsOpenByDefault()));

        return true;
    }

    void getEditableProperties (Component* component, JucerDocument& document, Array <PropertyComponent*>& properties)
    {
        ComponentTypeHandler::getEditableProperties (component, document, properties);
        TreeView* const t = dynamic_cast <TreeView*> (component);

        properties.add (new TreeViewRootItemProperty (t, document));
        properties.add (new TreeViewRootOpennessProperty (t, document));

        addColourProperties (t, document, properties);
    }

    const String getCreationParameters (Component* comp)
    {
        return quotedString (comp->getName());
    }

    void fillInCreationCode (GeneratedCode& code, Component* component, const String& memberVariableName)
    {
        TreeView defaultTreeView;
        TreeView* const t = dynamic_cast <TreeView*> (component);

        ComponentTypeHandler::fillInCreationCode (code, component, memberVariableName);

        if (defaultTreeView.isRootItemVisible() != t->isRootItemVisible())
        {
            code.constructorCode
                << memberVariableName << "->setRootItemVisible ("
                << boolToString (t->isRootItemVisible()) << ");\n";
        }

        if (defaultTreeView.areItemsOpenByDefault() != t->areItemsOpenByDefault())
        {
            code.constructorCode
                << memberVariableName << "->setDefaultOpenness ("
                << boolToString (t->areItemsOpenByDefault()) << ");\n";
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
            TreeViewItem* root = getRootItem();
            setRootItem (0);
            delete root;
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

            ~DemoTreeViewItem()
            {
            }

            void paintItem (Graphics& g, int width, int height)
            {
                if (isSelected())
                    g.fillAll (Colours::lightblue);

                g.setColour (Colours::black);
                g.setFont (height * 0.7f);
                g.drawText (name, 4, 0, width - 4, height, Justification::centredLeft, true);
            }

            bool mightContainSubItems()
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
        TreeViewRootItemProperty (TreeView* comp, JucerDocument& document)
            : ComponentBooleanProperty <TreeView> ("show root item", "Root item visible", "Root item visible", comp, document)
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
            TreeviewRootChangeAction (TreeView* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <TreeView> (comp, layout),
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
        TreeViewRootOpennessProperty (TreeView* comp, JucerDocument& document)
            : ComponentChoiceProperty <TreeView> ("default openness", comp, document)
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
            TreeviewOpennessChangeAction (TreeView* const comp, ComponentLayout& layout, const bool newState_)
                : ComponentUndoableAction <TreeView> (comp, layout),
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


#endif   // __JUCER_TREEVIEWHANDLER_JUCEHEADER__
