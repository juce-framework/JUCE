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

#include <map>
#include "jucer_AppearanceSettings.h"

//==============================================================================
class StoredSettings final : private ValueTree::Listener
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

    struct ColourSelectorWithSwatches final : public ColourSelector
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
