/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Application/jucer_Headers.h"
#include "../../Utility/UI/jucer_IconButton.h"
#include "jucer_UserAvatarComponent.h"

class Project;
class ProjectContentComponent;
class ProjectExporter;
class CompileEngineChildProcess;

//==============================================================================
class HeaderComponent    : public Component,
                           private ValueTree::Listener,
                           private ChangeListener,
                           private Value::Listener,
                           private Timer
{
public:
    HeaderComponent (ProjectContentComponent* projectContentComponent);
    ~HeaderComponent() override;

    //==============================================================================
    void resized() override;
    void paint (Graphics&) override;

    //==============================================================================
    void setCurrentProject (Project*);

    void updateExporters();
    std::unique_ptr<ProjectExporter> getSelectedExporter() const;
    bool canCurrentExporterLaunchProject() const;

    void sidebarTabsWidthChanged (int newWidth);
    void liveBuildEnablementChanged (bool isEnabled);

private:
    //==============================================================================
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void valueChanged (Value&) override;
    void timerCallback() override;

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override        { updateIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override { updateIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override   { updateIfNeeded (parentTree); }

    void updateIfNeeded (ValueTree tree)
    {
        if (tree == exportersTree)
            updateExporters();
    }

    //==============================================================================
    void initialiseButtons();

    void updateName();
    void updateExporterButton();

    //==============================================================================
    void buildPing();
    void buildFinished (bool);
    void setRunAppButtonState (bool);

    //==============================================================================
    int tabsWidth = 200;
    bool isBuilding = false;

    ProjectContentComponent* projectContentComponent = nullptr;
    Project* project = nullptr;
    ValueTree exportersTree;

    Value projectNameValue;

    ComboBox exporterBox;
    Label configLabel  { "Config Label", "Selected exporter" }, projectNameLabel;

    ImageComponent juceIcon;
    UserAvatarComponent userAvatar { true };

    IconButton projectSettingsButton { "Project Settings", getIcons().settings },
               saveAndOpenInIDEButton { "Save and Open in IDE", Image() },
               runAppButton { "Run Application", getIcons().play };

    ReferenceCountedObjectPtr<CompileEngineChildProcess> childProcess;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
