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

    int getTabButtonOverlap (int tabDepth)                          { return -1; }
    int getTabButtonSpaceAroundImage()                              { return 1; }
    int getTabButtonBestWidth (TabBarButton& button, int tabDepth)  { return 120; }

    void createTabTextLayout (const TabBarButton& button, const Rectangle<int>& textArea, GlyphArrangement& textLayout)
    {
        Font font (textArea.getHeight() * 0.5f);
        font.setUnderline (button.hasKeyboardFocus (false));

        textLayout.addFittedText (font, button.getButtonText().trim(),
                                  (float) textArea.getX(), (float) textArea.getY(), (float) textArea.getWidth(), (float) textArea.getHeight(),
                                  Justification::centred, 1);
    }

    static Colour getTabBackgroundColour (TabBarButton& button)
    {
        Colour normalBkg (button.getTabBackgroundColour());
        Colour bkg (normalBkg.contrasting (0.15f));
        if (button.isFrontTab())
            bkg = bkg.overlaidWith (Colours::yellow.withAlpha (0.5f));

        return bkg;
    }

    void drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
    {
        const Rectangle<int> activeArea (button.getActiveArea());

        Colour bkg (getTabBackgroundColour (button));

        g.setGradientFill (ColourGradient (bkg.brighter (0.1f), 0, (float) activeArea.getY(),
                                           bkg.darker (0.1f), 0, (float) activeArea.getBottom(), false));
        g.fillRect (activeArea);

        g.setColour (button.getTabBackgroundColour().darker (0.3f));
        g.drawRect (activeArea);

        GlyphArrangement textLayout;
        createTabTextLayout (button, button.getTextArea(), textLayout);

        const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
        g.setColour (bkg.contrasting().withMultipliedAlpha (alpha));
        textLayout.draw (g);
    }

    Rectangle<int> getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp)
    {
        GlyphArrangement textLayout;
        createTabTextLayout (button, textArea, textLayout);
        const int textWidth = (int) textLayout.getBoundingBox (0, -1, false).getWidth();
        const int extraSpace = jmax (0, textArea.getWidth() - (textWidth + comp.getWidth())) / 2;

        textArea.removeFromRight (extraSpace);
        textArea.removeFromLeft (extraSpace);
        return textArea.removeFromRight (comp.getWidth());
    }

    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int, int) {}

    void drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/, bool isMouseOver, bool isMouseDragging)
    {
        if (isMouseOver || isMouseDragging)
            g.fillAll (Colours::yellow.withAlpha (0.4f));
    }

    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&);
};


#endif
