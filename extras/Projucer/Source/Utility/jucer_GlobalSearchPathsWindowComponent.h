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

class GlobalSearchPathsWindowComponent    : public Component,
                                            private ComboBox::Listener
{
public:
    GlobalSearchPathsWindowComponent()
        : modulesLabel ("modulesLabel", "Modules"),
          sdksLabel ("sdksLabel", "SDKs")
    {
        addAndMakeVisible (modulesLabel);
        addAndMakeVisible (sdksLabel);

        modulesLabel.setFont (Font (18.0f, Font::FontStyleFlags::bold));
        sdksLabel.setFont (Font (18.0f, Font::FontStyleFlags::bold));

        modulesLabel.setJustificationType (Justification::centredLeft);
        sdksLabel.setJustificationType (Justification::centredLeft);

        addAndMakeVisible (info);
        info.setInfoToDisplay ("Use this dropdown to set the global paths for different OSes. "
                               "\nN.B. These paths are stored locally and will only be used when "
                               "saving a project on this machine. Other machines will have their own "
                               "locally stored paths.");

        addAndMakeVisible (osSelector);
        osSelector.addItem ("OSX", 1);
        osSelector.addItem ("Windows", 2);
        osSelector.addItem ("Linux", 3);

        osSelector.addListener (this);

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

    void resized() override
    {
        auto b = getLocalBounds().reduced (10);

        auto topSlice = b.removeFromTop (25);
        osSelector.setSize (200, 25);
        osSelector.setCentrePosition (topSlice.getCentre());

        info.setBounds (osSelector.getBounds().withWidth (osSelector.getHeight()).translated ((osSelector.getWidth() + 5), 0).reduced (2));

        modulesLabel.setBounds (b.removeFromTop (20));
        b.removeFromTop (20);

        auto i = 0;
        for (auto propertyComponent : pathPropertyComponents)
        {
            propertyComponent->setBounds (b.removeFromTop (propertyComponent->getPreferredHeight()));
            b.removeFromTop (5);

            if (i == 1)
            {
                b.removeFromTop (15);
                sdksLabel.setBounds (b.removeFromTop (20));
                b.removeFromTop (20);
            }

            ++i;
        }
    }

private:
    Label modulesLabel, sdksLabel;
    OwnedArray<PropertyComponent> pathPropertyComponents;
    ComboBox osSelector;
    ConfigTreeItemTypes::InfoButton info;

    void comboBoxChanged (ComboBox*) override
    {
        updateFilePathPropertyComponents();
    }

    void updateFilePathPropertyComponents()
    {
        pathPropertyComponents.clear();

        auto thisOS = TargetOS::getThisOS();
        auto selectedOS = TargetOS::unknown;

        switch (osSelector.getSelectedId())
        {
            case 1: selectedOS = TargetOS::osx;     break;
            case 2: selectedOS = TargetOS::windows; break;
            case 3: selectedOS = TargetOS::linux;   break;
            default:                                break;
        }

        auto& settings = getAppSettings();

        if (selectedOS == thisOS)
        {
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::defaultJuceModulePath),
                                                                                          "JUCE Modules", true)));
            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::defaultUserModulePath),
                                                                                          "User Modules", true, {}, {}, true)));

            addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (settings.getStoredPath (Ids::vst3Path),
                                                                                          "VST3 SDK", true)));

            if (selectedOS == TargetOS::linux)
            {
                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (Value(), "RTAS SDK", true)));
                pathPropertyComponents.getLast()->setEnabled (false);

                addAndMakeVisible (pathPropertyComponents.add (new FilePathPropertyComponent (Value(), "AAX SDK", true)));
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
        }
        else
        {
            auto maxChars = 1024;

            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::defaultJuceModulePath, selectedOS),
                                                                                      "JUCE Modules", maxChars, false)));
            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::defaultUserModulePath, selectedOS),
                                                                                      "User Modules", maxChars, false)));

            addAndMakeVisible (pathPropertyComponents.add (new TextPropertyComponent (settings.getFallbackPathForOS (Ids::vst3Path, selectedOS),
                                                                                      "VST3 SDK", maxChars, false)));

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
        }

        resized();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalSearchPathsWindowComponent)
};
