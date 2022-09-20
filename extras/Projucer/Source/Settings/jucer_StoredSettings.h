/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include <map>
#include "jucer_AppearanceSettings.h"

//==============================================================================
class StoredSettings   : private ValueTree::Listener
{
public:
    StoredSettings();
    ~StoredSettings() override;

    PropertiesFile& getGlobalProperties();
    PropertiesFile& getProjectProperties (const String& projectUID);

    void flush();
    void reload();

    //==============================================================================
    RecentlyOpenedFilesList recentFiles;

    Array<File> getLastProjects();
    void setLastProjects (const Array<File>& files);

    //==============================================================================
    Array<Colour> swatchColours;

    struct ColourSelectorWithSwatches   : public ColourSelector
    {
        ColourSelectorWithSwatches();
        ~ColourSelectorWithSwatches() override;

        int getNumSwatches() const override;
        Colour getSwatchColour (int index) const override;
        void setSwatchColour (int index, const Colour& newColour) override;
    };

    //==============================================================================
    void addProjectDefaultsListener (ValueTree::Listener&);
    void removeProjectDefaultsListener (ValueTree::Listener&);

    void addFallbackPathsListener (ValueTree::Listener&);
    void removeFallbackPathsListener (ValueTree::Listener&);

    ValueTreePropertyWithDefault getStoredPath (const Identifier& key, DependencyPathOS os);
    bool isJUCEPathIncorrect();

    //==============================================================================
    AppearanceSettings appearance;
    StringArray monospacedFontNames;
    File lastWizardFolder;

private:
    //==============================================================================
    void updateGlobalPreferences();
    void updateRecentFiles();
    void updateLastWizardFolder();
    void updateKeyMappings();

    void loadSwatchColours();
    void saveSwatchColours();

    void updateOldProjectSettingsFiles();
    void checkJUCEPaths();

    //==============================================================================
    void changed (bool);

    void valueTreePropertyChanged (ValueTree& vt, const Identifier&) override  { changed (vt == projectDefaults); }
    void valueTreeChildAdded (ValueTree& vt, ValueTree&) override              { changed (vt == projectDefaults); }
    void valueTreeChildRemoved (ValueTree& vt, ValueTree&, int) override       { changed (vt == projectDefaults); }
    void valueTreeChildOrderChanged (ValueTree& vt, int, int) override         { changed (vt == projectDefaults); }
    void valueTreeParentChanged (ValueTree& vt) override                       { changed (vt == projectDefaults); }

    //==============================================================================
    OwnedArray<PropertiesFile> propertyFiles;
    ValueTree projectDefaults;
    ValueTree fallbackPaths;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings)
};

StoredSettings& getAppSettings();
PropertiesFile& getGlobalProperties();
