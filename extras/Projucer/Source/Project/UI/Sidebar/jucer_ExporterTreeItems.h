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
class ExporterItem   : public ProjectTreeItemBase,
                       private Value::Listener
{
public:
    ExporterItem (Project& p, ProjectExporter* e, int index)
        : project (p), exporter (e), configListTree (exporter->getConfigurations()),
          exporterIndex (index)
    {
        exporter->initialiseDependencyPathValues();
        configListTree.addListener (this);
        targetLocationValue.referTo (exporter->getTargetLocationValue());
        targetLocationValue.addListener (this);
    }

    int getItemHeight() const override        { return 25; }
    bool canBeSelected() const override       { return true; }
    bool mightContainSubItems() override      { return exporter->getNumConfigurations() > 0; }
    String getUniqueName() const override     { return "exporter_" + String (exporterIndex); }
    String getRenamingName() const override   { return getDisplayName(); }
    String getDisplayName() const override    { return exporter->getName(); }
    void setName (const String&) override     {}
    bool isMissing() const override           { return false; }
    String getTooltip() override              { return getDisplayName(); }

    static Icon getIconForExporter (ProjectExporter* e)
    {
        if (e != nullptr)
        {
            if         (e->isXcode())        return Icon (getIcons().xcode,        Colours::transparentBlack);
            else if    (e->isVisualStudio()) return Icon (getIcons().visualStudio, Colours::transparentBlack);
            else if    (e->isAndroid())      return Icon (getIcons().android,      Colours::transparentBlack);
            else if    (e->isCodeBlocks())   return Icon (getIcons().codeBlocks,   Colours::transparentBlack);
            else if    (e->isMakefile())     return Icon (getIcons().linux,        Colours::transparentBlack);
            else if    (e->isCLion())        return Icon (getIcons().clion,        Colours::transparentBlack);
        }

        return Icon();
    }

    Icon getIcon() const override
    {
        return getIconForExporter (exporter.get()).withColour (getContentColour (true));
    }

    void showDocument() override
    {
        showSettingsPage (new SettingsComp (exporter.get()));
    }

    void deleteItem() override
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Exporter",
                                          "Are you sure you want to delete this export target?"))
        {
            closeSettingsPage();
            ValueTree parent (exporter->settings.getParent());
            parent.removeChild (exporter->settings, project.getUndoManagerFor (parent));
        }
    }

    void addSubItems() override
    {
        for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
            addSubItem (new ConfigItem (config.config, *exporter));
    }

    void showPopupMenu() override
    {
        PopupMenu menu;
        menu.addItem (1, "Add a new configuration", exporter->supportsUserDefinedConfigurations());
        menu.addItem (2, "Save this exporter");
        menu.addSeparator();
        menu.addItem (3, "Delete this exporter");

        launchPopupMenu (menu);
    }

    void showAddMenu() override
    {
        PopupMenu menu;
        menu.addItem (1, "Add a new configuration", exporter->supportsUserDefinedConfigurations());

        launchPopupMenu (menu);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        if (resultCode == 1)
        {
            exporter->addNewConfiguration (false);
        }
        else if (resultCode == 2)
        {
            const ScopedValueSetter<String> valueSetter (project.specifiedExporterToSave, exporter->getName(), {});
            project.save (true, true);
        }
        else if (resultCode == 3)
        {
            deleteAllSelectedItems();
        }
    }

    var getDragSourceDescription() override
    {
        return getParentItem()->getUniqueName() + "/" + String (exporterIndex);
    }

    bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails) override
    {
        return dragSourceDetails.description.toString().startsWith (getUniqueName());
    }

    void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex) override
    {
        auto oldIndex = indexOfConfig (dragSourceDetails.description.toString().fromLastOccurrenceOf ("||", false, false));

        if (oldIndex >= 0)
            configListTree.moveChild (oldIndex, insertIndex, project.getUndoManagerFor (configListTree));
    }

    int indexOfConfig (const String& configName)
    {
        int i = 0;
        for (ProjectExporter::ConfigIterator config (*exporter); config.next(); ++i)
            if (config->getName() == configName)
                return i;

        return -1;
    }

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override         { refreshIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override  { refreshIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override    { refreshIfNeeded (parentTree); }

    void refreshIfNeeded (ValueTree& changedTree)
    {
        if (changedTree == configListTree)
            refreshSubItems();
    }

private:
    Project& project;
    std::unique_ptr<ProjectExporter> exporter;
    ValueTree configListTree;
    int exporterIndex;

    Value targetLocationValue;

    void valueChanged (Value& value) override
    {
        if (value == exporter->getTargetLocationValue())
            refreshSubItems();
    }

    //==============================================================================
    struct SettingsComp  : public Component
    {
        SettingsComp (ProjectExporter* exp)
            : group (exp->getName(),
                     ExporterItem::getIconForExporter (exp),
                     exp->getDescription())
        {
            addAndMakeVisible (group);

            PropertyListBuilder props;
            exp->createPropertyEditors (props);
            group.setProperties (props);
            parentSizeChanged();
        }

        void parentSizeChanged() override   { updateSize (*this, group); }
        void resized() override             { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

        PropertyGroupComponent group;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterItem)
};


//==============================================================================
class ConfigItem   : public ProjectTreeItemBase
{
public:
    ConfigItem (const ProjectExporter::BuildConfiguration::Ptr& conf, ProjectExporter& e)
        : config (conf), exporter (e), configTree (config->config)
    {
        jassert (config != nullptr);
        configTree.addListener (this);
    }

    bool isMissing() const override                 { return false; }
    bool canBeSelected() const override             { return true; }
    bool mightContainSubItems() override            { return false; }
    String getUniqueName() const override           { return "config_" + config->getName(); }
    String getRenamingName() const override         { return getDisplayName(); }
    String getDisplayName() const override          { return config->getName(); }
    void setName (const String&) override           {}
    Icon getIcon() const override                   { return Icon (getIcons().config, getContentColour (true)); }
    void itemOpennessChanged (bool) override        {}

    void showDocument() override
    {
        showSettingsPage (new SettingsComp (config));
    }

    void deleteItem() override
    {
        if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Configuration",
                                          "Are you sure you want to delete this configuration?"))
        {
            closeSettingsPage();
            config->removeFromExporter();
        }
    }

    void showPopupMenu() override
    {
        bool enabled = exporter.supportsUserDefinedConfigurations();

        PopupMenu menu;
        menu.addItem (1, "Create a copy of this configuration", enabled);
        menu.addSeparator();
        menu.addItem (2, "Delete this configuration", enabled);

        launchPopupMenu (menu);
    }

    void handlePopupMenuResult (int resultCode) override
    {
        if (resultCode == 1)
            exporter.addNewConfigurationFromExisting (*config);
        else if (resultCode == 2)
            deleteAllSelectedItems();
    }

    var getDragSourceDescription() override
    {
        return getParentItem()->getUniqueName() + "||" + config->getName();
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override  { repaintItem(); }

private:
    ProjectExporter::BuildConfiguration::Ptr config;
    ProjectExporter& exporter;
    ValueTree configTree;

    //==============================================================================
    class SettingsComp  : public Component
    {
    public:
        SettingsComp (ProjectExporter::BuildConfiguration* conf)
            : group (conf->exporter.getName() + " - " + conf->getName(), Icon (getIcons().config, Colours::transparentBlack))
        {
            addAndMakeVisible (group);

            PropertyListBuilder props;
            conf->createPropertyEditors (props);
            group.setProperties (props);
            parentSizeChanged();
        }

        void parentSizeChanged() override  { updateSize (*this, group); }

        void resized() override { group.setBounds (getLocalBounds().withTrimmedLeft (12)); }

    private:
        PropertyGroupComponent group;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigItem)
};

//==============================================================================
class ExportersTreeRoot    : public ProjectTreeItemBase
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
            addSubItem (new TreeItemTypes::ExporterItem (project, exporter.exporter.release(), i));
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
        if (auto* exporter = dynamic_cast<TreeItemTypes::ExporterItem*> (getSubItem (index)))
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
