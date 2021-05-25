/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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

//==============================================================================
class HeaderComponent    : public Component,
                           private ValueTree::Listener,
                           private ChangeListener,
                           private Value::Listener,
                           private Timer
{
public:
    HeaderComponent (ProjectContentComponent* projectContentComponent);

    //==============================================================================
    void resized() override;
    void paint (Graphics&) override;

    //==============================================================================
    void setCurrentProject (Project*);

    void updateExporters();
    std::unique_ptr<ProjectExporter> getSelectedExporter() const;
    bool canCurrentExporterLaunchProject() const;

    void sidebarTabsWidthChanged (int newWidth);

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
    int tabsWidth = 200;

    ProjectContentComponent* projectContentComponent = nullptr;
    Project* project = nullptr;
    ValueTree exportersTree;

    Value projectNameValue;

    ComboBox exporterBox;
    Label configLabel  { "Config Label", "Selected exporter" }, projectNameLabel;

    ImageComponent juceIcon;
    UserAvatarComponent userAvatar { true };

    IconButton projectSettingsButton { "Project Settings", getIcons().settings },
               saveAndOpenInIDEButton { "Save and Open in IDE", Image() };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
