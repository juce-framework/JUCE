/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#ifndef __JUCE_LOOKANDFEEL_JUCEHEADER__
#define __JUCE_LOOKANDFEEL_JUCEHEADER__

#include "../../graphics/contexts/juce_Graphics.h"
#include "../../graphics/effects/juce_DropShadowEffect.h"
#include "../controls/juce_Slider.h"
#include "../layout/juce_TabbedComponent.h"

class ToggleButton;
class AlertWindow;
class TextLayout;
class ScrollBar;
class BubbleComponent;
class ComboBox;
class Button;
class FilenameComponent;
class DocumentWindow;
class ResizableWindow;
class GroupComponent;
class MenuBarComponent;
class DropShadower;
class GlyphArrangement;
class PropertyComponent;
class TableHeaderComponent;
class Toolbar;
class ToolbarItemComponent;
class PopupMenu;
class ProgressBar;


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
    static LookAndFeel& getDefaultLookAndFeel() throw();

    /** Changes the default look-and-feel.

        @param newDefaultLookAndFeel    the new look-and-feel object to use - if this is
                                        set to 0, it will revert to using the default one. The
                                        object passed-in must be deleted by the caller when
                                        it's no longer needed.
        @see getDefaultLookAndFeel
    */
    static void setDefaultLookAndFeel (LookAndFeel* newDefaultLookAndFeel) throw();


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
    const Colour findColour (const int colourId) const throw();

    /** Registers a colour to be used for a particular purpose.

        For more details, see the comments for findColour().

        @see findColour, Component::findColour, Component::setColour
    */
    void setColour (const int colourId, const Colour& colour) throw();

    //==============================================================================
    /** Draws the lozenge-shaped background for a standard button. */
    virtual void drawButtonBackground (Graphics& g,
                                       Button& button,
                                       const Colour& backgroundColour,
                                       bool isMouseOverButton,
                                       bool isButtonDown);


    /** Draws the contents of a standard ToggleButton. */
    virtual void drawToggleButton (Graphics& g,
                                   ToggleButton& button,
                                   bool isMouseOverButton,
                                   bool isButtonDown);

    virtual void changeToggleButtonWidthToFitText (ToggleButton& button);

    virtual void drawTickBox (Graphics& g,
                              Component& component,
                              int x, int y, int w, int h,
                              const bool ticked,
                              const bool isEnabled,
                              const bool isMouseOverButton,
                              const bool isButtonDown);

    //==============================================================================
    /** Draws the contents of a message box.
    */
    virtual void drawAlertBox (Graphics& g,
                               AlertWindow& alert,
                               const Rectangle& textArea,
                               TextLayout& textLayout);

    virtual int getAlertBoxWindowFlags();

    /** Draws a progress bar.

        (Used by progress bars in AlertWindow).
    */
    virtual void drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                  int x, int y, int w, int h,
                                  float progress);

    //==============================================================================
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
        @param thumbStartPosition   for vertical bars, the y co-ordinate of the top of the
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
    virtual const Path getTickShape (const float height);
    /** Returns a cross shape for use in yes/no boxes, etc. */
    virtual const Path getCrossShape (const float height);

    //==============================================================================
    /** Draws the + or - box in a treeview. */
    virtual void drawTreeviewPlusMinusBox (Graphics& g, int x, int y, int w, int h, bool isPlus);

    //==============================================================================
    virtual void drawTextEditorOutline (Graphics& g,
                                        int width, int height,
                                        TextEditor& textEditor);

    //==============================================================================
    // these return an image from the ImageCache, so use ImageCache::release() to free it
    virtual Image* getDefaultFolderImage();
    virtual Image* getDefaultDocumentFileImage();

    virtual void createFileChooserHeaderText (const String& title,
                                              const String& instructions,
                                              GlyphArrangement& destArrangement,
                                              int width);

    virtual void drawFileBrowserRow (Graphics& g, int width, int height,
                                     const String& filename, Image* icon,
                                     const String& fileSizeDescription,
                                     const String& fileTimeDescription,
                                     const bool isDirectory,
                                     const bool isItemSelected);

    //==============================================================================
    virtual void drawBubble (Graphics& g,
                             float tipX, float tipY,
                             float boxX, float boxY, float boxW, float boxH);

    //==============================================================================
    /** Fills the background of a popup menu component. */
    virtual void drawPopupMenuBackground (Graphics& g, int width, int height);

    /** Draws one of the items in a popup menu. */
    virtual void drawPopupMenuItem (Graphics& g,
                                    int width, int height,
                                    const bool isSeparator,
                                    const bool isActive,
                                    const bool isHighlighted,
                                    const bool isTicked,
                                    const bool hasSubMenu,
                                    const String& text,
                                    const String& shortcutKeyText,
                                    Image* image,
                                    const Colour* const textColour);

    /** Returns the size and style of font to use in popup menus. */
    virtual const Font getPopupMenuFont();

    virtual void drawPopupMenuUpDownArrow (Graphics& g,
                                           int width, int height,
                                           bool isScrollUpArrow);

    /** Finds the best size for an item in a popup menu. */
    virtual void getIdealPopupMenuItemSize (const String& text,
                                            const bool isSeparator,
                                            int standardMenuItemHeight,
                                            int& idealWidth,
                                            int& idealHeight);

    virtual int getMenuWindowFlags();

    virtual void drawMenuBarBackground (Graphics& g, int width, int height,
                                        bool isMouseOverBar,
                                        MenuBarComponent& menuBar);

    virtual int getMenuBarItemWidth (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

    virtual const Font getMenuBarFont (MenuBarComponent& menuBar, int itemIndex, const String& itemText);

    virtual void drawMenuBarItem (Graphics& g,
                                  int width, int height,
                                  int itemIndex,
                                  const String& itemText,
                                  bool isMouseOverItem,
                                  bool isMenuOpen,
                                  bool isMouseOverBar,
                                  MenuBarComponent& menuBar);

    //==============================================================================
    virtual void drawComboBox (Graphics& g, int width, int height,
                               const bool isButtonDown,
                               int buttonX, int buttonY,
                               int buttonW, int buttonH,
                               ComboBox& box);

    virtual const Font getComboBoxFont (ComboBox& box);

    //==============================================================================
    virtual void drawLinearSlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPos,
                                   float minSliderPos,
                                   float maxSliderPos,
                                   const Slider::SliderStyle style,
                                   Slider& slider);

    virtual int getSliderThumbRadius (Slider& slider);

    virtual void drawRotarySlider (Graphics& g,
                                   int x, int y,
                                   int width, int height,
                                   float sliderPosProportional,
                                   const float rotaryStartAngle,
                                   const float rotaryEndAngle,
                                   Slider& slider);

    virtual Button* createSliderButton (const bool isIncrement);
    virtual Label* createSliderTextBox (Slider& slider);

    virtual ImageEffectFilter* getSliderEffect();

    //==============================================================================
    virtual void getTooltipSize (const String& tipText, int& width, int& height);

    virtual void drawTooltip (Graphics& g, const String& text, int width, int height);

    //==============================================================================
    virtual Button* createFilenameComponentBrowseButton (const String& text);

    virtual void layoutFilenameComponent (FilenameComponent& filenameComp,
                                          ComboBox* filenameBox, Button* browseButton);

    //==============================================================================
    virtual void drawCornerResizer (Graphics& g,
                                    int w, int h,
                                    bool isMouseOver,
                                    bool isMouseDragging);

    virtual void drawResizableFrame (Graphics& g,
                                    int w, int h,
                                    const BorderSize& borders);

    //==============================================================================
    virtual void drawResizableWindowBorder (Graphics& g,
                                            int w, int h,
                                            const BorderSize& border,
                                            ResizableWindow& window);

    //==============================================================================
    virtual void drawDocumentWindowTitleBar (DocumentWindow& window,
                                             Graphics& g, int w, int h,
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
    virtual void drawStretchableLayoutResizerBar (Graphics& g,
                                                  int w, int h,
                                                  bool isVerticalBar,
                                                  bool isMouseOver,
                                                  bool isMouseDragging);

    //==============================================================================
    virtual void drawGroupComponentOutline (Graphics& g, int w, int h,
                                            const String& text,
                                            const Justification& position,
                                            GroupComponent& group);

    //==============================================================================
    virtual void createTabButtonShape (Path& p,
                                       int width, int height,
                                       int tabIndex,
                                       const String& text,
                                       Button& button,
                                       TabbedButtonBar::Orientation orientation,
                                       const bool isMouseOver,
                                       const bool isMouseDown,
                                       const bool isFrontTab);

    virtual void fillTabButtonShape (Graphics& g,
                                     const Path& path,
                                     const Colour& preferredBackgroundColour,
                                     int tabIndex,
                                     const String& text,
                                     Button& button,
                                     TabbedButtonBar::Orientation orientation,
                                     const bool isMouseOver,
                                     const bool isMouseDown,
                                     const bool isFrontTab);

    virtual void drawTabButtonText (Graphics& g,
                                    int x, int y, int w, int h,
                                    const Colour& preferredBackgroundColour,
                                    int tabIndex,
                                    const String& text,
                                    Button& button,
                                    TabbedButtonBar::Orientation orientation,
                                    const bool isMouseOver,
                                    const bool isMouseDown,
                                    const bool isFrontTab);

    virtual int getTabButtonOverlap (int tabDepth);

    virtual int getTabButtonBestWidth (int tabIndex,
                                       const String& text,
                                       int tabDepth,
                                       Button& button);

    virtual void drawTabButton (Graphics& g,
                                int w, int h,
                                const Colour& preferredColour,
                                int tabIndex,
                                const String& text,
                                Button& button,
                                TabbedButtonBar::Orientation orientation,
                                const bool isMouseOver,
                                const bool isMouseDown,
                                const bool isFrontTab);

    virtual void drawTabAreaBehindFrontButton (Graphics& g,
                                               int w, int h,
                                               TabbedButtonBar& tabBar,
                                               TabbedButtonBar::Orientation orientation);

    virtual Button* createTabBarExtrasButton();

    //==============================================================================
    virtual void drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header);

    virtual void drawTableHeaderColumn (Graphics& g, const String& columnName, int columnId,
                                        int width, int height,
                                        bool isMouseOver, bool isMouseDown,
                                        int columnFlags);

    //==============================================================================
    virtual void paintToolbarBackground (Graphics& g, int width, int height, Toolbar& toolbar);

    virtual Button* createToolbarMissingItemsButton (Toolbar& toolbar);

    virtual void paintToolbarButtonBackground (Graphics& g, int width, int height,
                                               bool isMouseOver, bool isMouseDown,
                                               ToolbarItemComponent& component);

    virtual void paintToolbarButtonLabel (Graphics& g, int x, int y, int width, int height,
                                          const String& text, ToolbarItemComponent& component);

    //==============================================================================
    virtual void drawPropertyPanelSectionHeader (Graphics& g, const String& name,
                                                 bool isOpen, int width, int height);

    virtual void drawPropertyComponentBackground (Graphics& g, int width, int height,
                                                  PropertyComponent& component);

    virtual void drawPropertyComponentLabel (Graphics& g, int width, int height,
                                             PropertyComponent& component);

    virtual const Rectangle getPropertyComponentContentPosition (PropertyComponent& component);


    //==============================================================================
    /** Utility function to draw a shiny, glassy circle (for round LED-type buttons). */
    static void drawGlassSphere (Graphics& g,
                                 const float x, const float y,
                                 const float diameter,
                                 const Colour& colour,
                                 const float outlineThickness) throw();

    static void drawGlassPointer (Graphics& g,
                                  const float x, const float y,
                                  const float diameter,
                                  const Colour& colour, const float outlineThickness,
                                  const int direction) throw();

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static void drawGlassLozenge (Graphics& g,
                                  const float x, const float y,
                                  const float width, const float height,
                                  const Colour& colour,
                                  const float outlineThickness,
                                  const float cornerSize,
                                  const bool flatOnLeft, const bool flatOnRight,
                                  const bool flatOnTop, const bool flatOnBottom) throw();

    //==============================================================================
    juce_UseDebuggingNewOperator


protected:
    // xxx the following methods are only here to cause a compiler error, because they've been
    // deprecated or their parameters have changed. Hopefully these definitions should cause an
    // error if you try to build a subclass with the old versions.

    virtual int drawTickBox (Graphics&, int, int, int, int, bool, const bool, const bool, const bool) { return 0; }

    virtual int drawProgressBar (Graphics&, int, int, int, int, float) { return 0; }

    virtual void getTabButtonBestWidth (int, const String&, int) {}

private:
    friend void JUCE_PUBLIC_FUNCTION shutdownJuce_GUI();
    static void clearDefaultLookAndFeel() throw(); // called at shutdown

    Array <int> colourIds;
    Array <Colour> colours;

    void drawShinyButtonShape (Graphics& g,
                               float x, float y, float w, float h, float maxCornerSize,
                               const Colour& baseColour,
                               const float strokeWidth,
                               const bool flatOnLeft,
                               const bool flatOnRight,
                               const bool flatOnTop,
                               const bool flatOnBottom) throw();

    LookAndFeel (const LookAndFeel&);
    const LookAndFeel& operator= (const LookAndFeel&);
};


#endif   // __JUCE_LOOKANDFEEL_JUCEHEADER__
