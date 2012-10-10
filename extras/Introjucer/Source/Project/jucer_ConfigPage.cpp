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

#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Application/jucer_Application.h"
#include "jucer_ConfigPage.h"
#include "jucer_ModulesPanel.h"


//==============================================================================
void SettingsTreeViewItemBase::showSettingsPage (Component* content)
{
    content->setComponentID (getUniqueName());

    ScopedPointer<Component> comp (content);
    ProjectContentComponent* pcc = getProjectContentComponent();

    if (pcc != nullptr)
        pcc->setEditorComponent (new PropertyPanelViewport (comp.release()), nullptr);
}

void SettingsTreeViewItemBase::closeSettingsPage()
{
    ProjectContentComponent* pcc = getProjectContentComponent();

    if (pcc != nullptr)
    {
        PropertyPanelViewport* ppv = dynamic_cast<PropertyPanelViewport*> (pcc->getEditorComponent());

        if (ppv != nullptr && ppv->viewport.getViewedComponent()->getComponentID() == getUniqueName())
            pcc->hideEditor();
    }
}

//==============================================================================
namespace ProjectSettingsTreeClasses
{
    class ConfigItem   : public SettingsTreeViewItemBase
    {
    public:
        ConfigItem (const ProjectExporter::BuildConfiguration::Ptr& config_, const String& exporterName_)
            : config (config_), exporterName (exporterName_), configTree (config->config)
        {
            jassert (config != nullptr);
            configTree.addListener (this);
        }

        bool isMissing()                        { return false; }
        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return false; }
        String getUniqueName() const            { return "config_" + config->getName(); }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return config->getName(); }
        void setName (const String&)            {}
        Icon getIcon() const                    { return Icon (getIcons().config, getContrastingColour (Colours::green, 0.5f)); }

        void showDocument()                     { showSettingsPage (new SettingsComp (config, exporterName)); }
        void itemOpennessChanged (bool)         {}

        void deleteItem()
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Configuration",
                                              "Are you sure you want to delete this configuration?"))
            {
                closeSettingsPage();
                config->removeFromExporter();
            }
        }

        void showPopupMenu()
        {
            PopupMenu menu;
            menu.addItem (1, "Create a copy of this configuration");
            menu.addSeparator();
            menu.addItem (2, "Delete this configuration");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode == 2)
            {
                deleteAllSelectedItems();
            }
            else if (resultCode == 1)
            {
                for (Project::ExporterIterator exporter (config->project); exporter.next();)
                {
                    if (config->config.isAChildOf (exporter.exporter->settings))
                    {
                        exporter.exporter->addNewConfiguration (config);
                        break;
                    }
                }
            }
        }

        var getDragSourceDescription()
        {
            return getParentItem()->getUniqueName() + "||" + config->getName();
        }

        void valueTreePropertyChanged (ValueTree&, const Identifier&)  { repaintItem(); }

    private:
        ProjectExporter::BuildConfiguration::Ptr config;
        String exporterName;
        ValueTree configTree;

        //==============================================================================
        class SettingsComp  : public Component
        {
        public:
            SettingsComp (ProjectExporter::BuildConfiguration* config, const String& exporterName)
            {
                addAndMakeVisible (&group);

                PropertyListBuilder props;
                config->createPropertyEditors (props);
                group.setProperties (props);
                group.setName (exporterName + " / " + config->getName());
                parentSizeChanged();
            }

            void parentSizeChanged()  { updateSize (*this, group); }

        private:
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConfigItem);
    };

    //==============================================================================
    class ExporterItem   : public SettingsTreeViewItemBase
    {
    public:
        ExporterItem (Project& project_, ProjectExporter* exporter_, int exporterIndex_)
            : project (project_), exporter (exporter_), configListTree (exporter->getConfigurations()),
              exporterIndex (exporterIndex_)
        {
            configListTree.addListener (this);
            jassert (exporter != nullptr);
        }

        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return exporter->getNumConfigurations() > 0; }
        String getUniqueName() const            { return "exporter_" + String (exporterIndex); }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return exporter->getName(); }
        void setName (const String&)            {}
        bool isMissing()                        { return false; }
        Icon getIcon() const                    { return Icon (getIcons().exporter, getContrastingColour (0.5f)); }
        void showDocument()                     { showSettingsPage (new SettingsComp (exporter)); }

        void deleteItem()
        {
            if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Delete Exporter",
                                              "Are you sure you want to delete this export target?"))
            {
                closeSettingsPage();
                ValueTree parent (exporter->settings.getParent());
                parent.removeChild (exporter->settings, project.getUndoManagerFor (parent));
            }
        }

        void addSubItems()
        {
            for (ProjectExporter::ConfigIterator config (*exporter); config.next();)
                addSubItem (new ConfigItem (config.config, exporter->getName()));
        }

        void showPopupMenu()
        {
            PopupMenu menu;
            menu.addItem (1, "Add a new configuration");
            menu.addSeparator();
            menu.addItem (2, "Delete this exporter");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode == 2)
                deleteAllSelectedItems();
            else if (resultCode == 1)
                exporter->addNewConfiguration (nullptr);
        }

        var getDragSourceDescription()
        {
            return getParentItem()->getUniqueName() + "/" + String (exporterIndex);
        }

        bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
        {
            return dragSourceDetails.description.toString().startsWith (getUniqueName());
        }

        void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex)
        {
            const int oldIndex = indexOfConfig (dragSourceDetails.description.toString().fromLastOccurrenceOf ("||", false, false));

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
        void valueTreeChildAdded (ValueTree& parentTree, ValueTree&)    { refreshIfNeeded (parentTree); }
        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&)  { refreshIfNeeded (parentTree); }
        void valueTreeChildOrderChanged (ValueTree& parentTree)         { refreshIfNeeded (parentTree); }

        void refreshIfNeeded (ValueTree& changedTree)
        {
            if (changedTree == configListTree)
                refreshSubItems();
        }

    private:
        Project& project;
        ScopedPointer<ProjectExporter> exporter;
        ValueTree configListTree;
        int exporterIndex;

        //==============================================================================
        class SettingsComp  : public Component
        {
        public:
            SettingsComp (ProjectExporter* exporter)
            {
                addAndMakeVisible (&group);

                PropertyListBuilder props;
                exporter->createPropertyEditors (props);
                group.setProperties (props);
                group.setName ("Export target: " + exporter->getName());
                parentSizeChanged();
            }

            void parentSizeChanged()  { updateSize (*this, group); }

        private:
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExporterItem);
    };

    //==============================================================================
    class ModulesItem   : public SettingsTreeViewItemBase
    {
    public:
        ModulesItem (Project& project_)  : project (project_) {}

        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return false; }
        String getUniqueName() const            { return "modules"; }
        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return "Modules"; }
        void setName (const String&)            {}
        bool isMissing()                        { return false; }
        Icon getIcon() const                    { return Icon (getIcons().graph, getContrastingColour (Colours::red, 0.5f)); }
        void showDocument()                     { showSettingsPage (new SettingsComp (project)); }

    private:
        Project& project;

        //==============================================================================
        class SettingsComp  : public Component
        {
        public:
            SettingsComp (Project& project_)   : project (project_)
            {
                addAndMakeVisible (&group);

                PropertyListBuilder props;
                props.add (new ModulesPanel (project));
                group.setProperties (props);
                group.setName ("Modules");

                parentSizeChanged();
            }

            void parentSizeChanged()
            {
                updateSize (*this, group);
            }

        private:
            Project& project;
            var lastProjectType;
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModulesItem);
    };

    //==============================================================================
    class RootItem   : public SettingsTreeViewItemBase
    {
    public:
        RootItem (Project& project_)
            : project (project_), exportersTree (project_.getExporters())
        {
            exportersTree.addListener (this);
        }

        String getRenamingName() const          { return getDisplayName(); }
        String getDisplayName() const           { return project.getTitle(); }
        void setName (const String&)            {}
        bool isMissing()                        { return false; }
        Icon getIcon() const                    { return project.getMainGroup().getIcon().withContrastingColourTo (getBackgroundColour()); }
        void showDocument()                     { showSettingsPage (new SettingsComp (project)); }
        bool canBeSelected() const              { return true; }
        bool mightContainSubItems()             { return project.getNumExporters() > 0; }
        String getUniqueName() const            { return "config_root"; }

        void addSubItems()
        {
            addSubItem (new ModulesItem (project));
            IntrojucerApp::getApp().addExtraConfigItems (project, *this);

            int i = 0;
            for (Project::ExporterIterator exporter (project); exporter.next(); ++i)
                addSubItem (new ExporterItem (project, exporter.exporter.release(), i));
        }

        void showPopupMenu()
        {
            PopupMenu menu;

            const StringArray exporters (ProjectExporter::getExporterNames());

            for (int i = 0; i < exporters.size(); ++i)
                menu.addItem (i + 1, "Create a new " + exporters[i] + " target");

            launchPopupMenu (menu);
        }

        void handlePopupMenuResult (int resultCode)
        {
            if (resultCode > 0)
            {
                String exporterName (ProjectExporter::getExporterNames() [resultCode - 1]);

                if (exporterName.isNotEmpty())
                    project.addNewExporter (exporterName);
            }
        }

        bool isInterestedInDragSource (const DragAndDropTarget::SourceDetails& dragSourceDetails)
        {
            return dragSourceDetails.description.toString().startsWith (getUniqueName());
        }

        void itemDropped (const DragAndDropTarget::SourceDetails& dragSourceDetails, int insertIndex)
        {
            int oldIndex = dragSourceDetails.description.toString().getTrailingIntValue();
            exportersTree.moveChild (oldIndex, jmax (0, insertIndex - 1), project.getUndoManagerFor (exportersTree));
        }

        //==============================================================================
        void valueTreeChildAdded (ValueTree& parentTree, ValueTree&)    { refreshIfNeeded (parentTree); }
        void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&)  { refreshIfNeeded (parentTree); }
        void valueTreeChildOrderChanged (ValueTree& parentTree)         { refreshIfNeeded (parentTree); }

        void refreshIfNeeded (ValueTree& changedTree)
        {
            if (changedTree == exportersTree)
                refreshSubItems();
        }

    private:
        Project& project;
        ValueTree exportersTree;

        //==============================================================================
        class SettingsComp  : public Component,
                              private ChangeListener
        {
        public:
            SettingsComp (Project& project_)
                : project (project_)
            {
                addAndMakeVisible (&group);

                updatePropertyList();
                project.addChangeListener (this);
            }

            ~SettingsComp()
            {
                project.removeChangeListener (this);
            }

            void parentSizeChanged()
            {
                updateSize (*this, group);
            }

            void updatePropertyList()
            {
                PropertyListBuilder props;
                project.createPropertyEditors (props);
                group.setProperties (props);
                group.setName ("Project Settings");

                lastProjectType = project.getProjectTypeValue().getValue();
                parentSizeChanged();
            }

            void changeListenerCallback (ChangeBroadcaster*)
            {
                if (lastProjectType != project.getProjectTypeValue().getValue())
                    updatePropertyList();
            }

        private:
            Project& project;
            var lastProjectType;
            PropertyGroup group;

            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SettingsComp);
        };

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootItem);
    };
}

JucerTreeViewBase* createProjectConfigTreeViewRoot (Project& project)
{
    return new ProjectSettingsTreeClasses::RootItem (project);
}
