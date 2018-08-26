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
    {
        addAndMakeVisible (&group);

        PropertyListBuilder props;
        p.getCompileEngineSettings().getLiveSettings (props);

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
        auto width = jmax (550, getParentWidth());
        auto y = group.updateSize (12, 0, width - 12);

        y = jmax (getParentHeight(), y);

        setSize (width, y);
    }

    PropertyGroupComponent group { "Live Build Settings", Icon (getIcons().settings, Colours::transparentBlack) };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LiveBuildSettingsComponent)
};

//==============================================================================
class LiveBuildTab    : public Component,
                        private ChangeListener
{
public:
    LiveBuildTab (const CompileEngineChildProcess::Ptr& child, String lastErrorMessage)
    {
        settingsButton.reset (new IconButton ("Settings", &getIcons().settings));
        addAndMakeVisible (settingsButton.get());
        settingsButton->onClick = [this]
        {
            if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->showLiveBuildSettings();
        };

        if (child != nullptr)
        {
            addAndMakeVisible (concertinaPanel);
            buildConcertina (*child);
            isEnabled = true;
        }
        else
        {
            errorMessage = getErrorMessage();
            errorMessageLabel.reset (new Label ("Error", errorMessage));
            errorMessageLabel->setJustificationType (Justification::centred);
            errorMessageLabel->setFont (Font (12.0f));
            errorMessageLabel->setMinimumHorizontalScale (1.0f);

            addAndMakeVisible (errorMessageLabel.get());

            if (showDownloadButton)
            {
                downloadButton.reset (new TextButton ("Download"));
                addAndMakeVisible (downloadButton.get());
                downloadButton->onClick = [this] { downloadDLL(); };
            }

            if (showEnableButton)
            {
                String buttonText ("Enable Now");

                if (! lastErrorMessage.isEmpty())
                {
                    errorMessageLabel->setText (lastErrorMessage, dontSendNotification);
                    buttonText = "Re-enable";
                }

                enableButton.reset (new TextButton (buttonText));
                addAndMakeVisible (enableButton.get());
                enableButton->onClick = [this]
                {
                    if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
                        pcc->setBuildEnabled (true);
                };
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

        settingsButton->setBounds (bounds.removeFromBottom (25)
                                         .removeFromRight  (25)
                                         .reduced (3));

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

    bool isEnabled = false;
    String errorMessage;
    Component::SafePointer<ProjucerAppClasses::ErrorListComp> errorListComp;

private:
    OwnedArray<ConcertinaHeader> headers;
    ConcertinaPanel concertinaPanel;
    std::unique_ptr<IconButton> settingsButton;

    std::unique_ptr<TextButton> downloadButton, enableButton;
    std::unique_ptr<Label> errorMessageLabel;
    bool showDownloadButton;
    bool showEnableButton;

    Rectangle<int> textBounds;

    //==========================================================================
    String getErrorMessage()
    {
        showDownloadButton = false;
        showEnableButton = false;

        auto osType = SystemStats::getOperatingSystemType();

        auto isMac = (osType & SystemStats::MacOSX) != 0;
        auto isWin = (osType & SystemStats::Windows) != 0;
        auto isLinux = (osType & SystemStats::Linux) != 0;

        if (! isMac && ! isWin && ! isLinux)
            return "Live-build features are not supported on your system.\n\n"
                   "Please check supported platforms at www.juce.com!";

        if (isLinux)
            return "Live-build features for Linux are under development.\n\n"
                   "Please check for updates at www.juce.com!";

        if (isMac)
            if (osType < SystemStats::MacOSX_10_9)
                return "Live-build features are available only on MacOSX 10.9 or higher.";

        if (isWin)
            if (! SystemStats::isOperatingSystem64Bit() || osType < SystemStats::Windows8_0)
                return "Live-build features are available only on 64-Bit Windows 8 or higher.";

        auto& compileEngineDll = *CompileEngineDLL::getInstance();
        auto dllPresent = compileEngineDll.isLoaded();

        if (! dllPresent)
        {
            showDownloadButton = true;
            return "Download the live-build engine to get started";
        }

        showEnableButton = true;
        return "Enable compilation to use the live-build engine";
    }

    void downloadDLL()
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

            if (auto* pcc = findParentComponentOfClass<ProjectContentComponent>())
                pcc->rebuildProjectTabs();
        }
    }

    void buildConcertina (CompileEngineChildProcess& child)
    {
        for (auto i = concertinaPanel.getNumPanels() - 1; i >= 0 ; --i)
            concertinaPanel.removePanel (concertinaPanel.getPanel (i));

        headers.clear();

        errorListComp = new ProjucerAppClasses::ErrorListComp (child.errorList);
        auto* activities = new CurrentActivitiesComp (child.activityList);
        auto* comps = new ComponentListComp (child);

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
