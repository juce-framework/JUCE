/*
  ==============================================================================

    jucer_GlobalPreferences.cpp
    Created: 22 Jul 2015 11:05:35am
    Author:  Timur Doumler

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_GlobalPreferences.h"




//==============================================================================
class AppearanceSettingsTab  : public GlobalPreferencesTab,
                               public Component
{
public:
    AppearanceSettingsTab();

    Component* getContent() override;
    void changeContent (Component* newContent);
    String getName() const noexcept override;

    ScopedPointer<Component> content;
};

//==============================================================================
namespace PathSettingsHelpers
{
    bool checkSdkPathContainsFile (const String& path, const String& fileToCheckFor)
    {
        return File::getCurrentWorkingDirectory().getChildFile( path + "/" + fileToCheckFor).existsAsFile();
    }
}

PathSettingsTab::PathSettingsTab (DependencyPathOS os)
{
    const int maxChars = 1024;

    vst2PathComponent       = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::vst2KeyName, os), "VST SDK",  maxChars, false));
    vst3PathComponent       = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::vst3KeyName, os), "VST3 SDK", maxChars, false));

   #if ! JUCE_LINUX
    rtasPathComponent       = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::rtasKeyName, os), "RTAS SDK", maxChars, false));
    aaxPathComponent        = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::aaxKeyName, os),  "AAX SDK",  maxChars, false));
   #endif

    androidSdkPathComponent = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::androidSdkKeyName, os), "Android SDK", maxChars, false));
    androidNdkPathComponent = pathComponents.add (new TextPropertyComponent (getPathByKey (DependencyPath::androidNdkKeyName, os), "Android NDK", maxChars, false));

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
    String keyName = getKeyForPropertyComponent (textPropertyComponent);

    Colour textColour = checkPathByKey (keyName, textPropertyComponent->getText()) ? Colours::black : Colours::red;
    textPropertyComponent->setColour (TextPropertyComponent::textColourId, textColour);
}

String PathSettingsTab::getKeyForPropertyComponent (TextPropertyComponent* component) const
{
    if (component == vst2PathComponent)       return DependencyPath::vst2KeyName;
    if (component == vst3PathComponent)       return DependencyPath::vst3KeyName;
    if (component == rtasPathComponent)       return DependencyPath::rtasKeyName;
    if (component == aaxPathComponent)        return DependencyPath::aaxKeyName;
    if (component == androidSdkPathComponent) return DependencyPath::androidSdkKeyName;
    if (component == androidNdkPathComponent) return DependencyPath::androidNdkKeyName;

    // this property component does not have a key associated to it!
    jassertfalse;
    return String::empty;
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
Value& PathSettingsTab::getPathByKey (const String& key, DependencyPathOS os)
{
    getAppSettings().pathValues[key].referTo (getAppSettings().projectDefaults.getPropertyAsValue (key, nullptr));
    Value& value = getAppSettings().pathValues[key];

    if (value.toString().isEmpty())
        value = getFallbackPathByKey (key, os);

    return value;
}

//==============================================================================
String PathSettingsTab::getFallbackPathByKey (const String& key, DependencyPathOS os)
{
    if (key == DependencyPath::vst2KeyName || key == DependencyPath::vst3KeyName)
        return os == DependencyPath::windows ? "c:\\SDKs\\VST3 SDK"
                                             : "~/SDKs/VST3 SDK";

    if (key == DependencyPath::rtasKeyName)
    {
        if (os == DependencyPath::windows)   return "c:\\SDKs\\PT_80_SDK";
        if (os == DependencyPath::osx)       return "~/SDKs/PT_80_SDK";

        // no RTAS on this OS!
        jassertfalse;
        return String();
    }

    if (key == DependencyPath::aaxKeyName)
    {
        if (os == DependencyPath::windows)   return "c:\\SDKs\\AAX";
        if (os == DependencyPath::osx)       return "~/SDKs/AAX" ;

        // no RTAS on this OS!
        jassertfalse;
        return String();
    }

    if (key == DependencyPath::androidSdkKeyName)
        return os == DependencyPath::windows ? "c:\\SDKs\\android-sdk"
                                             : "~/Library/Android/sdk";

    if (key == DependencyPath::androidNdkKeyName)
        return os == DependencyPath::windows ? "c:\\SDKs\\android-ndk"
                                             : "~/Library/Android/ndk";

    // didn't recognise the key provided!
    jassertfalse;
    return String();
}

//==============================================================================
bool PathSettingsTab::checkPathByKey (const String& key, const String& path)
{
    String fileToCheckFor;

    if (key == DependencyPath::vst2KeyName)
    {
        fileToCheckFor = "public.sdk/source/vst2.x/audioeffectx.h";
    }
    else if (key == DependencyPath::vst3KeyName)
    {
        fileToCheckFor = "base/source/baseiids.cpp";
    }
    else if (key == DependencyPath::rtasKeyName)
    {
        fileToCheckFor = "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses/CEffectProcessMIDI.cpp";
    }
    else if (key == DependencyPath::aaxKeyName)
    {
        fileToCheckFor = "Interfaces/AAX_Exports.cpp";
    }
    else if (key == DependencyPath::androidSdkKeyName)
    {
       #if JUCE_WINDOWS
        fileToCheckFor = "platform-tools/adb.exe";
       #else
        fileToCheckFor = "platform-tools/adb";
       #endif
    }
    else if (key == DependencyPath::androidNdkKeyName)
    {
       #if JUCE_WINDOWS
        fileToCheckFor = "ndk-depends.exe";
       #else
        fileToCheckFor = "ndk-depends";
       #endif
    }
    else
    {
        // didn't recognise the key provided!
        jassertfalse;
        return false;
    }

    return PathSettingsHelpers::checkSdkPathContainsFile (path, fileToCheckFor);
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
            names.add (String::empty);
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
    {
        ownerPointer->toFront (true);
    }
    else
    {
        new FloatingToolWindow ("Global Preferences",
                                "globalPreferencesEditorPos",
                                new GlobalPreferencesComponent,
                                ownerPointer,
                                500, 500, 500, 500, 500, 500);
    }
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
    addAndMakeVisible(content);
    content->setBoundsInset(BorderSize<int>());
}

String AppearanceSettingsTab::getName() const noexcept
{
    return "Code Editor";
}

//==============================================================================
GlobalPreferencesComponent::GlobalPreferencesComponent()
   : TabbedComponent (TabbedButtonBar::TabsAtTop)
{
    preferenceTabs.add (new PathSettingsTab (DependencyPath::getThisOS()));
    preferenceTabs.add (new AppearanceSettingsTab);

    for (GlobalPreferencesTab** tab = preferenceTabs.begin(); tab != preferenceTabs.end(); ++tab)
        addTab ((*tab)->getName(), findColour(mainBackgroundColourId, true), (*tab)->getContent(), true);
}
