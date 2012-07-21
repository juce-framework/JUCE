/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_MODULESPANEL_H_D0C12034__
#define __JUCER_MODULESPANEL_H_D0C12034__


class ModulesPanel  : public PropertyComponent,
                      public FilenameComponentListener,
                      public ButtonListener
{
public:
    ModulesPanel (Project& project_)
        : PropertyComponent ("Modules", 500),
          project (project_),
          modulesLocation ("modules", ModuleList::getLocalModulesFolder (&project),
                           true, true, false, "*", String::empty,
                           "Select a folder containing your JUCE modules..."),
          modulesLabel (String::empty, "Module source folder:"),
          updateModulesButton ("Check for module updates..."),
          moduleListBox (moduleList),
          copyingMessage (project_, moduleList)
    {
        moduleList.rescan (ModuleList::getLocalModulesFolder (&project));

        addAndMakeVisible (&modulesLocation);
        modulesLocation.addListener (this);

        modulesLabel.attachToComponent (&modulesLocation, true);

        addAndMakeVisible (&updateModulesButton);
        updateModulesButton.addListener (this);

        moduleListBox.setOwner (this);
        addAndMakeVisible (&moduleListBox);

        addAndMakeVisible (&copyingMessage);
        copyingMessage.refresh();
    }

    void filenameComponentChanged (FilenameComponent*)
    {
        moduleList.rescan (modulesLocation.getCurrentFile());
        modulesLocation.setCurrentFile (moduleList.getModulesFolder(), false, false);
        ModuleList::setLocalModulesFolder (moduleList.getModulesFolder());
        moduleListBox.refresh();
    }

    void buttonClicked (Button*)
    {
        JuceUpdater::show (moduleList, getTopLevelComponent(), "");

        filenameComponentChanged (nullptr);
    }

    bool isEnabled (const ModuleList::Module* m) const
    {
        return project.isModuleEnabled (m->uid);
    }

    void setEnabled (const ModuleList::Module* m, bool enable)
    {
        if (enable)
            project.addModule (m->uid, true);
        else
            project.removeModule (m->uid);

        refresh();
    }

    bool areDependenciesMissing (const ModuleList::Module* m)
    {
        return moduleList.getExtraDependenciesNeeded (project, *m).size() > 0;
    }

    void selectionChanged (const ModuleList::Module* selectedModule)
    {
        settings = nullptr;

        if (selectedModule != nullptr)
            addAndMakeVisible (settings = new ModuleSettingsPanel (project, moduleList, selectedModule->uid));

        copyingMessage.refresh();
        resized();
    }

    void refresh()
    {
        moduleListBox.refresh();

        if (settings != nullptr)
            settings->refreshAll();

        copyingMessage.refresh();
    }

    void paint (Graphics& g) // (overridden to avoid drawing the name)
    {
        getLookAndFeel().drawPropertyComponentBackground (g, getWidth(), getHeight(), *this);
    }

    void resized()
    {
        modulesLocation.setBounds (150, 3, getWidth() - 180 - 150, 25);
        updateModulesButton.setBounds (modulesLocation.getRight() + 6, 3, getWidth() - modulesLocation.getRight() - 12, 25);
        moduleListBox.setBounds (4, 31, getWidth() / 3 - 4, getHeight() - 63);
        copyingMessage.setBounds (4, getHeight() - 30, getWidth() - 8, getHeight() - 31);

        if (settings != nullptr)
            settings->setBounds (moduleListBox.getRight() + 5, moduleListBox.getY(),
                                 getWidth() - (moduleListBox.getRight() + 5), moduleListBox.getHeight());
    }

    //==============================================================================
    class ModuleSelectionListBox    : public ListBox,
                                      public ListBoxModel
    {
    public:
        ModuleSelectionListBox (ModuleList& list_)
            : list (list_), owner (nullptr)
        {
            setColour (ListBox::backgroundColourId, Colours::white.withAlpha (0.4f));
            setTooltip ("Use this list to select which modules should be included in your app.\n"
                        "Any modules which have missing dependencies will be shown in red.");
        }

        void setOwner (ModulesPanel* owner_)
        {
            owner = owner_;
            setModel (this);
        }

        void refresh()
        {
            updateContent();
            repaint();
        }

        int getNumRows()
        {
            return list.modules.size();
        }

        void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
        {
            if (rowIsSelected)
                g.fillAll (findColour (TextEditor::highlightColourId));

            const ModuleList::Module* const m = list.modules [rowNumber];

            if (m != nullptr)
            {
                const float tickSize = height * 0.7f;

                getLookAndFeel().drawTickBox (g, *this, (height - tickSize) / 2, (height - tickSize) / 2, tickSize, tickSize,
                                              owner->isEnabled (m), true, false, false);

                if (owner->isEnabled (m) && owner->areDependenciesMissing (m))
                    g.setColour (Colours::red);
                else
                    g.setColour (Colours::black);

                g.setFont (Font (height * 0.7f, Font::bold));
                g.drawFittedText (m->uid, height, 0, width - height, height, Justification::centredLeft, 1);
            }
        }

        void listBoxItemClicked (int row, const MouseEvent& e)
        {
            if (e.x < getRowHeight())
                flipRow (row);
        }

        void listBoxItemDoubleClicked (int row, const MouseEvent& e)
        {
            flipRow (row);
        }

        void returnKeyPressed (int row)
        {
            flipRow (row);
        }

        void selectedRowsChanged (int lastRowSelected)
        {
            owner->selectionChanged (list.modules [lastRowSelected]);
        }

        void flipRow (int row)
        {
            const ModuleList::Module* const m = list.modules [row];

            if (m != nullptr)
                owner->setEnabled (m, ! owner->isEnabled (m));
        }

    private:
        ModuleList& list;
        ModulesPanel* owner;
    };

    //==============================================================================
    class ModuleSettingsPanel  : public PropertyPanel
    {
    public:
        ModuleSettingsPanel (Project& project_, ModuleList& moduleList_, const String& moduleID_)
            : project (project_), moduleList (moduleList_), moduleID (moduleID_)
        {
            refreshAll();
        }

        void refreshAll()
        {
            setEnabled (project.isModuleEnabled (moduleID));

            clear();
            PropertyListBuilder props;

            ScopedPointer<LibraryModule> module (moduleList.loadModule (moduleID));

            if (module != nullptr)
            {
                props.add (new ModuleInfoComponent (project, moduleList, moduleID));

                if (project.isModuleEnabled (moduleID))
                {
                    const ModuleList::Module* m = moduleList.findModuleInfo (moduleID);
                    if (m != nullptr && moduleList.getExtraDependenciesNeeded (project, *m).size() > 0)
                        props.add (new MissingDependenciesComponent (project, moduleList, moduleID));
                }

                props.add (new BooleanPropertyComponent (project.shouldShowAllModuleFilesInProject (moduleID),
                                                         "Add source to project", "Make module files browsable in projects"),
                           "If this is enabled, then the entire source tree from this module will be shown inside your project, "
                           "making it easy to browse/edit the module's classes. If disabled, then only the minimum number of files "
                           "required to compile it will appear inside your project.");

                props.add (new BooleanPropertyComponent (project.shouldCopyModuleFilesLocally (moduleID),
                                                         "Create local copy", "Copy the module into the project folder"),
                           "If this is enabled, then a local copy of the entire module will be made inside your project (in the auto-generated JuceLibraryFiles folder), "
                           "so that your project will be self-contained, and won't need to contain any references to files in other folders. "
                           "This also means that you can check the module into your source-control system to make sure it is always in sync with your own code.");

                StringArray possibleValues;
                possibleValues.add ("(Use Default)");
                possibleValues.add ("Enabled");
                possibleValues.add ("Disabled");

                Array<var> mappings;
                mappings.add (Project::configFlagDefault);
                mappings.add (Project::configFlagEnabled);
                mappings.add (Project::configFlagDisabled);

                OwnedArray <Project::ConfigFlag> flags;
                module->getConfigFlags (project, flags);

                for (int i = 0; i < flags.size(); ++i)
                {
                    ChoicePropertyComponent* c = new ChoicePropertyComponent (flags[i]->value, flags[i]->symbol, possibleValues, mappings);
                    c->setTooltip (flags[i]->description);
                    props.add (c);
                }
            }

            addProperties (props.components);
        }

    private:
        Project& project;
        ModuleList& moduleList;
        String moduleID;

        //==============================================================================
        class ModuleInfoComponent  : public PropertyComponent
        {
        public:
            ModuleInfoComponent (Project& project_, ModuleList& moduleList_, const String& moduleID_)
                : PropertyComponent ("Module", 100), project (project_), moduleList (moduleList_), moduleID (moduleID_)
            {
            }

            void refresh() {}

            void paint (Graphics& g)
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (0, 0, getWidth(), getHeight() - 1);

                const ModuleList::Module* module = moduleList.findModuleInfo (moduleID);

                if (module != nullptr)
                {
                    String text;
                    text << module->name << newLine << "Version: " << module->version << newLine << newLine
                         << module->description;

                    GlyphArrangement ga;
                    ga.addJustifiedText (Font (13.0f), text, 4.0f, 16.0f, getWidth() - 8.0f, Justification::topLeft);
                    g.setColour (Colours::black);
                    ga.draw (g);
                }
            }

        private:
            Project& project;
            ModuleList& moduleList;
            String moduleID;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModuleInfoComponent);
        };

        //==============================================================================
        class MissingDependenciesComponent  : public PropertyComponent,
                                              public ButtonListener
        {
        public:
            MissingDependenciesComponent (Project& project_, ModuleList& moduleList_, const String& moduleID_)
                : PropertyComponent ("Dependencies", 100),
                  project (project_), moduleList (moduleList_), moduleID (moduleID_),
                  fixButton ("Enable Required Modules")
            {
                const ModuleList::Module* module = moduleList.findModuleInfo (moduleID);

                if (module != nullptr)
                    missingDependencies = moduleList.getExtraDependenciesNeeded (project, *module);

                addAndMakeVisible (&fixButton);
                fixButton.setColour (TextButton::buttonColourId, Colours::red);
                fixButton.setColour (TextButton::textColourOffId, Colours::white);
                fixButton.setBounds ("right - 160, parent.height - 26, parent.width - 8, top + 22");
                fixButton.addListener (this);
            }

            void refresh() {}

            void paint (Graphics& g)
            {
                g.setColour (Colours::white.withAlpha (0.4f));
                g.fillRect (0, 0, getWidth(), getHeight() - 1);

                String text ("This module requires the following dependencies:\n");
                text << missingDependencies.joinIntoString (", ");

                GlyphArrangement ga;
                ga.addJustifiedText (Font (13.0f), text, 4.0f, 16.0f, getWidth() - 8.0f, Justification::topLeft);
                g.setColour (Colours::red);
                ga.draw (g);
            }

            void buttonClicked (Button*)
            {
                bool isModuleCopiedLocally = project.shouldCopyModuleFilesLocally (moduleID).getValue();

                for (int i = missingDependencies.size(); --i >= 0;)
                    project.addModule (missingDependencies[i], isModuleCopiedLocally);

                ModulesPanel* mp = findParentComponentOfClass<ModulesPanel>();
                if (mp != nullptr)
                    mp->refresh();
            }

        private:
            Project& project;
            ModuleList& moduleList;
            String moduleID;
            StringArray missingDependencies;
            TextButton fixButton;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MissingDependenciesComponent);
        };
    };

    //==============================================================================
    class ModuleCopyingInfo  : public Component,
                               public ButtonListener,
                               public Timer
    {
    public:
        ModuleCopyingInfo (Project& project_, ModuleList& list_)
            : project (project_), list (list_),
              copyModeButton ("Set Copying Mode...")
        {
            addAndMakeVisible (&copyModeButton);
            copyModeButton.setBounds ("4, parent.height / 2 - 10, 160, parent.height / 2 + 10");
            copyModeButton.addListener (this);

            startTimer (1500);
        }

        void paint (Graphics& g)
        {
            g.setFont (11.0f);
            g.setColour (Colours::darkred);
            g.drawFittedText (getName(), copyModeButton.getRight() + 10, 0,
                              getWidth() - copyModeButton.getRight() - 16, getHeight(),
                              Justification::centredRight, 4);
        }

        void refresh()
        {
            int numCopied, numNonCopied;
            countCopiedModules (numCopied, numNonCopied);

            String newName;

            if (numCopied > 0 && numNonCopied > 0)
                newName = "Warning! Some of your modules are set to use local copies, and others are using remote references.\n"
                          "This may create problems if some modules expect to share the same parent folder, so you may "
                          "want to make sure that they are all either copied or not.";

            if (project.isAudioPluginModuleMissing())
                newName = "Warning! Your project is an audio plugin, but you haven't enabled the 'juce_audio_plugin_client' module!";

            if (newName != getName())
            {
                setName (newName);
                repaint();
            }
        }

        void countCopiedModules (int& numCopied, int& numNonCopied)
        {
            numCopied = numNonCopied = 0;

            for (int i = list.modules.size(); --i >= 0;)
            {
                const String moduleID (list.modules.getUnchecked(i)->uid);

                if (project.isModuleEnabled (moduleID))
                {
                    if (project.shouldCopyModuleFilesLocally (moduleID).getValue())
                        ++numCopied;
                    else
                        ++numNonCopied;
                }
            }
        }

        void buttonClicked (Button*)
        {
            PopupMenu menu;
            menu.addItem (1, "Enable local copying for all modules");
            menu.addItem (2, "Disable local copying for all modules");

            menu.showMenuAsync (PopupMenu::Options().withTargetComponent (&copyModeButton),
                                ModalCallbackFunction::forComponent (copyMenuItemChosen, this));
        }

        static void copyMenuItemChosen (int resultCode, ModuleCopyingInfo* comp)
        {
            if (resultCode > 0 && comp != nullptr)
                comp->setCopyModeForAllModules (resultCode == 1);
        }

        void setCopyModeForAllModules (bool copyEnabled)
        {
            for (int i = list.modules.size(); --i >= 0;)
                project.shouldCopyModuleFilesLocally (list.modules.getUnchecked(i)->uid) = copyEnabled;

            refresh();
        }

        void timerCallback()
        {
            refresh();
        }

    private:
        Project& project;
        ModuleList& list;
        TextButton copyModeButton;
    };

private:
    Project& project;
    ModuleList moduleList;
    FilenameComponent modulesLocation;
    Label modulesLabel;
    TextButton updateModulesButton;
    ModuleSelectionListBox moduleListBox;
    ModuleCopyingInfo copyingMessage;
    ScopedPointer<ModuleSettingsPanel> settings;
};

#endif
