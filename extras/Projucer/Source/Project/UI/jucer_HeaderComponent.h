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

#include "../../Utility/UI/jucer_IconButton.h"
#include "../../Utility/UI/jucer_UserSettingsPopup.h"

//==============================================================================
class HeaderComponent    : public Component,
                           private Button::Listener,
                           private ComboBox::Listener,
                           private ValueTree::Listener,
                           private ChangeListener
{
public:
    HeaderComponent()
        : configLabel ("Config Label", "Selected exporter")
    {
        addAndMakeVisible (configLabel);
        addAndMakeVisible (exporterBox);

        exporterBox.addListener (this);

        addAndMakeVisible  (juceIcon = new ImageComponent ("icon"));
        juceIcon->setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize),
                            RectanglePlacement::centred);

        projectNameLabel.setText (String(), dontSendNotification);
        addAndMakeVisible (projectNameLabel);

        initialiseButtons();
    }

    ~HeaderComponent()
    {
        if (userSettingsWindow != nullptr)
            userSettingsWindow->dismiss();
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        configLabel.setFont (Font (bounds.getHeight() / 3.0f));

        //======================================================================
        auto projectHeaderBounds = bounds.removeFromLeft (tabsWidth);
        juceIcon->setBounds (projectHeaderBounds.removeFromLeft (projectHeaderBounds.getHeight()).reduced (5, 5));

        projectSettingsButton->setBounds (projectHeaderBounds.removeFromRight (projectHeaderBounds.getHeight()).reduced (2, 2));

        projectNameLabel.setBounds (projectHeaderBounds);

        //======================================================================
        bounds.removeFromLeft (33);
        continuousRebuildButton->setBounds (bounds.removeFromLeft (bounds.getHeight()).reduced (2, 2));
        bounds.removeFromLeft (5);
        buildNowButton->setBounds (bounds.removeFromLeft (bounds.getHeight()).reduced (2, 2));

        bounds.removeFromRight (5);
        userSettingsButton->setBounds (bounds.removeFromRight (bounds.getHeight()).reduced (2, 2));

        auto exporterWidth = jmax (250, bounds.getWidth() / 2);
        auto spacing = bounds.getWidth() - exporterWidth;

        auto leftSpacing = jmax (20, spacing / 3);
        auto rightSpacing = jmax (40, 2 * (spacing / 3));

        bounds.removeFromLeft (leftSpacing);
        bounds.removeFromRight (rightSpacing);

        saveAndOpenInIDEButton->setBounds (bounds.removeFromRight (bounds.getHeight()).reduced (2, 2));
        bounds.removeFromRight (5);
        exporterSettingsButton->setBounds (bounds.removeFromRight (bounds.getHeight()).reduced (2, 2));
        bounds.removeFromRight (10);

        exporterBox.setBounds (bounds.removeFromBottom (roundToInt (bounds.getHeight() / 1.8f)));
        configLabel.setBounds (bounds);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

    void setCurrentProject (Project* p)
    {
        project = p;

        exportersTree = project->getExporters();
        exportersTree.addListener (this);
        updateExporters();

        project->addChangeListener (this);
        updateName();

        if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
            updateBuildButtons (pcc->isBuildEnabled(), pcc->isContinuousRebuildEnabled());
    }

    void updateExporters()
    {
        auto selectedName = getSelectedExporterName();

        exporterBox.clear();
        auto preferredExporterIndex = -1;

        int i = 0;
        for (Project::ExporterIterator exporter (*project); exporter.next(); ++i)
        {
            exporterBox.addItem (exporter->getName(), i + 1);

            if (selectedName == exporter->getName())
                exporterBox.setSelectedId (i + 1);

            if (exporter->canLaunchProject() && preferredExporterIndex == -1)
                preferredExporterIndex = i;
        }

        if (exporterBox.getSelectedItemIndex() == -1)
            exporterBox.setSelectedItemIndex (preferredExporterIndex != -1 ? preferredExporterIndex
                                                                           : 0);

        updateExporterButton();
    }

    String getSelectedExporterName()
    {
        return exporterBox.getItemText (exporterBox.getSelectedItemIndex());
    }

    bool canCurrentExporterLaunchProject()
    {
        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (exporter->getName() == getSelectedExporterName() && exporter->canLaunchProject())
                return true;

        return false;
    }

    int getUserButtonWidth()            { return userSettingsButton->getWidth(); }

    void sidebarTabsWidthChanged (int newWidth)
    {
        tabsWidth = newWidth;
        resized();
    }

    void showUserSettings()
    {
       #if JUCER_ENABLE_GPL_MODE
        const int settingsPopupHeight = 75;
       #else
        const int settingsPopupHeight = 150;
       #endif

        auto* content = new UserSettingsPopup (false);


        content->setSize (200, settingsPopupHeight);

        userSettingsWindow = &CallOutBox::launchAsynchronously (content, userSettingsButton->getScreenBounds(), nullptr);
    }

    void updateBuildButtons (bool isBuildEnabled, bool isContinuousRebuildEnabled)
    {
        buildNowButton->setEnabled (isBuildEnabled && ! isContinuousRebuildEnabled);
        continuousRebuildButton->setEnabled (isBuildEnabled);

        continuousRebuildButton->icon = Icon (isContinuousRebuildEnabled ? &getIcons().continuousBuildStop : &getIcons().continuousBuildStart,
                                              Colours::transparentBlack);
        repaint();
    }

    void lookAndFeelChanged() override
    {
        if (userSettingsWindow != nullptr)
            userSettingsWindow->sendLookAndFeelChange();
    }

private:
    Project* project = nullptr;
    ValueTree exportersTree;

    Label configLabel, projectNameLabel;
    ComboBox exporterBox;

    ScopedPointer<ImageComponent> juceIcon;
    ScopedPointer<IconButton> projectSettingsButton, continuousRebuildButton, buildNowButton,
                              exporterSettingsButton, saveAndOpenInIDEButton, userSettingsButton;

    SafePointer<CallOutBox> userSettingsWindow;

    int tabsWidth = 200;

    //==========================================================================
    void buttonClicked (Button* b) override
    {
        auto* pcc = findParentComponentOfClass<ProjectContentComponent>();

        if      (b == projectSettingsButton)      pcc->showProjectSettings();
        else if (b == continuousRebuildButton)    pcc->setContinuousRebuildEnabled (! pcc->isContinuousRebuildEnabled());
        else if (b == buildNowButton)             pcc->rebuildNow();
        else if (b == exporterSettingsButton)     pcc->showExporterSettings (getSelectedExporterName());
        else if (b == saveAndOpenInIDEButton)     pcc->openInSelectedIDE (true);
        else if (b == userSettingsButton)         showUserSettings();
    }

    void comboBoxChanged (ComboBox* c) override
    {
        if (c == &exporterBox)
            updateExporterButton();
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == project)
        {
            updateName();
            updateExporters();
        }
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override       {}
    void valueTreeParentChanged (ValueTree&) override                            {}

    void valueTreeChildAdded (ValueTree& parentTree, ValueTree&) override        { updateIfNeeded (parentTree); }
    void valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int) override { updateIfNeeded (parentTree); }
    void valueTreeChildOrderChanged (ValueTree& parentTree, int, int) override   { updateIfNeeded (parentTree); }

    void initialiseButtons()
    {
        auto& icons = getIcons();

        addAndMakeVisible (projectSettingsButton = new IconButton ("Project Settings", &icons.settings));
        projectSettingsButton->addListener (this);

        addAndMakeVisible (continuousRebuildButton = new IconButton ("Continuous Rebuild", &icons.continuousBuildStart));
        continuousRebuildButton->addListener (this);

        addAndMakeVisible (buildNowButton = new IconButton ("Build Now", &icons.buildNow));
        buildNowButton->addListener (this);

        addAndMakeVisible (exporterSettingsButton = new IconButton ("Exporter Settings", &icons.edit));
        exporterSettingsButton->addListener (this);

        addAndMakeVisible (saveAndOpenInIDEButton = new IconButton ("Save and Open in IDE", nullptr));
        saveAndOpenInIDEButton->addListener (this);
        saveAndOpenInIDEButton->isIDEButton = true;

        addAndMakeVisible (userSettingsButton = new IconButton ("User Settings", &icons.user));
        userSettingsButton->addListener (this);
        userSettingsButton->isUserButton = true;

        updateExporterButton();
        updateUserAvatar();
    }

    void updateIfNeeded (ValueTree tree)
    {
        if (tree == exportersTree)
            updateExporters();
    }

    void updateName()
    {
        projectNameLabel.setText (project->getDocumentTitle(), dontSendNotification);
    }

    void updateExporterButton()
    {
        auto currentExporterName = getSelectedExporterName();

        for (auto info : ProjectExporter::getExporterTypes())
        {
            if (currentExporterName.contains (info.name))
            {
                saveAndOpenInIDEButton->iconImage = info.getIcon();
                saveAndOpenInIDEButton->repaint();
                saveAndOpenInIDEButton->setEnabled (canCurrentExporterLaunchProject());
            }
        }
    }

    void updateUserAvatar()
    {
        if (LicenseController* controller = ProjucerApplication::getApp().licenseController)
        {
            auto state = controller->getState();

            userSettingsButton->iconImage = state.avatar;
            userSettingsButton->repaint();
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HeaderComponent)
};
