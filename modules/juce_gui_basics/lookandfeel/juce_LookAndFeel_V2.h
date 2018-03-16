/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
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
    This LookAndFeel subclass implements the juce style from around 2008-12.

    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V3

    @tags{GUI}
*/
class JUCE_API  LookAndFeel_V2  : public LookAndFeel
{
public:
    LookAndFeel_V2();
    ~LookAndFeel_V2();

    //==============================================================================
    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;
    Font getTextButtonFont (TextButton&, int buttonHeight) override;

    void drawButtonText (Graphics&, TextButton&, bool isMouseOverButton, bool isButtonDown) override;
    int getTextButtonWidthToFitText (TextButton&, int buttonHeight) override;

    void drawToggleButton (Graphics&, ToggleButton&, bool isMouseOverButton, bool isButtonDown) override;

    void changeToggleButtonWidthToFitText (ToggleButton&) override;

    void drawTickBox (Graphics&, Component&,
                      float x, float y, float w, float h,
                      bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override;

    void drawDrawableButton (Graphics&, DrawableButton&, bool isMouseOverButton, bool isButtonDown) override;

    //==============================================================================
    AlertWindow* createAlertWindow (const String& title, const String& message,
                                    const String& button1,
                                    const String& button2,
                                    const String& button3,
                                    AlertWindow::AlertIconType iconType,
                                    int numButtons, Component* associatedComponent) override;

    void drawAlertBox (Graphics&, AlertWindow&, const Rectangle<int>& textArea, TextLayout&) override;
    int getAlertBoxWindowFlags() override;

    Array<int> getWidthsForTextButtons (AlertWindow&, const Array<TextButton*>&) override;
    int getAlertWindowButtonHeight() override;

    /** Override this function to supply a custom font for the alert window title.
        This default implementation will use a boldened and slightly larger version
        of the alert window message font.

        @see getAlertWindowMessageFont.
    */
    Font getAlertWindowTitleFont() override;

    /** Override this function to supply a custom font for the alert window message.
        This default implementation will use the default font with height set to 15.0f.

        @see getAlertWindowTitleFont
    */
    Font getAlertWindowMessageFont() override;

    Font getAlertWindowFont() override;

    //==============================================================================
    void drawProgressBar (Graphics&, ProgressBar&, int width, int height, double progress, const String& textToShow) override;
    void drawSpinningWaitAnimation (Graphics&, const Colour& colour, int x, int y, int w, int h) override;
    bool isProgressBarOpaque (ProgressBar&) override;

    //==============================================================================
    bool areScrollbarButtonsVisible() override;
    void drawScrollbarButton (Graphics&, ScrollBar&, int width, int height, int buttonDirection,
                              bool isScrollbarVertical, bool isMouseOverButton, bool isButtonDown) override;

    void drawScrollbar (Graphics&, ScrollBar&, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

    ImageEffectFilter* getScrollbarEffect() override;
    int getMinimumScrollbarThumbSize (ScrollBar&) override;
    int getDefaultScrollbarWidth() override;
    int getScrollbarButtonSize (ScrollBar&) override;

    //==============================================================================
    Path getTickShape (float height) override;
    Path getCrossShape (float height) override;

    //==============================================================================
    void drawTreeviewPlusMinusBox (Graphics&, const Rectangle<float>& area,
                                   Colour backgroundColour, bool isOpen, bool isMouseOver) override;
    bool areLinesDrawnForTreeView (TreeView&) override;
    int getTreeViewIndentSize (TreeView&) override;

    //==============================================================================
    void fillTextEditorBackground (Graphics&, int width, int height, TextEditor&) override;
    void drawTextEditorOutline (Graphics&, int width, int height, TextEditor&) override;
    CaretComponent* createCaretComponent (Component* keyFocusOwner) override;

    //==============================================================================
    const Drawable* getDefaultFolderImage() override;
    const Drawable* getDefaultDocumentFileImage() override;

    AttributedString createFileChooserHeaderText (const String& title, const String& instructions) override;

    void drawFileBrowserRow (Graphics&, int width, int height,
                             const File& file, const String& filename, Image* icon,
                             const String& fileSizeDescription, const String& fileTimeDescription,
                             bool isDirectory, bool isItemSelected, int itemIndex,
                             DirectoryContentsDisplayComponent&) override;

    Button* createFileBrowserGoUpButton() override;

    void layoutFileBrowserComponent (FileBrowserComponent&,
                                     DirectoryContentsDisplayComponent*,
                                     FilePreviewComponent*,
                                     ComboBox* currentPathBox,
                                     TextEditor* filenameBox,
                                     Button* goUpButton) override;

    //==============================================================================
    void drawBubble (Graphics&, BubbleComponent&, const Point<float>& tip, const Rectangle<float>& body) override;

    void drawLasso (Graphics&, Component&) override;

    //==============================================================================
    void drawPopupMenuBackground (Graphics&, int width, int height) override;

    void drawPopupMenuItem (Graphics&, const Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                            const String& text, const String& shortcutKeyText,
                            const Drawable* icon, const Colour* textColour) override;

    void drawPopupMenuSectionHeader (Graphics&, const Rectangle<int>& area,
                                     const String& sectionName) override;

    Font getPopupMenuFont() override;

    void drawPopupMenuUpDownArrow (Graphics&, int width, int height, bool isScrollUpArrow) override;

    void getIdealPopupMenuItemSize (const String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;
    int getMenuWindowFlags() override;
    void preparePopupMenuWindow (Component&) override;

    void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;
    int getMenuBarItemWidth (MenuBarComponent&, int itemIndex, const String& itemText) override;
    Font getMenuBarFont (MenuBarComponent&, int itemIndex, const String& itemText) override;
    int getDefaultMenuBarHeight() override;

    void drawMenuBarItem (Graphics&, int width, int height,
                          int itemIndex, const String& itemText,
                          bool isMouseOverItem, bool isMenuOpen, bool isMouseOverBar,
                          MenuBarComponent&) override;

    Component* getParentComponentForMenuOptions (const PopupMenu::Options& options) override;

    bool shouldPopupMenuScaleWithTargetComponent (const PopupMenu::Options& options) override;

    int getPopupMenuBorderSize() override;

    //==============================================================================
    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       ComboBox&) override;
    Font getComboBoxFont (ComboBox&) override;
    Label* createComboBoxTextBox (ComboBox&) override;
    void positionComboBoxText (ComboBox&, Label&) override;

    //==============================================================================
    void drawLabel (Graphics&, Label&) override;
    Font getLabelFont (Label&) override;

    //==============================================================================
    void drawLinearSlider (Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle, Slider&) override;

    void drawLinearSliderBackground (Graphics&, int x, int y, int width, int height,
                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                     const Slider::SliderStyle, Slider&) override;

    void drawLinearSliderThumb (Graphics&, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                const Slider::SliderStyle, Slider&) override;

    void drawRotarySlider (Graphics&, int x, int y, int width, int height,
                           float sliderPosProportional, float rotaryStartAngle, float rotaryEndAngle,
                           Slider&) override;

    int getSliderThumbRadius (Slider&) override;
    Button* createSliderButton (Slider&, bool isIncrement) override;
    Label* createSliderTextBox (Slider&) override;
    ImageEffectFilter* getSliderEffect (Slider&) override;
    Font getSliderPopupFont (Slider&) override;
    int getSliderPopupPlacement (Slider&) override;
    Slider::SliderLayout getSliderLayout (Slider&) override;

    //==============================================================================
    Rectangle<int> getTooltipBounds (const String& tipText, Point<int> screenPos, Rectangle<int> parentArea) override;
    void drawTooltip (Graphics&, const String& text, int width, int height) override;

    //==============================================================================
    Button* createFilenameComponentBrowseButton (const String& text) override;
    void layoutFilenameComponent (FilenameComponent&, ComboBox* filenameBox, Button* browseButton) override;

    //==============================================================================
    void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area,
                                    bool isMouseOver, bool isMouseDown,
                                    ConcertinaPanel&, Component& panel) override;

    //==============================================================================
    void drawCornerResizer (Graphics&, int w, int h, bool isMouseOver, bool isMouseDragging) override;
    void drawResizableFrame (Graphics&, int w, int h, const BorderSize<int>&) override;

    //==============================================================================
    void fillResizableWindowBackground (Graphics&, int w, int h, const BorderSize<int>&, ResizableWindow&) override;
    void drawResizableWindowBorder (Graphics&, int w, int h, const BorderSize<int>& border, ResizableWindow&) override;

    //==============================================================================
    void drawDocumentWindowTitleBar (DocumentWindow&, Graphics&, int w, int h,
                                     int titleSpaceX, int titleSpaceW,
                                     const Image* icon, bool drawTitleTextOnLeft) override;

    Button* createDocumentWindowButton (int buttonType) override;

    void positionDocumentWindowButtons (DocumentWindow&,
                                        int titleBarX, int titleBarY, int titleBarW, int titleBarH,
                                        Button* minimiseButton,
                                        Button* maximiseButton,
                                        Button* closeButton,
                                        bool positionTitleBarButtonsOnLeft) override;

    //==============================================================================
    DropShadower* createDropShadowerForComponent (Component*) override;

    //==============================================================================
    void drawStretchableLayoutResizerBar (Graphics&, int w, int h, bool isVerticalBar,
                                          bool isMouseOver, bool isMouseDragging) override;

    //==============================================================================
    void drawGroupComponentOutline (Graphics&, int w, int h, const String& text,
                                    const Justification&, GroupComponent&) override;

    //==============================================================================
    int getTabButtonSpaceAroundImage() override;
    int getTabButtonOverlap (int tabDepth) override;
    int getTabButtonBestWidth (TabBarButton&, int tabDepth) override;
    Rectangle<int> getTabButtonExtraComponentBounds (const TabBarButton&, Rectangle<int>& textArea, Component& extraComp) override;

    void drawTabButton (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;
    Font getTabButtonFont (TabBarButton&, float height) override;
    void drawTabButtonText (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&) override;
    void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int w, int h) override;

    void createTabButtonShape (TabBarButton&, Path&,  bool isMouseOver, bool isMouseDown) override;
    void fillTabButtonShape (TabBarButton&, Graphics&, const Path&, bool isMouseOver, bool isMouseDown) override;

    Button* createTabBarExtrasButton() override;

    //==============================================================================
    void drawImageButton (Graphics&, Image*,
                          int imageX, int imageY, int imageW, int imageH,
                          const Colour& overlayColour, float imageOpacity, ImageButton&) override;

    //==============================================================================
    void drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    void drawTableHeaderColumn (Graphics&, TableHeaderComponent&, const String& columnName,
                                int columnId, int width, int height, bool isMouseOver,
                                bool isMouseDown, int columnFlags) override;

    //==============================================================================
    void paintToolbarBackground (Graphics&, int width, int height, Toolbar&) override;

    Button* createToolbarMissingItemsButton (Toolbar&) override;

    void paintToolbarButtonBackground (Graphics&, int width, int height,
                                       bool isMouseOver, bool isMouseDown,
                                       ToolbarItemComponent&) override;

    void paintToolbarButtonLabel (Graphics&, int x, int y, int width, int height,
                                  const String& text, ToolbarItemComponent&) override;

    //==============================================================================
    void drawPropertyPanelSectionHeader (Graphics&, const String& name, bool isOpen, int width, int height) override;
    void drawPropertyComponentBackground (Graphics&, int width, int height, PropertyComponent&) override;
    void drawPropertyComponentLabel (Graphics&, int width, int height, PropertyComponent&) override;
    Rectangle<int> getPropertyComponentContentPosition (PropertyComponent&) override;
    int getPropertyPanelSectionHeaderHeight (const String& sectionTitle) override;

    //==============================================================================
    void drawCallOutBoxBackground (CallOutBox&, Graphics&, const Path& path, Image& cachedImage) override;
    int getCallOutBoxBorderSize (const CallOutBox&) override;

    //==============================================================================
    void drawLevelMeter (Graphics&, int width, int height, float level) override;

    void drawKeymapChangeButton (Graphics&, int width, int height, Button&, const String& keyDescription) override;

    //==============================================================================
    Font getSidePanelTitleFont (SidePanel&) override;
    Justification getSidePanelTitleJustification (SidePanel&) override;
    Path getSidePanelDismissButtonShape (SidePanel&) override;

    //==============================================================================
    /** Draws a 3D raised (or indented) bevel using two colours.

        The bevel is drawn inside the given rectangle, and greater bevel thicknesses
        extend inwards.

        The top-left colour is used for the top- and left-hand edges of the
        bevel; the bottom-right colour is used for the bottom- and right-hand
        edges.

        If useGradient is true, then the bevel fades out to make it look more curved
        and less angular. If sharpEdgeOnOutside is true, the outside of the bevel is
        sharp, and it fades towards the centre; if sharpEdgeOnOutside is false, then
        the centre edges are sharp and it fades towards the outside.
    */
    static void drawBevel (Graphics&,
                           int x, int y, int width, int height,
                           int bevelThickness,
                           const Colour& topLeftColour = Colours::white,
                           const Colour& bottomRightColour = Colours::black,
                           bool useGradient = true,
                           bool sharpEdgeOnOutside = true);

    /** Utility function to draw a shiny, glassy circle (for round LED-type buttons). */
    static void drawGlassSphere (Graphics&, float x, float y, float diameter,
                                 const Colour&, float outlineThickness) noexcept;

    static void drawGlassPointer (Graphics&, float x, float y, float diameter,
                                  const Colour&, float outlineThickness, int direction) noexcept;

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static void drawGlassLozenge (Graphics&,
                                  float x, float y, float width, float height,
                                  const Colour&, float outlineThickness, float cornerSize,
                                  bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom) noexcept;

private:
    //==============================================================================
    ScopedPointer<Drawable> folderImage, documentImage;

    void drawShinyButtonShape (Graphics&,
                               float x, float y, float w, float h, float maxCornerSize,
                               const Colour&, float strokeWidth,
                               bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom) noexcept;

    class GlassWindowButton;
    class SliderLabelComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V2)
};

} // namespace juce
