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

#include "../../Utility/UI/PropertyComponents/jucer_ColorPropertyComponent.h"

//==============================================================================
class EditorColorSchemeWindowComponent    : public Component
{
public:
    EditorColorSchemeWindowComponent()
    {
        if (getAppSettings().monospacedFontNames.size() == 0)
            content.reset (new AppearanceEditor::FontScanPanel());
        else
            content.reset (new AppearanceEditor::EditorPanel());

        changeContent (content.get());
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColor (backgroundColorId));
    }

    void resized() override
    {
       content->setBounds (getLocalBounds());
    }

    void changeContent (Component* newContent)
    {
        content.reset (newContent);
        addAndMakeVisible (newContent);
        content->setBounds (getLocalBounds().reduced (10));
    }

private:
    ScopedPointer<Component> content;

    //==============================================================================
    struct AppearanceEditor
    {
        struct FontScanPanel   : public Component,
                                 private Timer
        {
            FontScanPanel()
            {
                fontsToScan = Font::findAllTypefaceNames();
                startTimer (1);
            }

            void paint (Graphics& g) override
            {
                g.fillAll (findColor (backgroundColorId));

                g.setFont (14.0f);
                g.setColor (findColor (defaultTextColorId));
                g.drawFittedText ("Scanning for fonts..", getLocalBounds(), Justification::centered, 2);

                const auto size = 30;
                getLookAndFeel().drawSpinningWaitAnimation (g, Colors::white, (getWidth() - size) / 2, getHeight() / 2 - 50, size, size);
            }

            void timerCallback() override
            {
                repaint();

                if (fontsToScan.size() == 0)
                {
                    getAppSettings().monospacedFontNames = fontsFound;

                    if (auto* owner = findParentComponentOfClass<EditorColorSchemeWindowComponent>())
                        owner->changeContent (new EditorPanel());
                }
                else
                {
                    if (isMonospacedTypeface (fontsToScan[0]))
                        fontsFound.add (fontsToScan[0]);

                    fontsToScan.remove (0);
                }
            }

            // A rather hacky trick to select only the fixed-pitch fonts..
            // This is unfortunately a bit slow, but will work on all platforms.
            static bool isMonospacedTypeface (const String& name)
            {
                const Font font (name, 20.0f, Font::plain);

                const auto width = font.getStringWidth ("....");

                return width == font.getStringWidth ("WWWW")
                && width == font.getStringWidth ("0000")
                && width == font.getStringWidth ("1111")
                && width == font.getStringWidth ("iiii");
            }

            StringArray fontsToScan, fontsFound;
        };

        //==============================================================================
        struct EditorPanel  : public Component
        {
            EditorPanel()
                : loadButton ("Load Scheme..."),
                  saveButton ("Save Scheme...")
            {
                rebuildProperties();
                addAndMakeVisible (panel);

                addAndMakeVisible (loadButton);
                addAndMakeVisible (saveButton);

                loadButton.onClick = [this] { loadScheme(); };
                saveButton.onClick = [this] { saveScheme (false); };

                lookAndFeelChanged();

                saveSchemeState();
            }

            ~EditorPanel()
            {
                if (hasSchemeBeenModifiedSinceSave())
                    saveScheme (true);
            }

            void rebuildProperties()
            {
                auto& scheme = getAppSettings().appearance;

                Array<PropertyComponent*> props;
                auto fontValue = scheme.getCodeFontValue();
                props.add (FontNameValueSource::createProperty ("Code Editor Font", fontValue));
                props.add (FontSizeValueSource::createProperty ("Font Size", fontValue));

                const auto colorNames = scheme.getColorNames();

                for (int i = 0; i < colorNames.size(); ++i)
                    props.add (new ColorPropertyComponent (nullptr, colorNames[i],
                                                            scheme.getColorValue (colorNames[i]),
                                                            Colors::white, false));

                panel.clear();
                panel.addProperties (props);
            }

            void resized() override
            {
                auto r = getLocalBounds();
                panel.setBounds (r.removeFromTop (getHeight() - 28).reduced (10, 2));
                loadButton.setBounds (r.removeFromLeft (getWidth() / 2).reduced (10, 1));
                saveButton.setBounds (r.reduced (10, 1));
            }

        private:
            PropertyPanel panel;
            TextButton loadButton, saveButton;

            Font codeFont;
            Array<var> colorValues;

            void saveScheme (bool isExit)
            {
                FileChooser fc ("Select a file in which to save this color-scheme...",
                                getAppSettings().appearance.getSchemesFolder()
                                .getNonexistentChildFile ("Scheme", AppearanceSettings::getSchemeFileSuffix()),
                                AppearanceSettings::getSchemeFileWildCard());

                if (fc.browseForFileToSave (true))
                {
                    File file (fc.getResult().withFileExtension (AppearanceSettings::getSchemeFileSuffix()));
                    getAppSettings().appearance.writeToFile (file);
                    getAppSettings().appearance.refreshPresetSchemeList();

                    saveSchemeState();
                    ProjucerApplication::getApp().selectEditorColorSchemeWithName (file.getFileNameWithoutExtension());
                }
                else if (isExit)
                {
                    restorePreviousScheme();
                }
            }

            void loadScheme()
            {
                FileChooser fc ("Please select a color-scheme file to load...",
                                getAppSettings().appearance.getSchemesFolder(),
                                AppearanceSettings::getSchemeFileWildCard());

                if (fc.browseForFileToOpen())
                {
                    if (getAppSettings().appearance.readFromFile (fc.getResult()))
                    {
                        rebuildProperties();
                        saveSchemeState();
                    }
                }
            }

            void lookAndFeelChanged() override
            {
                loadButton.setColor (TextButton::buttonColorId,
                                      findColor (secondaryButtonBackgroundColorId));
            }

            void saveSchemeState()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colorNames = appearance.getColorNames();

                codeFont = appearance.getCodeFont();

                colorValues.clear();
                for (int i = 0; i < colorNames.size(); ++i)
                    colorValues.add (appearance.getColorValue (colorNames[i]).getValue());
            }

            bool hasSchemeBeenModifiedSinceSave()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colorNames = appearance.getColorNames();

                if (codeFont != appearance.getCodeFont())
                    return true;

                for (int i = 0; i < colorNames.size(); ++i)
                    if (colorValues[i] != appearance.getColorValue (colorNames[i]).getValue())
                        return true;

                return false;
            }

            void restorePreviousScheme()
            {
                auto& appearance = getAppSettings().appearance;
                const auto colorNames = appearance.getColorNames();

                appearance.getCodeFontValue().setValue (codeFont.toString());

                for (int i = 0; i < colorNames.size(); ++i)
                    appearance.getColorValue (colorNames[i]).setValue (colorValues[i]);
            }


            JUCE_DECLARE_NON_COPYABLE (EditorPanel)
        };

        //==============================================================================
        struct FontNameValueSource   : public ValueSourceFilter
        {
            FontNameValueSource (const Value& source)  : ValueSourceFilter (source) {}

            var getValue() const override
            {
                return Font::fromString (sourceValue.toString()).getTypefaceName();
            }

            void setValue (const var& newValue) override
            {
                auto font = Font::fromString (sourceValue.toString());
                font.setTypefaceName (newValue.toString().isEmpty() ? Font::getDefaultMonospacedFontName()
                                      : newValue.toString());
                sourceValue = font.toString();
            }

            static ChoicePropertyComponent* createProperty (const String& title, const Value& value)
            {
                auto fontNames = getAppSettings().monospacedFontNames;

                Array<var> values;
                values.add (Font::getDefaultMonospacedFontName());
                values.add (var());

                for (int i = 0; i < fontNames.size(); ++i)
                    values.add (fontNames[i]);

                StringArray names;
                names.add ("<Default Monospaced>");
                names.add (String());
                names.addArray (getAppSettings().monospacedFontNames);

                return new ChoicePropertyComponent (Value (new FontNameValueSource (value)),
                                                    title, names, values);
            }
        };

        //==============================================================================
        struct FontSizeValueSource   : public ValueSourceFilter
        {
            FontSizeValueSource (const Value& source)  : ValueSourceFilter (source) {}

            var getValue() const override
            {
                return Font::fromString (sourceValue.toString()).getHeight();
            }

            void setValue (const var& newValue) override
            {
                sourceValue = Font::fromString (sourceValue.toString()).withHeight (newValue).toString();
            }

            static PropertyComponent* createProperty (const String& title, const Value& value)
            {
                return new SliderPropertyComponent (Value (new FontSizeValueSource (value)),
                                                    title, 5.0, 40.0, 0.1, 0.5);
            }
        };
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditorColorSchemeWindowComponent)
};
