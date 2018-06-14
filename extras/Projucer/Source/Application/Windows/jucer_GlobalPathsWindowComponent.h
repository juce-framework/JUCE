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
class GlobalPathsWindowComponent    : public Component,
                                      private Timer
{
public:
    GlobalPathsWindowComponent()
    {
        addLabelsAndSetProperties();

        addAndMakeVisible (info);
        info.setInfoToDisplay ("Use this dropdown to set the global paths for different OSes. "
                               "\nN.B. These paths are stored locally and will only be used when "
                               "saving a project on this machine. Other machines will have their own "
                               "locally stored paths.");

        addAndMakeVisible (osSelector);
        osSelector.addItem ("OSX", 1);
        osSelector.addItem ("Windows", 2);
        osSelector.addItem ("Linux", 3);

        osSelector.onChange = [this]
        {
            addLabelsAndSetProperties();
            updateFilePathPropertyComponents();
        };

        auto os = TargetOS::getThisOS();

        if      (os == TargetOS::osx)     osSelector.setSelectedId (1);
        else if (os == TargetOS::windows) osSelector.setSelectedId (2);
        else if (os == TargetOS::linux)   osSelector.setSelectedId (3);

        updateFilePathPropertyComponents();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));
    }

    void paintOverChildren (Graphics& g) override
    {
        g.setColour (findColour (defaultHighlightColourId).withAlpha (flashAlpha));
        g.fillRect (boundsToHighlight);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (10);

        auto topSlice = b.removeFromTop (25);
        osSelector.setSize (200, 25);
        osSelector.setCentrePosition (topSlice.getCentre());

        info.setBounds (osSelector.getBounds().withWidth (osSelector.getHeight()).translated ((osSelector.getWidth() + 5), 0).reduced (2));

        int labelIndex = 0;
        bool isFirst = true;

        for (auto* pathComp : pathPropertyComponents)
        {
            if (pathComp == nullptr)
            {
                b.removeFromTop (15);
                pathPropertyLabels.getUnchecked (labelIndex++)->setBounds (b.removeFromTop (20));
                b.removeFromTop (20);
            }
            else
            {
                if (isFirst)
                    b.removeFromTop (20);

                pathComp->setBounds (b.removeFromTop (pathComp->getPreferredHeight()));
                b.removeFromTop (5);
            }

            isFirst = false;
        }
    }

    void highlightJUCEPath()
    {
        if (! isTimerRunning() && isSelectedOSThisOS())
        {
            if (auto* jucePathComp = pathPropertyComponents.getFirst())
                boundsToHighlight = jucePathComp->getBounds();

            flashAlpha = 0.0f;
            hasFlashed = false;

            startTimer (25);
        }
    }

private:
    OwnedArray<Label> pathPropertyLabels;
    OwnedArray<PropertyComponent> pathPropertyComponents;

    ComboBox osSelector;
    InfoButton info;

    Rectangle<int> boundsToHighlight;
    float flashAlpha = 0.0f;
    bool hasFlashed = false;

    //==============================================================================
    void timerCallback() override
    {
        flashAlpha += (hasFlashed ? -0.05f : 0.05f);

        if (flashAlpha > 0.75f)
        {
            hasFlashed = true;
        }
        else if (flashAlpha < 0.0f)
        {
            flashAlpha = 0.0f;
            boundsToHighlight = {};

            stopTimer();
        }

        repaint();
    }

    //==============================================================================
    bool isSelectedOSThisOS()    { return TargetOS::getThisOS() == getSelectedOS(); }

    TargetOS::OS getSelectedOS() const
    {
        auto selectedOS = TargetOS::unknown;

        switch (osSelector.getSelectedId())
        {
            case 1: selectedOS = TargetOS::osx;     break;
            case 2: selectedOS = TargetOS::windows; break;
            case 3: selectedOS = TargetOS::linux;   break;
            default:                                break;
        }

        return selectedOS;
    }

    void updateFilePathPropertyComponents()
    {
        pathPropertyComponents.clear();

        auto& settings = getAppSettings();

        if (isSelectedOSThisOS())
        {
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::jucePath),
                                                                                          "Path to JUCE", true)));

            pathPropertyComponents.add (nullptr);

            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::defaultJuceModulePath),
                                                                                          "JUCE Modules", true)));
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::defaultUserModulePath),
                                                                                          "User Modules", true, {}, {}, true)));

            pathPropertyComponents.add (nullptr);

            if (getSelectedOS() == TargetOS::linux)
            {
                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent ({}, "RTAS SDK", true)));
                pathPropertyComponents.getLast()->setEnabled (false);

                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent ({}, "AAX SDK", true)));
                pathPropertyComponents.getLast()->setEnabled (false);
            }
            else
            {
                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::rtasPath),
                                                                                              "RTAS SDK", true)));
                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::aaxPath),
                                                                                              "AAX SDK", true)));
            }

            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::androidSDKPath),
                                                                                          "Android SDK", true)));
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::androidNDKPath),
                                                                                          "Android NDK", true)));

            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::vst3Path),
                                                                                          "Custom VST3 SDK", true)));

            pathPropertyComponents.add (nullptr);

           #if JUCE_MAC
            String exeLabel ("app");
           #elif JUCE_WINDOWS
            String exeLabel ("executable");
           #else
            String exeLabel ("startup script");
           #endif
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::clionExePath),
                                                                                          "CLion " + exeLabel, false)));
        }
        else
        {
            auto selectedOS = getSelectedOS();
            auto maxChars = 1024;

            pathPropertyComponents.add (nullptr);

            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::defaultJuceModulePath, selectedOS),
                                                                                      "JUCE Modules", maxChars, false)));
            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::defaultUserModulePath, selectedOS),
                                                                                      "User Modules", maxChars, false)));

            pathPropertyComponents.add (nullptr);

            if (selectedOS == TargetOS::linux)
            {
                addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (Value(), "RTAS SDK", maxChars, false)));
                pathPropertyComponents.getLast()->setEnabled (false);

                addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (Value(), "AAX SDK", maxChars, false)));
                pathPropertyComponents.getLast()->setEnabled (false);
            }
            else
            {
                addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::rtasPath, selectedOS),
                                                                                          "RTAS SDK", maxChars, false)));
                addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::aaxPath, selectedOS),
                                                                                          "AAX SDK", maxChars, false)));
            }


            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::androidSDKPath, selectedOS),
                                                                                      "Android SDK", maxChars, false)));
            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::androidNDKPath, selectedOS),
                                                                                      "Android NDK", maxChars, false)));

            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::vst3Path, selectedOS),
                                                                                      "Custom VST3 SDK", maxChars, false)));
        }

        resized();
    }

    void addLabelsAndSetProperties()
    {
        pathPropertyLabels.clear();

        pathPropertyLabels.add (new Label ("modulesLabel", "Modules"));
        pathPropertyLabels.add (new Label ("sdksLabel", "SDKs"));
        pathPropertyLabels.add (new Label ("otherLabel", "Other"));

        for (auto* l : pathPropertyLabels)
        {
            addAndMakeVisible (l);
            l->setFont (Font (18.0f, Font::FontStyleFlags::bold));
            l->setJustificationType (Justification::centredLeft);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPathsWindowComponent)
};
