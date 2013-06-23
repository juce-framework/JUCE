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

LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (const Image& image)
    : savedState (new RenderingHelpers::SoftwareRendererSavedState (image, image.getBounds()))
{
}

LowLevelGraphicsSoftwareRenderer::LowLevelGraphicsSoftwareRenderer (const Image& image, Point<int> origin,
                                                                    const RectangleList& initialClip)
    : savedState (new RenderingHelpers::SoftwareRendererSavedState (image, initialClip, origin.x, origin.y))
{
}

LowLevelGraphicsSoftwareRenderer::~LowLevelGraphicsSoftwareRenderer() {}

//==============================================================================
bool LowLevelGraphicsSoftwareRenderer::isVectorDevice() const                         { return false; }

void LowLevelGraphicsSoftwareRenderer::setOrigin (int x, int y)                       { savedState->transform.setOrigin (x, y); }
void LowLevelGraphicsSoftwareRenderer::addTransform (const AffineTransform& t)        { savedState->transform.addTransform (t); }
float LowLevelGraphicsSoftwareRenderer::getScaleFactor()                              { return savedState->transform.getScaleFactor(); }

Rectangle<int> LowLevelGraphicsSoftwareRenderer::getClipBounds() const                { return savedState->getClipBounds(); }
bool LowLevelGraphicsSoftwareRenderer::isClipEmpty() const                            { return savedState->clip == nullptr; }

bool LowLevelGraphicsSoftwareRenderer::clipToRectangle (const Rectangle<int>& r)      { return savedState->clipToRectangle (r); }
bool LowLevelGraphicsSoftwareRenderer::clipToRectangleList (const RectangleList& r)   { return savedState->clipToRectangleList (r); }
void LowLevelGraphicsSoftwareRenderer::excludeClipRectangle (const Rectangle<int>& r) { savedState->excludeClipRectangle (r); }

void LowLevelGraphicsSoftwareRenderer::clipToPath (const Path& path, const AffineTransform& transform)
{
    savedState->clipToPath (path, transform);
}

void LowLevelGraphicsSoftwareRenderer::clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
{
    savedState->clipToImageAlpha (sourceImage, transform);
}

bool LowLevelGraphicsSoftwareRenderer::clipRegionIntersects (const Rectangle<int>& r)
{
    return savedState->clipRegionIntersects (r);
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::saveState()                              { savedState.save(); }
void LowLevelGraphicsSoftwareRenderer::restoreState()                           { savedState.restore(); }

void LowLevelGraphicsSoftwareRenderer::beginTransparencyLayer (float opacity)   { savedState.beginTransparencyLayer (opacity); }
void LowLevelGraphicsSoftwareRenderer::endTransparencyLayer()                   { savedState.endTransparencyLayer(); }

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::setFill (const FillType& fillType)
{
    savedState->fillType = fillType;
}

void LowLevelGraphicsSoftwareRenderer::setOpacity (float newOpacity)
{
    savedState->fillType.setOpacity (newOpacity);
}

void LowLevelGraphicsSoftwareRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)
{
    savedState->interpolationQuality = quality;
}

//==============================================================================
void LowLevelGraphicsSoftwareRenderer::fillRect (const Rectangle<int>& r, const bool replaceExistingContents)
{
    savedState->fillRect (r, replaceExistingContents);
}

void LowLevelGraphicsSoftwareRenderer::fillPath (const Path& path, const AffineTransform& transform)
{
    savedState->fillPath (path, transform);
}

void LowLevelGraphicsSoftwareRenderer::drawImage (const Image& sourceImage, const AffineTransform& transform)
{
    savedState->renderImage (sourceImage, transform, nullptr);
}

void LowLevelGraphicsSoftwareRenderer::drawLine (const Line <float>& line)
{
    Path p;
    p.addLineSegment (line, 1.0f);
    fillPath (p, AffineTransform::identity);
}

void LowLevelGraphicsSoftwareRenderer::drawVerticalLine (const int x, const float top, const float bottom)
{
    if (bottom > top)
        savedState->fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top));
}

void LowLevelGraphicsSoftwareRenderer::drawHorizontalLine (const int y, const float left, const float right)
{
    if (right > left)
        savedState->fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f));
}

void LowLevelGraphicsSoftwareRenderer::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    const Font& f = savedState->font;

    if (transform.isOnlyTranslation() && savedState->transform.isOnlyTranslated)
    {
        using namespace RenderingHelpers;

        GlyphCache <CachedGlyphEdgeTable <SoftwareRendererSavedState>, SoftwareRendererSavedState>::getInstance()
            .drawGlyph (*savedState, f, glyphNumber,
                        transform.getTranslationX(),
                        transform.getTranslationY());
    }
    else
    {
        const float fontHeight = f.getHeight();
        savedState->drawGlyph (f, glyphNumber,
                               AffineTransform::scale (fontHeight * f.getHorizontalScale(), fontHeight)
                                               .followedBy (transform));
    }
}

void LowLevelGraphicsSoftwareRenderer::setFont (const Font& newFont)    { savedState->font = newFont; }
const Font& LowLevelGraphicsSoftwareRenderer::getFont()                 { return savedState->font; }
