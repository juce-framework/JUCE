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
HeaderComponent::HeaderComponent()
{
    addAndMakeVisible (configLabel);
    addAndMakeVisible (exporterBox);

    exporterBox.onChange = [this] { updateExporterButton(); };

    juceIcon.reset (new ImageComponent ("icon"));
    addAndMakeVisible (juceIcon.get());
    juceIcon->setImage (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize),
                        RectanglePlacement::centred);

    projectNameLabel.setText ({}, dontSendNotification);
    addAndMakeVisible (projectNameLabel);

    initialiseButtons();
}

HeaderComponent::~HeaderComponent()
{
    if (childProcess != nullptr)
    {
        childProcess->activityList.removeChangeListener(this);
        childProcess->errorList.removeChangeListener (this);
    }
}

//==============================================================================
void HeaderComponent::resized()
{
    auto bounds = getLocalBounds();
    configLabel.setFont ({ bounds.getHeight() / 3.0f });

    //==============================================================================
    {
        auto headerBounds = bounds.removeFromLeft (tabsWidth);

        const int buttonSize = 25;
        auto buttonBounds = headerBounds.removeFromRight (buttonSize);

        projectSettingsButton->setBounds (buttonBounds.removeFromBottom (buttonSize).reduced (2));

        juceIcon->setBounds (headerBounds.removeFromLeft (headerBounds.getHeight()).reduced (2));

        headerBounds.removeFromRight (5);
        projectNameLabel.setBounds (headerBounds);
    }

    //==============================================================================
    auto exporterWidth = jmin (400, bounds.getWidth() / 2);
    Rectangle<int> exporterBounds (0, 0, exporterWidth, bounds.getHeight());

    exporterBounds.setCentre (bounds.getCentre());

    runAppButton->setBounds (exporterBounds.removeFromRight (exporterBounds.getHeight()).reduced (2));
    saveAndOpenInIDEButton->setBounds (exporterBounds.removeFromRight (exporterBounds.getHeight()).reduced (2));

    exporterBounds.removeFromRight (5);
    exporterBox.setBounds (exporterBounds.removeFromBottom (roundToInt (exporterBounds.getHeight() / 1.8f)));
    configLabel.setBounds (exporterBounds);
}

void HeaderComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));

    if (isBuilding)
        getLookAndFeel().drawSpinningWaitAnimation (g, findColour (treeIconColourId),
                                                    runAppButton->getX(), runAppButton->getY(),
                                                    runAppButton->getWidth(), runAppButton->getHeight());
}

//==============================================================================
void HeaderComponent::setCurrentProject (Project* p) noexcept
{
    project = p;

    exportersTree = project->getExporters();
    exportersTree.addListener (this);
    updateExporters();

    projectNameValue.referTo (project->getProjectValue (Ids::name));
    projectNameValue.addListener (this);
    updateName();

    isBuilding = false;
    stopTimer();
    repaint();

    childProcess = ProjucerApplication::getApp().childProcessCache->getExisting (*project);

    if (childProcess != nullptr)
    {
        childProcess->activityList.addChangeListener (this);
        childProcess->errorList.addChangeListener (this);

        runAppButton->setTooltip ({});
        runAppButton->setEnabled (true);
    }
    else
    {
        runAppButton->setTooltip ("Enable live-build engine to launch application");
        runAppButton->setEnabled (false);
    }
}

//==============================================================================
void HeaderComponent::updateExporters() noexcept
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

        if (exporter->getName().contains (ProjectExporter::getCurrentPlatformExporterName()) && preferredExporterIndex == -1)
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

String HeaderComponent::getSelectedExporterName() const noexcept
{
    return exporterBox.getItemText (exporterBox.getSelectedItemIndex());
}

bool HeaderComponent::canCurrentExporterLaunchProject() const noexcept
{
    for (Project::ExporterIterator exporter (*project); exporter.next();)
        if (exporter->getName() == getSelectedExporterName() && exporter->canLaunchProject())
            return true;

    return false;
}

//==============================================================================
void HeaderComponent::sidebarTabsWidthChanged (int newWidth) noexcept
{
    tabsWidth = newWidth;
    resized();
}

//==============================================================================
void HeaderComponent::changeListenerCallback (ChangeBroadcaster*)
{
    if (childProcess != nullptr)
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
void HeaderComponent::initialiseButtons() noexcept
{
    auto& icons = getIcons();

    projectSettingsButton.reset (new IconButton ("Project Settings", &icons.settings));
    addAndMakeVisible (projectSettingsButton.get());
    projectSettingsButton->onClick = [this]
    {
        if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
            pcc->showProjectSettings();
    };

    saveAndOpenInIDEButton.reset (new IconButton ("Save and Open in IDE", nullptr));
    addAndMakeVisible (saveAndOpenInIDEButton.get());
    saveAndOpenInIDEButton->isIDEButton = true;
    saveAndOpenInIDEButton->onClick = [this]
    {
        if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
            pcc->openInSelectedIDE (true);
    };

    runAppButton.reset (new IconButton ("Run Application", &icons.play));
    addAndMakeVisible (runAppButton.get());
    runAppButton->onClick = [this]
    {
        if (childProcess != nullptr)
            childProcess->launchApp();
    };

    updateExporterButton();
}

void HeaderComponent::updateName() noexcept
{
    projectNameLabel.setText (project->getDocumentTitle(), dontSendNotification);
}

void HeaderComponent::updateExporterButton() noexcept
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

//==============================================================================
void HeaderComponent::buildPing()
{
    if (! isTimerRunning())
    {
        isBuilding = true;
        runAppButton->setEnabled (false);
        runAppButton->setTooltip ("Building...");

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
                runAppButton->setTooltip ("Launch application");
                shouldEnableButton = true;
            }
            else
            {
                runAppButton->setTooltip ("Application can't be launched");
            }
        }
        else
        {
            runAppButton->setTooltip ("Enable live-build engine to launch application");
        }
    }
    else
    {
        runAppButton->setTooltip ("Error building application");
    }

    runAppButton->setEnabled (shouldEnableButton);
}
