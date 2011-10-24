/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

BEGIN_JUCE_NAMESPACE

namespace
{
    void applyFlippedMatrix (int x, int y, int width, int height)
    {
        OpenGLHelpers::prepareFor2D (width, height);
        OpenGLHelpers::applyTransform (AffineTransform::translation ((float) -x, (float) -y)
                                        .followedBy (AffineTransform::verticalFlip ((float) height)));
    }
}

struct OpenGLTarget
{
    OpenGLTarget (GLuint frameBufferID_, int width_, int height_) noexcept
        : frameBuffer (nullptr), frameBufferID (frameBufferID_),
          x (0), y (0), width (width_), height (height_)
    {}

    OpenGLTarget (OpenGLFrameBuffer& frameBuffer_, const Point<int>& origin) noexcept
        : frameBuffer (&frameBuffer_), frameBufferID (0), x (origin.getX()), y (origin.getY()),
          width (frameBuffer_.getWidth()), height (frameBuffer_.getHeight())
    {}

    OpenGLTarget (const OpenGLTarget& other) noexcept
        : frameBuffer (other.frameBuffer), frameBufferID (other.frameBufferID),
          x (other.x), y (other.y), width (other.width), height (other.height)
    {}

    void makeActiveFor2D() const
    {
        if (frameBuffer != nullptr)
            frameBuffer->makeCurrentRenderingTarget();
        else
            OpenGLFrameBuffer::setCurrentFrameBufferTarget (frameBufferID);

        applyFlippedMatrix (x, y, width, height);
        glDisable (GL_DEPTH_TEST);
    }

    void scissor (Rectangle<int> r) const
    {
        r = r.translated (-x, -y);
        OpenGLHelpers::enableScissorTest (r.withY (height - r.getBottom()));
    }

    OpenGLFrameBuffer* frameBuffer;
    GLuint frameBufferID;
    int x, y, width, height;
};

//==============================================================================
namespace
{
    enum { defaultOversamplingLevel = 4 };

    void fillRectangleList (const RectangleList& list)
    {
        glEnableClientState (GL_VERTEX_ARRAY);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);

        GLfloat vertices [8];
        glVertexPointer (2, GL_FLOAT, 0, vertices);

        for (RectangleList::Iterator i (list); i.next();)
        {
            vertices[0] = vertices[4] = (GLfloat) i.getRectangle()->getX();
            vertices[1] = vertices[3] = (GLfloat) i.getRectangle()->getY();
            vertices[2] = vertices[6] = (GLfloat) i.getRectangle()->getRight();
            vertices[5] = vertices[7] = (GLfloat) i.getRectangle()->getBottom();

            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    inline void setColour (const float alpha) noexcept
    {
        glColor4f (alpha, alpha, alpha, alpha);
    }

    void drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords, const int numVertices) noexcept
    {
        glEnable (GL_TEXTURE_2D);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glEnableClientState (GL_VERTEX_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
        glDrawArrays (GL_TRIANGLE_STRIP, 0, numVertices);
    }

    void drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords,
                            const int numVertices, const GLuint textureID) noexcept
    {
        jassert (textureID != 0);
        glBindTexture (GL_TEXTURE_2D, textureID);
        drawTriangleStrip (vertices, textureCoords, numVertices);
        glBindTexture (GL_TEXTURE_2D, 0);
    }

    void drawTextureQuad (GLuint textureID, int x, int y, int w, int h)
    {
        const GLfloat l = (GLfloat) x;
        const GLfloat t = (GLfloat) y;
        const GLfloat r = (GLfloat) (x + w);
        const GLfloat b = (GLfloat) (y + h);

        const GLfloat vertices[]      = { l, t, r, t, l, b, r, b };
        const GLfloat textureCoords[] = { 0, 1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 0 };

        drawTriangleStrip (vertices, textureCoords, 4, textureID);
    }

    void fillRectWithTexture (const Rectangle<int>& rect, GLuint textureID, const float alpha)
    {
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glColor4f (1.0f, 1.0f, 1.0f, alpha);

        drawTextureQuad (textureID, rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
    }

    void clipFrameBuffers (const OpenGLTarget& dest, OpenGLFrameBuffer& source,
                           const Point<int> sourceOrigin, const bool shouldMaskRGB)
    {
        dest.makeActiveFor2D();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ZERO, GL_SRC_ALPHA);
        setColour (1.0f);

        if (shouldMaskRGB)
            glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);

        drawTextureQuad (source.getTextureID(), sourceOrigin.getX(), sourceOrigin.getY(),
                         source.getWidth(), source.getHeight());

        if (shouldMaskRGB)
            glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    }

    void renderPath (const Path& path, const AffineTransform& transform, int oversamplingLevel)
    {
        glEnableClientState (GL_VERTEX_ARRAY);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisable (GL_TEXTURE_2D);
        glEnable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE);

        TriangulatedPath (path, transform).draw (oversamplingLevel);
    }

    void setNormalBlendingMode() noexcept
    {
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void setBlendMode (const bool replaceExistingContents) noexcept
    {
        if (replaceExistingContents)
            glDisable (GL_BLEND);
        else
            setNormalBlendingMode();
    }

    void fillRectWithTiledTexture (const OpenGLTarget& target, int textureWidth, int textureHeight,
                                   const Rectangle<int>& clip, const AffineTransform& transform, float alpha)
    {
        glEnable (GL_TEXTURE_2D);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glColor4f (1.0f, 1.0f, 1.0f, alpha);

        static bool canDoNonPowerOfTwos = OpenGLHelpers::isExtensionSupported ("GL_ARB_texture_non_power_of_two");

        if (canDoNonPowerOfTwos || (isPowerOfTwo (textureWidth) && isPowerOfTwo (textureHeight)))
        {
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            const GLfloat clipX = (GLfloat) clip.getX();
            const GLfloat clipY = (GLfloat) clip.getY();
            const GLfloat clipR = (GLfloat) clip.getRight();
            const GLfloat clipB = (GLfloat) clip.getBottom();

            const GLfloat vertices[]  = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };
            GLfloat textureCoords[]   = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };

            {
                const AffineTransform t (transform.inverted().scaled (1.0f / textureWidth,
                                                                      1.0f / textureHeight));
                t.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
                t.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);
            }

            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }
        else
        {
            // For hardware that can't handle non-power-of-two textures, this is a fallback algorithm
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            target.scissor (clip);
            glPushMatrix();
            OpenGLHelpers::applyTransform (transform);

            GLfloat vertices[8];
            const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };
            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

            const Rectangle<int> targetArea (clip.toFloat().transformed (transform.inverted()).getSmallestIntegerContainer());
            int x = targetArea.getX() - negativeAwareModulo (targetArea.getX(), textureWidth);
            int y = targetArea.getY() - negativeAwareModulo (targetArea.getY(), textureHeight);
            const int right  = targetArea.getRight();
            const int bottom = targetArea.getBottom();

            while (y < bottom)
            {
                vertices[1] = vertices[3] = (GLfloat) y;
                vertices[5] = vertices[7] = (GLfloat) (y + textureHeight);

                for (int x1 = x; x1 < right; x1 += textureWidth)
                {
                    vertices[0] = vertices[4] = (GLfloat) x1;
                    vertices[2] = vertices[6] = (GLfloat) (x1 + textureWidth);
                    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
                }

                y += textureHeight;
            }

            glPopMatrix();
            glDisable (GL_SCISSOR_TEST);
        }
    }

    void fillWithLinearGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform,
                                 const int textureSize)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.getX() - (grad.point2.getY() - grad.point1.getY()) / textureSize,
                                             grad.point1.getY() + (grad.point2.getX() - grad.point1.getX()) / textureSize).transformedBy (transform));

        const AffineTransform textureTransform (AffineTransform::fromTargetPoints (p1.getX(), p1.getY(),  0.0f, 0.0f,
                                                                                   p2.getX(), p2.getY(),  1.0f, 0.0f,
                                                                                   p3.getX(), p3.getY(),  0.0f, 1.0f));

        const GLfloat l = (GLfloat) rect.getX();
        const GLfloat r = (GLfloat) rect.getRight();
        const GLfloat t = (GLfloat) rect.getY();
        const GLfloat b = (GLfloat) rect.getBottom();

        const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]  = { l, t, r, t, l, b, r, b };

        textureTransform.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        textureTransform.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        drawTriangleStrip (vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (const OpenGLTarget& target, const Rectangle<int>& rect,
                                 const ColourGradient& grad, const AffineTransform& transform)
    {
        const Point<float> centre (grad.point1.transformedBy (transform));

        const float screenRadius = centre.getDistanceFrom (rect.getCentre().toFloat())
                                    + Point<int> (rect.getWidth() / 2,
                                                  rect.getHeight() / 2).getDistanceFromOrigin()
                                    + 8.0f;

        const AffineTransform inverse (transform.inverted());
        const float sourceRadius = jmax (Point<float> (screenRadius, 0.0f).transformedBy (inverse).getDistanceFromOrigin(),
                                         Point<float> (0.0f, screenRadius).transformedBy (inverse).getDistanceFromOrigin());

        const int numDivisions = 90;
        GLfloat vertices      [4 + numDivisions * 2];
        GLfloat textureCoords [4 + numDivisions * 2];

        {
            GLfloat* t = textureCoords;
            *t++ = 0.0f;
            *t++ = 0.0f;

            const GLfloat texturePos = sourceRadius / grad.point1.getDistanceFrom (grad.point2);

            for (int i = numDivisions + 1; --i >= 0;)
            {
                *t++ = texturePos;
                *t++ = 0.0f;
            }
        }

        {
            GLfloat* v = vertices;
            *v++ = centre.getX();
            *v++ = centre.getY();

            const Point<float> first (grad.point1.translated (0, -sourceRadius)
                                                 .transformedBy (transform));
            *v++ = first.getX();
            *v++ = first.getY();

            for (int i = 1; i < numDivisions; ++i)
            {
                const float angle = i * (float_Pi * 2.0f / numDivisions);
                const Point<float> p (grad.point1.translated (std::sin (angle) * sourceRadius,
                                                              std::cos (angle) * -sourceRadius)
                                                 .transformedBy (transform));
                *v++ = p.getX();
                *v++ = p.getY();
            }

            *v++ = first.getX();
            *v++ = first.getY();
        }

        target.scissor (rect);
        glEnable (GL_TEXTURE_2D);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
        setColour (1.0f);
        glDrawArrays (GL_TRIANGLE_FAN, 0, numDivisions + 2);
        glDisable (GL_SCISSOR_TEST);
    }

    void fillRectWithColourGradient (const OpenGLTarget& target, const Rectangle<int>& rect,
                                     const ColourGradient& gradient, const AffineTransform& transform)
    {
        if (gradient.point1 == gradient.point2)
        {
            OpenGLHelpers::fillRectWithColour (rect, gradient.getColourAtPosition (1.0));
        }
        else
        {
            const int textureSize = 256;
            OpenGLTexture texture;

            HeapBlock<PixelARGB> lookup (textureSize);
            gradient.createLookupTable (lookup, textureSize);
            texture.load (lookup, textureSize, 1);
            texture.bind();

            if (gradient.isRadial)
                fillWithRadialGradient (target, rect, gradient, transform);
            else
                fillWithLinearGradient (rect, gradient, transform, textureSize);
        }
    }

    void fillRectWithFillType (const OpenGLTarget& target, const Rectangle<int>& rect,
                               const FillType& fill, const bool replaceExistingContents)
    {
        jassert (! fill.isInvisible());
        jassert (! fill.isColour());

        if (fill.isGradient())
        {
            target.makeActiveFor2D();
            setBlendMode (replaceExistingContents);

            ColourGradient g2 (*(fill.gradient));
            g2.multiplyOpacity (fill.getOpacity());

            fillRectWithColourGradient (target, rect, g2, fill.transform);
        }
        else if (fill.isTiledImage())
        {
            OpenGLTextureFromImage t (fill.image);

            target.makeActiveFor2D();
            setBlendMode (replaceExistingContents);

            glBindTexture (GL_TEXTURE_2D, t.textureID);
            fillRectWithTiledTexture (target, t.width, t.height, rect,
                                      fill.transform, fill.colour.getFloatAlpha());
            glBindTexture (GL_TEXTURE_2D, 0);
        }
    }
}

class ClipRegion_Mask;

//==============================================================================
class ClipRegionBase  : public SingleThreadedReferenceCountedObject
{
public:
    ClipRegionBase() noexcept {}
    virtual ~ClipRegionBase() {}

    typedef ReferenceCountedObjectPtr<ClipRegionBase> Ptr;

    virtual Ptr clone() const = 0;
    virtual Ptr applyClipTo (const Ptr& target) = 0;
    virtual Ptr clipToRectangle (const Rectangle<int>&) = 0;
    virtual Ptr clipToRectangleList (const RectangleList&) = 0;
    virtual Ptr excludeClipRectangle (const Rectangle<int>&) = 0;
    virtual Ptr clipToPath (const Path& p, const AffineTransform&) = 0;
    virtual Ptr clipToEdgeTable (const EdgeTable&) = 0;
    virtual Ptr clipToImageAlpha (const OpenGLTextureFromImage&, const AffineTransform&) = 0;
    virtual Ptr clipToMask (ClipRegion_Mask*) = 0;
    virtual void translate (const Point<int>& delta) = 0;
    virtual const Rectangle<int>& getClipBounds() const = 0;
    virtual void fillAll (const OpenGLTarget&, const FillType& fill, bool replaceContents) = 0;
    virtual void fillRect (const OpenGLTarget&, const Rectangle<int>& area, const FillType& fill, bool replaceContents) = 0;
    virtual void drawImage (const OpenGLTarget&, const OpenGLTextureFromImage&, float alpha, const Rectangle<int>& targetArea) = 0;
};

//==============================================================================
class ClipRegion_Mask  : public ClipRegionBase
{
public:
    ClipRegion_Mask (const ClipRegion_Mask& other)
        : clip (other.clip),
          maskOrigin (other.maskOrigin)
    {
        const bool ok = mask.initialise (other.mask);
        (void) ok; jassert (ok);
    }

    explicit ClipRegion_Mask (const Rectangle<int>& r)
        : clip (r),
          maskOrigin (r.getPosition())
    {
        const bool ok = mask.initialise (r.getWidth(), r.getHeight());
        (void) ok; jassert (ok);
        mask.clear (Colours::white);
    }

    explicit ClipRegion_Mask (const Rectangle<float>& r)
        : clip (r.getSmallestIntegerContainer()),
          maskOrigin (clip.getPosition())
    {
        initialiseClear();

        glEnableClientState (GL_VERTEX_ARRAY);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);

        RenderingHelpers::FloatRectangleRasterisingInfo fr (r);
        FillFloatRectCallback callback;
        fr.iterate (callback);
    }

    explicit ClipRegion_Mask (const EdgeTable& e)
        : clip (e.getMaximumBounds()),
          maskOrigin (clip.getPosition())
    {
        initialiseClear();
        OpenGLHelpers::fillEdgeTable (e, 0, 0, 0);
    }

    ClipRegion_Mask (const Rectangle<int>& bounds, const Path& p, const AffineTransform& transform, int oversamplingLevel)
        : clip (bounds), maskOrigin (clip.getPosition())
    {
        initialiseClear();
        renderPath (p, transform, oversamplingLevel);
    }

    static ClipRegion_Mask* createFromPath (Rectangle<int> bounds, const Path& p, const AffineTransform& transform)
    {
        bounds = bounds.getIntersection (p.getBoundsTransformed (transform).getSmallestIntegerContainer());

        return bounds.isEmpty() ? nullptr
                                : new ClipRegion_Mask (bounds, p, transform, (int) defaultOversamplingLevel);
    }

    Ptr clone() const                               { return new ClipRegion_Mask (*this); }
    const Rectangle<int>& getClipBounds() const     { return clip; }
    Ptr applyClipTo (const Ptr& target)             { return target->clipToMask (this); }

    void translate (const Point<int>& delta)
    {
        maskOrigin += delta;
        clip += delta;
    }

    Ptr clipToRectangle (const Rectangle<int>& r)
    {
        clip = clip.getIntersection (r);
        return clip.isEmpty() ? nullptr : this;
    }

    Ptr clipToRectangleList (const RectangleList& r)
    {
        clip = clip.getIntersection (r.getBounds());
        if (clip.isEmpty())
            return nullptr;

        RectangleList excluded (clip);

        if (excluded.subtract (r))
        {
            if (excluded.getNumRectangles() == 1)
                return excludeClipRectangle (excluded.getRectangle (0));

            makeMaskActive();
            glDisable (GL_BLEND);
            setColour (0);
            fillRectangleList (excluded);
        }

        return this;
    }

    Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        if (r.contains (clip))
            return nullptr;

        makeMaskActive();
        glDisable (GL_BLEND);
        setColour (0);
        OpenGLHelpers::fillRect (r);
        return this;
    }

    Ptr clipToPath (const Path& p, const AffineTransform& t)
    {
        ClipRegion_Mask* tempMask = createFromPath (clip, p, t);
        const Ptr tempMaskPtr (tempMask);
        return tempMask == nullptr ? nullptr : clipToMask (tempMask);
    }

    Ptr clipToEdgeTable (const EdgeTable& et)
    {
        ClipRegion_Mask* const tempMask = new ClipRegion_Mask (et);
        const Ptr tempMaskPtr (tempMask);
        return clipToMask (tempMask);
    }

    Ptr clipToMask (ClipRegion_Mask* m)
    {
        jassert (m != nullptr && m != this);
        clip = clip.getIntersection (m->clip);

        if (clip.isEmpty())
            return nullptr;

        clipFrameBuffers (OpenGLTarget (mask, maskOrigin), m->mask, m->maskOrigin, true);
        return this;
    }

    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)
    {
        makeMaskActive();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ZERO, GL_SRC_ALPHA);
        glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        fillMaskWithSourceImage (image, transform);
        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        return this;
    }

    void fillAll (const OpenGLTarget& target, const FillType& fill, bool replaceContents)
    {
        jassert (! replaceContents);
        fillRectInternal (target, clip, fill, false);
    }

    void fillRect (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        jassert (! replaceContents);
        const Rectangle<int> r (clip.getIntersection (area));

        if (! r.isEmpty())
            fillRectInternal (target, r, fill, false);
    }

    void fillRectInternal (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        if (fill.isColour())
        {
            target.makeActiveFor2D();

            setBlendMode (replaceContents);
            OpenGLHelpers::setColour (fill.colour);
            target.scissor (area);
            drawFrameBuffer (mask, maskOrigin);
            glDisable (GL_SCISSOR_TEST);
        }
        else
        {
            OpenGLFrameBuffer patternBuffer;
            bool ok = patternBuffer.initialise (area.getWidth(), area.getHeight());
            (void) ok; jassert (ok);

            fillRectWithFillType (OpenGLTarget (patternBuffer, area.getPosition()), area, fill, true);
            clipAndDraw (target, OpenGLTarget (patternBuffer, area.getPosition()));
        }
    }

    void drawImage (const OpenGLTarget& target, const OpenGLTextureFromImage& source, float alpha, const Rectangle<int>& targetArea)
    {
        const Rectangle<int> bufferArea (targetArea.getIntersection (clip));

        if (! bufferArea.isEmpty())
        {
            OpenGLFrameBuffer buffer;
            bool ok = buffer.initialise (bufferArea.getWidth(), bufferArea.getHeight());
            (void) ok; jassert (ok);

            OpenGLTarget bufferTarget (buffer, bufferArea.getPosition());
            bufferTarget.makeActiveFor2D();
            glDisable (GL_BLEND);
            fillRectWithTexture (targetArea, source.textureID, alpha);

            clipAndDraw (target, bufferTarget);
        }
    }

    void drawImageSelfDestructively (const OpenGLTarget& target, const OpenGLTextureFromImage& source,
                                     float alpha, const AffineTransform& transform)
    {
        makeMaskActive();
        glEnable (GL_BLEND);
        glBlendFunc (GL_DST_ALPHA, GL_ZERO);
        fillMaskWithSourceImage (source, transform);

        target.makeActiveFor2D();
        setColour (alpha);
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        drawFrameBuffer (mask, maskOrigin);
    }

private:
    OpenGLFrameBuffer mask;
    Rectangle<int> clip;
    Point<int> maskOrigin;

    void prepareFor2D() const
    {
        applyFlippedMatrix (maskOrigin.getX(), maskOrigin.getY(), mask.getWidth(), mask.getHeight());
    }

    void makeMaskActive()
    {
        const bool b = mask.makeCurrentRenderingTarget();
        (void) b; jassert (b);
        prepareFor2D();
    }

    void initialiseClear()
    {
        jassert (! clip.isEmpty());
        bool ok = mask.initialise (clip.getWidth(), clip.getHeight());
        mask.makeCurrentAndClear();
        (void) ok; jassert (ok);
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_BLEND);
        prepareFor2D();
    }

    struct FillFloatRectCallback
    {
        void operator() (const int x, const int y, const int w, const int h, const int alpha) const
        {
            const GLfloat l = (GLfloat) x;
            const GLfloat t = (GLfloat) y;
            const GLfloat r = (GLfloat) (x + w);
            const GLfloat b = (GLfloat) (y + h);

            const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
            glColor4f (1.0f, 1.0f, 1.0f, alpha / 255.0f);
            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }
    };

    void clipAndDraw (const OpenGLTarget& target, const OpenGLTarget& buffer)
    {
        clipFrameBuffers (buffer, mask, maskOrigin, false);

        target.makeActiveFor2D();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        setColour (1.0f);

        drawFrameBuffer (*buffer.frameBuffer, Point<int> (buffer.x, buffer.y));
    }

    void drawFrameBuffer (const OpenGLFrameBuffer& buffer, const Point<int>& topLeft)
    {
        drawTextureQuad (buffer.getTextureID(), topLeft.getX(), topLeft.getY(),
                         buffer.getWidth(), buffer.getHeight());
    }

    void fillMaskWithSourceImage (const OpenGLTextureFromImage& image, const AffineTransform& transform) const
    {
        setColour (1.0f);
        glBindTexture (GL_TEXTURE_2D, image.textureID);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        const GLfloat l = (GLfloat) maskOrigin.getX();
        const GLfloat t = (GLfloat) maskOrigin.getY();
        const GLfloat r = (GLfloat) (maskOrigin.getX() + mask.getWidth());
        const GLfloat b = (GLfloat) (maskOrigin.getY() + mask.getHeight());
        const GLfloat vertices[]  = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]   = { l, t, r, t, l, b, r, b };

        const AffineTransform inv (transform.inverted().scaled (1.0f / image.width,
                                                                1.0f / image.height));

        inv.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        inv.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        drawTriangleStrip (vertices, textureCoords, 4);
    }

    ClipRegion_Mask& operator= (const ClipRegion_Mask&);
};


//==============================================================================
class ClipRegion_Rectangle  : public ClipRegionBase
{
public:
    explicit ClipRegion_Rectangle (const Rectangle<int>& r) noexcept
        : clip (r)
    {}

    Ptr clone() const                               { return new ClipRegion_Rectangle (clip); }
    const Rectangle<int>& getClipBounds() const     { return clip; }
    Ptr applyClipTo (const Ptr& target)             { return target->clipToRectangle (clip); }
    void translate (const Point<int>& delta)        { clip += delta; }

    Ptr clipToRectangle (const Rectangle<int>& r)
    {
        clip = clip.getIntersection (r);
        return clip.isEmpty() ? nullptr : this;
    }

    Ptr clipToRectangleList (const RectangleList& r)
    {
        if (r.getNumRectangles() <= 1)
            return clipToRectangle (r.getRectangle (0));

        if (r.containsRectangle (clip))
            return this;

        return toMask()->clipToRectangleList (r);
    }

    Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        return r.contains (clip) ? nullptr
                                 : toMask()->excludeClipRectangle (r);
    }

    Ptr clipToMask (ClipRegion_Mask* m)                                 { return m->clipToRectangle (clip); }
    Ptr clipToPath (const Path& p, const AffineTransform& transform)    { return toMask()->clipToPath (p, transform); }
    Ptr clipToEdgeTable (const EdgeTable& et)                           { return toMask()->clipToEdgeTable (et); }
    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)    { return toMask()->clipToImageAlpha (image, transform); }

    void fillAll (const OpenGLTarget& target, const FillType& fill, bool replaceContents)
    {
        fillRectInternal (target, clip, fill, replaceContents);
    }

    void fillRect (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        const Rectangle<int> r (clip.getIntersection (area));

        if (! r.isEmpty())
            fillRectInternal (target, r, fill, replaceContents);
    }

    void drawImage (const OpenGLTarget& target, const OpenGLTextureFromImage& source, float alpha, const Rectangle<int>& targetArea)
    {
        target.makeActiveFor2D();
        target.scissor (clip);
        setNormalBlendingMode();
        fillRectWithTexture (targetArea, source.textureID, alpha);
        glDisable (GL_SCISSOR_TEST);
    }

private:
    Rectangle<int> clip;

    void fillRectInternal (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        if (fill.isColour())
        {
            target.makeActiveFor2D();
            setBlendMode (replaceContents);
            glDisable (GL_TEXTURE_2D);
            OpenGLHelpers::fillRectWithColour (area, fill.colour);
        }
        else
        {
            fillRectWithFillType (target, area, fill, replaceContents);
        }
    }

    Ptr toMask() const
    {
        return new ClipRegion_Mask (clip);
    }

    ClipRegion_Rectangle& operator= (const ClipRegion_Rectangle&);
};


//==============================================================================
class OpenGLRenderer::SavedState
{
public:
    SavedState (const OpenGLTarget& target_)
        : clip (new ClipRegion_Rectangle (Rectangle<int> (target_.width, target_.height))),
          transform (0, 0), interpolationQuality (Graphics::mediumResamplingQuality),
          target (target_), transparencyLayerAlpha (1.0f)
    {
    }

    SavedState (const SavedState& other)
        : clip (other.clip), transform (other.transform), font (other.font),
          fillType (other.fillType), interpolationQuality (other.interpolationQuality),
          target (other.target), transparencyLayerAlpha (other.transparencyLayerAlpha),
          transparencyLayer (other.transparencyLayer)
    {
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.translated (r));
            }
            else
            {
                Path p;
                p.addRectangle (r);
                clipToPath (p, AffineTransform::identity);
            }
        }

        return clip != nullptr;
    }

    bool clipToRectangleList (const RectangleList& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();
                RectangleList offsetList (r);
                offsetList.offsetAll (transform.xOffset, transform.yOffset);
                clip = clip->clipToRectangleList (offsetList);
            }
            else
            {
                clipToPath (r.toPath(), AffineTransform::identity);
            }
        }

        return clip != nullptr;
    }

    bool excludeClipRectangle (const Rectangle<int>& r)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();

            if (transform.isOnlyTranslated)
            {
                clip = clip->excludeClipRectangle (transform.translated (r));
            }
            else
            {
                Path p;
                p.addRectangle (r.toFloat());
                p.applyTransform (transform.complexTransform);
                p.addRectangle (clip->getClipBounds().toFloat());
                p.setUsingNonZeroWinding (false);
                clip = clip->clipToPath (p, AffineTransform::identity);
            }
        }

        return clip != nullptr;
    }

    void clipToPath (const Path& p, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            cloneClipIfMultiplyReferenced();
            clip = clip->clipToPath (p, transform.getTransformWith (t));
        }
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            Path p;
            p.addRectangle (sourceImage.getBounds());
            clipToPath (p, t);

            if (sourceImage.hasAlphaChannel() && clip != nullptr)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToImageAlpha (sourceImage, transform.getTransformWith (t));
            }
        }
    }

    bool clipRegionIntersects (const Rectangle<int>& r) const
    {
        return clip != nullptr
                && (transform.isOnlyTranslated ? clip->getClipBounds().intersects (transform.translated (r))
                                               : getClipBounds().intersects (r));
    }

    Rectangle<int> getClipBounds() const
    {
        return clip != nullptr ? transform.deviceSpaceToUserSpace (clip->getClipBounds())
                               : Rectangle<int>();
    }

    SavedState* beginTransparencyLayer (float opacity)
    {
        SavedState* s = new SavedState (*this);

        if (clip != nullptr)
        {
            const Rectangle<int>& clipBounds = clip->getClipBounds();

            OpenGLFrameBufferImage* fbi = new OpenGLFrameBufferImage (clipBounds.getWidth(), clipBounds.getHeight());
            fbi->frameBuffer.clear (Colours::transparentBlack);
            s->transparencyLayer = Image (fbi);
            s->target = OpenGLTarget (fbi->frameBuffer, Point<int>());
            s->transparencyLayerAlpha = opacity;
            s->transform.moveOriginInDeviceSpace (-clipBounds.getX(), -clipBounds.getY());
            s->cloneClipIfMultiplyReferenced();
            s->clip->translate (-clipBounds.getPosition());
        }

        return s;
    }

    void endTransparencyLayer (SavedState& finishedLayerState)
    {
        if (clip != nullptr)
            clip->drawImage (target, finishedLayerState.transparencyLayer,
                             finishedLayerState.transparencyLayerAlpha, clip->getClipBounds());
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, const bool replaceContents)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                clip->fillRect (target, r.translated (transform.xOffset, transform.yOffset),
                                getFillType(), replaceContents);
            }
            else
            {
                Path p;
                p.addRectangle (r);
                fillPath (p, AffineTransform::identity);
            }
        }
    }

    void fillRect (const Rectangle<float>& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                fillShape (new ClipRegion_Mask (r.translated ((float) transform.xOffset,
                                                              (float) transform.yOffset)), false);
            }
            else
            {
                Path p;
                p.addRectangle (r);
                fillPath (p, AffineTransform::identity);
            }
        }
    }

    void fillPath (const Path& path, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            ClipRegion_Mask* m = ClipRegion_Mask::createFromPath (clip->getClipBounds(), path,
                                                                  transform.getTransformWith (t));

            if (m != nullptr)
                fillShape (m, false);
        }
    }

    void drawGlyph (int glyphNumber, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            const float fontHeight = font.getHeight();

            const ScopedPointer<EdgeTable> et (font.getTypeface()->getEdgeTableForGlyph
                    (glyphNumber, transform.getTransformWith (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                              .followedBy (t))));

            if (et != nullptr)
                fillShape (new ClipRegion_Mask (*et), false);
        }
    }

    void drawLine (const Line <float>& line)
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, AffineTransform::identity);
    }

    void fillShape (ClipRegionBase::Ptr shapeToFill, const bool replaceContents)
    {
        jassert (clip != nullptr && shapeToFill != nullptr);

        if (! fillType.isInvisible())
        {
            shapeToFill = clip->applyClipTo (shapeToFill);

            if (shapeToFill != nullptr)
                shapeToFill->fillAll (target, getFillType(), replaceContents);
        }
    }

    //==============================================================================
    void drawImage (const Image& image, const AffineTransform& trans)
    {
        if (clip == nullptr || fillType.colour.isTransparent())
            return;

        const AffineTransform t (transform.getTransformWith (trans));
        const float alpha = fillType.colour.getFloatAlpha();

        if (t.isOnlyTranslation())
        {
            int tx = (int) (t.getTranslationX() * 256.0f);
            int ty = (int) (t.getTranslationY() * 256.0f);

            if (((tx | ty) & 0xf8) == 0)
            {
                tx = ((tx + 128) >> 8);
                ty = ((ty + 128) >> 8);

                clip->drawImage (target, image, alpha, Rectangle<int> (tx, ty, image.getWidth(), image.getHeight()));
                return;
            }
        }

        if (t.isSingularity())
            return;

        Path p;
        p.addRectangle (image.getBounds());
        ClipRegion_Mask* m = ClipRegion_Mask::createFromPath (clip->getClipBounds(), p, t);

        if (m != nullptr)
        {
            ClipRegionBase::Ptr c (clip->applyClipTo (m));

            if (c != nullptr)
            {
                m = dynamic_cast<ClipRegion_Mask*> (c.getObject());

                jassert (m != nullptr);
                m->drawImageSelfDestructively (target, image, alpha, t);
            }
        }
    }

    //==============================================================================
    ClipRegionBase::Ptr clip;
    RenderingHelpers::TranslationOrTransform transform;
    Font font;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;

private:
    OpenGLTarget target;
    float transparencyLayerAlpha;
    Image transparencyLayer;

    void cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    FillType getFillType() const
    {
        return fillType.transformed (transform.getTransform());
    }

    SavedState& operator= (const SavedState&);
};


//==============================================================================
OpenGLRenderer::OpenGLRenderer (OpenGLComponent& target)
    : stack (new SavedState (OpenGLTarget (target.getFrameBufferID(), target.getWidth(), target.getHeight())))
{
    target.makeCurrentRenderingTarget();
}

OpenGLRenderer::OpenGLRenderer (OpenGLFrameBuffer& target)
    : stack (new SavedState (OpenGLTarget (target, Point<int>())))
{
    // This object can only be created and used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());
}

OpenGLRenderer::~OpenGLRenderer() {}

bool OpenGLRenderer::isVectorDevice() const                                         { return false; }
void OpenGLRenderer::setOrigin (int x, int y)                                       { stack->transform.setOrigin (x, y); }
void OpenGLRenderer::addTransform (const AffineTransform& t)                        { stack->transform.addTransform (t); }
float OpenGLRenderer::getScaleFactor()                                              { return stack->transform.getScaleFactor(); }
Rectangle<int> OpenGLRenderer::getClipBounds() const                                { return stack->getClipBounds(); }
bool OpenGLRenderer::isClipEmpty() const                                            { return stack->clip == nullptr; }
bool OpenGLRenderer::clipRegionIntersects (const Rectangle<int>& r)                 { return stack->clipRegionIntersects (r); }
bool OpenGLRenderer::clipToRectangle (const Rectangle<int>& r)                      { return stack->clipToRectangle (r); }
bool OpenGLRenderer::clipToRectangleList (const RectangleList& r)                   { return stack->clipToRectangleList (r); }
void OpenGLRenderer::excludeClipRectangle (const Rectangle<int>& r)                 { stack->excludeClipRectangle (r); }
void OpenGLRenderer::clipToPath (const Path& path, const AffineTransform& t)        { stack->clipToPath (path, t); }
void OpenGLRenderer::clipToImageAlpha (const Image& im, const AffineTransform& t)   { stack->clipToImageAlpha (im, t); }
void OpenGLRenderer::saveState()                                                    { stack.save(); }
void OpenGLRenderer::restoreState()                                                 { stack.restore(); }
void OpenGLRenderer::beginTransparencyLayer (float opacity)                         { stack.beginTransparencyLayer (opacity); }
void OpenGLRenderer::endTransparencyLayer()                                         { stack.endTransparencyLayer(); }
void OpenGLRenderer::setFill (const FillType& fillType)                             { stack->fillType = fillType; }
void OpenGLRenderer::setOpacity (float newOpacity)                                  { stack->fillType.setOpacity (newOpacity); }
void OpenGLRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)  { stack->interpolationQuality = quality; }
void OpenGLRenderer::fillRect (const Rectangle<int>& r, bool replace)               { stack->fillRect (r, replace); }
void OpenGLRenderer::fillPath (const Path& path, const AffineTransform& t)          { stack->fillPath (path, t); }
void OpenGLRenderer::drawImage (const Image& im, const AffineTransform& t)          { stack->drawImage (im, t); }
void OpenGLRenderer::drawVerticalLine (int x, float top, float bottom)              { stack->fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top)); }
void OpenGLRenderer::drawHorizontalLine (int y, float left, float right)            { stack->fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f)); }
void OpenGLRenderer::drawGlyph (int glyphNumber, const AffineTransform& t)          { stack->drawGlyph (glyphNumber, t); }
void OpenGLRenderer::drawLine (const Line <float>& line)                            { stack->drawLine (line); }
void OpenGLRenderer::setFont (const Font& newFont)                                  { stack->font = newFont; }
Font OpenGLRenderer::getFont()                                                      { return stack->font; }

END_JUCE_NAMESPACE
