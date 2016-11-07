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

#ifndef JUCER_NEWPROJECTWIZARDCOMPONENT_H_INCLUDED
#define JUCER_NEWPROJECTWIZARDCOMPONENT_H_INCLUDED


class ModulesFolderPathBox  : public Component,
                              private ButtonListener,
                              private ComboBoxListener
{
public:
    ModulesFolderPathBox (File initialFileOrDirectory)
        : currentPathBox ("currentPathBox"),
          openFolderButton (TRANS("...")),
          modulesLabel (String(), TRANS("Modules Folder") + ":")
    {
        if (initialFileOrDirectory == File())
            initialFileOrDirectory = findDefaultModulesFolder();

        setModulesFolder (initialFileOrDirectory);

        addAndMakeVisible (currentPathBox);
        currentPathBox.setEditableText (true);
        currentPathBox.addListener (this);

        addAndMakeVisible (openFolderButton);
        openFolderButton.addListener (this);
        openFolderButton.setTooltip (TRANS ("Select JUCE modules folder"));

        addAndMakeVisible (modulesLabel);
        modulesLabel.attachToComponent (&currentPathBox, true);
    }

    void resized() override
    {
        Rectangle<int> r = getLocalBounds();

        modulesLabel.setBounds (r.removeFromLeft (110));

        openFolderButton.setBounds (r.removeFromRight (40));
        r.removeFromRight (5);

        currentPathBox.setBounds (r);
    }

    static bool selectJuceFolder (File& result)
    {
        for (;;)
        {
            FileChooser fc ("Select your JUCE modules folder...",
                            findDefaultModulesFolder(),
                            "*");

            if (! fc.browseForDirectory())
                return false;

            if (isJuceModulesFolder (fc.getResult()))
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

    void buttonClicked (Button*) override
    {
        selectJuceFolder();
    }

    void comboBoxChanged (ComboBox*) override
    {
        setModulesFolder (File::getCurrentWorkingDirectory().getChildFile (currentPathBox.getText()));
    }

    File modulesFolder;

private:
    ComboBox currentPathBox;
    TextButton openFolderButton;
    Label modulesLabel;
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

        for (int i = 0; i < types.size(); ++i)
        {
            const ProjectExporter::ExporterTypeInfo& type = types.getReference (i);
            platforms.add (new PlatformType (type.getIcon(), type.name));
        }

        listBox.setRowHeight (35);
        listBox.setModel (this);
        listBox.setOpaque (false);
        listBox.setMultipleSelectionEnabled (true);
        listBox.setClickingTogglesRowSelection (true);
        listBox.setColour (ListBox::backgroundColourId, Colours::white.withAlpha (0.0f));
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
        if (PlatformType* platform = platforms[rowNumber])
        {
            if (rowIsSelected)
                g.fillAll (Colour (0x99f29000));

            Rectangle<float> dotSelect ((float) height, (float) height);
            dotSelect.reduce (12, 12);

            g.setColour (Colour (0x33ffffff));
            g.fillEllipse (dotSelect);

            if (rowIsSelected)
            {
                const float tx = dotSelect.getCentreX();
                const float ty = dotSelect.getCentreY() + 1.0f;

                Path tick;
                tick.startNewSubPath (tx - 5.0f, ty - 6.0f);
                tick.lineTo (tx, ty);
                tick.lineTo (tx + 8.0f, ty - 13.0f);

                g.setColour (Colours::white);
                g.strokePath (tick, PathStrokeType (3.0f));
            }

            g.setColour (Colours::black);
            g.drawImageWithin (platform->icon, 40, 0, height, height, RectanglePlacement::stretchToFit);
            g.drawText (platform->name, 90, 0, width, height, Justification::left);
        }
    }

    void selectedRowsChanged (int) override
    {
        selectDefaultExporterIfNoneSelected();
    }

private:
    struct PlatformType
    {
        PlatformType (const Image& platformIcon, const String& platformName)
            : icon (platformIcon), name (platformName)
        {
        }

        Image icon;
        String name;
    };

    ListBox listBox;
    OwnedArray<PlatformType> platforms;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTargetsComp)
};



//==============================================================================
/**
    The Component for project creation.
    Features a file browser to select project destination and
    a list box of platform targets to generate.
*/
class WizardComp  : public Component,
                    private ButtonListener,
                    private ComboBoxListener,
                    private TextEditorListener,
                    private FileBrowserListener
{
public:
    WizardComp()
        : platformTargets(),
          projectName (TRANS("Project name")),
          nameLabel (String(), TRANS("Project Name") + ":"),
          typeLabel (String(), TRANS("Project Type") + ":"),
          fileBrowser (FileBrowserComponent::saveMode
                         | FileBrowserComponent::canSelectDirectories
                         | FileBrowserComponent::doNotClearFileNameOnRootChange,
                       NewProjectWizardClasses::getLastWizardFolder(), nullptr, nullptr),
          fileOutline (String(), TRANS("Project Folder") + ":"),
          targetsOutline (String(), TRANS("Target Platforms") + ":"),
          createButton (TRANS("Create") + "..."),
          cancelButton (TRANS("Cancel")),
          modulesPathBox (findDefaultModulesFolder())
    {
        setOpaque (false);

        addChildAndSetID (&projectName, "projectName");
        projectName.setText ("NewProject");
        projectName.setBounds ("120, 34, parent.width / 2 - 10, top + 22");
        nameLabel.attachToComponent (&projectName, true);
        projectName.addListener (this);

        addChildAndSetID (&projectType, "projectType");
        projectType.addItemList (getWizardNames(), 1);
        projectType.setSelectedId (1, dontSendNotification);
        projectType.setBounds ("120, projectName.bottom + 4, projectName.right, top + 22");
        typeLabel.attachToComponent (&projectType, true);
        projectType.addListener (this);

        addChildAndSetID (&fileOutline, "fileOutline");
        fileOutline.setColour (GroupComponent::outlineColourId, Colours::black.withAlpha (0.2f));
        fileOutline.setTextLabelPosition (Justification::centred);
        fileOutline.setBounds ("30, projectType.bottom + 20, projectType.right, parent.height - 30");

        addChildAndSetID (&targetsOutline, "targetsOutline");
        targetsOutline.setColour (GroupComponent::outlineColourId, Colours::black.withAlpha (0.2f));
        targetsOutline.setTextLabelPosition (Justification::centred);
        targetsOutline.setBounds ("fileOutline.right + 20, projectType.bottom + 20, parent.width - 30, parent.height - 70");

        addChildAndSetID (&platformTargets, "platformTargets");
        platformTargets.setBounds ("targetsOutline.left + 15, projectType.bottom + 45, parent.width - 40, parent.height - 90");

        addChildAndSetID (&fileBrowser, "fileBrowser");
        fileBrowser.setBounds ("fileOutline.left + 10, fileOutline.top + 20, fileOutline.right - 10, fileOutline.bottom - 32");
        fileBrowser.setFilenameBoxLabel ("Folder:");
        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
        fileBrowser.addListener (this);

        addChildAndSetID (&createButton, "createButton");
        createButton.setBounds ("right - 130, bottom - 34, parent.width - 30, parent.height - 30");
        createButton.addListener (this);

        addChildAndSetID (&cancelButton, "cancelButton");
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));
        cancelButton.setBounds ("right - 130, createButton.top, createButton.left - 10, createButton.bottom");
        cancelButton.addListener (this);

        addChildAndSetID (&modulesPathBox, "modulesPathBox");
        modulesPathBox.setBounds ("targetsOutline.left, targetsOutline.top - 45, targetsOutline.right, targetsOutline.top - 20");


        updateCustomItems();
        updateCreateButton();
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> rect = getLocalBounds().reduced (10, 10);

        g.setColour (Colours::white.withAlpha (0.3f));
        g.fillRect (rect);
        g.fillRect (rect.reduced (10, 10));
    }

    void buttonClicked (Button* b) override
    {
        if (b == &createButton)
        {
            createProject();
        }
        else if (b == &cancelButton)
        {
            returnToTemplatesPage();
        }
    }

    void returnToTemplatesPage()
    {
        if (SlidingPanelComponent* parent = findParentComponentOfClass<SlidingPanelComponent>())
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
        MainWindow* mw = Component::findParentComponentOfClass<MainWindow>();
        jassert (mw != nullptr);

        ScopedPointer<NewProjectWizardClasses::NewProjectWizard> wizard (createWizard());

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

            wizard->modulesFolder = modulesPathBox.modulesFolder;

            if (! isJuceModulesFolder (wizard->modulesFolder))
                if (! wizard->selectJuceFolder())
                    return;

            ScopedPointer<Project> project (wizard->runWizard (*this, projectName.getText(),
                                                               fileBrowser.getSelectedFile (0)));

            if (project != nullptr)
                mw->setProject (project.release());
        }
    }

    void updateCustomItems()
    {
        customItems.clear();

        ScopedPointer<NewProjectWizardClasses::NewProjectWizard> wizard (createWizard());

        if (wizard != nullptr)
            wizard->addSetupItems (*this, customItems);
    }

    void comboBoxChanged (ComboBox*) override
    {
        updateCustomItems();
    }

    void textEditorTextChanged (TextEditor&) override
    {
        updateCreateButton();
        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
    }

    void selectionChanged() override {}

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&) override {}

    void browserRootChanged (const File&) override
    {
        fileBrowser.setFileName (File::createLegalFileName (projectName.getText()));
    }

    ComboBox projectType;
    PlatformTargetsComp platformTargets;

private:
    TextEditor projectName;
    Label nameLabel, typeLabel;
    FileBrowserComponent fileBrowser;
    GroupComponent fileOutline;
    GroupComponent targetsOutline;
    TextButton createButton, cancelButton;
    OwnedArray<Component> customItems;
    ModulesFolderPathBox modulesPathBox;

    NewProjectWizardClasses::NewProjectWizard* createWizard()
    {
        return createWizardType (projectType.getSelectedItemIndex());
    }

    void updateCreateButton()
    {
        createButton.setEnabled (projectName.getText().trim().isNotEmpty());
    }
};


#endif   // JUCER_NEWPROJECTWIZARDCOMPONENT_H_INCLUDED
