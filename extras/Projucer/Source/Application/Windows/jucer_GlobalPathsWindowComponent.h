/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../../Utility/UI/PropertyComponents/jucer_LabelPropertyComponent.h"

//==============================================================================
class GlobalPathsWindowComponent final : public Component,
                                         private Timer,
                                         private Value::Listener,
                                         private ChangeListener
{
public:
    GlobalPathsWindowComponent()
    {
        addChildComponent (rescanJUCEPathButton);
        rescanJUCEPathButton.onClick = [this]
        {
            ProjucerApplication::getApp().rescanJUCEPathModules();
            lastJUCEModulePath = getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()).get();
        };

        addChildComponent (rescanUserPathButton);
        rescanUserPathButton.onClick = [this]
        {
            ProjucerApplication::getApp().rescanUserPathModules();
            lastUserModulePath = getAppSettings().getStoredPath (Ids::defaultUserModulePath, TargetOS::getThisOS()).get();
        };

        addChildComponent (warnAboutJUCEPathButton);
        warnAboutJUCEPathButton.setToggleState (ProjucerApplication::getApp().shouldPromptUserAboutIncorrectJUCEPath(),
                                                dontSendNotification);
        warnAboutJUCEPathButton.onClick = [this]
        {
            ProjucerApplication::getApp().setShouldPromptUserAboutIncorrectJUCEPath (warnAboutJUCEPathButton.getToggleState());
        };

        getGlobalProperties().addChangeListener (this);

        addAndMakeVisible (resetToDefaultsButton);
        resetToDefaultsButton.onClick = [this] { resetCurrentOSPathsToDefaults(); };

        addAndMakeVisible (propertyViewport);
        propertyViewport.setViewedComponent (&propertyGroup, false);

        auto os = TargetOS::getThisOS();

        if      (os == TargetOS::osx)     selectedOSValue = "osx";
        else if (os == TargetOS::windows) selectedOSValue = "windows";
        else if (os == TargetOS::linux)   selectedOSValue = "linux";

        selectedOSValue.addListener (this);

        buildProps();

        lastJUCEModulePath = getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS()).get();
        lastUserModulePath = getAppSettings().getStoredPath (Ids::defaultUserModulePath, TargetOS::getThisOS()).get();
    }

    ~GlobalPathsWindowComponent() override
    {
        getGlobalProperties().removeChangeListener (this);

        auto juceValue = getAppSettings().getStoredPath (Ids::defaultJuceModulePath, TargetOS::getThisOS());
        auto userValue = getAppSettings().getStoredPath (Ids::defaultUserModulePath, TargetOS::getThisOS());

        if (juceValue.get() != lastJUCEModulePath)  ProjucerApplication::getApp().rescanJUCEPathModules();
        if (userValue.get() != lastUserModulePath)  ProjucerApplication::getApp().rescanUserPathModules();
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

        auto bottomBounds = b.removeFromBottom (80);
        auto buttonBounds = bottomBounds.removeFromBottom (50);

        rescanJUCEPathButton.setBounds (buttonBounds.removeFromLeft (150).reduced (5, 10));
        rescanUserPathButton.setBounds (buttonBounds.removeFromLeft (150).reduced (5, 10));

        resetToDefaultsButton.setBounds (buttonBounds.removeFromRight (150).reduced (5, 10));
        warnAboutJUCEPathButton.setBounds (bottomBounds.reduced (0, 5));
        warnAboutJUCEPathButton.changeWidthToFitText();

        propertyGroup.updateSize (0, 0, getWidth() - 20 - propertyViewport.getScrollBarThickness());
        propertyViewport.setBounds (b);
    }

    void highlightJUCEPath()
    {
        if (isTimerRunning() || ! isSelectedOSThisOS())
            return;

        const auto findJucePathPropertyComponent = [this]() -> PropertyComponent*
        {
            for (const auto& prop : propertyGroup.getProperties())
                if (prop->getName() == "Path to JUCE")
                    return prop.get();

            return nullptr;
        };

        if (auto* propComponent = findJucePathPropertyComponent())
        {
            boundsToHighlight = getLocalArea (nullptr, propComponent->getScreenBounds());
            flashAlpha = 0.0f;
            hasFlashed = false;

            startTimer (25);
        }
    }

private:
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

    void valueChanged (Value&) override
    {
        buildProps();
        resized();
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        warnAboutJUCEPathButton.setToggleState (ProjucerApplication::getApp().shouldPromptUserAboutIncorrectJUCEPath(),
                                                dontSendNotification);
    }

    //==============================================================================
    bool isSelectedOSThisOS()    { return TargetOS::getThisOS() == getSelectedOS(); }

    TargetOS::OS getSelectedOS() const
    {
        auto val = selectedOSValue.getValue();

        if      (val == "osx")      return TargetOS::osx;
        else if (val == "windows")  return TargetOS::windows;
        else if (val == "linux")    return TargetOS::linux;

        jassertfalse;
        return TargetOS::unknown;
    }

    //==============================================================================
    void buildProps()
    {
        updateValues();

        PropertyListBuilder builder;
        auto isThisOS = isSelectedOSThisOS();

        builder.add (new ChoicePropertyComponent (selectedOSValue, "OS", { "OSX", "Windows", "Linux" }, { "osx", "windows", "linux" }),
                     "Use this dropdown to set the global paths for different OSes. "
                     "\nN.B. These paths are stored locally and will only be used when "
                     "saving a project on this machine. Other machines will have their own "
                     "locally stored paths.");

        builder.add (new LabelPropertyComponent ("JUCE"), {});

        builder.add (new FilePathPropertyComponent (jucePathValue, "Path to JUCE", true, isThisOS),
                     "This should be the path to the top-level directory of your JUCE folder. "
                     "This path will be used when searching for the JUCE examples and DemoRunner application.");

        builder.add (new FilePathPropertyComponent (juceModulePathValue, "JUCE Modules", true, isThisOS),
                     String ("This should be the path to the folder containing the JUCE modules that you wish to use, typically the \"modules\" directory of your JUCE folder.")
                     + (isThisOS ? " Use the button below to re-scan a new path." : ""));
        builder.add (new FilePathPropertyComponent (userModulePathValue, "User Modules", true, isThisOS),
                     String ("A path to a folder containing any custom modules that you wish to use.")
                     + (isThisOS ? " Use the button below to re-scan new paths." : ""));

        builder.add (new LabelPropertyComponent ("SDKs"), {});

        builder.add (new FilePathPropertyComponent (vstPathValue,  "VST (Legacy) SDK", true, isThisOS),
                     "If you are building a legacy VST plug-in then this path should point to a VST2 SDK. "
                     "The VST2 SDK can be obtained from the vstsdk3610_11_06_2018_build_37 (or older) VST3 SDK or JUCE version 5.3.2. "
                     "You also need a VST2 license from Steinberg to distribute VST2 plug-ins.");
        builder.add (new FilePathPropertyComponent (araPathValue, "ARA SDK", true, isThisOS),
                     "If you are building ARA enabled plug-ins, this should be the path to the ARA SDK folder.");

        if (getSelectedOS() != TargetOS::linux)
        {
            builder.add (new FilePathPropertyComponent (aaxPathValue, "AAX SDK", true, isThisOS),
                         "If you need to use a custom version of the AAX SDK, this should be the path to the AAX SDK folder. "
                         "JUCE bundles a copy of the AAX SDK, so you normally shouldn't need to set this.");
        }

        builder.add (new FilePathPropertyComponent (androidSDKPathValue, "Android SDK", true, isThisOS),
                     "This path will be used when writing the local.properties file of an Android project and should point to the Android SDK folder.");

        if (isThisOS)
        {
            builder.add (new LabelPropertyComponent ("Other"), {});

           #if JUCE_MAC
            String exeLabel ("app");
           #elif JUCE_WINDOWS
            String exeLabel ("executable");
           #else
            String exeLabel ("startup script");
           #endif

            builder.add (new FilePathPropertyComponent (androidStudioExePathValue, "Android Studio " + exeLabel, false, isThisOS),
                         "This path will be used for the \"Save Project and Open in IDE...\" option of the Android Studio exporter.");
        }

        rescanJUCEPathButton.setVisible (isThisOS);
        rescanUserPathButton.setVisible (isThisOS);
        warnAboutJUCEPathButton.setVisible (isThisOS);

        propertyGroup.setProperties (builder);
    }

    void updateValues()
    {
        auto& settings = getAppSettings();
        auto os = getSelectedOS();

        jucePathValue             = settings.getStoredPath (Ids::jucePath, os);
        juceModulePathValue       = settings.getStoredPath (Ids::defaultJuceModulePath, os);
        userModulePathValue       = settings.getStoredPath (Ids::defaultUserModulePath, os);
        vstPathValue              = settings.getStoredPath (Ids::vstLegacyPath, os);
        aaxPathValue              = settings.getStoredPath (Ids::aaxPath, os);
        araPathValue              = settings.getStoredPath (Ids::araPath, os);
        androidSDKPathValue       = settings.getStoredPath (Ids::androidSDKPath, os);
        androidStudioExePathValue = settings.getStoredPath (Ids::androidStudioExePath, os);
    }

    void resetCurrentOSPathsToDefaults()
    {
        jucePathValue            .resetToDefault();
        juceModulePathValue      .resetToDefault();
        userModulePathValue      .resetToDefault();
        vstPathValue             .resetToDefault();
        aaxPathValue             .resetToDefault();
        araPathValue             .resetToDefault();
        androidSDKPathValue      .resetToDefault();
        androidStudioExePathValue.resetToDefault();

        repaint();
    }

    //==============================================================================
    Value selectedOSValue;

    ValueTreePropertyWithDefault jucePathValue, juceModulePathValue, userModulePathValue,
                                 vstPathValue, aaxPathValue, araPathValue, androidSDKPathValue,
                                 androidStudioExePathValue;

    Viewport propertyViewport;
    PropertyGroupComponent propertyGroup  { "Global Paths", { getIcons().openFolder, Colours::transparentBlack } };

    ToggleButton warnAboutJUCEPathButton { "Warn about incorrect JUCE path" };
    TextButton rescanJUCEPathButton  { "Re-scan JUCE Modules" },
               rescanUserPathButton  { "Re-scan User Modules" },
               resetToDefaultsButton { "Reset to Defaults" };

    Rectangle<int> boundsToHighlight;
    float flashAlpha = 0.0f;
    bool hasFlashed = false;

    var lastJUCEModulePath, lastUserModulePath;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPathsWindowComponent)
};
