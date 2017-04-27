/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
        bool isMissing() const override             { return false; }
        Icon getIcon() const override               { return Icon (getIcons().graph, getContentColour (true)); }
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
        bool isMissing() const override             { return false; }
        Icon getIcon() const override               { return Icon (getIcons().box, getContentColour (true)); }
        bool canBeSelected() const override         { return true; }
        bool mightContainSubItems() override        { return false; }
        String getUniqueName() const override       { return comp.getName(); }
        int getRightHandButtonSpace() override      { return canBeLaunched() ? 60 : 40; }
        Component* createItemComponent() override
        {
            auto* content = new TreeItemComponent (*this);

            content->addRightHandButton (new ClassItemButton (*this, true));
            if (canBeLaunched())
                content->addRightHandButton (new ClassItemButton (*this, false));

            return content;
        }

        Colour getContentColour (bool isIcon) const override
        {
            auto alpha = comp.getInstantiationFlags().canBeInstantiated() ? 1.0f : 0.4f;
            auto& lf = ProjucerApplication::getApp().lookAndFeel;

            if (isSelected())
                return lf.findColour (defaultHighlightedTextColourId).withMultipliedAlpha (alpha);

            return lf.findColour (isIcon ? treeIconColourId : defaultTextColourId).withMultipliedAlpha (alpha);
        }

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

                auto colour = classItem.getContentColour (true).withAlpha (isButtonDown ? 1.0f
                                                                                        : (isMouseOverButton ? 0.8f
                                                                                                             : 0.5f));

                Icon (path, colour).draw (g, getLocalBounds().reduced (getHeight() / 5).toFloat(), false);
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

        struct ClassComponent    : public Component
        {
            ClassComponent (ClassItem& item, bool canBeLaunched)
            {
                addAndMakeVisible (buttons.add (new ClassItemButton (item, true)));

                if (canBeLaunched)
                    addAndMakeVisible (buttons.add (new ClassItemButton (item, false)));

                setInterceptsMouseClicks (false, true);
            }

            void resized() override
            {
                auto bounds = getLocalBounds();

                for (auto b : buttons)
                    b->setBounds (bounds.removeFromRight (25).reduced (2));
            }

            OwnedArray<ClassItemButton> buttons;
        };

        const ClassDatabase::Class comp;
        String displayName;
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ComponentListComp)
};
