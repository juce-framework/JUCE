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
class ModulesInformationComponent  : public Component,
                                     private ListBoxModel,
                                     private ValueTree::Listener
{
public:
    ModulesInformationComponent (Project& p)
        : project (p),
          modulesValueTree (project.getEnabledModules().getState())
    {
        auto tempHeader = std::make_unique<ListBoxHeader> (Array<String> { "Module", "Version", "Make Local Copy", "Paths" },
                                                           Array<float> { 0.25f, 0.2f, 0.2f, 0.35f });
        listHeader = tempHeader.get();
        list.setHeaderComponent (std::move (tempHeader));
        list.setModel (this);
        list.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (list);
        list.updateContent();
        list.setRowHeight (30);
        list.setMultipleSelectionEnabled (true);

        addAndMakeVisible (header);

        addAndMakeVisible (setCopyModeButton);
        setCopyModeButton.setTriggeredOnMouseDown (true);
        setCopyModeButton.onClick = [this] { showCopyModeMenu(); };

        addAndMakeVisible (copyPathButton);
        copyPathButton.setTriggeredOnMouseDown (true);
        copyPathButton.onClick = [this] { showSetPathsMenu(); };

        addAndMakeVisible (globalPathsButton);
        globalPathsButton.onClick = [this] { showGlobalPathsMenu(); };

        modulesValueTree.addListener (this);
        lookAndFeelChanged();
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (secondaryBackgroundColourId));
        g.fillRect (getLocalBounds().reduced (12, 0));
    }

    void resized() override
    {
        auto bounds = getLocalBounds().reduced (12, 0);

        header.setBounds (bounds.removeFromTop (40));

        bounds.reduce (10, 0);
        list.setBounds (bounds.removeFromTop (list.getRowPosition (getNumRows() - 1, true).getBottom() + 20));

        if (bounds.getHeight() < 35)
        {
            parentSizeChanged();
        }
        else
        {
            auto buttonRow = bounds.removeFromTop (35);
            setCopyModeButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
            buttonRow.removeFromLeft (8);
            copyPathButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
            buttonRow.removeFromLeft (8);
            globalPathsButton.setBounds (buttonRow.removeFromLeft (jmin (200, bounds.getWidth() / 3)));
        }
    }

    void parentSizeChanged() override
    {
        auto width = jmax (550, getParentWidth());
        auto y = list.getRowPosition (getNumRows() - 1, true).getBottom() + 200;

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    int getNumRows() override
    {
        return project.getEnabledModules().getNumModules();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (height);

        Rectangle<int> bounds (0, 0, width, height);

        g.setColour (rowIsSelected ? findColour (defaultHighlightColourId) : findColour (rowNumber % 2 == 0 ? widgetBackgroundColourId
                                                                                                            : secondaryWidgetBackgroundColourId));
        g.fillRect (bounds.withTrimmedBottom (1));

        bounds.removeFromLeft (5);
        g.setColour (rowIsSelected ? findColour (defaultHighlightedTextColourId) : findColour (widgetTextColourId));

        //==============================================================================
        auto moduleID = project.getEnabledModules().getModuleID (rowNumber);

        g.drawFittedText (moduleID, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (0) * (float) width)), Justification::centredLeft, 1);

        //==============================================================================
        auto version = project.getEnabledModules().getModuleInfo (moduleID).getVersion();
        if (version.isEmpty())
            version = "?";

        g.drawFittedText (version, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (1) * (float) width)), Justification::centredLeft, 1);

        //==============================================================================
        g.drawFittedText (String (project.getEnabledModules().shouldCopyModuleFilesLocally (moduleID) ? "Yes" : "No"),
                          bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (2) * (float) width)), Justification::centredLeft, 1);

        //==============================================================================
        String pathText;

        if (project.getEnabledModules().shouldUseGlobalPath (moduleID))
        {
            pathText = "Global";
        }
        else
        {
            StringArray paths;

            for (Project::ExporterIterator exporter (project); exporter.next();)
                paths.addIfNotAlreadyThere (exporter->getPathForModuleString (moduleID).trim());

            paths.removeEmptyStrings();
            paths.removeDuplicates (true);

            pathText = paths.joinIntoString (", ");
        }

        g.drawFittedText (pathText, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (3) * (float) width)), Justification::centredLeft, 1);
    }

    void listBoxItemDoubleClicked (int row, const MouseEvent&) override
    {
        auto moduleID = project.getEnabledModules().getModuleID (row);

        if (moduleID.isNotEmpty())
            if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showModule (moduleID);
    }

    void deleteKeyPressed (int row) override
    {
        project.getEnabledModules().removeModule (project.getEnabledModules().getModuleID (row));
    }

    void lookAndFeelChanged() override
    {
        setCopyModeButton.setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
        copyPathButton.setColour    (TextButton::buttonColourId, findColour (defaultButtonBackgroundColourId));
        globalPathsButton.setColour (TextButton::buttonColourId, findColour (defaultButtonBackgroundColourId));
    }

private:
    enum
    {
        nameCol = 1,
        versionCol,
        copyCol,
        pathCol
    };

    Project& project;
    ValueTree modulesValueTree;

    ContentViewHeader header  { "Modules", { getIcons().modules, Colours::transparentBlack } };
    ListBox list;
    ListBoxHeader* listHeader;

    TextButton setCopyModeButton  { "Set copy-mode for all modules..." };
    TextButton copyPathButton     { "Set paths for all modules..." };
    TextButton globalPathsButton  { "Enable/disable global path for modules..." };

    std::map<String, var> modulePathClipboard;

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override         { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override           { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

    void itemChanged()
    {
        list.updateContent();
        resized();
        repaint();
    }

    static void setLocalCopyModeForAllModules (Project& project, bool copyLocally)
    {
        auto& modules = project.getEnabledModules();

        for (auto i = modules.getNumModules(); --i >= 0;)
           modules.shouldCopyModuleFilesLocallyValue (modules.getModuleID (i)) = copyLocally;
    }

    void showCopyModeMenu()
    {
        PopupMenu m;

        m.addItem (PopupMenu::Item ("Set all modules to copy locally")
                     .setAction ([&] { setLocalCopyModeForAllModules (project, true); }));

        m.addItem (PopupMenu::Item ("Set all modules to not copy locally")
                     .setAction ([&] { setLocalCopyModeForAllModules (project, false); }));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (setCopyModeButton));
    }

    static void setAllModulesToUseGlobalPaths (Project& project, bool useGlobal)
    {
        auto& modules = project.getEnabledModules();

        for (auto moduleID : modules.getAllModules())
            modules.shouldUseGlobalPathValue (moduleID) = useGlobal;
    }

    static void setSelectedModulesToUseGlobalPaths (Project& project, SparseSet<int> selected, bool useGlobal)
    {
        auto& modules = project.getEnabledModules();

        for (int i = 0; i < selected.size(); ++i)
            modules.shouldUseGlobalPathValue (modules.getModuleID (selected[i])) = useGlobal;
    }

    void showGlobalPathsMenu()
    {
        PopupMenu m;

        m.addItem (PopupMenu::Item ("Set all modules to use global paths")
                    .setAction ([&] { setAllModulesToUseGlobalPaths (project, true); }));

        m.addItem (PopupMenu::Item ("Set all modules to not use global paths")
                    .setAction ([&] { setAllModulesToUseGlobalPaths (project, false); }));

        m.addItem (PopupMenu::Item ("Set selected modules to use global paths")
                    .setEnabled (list.getNumSelectedRows() > 0)
                    .setAction ([&] { setSelectedModulesToUseGlobalPaths (project, list.getSelectedRows(), true); }));

        m.addItem (PopupMenu::Item ("Set selected modules to not use global paths")
                    .setEnabled (list.getNumSelectedRows() > 0)
                    .setAction ([&] { setSelectedModulesToUseGlobalPaths (project, list.getSelectedRows(), false); }));

        m.showMenuAsync (PopupMenu::Options().withTargetComponent (globalPathsButton));
    }

    void showSetPathsMenu()
    {
        PopupMenu m;
        auto moduleToCopy = project.getEnabledModules().getModuleID (list.getSelectedRow());

        if (moduleToCopy.isNotEmpty())
        {
            m.addItem (PopupMenu::Item ("Copy the paths from the module '" + moduleToCopy + "' to all other modules")
                         .setAction ([this, moduleToCopy]
                                     {
                                         auto& modulesList = project.getEnabledModules();

                                         for (Project::ExporterIterator exporter (project); exporter.next();)
                                         {
                                             for (int i = 0; i < modulesList.getNumModules(); ++i)
                                             {
                                                 auto modID = modulesList.getModuleID (i);

                                                 if (modID != moduleToCopy)
                                                     exporter->getPathForModuleValue (modID) = exporter->getPathForModuleValue (moduleToCopy).get();
                                             }
                                         }

                                         list.repaint();
                                     }));

            m.addItem (PopupMenu::Item ("Copy paths from selected module")
                         .setEnabled (list.getNumSelectedRows() == 1)
                         .setAction ([this, moduleToCopy]
                                     {
                                         modulePathClipboard.clear();

                                         for (Project::ExporterIterator exporter (project); exporter.next();)
                                             modulePathClipboard[exporter->getUniqueName()] = exporter->getPathForModuleValue (moduleToCopy).get();

                                         list.repaint();
                                     }));

            m.addItem (PopupMenu::Item ("Paste paths to selected modules")
                         .setEnabled (! modulePathClipboard.empty())
                         .setAction ([this]
                                     {
                                         for (int selectionId = 0; selectionId < list.getNumSelectedRows(); ++selectionId)
                                         {
                                             auto rowNumber = list.getSelectedRow (selectionId);
                                             auto modID = project.getEnabledModules().getModuleID (rowNumber);

                                             for (Project::ExporterIterator exporter (project); exporter.next();)
                                                 exporter->getPathForModuleValue (modID) = modulePathClipboard[exporter->getUniqueName()];
                                         }

                                         list.repaint();
                                     }));
        }
        else
        {
            m.addItem (PopupMenu::Item ("(Select a module in the list above to use this option)")
                         .setEnabled (false));
        }

        m.showMenuAsync (PopupMenu::Options()
                           .withDeletionCheck (*this)
                           .withTargetComponent (copyPathButton));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesInformationComponent)
};
