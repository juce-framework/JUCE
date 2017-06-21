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

#include "../jucer_Headers.h"
#include "jucer_TreeItemTypes.h"

//==============================================================================
class ConcertinaHeader    : public Component,
                            public ChangeBroadcaster
{
public:
    ConcertinaHeader (String n, Path p)
        : Component (n), name (n), iconPath (p)
    {
        panelIcon = Icon (iconPath, Colours::white);

        nameLabel.setText (name, dontSendNotification);
        nameLabel.setJustificationType (Justification::centredLeft);
        nameLabel.setInterceptsMouseClicks (false, false);
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
        g.fillPath (arrowPath = ProjucerLookAndFeel::getArrowPath (arrowBounds,
                                                                   getParentComponent()->getBoundsInParent().getY() == yPosition ? 2 : 0,
                                                                   true, Justification::centred));

        panelIcon.draw (g, iconBounds.toFloat(), false);
    }

    void mouseUp (const MouseEvent& e) override
    {
        if (arrowPath.getBounds().expanded (3).contains (e.getPosition().toFloat()))
            sendChangeMessage();
    }

    int direction = 0;
    int yPosition = 0;

private:
    String name;
    Label nameLabel;

    Path iconPath;
    Icon panelIcon;

    Rectangle<float> arrowBounds, iconBounds;
    Path arrowPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaHeader)
};

//==============================================================================
class ExportersTreeRoot    : public JucerTreeViewBase,
                            private ValueTree::Listener
{
public:
    ExportersTreeRoot (Project& p)
        : project (p),
          exportersTree (project.getExporters())
    {
        exportersTree.addListener (this);
    }

    bool isRoot() const override                     { return true; }
    bool canBeSelected() const override              { return true; }
    bool isMissing() const override                  { return false; }
    bool mightContainSubItems() override             { return project.getNumExporters() > 0; }
    String getUniqueName() const override            { return "exporters"; }
    String getRenamingName() const override          { return getDisplayName(); }
    String getDisplayName() const override           { return "Exporters"; }
    void setName (const String&) override            {}
    Icon getIcon() const override                    { return project.getMainGroup().getIcon (isOpen()).withColour (getContentColour (true)); }

    void showPopupMenu() override
    {
        if (auto* pcc = getProjectContentComponent())
            pcc->showNewExporterMenu();
    }

    void addSubItems() override
    {
        int i = 0;
        for (Project::ExporterIterator exporter (project); exporter.next(); ++i)
            addSubItem (new ConfigTreeItemTypes::ExporterItem (project, exporter.exporter.release(), i));
    }

    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description.toString().startsWith (getUniqueName());
    }

    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override
    {
        int oldIndex = dragSourceDetails.description.toString().getTrailingIntValue();
        exportersTree.moveChild (oldIndex, jmax (0, insertIndex), project.getUndoManagerFor (exportersTree));
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
            refreshSubItems();
    }

    void removeExporter (int index)
    {
        if (auto* exporter = dynamic_cast<ConfigTreeItemTypes::ExporterItem*> (getSubItem (index)))
            exporter->deleteItem();
    }

private:
    Project& project;
    ValueTree exportersTree;

    //==========================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override        {}
    void valueTreeParentChanged (ValueTree&) override                             {}
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { refreshIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { refreshIfNeeded (parentTree); }

    void refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == exportersTree)
            refreshSubItems();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExportersTreeRoot)
};

struct FileTreePanel   : public TreePanelBase
{
    FileTreePanel (Project& p)
        : TreePanelBase (&p, "fileTreeState")
    {
        tree.setMultiSelectEnabled (true);
        setRoot (new FileTreeItemTypes::GroupItem (p.getMainGroup()));
        tree.setRootItemVisible (false);
    }

    void updateMissingFileStatuses()
    {
        if (auto* p = dynamic_cast<FileTreeItemTypes::ProjectTreeItemBase*> (rootItem.get()))
            p->checkFileStatus();
    }

    void setSearchFilter (const String& filter)
    {
        if (auto* p = dynamic_cast<FileTreeItemTypes::GroupItem*> (rootItem.get()))
            p->setSearchFilter (filter);
    }
};

struct ModuleTreePanel    : public TreePanelBase
{
    ModuleTreePanel (Project& p)
        : TreePanelBase (&p, "moduleTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (new ConfigTreeItemTypes::EnabledModulesItem (p));
        tree.setRootItemVisible (false);
    }
};

struct ExportersTreePanel    : public TreePanelBase
{
    ExportersTreePanel (Project& p)
        : TreePanelBase (&p, "exportersTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (new ExportersTreeRoot (p));
        tree.setRootItemVisible (false);
    }

    void deleteSelectedItems() override
    {
        for (int i = rootItem->getNumSubItems() - 1; i >= 0; --i)
            if (rootItem->getSubItem (i)->isSelected())
                if (auto* root = dynamic_cast<ExportersTreeRoot*> (rootItem.get()))
                    root->removeExporter (i);
    }
};

//==============================================================================
class ConcertinaTreeComponent    : public Component,
                                   private Button::Listener,
                                   private ChangeListener
{
public:
    ConcertinaTreeComponent (TreePanelBase* tree, bool showSettingsButton = false, bool showFindPanel = false)
         : treeToDisplay (tree)
    {
        addAndMakeVisible (popupMenuButton = new IconButton ("Add", &getIcons().plus));
        popupMenuButton->addListener (this);

        if (showSettingsButton)
        {
            addAndMakeVisible (settingsButton = new IconButton ("Settings", &getIcons().settings));
            settingsButton->addListener (this);
        }

        if (showFindPanel)
        {
            addAndMakeVisible (findPanel = new FindPanel());
            findPanel->addChangeListener (this);
        }

        addAndMakeVisible (treeToDisplay);
    }

    ~ConcertinaTreeComponent()
    {
        treeToDisplay = nullptr;
        popupMenuButton = nullptr;
        findPanel = nullptr;
        settingsButton = nullptr;
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto bottomSlice = bounds.removeFromBottom (25);
        bottomSlice.removeFromRight (5);
        popupMenuButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

        if (settingsButton != nullptr)
            settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

        if (findPanel != nullptr)
            findPanel->setBounds (bottomSlice.reduced (2));

        treeToDisplay->setBounds (bounds);
    }

    TreePanelBase* getTree() const noexcept             { return treeToDisplay.get(); }

    void grabFindFocus()
    {
        if (findPanel != nullptr)
            findPanel->grabKeyboardFocus();
    }

private:
    ScopedPointer<TreePanelBase> treeToDisplay;
    ScopedPointer<IconButton> popupMenuButton, settingsButton;

    void buttonClicked (Button* b) override
    {
        if (b == popupMenuButton)
        {
            auto numSelected = treeToDisplay->tree.getNumSelectedItems();

            if (numSelected > 1)
                return;

            if (numSelected == 0)
            {
                if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
                    root->showPopupMenu();
            }
            else
            {
                auto* selectedItem = treeToDisplay->tree.getSelectedItem (0);

                if (auto* fileItem = dynamic_cast<FileTreeItemTypes::ProjectTreeItemBase*> (selectedItem))
                    fileItem->showPlusMenu();
                else if (auto* exporterItem = dynamic_cast<ConfigTreeItemTypes::ExporterItem*> (selectedItem))
                    exporterItem->showPlusMenu();
            }
        }
        else if (b == settingsButton)
        {
            if (auto* root = dynamic_cast<JucerTreeViewBase*> (treeToDisplay->tree.getRootItem()))
            {
                treeToDisplay->tree.clearSelectedItems();
                root->showDocument();
            }
        }
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == findPanel)
            if (auto* fileTree = dynamic_cast<FileTreePanel*> (treeToDisplay.get()))
                fileTree->setSearchFilter (findPanel->editor.getText());
    }

    class FindPanel : public Component,
                      public ChangeBroadcaster,
                      private TextEditor::Listener,
                      private Timer,
                      private FocusChangeListener
    {
    public:
        FindPanel()
        {
            addAndMakeVisible (editor);
            editor.addListener (this);

            Desktop::getInstance().addFocusChangeListener (this);

            lookAndFeelChanged();
        }

        ~FindPanel()
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

        TextEditor editor;

    private:

        void lookAndFeelChanged() override
        {
            editor.setTextToShowWhenEmpty ("Filter...", findColour (widgetTextColourId).withAlpha (0.3f));
        }

        void textEditorTextChanged (TextEditor&) override
        {
            startTimer (250);
        }

        void textEditorFocusLost (TextEditor&) override
        {
            isFocused = false;
            repaint();
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
            sendChangeMessage();
        }

        bool isFocused = false;
    };
    ScopedPointer<FindPanel> findPanel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConcertinaTreeComponent)
};

//==============================================================================
class ProjectTab    : public Component,
                      private ChangeListener
{
public:
    ProjectTab (Project* p)
        : project (p)
    {
        addAndMakeVisible (concertinaPanel);
        buildConcertina();
    }

    ~ProjectTab()
    {
        getFileTreePanel()->saveOpenness();
        getModuleTreePanel()->saveOpenness();
        getExportersTreePanel()->saveOpenness();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        concertinaPanel.setBounds (getLocalBounds());
    }

    TreePanelBase* getTreeWithSelectedItems()
    {
        for (int i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
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

    FileTreePanel* getFileTreePanel()
    {
        if (auto* panel = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (0)))
            return dynamic_cast<FileTreePanel*> (panel->getTree());

        return nullptr;
    }

    ModuleTreePanel* getModuleTreePanel()
    {
        if (auto* panel = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (1)))
            return dynamic_cast<ModuleTreePanel*> (panel->getTree());

        return nullptr;
    }

    ExportersTreePanel* getExportersTreePanel()
    {
        if (auto* panel = dynamic_cast<ConcertinaTreeComponent*> (concertinaPanel.getPanel (2)))
            return dynamic_cast<ExportersTreePanel*> (panel->getTree());

        return nullptr;
    }

    void showPanel (int panelIndex)
    {
        jassert (isPositiveAndBelow (panelIndex, concertinaPanel.getNumPanels()));

        concertinaPanel.expandPanelFully (concertinaPanel.getPanel (panelIndex), true);
    }

    void setPanelHeightProportion (int panelIndex, float prop)
    {
        jassert (isPositiveAndBelow (panelIndex, concertinaPanel.getNumPanels()));

        concertinaPanel.setPanelSize (concertinaPanel.getPanel (panelIndex),
                                      roundToInt (prop * (concertinaPanel.getHeight() - 90)), false);
    }

    float getPanelHeightProportion (int panelIndex)
    {
        jassert (isPositiveAndBelow (panelIndex, concertinaPanel.getNumPanels()));

        return ((float) (concertinaPanel.getPanel (panelIndex)->getHeight()) / (concertinaPanel.getHeight() - 90));
    }

private:
    ConcertinaPanel concertinaPanel;
    OwnedArray<ConcertinaHeader> headers;
    Project* project = nullptr;

    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* header = dynamic_cast<ConcertinaHeader*> (source))
        {
            auto index = headers.indexOf (header);
            concertinaPanel.expandPanelFully (concertinaPanel.getPanel (index), true);
        }
    }

    void buildConcertina()
    {
        for (int i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        if (project != nullptr)
        {
            concertinaPanel.addPanel (0, new ConcertinaTreeComponent (new FileTreePanel (*project), false, true), true);
            concertinaPanel.addPanel (1, new ConcertinaTreeComponent (new ModuleTreePanel (*project), true), true);
            concertinaPanel.addPanel (2, new ConcertinaTreeComponent (new ExportersTreePanel (*project)), true);
        }

        headers.add (new ConcertinaHeader ("File explorer", getIcons().fileExplorer));
        headers.add (new ConcertinaHeader ("Modules",       getIcons().modules));
        headers.add (new ConcertinaHeader ("Exporters",     getIcons().exporter));

        for (int i = 0; i < concertinaPanel.getNumPanels(); ++i)
        {
            auto* p = concertinaPanel.getPanel (i);
            auto* h = headers.getUnchecked (i);
            p->addMouseListener (this, true);

            h->addChangeListener (this);
            h->yPosition = i * 30;

            concertinaPanel.setCustomPanelHeader (p, h, false);
            concertinaPanel.setPanelHeaderSize (p, 30);
        }
    }

    void mouseDown (const MouseEvent& e) override
    {
        for (int i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
        {
            auto* p = concertinaPanel.getPanel (i);

            if (! (p->isParentOf (e.eventComponent)))
            {
                auto* base = dynamic_cast<TreePanelBase*> (p);

                if (base == nullptr)
                    base = dynamic_cast<ConcertinaTreeComponent*> (p)->getTree();

                if (base != nullptr)
                    base->tree.clearSelectedItems();
            }
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectTab)
};
