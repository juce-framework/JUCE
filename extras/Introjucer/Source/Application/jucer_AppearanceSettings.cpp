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

#include "jucer_Application.h"
#include "jucer_AppearanceSettings.h"
#include "jucer_GlobalPreferences.h"

namespace AppearanceColours
{
    struct ColourInfo
    {
        const char* name;
        int colourID;
        bool mustBeOpaque;
        bool applyToEditorOnly;
    };

    static const ColourInfo colours[] =
    {
        { "Main Window Bkgd",   mainBackgroundColourId, true, false },
        { "Treeview Highlight", TreeView::selectedItemBackgroundColourId, false, false },

        { "Code Background",    CodeEditorComponent::backgroundColourId, true, false },
        { "Line Number Bkgd",   CodeEditorComponent::lineNumberBackgroundId, false, false },
        { "Line Numbers",       CodeEditorComponent::lineNumberTextId, false, false },
        { "Plain Text",         CodeEditorComponent::defaultTextColourId, false, false },
        { "Selected Text Bkgd", CodeEditorComponent::highlightColourId, false, false },
        { "Caret",              CaretComponent::caretColourId, false, true }
    };

    enum
    {
        numColours = sizeof (AppearanceColours::colours) / sizeof (AppearanceColours::colours[0])
    };
}

//==============================================================================
AppearanceSettings::AppearanceSettings (bool updateAppWhenChanged)
    : settings ("COLOUR_SCHEME")
{
    if (! IntrojucerApp::getApp().isRunningCommandLine)
    {
        IntrojucerLookAndFeel lf;

        for (int i = 0; i < AppearanceColours::numColours; ++i)
            getColourValue (AppearanceColours::colours[i].name) = lf.findColour (AppearanceColours::colours[i].colourID).toString();

        CodeDocument doc;
        CPlusPlusCodeTokeniser tokeniser;
        CodeEditorComponent editor (doc, &tokeniser);

        const CodeEditorComponent::ColourScheme cs (editor.getColourScheme());

        for (int i = cs.types.size(); --i >= 0;)
        {
            CodeEditorComponent::ColourScheme::TokenType& t = cs.types.getReference(i);
            getColourValue (t.name) = t.colour.toString();
        }

        getCodeFontValue() = getDefaultCodeFont().toString();

        if (updateAppWhenChanged)
            settings.addListener (this);
    }
}

File AppearanceSettings::getSchemesFolder()
{
    File f (getGlobalProperties().getFile().getSiblingFile ("Schemes"));
    f.createDirectory();
    return f;
}

void AppearanceSettings::writeDefaultSchemeFile (const String& xmlString, const String& name)
{
    const File file (getSchemesFolder().getChildFile (name).withFileExtension (getSchemeFileSuffix()));

    AppearanceSettings settings (false);

    ScopedPointer<XmlElement> xml (XmlDocument::parse (xmlString));
    if (xml != nullptr)
        settings.readFromXML (*xml);

    settings.writeToFile (file);
}

void AppearanceSettings::refreshPresetSchemeList()
{
    writeDefaultSchemeFile (BinaryData::colourscheme_dark_xml,  "Default (Dark)");
    writeDefaultSchemeFile (BinaryData::colourscheme_light_xml, "Default (Light)");

    Array<File> newSchemes;
    getSchemesFolder().findChildFiles (newSchemes, File::findFiles, false, String ("*") + getSchemeFileSuffix());

    if (newSchemes != presetSchemeFiles)
    {
        presetSchemeFiles.swapWith (newSchemes);
        IntrojucerApp::getCommandManager().commandStatusChanged();
    }
}

StringArray AppearanceSettings::getPresetSchemes()
{
    StringArray s;
    for (int i = 0; i < presetSchemeFiles.size(); ++i)
        s.add (presetSchemeFiles.getReference(i).getFileNameWithoutExtension());

    return s;
}

void AppearanceSettings::selectPresetScheme (int index)
{
    readFromFile (presetSchemeFiles [index]);
}

bool AppearanceSettings::readFromXML (const XmlElement& xml)
{
    if (xml.hasTagName (settings.getType().toString()))
    {
        const ValueTree newSettings (ValueTree::fromXml (xml));

        // we'll manually copy across the new properties to the existing tree so that
        // any open editors will be kept up to date..
        settings.copyPropertiesFrom (newSettings, nullptr);

        for (int i = settings.getNumChildren(); --i >= 0;)
        {
            ValueTree c (settings.getChild (i));

            const ValueTree newValue (newSettings.getChildWithProperty (Ids::name, c.getProperty (Ids::name)));

            if (newValue.isValid())
                c.copyPropertiesFrom (newValue, nullptr);
        }

        return true;
    }

    return false;
}

bool AppearanceSettings::readFromFile (const File& file)
{
    const ScopedPointer<XmlElement> xml (XmlDocument::parse (file));
    return xml != nullptr && readFromXML (*xml);
}

bool AppearanceSettings::writeToFile (const File& file) const
{
    const ScopedPointer<XmlElement> xml (settings.createXml());
    return xml != nullptr && xml->writeToFile (file, String::empty);
}

Font AppearanceSettings::getDefaultCodeFont()
{
    return Font (Font::getDefaultMonospacedFontName(), Font::getDefaultStyle(), 13.0f);
}

StringArray AppearanceSettings::getColourNames() const
{
    StringArray s;

    for (int i = 0; i < settings.getNumChildren(); ++i)
    {
        const ValueTree c (settings.getChild(i));

        if (c.hasType ("COLOUR"))
            s.add (c [Ids::name]);
    }

    return s;
}

void AppearanceSettings::updateColourScheme()
{
    applyToLookAndFeel (LookAndFeel::getDefaultLookAndFeel());
    IntrojucerApp::getApp().mainWindowList.sendLookAndFeelChange();
}

void AppearanceSettings::applyToLookAndFeel (LookAndFeel& lf) const
{
    for (int i = 0; i < AppearanceColours::numColours; ++i)
    {
        Colour col;
        if (getColour (AppearanceColours::colours[i].name, col))
        {
            if (AppearanceColours::colours[i].mustBeOpaque)
                col = Colours::white.overlaidWith (col);

            if (! AppearanceColours::colours[i].applyToEditorOnly)
                lf.setColour (AppearanceColours::colours[i].colourID, col);
        }
    }

    lf.setColour (ScrollBar::thumbColourId, lf.findColour (mainBackgroundColourId).contrasting().withAlpha (0.13f));
}

void AppearanceSettings::applyToCodeEditor (CodeEditorComponent& editor) const
{
    CodeEditorComponent::ColourScheme cs (editor.getColourScheme());

    for (int i = cs.types.size(); --i >= 0;)
    {
        CodeEditorComponent::ColourScheme::TokenType& t = cs.types.getReference(i);
        getColour (t.name, t.colour);
    }

    editor.setColourScheme (cs);
    editor.setFont (getCodeFont());

    for (int i = 0; i < AppearanceColours::numColours; ++i)
    {
        if (AppearanceColours::colours[i].applyToEditorOnly)
        {
            Colour col;
            if (getColour (AppearanceColours::colours[i].name, col))
                editor.setColour (AppearanceColours::colours[i].colourID, col);
        }
    }

    editor.setColour (ScrollBar::thumbColourId, editor.findColour (CodeEditorComponent::backgroundColourId)
                                                      .contrasting()
                                                      .withAlpha (0.13f));
}

Font AppearanceSettings::getCodeFont() const
{
    const String fontString (settings [Ids::font].toString());

    if (fontString.isEmpty())
        return getDefaultCodeFont();

    return Font::fromString (fontString);
}

Value AppearanceSettings::getCodeFontValue()
{
    return settings.getPropertyAsValue (Ids::font, nullptr);
}

Value AppearanceSettings::getColourValue (const String& colourName)
{
    ValueTree c (settings.getChildWithProperty (Ids::name, colourName));

    if (! c.isValid())
    {
        c = ValueTree ("COLOUR");
        c.setProperty (Ids::name, colourName, nullptr);
        settings.addChild (c, -1, nullptr);
    }

    return c.getPropertyAsValue (Ids::colour, nullptr);
}

bool AppearanceSettings::getColour (const String& name, Colour& result) const
{
    const ValueTree colour (settings.getChildWithProperty (Ids::name, name));

    if (colour.isValid())
    {
        result = Colour::fromString (colour [Ids::colour].toString());
        return true;
    }

    return false;
}

//==============================================================================
IntrojucerLookAndFeel::IntrojucerLookAndFeel()
{
    setColour (mainBackgroundColourId, Colour::greyLevel (0.8f));
}

int IntrojucerLookAndFeel::getTabButtonBestWidth (TabBarButton&, int)   { return 120; }

Colour IntrojucerLookAndFeel::getTabBackgroundColour (TabBarButton& button)
{
    const Colour bkg (button.findColour (mainBackgroundColourId).contrasting (0.15f));

    if (button.isFrontTab())
        return bkg.overlaidWith (Colours::yellow.withAlpha (0.5f));

    return bkg;
}

void IntrojucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());

    const Colour bkg (getTabBackgroundColour (button));

    g.setGradientFill (ColourGradient (bkg.brighter (0.1f), 0, (float) activeArea.getY(),
                                       bkg.darker (0.1f), 0, (float) activeArea.getBottom(), false));
    g.fillRect (activeArea);

    g.setColour (button.findColour (mainBackgroundColourId).darker (0.3f));
    g.drawRect (activeArea);

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    const Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    TextLayout textLayout;
    LookAndFeel_V3::createTabTextLayout (button, (float) activeArea.getWidth(), (float) activeArea.getHeight(), col, textLayout);

    textLayout.draw (g, button.getTextArea().toFloat());
}

void IntrojucerLookAndFeel::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                                       bool isMouseOver, bool /*isMouseDown*/,
                                                       ConcertinaPanel&, Component& panel)
{
    const Colour bkg (Colours::grey);

    g.setGradientFill (ColourGradient (Colour::greyLevel (isMouseOver ? 0.6f : 0.5f), 0, (float) area.getY(),
                                       Colour::greyLevel (0.4f), 0, (float) area.getBottom(), false));
    g.fillAll();

    g.setColour (bkg.contrasting().withAlpha (0.1f));
    g.fillRect (area.withHeight (1));
    g.fillRect (area.withTop (area.getBottom() - 1));

    g.setColour (bkg.contrasting());
    g.setFont (Font (area.getHeight() * 0.6f).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

static Range<float> getBrightnessRange (const Image& im)
{
    float minB = 1.0f, maxB = 0;
    const int w = im.getWidth();
    const int h = im.getHeight();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const float b = im.getPixelAt (x, y).getBrightness();
            minB = jmin (minB, b);
            maxB = jmax (maxB, b);
        }
    }

    return Range<float> (minB, maxB);
}

void IntrojucerLookAndFeel::fillWithBackgroundTexture (Graphics& g)
{
    const Colour bkg (findColour (mainBackgroundColourId));

    if (backgroundTextureBaseColour != bkg)
    {
        backgroundTextureBaseColour = bkg;

        const Image original (ImageCache::getFromMemory (BinaryData::background_tile_png,
                                                         BinaryData::background_tile_pngSize));
        const int w = original.getWidth();
        const int h = original.getHeight();

        backgroundTexture = Image (Image::RGB, w, h, false);

        const Range<float> brightnessRange (getBrightnessRange (original));
        const float brightnessOffset = (brightnessRange.getStart() + brightnessRange.getEnd()) / 2.0f;
        const float brightnessScale = 0.025f / brightnessRange.getLength();
        const float bkgB = bkg.getBrightness();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const float b = (original.getPixelAt (x, y).getBrightness() - brightnessOffset) * brightnessScale;
                backgroundTexture.setPixelAt (x, y, bkg.withBrightness (jlimit (0.0f, 1.0f, bkgB + b)));
            }
        }
    }

    g.setTiledImageFill (backgroundTexture, 0, 0, 1.0f);
    g.fillAll();
}

void IntrojucerLookAndFeel::fillWithBackgroundTexture (Component& c, Graphics& g)
{
    dynamic_cast<IntrojucerLookAndFeel&> (c.getLookAndFeel()).fillWithBackgroundTexture (g);
}
