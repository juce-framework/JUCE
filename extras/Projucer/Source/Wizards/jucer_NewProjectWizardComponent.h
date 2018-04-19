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
class ModulesFolderPathBox  : public Component
{
public:
    ModulesFolderPathBox (File initialFileOrDirectory)
        : currentPathBox ("currentPathBox"),
          openFolderButton (TRANS("...")),
          modulesLabel (String(), TRANS("Modules Folder") + ":"),
          useGlobalPathsToggle ("Use global module path")
    {
        if (initialFileOrDirectory == File())
            initialFileOrDirectory = EnabledModuleList::findGlobalModulesFolder();

        setModulesFolder (initialFileOrDirectory);

        addAndMakeVisible (currentPathBox);
        currentPathBox.setEditableText (true);
        currentPathBox.onChange = [this] { setModulesFolder (File::getCurrentWorkingDirectory()
                                                                  .getChildFile (currentPathBox.getText())); };

        addAndMakeVisible (openFolderButton);
        openFolderButton.setTooltip (TRANS ("Select JUCE modules folder"));
        openFolderButton.onClick = [this] { selectJuceFolder(); };

        addAndMakeVisible (modulesLabel);
        modulesLabel.attachToComponent (&currentPathBox, true);

        addAndMakeVisible (useGlobalPathsToggle);
        useGlobalPathsToggle.setToggleState (true, sendNotification);
        useGlobalPathsToggle.onClick = [this]
        {
            isUsingGlobalPaths = useGlobalPathsToggle.getToggleState();

            currentPathBox.setEnabled   (! isUsingGlobalPaths);
            openFolderButton.setEnabled (! isUsingGlobalPaths);
            modulesLabel.setEnabled     (! isUsingGlobalPaths);
        };
    }

    void resized() override
    {
        auto b = getLocalBounds();

        auto topSlice = b.removeFromTop (b.getHeight() / 2);

        openFolderButton.setBounds (topSlice.removeFromRight (30));
        modulesLabel.setBounds (topSlice.removeFromLeft (110));
        currentPathBox.setBounds (topSlice);

        b.removeFromTop (5);
        useGlobalPathsToggle.setBounds (b.translated (20, 0));
    }

    static bool selectJuceFolder (File& result)
    {
        for (;;)
        {
            FileChooser fc ("Select your JUCE modules folder...",
                            EnabledModuleList::findGlobalModulesFolder(),
                            "*");

            if (! fc.browseForDirectory())
                return false;

            if (isJUCEModulesFolder (fc.getResult()))
            {
                result = fc.getResult();
                return true;
            }

            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         "Not a valid JUCE modules folder!",
                                         "Please select the folder containing your juce_* modules!\n\n"
                                         "This is required so that the new project can be given some essential core modules.");
        }
    }

    void selectJuceFolder()
    {
        File result;

        if (selectJuceFolder (result))
            setModulesFolder (result);
    }

    void setModulesFolder (const File& newFolder)
    {
        if (modulesFolder != newFolder)
        {
            modulesFolder = newFolder;
            currentPathBox.setText (modulesFolder.getFullPathName(), dontSendNotification);
        }
    }

    File modulesFolder;
    bool isUsingGlobalPaths;

private:
    ComboBox currentPathBox;
    TextButton openFolderButton;
    Label modulesLabel;
    ToggleButton useGlobalPathsToggle;
};


/** The target platforms chooser for the chosen template. */
class PlatformTargetsComp    : public Component,
                               private ListBoxModel
{
public:
    PlatformTargetsComp()
    {
        setOpaque (false);

        const Array<ProjectExporter::ExporterTypeInfo> types (ProjectExporter::getExporterTypes());

        for (auto& type : types)
        {
            platforms.add (new PlatformType { type.getIcon(), type.name });
            addAndMakeVisible (toggles.add (new ToggleButton (String())));
        }

        listBox.setRowHeight (30);
        listBox.setModel (this);
        listBox.setOpaque (false);
        listBox.setMultipleSelectionEnabled (true);
        listBox.setClickingTogglesRowSelection (true);
        listBox.setColour (ListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (listBox);

        selectDefaultExporterIfNoneSelected();
    }

    StringArray getSelectedPlatforms() const
    {
        StringArray list;

        for (int i = 0; i < platforms.size(); ++i)
            if (listBox.isRowSelected (i))
                list.add (platforms.getUnchecked(i)->name);

        return list;
    }

    void selectDefaultExporterIfNoneSelected()
    {
        if (listBox.getNumSelectedRows() == 0)
        {
            for (int i = platforms.size(); --i >= 0;)
            {
                if (platforms.getUnchecked(i)->name == ProjectExporter::getCurrentPlatformExporterName())
                {
                    listBox.selectRow (i);
                    break;
                }
            }
        }
    }

    void resized() override
    {
        listBox.setBounds (getLocalBounds());
    }

    int getNumRows() override
    {
        return platforms.size();
    }

    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        ignoreUnused (width);

        if (auto* platform = platforms[rowNumber])
        {
            auto bounds = getLocalBounds().withHeight (height).withTrimmedBottom (1);
            g.setColour (findColour (rowNumber % 2 == 0 ? widgetBackgroundColourId
                                                        : secondaryWidgetBackgroundColourId));
            g.fillRect (bounds);

            bounds.removeFromLeft (10);

            auto toggleBounds = bounds.removeFromLeft (height);
            drawToggle (g, toggleBounds, rowIsSelected);

            auto iconBounds = bounds.removeFromLeft (height).reduced (5);

            g.drawImageWithin (platform->icon, iconBounds.getX(), iconBounds.getY(), iconBounds.getWidth(),
                               iconBounds.getHeight(), RectanglePlacement::fillDestination);

            bounds.removeFromLeft (10);
            g.setColour (findColour (widgetTextColourId));
            g.drawFittedText (platform->name, bounds, Justification::centredLeft, 1);
        }
    }

    void selectedRowsChanged (int) override
    {
        selectDefaultExporterIfNoneSelected();
    }

private:
    struct PlatformType
    {
        Image icon;
        String name;
    };

    void drawToggle (Graphics& g, Rectangle<int> bounds, bool isToggled)
    {
        auto sideLength = jmin (bounds.getWidth(), bounds.getHeight());

        bounds = bounds.withSizeKeepingCentre (sideLength, sideLength).reduced (4);

        g.setColour (findColour (ToggleButton::tickDisabledColourId));
        g.drawRoundedRectangle (bounds.toFloat(), 2.0f, 1.0f);

        if (isToggled)
        {
            g.setColour (findColour (ToggleButton::tickColourId));
            const auto tick = getTickShape (0.75f);
            g.fillPath (tick, tick.getTransformToScaleToFit (bounds.reduced (4, 5).toFloat(), false));
        }
    }

    Path getTickShape (float height)
    {
        static const unsigned char pathData[] = { 110,109,32,210,202,64,126,183,148,64,108,39,244,247,64,245,76,124,64,108,178,131,27,65,246,76,252,64,108,175,242,4,65,246,76,252,
            64,108,236,5,68,65,0,0,160,180,108,240,150,90,65,21,136,52,63,108,48,59,16,65,0,0,32,65,108,32,210,202,64,126,183,148,64, 99,101,0,0 };

        Path path;
        path.loadPathFromData (pathData, sizeof (pathData));
        path.scaleToFit (0, 0, height * 2.0f, height, true);

        return path;
    }

    ListBox listBox;
    OwnedArray<PlatformType> platforms;
    OwnedArray<ToggleButton> toggles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTargetsComp)
};



//==============================================================================
/**
    The Component for project creation.
    Features a file browser to select project destination and
    a list box of platform targets to generate.
*/
class WizardComp  : public Component,
                    private FileBrowserListener
{
public:
    WizardComp()
        : platformTargets(),
          projectName (TRANS("Project name")),
          modulesPathBox (EnabledModuleList::findGlobalModulesFolder())
    {
        setOpaque (false);

        addChildAndSetID (&projectName, "projectName");
        projectName.setText ("NewProject");
        nameLabel.attachToComponent (&projectName, true);
        projectName.onTextChange = [this]
        {
            updateCreateButton();
            fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
        };

        addChildAndSetID (&projectType, "projectType");
        projectType.addItemList (getWizardNames(), 1);
        projectType.setSelectedId (1, dontSendNotification);
        typeLabel.attachToComponent (&projectType, true);
        projectType.onChange = [this] { updateFileCreationTypes(); };

        addChildAndSetID (&fileOutline, "fileOutline");
        fileOutline.setColour (GroupComponent::outlineColourId, Colours::black.withAlpha (0.2f));
        fileOutline.setTextLabelPosition (Justification::centred);

        addChildAndSetID (&targetsOutline, "targetsOutline");
        targetsOutline.setColour (GroupComponent::outlineColourId, Colours::black.withAlpha (0.2f));
        targetsOutline.setTextLabelPosition (Justification::centred);

        addChildAndSetID (&platformTargets, "platformTargets");

        addChildAndSetID (&fileBrowser, "fileBrowser");
        fileBrowser.setFilenameBoxLabel ("Folder:");
        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
        fileBrowser.addListener (this);

        addChildAndSetID (&createButton, "createButton");
        createButton.onClick = [this] { createProject(); };

        addChildAndSetID (&cancelButton, "cancelButton");
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));
        cancelButton.onClick = [this] { returnToTemplatesPage(); };

        addChildAndSetID (&modulesPathBox, "modulesPathBox");

        addChildAndSetID (&filesToCreate, "filesToCreate");
        filesToCreateLabel.attachToComponent (&filesToCreate, true);

        updateFileCreationTypes();
        updateCreateButton();

        lookAndFeelChanged();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

    void resized() override
    {
        auto r = getLocalBounds();

        auto left = r.removeFromLeft (getWidth() / 2).reduced (15);
        auto right = r.reduced (15);

        projectName.setBounds (left.removeFromTop (22).withTrimmedLeft (120));
        left.removeFromTop (20);
        projectType.setBounds (left.removeFromTop (22).withTrimmedLeft (120));
        left.removeFromTop (20);
        fileOutline.setBounds (left);
        fileBrowser.setBounds (left.reduced (25));

        auto buttons = right.removeFromBottom (30);
        right.removeFromBottom (10);
        createButton.setBounds (buttons.removeFromRight (130));
        buttons.removeFromRight (10);
        cancelButton.setBounds (buttons.removeFromRight (130));

        filesToCreate.setBounds (right.removeFromTop (22).withTrimmedLeft (150));
        right.removeFromTop (20);
        modulesPathBox.setBounds (right.removeFromTop (50));
        right.removeFromTop (20);

        targetsOutline.setBounds (right);
        platformTargets.setBounds (right.reduced (25));
    }

    void returnToTemplatesPage()
    {
        if (auto* parent = findParentComponentOfClass<SlidingPanelComponent>())
        {
            if (parent->getNumTabs() > 0)
                parent->goToTab (parent->getCurrentTabIndex() - 1);
        }
        else
        {
            jassertfalse;
        }
    }

    void createProject()
    {
        auto* mw = Component::findParentComponentOfClass<MainWindow>();
        jassert (mw != nullptr);

        std::unique_ptr<NewProjectWizardClasses::NewProjectWizard> wizard = createWizard();

        if (wizard != nullptr)
        {
            Result result (wizard->processResultsFromSetupItems (*this));

            if (result.failed())
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             TRANS("Create Project"),
                                             result.getErrorMessage());
                return;
            }


            wizard->modulesFolder = modulesPathBox.isUsingGlobalPaths ? File (getAppSettings().getStoredPath (Ids::defaultJuceModulePath).toString())
                                                                      : modulesPathBox.modulesFolder;

            if (! isJUCEModulesFolder (wizard->modulesFolder))
            {
                if (modulesPathBox.isUsingGlobalPaths)
                    AlertWindow::showMessageBox (AlertWindow::AlertIconType::WarningIcon, "Invalid Global Path",
                                                 "Your global JUCE module search path is invalid. Please select the folder containing your JUCE modules "
                                                 "to set as the default path.");

                if (! wizard->selectJuceFolder())
                    return;

                if (modulesPathBox.isUsingGlobalPaths)
                    getAppSettings().getStoredPath (Ids::defaultJuceModulePath).setValue (wizard->modulesFolder.getFullPathName());
            }

            auto projectDir = fileBrowser.getSelectedFile (0);
            std::unique_ptr<Project> project (wizard->runWizard (*this, projectName.getText(),
                                                               projectDir,
                                                               modulesPathBox.isUsingGlobalPaths));

            if (project != nullptr)
            {
                mw->setProject (project.release());
                getAppSettings().lastWizardFolder = projectDir.getParentDirectory();
            }
        }
    }

    void updateFileCreationTypes()
    {
        StringArray items;

        std::unique_ptr<NewProjectWizardClasses::NewProjectWizard> wizard = createWizard();

        if (wizard != nullptr)
            items = wizard->getFileCreationOptions();

        filesToCreate.clear();
        filesToCreate.addItemList (items, 1);
        filesToCreate.setSelectedId (1, dontSendNotification);
    }

    void selectionChanged() override {}

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&) override {}

    void browserRootChanged (const File&) override
    {
        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
    }

    int getFileCreationComboID() const
    {
        return filesToCreate.getSelectedItemIndex();
    }

    ComboBox projectType, filesToCreate;
    PlatformTargetsComp platformTargets;

private:
    TextEditor projectName;

    Label nameLabel { {}, TRANS("Project Name") + ":" };
    Label typeLabel { {}, TRANS("Project Type") + ":" };
    Label filesToCreateLabel { {}, TRANS("Files to Auto-Generate") + ":" };

    FileBrowserComponent fileBrowser { FileBrowserComponent::saveMode
                                        | FileBrowserComponent::canSelectDirectories
                                        | FileBrowserComponent::doNotClearFileNameOnRootChange,
                                       NewProjectWizardClasses::getLastWizardFolder(), nullptr, nullptr };

    GroupComponent fileOutline { {}, TRANS("Project Folder") + ":" };
    GroupComponent targetsOutline { {}, TRANS("Target Platforms") + ":" };

    TextButton createButton { TRANS("Create") + "..." };
    TextButton cancelButton { TRANS("Cancel") };
    ModulesFolderPathBox modulesPathBox;

    std::unique_ptr<NewProjectWizardClasses::NewProjectWizard> createWizard()
    {
        return createWizardType (projectType.getSelectedItemIndex());
    }

    void updateCreateButton()
    {
        createButton.setEnabled (projectName.getText().trim().isNotEmpty());
    }

    void lookAndFeelChanged() override
    {
        projectName.setColour (TextEditor::backgroundColourId, findColour (backgroundColourId));
        projectName.setColour (TextEditor::textColourId, findColour (defaultTextColourId));
        projectName.setColour (TextEditor::outlineColourId, findColour (defaultTextColourId));
        projectName.applyFontToAllText (projectName.getFont());

        fileBrowser.resized();
    }
};
