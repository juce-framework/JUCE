/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../jucer_Headers.h"
#include "jucer_ProjucerLookAndFeel.h"


ProjucerLookAndFeel::ProjucerLookAndFeel()
{
    setColour (mainBackgroundColourId, Colour::greyLevel (0.8f));
}

int ProjucerLookAndFeel::getTabButtonBestWidth (TabBarButton&, int)
{
    return 120;
}

Colour ProjucerLookAndFeel::getTabBackgroundColour (TabBarButton& button)
{
    const Colour bkg (button.findColour (mainBackgroundColourId).contrasting (0.15f));

    if (button.isFrontTab())
        return bkg.overlaidWith (Colours::yellow.withAlpha (0.5f));

    return bkg;
}

void ProjucerLookAndFeel::drawTabButton (TabBarButton& button, Graphics& g, bool isMouseOver, bool isMouseDown)
{
    const Rectangle<int> activeArea (button.getActiveArea());

    const Colour bkg (getTabBackgroundColour (button));

    g.setGradientFill (ColourGradient (bkg.brighter (0.1f), 0, (float) activeArea.getY(),
                                       bkg.darker (0.1f), 0, (float) activeArea.getBottom(), false));
    g.fillRect (activeArea);

    g.setColour (button.findColour (mainBackgroundColourId).darker (0.3f));
    g.drawRect (activeArea);

    const float alpha = button.isEnabled() ? ((isMouseOver || isMouseDown) ? 1.0f : 0.8f) : 0.3f;
    const Colour col (bkg.contrasting().withMultipliedAlpha (alpha));

    TextLayout textLayout;
    LookAndFeel_V3::createTabTextLayout (button, (float) activeArea.getWidth(), (float) activeArea.getHeight(), col, textLayout);

    textLayout.draw (g, button.getTextArea().toFloat());
}

void ProjucerLookAndFeel::drawConcertinaPanelHeader (Graphics& g, const Rectangle<int>& area,
                                                     bool isMouseOver, bool /*isMouseDown*/,
                                                     ConcertinaPanel&, Component& panel)
{
    const Colour bkg (Colours::grey);

    g.setGradientFill (ColourGradient (Colour::greyLevel (isMouseOver ? 0.6f : 0.5f), 0, (float) area.getY(),
                                       Colour::greyLevel (0.4f), 0, (float) area.getBottom(), false));
    g.fillAll();

    g.setColour (bkg.contrasting().withAlpha (0.1f));
    g.fillRect (area.withHeight (1));
    g.fillRect (area.withTop (area.getBottom() - 1));

    g.setColour (bkg.contrasting());
    g.setFont (Font (area.getHeight() * 0.6f).boldened());
    g.drawFittedText (panel.getName(), 4, 0, area.getWidth() - 6, area.getHeight(), Justification::centredLeft, 1);
}

static Range<float> getBrightnessRange (const Image& im)
{
    float minB = 1.0f, maxB = 0;
    const int w = im.getWidth();
    const int h = im.getHeight();

    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            const float b = im.getPixelAt (x, y).getBrightness();
            minB = jmin (minB, b);
            maxB = jmax (maxB, b);
        }
    }

    return Range<float> (minB, maxB);
}

void ProjucerLookAndFeel::fillWithBackgroundTexture (Graphics& g)
{
    const Colour bkg (findColour (mainBackgroundColourId));

    if (backgroundTextureBaseColour != bkg)
    {
        backgroundTextureBaseColour = bkg;

        const Image original (ImageCache::getFromMemory (BinaryData::background_tile_png,
                                                         BinaryData::background_tile_pngSize));
        const int w = original.getWidth();
        const int h = original.getHeight();

        backgroundTexture = Image (Image::RGB, w, h, false);

        const Range<float> brightnessRange (getBrightnessRange (original));
        const float brightnessOffset = (brightnessRange.getStart() + brightnessRange.getEnd()) / 2.0f;
        const float brightnessScale = 0.025f / brightnessRange.getLength();
        const float bkgB = bkg.getBrightness();

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const float b = (original.getPixelAt (x, y).getBrightness() - brightnessOffset) * brightnessScale;
                backgroundTexture.setPixelAt (x, y, bkg.withBrightness (jlimit (0.0f, 1.0f, bkgB + b)));
            }
        }
    }

    g.setTiledImageFill (backgroundTexture, 0, 0, 1.0f);
    g.fillAll();
}

void ProjucerLookAndFeel::fillWithBackgroundTexture (Component& c, Graphics& g)
{
    dynamic_cast<ProjucerLookAndFeel&> (c.getLookAndFeel()).fillWithBackgroundTexture (g);
}
