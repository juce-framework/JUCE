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

// this will throw an assertion if you try to draw something that's not
// possible in postscript
#define WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS 0

//==============================================================================
#if JUCE_DEBUG && WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS
 #define notPossibleInPostscriptAssert jassertfalse
#else
 #define notPossibleInPostscriptAssert
#endif

//==============================================================================
LowLevelGraphicsPostScriptRenderer::LowLevelGraphicsPostScriptRenderer (OutputStream& resultingPostScript,
                                                                        const String& documentTitle,
                                                                        const int totalWidth_,
                                                                        const int totalHeight_)
    : out (resultingPostScript),
      totalWidth (totalWidth_),
      totalHeight (totalHeight_),
      needToClip (true)
{
    stateStack.add (new SavedState());
    stateStack.getLast()->clip = Rectangle<int> (totalWidth_, totalHeight_);

    const float scale = jmin ((520.0f / totalWidth_), (750.0f / totalHeight));

    out << "%!PS-Adobe-3.0 EPSF-3.0"
           "\n%%BoundingBox: 0 0 600 824"
           "\n%%Pages: 0"
           "\n%%Creator: Raw Material Software JUCE"
           "\n%%Title: " << documentTitle <<
           "\n%%CreationDate: none"
           "\n%%LanguageLevel: 2"
           "\n%%EndComments"
           "\n%%BeginProlog"
           "\n%%BeginResource: JRes"
           "\n/bd {bind def} bind def"
           "\n/c {setrgbcolor} bd"
           "\n/m {moveto} bd"
           "\n/l {lineto} bd"
           "\n/rl {rlineto} bd"
           "\n/ct {curveto} bd"
           "\n/cp {closepath} bd"
           "\n/pr {3 index 3 index moveto 1 index 0 rlineto 0 1 index rlineto pop neg 0 rlineto pop pop closepath} bd"
           "\n/doclip {initclip newpath} bd"
           "\n/endclip {clip newpath} bd"
           "\n%%EndResource"
           "\n%%EndProlog"
           "\n%%BeginSetup"
           "\n%%EndSetup"
           "\n%%Page: 1 1"
           "\n%%BeginPageSetup"
           "\n%%EndPageSetup\n\n"
        << "40 800 translate\n"
        << scale << ' ' << scale << " scale\n\n";
}

LowLevelGraphicsPostScriptRenderer::~LowLevelGraphicsPostScriptRenderer()
{
}

//==============================================================================
bool LowLevelGraphicsPostScriptRenderer::isVectorDevice() const
{
    return true;
}

void LowLevelGraphicsPostScriptRenderer::setOrigin (Point<int> o)
{
    if (! o.isOrigin())
    {
        stateStack.getLast()->xOffset += o.x;
        stateStack.getLast()->yOffset += o.y;
        needToClip = true;
    }
}

void LowLevelGraphicsPostScriptRenderer::addTransform (const AffineTransform& /*transform*/)
{
    //xxx
    jassertfalse;
}

float LowLevelGraphicsPostScriptRenderer::getPhysicalPixelScaleFactor()    { return 1.0f; }

bool LowLevelGraphicsPostScriptRenderer::clipToRectangle (const Rectangle<int>& r)
{
    needToClip = true;
    return stateStack.getLast()->clip.clipTo (r.translated (stateStack.getLast()->xOffset, stateStack.getLast()->yOffset));
}

bool LowLevelGraphicsPostScriptRenderer::clipToRectangleList (const RectangleList<int>& clipRegion)
{
    needToClip = true;
    return stateStack.getLast()->clip.clipTo (clipRegion);
}

void LowLevelGraphicsPostScriptRenderer::excludeClipRectangle (const Rectangle<int>& r)
{
    needToClip = true;
    stateStack.getLast()->clip.subtract (r.translated (stateStack.getLast()->xOffset, stateStack.getLast()->yOffset));
}

void LowLevelGraphicsPostScriptRenderer::clipToPath (const Path& path, const AffineTransform& transform)
{
    writeClip();

    Path p (path);
    p.applyTransform (transform.translated ((float) stateStack.getLast()->xOffset, (float) stateStack.getLast()->yOffset));
    writePath (p);
    out << "clip\n";
}

void LowLevelGraphicsPostScriptRenderer::clipToImageAlpha (const Image& /*sourceImage*/, const AffineTransform& /*transform*/)
{
    needToClip = true;
    jassertfalse; // xxx
}

bool LowLevelGraphicsPostScriptRenderer::clipRegionIntersects (const Rectangle<int>& r)
{
    return stateStack.getLast()->clip.intersectsRectangle (r.translated (stateStack.getLast()->xOffset, stateStack.getLast()->yOffset));
}

Rectangle<int> LowLevelGraphicsPostScriptRenderer::getClipBounds() const
{
    return stateStack.getLast()->clip.getBounds().translated (-stateStack.getLast()->xOffset,
                                                              -stateStack.getLast()->yOffset);
}

bool LowLevelGraphicsPostScriptRenderer::isClipEmpty() const
{
    return stateStack.getLast()->clip.isEmpty();
}

//==============================================================================
LowLevelGraphicsPostScriptRenderer::SavedState::SavedState()
    : xOffset (0),
      yOffset (0)
{
}

LowLevelGraphicsPostScriptRenderer::SavedState::~SavedState()
{
}

void LowLevelGraphicsPostScriptRenderer::saveState()
{
    stateStack.add (new SavedState (*stateStack.getLast()));
}

void LowLevelGraphicsPostScriptRenderer::restoreState()
{
    jassert (stateStack.size() > 0);

    if (stateStack.size() > 0)
        stateStack.removeLast();
}

void LowLevelGraphicsPostScriptRenderer::beginTransparencyLayer (float)
{
}

void LowLevelGraphicsPostScriptRenderer::endTransparencyLayer()
{
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::writeClip()
{
    if (needToClip)
    {
        needToClip = false;

        out << "doclip ";

        int itemsOnLine = 0;

        for (const Rectangle<int>* i = stateStack.getLast()->clip.begin(), * const e = stateStack.getLast()->clip.end(); i != e; ++i)
        {
            if (++itemsOnLine == 6)
            {
                itemsOnLine = 0;
                out << '\n';
            }

            out << i->getX() << ' ' << -i->getY() << ' '
                << i->getWidth() << ' ' << -i->getHeight() << " pr ";
        }

        out << "endclip\n";
    }
}

void LowLevelGraphicsPostScriptRenderer::writeColour (Colour colour)
{
    Colour c (Colours::white.overlaidWith (colour));

    if (lastColour != c)
    {
        lastColour = c;

        out << String (c.getFloatRed(), 3) << ' '
            << String (c.getFloatGreen(), 3) << ' '
            << String (c.getFloatBlue(), 3) << " c\n";
    }
}

void LowLevelGraphicsPostScriptRenderer::writeXY (const float x, const float y) const
{
    out << String (x, 2) << ' '
        << String (-y, 2) << ' ';
}

void LowLevelGraphicsPostScriptRenderer::writePath (const Path& path) const
{
    out << "newpath ";

    float lastX = 0.0f;
    float lastY = 0.0f;
    int itemsOnLine = 0;

    Path::Iterator i (path);

    while (i.next())
    {
        if (++itemsOnLine == 4)
        {
            itemsOnLine = 0;
            out << '\n';
        }

        switch (i.elementType)
        {
        case Path::Iterator::startNewSubPath:
            writeXY (i.x1, i.y1);
            lastX = i.x1;
            lastY = i.y1;
            out << "m ";
            break;

        case Path::Iterator::lineTo:
            writeXY (i.x1, i.y1);
            lastX = i.x1;
            lastY = i.y1;
            out << "l ";
            break;

        case Path::Iterator::quadraticTo:
            {
                const float cp1x = lastX + (i.x1 - lastX) * 2.0f / 3.0f;
                const float cp1y = lastY + (i.y1 - lastY) * 2.0f / 3.0f;
                const float cp2x = cp1x + (i.x2 - lastX) / 3.0f;
                const float cp2y = cp1y + (i.y2 - lastY) / 3.0f;

                writeXY (cp1x, cp1y);
                writeXY (cp2x, cp2y);
                writeXY (i.x2, i.y2);
                out << "ct ";
                lastX = i.x2;
                lastY = i.y2;
            }
            break;

        case Path::Iterator::cubicTo:
            writeXY (i.x1, i.y1);
            writeXY (i.x2, i.y2);
            writeXY (i.x3, i.y3);
            out << "ct ";
            lastX = i.x3;
            lastY = i.y3;
            break;

        case Path::Iterator::closePath:
            out << "cp ";
            break;

        default:
            jassertfalse;
            break;
        }
    }

    out << '\n';
}

void LowLevelGraphicsPostScriptRenderer::writeTransform (const AffineTransform& trans) const
{
    out << "[ "
        << trans.mat00 << ' '
        << trans.mat10 << ' '
        << trans.mat01 << ' '
        << trans.mat11 << ' '
        << trans.mat02 << ' '
        << trans.mat12 << " ] concat ";
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::setFill (const FillType& fillType)
{
    stateStack.getLast()->fillType = fillType;
}

void LowLevelGraphicsPostScriptRenderer::setOpacity (float /*opacity*/)
{
}

void LowLevelGraphicsPostScriptRenderer::setInterpolationQuality (Graphics::ResamplingQuality /*quality*/)
{
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::fillRect (const Rectangle<int>& r, const bool /*replaceExistingContents*/)
{
    fillRect (r.toFloat());
}

void LowLevelGraphicsPostScriptRenderer::fillRect (const Rectangle<float>& r)
{
    if (stateStack.getLast()->fillType.isColour())
    {
        writeClip();
        writeColour (stateStack.getLast()->fillType.colour);

        Rectangle<float> r2 (r.translated ((float) stateStack.getLast()->xOffset,
                                           (float) stateStack.getLast()->yOffset));

        out << r2.getX() << ' ' << -r2.getBottom() << ' ' << r2.getWidth() << ' ' << r2.getHeight() << " rectfill\n";
    }
    else
    {
        Path p;
        p.addRectangle (r);
        fillPath (p, AffineTransform::identity);
    }
}

void LowLevelGraphicsPostScriptRenderer::fillRectList (const RectangleList<float>& list)
{
    fillPath (list.toPath(), AffineTransform::identity);
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::fillPath (const Path& path, const AffineTransform& t)
{
    if (stateStack.getLast()->fillType.isColour())
    {
        writeClip();

        Path p (path);
        p.applyTransform (t.translated ((float) stateStack.getLast()->xOffset,
                                        (float) stateStack.getLast()->yOffset));
        writePath (p);

        writeColour (stateStack.getLast()->fillType.colour);

        out << "fill\n";
    }
    else if (stateStack.getLast()->fillType.isGradient())
    {
        // this doesn't work correctly yet - it could be improved to handle solid gradients, but
        // postscript can't do semi-transparent ones.
        notPossibleInPostscriptAssert;   // you can disable this warning by setting the WARN_ABOUT_NON_POSTSCRIPT_OPERATIONS flag at the top of this file

        writeClip();
        out << "gsave ";

        {
            Path p (path);
            p.applyTransform (t.translated ((float) stateStack.getLast()->xOffset, (float) stateStack.getLast()->yOffset));
            writePath (p);
            out << "clip\n";
        }

        const Rectangle<int> bounds (stateStack.getLast()->clip.getBounds());

        // ideally this would draw lots of lines or ellipses to approximate the gradient, but for the
        // time-being, this just fills it with the average colour..
        writeColour (stateStack.getLast()->fillType.gradient->getColourAtPosition (0.5f));
        out << bounds.getX() << ' ' << -bounds.getBottom() << ' ' << bounds.getWidth() << ' ' << bounds.getHeight() << " rectfill\n";

        out << "grestore\n";
    }
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::writeImage (const Image& im,
                                                     const int sx, const int sy,
                                                     const int maxW, const int maxH) const
{
    out << "{<\n";

    const int w = jmin (maxW, im.getWidth());
    const int h = jmin (maxH, im.getHeight());

    int charsOnLine = 0;
    const Image::BitmapData srcData (im, 0, 0, w, h);
    Colour pixel;

    for (int y = h; --y >= 0;)
    {
        for (int x = 0; x < w; ++x)
        {
            const uint8* pixelData = srcData.getPixelPointer (x, y);

            if (x >= sx && y >= sy)
            {
                if (im.isARGB())
                {
                    PixelARGB p (*(const PixelARGB*) pixelData);
                    p.unpremultiply();
                    pixel = Colours::white.overlaidWith (Colour (p.getARGB()));
                }
                else if (im.isRGB())
                {
                    pixel = Colour (((const PixelRGB*) pixelData)->getARGB());
                }
                else
                {
                    pixel = Colour ((uint8) 0, (uint8) 0, (uint8) 0, *pixelData);
                }
            }
            else
            {
                pixel = Colours::transparentWhite;
            }

            const uint8 pixelValues[3] = { pixel.getRed(), pixel.getGreen(), pixel.getBlue() };

            out << String::toHexString (pixelValues, 3, 0);
            charsOnLine += 3;

            if (charsOnLine > 100)
            {
                out << '\n';
                charsOnLine = 0;
            }
        }
    }

    out << "\n>}\n";
}

void LowLevelGraphicsPostScriptRenderer::drawImage (const Image& sourceImage, const AffineTransform& transform)
{
    const int w = sourceImage.getWidth();
    const int h = sourceImage.getHeight();

    writeClip();

    out << "gsave ";
    writeTransform (transform.translated ((float) stateStack.getLast()->xOffset, (float) stateStack.getLast()->yOffset)
                             .scaled (1.0f, -1.0f));

    RectangleList<int> imageClip;
    sourceImage.createSolidAreaMask (imageClip, 0.5f);

    out << "newpath ";
    int itemsOnLine = 0;

    for (const Rectangle<int>* i = imageClip.begin(), * const e = imageClip.end(); i != e; ++i)
    {
        if (++itemsOnLine == 6)
        {
            out << '\n';
            itemsOnLine = 0;
        }

        out << i->getX() << ' ' << i->getY() << ' ' << i->getWidth() << ' ' << i->getHeight() << " pr ";
    }

    out << " clip newpath\n";

    out << w << ' ' << h << " scale\n";
    out << w << ' ' << h << " 8 [" << w << " 0 0 -" << h << ' ' << (int) 0 << ' ' << h << " ]\n";

    writeImage (sourceImage, 0, 0, w, h);

    out << "false 3 colorimage grestore\n";
    needToClip = true;
}


//==============================================================================
void LowLevelGraphicsPostScriptRenderer::drawLine (const Line <float>& line)
{
    Path p;
    p.addLineSegment (line, 1.0f);
    fillPath (p, AffineTransform::identity);
}

//==============================================================================
void LowLevelGraphicsPostScriptRenderer::setFont (const Font& newFont)
{
    stateStack.getLast()->font = newFont;
}

const Font& LowLevelGraphicsPostScriptRenderer::getFont()
{
    return stateStack.getLast()->font;
}

void LowLevelGraphicsPostScriptRenderer::drawGlyph (int glyphNumber, const AffineTransform& transform)
{
    Path p;
    Font& font = stateStack.getLast()->font;
    font.getTypeface()->getOutlineForGlyph (glyphNumber, p);
    fillPath (p, AffineTransform::scale (font.getHeight() * font.getHorizontalScale(), font.getHeight()).followedBy (transform));
}
