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
class ConcertinaHeader    : public Component,
                            public ChangeBroadcaster
{
public:
    ConcertinaHeader (String n, Path p)
        : Component (n), name (n), iconPath (p)
    {
        setTitle (getName());

        panelIcon = Icon (iconPath, Colours::white);

        nameLabel.setText (name, dontSendNotification);
        nameLabel.setJustificationType (Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks (false, false);
        nameLabel.setAccessible (false);
        nameLabel.setColour (Label::textColourId, Colours::white);

        addAndMakeVisible (nameLabel);
    }

    void resized() override
    {
        auto b = getLocalBounds().toFloat();

        iconBounds = b.removeFromLeft (b.getHeight()).reduced (7, 7);
        arrowBounds = b.removeFromRight (b.getHeight());
        nameLabel.setBounds (b.toNearestInt());
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (defaultButtonBackgroundColourId));
        g.fillRoundedRectangle (getLocalBounds().reduced (2, 3).toFloat(), 2.0f);

        g.setColour (Colours::white);
        g.fillPath (ProjucerLookAndFeel::getArrowPath (arrowBounds,
                                                       getParentComponent()->getBoundsInParent().getY() == yPosition ? 2 : 0,
                                                       true, Justification::centred));

        panelIcon.draw (g, iconBounds.toFloat(), false);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (! e.mouseWasDraggedSinceMouseDown())
            sendChangeMessage();
    }

    std::unique_ptr<AccessibilityHandler> createAccessibilityHandler() override
    {
        return std::make_unique<AccessibilityHandler> (*this,
                                                       AccessibilityRole::button,
                                                       AccessibilityActions().addAction (AccessibilityActionType::press,
                                                                                         [this] { sendChangeMessage(); }));
    }

    int direction = 0;
    int yPosition = 0;

private:
    String name;
    Label nameLabel;

    Path iconPath;
    Icon panelIcon;

    Rectangle<float> arrowBounds, iconBounds;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaHeader)
};

//==============================================================================
class FindPanel    : public Component,
                     private Timer,
                     private FocusChangeListener
{
public:
    FindPanel (std::function<void (const String&)> cb)
        : callback (cb)
    {
        addAndMakeVisible (editor);
        editor.onTextChange = [this] { startTimer (250); };
        editor.onFocusLost  = [this]
        {
            isFocused = false;
            repaint();
        };

        Desktop::getInstance().addFocusChangeListener (this);

        lookAndFeelChanged();
    }

    ~FindPanel() override
    {
        Desktop::getInstance().removeFocusChangeListener (this);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (! isFocused)
            return;

        g.setColour (findColour (defaultHighlightColourId));

        Path p;
        p.addRoundedRectangle (getLocalBounds().reduced (2), 3.0f);
        g.strokePath (p, PathStrokeType (2.0f));
    }


    void resized() override
    {
        editor.setBounds (getLocalBounds().reduced (2));
    }

private:
    TextEditor editor;
    bool isFocused = false;
    std::function<void (const String&)> callback;

    //==============================================================================
    void lookAndFeelChanged() override
    {
        editor.setTextToShowWhenEmpty ("Filter...", findColour (widgetTextColourId).withAlpha (0.3f));
    }

    void globalFocusChanged (Component* focusedComponent) override
    {
        if (focusedComponent == &editor)
        {
            isFocused = true;
            repaint();
        }
    }

    void timerCallback() override
    {
        stopTimer();
        callback (editor.getText());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FindPanel)
};

//==============================================================================
class ConcertinaTreeComponent    : public Component
{
public:
    class AdditionalComponents
    {
    public:
        enum Type
        {
            addButton      = (1 << 0),
            settingsButton = (1 << 1),
            findPanel      = (1 << 2)
        };

        JUCE_NODISCARD AdditionalComponents with (Type t)
        {
            auto copy = *this;
            copy.componentTypes |= t;

            return copy;
        }

        bool has (Type t) const noexcept
        {
            return (componentTypes & t) != 0;
        }

    private:
        int componentTypes = 0;
    };

    ConcertinaTreeComponent (const String& name,
                             TreePanelBase* tree,
                             AdditionalComponents additionalComponents)
         : Component (name),
           treeToDisplay (tree)
    {
        setTitle (getName());
        setFocusContainerType (FocusContainerType::focusContainer);

        if (additionalComponents.has (AdditionalComponents::addButton))
        {
            addButton = std::make_unique<IconButton> ("Add", getIcons().plus);
            addAndMakeVisible (addButton.get());
            addButton->onClick = [this] { showAddMenu(); };
        }

        if (additionalComponents.has (AdditionalComponents::settingsButton))
        {
            settingsButton = std::make_unique<IconButton> ("Settings", getIcons().settings);
            addAndMakeVisible (settingsButton.get());
            settingsButton->onClick = [this] { showSettings(); };
        }

        if (additionalComponents.has (AdditionalComponents::findPanel))
        {
            findPanel = std::make_unique<FindPanel> ([this] (const String& filter) { treeToDisplay->rootItem->setSearchFilter (filter); });
            addAndMakeVisible (findPanel.get());
        }

        addAndMakeVisible (treeToDisplay.get());
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        if (addButton != nullptr || settingsButton != nullptr || findPanel != nullptr)
        {
            auto bottomSlice = bounds.removeFromBottom (25);
            bottomSlice.removeFromRight (3);

            if (addButton != nullptr)
                addButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (settingsButton != nullptr)
                settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

            if (findPanel != nullptr)
                findPanel->setBounds (bottomSlice.reduced (2));
        }

        treeToDisplay->setBounds (bounds);
    }

    TreePanelBase* getTree() const noexcept    { return treeToDisplay.get(); }

private:
    std::unique_ptr<TreePanelBase> treeToDisplay;
    std::unique_ptr<IconButton> addButton, settingsButton;
    std::unique_ptr<FindPanel> findPanel;

    void showAddMenu()
    {
        auto numSelected = treeToDisplay->tree.getNumSelectedItems();

        if (numSelected > 1)
            return;

        if (numSelected == 0)
        {
            if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
                root->showPopupMenu (addButton->getScreenBounds().getCentre());
        }
        else
        {
            if (auto* item = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getSelectedItem (0)))
                item->showAddMenu (addButton->getScreenBounds().getCentre());
        }
    }

    void showSettings()
    {
        if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
        {
            treeToDisplay->tree.clearSelectedItems();
            root->showDocument();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaTreeComponent)
};


//==============================================================================
struct ProjectSettingsComponent  : public Component,
                                   private ChangeListener
{
    ProjectSettingsComponent (Project& p)
        : project (p),
          group (project.getProjectFilenameRootString(),
                 Icon (getIcons().settings, Colours::transparentBlack))
    {
        setTitle ("Project Settings");
        setFocusContainerType (FocusContainerType::focusContainer);

        addAndMakeVisible (group);

        updatePropertyList();
        project.addChangeListener (this);
    }

    ~ProjectSettingsComponent() override
    {
        project.removeChangeListener (this);
    }

    void resized() override
    {
        group.updateSize (12, 0, getWidth() - 24);
        group.setBounds (getLocalBounds().reduced (12, 0));
    }

    void updatePropertyList()
    {
        PropertyListBuilder props;
        project.createPropertyEditors (props);
        group.setProperties (props);
        group.setName ("Project Settings");

        lastProjectType = project.getProjectTypeString();
        parentSizeChanged();
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        if (lastProjectType != project.getProjectTypeString())
            updatePropertyList();
    }

    void parentSizeChanged() override
    {
        auto width = jmax (550, getParentWidth());
        auto y = group.updateSize (12, 0, width - 12);

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    Project& project;
    var lastProjectType;
    PropertyGroupComponent group;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectSettingsComponent)
};

//==============================================================================
struct FileTreePanel   : public TreePanelBase
{
    FileTreePanel (Project& p)
        : TreePanelBase (&p, "fileTreeState")
    {
        tree.setMultiSelectEnabled (true);
        setRoot (std::make_unique<TreeItemTypes::GroupItem> (p.getMainGroup()));
        tree.setRootItemVisible (false);
    }

    void updateMissingFileStatuses()
    {
        if (auto* p = dynamic_cast<TreeItemTypes::FileTreeItemBase*> (rootItem.get()))
            p->checkFileStatus();
    }
};

struct ModuleTreePanel    : public TreePanelBase
{
    ModuleTreePanel (Project& p)
        : TreePanelBase (&p, "moduleTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (std::make_unique<TreeItemTypes::EnabledModulesItem> (p));
        tree.setRootItemVisible (false);
    }
};

struct ExportersTreePanel    : public TreePanelBase
{
    ExportersTreePanel (Project& p)
        : TreePanelBase (&p, "exportersTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (std::make_unique<TreeItemTypes::ExportersTreeRoot> (p));
        tree.setRootItemVisible (false);
    }
};

//==============================================================================
class Sidebar    : public Component,
                   private ChangeListener
{
public:
    Sidebar (Project* p)
        : project (p)
    {
        setFocusContainerType (FocusContainerType::focusContainer);

        if (project != nullptr)
            buildConcertina();
    }

    ~Sidebar() override
    {
        TreePanelBase* panels[] = { getFileTreePanel(), getModuleTreePanel(), getExportersTreePanel() };

        for (auto* panel : panels)
            if (panel != nullptr)
                panel->saveOpenness();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        concertinaPanel.setBounds (getLocalBounds().withTrimmedBottom (3));
    }

    TreePanelBase* getTreeWithSelectedItems()
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
        {
            if (auto* treeComponent = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (i)))
            {
                if (auto* base = treeComponent->getTree())
                    if (base->tree.getNumSelectedItems() != 0)
                        return base;
            }
        }

        return nullptr;
    }

    FileTreePanel*      getFileTreePanel()        { return getPanel<FileTreePanel>      (0); }
    ModuleTreePanel*    getModuleTreePanel()      { return getPanel<ModuleTreePanel>    (1); }
    ExportersTreePanel* getExportersTreePanel()   { return getPanel<ExportersTreePanel> (2); }

    void showPanel (int panelIndex)
    {
        jassert (isPositiveAndBelow (panelIndex, concertinaPanel.getNumPanels()));

        concertinaPanel.expandPanelFully (concertinaPanel.getPanel (panelIndex), true);
    }

private:
    //==============================================================================
    template <typename PanelType>
    PanelType* getPanel (int panelIndex)
    {
        if (auto* panel = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (panelIndex)))
            return dynamic_cast<PanelType*> (panel->getTree());

        return nullptr;
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        const auto pointerMatches = [source] (const std::unique_ptr<ConcertinaHeader>& header) { return header.get() == source; };
        const auto it = std::find_if (headers.begin(), headers.end(), pointerMatches);
        const auto index = (int) std::distance (headers.begin(), it);

        if (index != (int) headers.size())
            concertinaPanel.expandPanelFully (concertinaPanel.getPanel (index), true);
    }

    void buildConcertina()
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        auto addPanel = [this] (const String& name,
                                TreePanelBase* tree,
                                ConcertinaTreeComponent::AdditionalComponents components,
                                const Path& icon)
        {
            if (project != nullptr)
                concertinaPanel.addPanel (-1, new ConcertinaTreeComponent (name, tree, components), true);

            headers.push_back (std::make_unique<ConcertinaHeader> (name, icon));
        };

        using AdditionalComponents = ConcertinaTreeComponent::AdditionalComponents;

        addPanel ("File Explorer", new FileTreePanel (*project),
                  AdditionalComponents{}
                      .with (AdditionalComponents::addButton)
                      .with (AdditionalComponents::findPanel),
                  getIcons().fileExplorer);

        addPanel ("Modules", new ModuleTreePanel (*project),
                  AdditionalComponents{}
                      .with (AdditionalComponents::addButton)
                      .with (AdditionalComponents::settingsButton),
                  getIcons().modules);

        addPanel ("Exporters", new ExportersTreePanel (*project),
                  AdditionalComponents{}.with (AdditionalComponents::addButton),
                  getIcons().exporter);

        for (int i = 0; i < concertinaPanel.getNumPanels(); ++i)
        {
            auto* p = concertinaPanel.getPanel (i);
            auto* h = headers[(size_t) i].get();
            p->addMouseListener (this, true);

            h->addChangeListener (this);
            h->yPosition = i * 30;

            concertinaPanel.setCustomPanelHeader (p, h, false);
            concertinaPanel.setPanelHeaderSize (p, 30);
        }

        addAndMakeVisible (concertinaPanel);
    }

    void mouseDown (const MouseEvent& e) override
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
        {
            if (auto* p = concertinaPanel.getPanel (i))
            {
                if (! (p->isParentOf (e.eventComponent)))
                {
                    auto* base = dynamic_cast<TreePanelBase*> (p);

                    if (base == nullptr)
                        if (auto* concertina = dynamic_cast<ConcertinaTreeComponent*> (p))
                            base = concertina->getTree();

                    if (base != nullptr)
                        base->tree.clearSelectedItems();
                }
            }
        }
    }

    //==============================================================================
    ConcertinaPanel concertinaPanel;
    std::vector<std::unique_ptr<ConcertinaHeader>> headers;
    Project* project = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Sidebar)
};
