/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

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
#include "../../Utility/UI/jucer_UserSettingsPopup.h"

class Project;

//==============================================================================
class HeaderComponent    : public Component,
                           private ValueTree::Listener,
                           private ChangeListener,
                           private Value::Listener,
                           private Timer
{
public:
    HeaderComponent();
    ~HeaderComponent() override;

    //==============================================================================
    void resized() override;
    void paint (Graphics&) override;

    //==============================================================================
    void setCurrentProject (Project*) noexcept;

    //==============================================================================
    void updateExporters() noexcept;
    String getSelectedExporterName() const noexcept;
    bool canCurrentExporterLaunchProject() const noexcept;

    //==============================================================================
    int getUserButtonWidth() const noexcept;
    void sidebarTabsWidthChanged (int newWidth) noexcept;

    //==============================================================================
    void showUserSettings() noexcept;

private:
    //==============================================================================
    void lookAndFeelChanged() override;
    void changeListenerCallback (ChangeBroadcaster* source) override;
    void valueChanged (Value&) override;
    void timerCallback() override;

    //==============================================================================
    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override        { updateIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override { updateIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override   { updateIfNeeded (parentTree); }

    void updateIfNeeded (ValueTree tree) noexcept
    {
        if (tree == exportersTree)
            updateExporters();
    }

    //==============================================================================
    void initialiseButtons() noexcept;

    void updateName() noexcept;
    void updateExporterButton() noexcept;
    void updateUserAvatar() noexcept;

    //==============================================================================
    void buildPing();
    void buildFinished (bool);
    void setRunAppButtonState (bool);

    //==============================================================================
    int tabsWidth = 200;
    bool isBuilding = false;

    Project* project = nullptr;
    ValueTree exportersTree;

    Value projectNameValue;

    ComboBox exporterBox;
    Label configLabel  { "Config Label", "Selected exporter" },
    projectNameLabel;

    std::unique_ptr<ImageComponent> juceIcon;
    std::unique_ptr<IconButton> projectSettingsButton, saveAndOpenInIDEButton, userSettingsButton, runAppButton;

    SafePointer<CallOutBox> userSettingsWindow;

    ReferenceCountedObjectPtr<CompileEngineChildProcess> childProcess;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
