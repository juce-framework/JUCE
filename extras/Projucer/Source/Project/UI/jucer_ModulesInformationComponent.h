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
class ModulesInformationComponent  : public Component,
                                     private ListBoxModel,
                                     private ValueTree::Listener
{
public:
    ModulesInformationComponent (Project& p)
        : project (p),
          modulesValueTree (p.getEnabledModules().state)
    {
        listHeader = new ListBoxHeader ( { "Module", "Version", "Make Local Copy", "Paths" },
                                        { 0.25f, 0.2f, 0.2f, 0.35f } );
        list.setHeaderComponent (listHeader);
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

        //======================================================================
        auto moduleID = project.getEnabledModules().getModuleID (rowNumber);

        g.drawFittedText (moduleID, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (0) * width)), Justification::centredLeft, 1);

        //======================================================================
        auto version = project.getEnabledModules().getModuleInfo (moduleID).getVersion();
        if (version.isEmpty())
            version = "?";

        g.drawFittedText (version, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (1) * width)), Justification::centredLeft, 1);

        //======================================================================
        auto copyLocally = project.getEnabledModules().shouldCopyModuleFilesLocally (moduleID).getValue() ? "Yes" : "No";

        g.drawFittedText (copyLocally, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (2) * width)), Justification::centredLeft, 1);

        //======================================================================
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

            pathText = paths.joinIntoString (", ");
        }

        g.drawFittedText (pathText, bounds.removeFromLeft (roundToInt (listHeader->getProportionAtIndex (3) * width)), Justification::centredLeft, 1);
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

    void showCopyModeMenu()
    {
        PopupMenu m;
        m.addItem (1, "Set all modules to copy locally");
        m.addItem (2, "Set all modules to not copy locally");

        auto res = m.showAt (&setCopyModeButton);

        if (res != 0)
            project.getEnabledModules().setLocalCopyModeForAllModules (res == 1);
    }

    void showGlobalPathsMenu()
    {
        auto areAnyModulesSelected = (list.getNumSelectedRows() > 0);

        PopupMenu m;
        m.addItem (1, "Set all modules to use global paths");
        m.addItem (2, "Set all modules to not use global paths");
        m.addItem (3, "Set selected modules to use global paths", areAnyModulesSelected);
        m.addItem (4, "Set selected modules to not use global paths", areAnyModulesSelected);

        auto res = m.showAt (&globalPathsButton);

        if (res != 0)
        {
            auto enableGlobalPaths = (res % 2 == 1);

            auto& moduleList = project.getEnabledModules();

            if (res < 3)
            {
                auto moduleIDs = moduleList.getAllModules();

                for (auto id : moduleIDs)
                    moduleList.getShouldUseGlobalPathValue (id).setValue (enableGlobalPaths);
            }
            else
            {
                auto selected = list.getSelectedRows();

                for (int i = 0; i < selected.size(); ++i)
                    moduleList.getShouldUseGlobalPathValue (moduleList.getModuleID (selected[i])).setValue (enableGlobalPaths);
            }
        }
    }

    void showSetPathsMenu()
    {
        enum
        {
            copyPathsToAllModulesID = 1,
            copyPathsID,
            pastePathsID
        };

        auto& moduleList = project.getEnabledModules();
        auto moduleToCopy = moduleList.getModuleID (list.getSelectedRow());

        if (moduleToCopy.isNotEmpty())
        {
            PopupMenu m;
            m.addItem (copyPathsToAllModulesID, "Copy the paths from the module '" + moduleToCopy + "' to all other modules");
            m.addItem (copyPathsID, "Copy paths from selected module", list.getNumSelectedRows() == 1);
            m.addItem (pastePathsID, "Paste paths to selected modules", ! modulePathClipboard.empty());

            int res = m.showAt (&copyPathButton);

            if (res == copyPathsToAllModulesID)
            {
                for (Project::ExporterIterator exporter (project); exporter.next();)
                {
                    for (int i = 0; i < moduleList.getNumModules(); ++i)
                    {
                        auto modID = moduleList.getModuleID (i);

                        if (modID != moduleToCopy)
                            exporter->getPathForModuleValue (modID) = exporter->getPathForModuleValue (moduleToCopy).getValue();
                    }
                }
            }
            else if (res == copyPathsID)
            {
                 modulePathClipboard.clear();

                 for (Project::ExporterIterator exporter (project); exporter.next();)
                     modulePathClipboard[exporter->getName()] = exporter->getPathForModuleValue (moduleToCopy).getValue();
            }
            else if (res == pastePathsID)
            {
                for (int selectionId = 0; selectionId < list.getNumSelectedRows(); ++selectionId)
                {
                    auto rowNumber = list.getSelectedRow (selectionId);
                    auto modID = moduleList.getModuleID (rowNumber);

                    for (Project::ExporterIterator exporter (project); exporter.next();)
                        exporter->getPathForModuleValue (modID) = modulePathClipboard[exporter->getName()];
                }
            }

            list.repaint();
        }
        else
        {
            PopupMenu m;
            m.addItem (1, "(Select a module in the list above to use this option)", false);

            m.showAt (&copyPathButton);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesInformationComponent)
};
