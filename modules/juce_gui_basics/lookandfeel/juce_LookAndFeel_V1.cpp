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

LookAndFeel_V1::LookAndFeel_V1()
{
    setColor (TextButton::buttonColorId,          Color (0xffbbbbff));
    setColor (ListBox::outlineColorId,            findColor (ComboBox::outlineColorId));
    setColor (ScrollBar::thumbColorId,            Color (0xffbbbbdd));
    setColor (ScrollBar::backgroundColorId,       Colors::transparentBlack);
    setColor (Slider::thumbColorId,               Colors::white);
    setColor (Slider::trackColorId,               Color (0x7f000000));
    setColor (Slider::textBoxOutlineColorId,      Colors::gray);
    setColor (ProgressBar::backgroundColorId,     Colors::white.withAlpha (0.6f));
    setColor (ProgressBar::foregroundColorId,     Colors::green.withAlpha (0.7f));
    setColor (PopupMenu::backgroundColorId,             Color (0xffeef5f8));
    setColor (PopupMenu::highlightedBackgroundColorId,  Color (0xbfa4c2ce));
    setColor (PopupMenu::highlightedTextColorId,        Colors::black);
    setColor (TextEditor::focusedOutlineColorId,  findColor (TextButton::buttonColorId));

    scrollbarShadow.setShadowProperties (DropShadow (Colors::black.withAlpha (0.5f), 2, Point<int>()));
}

LookAndFeel_V1::~LookAndFeel_V1()
{
}

//==============================================================================
void LookAndFeel_V1::drawButtonBackground (Graphics& g, Button& button, const Color& backgroundColor,
                                           bool isMouseOverButton, bool isButtonDown)
{
    const int width = button.getWidth();
    const int height = button.getHeight();

    const float indent = 2.0f;
    const int cornerSize = jmin (roundToInt (width * 0.4f),
                                 roundToInt (height * 0.4f));

    Path p;
    p.addRoundedRectangle (indent, indent,
                           width - indent * 2.0f,
                           height - indent * 2.0f,
                           (float) cornerSize);

    Color bc (backgroundColor.withMultipliedSaturation (0.3f));

    if (isMouseOverButton)
    {
        if (isButtonDown)
            bc = bc.brighter();
        else if (bc.getBrightness() > 0.5f)
            bc = bc.darker (0.1f);
        else
            bc = bc.brighter (0.1f);
    }

    g.setColor (bc);
    g.fillPath (p);

    g.setColor (bc.contrasting().withAlpha ((isMouseOverButton) ? 0.6f : 0.4f));
    g.strokePath (p, PathStrokeType ((isMouseOverButton) ? 2.0f : 1.4f));
}

void LookAndFeel_V1::drawTickBox (Graphics& g, Component& /*component*/,
                                  float x, float y, float w, float h,
                                  const bool ticked,
                                  const bool isEnabled,
                                  const bool /*isMouseOverButton*/,
                                  const bool isButtonDown)
{
    Path box;
    box.addRoundedRectangle (0.0f, 2.0f, 6.0f, 6.0f, 1.0f);

    g.setColor (isEnabled ? Colors::blue.withAlpha (isButtonDown ? 0.3f : 0.1f)
                           : Colors::lightgray.withAlpha (0.1f));

    AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f).translated (x, y));

    g.fillPath (box, trans);

    g.setColor (Colors::black.withAlpha (0.6f));
    g.strokePath (box, PathStrokeType (0.9f), trans);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColor (isEnabled ? Colors::black : Colors::gray);
        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

void LookAndFeel_V1::drawToggleButton (Graphics& g, ToggleButton& button, bool isMouseOverButton, bool isButtonDown)
{
    if (button.hasKeyboardFocus (true))
    {
        g.setColor (button.findColor (TextEditor::focusedOutlineColorId));
        g.drawRect (0, 0, button.getWidth(), button.getHeight());
    }

    const int tickWidth = jmin (20, button.getHeight() - 4);

    drawTickBox (g, button, 4.0f, (button.getHeight() - tickWidth) * 0.5f,
                 (float) tickWidth, (float) tickWidth,
                 button.getToggleState(),
                 button.isEnabled(),
                 isMouseOverButton,
                 isButtonDown);

    g.setColor (button.findColor (ToggleButton::textColorId));
    g.setFont (jmin (15.0f, button.getHeight() * 0.6f));

    if (! button.isEnabled())
        g.setOpacity (0.5f);

    const int textX = tickWidth + 5;

    g.drawFittedText (button.getButtonText(),
                      textX, 4,
                      button.getWidth() - textX - 2, button.getHeight() - 8,
                      Justification::centeredLeft, 10);
}

void LookAndFeel_V1::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                      int width, int height,
                                      double progress, const String& textToShow)
{
    if (progress < 0 || progress >= 1.0)
    {
        LookAndFeel_V2::drawProgressBar (g, progressBar, width, height, progress, textToShow);
    }
    else
    {
        const Color background (progressBar.findColor (ProgressBar::backgroundColorId));
        const Color foreground (progressBar.findColor (ProgressBar::foregroundColorId));

        g.fillAll (background);
        g.setColor (foreground);

        g.fillRect (1, 1,
                    jlimit (0, width - 2, roundToInt (progress * (width - 2))),
                    height - 2);

        if (textToShow.isNotEmpty())
        {
            g.setColor (Color::contrasting (background, foreground));
            g.setFont (height * 0.6f);

            g.drawText (textToShow, 0, 0, width, height, Justification::centered, false);
        }
    }
}

void LookAndFeel_V1::drawScrollbarButton (Graphics& g, ScrollBar& bar,
                                          int width, int height, int buttonDirection,
                                          bool isScrollbarVertical,
                                          bool isMouseOverButton,
                                          bool isButtonDown)
{
    if (isScrollbarVertical)
        width -= 2;
    else
        height -= 2;

    Path p;

    if (buttonDirection == 0)
        p.addTriangle (width * 0.5f, height * 0.2f,
                       width * 0.1f, height * 0.7f,
                       width * 0.9f, height * 0.7f);
    else if (buttonDirection == 1)
        p.addTriangle (width * 0.8f, height * 0.5f,
                       width * 0.3f, height * 0.1f,
                       width * 0.3f, height * 0.9f);
    else if (buttonDirection == 2)
        p.addTriangle (width * 0.5f, height * 0.8f,
                       width * 0.1f, height * 0.3f,
                       width * 0.9f, height * 0.3f);
    else if (buttonDirection == 3)
        p.addTriangle (width * 0.2f, height * 0.5f,
                       width * 0.7f, height * 0.1f,
                       width * 0.7f, height * 0.9f);

    if (isButtonDown)
        g.setColor (Colors::white);
    else if (isMouseOverButton)
        g.setColor (Colors::white.withAlpha (0.7f));
    else
        g.setColor (bar.findColor (ScrollBar::thumbColorId).withAlpha (0.5f));

    g.fillPath (p);

    g.setColor (Colors::black.withAlpha (0.5f));
    g.strokePath (p, PathStrokeType (0.5f));
}

void LookAndFeel_V1::drawScrollbar (Graphics& g, ScrollBar& bar,
                                    int x, int y, int width, int height,
                                    bool isScrollbarVertical, int thumbStartPosition, int thumbSize,
                                    bool isMouseOver, bool isMouseDown)
{
    g.fillAll (bar.findColor (ScrollBar::backgroundColorId));

    g.setColor (bar.findColor (ScrollBar::thumbColorId)
                    .withAlpha ((isMouseOver || isMouseDown) ? 0.4f : 0.15f));

    if (thumbSize > 0.0f)
    {
        Rectangle<int> thumb;

        if (isScrollbarVertical)
        {
            width -= 2;
            g.fillRect (x + roundToInt (width * 0.35f), y,
                        roundToInt (width * 0.3f), height);

            thumb.setBounds (x + 1, thumbStartPosition,
                             width - 2, thumbSize);
        }
        else
        {
            height -= 2;
            g.fillRect (x, y + roundToInt (height * 0.35f),
                        width, roundToInt (height * 0.3f));

            thumb.setBounds (thumbStartPosition, y + 1,
                             thumbSize, height - 2);
        }

        g.setColor (bar.findColor (ScrollBar::thumbColorId)
                        .withAlpha ((isMouseOver || isMouseDown) ? 0.95f : 0.7f));

        g.fillRect (thumb);

        g.setColor (Colors::black.withAlpha ((isMouseOver || isMouseDown) ? 0.4f : 0.25f));
        g.drawRect (thumb.getX(), thumb.getY(), thumb.getWidth(), thumb.getHeight());

        if (thumbSize > 16)
        {
            for (int i = 3; --i >= 0;)
            {
                const float linePos = thumbStartPosition + thumbSize / 2 + (i - 1) * 4.0f;
                g.setColor (Colors::black.withAlpha (0.15f));

                if (isScrollbarVertical)
                {
                    g.drawLine (x + width * 0.2f, linePos, width * 0.8f, linePos);
                    g.setColor (Colors::white.withAlpha (0.15f));
                    g.drawLine (width * 0.2f, linePos - 1, width * 0.8f, linePos - 1);
                }
                else
                {
                    g.drawLine (linePos, height * 0.2f, linePos, height * 0.8f);
                    g.setColor (Colors::white.withAlpha (0.15f));
                    g.drawLine (linePos - 1, height * 0.2f, linePos - 1, height * 0.8f);
                }
            }
        }
    }
}

ImageEffectFilter* LookAndFeel_V1::getScrollbarEffect()
{
    return &scrollbarShadow;
}


//==============================================================================
void LookAndFeel_V1::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    g.fillAll (findColor (PopupMenu::backgroundColorId));

    g.setColor (Colors::black.withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
}

void LookAndFeel_V1::drawMenuBarBackground (Graphics& g, int /*width*/, int /*height*/, bool, MenuBarComponent& menuBar)
{
    g.fillAll (menuBar.findColor (PopupMenu::backgroundColorId));
}


//==============================================================================
void LookAndFeel_V1::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        g.setColor (textEditor.findColor (TextEditor::outlineColorId));
        g.drawRect (0, 0, width, height);
    }
}

//==============================================================================
void LookAndFeel_V1::drawComboBox (Graphics& g, int width, int height,
                                   const bool isButtonDown,
                                   int buttonX, int buttonY, int buttonW, int buttonH,
                                   ComboBox& box)
{
    g.fillAll (box.findColor (ComboBox::backgroundColorId));

    g.setColor (box.findColor ((isButtonDown) ? ComboBox::buttonColorId
                                                : ComboBox::backgroundColorId));
    g.fillRect (buttonX, buttonY, buttonW, buttonH);

    g.setColor (box.findColor (ComboBox::outlineColorId));
    g.drawRect (0, 0, width, height);

    const float arrowX = 0.2f;
    const float arrowH = 0.3f;

    if (box.isEnabled())
    {
        Path p;
        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

        g.setColor (box.findColor ((isButtonDown) ? ComboBox::backgroundColorId
                                                    : ComboBox::buttonColorId));
        g.fillPath (p);
    }
}

Font LookAndFeel_V1::getComboBoxFont (ComboBox& box)
{
    Font f (jmin (15.0f, box.getHeight() * 0.85f));
    f.setHorizontalScale (0.9f);
    return f;
}

//==============================================================================
static void drawTriangle (Graphics& g, float x1, float y1, float x2, float y2, float x3, float y3, Color fill, Color outline)
{
    Path p;
    p.addTriangle (x1, y1, x2, y2, x3, y3);
    g.setColor (fill);
    g.fillPath (p);

    g.setColor (outline);
    g.strokePath (p, PathStrokeType (0.3f));
}

void LookAndFeel_V1::drawLinearSlider (Graphics& g,
                                       int x, int y, int w, int h,
                                       float sliderPos, float minSliderPos, float maxSliderPos,
                                       const Slider::SliderStyle style,
                                       Slider& slider)
{
    g.fillAll (slider.findColor (Slider::backgroundColorId));

    if (style == Slider::LinearBar)
    {
        g.setColor (slider.findColor (Slider::thumbColorId));
        g.fillRect (x, y, (int) sliderPos - x, h);

        g.setColor (slider.findColor (Slider::textBoxTextColorId).withMultipliedAlpha (0.5f));
        g.drawRect (x, y, (int) sliderPos - x, h);
    }
    else
    {
        g.setColor (slider.findColor (Slider::trackColorId)
                           .withMultipliedAlpha (slider.isEnabled() ? 1.0f : 0.3f));

        if (slider.isHorizontal())
        {
            g.fillRect (x, y + roundToInt (h * 0.6f),
                        w, roundToInt (h * 0.2f));
        }
        else
        {
            g.fillRect (x + roundToInt (w * 0.5f - jmin (3.0f, w * 0.1f)), y,
                        jmin (4, roundToInt (w * 0.2f)), h);
        }

        float alpha = 0.35f;

        if (slider.isEnabled())
            alpha = slider.isMouseOverOrDragging() ? 1.0f : 0.7f;

        const Color fill (slider.findColor (Slider::thumbColorId).withAlpha (alpha));
        const Color outline (Colors::black.withAlpha (slider.isEnabled() ? 0.7f : 0.35f));

        if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
        {
            drawTriangle (g, x + w * 0.5f + jmin (4.0f, w * 0.3f), minSliderPos,
                          x + w * 0.5f - jmin (8.0f, w * 0.4f), minSliderPos - 7.0f,
                          x + w * 0.5f - jmin (8.0f, w * 0.4f), minSliderPos,
                          fill, outline);

            drawTriangle (g, x + w * 0.5f + jmin (4.0f, w * 0.3f), maxSliderPos,
                          x + w * 0.5f - jmin (8.0f, w * 0.4f), maxSliderPos,
                          x + w * 0.5f - jmin (8.0f, w * 0.4f), maxSliderPos + 7.0f,
                          fill, outline);
        }
        else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
        {
            drawTriangle (g, minSliderPos, y + h * 0.6f - jmin (4.0f, h * 0.3f),
                          minSliderPos - 7.0f, y + h * 0.9f ,
                          minSliderPos, y + h * 0.9f,
                          fill, outline);

            drawTriangle (g, maxSliderPos, y + h * 0.6f - jmin (4.0f, h * 0.3f),
                          maxSliderPos, y + h * 0.9f,
                          maxSliderPos + 7.0f, y + h * 0.9f,
                          fill, outline);
        }

        if (style == Slider::LinearHorizontal || style == Slider::ThreeValueHorizontal)
        {
            drawTriangle (g, sliderPos, y + h * 0.9f,
                          sliderPos - 7.0f, y + h * 0.2f,
                          sliderPos + 7.0f, y + h * 0.2f,
                          fill, outline);
        }
        else if (style == Slider::LinearVertical || style == Slider::ThreeValueVertical)
        {
            drawTriangle (g, x + w * 0.5f - jmin (4.0f, w * 0.3f), sliderPos,
                          x + w * 0.5f + jmin (8.0f, w * 0.4f), sliderPos - 7.0f,
                          x + w * 0.5f + jmin (8.0f, w * 0.4f), sliderPos + 7.0f,
                          fill, outline);
        }
    }
}

Button* LookAndFeel_V1::createSliderButton (Slider&, const bool isIncrement)
{
    if (isIncrement)
        return new ArrowButton ("u", 0.75f, Colors::white.withAlpha (0.8f));

    return new ArrowButton ("d", 0.25f, Colors::white.withAlpha (0.8f));
}

ImageEffectFilter* LookAndFeel_V1::getSliderEffect (Slider&)
{
    return &scrollbarShadow;
}

int LookAndFeel_V1::getSliderThumbRadius (Slider&)
{
    return 8;
}

//==============================================================================
void LookAndFeel_V1::drawCornerResizer (Graphics& g, int w, int h, bool isMouseOver, bool isMouseDragging)
{
    g.setColor ((isMouseOver || isMouseDragging) ? Colors::lightgray
                                                  : Colors::darkgray);

    const float lineThickness = jmin (w, h) * 0.1f;

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.drawLine (w * i,
                    h + 1.0f,
                    w + 1.0f,
                    h * i,
                    lineThickness);
    }
}

//==============================================================================
Button* LookAndFeel_V1::createDocumentWindowButton (int buttonType)
{
    Path shape;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.0f, 1.0f, 1.0f), 0.35f);
        shape.addLineSegment (Line<float> (1.0f, 0.0f, 0.0f, 1.0f), 0.35f);

        ShapeButton* const b = new ShapeButton ("close",
                                                Color (0x7fff3333),
                                                Color (0xd7ff3333),
                                                Color (0xf7ff3333));

        b->setShape (shape, true, true, true);
        return b;
    }
    else if (buttonType == DocumentWindow::minimizeButton)
    {
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), 0.25f);

        DrawableButton* b = new DrawableButton ("minimize", DrawableButton::ImageFitted);
        DrawablePath dp;
        dp.setPath (shape);
        dp.setFill (Colors::black.withAlpha (0.3f));
        b->setImages (&dp);
        return b;
    }
    else if (buttonType == DocumentWindow::maximizeButton)
    {
        shape.addLineSegment (Line<float> (0.5f, 0.0f, 0.5f, 1.0f), 0.25f);
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), 0.25f);

        DrawableButton* b = new DrawableButton ("maximize", DrawableButton::ImageFitted);
        DrawablePath dp;
        dp.setPath (shape);
        dp.setFill (Colors::black.withAlpha (0.3f));
        b->setImages (&dp);
        return b;
    }

    jassertfalse;
    return nullptr;
}

void LookAndFeel_V1::positionDocumentWindowButtons (DocumentWindow&,
                                                    int titleBarX, int titleBarY, int titleBarW, int titleBarH,
                                                    Button* minimizeButton,
                                                    Button* maximizeButton,
                                                    Button* closeButton,
                                                    bool positionTitleBarButtonsOnLeft)
{
    titleBarY += titleBarH / 8;
    titleBarH -= titleBarH / 4;

    const int buttonW = titleBarH;

    int x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - 4;

    if (closeButton != nullptr)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW + buttonW / 5
                                           : -(buttonW + buttonW / 5);
    }

    if (positionTitleBarButtonsOnLeft)
        std::swap (minimizeButton, maximizeButton);

    if (maximizeButton != nullptr)
    {
        maximizeButton->setBounds (x, titleBarY - 2, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimizeButton != nullptr)
        minimizeButton->setBounds (x, titleBarY - 2, buttonW, titleBarH);
}

} // namespace juce
