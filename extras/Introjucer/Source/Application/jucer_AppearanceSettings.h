/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef __JUCER_APPEARANCESETTINGS_JUCEHEADER__
#define __JUCER_APPEARANCESETTINGS_JUCEHEADER__


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

    static void showEditorWindow (ScopedPointer<Component>& ownerPointer);

    static const char* getSchemeFileSuffix()      { return ".scheme"; }
    static const char* getSchemeFileWildCard()    { return "*.scheme"; }

private:

    Array<File> presetSchemeFiles;

    static void writeDefaultSchemeFile (const String& xml, const String& name);

    void applyToLookAndFeel (LookAndFeel&) const;

    void valueTreePropertyChanged (ValueTree&, const Identifier&)   { updateColourScheme(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&)               { updateColourScheme(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&)             { updateColourScheme(); }
    void valueTreeChildOrderChanged (ValueTree&)                    { updateColourScheme(); }
    void valueTreeParentChanged (ValueTree&)                        { updateColourScheme(); }
    void valueTreeRedirected (ValueTree&)                           { updateColourScheme(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppearanceSettings)
};

//==============================================================================
class IntrojucerLookAndFeel   : public LookAndFeel
{
public:
    IntrojucerLookAndFeel();

    void fillWithBackgroundTexture (Graphics&);
    static void fillWithBackgroundTexture (Component&, Graphics&);

    int getTabButtonOverlap (int tabDepth);
    int getTabButtonSpaceAroundImage();
    int getTabButtonBestWidth (TabBarButton& button, int tabDepth);
    static Colour getTabBackgroundColour (TabBarButton& button);
    void createTabTextLayout (const TabBarButton& button, const Rectangle<int>& textArea, GlyphArrangement& textLayout);
    void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown);

    Rectangle<int> getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp);
    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int, int) {}

    void drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/, bool isMouseOver, bool isMouseDragging);
    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&);

    bool areScrollbarButtonsVisible()   { return false; }

    void drawScrollbar (Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height, bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize, bool /*isMouseOver*/, bool /*isMouseDown*/);

    void drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                    bool isMouseOver, bool isMouseDown,
                                    ConcertinaPanel& concertina, Component& panel);

    void drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown);

    static Colour getScrollbarColourForBackground (const Colour& background);

private:
    Image backgroundTexture;
    Colour backgroundTextureBaseColour;
};


#endif   // __JUCER_APPEARANCESETTINGS_JUCEHEADER__
