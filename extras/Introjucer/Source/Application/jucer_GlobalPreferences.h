/*
  ==============================================================================

    jucer_GlobalPreferences.h
    Created: 22 Jul 2015 11:05:35am
    Author:  Timur Doumler

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

    TextPropertyComponent* vst2PathComponent;
    TextPropertyComponent* vst3PathComponent;
    TextPropertyComponent* rtasPathComponent;
    TextPropertyComponent* aaxPathComponent;
    TextPropertyComponent* androidSdkPathComponent;
    TextPropertyComponent* androidNdkPathComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PathSettingsTab)
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
