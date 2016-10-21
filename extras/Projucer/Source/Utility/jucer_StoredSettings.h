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

#ifndef JUCER_STOREDSETTINGS_H_INCLUDED
#define JUCER_STOREDSETTINGS_H_INCLUDED

#include <map>
#include "../Application/jucer_AppearanceSettings.h"

//==============================================================================
class StoredSettings : public ValueTree::Listener
{
public:
    StoredSettings();
    ~StoredSettings();

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

    struct ColourSelectorWithSwatches    : public ColourSelector
    {
        ColourSelectorWithSwatches() {}

        int getNumSwatches() const override;
        Colour getSwatchColour (int index) const override;
        void setSwatchColour (int index, const Colour& newColour) const override;
    };

    //==============================================================================
    AppearanceSettings appearance;

    StringArray monospacedFontNames;

    //==============================================================================
    Value getGlobalPath (const Identifier& key, DependencyPathOS);
    String getFallbackPath (const Identifier& key, DependencyPathOS);

    bool isGlobalPathValid (const File& relativeTo, const Identifier& key, const String& path);

private:
    OwnedArray<PropertiesFile> propertyFiles;
    ValueTree projectDefaults;

    void changed()
    {
        ScopedPointer<XmlElement> data (projectDefaults.createXml());
        propertyFiles.getUnchecked (0)->setValue ("PROJECT_DEFAULT_SETTINGS", data);
    }

    void updateGlobalPreferences();
    void updateAppearanceSettings();
    void updateRecentFiles();
    void updateKeyMappings();

    void loadSwatchColours();
    void saveSwatchColours();

    //==============================================================================
    void valueTreePropertyChanged (ValueTree&, const Identifier&) override  { changed(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override              { changed(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override       { changed(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override         { changed(); }
    void valueTreeParentChanged (ValueTree&) override                       { changed(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StoredSettings)
};

StoredSettings& getAppSettings();
PropertiesFile& getGlobalProperties();


#endif   // JUCER_STOREDSETTINGS_H_INCLUDED
