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

LookAndFeel_V3::LookAndFeel_V3()
{
    setColour (TreeView::selectedItemBackgroundColourId, Colour (0x301111ee));

    const Colour textButtonColour (0xffeeeeff);
    setColour (TextButton::buttonColourId, textButtonColour);
    setColour (ComboBox::buttonColourId, textButtonColour);
    setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    setColour (TabbedButtonBar::tabOutlineColourId, Colour (0xff999999));
    setColour (TabbedComponent::outlineColourId, Colour (0xff999999));
    setColour (Slider::trackColourId, Colour (0xbbffffff));
    setColour (Slider::thumbColourId, Colour (0xffddddff));

    setColour (ScrollBar::thumbColourId, Colour::greyLevel (0.8f).contrasting().withAlpha (0.13f));
}

LookAndFeel_V3::~LookAndFeel_V3() {}

bool LookAndFeel_V3::areScrollbarButtonsVisible()        { return false; }

void LookAndFeel_V3::drawStretchableLayoutResizerBar (Graphics& g, int /*w*/, int /*h*/, bool /*isVerticalBar*/,
                                                      bool isMouseOver, bool isMouseDragging)
{
    if (isMouseOver || isMouseDragging)
        g.fillAll (Colours::yellow.withAlpha (0.4f));
}

void LookAndFeel_V3::drawScrollbar (Graphics& g, ScrollBar& scrollbar, int x, int y, int width, int height,
                                    bool isScrollbarVertical, int thumbStartPosition, int thumbSize, bool isMouseOver, bool isMouseDown)
{
    Path thumbPath;

    if (thumbSize > 0)
    {
        const float thumbIndent = (isScrollbarVertical ? width : height) * 0.25f;
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

void LookAndFeel_V3::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                                bool isMouseOver, bool /*isMouseDown*/,
                                                ConcertinaPanel&, Component& panel)
{
    const Colour bkg (Colours::grey);

    g.setGradientFill (ColourGradient (Colours::white.withAlpha (isMouseOver ? 0.4f : 0.2f), 0, (float) area.getY(),
                                       Colours::darkgrey.withAlpha (0.1f), 0, (float) area.getBottom(), false));
    g.fillAll();

    g.setColour (bkg.contrasting().withAlpha (0.1f));
    g.fillRect (area.withHeight (1));
    g.fillRect (area.withTop (area.getBottom() - 1));

    g.setColour (bkg.contrasting());
    g.setFont (Font (area.getHeight() * 0.6f).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

static void drawButtonShape (Graphics& g, const Path& outline, Colour baseColour, float height)
{
    const float mainBrightness = baseColour.getBrightness();
    const float mainAlpha = baseColour.getFloatAlpha();

    g.setGradientFill (ColourGradient (baseColour.brighter (0.2f), 0.0f, 0.0f,
                                       baseColour.darker (0.25f), 0.0f, height, false));
    g.fillPath (outline);

    g.setColour (Colours::white.withAlpha (0.4f * mainAlpha * mainBrightness * mainBrightness));
    g.strokePath (outline, PathStrokeType (1.0f), AffineTransform::translation (0.0f, 1.0f)
                                                        .scaled (1.0f, (height - 1.6f) / height));

    g.setColour (Colours::black.withAlpha (0.4f * mainAlpha));
    g.strokePath (outline, PathStrokeType (1.0f));
}

void LookAndFeel_V3::drawButtonBackground (Graphics& g, Button& button, const Colour& backgroundColour,
                                           bool isMouseOverButton, bool isButtonDown)
{
    Colour baseColour (backgroundColour.withMultipliedSaturation (button.hasKeyboardFocus (true) ? 1.3f : 0.9f)
                                       .withMultipliedAlpha (button.isEnabled() ? 0.9f : 0.5f));

    if (isButtonDown || isMouseOverButton)
        baseColour = baseColour.contrasting (isButtonDown ? 0.2f : 0.1f);

    const bool flatOnLeft   = button.isConnectedOnLeft();
    const bool flatOnRight  = button.isConnectedOnRight();
    const bool flatOnTop    = button.isConnectedOnTop();
    const bool flatOnBottom = button.isConnectedOnBottom();

    const float width  = button.getWidth() - 1.0f;
    const float height = button.getHeight() - 1.0f;

    if (width > 0 && height > 0)
    {
        const float cornerSize = 4.0f;

        Path outline;
        outline.addRoundedRectangle (0.5f, 0.5f, width, height, cornerSize, cornerSize,
                                     ! (flatOnLeft  || flatOnTop),
                                     ! (flatOnRight || flatOnTop),
                                     ! (flatOnLeft  || flatOnBottom),
                                     ! (flatOnRight || flatOnBottom));

        drawButtonShape (g, outline, baseColour, height);
    }
}

void LookAndFeel_V3::drawTableHeaderBackground (Graphics& g, TableHeaderComponent& header)
{
    Rectangle<int> r (header.getLocalBounds());

    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillRect (r.removeFromBottom (1));

    g.setColour (Colours::white.withAlpha (0.6f));
    g.fillRect (r);

    g.setColour (Colours::black.withAlpha (0.5f));

    for (int i = header.getNumColumns (true); --i >= 0;)
        g.fillRect (header.getColumnPosition (i).removeFromRight (1));
}

int LookAndFeel_V3::getTabButtonOverlap (int /*tabDepth*/)            { return -1; }
int LookAndFeel_V3::getTabButtonSpaceAroundImage()                    { return 0; }

void LookAndFeel_V3::createTabTextLayout (const TabBarButton& button, float length, float depth,
                                          Colour colour, TextLayout& textLayout)
{
    Font font (depth * 0.5f);
    font.setUnderline (button.hasKeyboardFocus (false));

    AttributedString s;
    s.setJustification (Justification::centred);
    s.append (button.getButtonText().trim(), font, colour);

    textLayout.createLayout (s, length);
}

void LookAndFeel_V3::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());

    const TabbedButtonBar::Orientation o = button.getTabbedButtonBar().getOrientation();

    const Colour bkg (button.getTabBackgroundColour());

    if (button.getToggleState())
    {
        g.setColour (bkg);
        g.fillRect (activeArea);
    }
    else
    {
        Point<int> p1, p2;

        switch (o)
        {
            case TabbedButtonBar::TabsAtBottom:   p1 = activeArea.getBottomLeft(); p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtTop:      p1 = activeArea.getTopLeft();    p2 = activeArea.getBottomLeft(); break;
            case TabbedButtonBar::TabsAtRight:    p1 = activeArea.getTopRight();   p2 = activeArea.getTopLeft();    break;
            case TabbedButtonBar::TabsAtLeft:     p1 = activeArea.getTopLeft();    p2 = activeArea.getTopRight();   break;
            default:                              jassertfalse; break;
        }

        g.setGradientFill (ColourGradient (bkg.brighter (0.2f), (float) p1.x, (float) p1.y,
                                           bkg.darker (0.1f),   (float) p2.x, (float) p2.y, false));
        g.fillRect (activeArea);
    }

    g.setColour (button.findColour (TabbedButtonBar::tabOutlineColourId));

    Rectangle<int> r (activeArea);

    if (o != TabbedButtonBar::TabsAtBottom)   g.fillRect (r.removeFromTop (1));
    if (o != TabbedButtonBar::TabsAtTop)      g.fillRect (r.removeFromBottom (1));
    if (o != TabbedButtonBar::TabsAtRight)    g.fillRect (r.removeFromLeft (1));
    if (o != TabbedButtonBar::TabsAtLeft)     g.fillRect (r.removeFromRight (1));

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    const Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    const Rectangle<float> area (button.getTextArea().toFloat());

    float length = area.getWidth();
    float depth  = area.getHeight();

    if (button.getTabbedButtonBar().isVertical())
        std::swap (length, depth);

    TextLayout textLayout;
    createTabTextLayout (button, length, depth, col, textLayout);

    AffineTransform t;

    switch (o)
    {
        case TabbedButtonBar::TabsAtLeft:   t = t.rotated (float_Pi * -0.5f).translated (area.getX(), area.getBottom()); break;
        case TabbedButtonBar::TabsAtRight:  t = t.rotated (float_Pi *  0.5f).translated (area.getRight(), area.getY()); break;
        case TabbedButtonBar::TabsAtTop:
        case TabbedButtonBar::TabsAtBottom: t = t.translated (area.getX(), area.getY()); break;
        default:                            jassertfalse; break;
    }

    g.addTransform (t);
    textLayout.draw (g, Rectangle<float> (length, depth));
}

void LookAndFeel_V3::drawTabAreaBehindFrontButton (TabbedButtonBar& bar, Graphics& g, const int w, const int h)
{
    const float shadowSize = 0.15f;

    Rectangle<int> shadowRect, line;
    ColourGradient gradient (Colours::black.withAlpha (bar.isEnabled() ? 0.08f : 0.04f), 0, 0,
                             Colours::transparentBlack, 0, 0, false);

    switch (bar.getOrientation())
    {
        case TabbedButtonBar::TabsAtLeft:
            gradient.point1.x = (float) w;
            gradient.point2.x = w * (1.0f - shadowSize);
            shadowRect.setBounds ((int) gradient.point2.x, 0, w - (int) gradient.point2.x, h);
            line.setBounds (w - 1, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtRight:
            gradient.point2.x = w * shadowSize;
            shadowRect.setBounds (0, 0, (int) gradient.point2.x, h);
            line.setBounds (0, 0, 1, h);
            break;

        case TabbedButtonBar::TabsAtTop:
            gradient.point1.y = (float) h;
            gradient.point2.y = h * (1.0f - shadowSize);
            shadowRect.setBounds (0, (int) gradient.point2.y, w, h - (int) gradient.point2.y);
            line.setBounds (0, h - 1, w, 1);
            break;

        case TabbedButtonBar::TabsAtBottom:
            gradient.point2.y = h * shadowSize;
            shadowRect.setBounds (0, 0, w, (int) gradient.point2.y);
            line.setBounds (0, 0, w, 1);
            break;

        default: break;
    }

    g.setGradientFill (gradient);
    g.fillRect (shadowRect.expanded (2, 2));

    g.setColour (bar.findColour (TabbedButtonBar::tabOutlineColourId));
    g.fillRect (line);
}

void LookAndFeel_V3::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            g.setColour (textEditor.findColour (TextEditor::focusedOutlineColourId));
            g.drawRect (0, 0, width, height, 2);
        }
        else
        {
            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawRect (0, 0, width, height);
        }
    }
}

void LookAndFeel_V3::drawTreeviewPlusMinusBox (Graphics& g, const Rectangle<float>& area,
                                               Colour backgroundColour, bool isOpen, bool isMouseOver)
{
    Path p;
    p.addTriangle (0.0f, 0.0f, 1.0f, isOpen ? 0.0f : 0.5f, isOpen ? 0.5f : 0.0f, 1.0f);

    g.setColour (backgroundColour.contrasting().withAlpha (isMouseOver ? 0.5f : 0.3f));
    g.fillPath (p, p.getTransformToScaleToFit (area.reduced (2, area.getHeight() / 4), true));
}

bool LookAndFeel_V3::areLinesDrawnForTreeView (TreeView&)
{
    return false;
}

int LookAndFeel_V3::getTreeViewIndentSize (TreeView&)
{
    return 20;
}

void LookAndFeel_V3::drawComboBox (Graphics& g, int width, int height, const bool /*isButtonDown*/,
                                   int buttonX, int buttonY, int buttonW, int buttonH, ComboBox& box)
{
    g.fillAll (box.findColour (ComboBox::backgroundColourId));

    const Colour buttonColour (box.findColour (ComboBox::buttonColourId));

    if (box.isEnabled() && box.hasKeyboardFocus (false))
    {
        g.setColour (buttonColour);
        g.drawRect (0, 0, width, height, 2);
    }
    else
    {
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRect (0, 0, width, height);
    }

    const float arrowX = 0.3f;
    const float arrowH = 0.2f;

    Path p;
    p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
                   buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                   buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

    p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
                   buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                   buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

    g.setColour (box.findColour (ComboBox::arrowColourId).withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.3f));
    g.fillPath (p);
}

void LookAndFeel_V3::drawLinearSlider (Graphics& g, int x, int y, int width, int height,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       const Slider::SliderStyle style, Slider& slider)
{
    g.fillAll (slider.findColour (Slider::backgroundColourId));

    if (style == Slider::LinearBar || style == Slider::LinearBarVertical)
    {
        Path p;

        if (style == Slider::LinearBarVertical)
            p.addRectangle ((float) x, sliderPos, (float) width, (height - sliderPos));
        else
            p.addRectangle ((float) x, (float) y, (sliderPos - x), (float) height);

        drawButtonShape (g, p, slider.findColour (Slider::thumbColourId)
                                .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f)
                                .withMultipliedAlpha (0.8f),
                         (float) height);
    }
    else
    {
        drawLinearSliderBackground (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
        drawLinearSliderThumb (g, x, y, width, height, sliderPos, minSliderPos, maxSliderPos, style, slider);
    }
}

void LookAndFeel_V3::drawLinearSliderBackground (Graphics& g, int x, int y, int width, int height,
                                                 float /*sliderPos*/,
                                                 float /*minSliderPos*/,
                                                 float /*maxSliderPos*/,
                                                 const Slider::SliderStyle /*style*/, Slider& slider)
{
    const float sliderRadius = (float) (getSliderThumbRadius (slider) - 2);

    const Colour trackColour (slider.findColour (Slider::trackColourId));
    const Colour gradCol1 (trackColour.overlaidWith (Colour (slider.isEnabled() ? 0x13000000 : 0x09000000)));
    const Colour gradCol2 (trackColour.overlaidWith (Colour (0x06000000)));
    Path indent;

    if (slider.isHorizontal())
    {
        const float iy = y + height * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColourGradient (gradCol1, 0.0f, iy,
                                           gradCol2, 0.0f, iy + sliderRadius, false));

        indent.addRoundedRectangle (x - sliderRadius * 0.5f, iy, width + sliderRadius, sliderRadius, 5.0f);
    }
    else
    {
        const float ix = x + width * 0.5f - sliderRadius * 0.5f;

        g.setGradientFill (ColourGradient (gradCol1, ix, 0.0f,
                                           gradCol2, ix + sliderRadius, 0.0f, false));

        indent.addRoundedRectangle (ix, y - sliderRadius * 0.5f, sliderRadius, height + sliderRadius, 5.0f);
    }

    g.fillPath (indent);

    g.setColour (trackColour.contrasting (0.5f));
    g.strokePath (indent, PathStrokeType (0.5f));
}

void LookAndFeel_V3::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    g.fillAll (findColour (PopupMenu::backgroundColourId));
    (void) width; (void) height;

   #if ! JUCE_MAC
    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
   #endif
}

void LookAndFeel_V3::drawMenuBarBackground (Graphics& g, int width, int height,
                                            bool, MenuBarComponent& menuBar)
{
    const Colour colour (menuBar.findColour (PopupMenu::backgroundColourId));

    Rectangle<int> r (width, height);

    g.setColour (colour.contrasting (0.15f));
    g.fillRect (r.removeFromTop (1));
    g.fillRect (r.removeFromBottom (1));

    g.setGradientFill (ColourGradient (colour, 0, 0, colour.darker (0.08f), 0, (float) height, false));
    g.fillRect (r);
}

void LookAndFeel_V3::drawKeymapChangeButton (Graphics& g, int width, int height,
                                             Button& button, const String& keyDescription)
{
    const Colour textColour (button.findColour (0x100ad01 /*KeyMappingEditorComponent::textColourId*/, true));

    if (keyDescription.isNotEmpty())
    {
        if (button.isEnabled())
        {
            g.setColour (textColour.withAlpha (button.isDown() ? 0.4f : (button.isOver() ? 0.2f : 0.1f)));
            g.fillRoundedRectangle (button.getLocalBounds().toFloat(), 4.0f);
            g.drawRoundedRectangle (button.getLocalBounds().toFloat(), 4.0f, 1.0f);
        }

        g.setColour (textColour);
        g.setFont (height * 0.6f);
        g.drawFittedText (keyDescription, 4, 0, width - 8, height, Justification::centred, 1);
    }
    else
    {
        const float thickness = 7.0f;
        const float indent = 22.0f;

        Path p;
        p.addEllipse (0.0f, 0.0f, 100.0f, 100.0f);
        p.addRectangle (indent, 50.0f - thickness, 100.0f - indent * 2.0f, thickness * 2.0f);
        p.addRectangle (50.0f - thickness, indent, thickness * 2.0f, 50.0f - indent - thickness);
        p.addRectangle (50.0f - thickness, 50.0f + thickness, thickness * 2.0f, 50.0f - indent - thickness);
        p.setUsingNonZeroWinding (false);

        g.setColour (textColour.darker(0.1f).withAlpha (button.isDown() ? 0.7f : (button.isOver() ? 0.5f : 0.3f)));
        g.fillPath (p, p.getTransformToScaleToFit (2.0f, 2.0f, width - 4.0f, height - 4.0f, true));
    }

    if (button.hasKeyboardFocus (false))
    {
        g.setColour (textColour.withAlpha (0.4f));
        g.drawRect (0, 0, width, height);
    }
}
