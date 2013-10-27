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

#ifndef JUCE_LOOKANDFEEL_V1_H_INCLUDED
#define JUCE_LOOKANDFEEL_V1_H_INCLUDED

//==============================================================================
/**
    The original JUCE look-and-feel, as used back from 2002 to about 2007ish.
    @see LookAndFeel, LookAndFeel_V2, LookAndFeel_V3
*/
class JUCE_API  LookAndFeel_V1    : public LookAndFeel_V2
{
public:
    LookAndFeel_V1();
    ~LookAndFeel_V1();

    //==============================================================================
    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    void drawToggleButton (Graphics&, ToggleButton&, bool isMouseOverButton, bool isButtonDown) override;

    void drawTickBox (Graphics&, Component&, float x, float y, float w, float h,
                      bool ticked, bool isEnabled, bool isMouseOverButton, bool isButtonDown) override;

    void drawProgressBar (Graphics&, ProgressBar&, int width, int height,
                          double progress, const String& textToShow) override;

    //==============================================================================
    void drawScrollbarButton (Graphics&, ScrollBar&, int width, int height,
                              int buttonDirection, bool isScrollbarVertical,
                              bool isMouseOverButton, bool isButtonDown) override;

    void drawScrollbar (Graphics&, ScrollBar&, int x, int y, int width, int height,
                        bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                        bool isMouseOver, bool isMouseDown) override;

    ImageEffectFilter* getScrollbarEffect() override;

    //==============================================================================
    void drawTextEditorOutline (Graphics&, int width, int height, TextEditor&) override;

    //==============================================================================
    void drawPopupMenuBackground (Graphics&, int width, int height) override;
    void drawMenuBarBackground (Graphics&, int width, int height, bool isMouseOverBar, MenuBarComponent&) override;

    //==============================================================================
    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, ComboBox&) override;

    Font getComboBoxFont (ComboBox&) override;

    //==============================================================================
    void drawLinearSlider (Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle, Slider&) override;

    int getSliderThumbRadius (Slider&) override;
    Button* createSliderButton (bool isIncrement) override;
    ImageEffectFilter* getSliderEffect() override;

    //==============================================================================
    void drawCornerResizer (Graphics&, int w, int h, bool isMouseOver, bool isMouseDragging) override;

    Button* createDocumentWindowButton (int buttonType) override;

    void positionDocumentWindowButtons (DocumentWindow&,
                                        int titleBarX, int titleBarY, int titleBarW, int titleBarH,
                                        Button* minimiseButton, Button* maximiseButton, Button* closeButton,
                                        bool positionTitleBarButtonsOnLeft) override;

private:
    DropShadowEffect scrollbarShadow;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeel_V1)
};


#endif   // JUCE_LOOKANDFEEL_H_INCLUDED
