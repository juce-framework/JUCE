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
