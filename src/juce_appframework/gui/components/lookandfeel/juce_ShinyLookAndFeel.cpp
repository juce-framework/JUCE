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

#include "../../../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ShinyLookAndFeel.h"
#include "../buttons/juce_TextButton.h"
#include "../buttons/juce_ToggleButton.h"
#include "../buttons/juce_ShapeButton.h"
#include "../buttons/juce_ArrowButton.h"
#include "../windows/juce_AlertWindow.h"
#include "../windows/juce_DocumentWindow.h"
#include "../layout/juce_ScrollBar.h"
#include "../controls/juce_Slider.h"
#include "../controls/juce_ProgressBar.h"
#include "../controls/juce_ListBox.h"
#include "../filebrowser/juce_FilenameComponent.h"
#include "../juce_Desktop.h"
#include "../../graphics/brushes/juce_GradientBrush.h"
#include "../../../../juce_core/text/juce_LocalisedStrings.h"


//==============================================================================
static const Colour createBaseColour (const Colour& buttonColour,
                                      const bool hasKeyboardFocus,
                                      const bool isMouseOverButton,
                                      const bool isButtonDown)
{
    float sat = hasKeyboardFocus ? 1.3f : 0.9f;

    Colour baseColour (buttonColour.withMultipliedSaturation (sat));

    if (isButtonDown)
        return baseColour.contrasting (0.2f);
    else if (isMouseOverButton)
        return baseColour.contrasting (0.1f);

    return baseColour;
}


//==============================================================================
ShinyLookAndFeel::ShinyLookAndFeel()
{
    setColour (ComboBox::buttonColourId,                    Colour (0xffbbbbff));
    setColour (ComboBox::outlineColourId,                   Colours::grey.withAlpha (0.7f));

    setColour (ListBox::outlineColourId,            findColour (ComboBox::outlineColourId));

    setColour (ScrollBar::backgroundColourId,       Colours::transparentBlack);
    setColour (ScrollBar::thumbColourId,            Colours::white);

    setColour (Slider::thumbColourId,               findColour (TextButton::buttonColourId));
    setColour (Slider::trackColourId,               Colour (0x7fffffff));
    setColour (Slider::textBoxOutlineColourId,      findColour (ComboBox::outlineColourId));

    setColour (ProgressBar::backgroundColourId,     Colours::white);
    setColour (ProgressBar::foregroundColourId,     Colour (0xffaaaaee));

    setColour (PopupMenu::backgroundColourId,             Colours::white);
    setColour (PopupMenu::highlightedTextColourId,        Colours::white);
    setColour (PopupMenu::highlightedBackgroundColourId,  Colour (0x991111aa));

    setColour (TextEditor::focusedOutlineColourId,  findColour (TextButton::buttonColourId));
}

ShinyLookAndFeel::~ShinyLookAndFeel()
{
}

//==============================================================================
void ShinyLookAndFeel::drawTextEditorOutline (Graphics& g, int width, int height, TextEditor& textEditor)
{
    if (textEditor.isEnabled())
    {
        if (textEditor.hasKeyboardFocus (true) && ! textEditor.isReadOnly())
        {
            const int border = 2;

            g.setColour (textEditor.findColour (TextEditor::focusedOutlineColourId));
            g.drawRect (0, 0, width, height, border);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId).withMultipliedAlpha (0.75f));
            g.drawBevel (0, 0, width, height + 2, border + 2, shadowColour, shadowColour);
        }
        else
        {
            g.setColour (textEditor.findColour (TextEditor::outlineColourId));
            g.drawRect (0, 0, width, height);

            g.setOpacity (1.0f);
            const Colour shadowColour (textEditor.findColour (TextEditor::shadowColourId));
            g.drawBevel (0, 0, width, height + 2, 3, shadowColour, shadowColour);
        }
    }
}

//==============================================================================
void ShinyLookAndFeel::drawComboBox (Graphics& g, int width, int height,
                                     const bool isButtonDown,
                                     int buttonX, int buttonY,
                                     int buttonW, int buttonH,
                                     ComboBox& box)
{
    g.fillAll (box.findColour (ComboBox::backgroundColourId));

    if (box.isEnabled() && box.hasKeyboardFocus (false))
    {
        g.setColour (box.findColour (TextButton::buttonColourId));
        g.drawRect (0, 0, width, height, 2);
    }
    else
    {
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRect (0, 0, width, height);
    }

    const float outlineThickness = box.isEnabled() ? (isButtonDown ? 1.2f : 0.5f) : 0.3f;

    const Colour baseColour (createBaseColour (box.findColour (ComboBox::buttonColourId),
                                               box.hasKeyboardFocus (true),
                                               false, isButtonDown)
                               .withMultipliedAlpha (box.isEnabled() ? 1.0f : 0.5f));

    drawGlassLozenge (g,
                      buttonX + outlineThickness, buttonY + outlineThickness,
                      buttonW - outlineThickness * 2.0f, buttonH - outlineThickness * 2.0f,
                      baseColour, outlineThickness, -1.0f,
                      true, true, true, true);

    if (box.isEnabled())
    {
        const float arrowX = 0.3f;
        const float arrowH = 0.2f;

        Path p;
        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.45f - arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.45f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.45f);

        p.addTriangle (buttonX + buttonW * 0.5f,            buttonY + buttonH * (0.55f + arrowH),
                       buttonX + buttonW * (1.0f - arrowX), buttonY + buttonH * 0.55f,
                       buttonX + buttonW * arrowX,          buttonY + buttonH * 0.55f);

        g.setColour (Colours::black.withAlpha (0.6f));
        g.fillPath (p);
    }
}

const Font ShinyLookAndFeel::getComboBoxFont (ComboBox& box)
{
    Font f (jmin (15.0f, box.getHeight() * 0.85f));
    return f;
}


//==============================================================================
void ShinyLookAndFeel::drawScrollbarButton (Graphics& g,
                                            ScrollBar& scrollbar,
                                            int width, int height,
                                            int buttonDirection,
                                            bool /*isScrollbarVertical*/,
                                            bool /*isMouseOverButton*/,
                                            bool isButtonDown)
{
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
        g.setColour (scrollbar.findColour (ScrollBar::thumbColourId).contrasting (0.2f));
    else
        g.setColour (scrollbar.findColour (ScrollBar::thumbColourId));

    g.fillPath (p);

    g.setColour (Colours::black.withAlpha (0.5f));
    g.strokePath (p, PathStrokeType (0.5f));
}

void ShinyLookAndFeel::drawScrollbar (Graphics& g,
                                      ScrollBar& scrollbar,
                                      int x, int y,
                                      int width, int height,
                                      bool isScrollbarVertical,
                                      int thumbStartPosition,
                                      int thumbSize,
                                      bool /*isMouseOver*/,
                                      bool /*isMouseDown*/)
{
    g.fillAll (scrollbar.findColour (ScrollBar::backgroundColourId));

    Path slotPath, thumbPath;

    const float slotIndent = jmin (width, height) > 15 ? 1.0f : 0.0f;
    const float thumbIndent = slotIndent + 1.0f;

    float gx1 = 0.0f, gy1 = 0.0f, gx2 = 0.0f, gy2 = 0.0f;

    if (isScrollbarVertical)
    {
        slotPath.addRoundedRectangle (x + slotIndent,
                                      y + slotIndent,
                                      width - slotIndent * 2.0f,
                                      height - slotIndent * 2.0f,
                                      (width - slotIndent * 2.0f) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (x + thumbIndent,
                                           thumbStartPosition + thumbIndent,
                                           width - thumbIndent * 2.0f,
                                           thumbSize - thumbIndent * 2.0f,
                                           (width - thumbIndent * 2.0f) * 0.5f);
        gx1 = (float) x;
        gx2 = x + width * 0.7f;
    }
    else
    {
        slotPath.addRoundedRectangle (x + slotIndent,
                                      y + slotIndent,
                                      width - slotIndent * 2.0f,
                                      height - slotIndent * 2.0f,
                                      (height - slotIndent * 2.0f) * 0.5f);

        if (thumbSize > 0)
            thumbPath.addRoundedRectangle (thumbStartPosition + thumbIndent,
                                           y + thumbIndent,
                                           thumbSize - thumbIndent * 2.0f,
                                           height - thumbIndent * 2.0f,
                                           (height - thumbIndent * 2.0f) * 0.5f);
        gy1 = (float) y;
        gy2 = y + height * 0.7f;
    }

    const Colour thumbColour (scrollbar.findColour (ScrollBar::thumbColourId));

    GradientBrush gb (thumbColour.overlaidWith (Colours::black.withAlpha (0.27f)),
                      gx1, gy1,
                      thumbColour.overlaidWith (Colours::black.withAlpha (0.1f)),
                      gx2, gy2, false);

    g.setBrush (&gb);
    g.fillPath (slotPath);

    if (isScrollbarVertical)
    {
        gx1 = x + width * 0.6f;
        gx2 = (float) x + width;
    }
    else
    {
        gy1 = y + height * 0.6f;
        gy2 = (float) y + height;
    }

    GradientBrush gb2 (Colours::black.withAlpha (0.0f),
                       gx1, gy1,
                       Colours::black.withAlpha (0.1f),
                       gx2, gy2, false);

    g.setBrush (&gb2);
    g.fillPath (slotPath);

    g.setColour (thumbColour);
    g.fillPath (thumbPath);

    GradientBrush gb3 (Colours::black.withAlpha (0.05f),
                       gx1, gy1,
                       Colours::black.withAlpha (0.0f),
                       gx2, gy2, false);

    g.saveState();
    g.setBrush (&gb3);

    if (isScrollbarVertical)
        g.reduceClipRegion (x + width / 2, y, width, height);
    else
        g.reduceClipRegion (x, y + height / 2, width, height);

    g.fillPath (thumbPath);
    g.restoreState();

    g.setColour (Colours::black.withAlpha (0.3f));
    g.strokePath (thumbPath, PathStrokeType (0.4f));
}

ImageEffectFilter* ShinyLookAndFeel::getScrollbarEffect()
{
    return 0;
}

//==============================================================================
void ShinyLookAndFeel::drawButtonBackground (Graphics& g,
                                             Button& button,
                                             const Colour& backgroundColour,
                                             bool isMouseOverButton,
                                             bool isButtonDown)
{
    const int width = button.getWidth();
    const int height = button.getHeight();

    const float outlineThickness = button.isEnabled() ? ((isButtonDown || isMouseOverButton) ? 1.2f : 0.7f) : 0.4f;

    const float indentL = button.isConnectedOnLeft()   ? 0.1f : outlineThickness * 0.5f;
    const float indentR = button.isConnectedOnRight()  ? 0.1f : outlineThickness * 0.5f;
    const float indentT = button.isConnectedOnTop()    ? 0.1f : outlineThickness * 0.5f;
    const float indentB = button.isConnectedOnBottom() ? 0.1f : outlineThickness * 0.5f;

    const Colour baseColour (createBaseColour (backgroundColour,
                                               button.hasKeyboardFocus (true),
                                               isMouseOverButton, isButtonDown)
                               .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    drawGlassLozenge (g,
                      indentL,
                      indentT,
                      width - indentL - indentR,
                      height - indentT - indentB,
                      baseColour, outlineThickness, -1.0f,
                      button.isConnectedOnLeft(),
                      button.isConnectedOnRight(),
                      button.isConnectedOnTop(),
                      button.isConnectedOnBottom());
}

//==============================================================================
void ShinyLookAndFeel::drawTickBox (Graphics& g,
                                    Component& component,
                                    int x, int y, int w, int h,
                                    const bool ticked,
                                    const bool isEnabled,
                                    const bool isMouseOverButton,
                                    const bool isButtonDown)
{
    const float boxSize = w * 0.7f;

    drawGlassSphere (g, (float) x, y + (h - boxSize) * 0.5f, boxSize,
                     createBaseColour (component.findColour (TextButton::buttonColourId)
                                                .withMultipliedAlpha (isEnabled ? 1.0f : 0.5f),
                                       true,
                                       isMouseOverButton,
                                       isButtonDown),
                     isEnabled ? ((isButtonDown || isMouseOverButton) ? 1.1f : 0.5f) : 0.3f);

    if (ticked)
    {
        Path tick;
        tick.startNewSubPath (1.5f, 3.0f);
        tick.lineTo (3.0f, 6.0f);
        tick.lineTo (6.0f, 0.0f);

        g.setColour (isEnabled ? Colours::black : Colours::grey);

        AffineTransform trans (AffineTransform::scale (w / 9.0f, h / 9.0f)
                                    .translated ((float) x, (float) y));

        g.strokePath (tick, PathStrokeType (2.5f), trans);
    }
}

//==============================================================================
int ShinyLookAndFeel::getSliderThumbRadius (Slider& slider)
{
    return jmin (7,
                 slider.getHeight() / 2,
                 slider.getWidth() / 2);
}

void ShinyLookAndFeel::drawLinearSlider (Graphics& g,
                                         int x, int y,
                                         int width, int height,
                                         float sliderPos,
                                         float minSliderPos,
                                         float maxSliderPos,
                                         const Slider::SliderStyle style,
                                         Slider& slider)
{
    g.fillAll (slider.findColour (Slider::backgroundColourId));

    const bool isMouseOver = slider.isMouseOverOrDragging() && slider.isEnabled();

    if (style == Slider::LinearBar)
    {
        Colour baseColour (createBaseColour (slider.findColour (Slider::thumbColourId)
                                                   .withMultipliedSaturation (slider.isEnabled() ? 1.0f : 0.5f),
                                             false,
                                             isMouseOver, isMouseOver || slider.isMouseButtonDown()));

        drawShinyButtonShape (g,
                              (float) x, (float) y, sliderPos - (float) x, (float) height, 0.0f,
                              baseColour,
                              slider.isEnabled() ? 0.9f : 0.3f,
                              true, true, true, true);
    }
    else
    {
        const float sliderRadius = (float) getSliderThumbRadius (slider);

        const Colour trackColour (slider.findColour (Slider::trackColourId));
        const Colour gradCol1 (trackColour.overlaidWith (Colours::black.withAlpha (slider.isEnabled() ? 0.25f : 0.13f)));
        const Colour gradCol2 (trackColour.overlaidWith (Colours::black.withAlpha (0.08f)));
        Path indent;

        if (slider.isHorizontal())
        {
            const float iy = y + height * 0.5f - sliderRadius * 0.5f;
            const float ih = sliderRadius;

            GradientBrush gb (gradCol1, 0.0f, iy,
                              gradCol2, 0.0f, iy + ih, false);
            g.setBrush (&gb);

            indent.addRoundedRectangle (x - sliderRadius * 0.5f, iy,
                                        width + sliderRadius, ih,
                                        5.0f);
            g.fillPath (indent);
        }
        else
        {
            const float ix = x + width * 0.5f - sliderRadius * 0.5f;
            const float iw = sliderRadius;

            GradientBrush gb (gradCol1, ix, 0.0f,
                              gradCol2, ix + iw, 0.0f, false);
            g.setBrush (&gb);

            indent.addRoundedRectangle (ix, y - sliderRadius * 0.5f,
                                        iw, height + sliderRadius,
                                        5.0f);
            g.fillPath (indent);
        }

        g.setColour (Colours::black.withAlpha (0.3f));
        g.strokePath (indent, PathStrokeType (0.5f));

        Colour knobColour (createBaseColour (slider.findColour (Slider::thumbColourId),
                                             slider.hasKeyboardFocus (false) && slider.isEnabled(),
                                             isMouseOver,
                                             slider.isMouseButtonDown() && slider.isEnabled()));

        const float outlineThickness = slider.isEnabled() ? 0.8f : 0.3f;

        if (style == Slider::LinearHorizontal || style == Slider::LinearVertical)
        {
            float kx, ky;

            if (style == Slider::LinearVertical)
            {
                kx = x + width * 0.5f;
                ky = sliderPos;
            }
            else
            {
                kx = sliderPos;
                ky = y + height * 0.5f;
            }

            drawGlassSphere (g,
                             kx - sliderRadius,
                             ky - sliderRadius,
                             sliderRadius * 2.0f,
                             knobColour, outlineThickness);
        }
        else
        {
            if (style == Slider::ThreeValueVertical)
            {
                drawGlassSphere (g, x + width * 0.5f - sliderRadius,
                                 sliderPos - sliderRadius,
                                 sliderRadius * 2.0f,
                                 knobColour, outlineThickness);
            }
            else if (style == Slider::ThreeValueHorizontal)
            {
                drawGlassSphere (g,sliderPos - sliderRadius,
                                 y + height * 0.5f - sliderRadius,
                                 sliderRadius * 2.0f,
                                 knobColour, outlineThickness);
            }

            if (style == Slider::TwoValueVertical || style == Slider::ThreeValueVertical)
            {
                const float sr = jmin (sliderRadius, width * 0.4f);

                drawGlassPointer (g, jmax (0.0f, x + width * 0.5f - sliderRadius * 2.0f),
                                  minSliderPos - sliderRadius,
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 1);

                drawGlassPointer (g, jmin (x + width - sliderRadius * 2.0f, x + width * 0.5f), maxSliderPos - sr,
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 3);
            }
            else if (style == Slider::TwoValueHorizontal || style == Slider::ThreeValueHorizontal)
            {
                const float sr = jmin (sliderRadius, height * 0.4f);

                drawGlassPointer (g, minSliderPos - sr,
                                  jmax (0.0f, y + height * 0.5f - sliderRadius * 2.0f),
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 2);

                drawGlassPointer (g, maxSliderPos - sliderRadius,
                                  jmin (y + height - sliderRadius * 2.0f, y + height * 0.5f),
                                  sliderRadius * 2.0f, knobColour, outlineThickness, 4);
            }
        }
    }
}

Button* ShinyLookAndFeel::createSliderButton (const bool isIncrement)
{
    return new TextButton (isIncrement ? "+" : "-", String::empty);
}

ImageEffectFilter* ShinyLookAndFeel::getSliderEffect()
{
    return 0;
}

//==============================================================================
void ShinyLookAndFeel::drawPopupMenuBackground (Graphics& g, int width, int height)
{
    const Colour background (findColour (PopupMenu::backgroundColourId));

    g.fillAll (background);
    g.setColour (background.overlaidWith (Colours::lightblue.withAlpha (0.17f)));

    for (int i = 0; i < height; i += 3)
        g.fillRect (0, i, width, 1);

    g.setColour (findColour (PopupMenu::textColourId).withAlpha (0.6f));
    g.drawRect (0, 0, width, height);
}

void ShinyLookAndFeel::drawMenuBarBackground (Graphics& g, int width, int height,
                                              bool /*isMouseOverBar*/,
                                              MenuBarComponent& menuBar)
{
    const Colour baseColour (createBaseColour (menuBar.findColour (PopupMenu::backgroundColourId), false, false, false));

    if (menuBar.isEnabled())
    {
        drawShinyButtonShape (g,
                              -4.0f, 0.0f,
                              width + 8.0f, (float) height,
                              0.0f,
                              baseColour,
                              0.4f,
                              true, true, true, true);
    }
    else
    {
        g.fillAll (baseColour);
    }
}

//==============================================================================
void ShinyLookAndFeel::drawCornerResizer (Graphics& g,
                                          int w, int h,
                                          bool /*isMouseOver*/,
                                          bool /*isMouseDragging*/)
{
    const float lineThickness = jmin (w, h) * 0.075f;

    for (float i = 0.0f; i < 1.0f; i += 0.3f)
    {
        g.setColour (Colours::lightgrey);

        g.drawLine (w * i,
                    h + 1.0f,
                    w + 1.0f,
                    h * i,
                    lineThickness);

        g.setColour (Colours::darkgrey);

        g.drawLine (w * i + lineThickness,
                    h + 1.0f,
                    w + 1.0f,
                    h * i + lineThickness,
                    lineThickness);
    }
}

//==============================================================================
void ShinyLookAndFeel::drawProgressBar (Graphics& g, ProgressBar& progressBar,
                                        int x, int y, int w, int h,
                                        float progress)
{
    const Colour background (progressBar.findColour (ProgressBar::backgroundColourId));
    g.fillAll (background);

    g.setColour (background.contrasting (0.2f));
    g.drawRect (x, y, w, h);

    drawGlassLozenge (g,
                      (float) (x + 1),
                      (float) (y + 1),
                      jlimit (0.0f, w - 2.0f, progress * (w - 2.0f)),
                      (float) (h - 2),
                      progressBar.findColour (ProgressBar::foregroundColourId),
                      0.5f,
                      0.0f,
                      true, true, true, true);
}

//==============================================================================
class GlassWindowButton   : public Button
{
public:
    //==============================================================================
    GlassWindowButton (const String& name, const Colour& col,
                       const Path& normalShape_,
                       const Path& toggledShape_)
        : Button (name),
          colour (col),
          normalShape (normalShape_),
          toggledShape (toggledShape_)
    {
    }

    ~GlassWindowButton()
    {
    }

    //==============================================================================
    void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown)
    {
        float alpha = isMouseOverButton ? (isButtonDown ? 1.0f : 0.8f) : 0.55f;

        if (! isEnabled())
            alpha *= 0.5f;

        float x = 0, y = 0, diam;

        if (getWidth() < getHeight())
        {
            diam = (float) getWidth();
            y = (getHeight() - getWidth()) * 0.5f;
        }
        else
        {
            diam = (float) getHeight();
            y = (getWidth() - getHeight()) * 0.5f;
        }

        x += diam * 0.05f;
        y += diam * 0.05f;
        diam *= 0.9f;

        GradientBrush gb1 (Colour::greyLevel (0.9f).withAlpha (alpha), 0, y + diam,
                           Colour::greyLevel (0.6f).withAlpha (alpha), 0, y, false);

        g.setBrush (&gb1);
        g.fillEllipse (x, y, diam, diam);

        x += 2.0f;
        y += 2.0f;
        diam -= 4.0f;

        ShinyLookAndFeel::drawGlassSphere (g, x, y, diam, colour.withAlpha (alpha), 1.0f);

        Path& p = getToggleState() ? toggledShape : normalShape;

        const AffineTransform t (p.getTransformToScaleToFit (x + diam * 0.3f, y + diam * 0.3f,
                                                             diam * 0.4f, diam * 0.4f, true));

        g.setColour (Colours::black.withAlpha (alpha * 0.6f));
        g.fillPath (p, t);
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    Colour colour;
    Path normalShape, toggledShape;

    GlassWindowButton (const GlassWindowButton&);
    const GlassWindowButton& operator= (const GlassWindowButton&);
};


Button* ShinyLookAndFeel::createDocumentWindowButton (int buttonType)
{
    Path shape;
    const float crossThickness = 0.25f;

    if (buttonType == DocumentWindow::closeButton)
    {
        shape.addLineSegment (0.0f, 0.0f, 1.0f, 1.0f, crossThickness * 1.4f);
        shape.addLineSegment (1.0f, 0.0f, 0.0f, 1.0f, crossThickness * 1.4f);

        return new GlassWindowButton ("close", Colour (0xffdd1100), shape, shape);
    }
    else if (buttonType == DocumentWindow::minimiseButton)
    {
        shape.addLineSegment (0.0f, 0.5f, 1.0f, 0.5f, crossThickness);

        return new GlassWindowButton ("minimise", Colour (0xffaa8811), shape, shape);
    }
    else if (buttonType == DocumentWindow::maximiseButton)
    {
        shape.addLineSegment (0.5f, 0.0f, 0.5f, 1.0f, crossThickness);
        shape.addLineSegment (0.0f, 0.5f, 1.0f, 0.5f, crossThickness);

        Path fullscreenShape;
        fullscreenShape.startNewSubPath (45.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 100.0f);
        fullscreenShape.lineTo (0.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 0.0f);
        fullscreenShape.lineTo (100.0f, 45.0f);
        fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
        PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);

        return new GlassWindowButton ("maximise", Colour (0xff119911), shape, fullscreenShape);
    }

    jassertfalse
    return 0;
}

void ShinyLookAndFeel::positionDocumentWindowButtons (DocumentWindow&,
                                                      int titleBarX, int titleBarY,
                                                      int titleBarW, int titleBarH,
                                                      Button* minimiseButton,
                                                      Button* maximiseButton,
                                                      Button* closeButton,
                                                      bool positionTitleBarButtonsOnLeft)
{
    const int buttonW = titleBarH - titleBarH / 8;

    int x = positionTitleBarButtonsOnLeft ? titleBarX + 4
                                          : titleBarX + titleBarW - buttonW - buttonW / 4;

    if (closeButton != 0)
    {
        closeButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -(buttonW + buttonW / 4);
    }

    if (positionTitleBarButtonsOnLeft)
        swapVariables (minimiseButton, maximiseButton);

    if (maximiseButton != 0)
    {
        maximiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
        x += positionTitleBarButtonsOnLeft ? buttonW : -buttonW;
    }

    if (minimiseButton != 0)
        minimiseButton->setBounds (x, titleBarY, buttonW, titleBarH);
}


//==============================================================================
static void createRoundedPath (Path& p,
                               const float x, const float y,
                               const float w, const float h,
                               const float cs,
                               const bool curveTopLeft, const bool curveTopRight,
                               const bool curveBottomLeft, const bool curveBottomRight)
{
    const float cs2 = 2.0f * cs;

    if (curveTopLeft)
    {
        p.startNewSubPath (x, y + cs);
        p.addArc (x, y, cs2, cs2, float_Pi * 1.5f, float_Pi * 2.0f);
    }
    else
    {
        p.startNewSubPath (x, y);
    }

    if (curveTopRight)
    {
        p.lineTo (x + w - cs, y);
        p.addArc (x + w - cs2, y, cs2, cs2, 0.0f, float_Pi * 0.5f);
    }
    else
    {
        p.lineTo (x + w, y);
    }

    if (curveBottomRight)
    {
        p.lineTo (x + w, y + h - cs);
        p.addArc (x + w - cs2, y + h - cs2, cs2, cs2, float_Pi * 0.5f, float_Pi);
    }
    else
    {
        p.lineTo (x + w, y + h);
    }

    if (curveBottomLeft)
    {
        p.lineTo (x + cs, y + h);
        p.addArc (x, y + h - cs2, cs2, cs2, float_Pi, float_Pi * 1.5f);
    }
    else
    {
        p.lineTo (x, y + h);
    }

    p.closeSubPath();
}

void ShinyLookAndFeel::drawShinyButtonShape (Graphics& g,
                                             float x, float y, float w, float h,
                                             float maxCornerSize,
                                             const Colour& baseColour,
                                             const float strokeWidth,
                                             const bool flatOnLeft,
                                             const bool flatOnRight,
                                             const bool flatOnTop,
                                             const bool flatOnBottom)
{
    if (w <= strokeWidth * 1.1f || h <= strokeWidth * 1.1f)
        return;

    const float cs = jmin (maxCornerSize, w * 0.5f, h * 0.5f);

    Path outline;
    createRoundedPath (outline, x, y, w, h, cs,
                        ! (flatOnLeft || flatOnTop),
                        ! (flatOnRight || flatOnTop),
                        ! (flatOnLeft || flatOnBottom),
                        ! (flatOnRight || flatOnBottom));

    ColourGradient cg (baseColour.overlaidWith (Colours::white.withAlpha (0.0f)), 0.0f, y,
                       baseColour.overlaidWith (Colours::blue.withAlpha (0.03f)), 0.0f, y + h,
                       false);

    cg.addColour (0.5, baseColour.overlaidWith (Colours::white.withAlpha (0.2f)));
    cg.addColour (0.51, baseColour.overlaidWith (Colours::blue.withAlpha (0.07f)));

    GradientBrush gb (cg);
    g.setBrush (&gb);
    g.fillPath (outline);

    g.setColour (Colours::black.withAlpha (0.5f));
    g.strokePath (outline, PathStrokeType (strokeWidth));
}

//==============================================================================
void ShinyLookAndFeel::drawGlassSphere (Graphics& g,
                                        float x, float y, float diameter,
                                        const Colour& colour,
                                        const float outlineThickness)
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.addEllipse (x, y, diameter, diameter);

    {
        ColourGradient cg (Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColour (0.4, Colours::white.overlaidWith (colour));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    {
        GradientBrush gb (Colours::white, 0, y + diameter * 0.06f,
                          Colours::transparentWhite, 0, y + diameter * 0.3f, false);

        g.setBrush (&gb);
        g.fillEllipse (x + diameter * 0.2f, y + diameter * 0.05f, diameter * 0.6f, diameter * 0.4f);
    }

    {
        ColourGradient cg (Colours::transparentBlack,
                           x + diameter * 0.5f, y + diameter * 0.5f,
                           Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                           x, y + diameter * 0.5f, true);

        cg.addColour (0.7, Colours::transparentBlack);
        cg.addColour (0.8, Colours::black.withAlpha (0.1f * outlineThickness));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.drawEllipse (x, y, diameter, diameter, outlineThickness);
}

void ShinyLookAndFeel::drawGlassPointer (Graphics& g, float x, float y, float diameter,
                                         const Colour& colour, const float outlineThickness,
                                         const int direction)
{
    if (diameter <= outlineThickness)
        return;

    Path p;
    p.startNewSubPath (x + diameter * 0.5f, y);
    p.lineTo (x + diameter, y + diameter * 0.6f);
    p.lineTo (x + diameter, y + diameter);
    p.lineTo (x, y + diameter);
    p.lineTo (x, y + diameter * 0.6f);
    p.closeSubPath();

    p.applyTransform (AffineTransform::rotation (direction * (float_Pi * 0.5f), x + diameter * 0.5f, y + diameter * 0.5f));

    {
        ColourGradient cg (Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y,
                           Colours::white.overlaidWith (colour.withMultipliedAlpha (0.3f)), 0, y + diameter, false);

        cg.addColour (0.4, Colours::white.overlaidWith (colour));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    {
        ColourGradient cg (Colours::transparentBlack,
                           x + diameter * 0.5f, y + diameter * 0.5f,
                           Colours::black.withAlpha (0.5f * outlineThickness * colour.getFloatAlpha()),
                           x - diameter * 0.2f, y + diameter * 0.5f, true);

        cg.addColour (0.5, Colours::transparentBlack);
        cg.addColour (0.7, Colours::black.withAlpha (0.07f * outlineThickness));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (p);
    }

    g.setColour (Colours::black.withAlpha (0.5f * colour.getFloatAlpha()));
    g.strokePath (p, PathStrokeType (outlineThickness));
}

//==============================================================================
void ShinyLookAndFeel::drawGlassLozenge (Graphics& g,
                                         float x, float y, float width, float height,
                                         const Colour& colour,
                                         const float outlineThickness,
                                         const float cornerSize,
                                         const bool flatOnLeft,
                                         const bool flatOnRight,
                                         const bool flatOnTop,
                                         const bool flatOnBottom)
{
    if (width <= outlineThickness || height <= outlineThickness)
        return;

    const int intX = (int) x;
    const int intY = (int) y;
    const int intW = (int) width;
    const int intH = (int) height;

    const float cs = cornerSize < 0 ? jmin (width * 0.5f, height * 0.5f) : cornerSize;
    const float edgeBlurRadius = height * 0.75f + (height - cs * 2.0f);
    const int intEdge = (int) edgeBlurRadius;

    Path outline;
    createRoundedPath (outline, x, y, width, height, cs,
                        ! (flatOnLeft || flatOnTop),
                        ! (flatOnRight || flatOnTop),
                        ! (flatOnLeft || flatOnBottom),
                        ! (flatOnRight || flatOnBottom));

    {
        ColourGradient cg (colour.darker (0.2f), 0, y,
                           colour.darker (0.2f), 0, y + height, false);

        cg.addColour (0.03, colour.withMultipliedAlpha (0.3f));
        cg.addColour (0.4, colour);
        cg.addColour (0.97, colour.withMultipliedAlpha (0.3f));

        GradientBrush gb (cg);
        g.setBrush (&gb);
        g.fillPath (outline);
    }

    ColourGradient cg (Colours::transparentBlack, x + edgeBlurRadius, y + height * 0.5f,
                       colour.darker (0.2f), x, y + height * 0.5f, true);

    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.5f) / edgeBlurRadius), Colours::transparentBlack);
    cg.addColour (jlimit (0.0, 1.0, 1.0 - (cs * 0.25f) / edgeBlurRadius), colour.darker (0.2f).withMultipliedAlpha (0.3f));

    if (! (flatOnLeft || flatOnTop || flatOnBottom))
    {
        GradientBrush gb (cg);

        g.saveState();
        g.setBrush (&gb);
        g.reduceClipRegion (intX, intY, intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    if (! (flatOnRight || flatOnTop || flatOnBottom))
    {
        cg.x1 = x + width - edgeBlurRadius;
        cg.x2 = x + width;
        GradientBrush gb (cg);

        g.saveState();
        g.setBrush (&gb);
        g.reduceClipRegion (intX + intW - intEdge, intY, 2 + intEdge, intH);
        g.fillPath (outline);
        g.restoreState();
    }

    {
        const float leftIndent = flatOnLeft ? 0.0f : cs * 0.4f;
        const float rightIndent = flatOnRight ? 0.0f : cs * 0.4f;

        Path highlight;
        createRoundedPath (highlight,
                           x + leftIndent,
                           y + cs * 0.1f,
                           width - (leftIndent + rightIndent),
                           height * 0.4f, cs * 0.4f,
                           ! (flatOnLeft || flatOnTop),
                           ! (flatOnRight || flatOnTop),
                           ! (flatOnLeft || flatOnBottom),
                           ! (flatOnRight || flatOnBottom));

        GradientBrush gb (colour.brighter (10.0f), 0, y + height * 0.06f,
                          Colours::transparentWhite, 0, y + height * 0.4f, false);

        g.setBrush (&gb);
        g.fillPath (highlight);
    }

    g.setColour (colour.darker().withMultipliedAlpha (1.5f));
    g.strokePath (outline, PathStrokeType (outlineThickness));
}


END_JUCE_NAMESPACE
