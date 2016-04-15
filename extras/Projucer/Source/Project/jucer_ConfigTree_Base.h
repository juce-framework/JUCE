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

class PropertyGroupComponent  : public Component
{
public:
    PropertyGroupComponent()  {}

    void setProperties (const PropertyListBuilder& newProps)
    {
        properties.clear();
        properties.addArray (newProps.components);

        for (int i = properties.size(); --i >= 0;)
            addAndMakeVisible (properties.getUnchecked(i));
    }

    int updateSize (int x, int y, int width)
    {
        int height = 38;

        for (int i = 0; i < properties.size(); ++i)
        {
            PropertyComponent* pp = properties.getUnchecked(i);
            pp->setBounds (10, height, width - 20, pp->getPreferredHeight());
            height += pp->getHeight();
        }

        height += 16;
        setBounds (x, y, width, height);
        return height;
    }

    void paint (Graphics& g) override
    {
        const Colour bkg (findColour (mainBackgroundColourId));

        g.setColour (Colours::white.withAlpha (0.35f));
        g.fillRect (0, 30, getWidth(), getHeight() - 38);

        g.setFont (Font (15.0f, Font::bold));
        g.setColour (bkg.contrasting (0.7f));
        g.drawFittedText (getName(), 12, 0, getWidth() - 16, 25, Justification::bottomLeft, 1);
    }

    OwnedArray<PropertyComponent> properties;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyGroupComponent)
};

//==============================================================================
class RolloverHelpComp   : public Component,
                           private Timer
{
public:
    RolloverHelpComp()  : lastComp (nullptr)
    {
        setInterceptsMouseClicks (false, false);
        startTimer (150);
    }

    void paint (Graphics& g) override
    {
        AttributedString s;
        s.setJustification (Justification::centredLeft);
        s.append (lastTip, Font (14.0f), findColour (mainBackgroundColourId).contrasting (0.7f));

        TextLayout tl;
        tl.createLayoutWithBalancedLineLengths (s, getWidth() - 10.0f);
        if (tl.getNumLines() > 3)
            tl.createLayout (s, getWidth() - 10.0f);

        tl.draw (g, getLocalBounds().toFloat());
    }

private:
    Component* lastComp;
    String lastTip;

    void timerCallback() override
    {
        Component* newComp = Desktop::getInstance().getMainMouseSource().getComponentUnderMouse();

        if (newComp != nullptr
             && (newComp->getTopLevelComponent() != getTopLevelComponent()
                  || newComp->isCurrentlyBlockedByAnotherModalComponent()))
            newComp = nullptr;

        if (newComp != lastComp)
        {
            lastComp = newComp;

            String newTip (findTip (newComp));

            if (newTip != lastTip)
            {
                lastTip = newTip;
                repaint();
            }
        }
    }

    static String findTip (Component* c)
    {
        while (c != nullptr)
        {
            if (TooltipClient* tc = dynamic_cast<TooltipClient*> (c))
            {
                const String tip (tc->getTooltip());

                if (tip.isNotEmpty())
                    return tip;
            }

            c = c->getParentComponent();
        }

        return String();
    }
};


//==============================================================================
class ConfigTreeItemBase  : public JucerTreeViewBase,
                            public ValueTree::Listener
{
public:
    ConfigTreeItemBase() {}

    void showSettingsPage (Component* content)
    {
        content->setComponentID (getUniqueName());

        ScopedPointer<Component> comp (content);

        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->setEditorComponent (new PropertyPanelViewport (comp.release()), nullptr);
    }

    void closeSettingsPage()
    {
        if (ProjectContentComponent* pcc = getProjectContentComponent())
        {
            if (PropertyPanelViewport* ppv = dynamic_cast<PropertyPanelViewport*> (pcc->getEditorComponent()))
                if (ppv->viewport.getViewedComponent()->getComponentID() == getUniqueName())
                    pcc->hideEditor();
        }
    }

    void deleteAllSelectedItems() override
    {
        TreeView* const tree = getOwnerView();
        jassert (tree->getNumSelectedItems() <= 1); // multi-select should be disabled

        if (ConfigTreeItemBase* s = dynamic_cast<ConfigTreeItemBase*> (tree->getSelectedItem (0)))
            s->deleteItem();
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
           refreshSubItems();
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override {}
    void valueTreeChildAdded (ValueTree&, ValueTree&) override {}
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override {}
    void valueTreeChildOrderChanged (ValueTree&, int, int) override {}
    void valueTreeParentChanged (ValueTree&) override {}

    virtual bool isProjectSettings() const          { return false; }
    virtual bool isModulesList() const              { return false; }

    static void updateSize (Component& comp, PropertyGroupComponent& group)
    {
        const int width = jmax (550, comp.getParentWidth() - 20);

        int y = 0;
        y += group.updateSize (12, y, width - 12);

        comp.setSize (width, y);
    }

private:
    //==============================================================================
    struct PropertyPanelViewport  : public Component
    {
        PropertyPanelViewport (Component* content)
        {
            addAndMakeVisible (viewport);
            addAndMakeVisible (rolloverHelp);
            viewport.setViewedComponent (content, true);
        }

        void paint (Graphics& g) override
        {
            ProjucerLookAndFeel::fillWithBackgroundTexture (*this, g);
        }

        void resized() override
        {
            Rectangle<int> r (getLocalBounds());
            rolloverHelp.setBounds (r.removeFromBottom (70).reduced (10, 0));
            viewport.setBounds (r);
        }

        Viewport viewport;
        RolloverHelpComp rolloverHelp;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyPanelViewport)
    };
};

//==============================================================================
class RootItem   : public ConfigTreeItemBase
{
public:
    RootItem (Project& p)
        : project (p), exportersTree (p.getExporters())
    {
        exportersTree.addListener (this);
    }

    bool isRoot() const override              { return true; }
    bool isProjectSettings() const override   { return true; }
    String getRenamingName() const override   { return getDisplayName(); }
    String getDisplayName() const override    { return project.getTitle(); }
    void setName (const String&) override     {}
    bool isMissing() override                 { return false; }
    Icon getIcon() const override             { return project.getMainGroup().getIcon().withContrastingColourTo (getBackgroundColour()); }
    void showDocument() override              { showSettingsPage (new SettingsComp (project)); }
    bool canBeSelected() const override       { return true; }
    bool mightContainSubItems() override      { return project.getNumExporters() > 0; }
    String getUniqueName() const override     { return "config_root"; }

    void addSubItems() override
    {
        addSubItem (new EnabledModulesItem (project));
        ProjucerApplication::getApp().addLiveBuildConfigItem (project, *this);

        int i = 0;
        for (Project::ExporterIterator exporter (project); exporter.next(); ++i)
            addSubItem (new ExporterItem (project, exporter.exporter.release(), i));
    }

    void showPopupMenu() override
    {
        if (ProjectContentComponent* pcc = getProjectContentComponent())
            pcc->showNewExporterMenu();
    }

    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description.toString().startsWith (getUniqueName());
    }

    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override
    {
        int oldIndex = dragSourceDetails.description.toString().getTrailingIntValue();
        exportersTree.moveChild (oldIndex, jmax (0, insertIndex - 1), project.getUndoManagerFor (exportersTree));
    }

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { refreshIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { refreshIfNeeded (parentTree); }

    void refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == exportersTree)
            refreshSubItems();
    }

private:
    Project& project;
    ValueTree exportersTree;

    //==============================================================================
    class SettingsComp  : public Component,
                          private ChangeListener
    {
    public:
        SettingsComp (Project& p)  : project (p)
        {
            addAndMakeVisible (group);

            updatePropertyList();
            project.addChangeListener (this);
        }

        ~SettingsComp()
        {
            project.removeChangeListener (this);
        }

        void parentSizeChanged() override
        {
            updateSize (*this, group);
        }

        void updatePropertyList()
        {
            PropertyListBuilder props;
            project.createPropertyEditors (props);
            group.setProperties (props);
            group.setName ("Project Settings");

            lastProjectType = project.getProjectTypeValue().getValue();
            parentSizeChanged();
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            if (lastProjectType != project.getProjectTypeValue().getValue())
                updatePropertyList();
        }

    private:
        Project& project;
        var lastProjectType;
        PropertyGroupComponent group;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootItem)
};
