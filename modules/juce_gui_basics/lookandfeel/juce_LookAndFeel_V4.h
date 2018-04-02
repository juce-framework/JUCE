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

namespace juce
{

//==============================================================================
/**
    The latest JUCE look-and-feel style, as introduced in 2017.
    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V2, LookAndFeel_V3

    @tags{GUI}
*/
class JUCE_API  LookAndFeel_V4   : public LookAndFeel_V3
{
public:
    /**
         A struct containing the set of colors to apply to the GUI
    */
    class ColorScheme
    {
    public:
        /** The standard set of colors to use. */
        enum UIColor
        {
            windowBackground = 0,
            widgetBackground,
            menuBackground,
            outline,
            defaultText,
            defaultFill,
            highlightedText,
            highlightedFill,
            menuText,

            numColors
        };

        template <typename... ItemColors>
        ColorScheme (ItemColors... colorsToUse)
        {
            static_assert (sizeof... (colorsToUse) == numColors, "Must supply one color for each UIColor item");
            const Color c[] = { Color (colorsToUse)... };

            for (int i = 0; i < numColors; ++i)
                palette[i] = c[i];
        }

        ColorScheme (const ColorScheme&) = default;
        ColorScheme& operator= (const ColorScheme&) = default;

        /** Returns a color from the scheme */
        Color getUIColor (UIColor colorToGet) const noexcept;

        /** Sets a scheme color. */
        void setUIColor (UIColor colorToSet, Color newColor) noexcept;

        /** Returns true if two ColorPalette objects contain the same colors. */
        bool operator== (const ColorScheme&) const noexcept;
        /** Returns false if two ColorPalette objects contain the same colors. */
        bool operator!= (const ColorScheme&) const noexcept;

    private:
        Color palette[numColors];
    };

    //==============================================================================
    /** Creates a LookAndFeel_V4 object with a default color scheme. */
    LookAndFeel_V4();

    /** Creates a LookAndFeel_V4 object with a given color scheme. */
    LookAndFeel_V4 (ColorScheme);

    /** Destructor. */
    ~LookAndFeel_V4();

    //==============================================================================
    void setColorScheme (ColorScheme);
    ColorScheme& getCurrentColorScheme() noexcept       { return currentColorScheme; }

    static ColorScheme getDarkColorScheme();
    static ColorScheme getMidnightColorScheme();
    static ColorScheme getGrayColorScheme();
    static ColorScheme getLightColorScheme();

    //==============================================================================
    Button* createDocumentWindowButton (int) override;
    void positionDocumentWindowButtons (DocumentWindow&, int, int, int, int, Button*, Button*, Button*, bool) override;
    void drawDocumentWindowTitleBar (DocumentWindow&, Graphics&, int, int, int, int, const Image*, bool) override;

    //==============================================================================
    Font getTextButtonFont (TextButton&, int buttonHeight) override;

    void drawButtonBackground (Graphics&, Button&, const Color& backgroundColor,
                               bool isMouseOverButton, bool isButtonDown) override;

    void drawToggleButton (Graphics&, ToggleButton&, bool isMouseOverButton, bool isButtonDown) override;
    void drawTickBox (Graphics&, Component&,
                      float x, float y, float w, float h,
                      bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override;

    void changeToggleButtonWidthToFitText (ToggleButton&) override;

    //==============================================================================
    AlertWindow* createAlertWindow (const String& title, const String& message,
                                    const String& button1,
                                    const String& button2,
                                    const String& button3,
                                    AlertWindow::AlertIconType iconType,
                                    int numButtons, Component* associatedComponent) override;
    void drawAlertBox (Graphics&, AlertWindow&, const Rectangle<int>& textArea, TextLayout&) override;

    int getAlertWindowButtonHeight() override;
    Font getAlertWindowTitleFont() override;
    Font getAlertWindowMessageFont() override;
    Font getAlertWindowFont() override;

    //==============================================================================
    void drawProgressBar (Graphics&, ProgressBar&, int width, int height, double progress, const String& textToShow) override;
    bool isProgressBarOpaque (ProgressBar&) override    { return false; }

    //==============================================================================
    int getDefaultScrollbarWidth() override;
    void drawScrollbar (Graphics&, ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override;

    //==============================================================================
    Path getTickShape (float height) override;
    Path getCrossShape (float height) override;

    //==============================================================================
    void fillTextEditorBackground (Graphics&, int width, int height, TextEditor&) override;
    void drawTextEditorOutline (Graphics&, int width, int height, TextEditor&) override;

    //==============================================================================
    Button* createFileBrowserGoUpButton() override;

    void layoutFileBrowserComponent (FileBrowserComponent&,
                                     DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*,
                                     ComboBox* currentPathBox,
                                     TextEditor* filenameBox,
                                     Button* goUpButton) override;

    void drawFileBrowserRow (Graphics&, int width, int height,
                             const File& file, const String& filename, Image* icon,
                             const String& fileSizeDescription, const String& fileTimeDescription,
                             bool isDirectory, bool isItemSelected, int itemIndex,
                             DirectoryContentsDisplayComponent&) override;

    //==============================================================================
    void drawPopupMenuItem (Graphics&, const Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                            const String& text, const String& shortcutKeyText,
                            const Drawable* icon, const Color* textColor) override;

    void getIdealPopupMenuItemSize (const String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;

    void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;

    void drawMenuBarItem (Graphics&, int width, int height,
                          int itemIndex, const String& itemText,
                          bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                          MenuBarComponent&) override;

    //==============================================================================
    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       ComboBox&) override;
    Font getComboBoxFont (ComboBox&) override;
    void positionComboBoxText (ComboBox&, Label&) override;

    //==============================================================================
    int getSliderThumbRadius (Slider&) override;

    void drawLinearSlider (Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle, Slider&) override;

    void drawRotarySlider (Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, Slider&) override;

    void drawPointer (Graphics&, float x, float y, float diameter,
                      const Color&, int direction) noexcept;

    Label* createSliderTextBox (Slider&) override;

    //==============================================================================
    void drawTooltip (Graphics&, const String& text, int width, int height) override;

    //==============================================================================
    void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area,
                                    bool isMouseOver, bool isMouseDown,
                                    ConcertinaPanel&, Component& panel) override;

    //==============================================================================
    void drawLevelMeter (Graphics&, int, int, float) override;

    //==============================================================================
    void paintToolbarBackground (Graphics&, int width, int height, Toolbar&) override;

    void paintToolbarButtonLabel (Graphics&, int x, int y, int width, int height,
                                  const String& text, ToolbarItemComponent&) override;

    //==============================================================================
    void drawPropertyPanelSectionHeader (Graphics&, const String& name, bool isOpen, int width, int height) override;
    void drawPropertyComponentBackground (Graphics&, int width, int height, PropertyComponent&) override;
    void drawPropertyComponentLabel (Graphics&, int width, int height, PropertyComponent&) override;
    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&) override;

    //==============================================================================
    void drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path&, Image&) override;

    //==============================================================================
    void drawStretchableLayoutResizerBar (Graphics&, int, int, bool, bool, bool) override;

private:
    //==============================================================================
    void drawLinearProgressBar (Graphics&, ProgressBar&, int width, int height, double progress, const String&);
    void drawCircularProgressBar (Graphics&, ProgressBar&, const String&);

    int getPropertyComponentIndent (PropertyComponent&);

    //==============================================================================
    void initializeColors();
    ColorScheme currentColorScheme;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V4)
};

} // namespace juce
