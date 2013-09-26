/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCE_LOOKANDFEEL_H_INCLUDED
#define JUCE_LOOKANDFEEL_H_INCLUDED


//==============================================================================
/**
    LookAndFeel objects define the appearance of all the JUCE widgets, and subclasses
    can be used to apply different 'skins' to the application.

*/
class JUCE_API  LookAndFeel
{
public:
    //==============================================================================
    /** Creates the default JUCE look and feel. */
    LookAndFeel();

    /** Destructor. */
    virtual ~LookAndFeel();

    //==============================================================================
    /** Returns the current default look-and-feel for a component to use when it
        hasn't got one explicitly set.

        @see setDefaultLookAndFeel
    */
    static LookAndFeel& getDefaultLookAndFeel() noexcept;

    /** Changes the default look-and-feel.

        @param newDefaultLookAndFeel    the new look-and-feel object to use - if this is
                                        set to null, it will revert to using the default one. The
                                        object passed-in must be deleted by the caller when
                                        it's no longer needed.
        @see getDefaultLookAndFeel
    */
    static void setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) noexcept;


    //==============================================================================
    /** Looks for a colour that has been registered with the given colour ID number.

        If a colour has been set for this ID number using setColour(), then it is
        returned. If none has been set, it will just return Colours::black.

        The colour IDs for various purposes are stored as enums in the components that
        they are relevent to - for an example, see Slider::ColourIds,
        Label::ColourIds, TextEditor::ColourIds, TreeView::ColourIds, etc.

        If you're looking up a colour for use in drawing a component, it's usually
        best not to call this directly, but to use the Component::findColour() method
        instead. That will first check whether a suitable colour has been registered
        directly with the component, and will fall-back on calling the component's
        LookAndFeel's findColour() method if none is found.

        @see setColour, Component::findColour, Component::setColour
    */
    Colour findColour (int colourId) const noexcept;

    /** Registers a colour to be used for a particular purpose.

        For more details, see the comments for findColour().

        @see findColour, Component::findColour, Component::setColour
    */
    void setColour (int colourId, Colour colour) noexcept;

    /** Returns true if the specified colour ID has been explicitly set using the
        setColour() method.
    */
    bool isColourSpecified (int colourId) const noexcept;


    //==============================================================================
    virtual Typeface::Ptr getTypefaceForFont (const Font& font);

    /** Allows you to change the default sans-serif font.

        If you need to supply your own Typeface object for any of the default fonts, rather
        than just supplying the name (e.g. if you want to use an embedded font), then
        you should instead override getTypefaceForFont() to create and return the typeface.
    */
    void setDefaultSansSerifTypefaceName (const String& newName);

    //==============================================================================
    /** Override this to get the chance to swap a component's mouse cursor for a
        customised one.
    */
    virtual MouseCursor getMouseCursorFor (Component& component);

    //==============================================================================
    // Creates a new graphics context object.
    virtual LowLevelGraphicsContext* createGraphicsContext (const Image& imageToRenderOn,
                                                            const Point<int>& origin,
                                                            const RectangleList<int>& initialClip);

    //==============================================================================
    /** Draws the lozenge-shaped background for a standard button. */
    virtual void drawButtonBackground (Graphics&,
                                       Button& button,
                                       const Colour& backgroundColour,
                                       bool isMouseOverButton,
                                       bool isButtonDown);

    virtual Font getTextButtonFont (TextButton& button);

    /** Draws the text for a TextButton. */
    virtual void drawButtonText (Graphics&,
                                 TextButton& button,
                                 bool isMouseOverButton,
                                 bool isButtonDown);

    /** Draws the contents of a standard ToggleButton. */
    virtual void drawToggleButton (Graphics&,
                                   ToggleButton& button,
                                   bool isMouseOverButton,
                                   bool isButtonDown);

    virtual void changeToggleButtonWidthToFitText (ToggleButton& button);

    virtual void drawTickBox (Graphics&,
                              Component& component,
                              float x, float y, float w, float h,
                              bool ticked,
                              bool isEnabled,
                              bool isMouseOverButton,
                              bool isButtonDown);

    virtual void drawDrawableButton (Graphics&,
                                     DrawableButton& button,
                                     bool isMouseOverButton,
                                     bool isButtonDown);

    //==============================================================================
    // AlertWindow handling..

    virtual AlertWindow* createAlertWindow (const String& title,
                                            const String& message,
                                            const String& button1,
                                            const String& button2,
                                            const String& button3,
                                            AlertWindow::AlertIconType iconType,
                                            int numButtons,
                                            Component* associatedComponent);

    virtual void drawAlertBox (Graphics&,
                               AlertWindow& alert,
                               const Rectangle<int>& textArea,
                               TextLayout& textLayout);

    virtual int getAlertBoxWindowFlags();

    virtual int getAlertWindowButtonHeight();

    virtual Font getAlertWindowMessageFont();
    virtual Font getAlertWindowFont();

    void setUsingNativeAlertWindows (bool shouldUseNativeAlerts);
    bool isUsingNativeAlertWindows();

    /** Draws a progress bar.

        If the progress value is less than 0 or greater than 1.0, this should draw a spinning
        bar that fills the whole space (i.e. to say that the app is still busy but the progress
        isn't known). It can use the current time as a basis for playing an animation.

        (Used by progress bars in AlertWindow).
    */
    virtual void drawProgressBar (Graphics&, ProgressBar& progressBar,
                                  int width, int height,
                                  double progress, const String& textToShow);

    //==============================================================================
    // Draws a small image that spins to indicate that something's happening..
    // This method should use the current time to animate itself, so just keep
    // repainting it every so often.
    virtual void drawSpinningWaitAnimation (Graphics&, const Colour& colour,
                                            int x, int y, int w, int h);

    //==============================================================================
    virtual bool areScrollbarButtonsVisible();

    /** Draws one of the buttons on a scrollbar.

        @param g                    the context to draw into
        @param scrollbar            the bar itself
        @param width                the width of the button
        @param height               the height of the button
        @param buttonDirection      the direction of the button, where 0 = up, 1 = right, 2 = down, 3 = left
        @param isScrollbarVertical  true if it's a vertical bar, false if horizontal
        @param isMouseOverButton    whether the mouse is currently over the button (also true if it's held down)
        @param isButtonDown         whether the mouse button's held down
    */
    virtual void drawScrollbarButton (Graphics& g,
                                      ScrollBar& scrollbar,
                                      int width, int height,
                                      int buttonDirection,
                                      bool isScrollbarVertical,
                                      bool isMouseOverButton,
                                      bool isButtonDown);

    /** Draws the thumb area of a scrollbar.

        @param g                    the context to draw into
        @param scrollbar            the bar itself
        @param x                    the x position of the left edge of the thumb area to draw in
        @param y                    the y position of the top edge of the thumb area to draw in
        @param width                the width of the thumb area to draw in
        @param height               the height of the thumb area to draw in
        @param isScrollbarVertical  true if it's a vertical bar, false if horizontal
        @param thumbStartPosition   for vertical bars, the y coordinate of the top of the
                                    thumb, or its x position for horizontal bars
        @param thumbSize            for vertical bars, the height of the thumb, or its width for
                                    horizontal bars. This may be 0 if the thumb shouldn't be drawn.
        @param isMouseOver          whether the mouse is over the thumb area, also true if the mouse is
                                    currently dragging the thumb
        @param isMouseDown          whether the mouse is currently dragging the scrollbar
    */
    virtual void drawScrollbar (Graphics& g,
                                ScrollBar& scrollbar,
                                int x, int y,
                                int width, int height,
                                bool isScrollbarVertical,
                                int thumbStartPosition,
                                int thumbSize,
                                bool isMouseOver,
                                bool isMouseDown);

    /** Returns the component effect to use for a scrollbar */
    virtual ImageEffectFilter* getScrollbarEffect();

    /** Returns the minimum length in pixels to use for a scrollbar thumb. */
    virtual int getMinimumScrollbarThumbSize (ScrollBar& scrollbar);

    /** Returns the default thickness to use for a scrollbar. */
    virtual int getDefaultScrollbarWidth();

    /** Returns the length in pixels to use for a scrollbar button. */
    virtual int getScrollbarButtonSize (ScrollBar& scrollbar);

    //==============================================================================
    /** Returns a tick shape for use in yes/no boxes, etc. */
    virtual Path getTickShape (float height);
    /** Returns a cross shape for use in yes/no boxes, etc. */
    virtual Path getCrossShape (float height);

    //==============================================================================
    /** Draws the + or - box in a treeview. */
    virtual void drawTreeviewPlusMinusBox (Graphics&, int x, int y, int w, int h, bool isPlus, bool isMouseOver);

    //==============================================================================
    virtual void fillTextEditorBackground (Graphics&, int width, int height, TextEditor& textEditor);
    virtual void drawTextEditorOutline (Graphics&, int width, int height, TextEditor& textEditor);

    virtual CaretComponent* createCaretComponent (Component* keyFocusOwner);

    //==============================================================================
    // These return a pointer to an internally cached drawable - make sure you don't keep
    // a copy of this pointer anywhere, as it may become invalid in the future.
    virtual const Drawable* getDefaultFolderImage();
    virtual const Drawable* getDefaultDocumentFileImage();

    virtual AttributedString createFileChooserHeaderText (const String& title,
                                                          const String& instructions);

    virtual void drawFileBrowserRow (Graphics&, int width, int height,
                                     const String& filename, Image* icon,
                                     const String& fileSizeDescription,
                                     const String& fileTimeDescription,
                                     bool isDirectory,
                                     bool isItemSelected,
                                     int itemIndex,
                                     DirectoryContentsDisplayComponent& component);

    virtual Button* createFileBrowserGoUpButton();

    virtual void layoutFileBrowserComponent (FileBrowserComponent& browserComp,
                                             DirectoryContentsDisplayComponent* fileListComponent,
                                             FilePreviewComponent* previewComp,
                                             ComboBox* currentPathBox,
                                             TextEditor* filenameBox,
                                             Button* goUpButton);

    //==============================================================================
    virtual void drawBubble (Graphics&, BubbleComponent&,
                             const Point<float>& tip, const Rectangle<float>& body);

    //==============================================================================
    virtual void drawLasso (Graphics&, Component& lassoComp);

    //==============================================================================
    /** Fills the background of a popup menu component. */
    virtual void drawPopupMenuBackground (Graphics&, int width, int height);

    /** Draws one of the items in a popup menu. */
    virtual void drawPopupMenuItem (Graphics&,
                                    int width, int height,
                                    bool isSeparator,
                                    bool isActive,
                                    bool isHighlighted,
                                    bool isTicked,
                                    bool hasSubMenu,
                                    const String& text,
                                    const String& shortcutKeyText,
                                    Image* image,
                                    const Colour* const textColour);

    /** Returns the size and style of font to use in popup menus. */
    virtual Font getPopupMenuFont();

    virtual void drawPopupMenuUpDownArrow (Graphics&,
                                           int width, int height,
                                           bool isScrollUpArrow);

    /** Finds the best size for an item in a popup menu. */
    virtual void getIdealPopupMenuItemSize (const String& text,
                                            bool isSeparator,
                                            int standardMenuItemHeight,
                                            int& idealWidth,
                                            int& idealHeight);

    virtual int getMenuWindowFlags();

    virtual void drawMenuBarBackground (Graphics&, int width, int height,
                                        bool isMouseOverBar,
                                        MenuBarComponent& menuBar);

    virtual int getMenuBarItemWidth (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

    virtual Font getMenuBarFont (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

    virtual void drawMenuBarItem (Graphics&,
                                  int width, int height,
                                  int itemIndex,
                                  const String& itemText,
                                  bool isMouseOverItem,
                                  bool isMenuOpen,
                                  bool isMouseOverBar,
                                  MenuBarComponent& menuBar);

    //==============================================================================
    virtual void drawComboBox (Graphics&, int width, int height,
                               bool isButtonDown,
                               int buttonX, int buttonY,
                               int buttonW, int buttonH,
                               ComboBox& box);

    virtual Font getComboBoxFont (ComboBox& box);

    virtual Label* createComboBoxTextBox (ComboBox& box);

    virtual void positionComboBoxText (ComboBox& box, Label& labelToPosition);

    //==============================================================================
    virtual void drawLabel (Graphics&, Label&);

    virtual Font getLabelFont (Label&);

    //==============================================================================
    virtual void drawLinearSlider (Graphics&,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider);

    virtual void drawLinearSliderBackground (Graphics&,
                                             int x, int y,
                                             int width, int height,
                                             float sliderPos,
                                             float minSliderPos,
                                             float maxSliderPos,
                                             const Slider::SliderStyle style,
                                             Slider& slider);

    virtual void drawLinearSliderThumb (Graphics&,
                                        int x, int y,
                                        int width, int height,
                                        float sliderPos,
                                        float minSliderPos,
                                        float maxSliderPos,
                                        const Slider::SliderStyle style,
                                        Slider& slider);

    virtual int getSliderThumbRadius (Slider& slider);

    virtual void drawRotarySlider (Graphics&,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPosProportional,
                                   float rotaryStartAngle,
                                   float rotaryEndAngle,
                                   Slider& slider);

    virtual Button* createSliderButton (bool isIncrement);
    virtual Label* createSliderTextBox (Slider& slider);

    virtual ImageEffectFilter* getSliderEffect();

    virtual Font getSliderPopupFont();
    virtual int getSliderPopupPlacement();

    //==============================================================================
    virtual void getTooltipSize (const String& tipText, int& width, int& height);

    virtual void drawTooltip (Graphics&, const String& text, int width, int height);

    //==============================================================================
    virtual Button* createFilenameComponentBrowseButton (const String& text);

    virtual void layoutFilenameComponent (FilenameComponent& filenameComp,
                                          ComboBox* filenameBox, Button* browseButton);

    //==============================================================================
    virtual void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area,
                                            bool isMouseOver, bool isMouseDown,
                                            ConcertinaPanel&, Component& panel);

    //==============================================================================
    virtual void drawCornerResizer (Graphics&,
                                    int w, int h,
                                    bool isMouseOver,
                                    bool isMouseDragging);

    virtual void drawResizableFrame (Graphics&,
                                    int w, int h,
                                    const BorderSize<int>&);

    //==============================================================================
    virtual void fillResizableWindowBackground (Graphics&, int w, int h,
                                                const BorderSize<int>&,
                                                ResizableWindow& window);

    virtual void drawResizableWindowBorder (Graphics&,
                                            int w, int h,
                                            const BorderSize<int>& border,
                                            ResizableWindow& window);

    //==============================================================================
    virtual void drawDocumentWindowTitleBar (DocumentWindow& window,
                                             Graphics&, int w, int h,
                                             int titleSpaceX, int titleSpaceW,
                                             const Image* icon,
                                             bool drawTitleTextOnLeft);

    virtual Button* createDocumentWindowButton (int buttonType);

    virtual void positionDocumentWindowButtons (DocumentWindow& window,
                                                int titleBarX, int titleBarY,
                                                int titleBarW, int titleBarH,
                                                Button* minimiseButton,
                                                Button* maximiseButton,
                                                Button* closeButton,
                                                bool positionTitleBarButtonsOnLeft);

    virtual int getDefaultMenuBarHeight();

    //==============================================================================
    virtual DropShadower* createDropShadowerForComponent (Component* component);

    //==============================================================================
    virtual void drawStretchableLayoutResizerBar (Graphics&,
                                                  int w, int h,
                                                  bool isVerticalBar,
                                                  bool isMouseOver,
                                                  bool isMouseDragging);

    //==============================================================================
    virtual void drawGroupComponentOutline (Graphics&, int w, int h,
                                            const String& text,
                                            const Justification& position,
                                            GroupComponent& group);

    //==============================================================================
    virtual int getTabButtonSpaceAroundImage();
    virtual int getTabButtonOverlap (int tabDepth);
    virtual int getTabButtonBestWidth (TabBarButton&, int tabDepth);
    virtual Rectangle<int> getTabButtonExtraComponentBounds (const TabBarButton&, Rectangle<int>& textArea, Component& extraComp);

    virtual void drawTabButton (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown);
    virtual void drawTabButtonText (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown);
    virtual void drawTabbedButtonBarBackground (TabbedButtonBar&, Graphics&);
    virtual void drawTabAreaBehindFrontButton (TabbedButtonBar&, Graphics&, int w, int h);

    virtual void createTabButtonShape (TabBarButton&, Path& path,  bool isMouseOver, bool isMouseDown);
    virtual void fillTabButtonShape (TabBarButton&, Graphics&, const Path& path, bool isMouseOver, bool isMouseDown);

    virtual Button* createTabBarExtrasButton();

    //==============================================================================
    virtual void drawImageButton (Graphics&, Image* image,
                                  int imageX, int imageY, int imageW, int imageH,
                                  const Colour& overlayColour,
                                  float imageOpacity,
                                  ImageButton& button);

    //==============================================================================
    virtual void drawTableHeaderBackground (Graphics&, TableHeaderComponent&);

    virtual void drawTableHeaderColumn (Graphics&, const String& columnName, int columnId,
                                        int width, int height,
                                        bool isMouseOver, bool isMouseDown,
                                        int columnFlags);

    //==============================================================================
    virtual void paintToolbarBackground (Graphics&, int width, int height, Toolbar& toolbar);

    virtual Button* createToolbarMissingItemsButton (Toolbar& toolbar);

    virtual void paintToolbarButtonBackground (Graphics&, int width, int height,
                                               bool isMouseOver, bool isMouseDown,
                                               ToolbarItemComponent& component);

    virtual void paintToolbarButtonLabel (Graphics&, int x, int y, int width, int height,
                                          const String& text, ToolbarItemComponent& component);

    //==============================================================================
    virtual void drawPropertyPanelSectionHeader (Graphics&, const String& name,
                                                 bool isOpen, int width, int height);

    virtual void drawPropertyComponentBackground (Graphics&, int width, int height,
                                                  PropertyComponent& component);

    virtual void drawPropertyComponentLabel (Graphics&, int width, int height,
                                             PropertyComponent& component);

    virtual Rectangle<int> getPropertyComponentContentPosition (PropertyComponent& component);

    //==============================================================================
    virtual void drawCallOutBoxBackground (CallOutBox& box, Graphics&, const Path& path, Image& cachedImage);

    //==============================================================================
    virtual void drawLevelMeter (Graphics&, int width, int height, float level);

    virtual void drawKeymapChangeButton (Graphics&, int width, int height, Button& button, const String& keyDescription);

    //==============================================================================
    /** Plays the system's default 'beep' noise, to alert the user about something very important.
    */
    virtual void playAlertSound();

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
    static void drawGlassSphere (Graphics&,
                                 float x, float y,
                                 float diameter,
                                 const Colour& colour,
                                 float outlineThickness) noexcept;

    static void drawGlassPointer (Graphics&,
                                  float x, float y,
                                  float diameter,
                                  const Colour& colour, float outlineThickness,
                                  int direction) noexcept;

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static void drawGlassLozenge (Graphics&,
                                  float x, float y,
                                  float width, float height,
                                  const Colour& colour,
                                  float outlineThickness,
                                  float cornerSize,
                                  bool flatOnLeft, bool flatOnRight,
                                  bool flatOnTop, bool flatOnBottom) noexcept;

    static Drawable* loadDrawableFromData (const void* data, size_t numBytes);

private:
    //==============================================================================
    friend class WeakReference<LookAndFeel>;
    WeakReference<LookAndFeel>::Master masterReference;

    struct ColourSetting
    {
        int colourID;
        Colour colour;

        bool operator<  (const ColourSetting& other) const noexcept  { return colourID <  other.colourID; }
        bool operator== (const ColourSetting& other) const noexcept  { return colourID == other.colourID; }
    };

    SortedSet<ColourSetting> colours;

    // default typeface names
    String defaultSans, defaultSerif, defaultFixed;

    ScopedPointer<Drawable> folderImage, documentImage;

    bool useNativeAlertWindows;

    void drawShinyButtonShape (Graphics&,
                               float x, float y, float w, float h, float maxCornerSize,
                               const Colour& baseColour,
                               float strokeWidth,
                               bool flatOnLeft,
                               bool flatOnRight,
                               bool flatOnTop,
                               bool flatOnBottom) noexcept;

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // These methods have been deprecated - see their new parameter lists..
    virtual int drawFileBrowserRow (Graphics&, int, int, const String&, Image*, const String&, const String&, bool, bool, int) { return 0; }
    virtual int drawTabButton (Graphics&, int, int, const Colour&, int, const String&, Button&, TabbedButtonBar::Orientation, bool, bool, bool) { return 0; }
    virtual int createTabButtonShape (Path&, int, int, int, const String&, Button&, TabbedButtonBar::Orientation, bool, bool, bool) { return 0; }
    virtual int fillTabButtonShape (Graphics&, const Path&, const Colour&, int, const String&, Button&, TabbedButtonBar::Orientation, bool, bool, bool) { return 0; }
    virtual int drawTabAreaBehindFrontButton (Graphics&, int, int, TabbedButtonBar&, TabbedButtonBar::Orientation) { return 0; }
    virtual int drawTabButtonText (Graphics&, int, int, int, int, const Colour&, int, const String&, Button&, TabbedButtonBar::Orientation, bool, bool, bool) { return 0; }
    virtual int getTabButtonBestWidth (int, const String&, int, Button&) { return 0; }
    virtual int drawBubble (Graphics&, float, float, float, float, float, float) { return 0; }
    virtual int getFontForTextButton (TextButton&) { return 0; }
    virtual int createFileChooserHeaderText (const String&, const String&, GlyphArrangement&, int) { return 0; }
   #endif

    class GlassWindowButton;
    class SliderLabelComp;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel)
};


#endif   // JUCE_LOOKANDFEEL_H_INCLUDED
