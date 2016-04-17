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

class ModulesPanel  : public Component,
                      private TableListBoxModel,
                      private ValueTree::Listener,
                      private Button::Listener
{
public:
    ModulesPanel (Project& p)
        : project (p),
          modulesValueTree (p.getModules().state),
          setCopyModeButton  ("Set copy-mode for all modules..."),
          copyPathButton ("Set paths for all modules...")
    {
        table.getHeader().addColumn ("Module", nameCol, 180, 100, 400, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Version", versionCol, 100, 100, 100, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Make Local Copy", copyCol, 100, 100, 100, TableHeaderComponent::notSortable);
        table.getHeader().addColumn ("Paths", pathCol, 250, 100, 600, TableHeaderComponent::notSortable);

        table.setModel (this);
        table.setColour (TableListBox::backgroundColourId, Colours::transparentBlack);
        addAndMakeVisible (table);
        table.updateContent();
        table.setRowHeight (20);

        addAndMakeVisible (setCopyModeButton);
        addAndMakeVisible (copyPathButton);
        setCopyModeButton.addListener (this);
        setCopyModeButton.setTriggeredOnMouseDown (true);
        copyPathButton.addListener (this);
        copyPathButton.setTriggeredOnMouseDown (true);

        modulesValueTree.addListener (this);
        lookAndFeelChanged();
    }

    void paint (Graphics& g) override
    {
        ProjucerLookAndFeel::fillWithBackgroundTexture (*this, g);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (5, 4));

        table.setBounds (r.removeFromTop (table.getRowPosition (getNumRows() - 1, true).getBottom() + 20));

        Rectangle<int> buttonRow (r.removeFromTop (32).removeFromBottom (28));
        setCopyModeButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
        buttonRow.removeFromLeft (8);
        copyPathButton.setBounds (buttonRow.removeFromLeft (jmin (260, r.getWidth() / 3)));
    }

    int getNumRows() override
    {
        return project.getModules().getNumModules();
    }

    void paintRowBackground (Graphics& g, int /*rowNumber*/, int width, int height, bool rowIsSelected) override
    {
        g.setColour (rowIsSelected ? Colours::lightblue.withAlpha (0.4f)
                                   : Colours::white.withAlpha (0.4f));
        g.fillRect (0, 0, width, height - 1);
    }

    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool /*rowIsSelected*/) override
    {
        String text;
        const String moduleID (project.getModules().getModuleID (rowNumber));

        if (columnId == nameCol)
        {
            text = moduleID;
        }
        else if (columnId == versionCol)
        {
            text = project.getModules().getModuleInfo (moduleID).getVersion();

            if (text.isEmpty())
                text = "?";
        }
        else if (columnId == copyCol)
        {
            text = project.getModules().shouldCopyModuleFilesLocally (moduleID).getValue()
                        ? "Yes" : "No";
        }
        else if (columnId == pathCol)
        {
            StringArray paths;

            for (Project::ExporterIterator exporter (project); exporter.next();)
                paths.addIfNotAlreadyThere (exporter->getPathForModuleString (moduleID).trim());

            text = paths.joinIntoString (", ");
        }

        g.setColour (Colours::black);
        g.setFont (height * 0.65f);
        g.drawText (text, Rectangle<int> (width, height).reduced (4, 0), Justification::centredLeft, true);
    }

    void cellDoubleClicked (int rowNumber, int, const MouseEvent&) override
    {
        const String moduleID (project.getModules().getModuleID (rowNumber));

        if (moduleID.isNotEmpty())
            if (ProjectContentComponent* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showModule (moduleID);
    }

    void deleteKeyPressed (int row) override
    {
        project.getModules().removeModule (project.getModules().getModuleID (row));
    }

    void buttonClicked (Button* b) override
    {
        if (b == &setCopyModeButton)   showCopyModeMenu();
        if (b == &copyPathButton)      showSetPathsMenu();
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
    TableListBox table;
    TextButton setCopyModeButton, copyPathButton;

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override    { itemChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                { itemChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override         { itemChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override           { itemChanged(); }
    void valueTreeParentChanged (ValueTree&) override                         { itemChanged(); }

    void itemChanged()
    {
        table.updateContent();
        resized();
        repaint();
    }

    void showCopyModeMenu()
    {
        PopupMenu m;
        m.addItem (1, "Set all modules to copy locally");
        m.addItem (2, "Set all modules to not copy locally");

        int res = m.showAt (&setCopyModeButton);

        if (res != 0)
            project.getModules().setLocalCopyModeForAllModules (res == 1);
    }

    void showSetPathsMenu()
    {
        EnabledModuleList& moduleList = project.getModules();

        const String moduleToCopy (moduleList.getModuleID (table.getSelectedRow()));

        if (moduleToCopy.isNotEmpty())
        {
            PopupMenu m;
            m.addItem (1, "Copy the paths from the module '" + moduleToCopy + "' to all other modules");

            int res = m.showAt (&copyPathButton);

            if (res != 0)
            {
                for (Project::ExporterIterator exporter (project); exporter.next();)
                {
                    for (int i = 0; i < moduleList.getNumModules(); ++i)
                    {
                        String modID = moduleList.getModuleID (i);

                        if (modID != moduleToCopy)
                            exporter->getPathForModuleValue (modID) = exporter->getPathForModuleValue (moduleToCopy).getValue();
                    }
                }
            }

            table.repaint();
        }
        else
        {
            PopupMenu m;
            m.addItem (1, "(Select a module in the list above to use this option)", false);

            m.showAt (&copyPathButton);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesPanel)
};
