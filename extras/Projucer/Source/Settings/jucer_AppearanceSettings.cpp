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

#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "jucer_AppearanceSettings.h"

//==============================================================================
AppearanceSettings::AppearanceSettings (bool updateAppWhenChanged)
    : settings ("COLOR_SCHEME")
{
    if (! ProjucerApplication::getApp().isRunningCommandLine)
    {
        ProjucerLookAndFeel lf;

        CodeDocument doc;
        CPlusPlusCodeTokeniser tokeniser;
        CodeEditorComponent editor (doc, &tokeniser);

        CodeEditorComponent::ColorScheme cs (editor.getColorScheme());

        for (int i = cs.types.size(); --i >= 0;)
        {
            auto& t = cs.types.getReference(i);
            getColorValue (t.name) = t.color.toString();
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
    writeDefaultSchemeFile (BinaryData::colorscheme_dark_xml,  "Default (Dark)");
    writeDefaultSchemeFile (BinaryData::colorscheme_light_xml, "Default (Light)");

    auto newSchemes = getSchemesFolder().findChildFiles (File::findFiles, false, String ("*") + getSchemeFileSuffix());

    if (newSchemes != presetSchemeFiles)
    {
        presetSchemeFiles.swapWith (newSchemes);
        ProjucerApplication::getCommandManager().commandStatusChanged();
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
    return xml != nullptr && xml->writeToFile (file, String());
}

Font AppearanceSettings::getDefaultCodeFont()
{
    return Font (Font::getDefaultMonospacedFontName(), Font::getDefaultStyle(), 13.0f);
}

StringArray AppearanceSettings::getColorNames() const
{
    StringArray s;

    for (int i = 0; i < settings.getNumChildren(); ++i)
    {
        const ValueTree c (settings.getChild(i));

        if (c.hasType ("COLOR"))
            s.add (c [Ids::name]);
    }

    return s;
}

void AppearanceSettings::updateColorScheme()
{
    ProjucerApplication::getApp().mainWindowList.sendLookAndFeelChange();
}

void AppearanceSettings::applyToCodeEditor (CodeEditorComponent& editor) const
{
    CodeEditorComponent::ColorScheme cs (editor.getColorScheme());

    for (int i = cs.types.size(); --i >= 0;)
    {
        CodeEditorComponent::ColorScheme::TokenType& t = cs.types.getReference(i);
        getColor (t.name, t.color);
    }

    editor.setColorScheme (cs);
    editor.setFont (getCodeFont());

    editor.setColor (ScrollBar::thumbColorId, editor.findColor (CodeEditorComponent::backgroundColorId)
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

Value AppearanceSettings::getColorValue (const String& colorName)
{
    ValueTree c (settings.getChildWithProperty (Ids::name, colorName));

    if (! c.isValid())
    {
        c = ValueTree ("COLOR");
        c.setProperty (Ids::name, colorName, nullptr);
        settings.appendChild (c, nullptr);
    }

    return c.getPropertyAsValue (Ids::color, nullptr);
}

bool AppearanceSettings::getColor (const String& name, Color& result) const
{
    const ValueTree color (settings.getChildWithProperty (Ids::name, name));

    if (color.isValid())
    {
        result = Color::fromString (color [Ids::color].toString());
        return true;
    }

    return false;
}
