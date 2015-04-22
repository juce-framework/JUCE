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

#ifndef JUCE_LOOKANDFEEL_V3_H_INCLUDED
#define JUCE_LOOKANDFEEL_V3_H_INCLUDED

//==============================================================================
/**
    The latest JUCE look-and-feel style, as introduced in 2013.
    @see LookAndFeel, LookAndFeel_V1, LookAndFeel_V2
*/
class JUCE_API  LookAndFeel_V3   : public LookAndFeel_V2
{
public:
    LookAndFeel_V3();
    ~LookAndFeel_V3();

    //==============================================================================
    void drawButtonBackground (Graphics&, Button&, const Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override;

    void drawTableHeaderBackground (Graphics&, TableHeaderComponent&) override;

    void drawTreeviewPlusMinusBox (Graphics&, const Rectangle<float>& area,
                                   Colour backgroundColour, bool isOpen, bool isMouseOver) override;
    bool areLinesDrawnForTreeView (TreeView&) override;
    int getTreeViewIndentSize (TreeView&) override;

    Button* createDocumentWindowButton (int buttonType) override;

    void drawComboBox (Graphics&, int width, int height, bool isButtonDown,
                       int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box) override;

    void drawKeymapChangeButton (Graphics&, int width, int height, Button& button, const String& keyDescription) override;

    void drawPopupMenuBackground (Graphics&, int width, int height) override;
    void drawMenuBarBackground (Graphics&, int width, int height, bool, MenuBarComponent&) override;

    int getTabButtonOverlap (int tabDepth) override;
    int getTabButtonSpaceAroundImage() override;
    void drawTabButton (TabBarButton&, Graphics&, bool isMouseOver, bool isMouseDown) override;
    void drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, int w, int h) override;

    void drawTextEditorOutline (Graphics&, int width, int height, TextEditor&) override;

    void drawStretchableLayoutResizerBar (Graphics&, int w, int h, bool isVerticalBar, bool isMouseOver, bool isMouseDragging) override;

    bool areScrollbarButtonsVisible() override;

    void drawScrollbar (Graphics&, ScrollBar&, int x, int y, int width, int height, bool isScrollbarVertical,
                        int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown) override;

    void drawLinearSlider (Graphics&, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const Slider::SliderStyle, Slider&) override;

    void drawLinearSliderBackground (Graphics&, int x, int y, int width, int height,
                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                     const Slider::SliderStyle, Slider&) override;

    void drawConcertinaPanelHeader (Graphics&, const Rectangle<int>& area, bool isMouseOver, bool isMouseDown,
                                    ConcertinaPanel&, Component&) override;

    Path getTickShape (float height) override;
    Path getCrossShape (float height) override;

    static void createTabTextLayout (const TabBarButton& button, float length, float depth, Colour colour, TextLayout&);

private:
    Image backgroundTexture;
    Colour backgroundTextureBaseColour;
};


#endif   // JUCE_LOOKANDFEEL_V3_H_INCLUDED
