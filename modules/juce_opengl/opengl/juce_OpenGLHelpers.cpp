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

//==============================================================================
void OpenGLHelpers::resetErrorState()
{
    while (glGetError() != GL_NO_ERROR) {}
}

void OpenGLHelpers::clear (const Colour& colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLHelpers::setColour (const Colour& colour)
{
    glColor4f (colour.getFloatRed(), colour.getFloatGreen(),
               colour.getFloatBlue(), colour.getFloatAlpha());
}

void OpenGLHelpers::prepareFor2D (const int width, const int height)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    glOrthof (0.0f, (float) width, 0.0f, (float) height, 0.0f, 1.0f);
   #else
    glOrtho  (0.0, width, 0.0, height, 0, 1);
   #endif

    glViewport (0, 0, width, height);
}

void OpenGLHelpers::setPerspective (double fovy, double aspect, double zNear, double zFar)
{
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    const float ymax = (float) (zNear * tan (fovy * double_Pi / 360.0));
    const float ymin = -ymax;

    glFrustumf (ymin * (float) aspect, ymax * (float) aspect, ymin, ymax, (float) zNear, (float) zFar);
   #else
    const double ymax = zNear * tan (fovy * double_Pi / 360.0);
    const double ymin = -ymax;

    glFrustum  (ymin * aspect, ymax * aspect, ymin, ymax, zNear, zFar);
   #endif
}

void OpenGLHelpers::applyTransform (const AffineTransform& t)
{
    const GLfloat m[] = { t.mat00, t.mat10, 0, 0,
                          t.mat01, t.mat11, 0, 0,
                          0,       0,       1, 0,
                          t.mat02, t.mat12, 0, 1 };
    glMultMatrixf (m);
}

void OpenGLHelpers::drawQuad2D (float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, x2, y2, x4, y4, x3, y3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (2, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLHelpers::drawQuad3D (float x1, float y1, float z1,
                                float x2, float y2, float z2,
                                float x3, float y3, float z3,
                                float x4, float y4, float z4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

//==============================================================================
namespace OpenGLGradientHelpers
{
    void drawTriangles (GLenum mode, const GLfloat* vertices, const GLfloat* textureCoords, const int numElements)
    {
        glEnable (GL_BLEND);
        glEnable (GL_TEXTURE_2D);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays (mode, 0, numElements);
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

        const float l = (float) rect.getX();
        const float r = (float) rect.getRight();
        const float t = (float) rect.getY();
        const float b = (float) rect.getBottom();

        const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]  = { l, t, r, t, l, b, r, b };

        textureTransform.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        textureTransform.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        drawTriangles (GL_TRIANGLE_STRIP, vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform)
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

            const float originalRadius = grad.point1.getDistanceFrom (grad.point2);
            const float texturePos = sourceRadius / originalRadius;

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

        glEnable (GL_SCISSOR_TEST);
        glScissor (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
        drawTriangles (GL_TRIANGLE_FAN, vertices, textureCoords, numDivisions + 2);
        glDisable (GL_SCISSOR_TEST);
    }
}

void OpenGLHelpers::fillRectWithColourGradient (const Rectangle<int>& rect,
                                                const ColourGradient& gradient,
                                                const AffineTransform& transform)
{
    const int textureSize = 256;
    OpenGLTexture texture;

    HeapBlock<PixelARGB> lookup (textureSize);
    gradient.createLookupTable (lookup, textureSize);
    texture.load (lookup, textureSize, 1);
    texture.bind();

    if (gradient.point1 == gradient.point2)
    {
        fillRectWithColour (rect, gradient.getColourAtPosition (1.0));
    }
    else
    {
        if (gradient.isRadial)
            OpenGLGradientHelpers::fillWithRadialGradient (rect, gradient, transform);
        else
            OpenGLGradientHelpers::fillWithLinearGradient (rect, gradient, transform, textureSize);
    }
}

void OpenGLHelpers::fillRectWithColour (const Rectangle<int>& rect, const Colour& colour)
{
    glEnableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    setColour (colour);
    fillRect (rect);
}

void OpenGLHelpers::fillRect (const Rectangle<int>& rect)
{
    const GLfloat vertices[] = { (float) rect.getX(),     (float) rect.getY(),
                                 (float) rect.getRight(), (float) rect.getY(),
                                 (float) rect.getX(),     (float) rect.getBottom(),
                                 (float) rect.getRight(), (float) rect.getBottom() };

    glVertexPointer (2, GL_FLOAT, 0, vertices);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

//==============================================================================
// This breaks down a path into a series of horizontal strips of trapezoids..
class TriangulatedPath::TrapezoidedPath
{
public:
    TrapezoidedPath (const Path& p, const AffineTransform& transform)
        : firstSlice (nullptr),
          windingMask (p.isUsingNonZeroWinding() ? -1 : 1)
    {
        for (PathFlatteningIterator iter (p, transform); iter.next();)
            addLine (floatToInt (iter.x1), floatToInt (iter.y1),
                     floatToInt (iter.x2), floatToInt (iter.y2));
    }

    ~TrapezoidedPath()
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr;)
        {
            const ScopedPointer<HorizontalSlice> deleter (s);
            s = s->next;
        }
    }

    template <class Consumer>
    void iterate (Consumer& consumer) const
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr; s = s->next)
            s->iterate (consumer, windingMask);
    }

private:
    void addLine (int x1, int y1, int x2, int y2)
    {
        int winding = 1;

        if (y2 < y1)
        {
            std::swap (x1, x2);
            std::swap (y1, y2);
            winding = -1;
        }

        HorizontalSlice* last = nullptr;
        HorizontalSlice* s = firstSlice;

        while (y2 > y1)
        {
            if (s == nullptr)
            {
                insert (last, new HorizontalSlice (nullptr, x1, y1, x2, y2, winding));
                break;
            }

            if (s->y2 > y1)
            {
                if (y1 < s->y1)
                {
                    if (y2 <= s->y1)
                    {
                        insert (last, new HorizontalSlice (s, x1, y1, x2, y2, winding));
                        break;
                    }
                    else
                    {
                        const int newX = x1 + (s->y1 - y1) * (x2 - x1) / (y2 - y1);
                        HorizontalSlice* const newSlice = new HorizontalSlice (s, x1, y1, newX, s->y1, winding);
                        insert (last, newSlice);
                        last = newSlice;
                        x1 = newX;
                        y1 = s->y1;
                        continue;
                    }
                }
                else if (y1 > s->y1)
                {
                    s->split (y1);
                    s = s->next;
                    jassert (s != nullptr);
                }

                jassert (y1 == s->y1);

                if (y2 > s->y2)
                {
                    const int newY = s->y2;
                    const int newX = x1 + (newY - y1) * (x2 - x1) / (y2 - y1);
                    s->addLine (x1, newX, winding);
                    x1 = newX;
                    y1 = newY;
                }
                else
                {
                    if (y2 < s->y2)
                        s->split (y2);

                    jassert (y2 == s->y2);
                    s->addLine (x1, x2, winding);
                    break;
                }
            }

            last = s;
            s = s->next;
        }
    }

    struct HorizontalSlice
    {
        HorizontalSlice (const HorizontalSlice& other, HorizontalSlice* const next_, int y1_, int y2_)
            : next (next_), y1 (y1_), y2 (y2_), segments (other.segments)
        {
        }

        HorizontalSlice (HorizontalSlice* const next_, int x1, int y1_, int x2, int y2_, int winding)
            : next (next_), y1 (y1_), y2 (y2_)
        {
            jassert (next != this);
            jassert (y2 > y1);
            segments.ensureStorageAllocated (32);
            segments.add (LineSegment (x1, x2, winding));
        }

        void addLine (const int x1, const int x2, int winding)
        {
            const int dy = y2 - y1;

            for (int i = 0; i < segments.size(); ++i)
            {
                const LineSegment& l = segments.getReference (i);

                const int diff1 = l.x1 - x1;
                const int diff2 = l.x2 - x2;

                if ((diff1 < 0) == (diff2 > 0))
                {
                    const int dx1 = l.x2 - l.x1;
                    const int dx2 = x2 - x1;
                    const int dxDiff = dx2 - dx1;

                    if (dxDiff != 0)
                    {
                        const int intersectionY = (dy * diff1) / dxDiff;

                        if (intersectionY > 0 && intersectionY < dy)
                        {
                            const int intersectionX = x1 + (intersectionY * dx2) / dy;
                            split (intersectionY + y1);
                            next->addLine (intersectionX, x2, winding);
                            addLine (x1, intersectionX, winding);
                            return;
                        }
                    }
                }

                if (diff1 + diff2 > 0)
                {
                    segments.insert (i, LineSegment (x1, x2, winding));
                    return;
                }
            }

            segments.add (LineSegment (x1, x2, winding));
        }

        void split (const int newY)
        {
            jassert (newY > y1 && newY < y2);

            const int dy1 = newY - y1;
            const int dy2 = y2 - y1;
            next = new HorizontalSlice (*this, next, newY, y2);
            y2 = newY;

            LineSegment* const oldSegments = segments.getRawDataPointer();
            LineSegment* const newSegments = next->segments.getRawDataPointer();

            for (int i = 0; i < segments.size(); ++i)
            {
                LineSegment& l = oldSegments[i];
                const int newX = l.x1 + dy1 * (l.x2 - l.x1) / dy2;
                newSegments[i].x1 = newX;
                l.x2 = newX;
            }
        }

        template <class Consumer>
        void iterate (Consumer& consumer, const int windingMask)
        {
            jassert (segments.size() > 0);

            const float fy1 = intToFloat (y1);
            const float fy2 = intToFloat (y2);

            const LineSegment* s1 = segments.getRawDataPointer();
            const LineSegment* s2 = s1;
            int winding = s1->winding;

            for (int i = segments.size(); --i > 0;)
            {
                ++s2;
                winding += s2->winding;

                if ((winding & windingMask) == 0)
                {
                    const float ax1 = intToFloat (s1->x1);
                    const float ax2 = intToFloat (s1->x2);

                    if (s1->x1 == s2->x1)
                        consumer.addTriangle (ax1, fy1, ax2, fy2, intToFloat (s2->x2), fy2);
                    else if (s1->x2 == s2->x2)
                        consumer.addTriangle (ax1, fy1, intToFloat (s2->x1), fy1, ax2, fy2);
                    else
                        consumer.addTrapezoid (fy1, fy2, ax1, ax2, intToFloat (s2->x1), intToFloat (s2->x2));

                    s1 = s2 + 1;
                }
            }
        }

        HorizontalSlice* next;
        int y1, y2;

    private:
        struct LineSegment
        {
            inline LineSegment (int x1_, int x2_, int winding_) noexcept
                : x1 (x1_), x2 (x2_), winding (winding_) {}

            int x1, x2;
            int winding;
        };

        Array<LineSegment> segments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HorizontalSlice);
    };

    HorizontalSlice* firstSlice;
    const int windingMask;

    inline void insert (HorizontalSlice* const last, HorizontalSlice* const newOne) noexcept
    {
        if (last == nullptr)
            firstSlice = newOne;
        else
            last->next = newOne;
    }

    enum { factor = 128 };
    static inline int floatToInt (const float n) noexcept     { return roundToInt (n * (float) factor); }
    static inline float intToFloat (const int n) noexcept     { return n * (1.0f / (float) factor); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrapezoidedPath);
};

//==============================================================================
struct TriangulatedPath::TriangleBlock
{
    TriangleBlock() noexcept
        : numVertices (0),
          triangles (maxVerticesPerBlock)
    {}

    void draw() const
    {
        glVertexPointer (2, GL_FLOAT, 0, triangles);
        glDrawArrays (GL_TRIANGLES, 0, numVertices / 2);
    }

    inline GLfloat* getNextTriangle() noexcept  { return triangles + numVertices; }
    void optimiseStorage()                      { triangles.realloc (numVertices); }

    // Some GL implementations can't take very large triangle lists, so store
    // the list as a series of blocks containing this max number of triangles.
    enum { maxVerticesPerBlock = 256 * 6 };

    unsigned int numVertices;
    HeapBlock<GLfloat> triangles;
};

TriangulatedPath::TriangulatedPath (const Path& path, const AffineTransform& transform)
{
    startNewBlock();
    TrapezoidedPath (path, transform).iterate (*this);
}

void TriangulatedPath::draw (const int oversamplingLevel) const
{
    glColor4f (1.0f, 1.0f, 1.0f, 1.0f / (oversamplingLevel * oversamplingLevel));

    glTranslatef (-0.5f, -0.5f, 0.0f);
    const float inc = 1.0f / oversamplingLevel;

    for (int y = oversamplingLevel; --y >= 0;)
    {
        for (int x = oversamplingLevel; --x >= 0;)
        {
            glTranslatef (inc, 0.0f, 0.0f);

            for (int i = 0; i < blocks.size(); ++i)
                blocks.getUnchecked(i)->draw();
        }

        glTranslatef (-1.0f, inc, 0.0f);
    }
}

void TriangulatedPath::optimiseStorage()
{
    currentBlock->optimiseStorage();
}

void TriangulatedPath::startNewBlock()
{
    currentBlock = new TriangleBlock();
    blocks.add (currentBlock);
}

void TriangulatedPath::addTriangle (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
{
    if (currentBlock->numVertices >= TriangleBlock::maxVerticesPerBlock)
        startNewBlock();

    GLfloat* t = currentBlock->getNextTriangle();
    *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y3;

    currentBlock->numVertices += 6;
}

void TriangulatedPath::addTrapezoid (GLfloat y1, GLfloat y2, GLfloat x1, GLfloat x2, GLfloat x3, GLfloat x4)
{
    if (currentBlock->numVertices >= TriangleBlock::maxVerticesPerBlock - 6)
        startNewBlock();

    GLfloat* t = currentBlock->getNextTriangle();
    *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;
    *t++ = x4; *t++ = y2; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;

    currentBlock->numVertices += 12;
}

//==============================================================================
OpenGLTextureFromImage::OpenGLTextureFromImage (const Image& image)
    : width (image.getWidth()),
      height (image.getHeight())
{
    OpenGLFrameBufferImage* glImage = dynamic_cast <OpenGLFrameBufferImage*> (image.getSharedImage());

    if (glImage != nullptr)
    {
        textureID = glImage->frameBuffer.getTextureID();
    }
    else
    {
        if (OpenGLTexture::isValidSize (width, height))
        {
            texture = new OpenGLTexture();
            texture->load (image);
            textureID = texture->getTextureID();
        }
        else
        {
            frameBuffer = new OpenGLFrameBuffer();
            frameBuffer->initialise (image);
            textureID = frameBuffer->getTextureID();
        }
    }
}

OpenGLTextureFromImage::~OpenGLTextureFromImage() {}

//==============================================================================
OpenGLRenderingTarget::OpenGLRenderingTarget() {}
OpenGLRenderingTarget::~OpenGLRenderingTarget() {}

void OpenGLRenderingTarget::prepareFor2D()
{
    OpenGLHelpers::prepareFor2D (getRenderingTargetWidth(),
                                 getRenderingTargetHeight());
}

namespace GLPathRendering
{
    void clipToPath (OpenGLRenderingTarget& target,
                     const Path& path, const AffineTransform& transform)
    {
        const int w = target.getRenderingTargetWidth();
        const int h = target.getRenderingTargetHeight();

        OpenGLFrameBuffer fb;
        fb.initialise (w, h);
        fb.makeCurrentAndClear();
        fb.createAlphaChannelFromPath (path, transform);

        target.makeCurrentRenderingTarget();
        target.prepareFor2D();

        glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
        glBlendFunc (GL_DST_ALPHA, GL_ZERO);

        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        fb.drawAt (0, 0);
    }

    void fillPathWithColour (OpenGLRenderingTarget& target,
                             const Rectangle<int>& clip, const Path& path,
                             const AffineTransform& pathTransform,
                             const Colour& colour)
    {
        OpenGLFrameBuffer f;
        f.initialise (clip.getWidth(), clip.getHeight());
        f.makeCurrentAndClear();

        f.createAlphaChannelFromPath (path, pathTransform.translated ((float) -clip.getX(), (float) -clip.getY())
                                                         .followedBy (AffineTransform::verticalFlip ((float) clip.getHeight())));
        f.releaseAsRenderingTarget();

        target.makeCurrentRenderingTarget();

        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        OpenGLHelpers::setColour (colour);
        target.prepareFor2D();

        f.drawAt ((float) clip.getX(), (float) (target.getRenderingTargetHeight() - clip.getBottom()));
    }

    void fillPathWithGradient (OpenGLRenderingTarget& target,
                               const Rectangle<int>& clip, const Path& path,
                               const AffineTransform& pathTransform,
                               const ColourGradient& grad,
                               const AffineTransform& gradientTransform,
                               const GLfloat alpha)
    {
        const int targetHeight = target.getRenderingTargetHeight();

        OpenGLFrameBuffer f;
        f.initialise (clip.getWidth(), clip.getHeight());
        f.makeCurrentAndClear();

        const AffineTransform correction (AffineTransform::translation ((float) -clip.getX(), (float) -clip.getY())
                                              .followedBy (AffineTransform::verticalFlip ((float) clip.getHeight())));

        f.createAlphaChannelFromPath (path, pathTransform.followedBy (correction));

        f.makeCurrentRenderingTarget();
        f.prepareFor2D();

        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendFunc (GL_DST_ALPHA, GL_ZERO);

        OpenGLHelpers::fillRectWithColourGradient (Rectangle<int> (0, 0, clip.getWidth(), clip.getHeight()),
                                                   grad, gradientTransform.followedBy (correction));
        f.releaseAsRenderingTarget();
        target.makeCurrentRenderingTarget();

        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f (alpha, alpha, alpha, alpha);
        target.prepareFor2D();

        f.drawAt ((float) clip.getX(), (float) (targetHeight - clip.getBottom()));
    }

    void fillPathWithImage (OpenGLRenderingTarget& target,
                            const Rectangle<int>& clip, const Path& path,
                            const AffineTransform& transform,
                            GLuint textureID, GLfloat textureWidth, GLfloat textureHeight,
                            const AffineTransform& textureTransform,
                            const bool tiled,
                            const GLfloat alpha)
    {
        const int targetHeight = target.getRenderingTargetHeight();

        OpenGLFrameBuffer f;
        f.initialise (clip.getWidth(), clip.getHeight());
        f.makeCurrentRenderingTarget();
        f.prepareFor2D();

        glDisable (GL_BLEND);
        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

        const GLfloat clipX = (GLfloat) clip.getX();
        const GLfloat clipY = (GLfloat) clip.getY();
        const GLfloat clipH = (GLfloat) clip.getHeight();
        const GLfloat clipB = (GLfloat) clip.getBottom();

        const AffineTransform correction (AffineTransform::translation (-clipX, -clipY)
                                            .followedBy (AffineTransform::verticalFlip (clipH)));

        glBindTexture (GL_TEXTURE_2D, textureID);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);

        if (tiled)
        {
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            const GLfloat clipW = (GLfloat) clip.getWidth();
            const GLfloat clipR = (GLfloat) clip.getRight();

            const GLfloat vertices[]  = { 0, clipH, clipW, clipH, 0, 0, clipW, 0 };
            GLfloat textureCoords[]   = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };

            {
                const AffineTransform t (textureTransform.inverted().scaled (1.0f / textureWidth,
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
            glClearColor (0, 0, 0, 0);
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            GLfloat vertices[] = { 0, 0, textureWidth, 0, 0, textureHeight, textureWidth, textureHeight };
            const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

            {
                const AffineTransform t (textureTransform.followedBy (correction));
                t.transformPoints (vertices[0], vertices[1], vertices[2], vertices[3]);
                t.transformPoints (vertices[4], vertices[5], vertices[6], vertices[7]);
            }

            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }

        glBindTexture (GL_TEXTURE_2D, 0);

        clipToPath (f, path, transform.followedBy (correction));

        f.releaseAsRenderingTarget();
        target.makeCurrentRenderingTarget();

        glColorMask (GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f (1.0f, 1.0f, 1.0f, alpha);
        target.prepareFor2D();

        f.drawAt (clipX, targetHeight - clipB);
    }
}

void OpenGLRenderingTarget::fillPath (const Rectangle<int>& clip,
                                      const Path& path, const AffineTransform& transform,
                                      const FillType& fill)
{
    if (! fill.isInvisible())
    {
        if (fill.isColour())
        {
            GLPathRendering::fillPathWithColour (*this, clip, path, transform, fill.colour);
        }
        else if (fill.isGradient())
        {
            GLPathRendering::fillPathWithGradient (*this, clip, path, transform,
                                                   *(fill.gradient), fill.transform,
                                                   fill.colour.getFloatAlpha());
        }
        else if (fill.isTiledImage())
        {
            OpenGLTextureFromImage t (fill.image);

            GLPathRendering::fillPathWithImage (*this, clip, path, transform,
                                                t.textureID, (GLfloat) t.width, (GLfloat) t.height,
                                                fill.transform, true,
                                                fill.colour.getFloatAlpha());
        }
    }
}

END_JUCE_NAMESPACE
