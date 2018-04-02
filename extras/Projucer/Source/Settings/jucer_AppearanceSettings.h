/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-license
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
class AppearanceSettings    : private ValueTree::Listener
{
public:
    AppearanceSettings (bool updateAppWhenChanged);

    bool readFromFile (const File& file);
    bool readFromXML (const XmlElement&);
    bool writeToFile (const File& file) const;

    void updateColorScheme();
    void applyToCodeEditor (CodeEditorComponent& editor) const;

    StringArray getColorNames() const;
    Value getColorValue (const String& colorName);
    bool getColor (const String& name, Color& resultIfFound) const;

    Font getCodeFont() const;
    Value getCodeFontValue();

    ValueTree settings;

    static File getSchemesFolder();
    StringArray getPresetSchemes();
    void refreshPresetSchemeList();
    void selectPresetScheme (int index);

    static Font getDefaultCodeFont();

    static const char* getSchemeFileSuffix()      { return ".scheme"; }
    static const char* getSchemeFileWildCard()    { return "*.scheme"; }

private:

    Array<File> presetSchemeFiles;

    static void writeDefaultSchemeFile (const String& xml, const String& name);

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override   { updateColorScheme(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override               { updateColorScheme(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override        { updateColorScheme(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override          { updateColorScheme(); }
    void valueTreeParentChanged (ValueTree&) override                        { updateColorScheme(); }
    void valueTreeRedirected (ValueTree&) override                           { updateColorScheme(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettings)
};
