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

#include "jucer_HeaderComponent.h"

#include "../../Application/jucer_Application.h"

#include "../../ProjectSaving/jucer_ProjectExporter.h"
#include "../../Project/UI/jucer_ProjectContentComponent.h"

#include "../../LiveBuildEngine/jucer_MessageIDs.h"
#include "../../LiveBuildEngine/jucer_SourceCodeRange.h"
#include "../../LiveBuildEngine/jucer_ClassDatabase.h"
#include "../../LiveBuildEngine/jucer_DiagnosticMessage.h"
#include "../../LiveBuildEngine/jucer_CompileEngineClient.h"

//==============================================================================
HeaderComponent::HeaderComponent (ProjectContentComponent* pcc)
    : projectContentComponent (pcc)
{
    addAndMakeVisible (configLabel);
    addAndMakeVisible (exporterBox);

    exporterBox.onChange = [this] { updateExporterButton(); };

    juceIcon.setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize), RectanglePlacement::centred);
    addAndMakeVisible (juceIcon);

    addAndMakeVisible (userAvatar);
    userAvatar.addChangeListener (this);

    projectNameLabel.setText ({}, dontSendNotification);
    addAndMakeVisible (projectNameLabel);

    initialiseButtons();
}

HeaderComponent::~HeaderComponent()
{
    if (childProcess != nullptr)
    {
        childProcess->activityList.removeChangeListener (this);
        childProcess->errorList.removeChangeListener (this);
    }
}

//==============================================================================
void HeaderComponent::resized()
{
    auto bounds = getLocalBounds();
    configLabel.setFont ({ (float) bounds.getHeight() / 3.0f });

    {
        auto headerBounds = bounds.removeFromLeft (tabsWidth);

        const int buttonSize = 25;
        auto buttonBounds = headerBounds.removeFromRight (buttonSize);

        projectSettingsButton.setBounds (buttonBounds.removeFromBottom (buttonSize).reduced (2));

        juceIcon.setBounds (headerBounds.removeFromLeft (headerBounds.getHeight()).reduced (2));

        headerBounds.removeFromRight (5);
        projectNameLabel.setBounds (headerBounds);
    }

    {
        auto exporterWidth = jmin (400, bounds.getWidth() / 2);
        Rectangle<int> exporterBounds (0, 0, exporterWidth, bounds.getHeight());

        exporterBounds.setCentre (bounds.getCentre());

        runAppButton.setBounds (exporterBounds.removeFromRight (exporterBounds.getHeight()).reduced (2));
        saveAndOpenInIDEButton.setBounds (exporterBounds.removeFromRight (exporterBounds.getHeight()).reduced (2));

        exporterBounds.removeFromRight (5);
        exporterBox.setBounds (exporterBounds.removeFromBottom (roundToInt ((float) exporterBounds.getHeight() / 1.8f)));
        configLabel.setBounds (exporterBounds);
    }

    userAvatar.setBounds (bounds.removeFromRight (userAvatar.isDisplaingGPLLogo() ? roundToInt ((float) bounds.getHeight() * 1.9f)
                                                                                  : bounds.getHeight()).reduced (2));
}

void HeaderComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    if (isBuilding)
        getLookAndFeel().drawSpinningWaitAnimation (g, findColour (treeIconColourId),
                                                    runAppButton.getX(), runAppButton.getY(),
                                                    runAppButton.getWidth(), runAppButton.getHeight());
}

//==============================================================================
void HeaderComponent::setCurrentProject (Project* newProject)
{
    isBuilding = false;
    stopTimer();
    repaint();

    projectNameLabel.setText ({}, dontSendNotification);

    project = newProject;

    if (project != nullptr)
    {
        exportersTree = project->getExporters();
        exportersTree.addListener (this);
        updateExporters();

        projectNameValue.referTo (project->getProjectValue (Ids::name));
        projectNameValue.addListener (this);
        updateName();

        childProcess = ProjucerApplication::getApp().childProcessCache->getExisting (*project);

        if (childProcess != nullptr)
        {
            childProcess->activityList.addChangeListener (this);
            childProcess->errorList.addChangeListener (this);

            runAppButton.setTooltip ({});
            runAppButton.setEnabled (true);
        }
        else
        {
            runAppButton.setTooltip ("Enable live-build engine to launch application");
            runAppButton.setEnabled (false);
        }
    }
}

//==============================================================================
void HeaderComponent::updateExporters()
{
    auto selectedExporter = getSelectedExporter();

    exporterBox.clear();
    auto preferredExporterIndex = -1;

    int i = 0;
    for (Project::ExporterIterator exporter (*project); exporter.next(); ++i)
    {
        auto exporterName = exporter->getUniqueName();

        exporterBox.addItem (exporterName, i + 1);

        if (selectedExporter != nullptr && exporterName == selectedExporter->getUniqueName())
            exporterBox.setSelectedId (i + 1);

        if (exporterName.contains (ProjectExporter::getCurrentPlatformExporterTypeInfo().displayName) && preferredExporterIndex == -1)
            preferredExporterIndex = i;
    }

    if (exporterBox.getSelectedItemIndex() == -1)
    {
        if (preferredExporterIndex == -1)
        {
            i = 0;
            for (Project::ExporterIterator exporter (*project); exporter.next(); ++i)
            {
                if (exporter->canLaunchProject())
                {
                    preferredExporterIndex = i;
                    break;
                }
            }
        }

        exporterBox.setSelectedItemIndex (preferredExporterIndex != -1 ? preferredExporterIndex : 0);
    }

    updateExporterButton();
}

std::unique_ptr<ProjectExporter> HeaderComponent::getSelectedExporter() const
{
    if (project != nullptr)
    {
        int i = 0;
        auto selectedIndex = exporterBox.getSelectedItemIndex();

        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (i++ == selectedIndex)
                return std::move (exporter.exporter);
    }

    return nullptr;
}

bool HeaderComponent::canCurrentExporterLaunchProject() const
{
    if (project != nullptr)
    {
        if (auto selectedExporter = getSelectedExporter())
        {
            for (Project::ExporterIterator exporter (*project); exporter.next();)
                if (exporter->canLaunchProject() && exporter->getUniqueName() == selectedExporter->getUniqueName())
                    return true;
        }
    }

    return false;
}

//==============================================================================
void HeaderComponent::sidebarTabsWidthChanged (int newWidth)
{
    tabsWidth = newWidth;
    resized();
}

void HeaderComponent::liveBuildEnablementChanged (bool isEnabled)
{
    runAppButton.setVisible (isEnabled);
}

//==============================================================================
void HeaderComponent::changeListenerCallback (ChangeBroadcaster* source)
{
    if (source == &userAvatar)
    {
        resized();
    }
    else if (childProcess != nullptr && source == &childProcess->activityList)
    {
        if (childProcess->activityList.getNumActivities() > 0)
            buildPing();
        else
            buildFinished (childProcess->errorList.getNumErrors() == 0);
    }
}

void HeaderComponent::valueChanged (Value&)
{
    updateName();
}

void HeaderComponent::timerCallback()
{
    repaint();
}

//==============================================================================
void HeaderComponent::initialiseButtons()
{
    addAndMakeVisible (projectSettingsButton);
    projectSettingsButton.onClick = [this] { projectContentComponent->showProjectSettings(); };

    addAndMakeVisible (saveAndOpenInIDEButton);
    saveAndOpenInIDEButton.setBackgroundColour (Colours::white);
    saveAndOpenInIDEButton.setIconInset (7);
    saveAndOpenInIDEButton.onClick = [this]
    {
        if (project != nullptr)
        {
            if (project->hasIncompatibleLicenseTypeAndSplashScreenSetting())
            {
                auto child = project->getProjectMessages().getChildWithName (ProjectMessages::Ids::warning)
                                                          .getChildWithName (ProjectMessages::Ids::incompatibleLicense);

                if (child.isValid())
                    child.setProperty (ProjectMessages::Ids::isVisible, true, nullptr);
            }
            else
            {
                if (auto exporter = getSelectedExporter())
                    project->openProjectInIDE (*exporter, true);
            }
        }
    };

    addAndMakeVisible (runAppButton);
    runAppButton.setIconInset (7);
    runAppButton.onClick = [this]
    {
        if (childProcess != nullptr)
            childProcess->launchApp();
    };

    updateExporterButton();
}

void HeaderComponent::updateName()
{
    if (project != nullptr)
        projectNameLabel.setText (project->getDocumentTitle(), dontSendNotification);
}

void HeaderComponent::updateExporterButton()
{
    if (auto selectedExporter = getSelectedExporter())
    {
        auto selectedName = selectedExporter->getUniqueName();

        for (auto info : ProjectExporter::getExporterTypeInfos())
        {
            if (selectedName.contains (info.displayName))
            {
                saveAndOpenInIDEButton.setImage (info.icon);
                saveAndOpenInIDEButton.repaint();
                saveAndOpenInIDEButton.setEnabled (canCurrentExporterLaunchProject());
            }
        }
    }
}

//==============================================================================
void HeaderComponent::buildPing()
{
    if (! isTimerRunning())
    {
        isBuilding = true;
        runAppButton.setEnabled (false);
        runAppButton.setTooltip ("Building...");

        startTimer (50);
    }
}

void HeaderComponent::buildFinished (bool success)
{
    stopTimer();
    isBuilding = false;

    repaint();

    setRunAppButtonState (success);
}

void HeaderComponent::setRunAppButtonState (bool buildWasSuccessful)
{
    bool shouldEnableButton = false;

    if (buildWasSuccessful)
    {
        if (childProcess != nullptr)
        {
            if (childProcess->isAppRunning() || (! childProcess->isAppRunning() && childProcess->canLaunchApp()))
            {
                runAppButton.setTooltip ("Launch application");
                shouldEnableButton = true;
            }
            else
            {
                runAppButton.setTooltip ("Application can't be launched");
            }
        }
        else
        {
            runAppButton.setTooltip ("Enable live-build engine to launch application");
        }
    }
    else
    {
        runAppButton.setTooltip ("Error building application");
    }

    runAppButton.setEnabled (shouldEnableButton);
}
