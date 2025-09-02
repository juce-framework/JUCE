/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
    ~LookAndFeel_V2() override;

    //==============================================================================
    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    Font getTextButtonFont (TextButton&, int buttonHeight) override;

    void drawButtonText (Graphics&, TextButton&,
                         bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
    int getTextButtonWidthToFitText (TextButton&, int buttonHeight) override;

    void drawToggleButton (Graphics&, ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void changeToggleButtonWidthToFitText (ToggleButton&) override;

    void drawTickBox (Graphics&, Component&,
                      float x, float y, float w, float h,
                      bool ticked, bool isEnabled,
                      bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawDrawableButton (Graphics&, DrawableButton&,
                             bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    //==============================================================================
    AlertWindow* createAlertWindow (const String& title, const String& message,
                                    const String& button1,
                                    const String& button2,
                                    const String& button3,
                                    MessageBoxIconType iconType,
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
    ProgressBar::Style getDefaultProgressBarStyle (const ProgressBar&) override;

    //==============================================================================
    bool areScrollbarButtonsVisible() override;
    void drawScrollbarButton (Graphics&, ScrollBar&, int width, int height, int buttonDirection,
                              bool isScrollbarVertical, bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

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
    void setComponentEffectForBubbleComponent (BubbleComponent& bubbleComponent) override;

    void drawLasso (Graphics&, Component&) override;

    //==============================================================================
    void drawPopupMenuBackground (Graphics&, int width, int height) override;
    void drawPopupMenuBackgroundWithOptions (Graphics&,
                                             int width,
                                             int height,
                                             const PopupMenu::Options&) override;

    void drawPopupMenuItem (Graphics&, const Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked, bool hasSubMenu,
                            const String& text, const String& shortcutKeyText,
                            const Drawable* icon, const Colour* textColour) override;

    void drawPopupMenuItemWithOptions (Graphics&, const Rectangle<int>& area,
                                       bool isHighlighted,
                                       const PopupMenu::Item& item,
                                       const PopupMenu::Options&) override;

    void drawPopupMenuSectionHeader (Graphics&, const Rectangle<int>& area,
                                     const String& sectionName) override;

    void drawPopupMenuSectionHeaderWithOptions (Graphics&, const Rectangle<int>& area,
                                                const String& sectionName,
                                                const PopupMenu::Options&) override;

    Font getPopupMenuFont() override;

    void drawPopupMenuUpDownArrow (Graphics&, int width, int height, bool isScrollUpArrow) override;

    void drawPopupMenuUpDownArrowWithOptions (Graphics&,
                                              int width, int height,
                                              bool isScrollUpArrow,
                                              const PopupMenu::Options&) override;

    void getIdealPopupMenuItemSize (const String& text, bool isSeparator, int standardMenuItemHeight,
                                    int& idealWidth, int& idealHeight) override;

    void getIdealPopupMenuItemSizeWithOptions (const String& text,
                                               bool isSeparator,
                                               int standardMenuItemHeight,
                                               int& idealWidth,
                                               int& idealHeight,
                                               const PopupMenu::Options&) override;

    void getIdealPopupMenuSectionHeaderSizeWithOptions (const String& text,
                                                        int standardMenuItemHeight,
                                                        int& idealWidth,
                                                        int& idealHeight,
                                                        const PopupMenu::Options&) override;

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

    int getPopupMenuBorderSizeWithOptions (const PopupMenu::Options&) override;

    void drawPopupMenuColumnSeparatorWithOptions (Graphics& g,
                                                  const Rectangle<int>& bounds,
                                                  const PopupMenu::Options&) override;

    int getPopupMenuColumnSeparatorWidthWithOptions (const PopupMenu::Options&) override;

    //==============================================================================
    void drawComboBox (Graphics&, int width, int height, bool isMouseButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       ComboBox&) override;
    Font getComboBoxFont (ComboBox&) override;
    Label* createComboBoxTextBox (ComboBox&) override;
    void positionComboBoxText (ComboBox&, Label&) override;
    PopupMenu::Options getOptionsForComboBoxPopupMenu (ComboBox&, Label&) override;
    void drawComboBoxTextWhenNothingSelected (Graphics&, ComboBox&, Label&) override;

    //==============================================================================
    void drawLabel (Graphics&, Label&) override;
    Font getLabelFont (Label&) override;
    BorderSize<int> getLabelBorderSize (Label&) override;

    //==============================================================================
    void drawLinearSlider (Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           Slider::SliderStyle, Slider&) override;

    void drawLinearSliderBackground (Graphics&, int x, int y, int width, int height,
                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                     Slider::SliderStyle, Slider&) override;

    void drawLinearSliderOutline (Graphics&, int x, int y, int width, int height,
                                  Slider::SliderStyle, Slider&) override;


    void drawLinearSliderThumb (Graphics&, int x, int y, int width, int height,
                                float sliderPos, float minSliderPos, float maxSliderPos,
                                Slider::SliderStyle, Slider&) override;

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
    std::unique_ptr<DropShadower> createDropShadowerForComponent (Component&) override;
    std::unique_ptr<FocusOutline> createFocusOutlineForComponent (Component&) override;

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
    float getCallOutBoxCornerSize (const CallOutBox&) override;

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
    std::unique_ptr<Drawable> folderImage, documentImage;
    DropShadowEffect bubbleShadow;

    void drawShinyButtonShape (Graphics&,
                               float x, float y, float w, float h, float maxCornerSize,
                               const Colour&, float strokeWidth,
                               bool flatOnLeft, bool flatOnRight, bool flatOnTop, bool flatOnBottom) noexcept;

    class GlassWindowButton;
    class SliderLabelComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V2)
};

} // namespace juce
