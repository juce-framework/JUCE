/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_APPEARANCESETTINGS_H_34D762C7__
#define __JUCER_APPEARANCESETTINGS_H_34D762C7__


class AppearanceSettings    : private ValueTree::Listener
{
public:
    AppearanceSettings();

    bool readFromFile (const File& file);
    bool readFromXML (const XmlElement&);
    bool writeToFile (const File& file) const;

    void applyToCodeEditor (CodeEditorComponent& editor) const;

    StringArray getColourNames() const;
    Value getColourValue (const String& colourName);
    bool getColour (const String& name, Colour& resultIfFound) const;

    Font getCodeFont() const;
    Value getCodeFontValue();

    ValueTree settings;

    File getSchemesFolder();
    StringArray getPresetSchemes();
    void refreshPresetSchemeList();
    void selectPresetScheme (int index);

    static Component* createEditorWindow();

private:
    static const char* getSchemeFileSuffix()      { return ".editorscheme"; }

    Array<File> presetSchemeFiles;

    void applyToLookAndFeel (LookAndFeel&) const;
    void updateColourScheme();

    void valueTreePropertyChanged (ValueTree&, const Identifier&)   { updateColourScheme(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&)               { updateColourScheme(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&)             { updateColourScheme(); }
    void valueTreeChildOrderChanged (ValueTree&)                    { updateColourScheme(); }
    void valueTreeParentChanged (ValueTree&)                        { updateColourScheme(); }
    void valueTreeRedirected (ValueTree&)                           { updateColourScheme(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettings);
};

//==============================================================================
class IntrojucerLookAndFeel   : public LookAndFeel
{
public:
    IntrojucerLookAndFeel();

    void drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/, bool isMouseOver, bool isMouseDragging)
    {
        if (isMouseOver || isMouseDragging)
            g.fillAll (Colours::grey.withAlpha (0.4f));
    }

    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent& component);
};


#endif
