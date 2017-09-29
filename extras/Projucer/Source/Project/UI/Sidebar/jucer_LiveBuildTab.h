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


//==============================================================================
struct LiveBuildSettingsComponent  : public Component
{
    LiveBuildSettingsComponent (Project& p)
        : group ("Live Build Settings",
                 Icon (getIcons().settings, Colours::transparentBlack))
    {
        addAndMakeVisible (&group);

        PropertyListBuilder props;
        LiveBuildProjectSettings::getLiveSettings (p, props);

        group.setProperties (props);
        group.setName ("Live Build Settings");
    }

    void resized() override
    {
        group.updateSize (12, 0, getWidth() - 24);
        group.setBounds (getLocalBounds().reduced (12, 0));
    }

    void parentSizeChanged() override
    {
        const auto width = jmax (550, getParentWidth());
        auto y = group.updateSize (12, 0, width - 12);

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    PropertyGroupComponent group;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveBuildSettingsComponent)
};

//==============================================================================
class LiveBuildTab    : public Component,
                        private ChangeListener,
                        private Button::Listener
{
public:
    LiveBuildTab (CompileEngineChildProcess* child, String lastErrorMessage)
    {
        addAndMakeVisible (settingsButton = new IconButton ("Settings", &getIcons().settings));
        settingsButton->addListener (this);

        if (child != nullptr)
        {
            addAndMakeVisible (concertinaPanel);
            buildConcertina (child);
            isEnabled = true;
        }
        else
        {
            isEnabled = false;

            errorMessage = getErrorMessage();
            errorMessageLabel = new Label ("Error", errorMessage);
            errorMessageLabel->setJustificationType (Justification::centred);
            errorMessageLabel->setFont (Font (12.0f));
            errorMessageLabel->setMinimumHorizontalScale (1.0f);

            addAndMakeVisible (errorMessageLabel);

            if (showDownloadButton)
            {
                addAndMakeVisible (downloadButton = new TextButton ("Download"));
                downloadButton->addListener (this);
            }

            if (showEnableButton)
            {
                auto buttonText = "Enable Now";

                if (! lastErrorMessage.isEmpty())
                {
                    errorMessageLabel->setText (lastErrorMessage, dontSendNotification);
                    buttonText = "Re-enable";
                }

                addAndMakeVisible (enableButton = new TextButton (buttonText));
                enableButton->addListener (this);
            }
        }
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (secondaryBackgroundColourId));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();

        auto bottomSlice = bounds.removeFromBottom (25);
        bottomSlice.removeFromRight (5);
        settingsButton->setBounds (bottomSlice.removeFromRight (25).reduced (2));

        if (errorMessageLabel != nullptr)
        {
            bounds.removeFromTop ((bounds.getHeight() / 2) - 40);

            errorMessageLabel->setBounds (bounds.removeFromTop (80));

            if (downloadButton != nullptr)
                downloadButton->setBounds (bounds.removeFromTop (20).reduced (20, 0));

            if (enableButton != nullptr)
                enableButton->setBounds (bounds.removeFromTop (20).reduced (20, 0));
        }
        else
        {
            concertinaPanel.setBounds (bounds);

            for (auto h : headers)
                if (h->getName() == "Activities")
                    h->yPosition = getHeight() - CurrentActivitiesComp::getMaxPanelHeight() - 55;
        }
    }

    bool isEnabled;
    String errorMessage;
    Component::SafePointer<ProjucerAppClasses::ErrorListComp> errorListComp;

private:
    OwnedArray<ConcertinaHeader> headers;
    ConcertinaPanel concertinaPanel;
    ScopedPointer<IconButton> settingsButton;

    ScopedPointer<TextButton> downloadButton, enableButton;
    ScopedPointer<Label> errorMessageLabel;
    bool showDownloadButton;
    bool showEnableButton;

    Rectangle<int> textBounds;

    //==========================================================================
    String getErrorMessage()
    {
        showDownloadButton = false;
        showEnableButton = false;

        const auto osType = SystemStats::getOperatingSystemType();

        const bool isMac = (osType & SystemStats::MacOSX) != 0;
        const bool isWin = (osType & SystemStats::Windows) != 0;
        const bool isLinux = (osType & SystemStats::Linux) != 0;

        if (! isMac && ! isWin && ! isLinux)
            return String ("Live-build features are not supported on your system.\n\n"
                           "Please check supported platforms at www.juce.com!");

        if (isLinux)
            return String ("Live-build features for Linux are under development.\n\n"
                           "Please check for updates at www.juce.com!");

        if (isMac)
            if (osType < SystemStats::MacOSX_10_9)
                return String ("Live-build features are available only on MacOSX 10.9 or higher.");

        if (isWin)
            if (! SystemStats::isOperatingSystem64Bit() || osType < SystemStats::Windows8_0)
                return String ("Live-build features are available only on 64-Bit Windows 8 or higher.");

        const auto& compileEngineDll = *CompileEngineDLL::getInstance();
        const auto dllPresent = compileEngineDll.isLoaded();

        if (! dllPresent)
        {
            showDownloadButton = true;
            return String ("Download the live-build engine to get started");
        }

        showEnableButton = true;
        return String ("Enable compilation to use the live-build engine");
    }

    void buttonClicked (Button* b) override
    {
        auto* pcc = findParentComponentOfClass<ProjectContentComponent>();

        if (b == settingsButton)
        {
            if (pcc != nullptr)
                pcc->showLiveBuildSettings();
        }
        else if (b == downloadButton)
        {
            if (DownloadCompileEngineThread::downloadAndInstall())
            {
                if (! CompileEngineDLL::getInstance()->tryLoadDll())
                {
                    AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                                                "Download and install",
                                                "Loading the live-build engine failed");
                    return;
                }

                if (pcc != nullptr)
                    pcc->rebuildProjectTabs();
            }
        }
        else if (b == enableButton)
        {
            if (pcc != nullptr)
                pcc->setBuildEnabled (true);
        }
    }

    void buildConcertina (CompileEngineChildProcess* child)
    {
        for (int i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        errorListComp = new ProjucerAppClasses::ErrorListComp (child->errorList);
        auto* activities = new CurrentActivitiesComp (child->activityList);
        auto* comps = new ComponentListComp (*child);

        concertinaPanel.addPanel (-1, errorListComp, true);
        concertinaPanel.addPanel (-1, comps, true);
        concertinaPanel.addPanel (-1, activities, true);

        headers.add (new ConcertinaHeader ("Errors",     getIcons().bug));
        headers.add (new ConcertinaHeader ("Components", getIcons().modules));
        headers.add (new ConcertinaHeader ("Activities", getIcons().buildTab));

        for (int i = 0; i < concertinaPanel.getNumPanels(); ++i)
        {
            auto* p = concertinaPanel.getPanel (i);
            auto* h = headers.getUnchecked (i);

            h->addChangeListener (this);
            h->yPosition = i * 30;

            concertinaPanel.setCustomPanelHeader (p, h, false);
            concertinaPanel.setPanelHeaderSize (p, 30);
        }

        concertinaPanel.setMaximumPanelSize (activities, CurrentActivitiesComp::getMaxPanelHeight());
        concertinaPanel.setPanelSize (errorListComp, 200, false);
        concertinaPanel.setPanelSize (comps, 300, false);
    }

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (auto* header = dynamic_cast<ConcertinaHeader*> (source))
        {
            auto index = headers.indexOf (header);
            concertinaPanel.expandPanelFully (concertinaPanel.getPanel (index), true);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveBuildTab)
};
