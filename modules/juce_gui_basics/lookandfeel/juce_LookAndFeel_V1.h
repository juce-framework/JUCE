/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

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
    The original JUCE look-and-feel, as used back from 2002 to about 2007ish.
    @see LookAndFeel, LookAndFeel_V2, LookAndFeel_V3

    @tags{GUI}
*/
class JUCE_API  LookAndFeel_V1    : public LookAndFeel_V2
{
public:
    LookAndFeel_V1();
    ~LookAndFeel_V1() override;

    //==============================================================================
    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawToggleButton (Graphics&, ToggleButton&,
                           bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawTickBox (Graphics&, Component&, float x, float y, float w, float h,
                      bool ticked, bool isEnabled,
                      bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

    void drawProgressBar (Graphics&, ProgressBar&, int width, int height,
                          double progress, const String& textToShow) override;

    //==============================================================================
    void drawScrollbarButton (Graphics&, ScrollBar&, int width, int height,
                              int buttonDirection, bool isScrollbarVertical,
                              bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;

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
    Button* createSliderButton (Slider&, bool isIncrement) override;
    ImageEffectFilter* getSliderEffect (Slider&) override;

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

} // namespace juce
