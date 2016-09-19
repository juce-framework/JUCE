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

class ComponentListComp   : public TreePanelBase,
                            private ActivityList::Listener
{
public:
    ComponentListComp (CompileEngineChildProcess& c)
        : TreePanelBase (&c.project, "compClassTreeState"),
          owner (c)
    {
        setName ("Components");

        tree.setRootItemVisible (false);
        tree.setMultiSelectEnabled (false);
        tree.setDefaultOpenness (true);
        setRoot (new NamespaceItem (&owner.getComponentList().globalNamespace));

        classListChanged (owner.getComponentList());

        owner.activityList.addListener (this);
    }

    ~ComponentListComp()
    {
        saveOpenness();
        owner.activityList.removeListener (this);
    }

    void classListChanged (const ClassDatabase::ClassList& newClasses) override
    {
        static_cast<NamespaceItem*> (rootItem.get())->setNamespace (&newClasses.globalNamespace);
    }

    void openPreview (const ClassDatabase::Class& comp)
    {
        owner.openPreview (comp);
    }

    void showClassDeclaration (const ClassDatabase::Class& comp)
    {
        owner.handleHighlightCode (comp.getClassDeclarationRange());
    }

private:
    CompileEngineChildProcess& owner;
    struct ClassItem;

    struct NamespaceItem  : public JucerTreeViewBase
    {
        NamespaceItem (const ClassDatabase::Namespace* n)
        {
            setNamespace (n);
        }

        void setNamespace (const ClassDatabase::Namespace* newNamespace)
        {
            namespaceToShow = newNamespace;
            uniqueID = namespaceToShow != nullptr ? "ns_" + namespaceToShow->fullName : "null";
            refreshSubItems();
        }

        String getRenamingName() const override     { return getDisplayName(); }
        String getDisplayName() const override      { return (namespaceToShow != nullptr ? namespaceToShow->name : String()) + "::"; }
        void setName (const String&) override       {}
        bool isMissing() override                   { return false; }
        Icon getIcon() const override               { return Icon (getIcons().graph, getContrastingColour (Colours::darkred, 0.5f)); }
        bool canBeSelected() const override         { return true; }
        bool mightContainSubItems() override        { return namespaceToShow != nullptr && ! namespaceToShow->isEmpty(); }
        String getUniqueName() const override       { return uniqueID; }

        void addSubItems() override
        {
            if (namespaceToShow != nullptr)
            {
                Array<ClassItem*> newComps;
                Array<NamespaceItem*> newNamespaces;

                for (const auto& c : namespaceToShow->components)
                    newComps.addSorted (*this, new ClassItem (c, *namespaceToShow));

                for(const auto& n : namespaceToShow->namespaces)
                {
                    if (n.getTotalClassesAndNamespaces() < 10)
                        createFlatItemList (n, newComps);
                    else
                        newNamespaces.add (new NamespaceItem (&n));
                }

                for (auto c : newComps)
                    addSubItem (c);

                for (auto n : newNamespaces)
                    addSubItem (n);
            }
        }

        void createFlatItemList (const ClassDatabase::Namespace& ns, Array<ClassItem*>& newComps)
        {
            for (const auto& c : ns.components)
                newComps.addSorted (*this, new ClassItem (c, *namespaceToShow));

            for (const auto& n : ns.namespaces)
                createFlatItemList (n, newComps);
        }

        static int compareElements (ClassItem* c1, ClassItem* c2)
        {
            return c1->comp.getName().compareIgnoreCase (c2->comp.getName());
        }

    private:
        const ClassDatabase::Namespace* namespaceToShow = nullptr;
        String uniqueID; // must be stored rather than calculated, in case the namespace obj is dangling
    };

    struct ClassItem  : public JucerTreeViewBase
    {
        ClassItem (const ClassDatabase::Class& c, const ClassDatabase::Namespace& parentNS)
            : comp (c),
              displayName (comp.getName().substring (parentNS.fullName.length()))
        {
        }

        String getRenamingName() const override     { return getDisplayName(); }
        String getDisplayName() const override      { return displayName; }
        void setName (const String&) override       {}
        bool isMissing() override                   { return false; }
        Icon getIcon() const override               { return Icon (getIcons().box, getTextColour()); }
        bool canBeSelected() const override         { return true; }
        bool mightContainSubItems() override        { return false; }
        String getUniqueName() const override       { return comp.getName(); }

        bool canBeLaunched() const
        {
            return comp.getInstantiationFlags().canBeInstantiated();
        }

        void showClassDeclaration() const
        {
            if (ComponentListComp* clc = getOwnerView()->findParentComponentOfClass<ComponentListComp>())
                clc->showClassDeclaration (comp);
        }

        void launchEditor() const
        {
            if (ComponentListComp* clc = getOwnerView()->findParentComponentOfClass<ComponentListComp>())
                clc->openPreview (comp);
        }

        void itemClicked (const MouseEvent&) override
        {
            if (! canBeLaunched())
                if (ProjectContentComponent* const pcc = getOwnerView()->findParentComponentOfClass<ProjectContentComponent>())
                    pcc->showBubbleMessage (pcc->getLocalArea (getOwnerView(), getItemPosition (true)),
                                            "Cannot create a live view:\n" + comp.getInstantiationFlags().getReasonForUnavailability());
        }

        void itemDoubleClicked (const MouseEvent&) override
        {
            if (canBeLaunched())
                launchEditor();
            else
                showClassDeclaration();
        }

        void paintContent (Graphics& g, const Rectangle<int>& area) override
        {
            g.setFont (getFont());
            g.setColour (getTextColour());

            g.drawFittedText (getDisplayName(),
                              area.withWidth (area.getWidth() - 40), // to account for buttons
                              Justification::centredLeft, 1, 0.8f);
        }

        Colour getTextColour() const
        {
            return getContrastingColour (comp.getInstantiationFlags().canBeInstantiated() ? 0.8f : 0.3f);
        }

        Component* createItemComponent() override
        {
            Component* c = JucerTreeViewBase::createItemComponent();
            jassert (dynamic_cast<TreeItemComponent*> (c) != nullptr);

            if (canBeLaunched())
                static_cast<TreeItemComponent*> (c)->addRightHandButton (new ClassItemButton (*this, false));

            static_cast<TreeItemComponent*> (c)->addRightHandButton (new ClassItemButton (*this, true));

            return c;
        }

        struct ClassItemButton  : public Button
        {
            ClassItemButton (const ClassItem& c, bool isShowCodeButton)
               : Button (String()), classItem (c), isShowCode (isShowCodeButton)
            {
                setMouseCursor (MouseCursor::PointingHandCursor);
            }

            void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
            {
                const Path& path = isShowCode ? getIcons().code
                                              : getIcons().play;

                Colour col (classItem.getBackgroundColour().contrasting (isShowCode ? Colours::white
                                                                                    : Colours::lightgreen, 0.6f));

                Icon (path, col.withAlpha (isButtonDown ? 1.0f : (isMouseOverButton ? 0.8f : 0.5f)))
                   .draw (g, getLocalBounds().reduced (getHeight() / 5).toFloat(), false);
            }

            void clicked() override
            {
                if (isShowCode)
                    classItem.showClassDeclaration();
                else
                    classItem.launchEditor();
            }

            const ClassItem& classItem;
            bool isShowCode;
        };

        const ClassDatabase::Class comp;
        String displayName;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentListComp)
};
