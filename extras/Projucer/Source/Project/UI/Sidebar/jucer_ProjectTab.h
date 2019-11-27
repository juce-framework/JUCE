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

#pragma once


//==============================================================================
struct ProjectSettingsComponent  : public Component,
                                   private ChangeListener
{
    ProjectSettingsComponent (Project& p)
        : project (p),
          group (project.getProjectFilenameRootString(),
                 Icon (getIcons().settings, Colours::transparentBlack))
    {
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
        setRoot (new TreeItemTypes::GroupItem (p.getMainGroup()));
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
        setRoot (new TreeItemTypes::EnabledModulesItem (p));
        tree.setRootItemVisible (false);
    }
};

struct ExportersTreePanel    : public TreePanelBase
{
    ExportersTreePanel (Project& p)
        : TreePanelBase (&p, "exportersTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (new TreeItemTypes::ExportersTreeRoot (p));
        tree.setRootItemVisible (false);
    }
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

    ~ProjectTab() override
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
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        if (project != nullptr)
        {
            concertinaPanel.addPanel (0, new ConcertinaTreeComponent (new FileTreePanel (*project), true, false, true), true);
            concertinaPanel.addPanel (1, new ConcertinaTreeComponent (new ModuleTreePanel (*project), true, true), true);
            concertinaPanel.addPanel (2, new ConcertinaTreeComponent (new ExportersTreePanel (*project), true), true);
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
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0; --i)
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
