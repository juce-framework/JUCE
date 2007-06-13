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

#ifndef __JUCE_SHINYLOOKANDFEEL_JUCEHEADER__
#define __JUCE_SHINYLOOKANDFEEL_JUCEHEADER__

#include "juce_LookAndFeel.h"


//==============================================================================
/**
    A new, slightly plasticky looking look-and-feel.

    To make this the default look for your app, just set it as the default in
    your initialisation code.

    e.g. @code
    void initialise (const String& commandLine)
    {
        static ShinyLookAndFeel shinyLook;
        LookAndFeel::setDefaultLookAndFeel (&shinyLook);
    }
    @endcode
*/
class JUCE_API  ShinyLookAndFeel  : public LookAndFeel
{
public:
    //==============================================================================
    /** Creates a ShinyLookAndFeel look and feel object. */
    ShinyLookAndFeel();

    /** Destructor. */
    virtual ~ShinyLookAndFeel();

    //==============================================================================
    void drawTextEditorOutline (Graphics& g,
                                int width, int height,
                                TextEditor& textEditor);

    //==============================================================================
    void drawComboBox (Graphics& g, int width, int height,
                       const bool isButtonDown,
                       int buttonX, int buttonY,
                       int buttonW, int buttonH,
                       ComboBox& box);

    const Font getComboBoxFont (ComboBox& box);

    //==============================================================================
    void drawScrollbarButton (Graphics& g,
                              ScrollBar& scrollbar,
                              int width, int height,
                              int buttonDirection,
                              bool isScrollbarVertical,
                              bool isMouseOverButton,
                              bool isButtonDown);

    void drawScrollbar (Graphics& g,
                        ScrollBar& scrollbar,
                        int x, int y,
                        int width, int height,
                        bool isScrollbarVertical,
                        int thumbStartPosition,
                        int thumbSize,
                        bool isMouseOver,
                        bool isMouseDown);

    ImageEffectFilter* getScrollbarEffect();

    //==============================================================================
    void drawButtonBackground (Graphics& g,
                               Button& button,
                               const Colour& backgroundColour,
                               bool isMouseOverButton,
                               bool isButtonDown);

    void drawTickBox (Graphics& g,
                      Component& component,
                      int x, int y, int w, int h,
                      const bool ticked,
                      const bool isEnabled,
                      const bool isMouseOverButton,
                      const bool isButtonDown);

    //==============================================================================
    void drawLinearSlider (Graphics& g,
                           int x, int y,
                           int width, int height,
                           float sliderPos,
                           float minSliderPos,
                           float maxSliderPos,
                           const Slider::SliderStyle style,
                           Slider& slider);

    int getSliderThumbRadius (Slider& slider);

    Button* createSliderButton (const bool isIncrement);

    ImageEffectFilter* getSliderEffect();

    //==============================================================================
    void drawPopupMenuBackground (Graphics& g, int width, int height);

    void drawMenuBarBackground (Graphics& g, int width, int height,
                                bool isMouseOverBar, MenuBarComponent& menuBar);

    //==============================================================================
    void positionDocumentWindowButtons (DocumentWindow& window,
                                        int titleBarX, int titleBarY,
                                        int titleBarW, int titleBarH,
                                        Button* minimiseButton,
                                        Button* maximiseButton,
                                        Button* closeButton,
                                        bool positionTitleBarButtonsOnLeft);

    Button* createDocumentWindowButton (int buttonType);

    //==============================================================================
    void drawCornerResizer (Graphics& g,
                            int w, int h,
                            bool isMouseOver,
                            bool isMouseDragging);

    //==============================================================================
    void drawProgressBar (Graphics& g,
                          ProgressBar& progressBar,
                          int x, int y, int w, int h,
                          float progress);

    //==============================================================================
    /** Utility function to draw a shiny, glassy circle (for round LED-type buttons). */
    static void drawGlassSphere (Graphics& g, float x, float y, float diameter,
                                 const Colour& colour, const float outlineThickness);

    static void drawGlassPointer (Graphics& g, float x, float y, float diameter,
                                  const Colour& colour, const float outlineThickness,
                                  const int direction);

    /** Utility function to draw a shiny, glassy oblong (for text buttons). */
    static void drawGlassLozenge (Graphics& g, float x, float y, float width, float height,
                                  const Colour& colour, const float outlineThickness, const float cornerSize,
                                  const bool flatOnLeft, const bool flatOnRight, const bool flatOnTop, const bool flatOnBottom);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    void drawShinyButtonShape (Graphics& g,
                               float x, float y, float w, float h, float maxCornerSize,
                               const Colour& baseColour,
                               const float strokeWidth,
                               const bool flatOnLeft,
                               const bool flatOnRight,
                               const bool flatOnTop,
                               const bool flatOnBottom);
};


#endif   // __JUCE_SHINYLOOKANDFEEL_JUCEHEADER__
