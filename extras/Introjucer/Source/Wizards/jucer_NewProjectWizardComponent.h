/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef NEWPROJECTWIZARDCOMPONENTS_H_INCLUDED
#define NEWPROJECTWIZARDCOMPONENTS_H_INCLUDED


/** The target platforms chooser for the chosen template. */
class PlatformTargetsComp       : public Component,
                                private ListBoxModel
{
public:

    PlatformTargetsComp()
    {
        setOpaque (false);

        addAndMakeVisible (listBox);

        listBox.setRowHeight (360 / getNumRows());
        listBox.setModel (this);
        listBox.setOpaque (false);
        listBox.setMultipleSelectionEnabled (true);
        listBox.setClickingTogglesRowSelection (true);
        listBox.setColour (ListBox::ColourIds::backgroundColourId, Colours::white.withAlpha (0.0f));

        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconXcode_png, BinaryData::projectIconXcode_pngSize), "Create a new XCode target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconXcodeIOS_png, BinaryData::projectIconXcodeIOS_pngSize), "Create a new XCode IOS target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconVisualStudio13_png, BinaryData::projectIconVisualStudio13_pngSize), "Create a new Visual Studio 2013 target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconVisualStudio12_png, BinaryData::projectIconVisualStudio12_pngSize), "Create a new Visual Studio 2012 target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconVisualStudio10_png, BinaryData::projectIconVisualStudio10_pngSize), "Create a new Visual Studio 2010 target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconVisualStudio08_png, BinaryData::projectIconVisualStudio08_pngSize), "Create a new Visual Studio 2008 target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconVisualStudio05_png, BinaryData::projectIconVisualStudio05_pngSize), "Create a new Visual Studio 2005 target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconAndroid_png, BinaryData::projectIconAndroid_pngSize), "Create a new Android target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconCodeblocks_png, BinaryData::projectIconCodeblocks_pngSize), "Create a new Codeblocks target"));
        platforms.add (new PlatformType (ImageCache::getFromMemory (BinaryData::projectIconLinuxMakefile_png, BinaryData::projectIconLinuxMakefile_pngSize), "Create a new linux makefile target"));


    }

    ~PlatformTargetsComp()
    {
    }

    void resized()
    {
        listBox.setBounds (getLocalBounds());
    }


    // these add the ListBoxModel virtual functions
    int getNumRows()
    {
        return 10;
    }

    void paintListBoxItem (int rowNumber, Graphics& g,
                           int width, int height, bool rowIsSelected)
    {
        Rectangle<float> dotSelect = Rectangle<float> (0, 0, height, height);
        dotSelect.reduce (12, 12);

        g.setColour (Colours::white);
        g.drawEllipse (dotSelect, 1);

        if (rowIsSelected)
        {
            g.fillAll (Colour(243, 145, 0));
            g.fillEllipse (dotSelect);
        }

        g.drawImageWithin (platforms.getUnchecked(rowNumber)->icon, 40, 0, height, height, RectanglePlacement::stretchToFit);
        g.setColour (Colours::black);
        g.drawText (platforms.getUnchecked (rowNumber)->name, 90, 0, width, height, Justification::left);
    }


private:

    struct PlatformType {
        PlatformType (const Image& platformIcon, const String& platformName){
            icon = platformIcon;
            name = platformName;
        }
        Image icon;
        String name;
    };

    ListBox listBox;

    OwnedArray<PlatformType> platforms;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlatformTargetsComp)

};



/**
 The Component for project creation.
 Features a file browser to select project destination and
 a list box of platform targets to generate.
*/
//==============================================================================
class WizardComp  : public Component,
                    private ButtonListener,
                    private ComboBoxListener,
                    private TextEditorListener
{
public:
    WizardComp()
        : projectName (TRANS("Project name")),
          nameLabel (String::empty, TRANS("Project Name") + ":"),
          typeLabel (String::empty, TRANS("Project Type") + ":"),
          fileBrowser (FileBrowserComponent::saveMode | FileBrowserComponent::canSelectDirectories,
                       NewProjectWizardClasses::getLastWizardFolder(), nullptr, nullptr),
          fileOutline (String::empty, TRANS("Project Folder") + ":"),
          targetsOutline (String::empty, TRANS("Project Targets") + ":"),
          createButton (TRANS("Create") + "..."),
          cancelButton (TRANS("Cancel")),
          platformTargets()
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

        addChildAndSetID (&createButton, "createButton");
        createButton.setBounds ("right - 130, bottom - 34, parent.width - 30, parent.height - 30");
        createButton.addListener (this);

        addChildAndSetID (&cancelButton, "cancelButton");
        cancelButton.addShortcut (KeyPress (KeyPress::escapeKey));
        cancelButton.setBounds ("right - 130, createButton.top, createButton.left - 10, createButton.bottom");
        cancelButton.addListener (this);

        updateCustomItems();
        updateCreateButton();
    }

    void paint (Graphics& g) override
    {
        Rectangle<int> rect = getLocalBounds().reduced (10, 10);

        g.setColour (Colours::white.withAlpha(0.3f));
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
            // returns to template icon page on cancel
            SlidingPanelComponent* parent = findParentComponentOfClass<SlidingPanelComponent>();

            if (parent->getNumTabs() > 0) parent->goToTab (parent->getCurrentTabIndex() - 1);
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

            if (! wizard->selectJuceFolder())
                return;

            ScopedPointer<Project> project (wizard->runWizard (mw, projectName.getText(),
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

    // projectType box is public so it can be set by the introjucer front page icons
    ComboBox projectType;

    private:
    TextEditor projectName;
    Label nameLabel, typeLabel;
    FileBrowserComponent fileBrowser;
    GroupComponent fileOutline;
    GroupComponent targetsOutline;
    TextButton createButton, cancelButton;
    OwnedArray<Component> customItems;
    PlatformTargetsComp platformTargets;

    NewProjectWizardClasses::NewProjectWizard* createWizard()
    {
        return createWizardType (projectType.getSelectedItemIndex());
    }

    void updateCreateButton()
    {
        createButton.setEnabled (projectName.getText().trim().isNotEmpty());
    }
};


//==============================================================================
static int getNumWizards()
{
    return 5;
}

static NewProjectWizardClasses::NewProjectWizard* createWizardType (int index)
{
    switch (index)
    {
        case 0:     return new NewProjectWizardClasses::GUIAppWizard();
        case 1:     return new NewProjectWizardClasses::ConsoleAppWizard();
        case 2:     return new NewProjectWizardClasses::AudioPluginAppWizard();
        case 3:     return new NewProjectWizardClasses::StaticLibraryWizard();
        case 4:     return new NewProjectWizardClasses::DynamicLibraryWizard();
        default:    jassertfalse; break;
    }

    return nullptr;
}

static StringArray getWizardNames()
{
    StringArray s;

    for (int i = 0; i < getNumWizards(); ++i)
    {
        ScopedPointer<NewProjectWizardClasses::NewProjectWizard> wiz (createWizardType (i));
        s.add (wiz->getName());
    }

    return s;
}



#endif  // NEWPROJECTWIZARDCOMPONENTS_H_INCLUDED
