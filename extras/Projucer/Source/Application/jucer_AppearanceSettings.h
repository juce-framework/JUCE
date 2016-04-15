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

#ifndef JUCER_APPEARANCESETTINGS_H_INCLUDED
#define JUCER_APPEARANCESETTINGS_H_INCLUDED


class AppearanceSettings    : private ValueTree::Listener
{
public:
    AppearanceSettings (bool updateAppWhenChanged);

    bool readFromFile (const File& file);
    bool readFromXML (const XmlElement&);
    bool writeToFile (const File& file) const;

    void updateColourScheme();
    void applyToCodeEditor (CodeEditorComponent& editor) const;

    StringArray getColourNames() const;
    Value getColourValue (const String& colourName);
    bool getColour (const String& name, Colour& resultIfFound) const;

    Font getCodeFont() const;
    Value getCodeFontValue();

    ValueTree settings;

    static File getSchemesFolder();
    StringArray getPresetSchemes();
    void refreshPresetSchemeList();
    void selectPresetScheme (int index);

    static Font getDefaultCodeFont();

    static void showGlobalPreferences (ScopedPointer<Component>& ownerPointer);

    static const char* getSchemeFileSuffix()      { return ".scheme"; }
    static const char* getSchemeFileWildCard()    { return "*.scheme"; }

private:

    Array<File> presetSchemeFiles;

    static void writeDefaultSchemeFile (const String& xml, const String& name);

    void applyToLookAndFeel (LookAndFeel&) const;

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override   { updateColourScheme(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override               { updateColourScheme(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override        { updateColourScheme(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override          { updateColourScheme(); }
    void valueTreeParentChanged (ValueTree&) override                        { updateColourScheme(); }
    void valueTreeRedirected (ValueTree&) override                           { updateColourScheme(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettings)
};



#endif   // JUCER_APPEARANCESETTINGS_H_INCLUDED
