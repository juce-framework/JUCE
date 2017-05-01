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

#include "../Project/jucer_DependencyPathPropertyComponent.h"


class GlobalPreferencesTab
{
public:
    virtual ~GlobalPreferencesTab() {}

    virtual Component* getContent() = 0;
    virtual String getName() const noexcept = 0;
};

//==============================================================================
/** This component implements the "Paths" tab in the global preferences window,
    which defines the default paths for dependencies like third-party SDKs
    for this machine.
*/
class PathSettingsTab  : public GlobalPreferencesTab,
                         public Component,
                         private TextPropertyComponent::Listener
{
public:
    PathSettingsTab (DependencyPathOS);
    ~PathSettingsTab();

    Component* getContent() override;
    String getName() const noexcept override;

    void resized() override;

private:
    void textPropertyComponentChanged (TextPropertyComponent*) override;

    Identifier getKeyForPropertyComponent (TextPropertyComponent*) const;

    OwnedArray<TextPropertyComponent> pathComponents;

    TextPropertyComponent* vst3PathComponent;
    TextPropertyComponent* rtasPathComponent;
    TextPropertyComponent* aaxPathComponent;
    TextPropertyComponent* androidSdkPathComponent;
    TextPropertyComponent* androidNdkPathComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathSettingsTab)
};

//==============================================================================
/** This component implements the "Code Editor" tabl in the global preferences window,
    which sets font sizes and colours for the Projucer's code editor.
    The content is either an EditorPanel (the actual settings tab) or a FontScanPanel
    (shown if the tab is scanning for available fonts before showing the EditorPanel).
*/
class AppearanceSettingsTab  : public GlobalPreferencesTab,
                               public Component
{
public:
    AppearanceSettingsTab();

    Component* getContent() override;
    void changeContent (Component* newContent);
    String getName() const noexcept override;

    void resized() override;

private:
    ScopedPointer<Component> content;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettingsTab)
};

//==============================================================================
class GlobalPreferencesComponent  : public TabbedComponent
{
public:
    GlobalPreferencesComponent();

private:
    OwnedArray<GlobalPreferencesTab> preferenceTabs;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GlobalPreferencesComponent)
};
