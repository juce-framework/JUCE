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

#include "jucer_Application.h"
#include "jucer_AppearanceSettings.h"

namespace AppearanceColours
{
    struct ColourInfo
    {
        const char* name;
        uint32 colourID;
        bool mustBeOpaque;
    };

    static const ColourInfo colours[] =
    {
        { "Main Window Bkgd",   mainBackgroundColourId, true },
        { "Treeview Highlight", treeviewHighlightColourId, false },

        { "Code Background",    CodeEditorComponent::backgroundColourId, true },
        { "Line Number Bkgd",   CodeEditorComponent::lineNumberBackgroundId, false },
        { "Line Numbers",       CodeEditorComponent::lineNumberTextId, false },
        { "Plain Text",         CodeEditorComponent::defaultTextColourId, false },
        { "Selected Text Bkgd", CodeEditorComponent::highlightColourId, false },
        { "Caret",              CaretComponent::caretColourId, false }
    };
}

//==============================================================================
AppearanceSettings::AppearanceSettings()
    : settings ("COLOUR_SCHEME")
{
    IntrojucerLookAndFeel lf;

    for (int i = 0; i < sizeof (AppearanceColours::colours) / sizeof (AppearanceColours::colours[0]); ++i)
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

    Font f (editor.getFont());
    f.setTypefaceName (f.getTypeface()->getName());
    getCodeFontValue() = f.toString();

    settings.addListener (this);
}

File AppearanceSettings::getSchemesFolder()
{
    File f (getAppProperties().getFile().getSiblingFile ("Colour Schemes"));
    f.createDirectory();
    return f;
}

void AppearanceSettings::refreshPresetSchemeList()
{
    const File defaultSchemeFile (getSchemesFolder().getChildFile ("Default").withFileExtension (getSchemeFileSuffix()));

    if (! defaultSchemeFile.exists())
        AppearanceSettings().writeToFile (defaultSchemeFile);

    Array<File> newSchemes;
    getSchemesFolder().findChildFiles (newSchemes, File::findFiles, false, String ("*") + getSchemeFileSuffix());

    if (newSchemes != presetSchemeFiles)
    {
        presetSchemeFiles.swapWithArray (newSchemes);
        commandManager->commandStatusChanged();
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
    JucerApplication::getApp().mainWindowList.sendLookAndFeelChange();
}

void AppearanceSettings::applyToLookAndFeel (LookAndFeel& lf) const
{
    for (int i = 0; i < sizeof (AppearanceColours::colours) / sizeof (AppearanceColours::colours[0]); ++i)
    {
        Colour col;
        if (getColour (AppearanceColours::colours[i].name, col))
        {
            if (AppearanceColours::colours[i].mustBeOpaque)
                col = Colours::white.overlaidWith (col);

            lf.setColour (AppearanceColours::colours[i].colourID, col);
        }
    }

    lf.setColour (ScrollBar::thumbColourId,
                  getScrollbarColourForBackground (lf.findColour (mainBackgroundColourId)));
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

    editor.setColour (ScrollBar::thumbColourId,
                      getScrollbarColourForBackground (editor.findColour (CodeEditorComponent::backgroundColourId)));
}

Font AppearanceSettings::getCodeFont() const
{
    const String fontString (settings [Ids::font].toString());

    if (fontString.isEmpty())
    {
       #if JUCE_MAC
        Font font (13.0f);
        font.setTypefaceName ("Menlo");
       #else
        Font font (10.0f);
        font.setTypefaceName (Font::getDefaultMonospacedFontName());
       #endif

        return font;
    }

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

Colour AppearanceSettings::getScrollbarColourForBackground (const Colour& background)
{
    return background.contrasting().withAlpha (0.13f);
}

//==============================================================================
struct AppearanceEditor
{
    class Window   : public DialogWindow
    {
    public:
        Window()   : DialogWindow ("Appearance Settings", Colours::black, true, true)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new EditorPanel(), false);
            setResizable (true, true);

            const int width = 350;
            setResizeLimits (width, 200, width, 1000);

            String windowState (getAppProperties().getValue (getWindowPosName()));

            if (windowState.isNotEmpty())
                restoreWindowStateFromString (windowState);
            else
                centreAroundComponent (Component::getCurrentlyFocusedComponent(), width, 500);

            setVisible (true);
        }

        ~Window()
        {
            getAppProperties().setValue (getWindowPosName(), getWindowStateAsString());
        }

        void closeButtonPressed()
        {
            JucerApplication::getApp().appearanceEditorWindow = nullptr;
        }

    private:
        static const char* getWindowPosName()   { return "colourSchemeEditorPos"; }

        JUCE_DECLARE_NON_COPYABLE (Window);
    };

    //==============================================================================
    class EditorPanel  : public Component,
                         private Button::Listener
    {
    public:
        EditorPanel()
            : loadButton ("Load Scheme..."),
              saveButton ("Save Scheme...")
        {
            setOpaque (true);

            rebuildProperties();
            addAndMakeVisible (&panel);

            loadButton.setColour (TextButton::buttonColourId, Colours::grey);
            saveButton.setColour (TextButton::buttonColourId, Colours::grey);

            addAndMakeVisible (&loadButton);
            addAndMakeVisible (&saveButton);

            loadButton.addListener (this);
            saveButton.addListener (this);
        }

        void rebuildProperties()
        {
            AppearanceSettings& scheme = getAppSettings().appearance;

            Array <PropertyComponent*> props;
            Value fontValue (scheme.getCodeFontValue());
            props.add (FontNameValueSource::createProperty  ("Code Editor Font", fontValue));
            props.add (FontSizeValueSource::createProperty  ("Font Size", fontValue));

            const StringArray colourNames (scheme.getColourNames());

            for (int i = 0; i < colourNames.size(); ++i)
                props.add (new ColourPropertyComponent (nullptr, colourNames[i],
                                                        scheme.getColourValue (colourNames[i]),
                                                        Colours::white, false));

            panel.clear();
            panel.addProperties (props);
        }

        void paint (Graphics& g)
        {
            g.fillAll (Colours::black);
        }

        void resized()
        {
            Rectangle<int> r (getLocalBounds());
            panel.setBounds (r.removeFromTop (getHeight() - 26).reduced (4, 3));
            loadButton.setBounds (r.removeFromLeft (getWidth() / 2).reduced (10, 3));
            saveButton.setBounds (r.reduced (10, 3));
        }

    private:
        PropertyPanel panel;
        TextButton loadButton, saveButton;

        void buttonClicked (Button* b)
        {
            if (b == &loadButton)
                loadScheme();
            else
                saveScheme();
        }

        void saveScheme()
        {
            FileChooser fc ("Select a file in which to save this colour-scheme...",
                            getAppSettings().appearance.getSchemesFolder().getNonexistentChildFile ("Scheme", ".editorscheme"),
                            "*.editorscheme");

            if (fc.browseForFileToSave (true))
            {
                File file (fc.getResult().withFileExtension (".editorscheme"));
                getAppSettings().appearance.writeToFile (file);
                getAppSettings().appearance.refreshPresetSchemeList();
            }
        }

        void loadScheme()
        {
            FileChooser fc ("Please select a colour-scheme file to load...",
                            getAppSettings().appearance.getSchemesFolder(),
                            "*.editorscheme");

            if (fc.browseForFileToOpen())
            {
                if (getAppSettings().appearance.readFromFile (fc.getResult()))
                    rebuildProperties();
            }
        }

        JUCE_DECLARE_NON_COPYABLE (EditorPanel);
    };

    //==============================================================================
    class FontNameValueSource   : public ValueSourceFilter
    {
    public:
        FontNameValueSource (const Value& source)  : ValueSourceFilter (source) {}

        var getValue() const
        {
            return Font::fromString (sourceValue.toString()).getTypefaceName();
        }

        void setValue (const var& newValue)
        {
            Font font (Font::fromString (sourceValue.toString()));
            font.setTypefaceName (newValue.toString());
            sourceValue = font.toString();
        }

        static ChoicePropertyComponent* createProperty (const String& title, const Value& value)
        {
            const StringArray& fontNames = getAppSettings().getFontNames();

            Array<var> values;
            for (int i = 0; i < fontNames.size(); ++i)
                values.add (fontNames[i]);

            return new ChoicePropertyComponent (Value (new FontNameValueSource (value)),
                                                title, fontNames, values);
        }
    };

    //==============================================================================
    class FontSizeValueSource   : public ValueSourceFilter
    {
    public:
        FontSizeValueSource (const Value& source)  : ValueSourceFilter (source) {}

        var getValue() const
        {
            return Font::fromString (sourceValue.toString()).getHeight();
        }

        void setValue (const var& newValue)
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

Component* AppearanceSettings::createEditorWindow()
{
    return new AppearanceEditor::Window();
}

//==============================================================================
IntrojucerLookAndFeel::IntrojucerLookAndFeel()
{
    setColour (mainBackgroundColourId, Colour::greyLevel (0.8f));
    setColour (treeviewHighlightColourId, Colour (0x401111ee));
}

Rectangle<int> IntrojucerLookAndFeel::getPropertyComponentContentPosition (PropertyComponent& component)
{
    if (component.findParentComponentOfClass<AppearanceEditor::EditorPanel>() != nullptr)
        return component.getLocalBounds().reduced (1, 1).removeFromRight (component.getWidth() / 2);

    return LookAndFeel::getPropertyComponentContentPosition (component);
}

int IntrojucerLookAndFeel::getTabButtonOverlap (int tabDepth)                          { return -1; }
int IntrojucerLookAndFeel::getTabButtonSpaceAroundImage()                              { return 1; }
int IntrojucerLookAndFeel::getTabButtonBestWidth (TabBarButton& button, int tabDepth)  { return 120; }

void IntrojucerLookAndFeel::createTabTextLayout (const TabBarButton& button, const Rectangle<int>& textArea, GlyphArrangement& textLayout)
{
    Font font (textArea.getHeight() * 0.5f);
    font.setUnderline (button.hasKeyboardFocus (false));

    textLayout.addFittedText (font, button.getButtonText().trim(),
                              (float) textArea.getX(), (float) textArea.getY(), (float) textArea.getWidth(), (float) textArea.getHeight(),
                              Justification::centred, 1);
}

Colour IntrojucerLookAndFeel::getTabBackgroundColour (TabBarButton& button)
{
    Colour normalBkg (button.getTabBackgroundColour());
    Colour bkg (normalBkg.contrasting (0.15f));
    if (button.isFrontTab())
        bkg = bkg.overlaidWith (Colours::yellow.withAlpha (0.5f));

    return bkg;
}

void IntrojucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
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

Rectangle<int> IntrojucerLookAndFeel::getTabButtonExtraComponentBounds (const TabBarButton& button, Rectangle<int>& textArea, Component& comp)
{
    GlyphArrangement textLayout;
    createTabTextLayout (button, textArea, textLayout);
    const int textWidth = (int) textLayout.getBoundingBox (0, -1, false).getWidth();
    const int extraSpace = jmax (0, textArea.getWidth() - (textWidth + comp.getWidth())) / 2;

    textArea.removeFromRight (extraSpace);
    textArea.removeFromLeft (extraSpace);
    return textArea.removeFromRight (comp.getWidth());
}

void IntrojucerLookAndFeel::drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/, bool isMouseOver, bool isMouseDragging)
{
    if (isMouseOver || isMouseDragging)
        g.fillAll (Colours::yellow.withAlpha (0.4f));
}

void IntrojucerLookAndFeel::drawScrollbar (Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height,
                                           bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                           bool isMouseOver, bool isMouseDown)
{
    Path thumbPath;

    if (thumbSize > 0)
    {
        const float thumbIndent = jmin (width, height) * 0.25f;
        const float thumbIndentx2 = thumbIndent * 2.0f;

        if (isScrollbarVertical)
            thumbPath.addRoundedRectangle (x + thumbIndent, thumbStartPosition + thumbIndent,
                                           width - thumbIndentx2, thumbSize - thumbIndentx2, (width - thumbIndentx2) * 0.5f);
        else
            thumbPath.addRoundedRectangle (thumbStartPosition + thumbIndent, y + thumbIndent,
                                           thumbSize - thumbIndentx2, height - thumbIndentx2, (height - thumbIndentx2) * 0.5f);
    }

    Colour thumbCol (scrollbar.findColour (ScrollBar::thumbColourId, true));

    if (isMouseOver || isMouseDown)
        thumbCol = thumbCol.withMultipliedAlpha (2.0f);

    g.setColour (thumbCol);
    g.fillPath (thumbPath);

    g.setColour (thumbCol.contrasting ((isMouseOver  || isMouseDown) ? 0.2f : 0.1f));
    g.strokePath (thumbPath, PathStrokeType (1.0f));
}
