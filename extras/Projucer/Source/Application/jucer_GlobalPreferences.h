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

#ifndef JUCER_GLOBALPREFERENCES_H_INCLUDED
#define JUCER_GLOBALPREFERENCES_H_INCLUDED

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


#endif  // JUCER_GLOBALPREFERENCES_H_INCLUDED
