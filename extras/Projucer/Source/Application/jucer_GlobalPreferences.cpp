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

#include "../jucer_Headers.h"
#include "jucer_GlobalPreferences.h"
#include "../Utility/jucer_FloatingToolWindow.h"
#include "../Utility/jucer_ColourPropertyComponent.h"

//==============================================================================
PathSettingsTab::PathSettingsTab (DependencyPathOS os)
{
    const int maxChars = 1024;

    StoredSettings& settings = getAppSettings();

    vst3PathComponent       = pathComponents.add (new TextPropertyComponent (settings.getGlobalPath (Ids::vst3Path, os), "VST3 SDK", maxChars, false));

   #if ! JUCE_LINUX
    rtasPathComponent       = pathComponents.add (new TextPropertyComponent (settings.getGlobalPath (Ids::rtasPath, os), "RTAS SDK", maxChars, false));
    aaxPathComponent        = pathComponents.add (new TextPropertyComponent (settings.getGlobalPath (Ids::aaxPath, os),  "AAX SDK",  maxChars, false));
   #endif

    androidSdkPathComponent = pathComponents.add (new TextPropertyComponent (settings.getGlobalPath (Ids::androidSDKPath, os), "Android SDK", maxChars, false));
    androidNdkPathComponent = pathComponents.add (new TextPropertyComponent (settings.getGlobalPath (Ids::androidNDKPath, os), "Android NDK", maxChars, false));

    for (TextPropertyComponent** component = pathComponents.begin(); component != pathComponents.end(); ++component)
    {
        addAndMakeVisible (**component);
        (*component)->addListener (this);
        textPropertyComponentChanged (*component);
    }
}

PathSettingsTab::~PathSettingsTab()
{
}

void PathSettingsTab::textPropertyComponentChanged (TextPropertyComponent* textPropertyComponent)
{
    Identifier keyName = getKeyForPropertyComponent (textPropertyComponent);

    Colour textColour = getAppSettings().isGlobalPathValid (File::getCurrentWorkingDirectory(), keyName, textPropertyComponent->getText())
                            ? Colours::black
                            : Colours::red;

    textPropertyComponent->setColour (TextPropertyComponent::textColourId, textColour);
}

Identifier PathSettingsTab::getKeyForPropertyComponent (TextPropertyComponent* component) const
{
    if (component == vst3PathComponent)       return Ids::vst3Path;
    if (component == rtasPathComponent)       return Ids::rtasPath;
    if (component == aaxPathComponent)        return Ids::aaxPath;
    if (component == androidSdkPathComponent) return Ids::androidSDKPath;
    if (component == androidNdkPathComponent) return Ids::androidNDKPath;

    // this property component does not have a key associated to it!
    jassertfalse;
    return String();
}

Component* PathSettingsTab::getContent()
{
    return this;
}

String PathSettingsTab::getName() const noexcept
{
    return "Paths";
}

void PathSettingsTab::resized()
{
    const int componentHeight = 25;

    for (TextPropertyComponent** component = pathComponents.begin(); component != pathComponents.end(); ++component)
    {
        const int elementNumber = pathComponents.indexOf (*component);
        (*component)->setBounds (0, componentHeight * elementNumber, getWidth(), componentHeight);
    }
}


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
            g.fillAll (Colours::darkgrey);

            g.setFont (14.0f);
            g.setColour (Colours::white);
            g.drawFittedText ("Scanning for fonts..", getLocalBounds(), Justification::centred, 2);

            const int size = 30;
            getLookAndFeel().drawSpinningWaitAnimation (g, Colours::white, (getWidth() - size) / 2, getHeight() / 2 - 50, size, size);
        }

        void timerCallback() override
        {
            repaint();

            if (fontsToScan.size() == 0)
            {
                getAppSettings().monospacedFontNames = fontsFound;

                if (AppearanceSettingsTab* tab = findParentComponentOfClass<AppearanceSettingsTab>())
                    tab->changeContent (new EditorPanel());
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

            const int width = font.getStringWidth ("....");

            return width == font.getStringWidth ("WWWW")
            && width == font.getStringWidth ("0000")
            && width == font.getStringWidth ("1111")
            && width == font.getStringWidth ("iiii");
        }

        StringArray fontsToScan, fontsFound;
    };

    //==============================================================================
    struct EditorPanel  : public Component,
                          private ButtonListener
    {
        EditorPanel()
           : loadButton ("Load Scheme..."),
             saveButton ("Save Scheme...")
        {
            rebuildProperties();
            addAndMakeVisible (panel);

            loadButton.setColour (TextButton::buttonColourId, Colours::lightgrey.withAlpha (0.5f));
            saveButton.setColour (TextButton::buttonColourId, Colours::lightgrey.withAlpha (0.5f));
            loadButton.setColour (TextButton::textColourOffId, Colours::white);
            saveButton.setColour (TextButton::textColourOffId, Colours::white);

            addAndMakeVisible (loadButton);
            addAndMakeVisible (saveButton);

            loadButton.addListener (this);
            saveButton.addListener (this);
        }

        void rebuildProperties()
        {
            AppearanceSettings& scheme = getAppSettings().appearance;

            Array<PropertyComponent*> props;
            Value fontValue (scheme.getCodeFontValue());
            props.add (FontNameValueSource::createProperty ("Code Editor Font", fontValue));
            props.add (FontSizeValueSource::createProperty ("Font Size", fontValue));

            const StringArray colourNames (scheme.getColourNames());

            for (int i = 0; i < colourNames.size(); ++i)
                props.add (new ColourPropertyComponent (nullptr, colourNames[i],
                                                        scheme.getColourValue (colourNames[i]),
                                                        Colours::white, false));

            panel.clear();
            panel.addProperties (props);
        }

        void resized() override
        {
            Rectangle<int> r (getLocalBounds());
            panel.setBounds (r.removeFromTop (getHeight() - 28).reduced (4, 2));
            loadButton.setBounds (r.removeFromLeft (getWidth() / 2).reduced (10, 4));
            saveButton.setBounds (r.reduced (10, 3));
        }

    private:
        PropertyPanel panel;
        TextButton loadButton, saveButton;

        void buttonClicked (Button* b) override
        {
            if (b == &loadButton)
                loadScheme();
            else
                saveScheme();
        }

        void saveScheme()
        {
            FileChooser fc ("Select a file in which to save this colour-scheme...",
                            getAppSettings().appearance.getSchemesFolder()
                            .getNonexistentChildFile ("Scheme", AppearanceSettings::getSchemeFileSuffix()),
                            AppearanceSettings::getSchemeFileWildCard());

            if (fc.browseForFileToSave (true))
            {
                File file (fc.getResult().withFileExtension (AppearanceSettings::getSchemeFileSuffix()));
                getAppSettings().appearance.writeToFile (file);
                getAppSettings().appearance.refreshPresetSchemeList();
            }
        }

        void loadScheme()
        {
            FileChooser fc ("Please select a colour-scheme file to load...",
                            getAppSettings().appearance.getSchemesFolder(),
                            AppearanceSettings::getSchemeFileWildCard());

            if (fc.browseForFileToOpen())
                if (getAppSettings().appearance.readFromFile (fc.getResult()))
                    rebuildProperties();
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
            Font font (Font::fromString (sourceValue.toString()));
            font.setTypefaceName (newValue.toString().isEmpty() ? Font::getDefaultMonospacedFontName()
                                                                : newValue.toString());
            sourceValue = font.toString();
        }

        static ChoicePropertyComponent* createProperty (const String& title, const Value& value)
        {
            StringArray fontNames = getAppSettings().monospacedFontNames;

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

void AppearanceSettings::showGlobalPreferences (ScopedPointer<Component>& ownerPointer)
{
    if (ownerPointer != nullptr)
        ownerPointer->toFront (true);
    else
        new FloatingToolWindow ("Preferences",
                                "globalPreferencesEditorPos",
                                new GlobalPreferencesComponent,
                                ownerPointer,
                                500, 500, 500, 500, 500, 500);
}

//==============================================================================
AppearanceSettingsTab::AppearanceSettingsTab()
{
    if (getAppSettings().monospacedFontNames.size() == 0)
        content = new AppearanceEditor::FontScanPanel();
    else
        content = new AppearanceEditor::EditorPanel();

    changeContent (content);
}

Component* AppearanceSettingsTab::getContent()
{
    return this;
}

void AppearanceSettingsTab::changeContent (Component* newContent)
{
    content = newContent;
    addAndMakeVisible (content);
    content->setBounds (getLocalBounds());
}

String AppearanceSettingsTab::getName() const noexcept
{
    return "Code Editor";
}

void AppearanceSettingsTab::resized()
{
    content->setBounds (getLocalBounds());
}

//==============================================================================
GlobalPreferencesComponent::GlobalPreferencesComponent()
   : TabbedComponent (TabbedButtonBar::TabsAtTop)
{
    preferenceTabs.add (new PathSettingsTab (TargetOS::getThisOS()));
    preferenceTabs.add (new AppearanceSettingsTab);

    for (GlobalPreferencesTab** tab = preferenceTabs.begin(); tab != preferenceTabs.end(); ++tab)
        addTab ((*tab)->getName(), findColour(mainBackgroundColourId, true), (*tab)->getContent(), true);
}
