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
   #if JUCE_WINDOWS
    enum
    {
        GL_OPERAND0_RGB = 0x8590,
        GL_OPERAND1_RGB = 0x8591,
        GL_OPERAND0_ALPHA = 0x8598,
        GL_OPERAND1_ALPHA = 0x8599,
        GL_SRC0_RGB = 0x8580,
        GL_SRC1_RGB = 0x8581,
        GL_SRC0_ALPHA = 0x8588,
        GL_SRC1_ALPHA = 0x8589,
        GL_TEXTURE0 = 0x84C0,
        GL_TEXTURE1 = 0x84C1,
        GL_TEXTURE2 = 0x84C2,
        GL_COMBINE = 0x8570,
        GL_COMBINE_RGB = 0x8571,
        GL_COMBINE_ALPHA = 0x8572,
        GL_PREVIOUS = 0x8578,
    };
   #endif

   #if JUCE_WINDOWS || JUCE_LINUX
    JUCE_DECLARE_GL_EXTENSION_FUNCTION (glActiveTexture, void, (GLenum));
    JUCE_DECLARE_GL_EXTENSION_FUNCTION (glClientActiveTexture, void, (GLenum));

    void initialiseMultiTextureExtensions()
    {
        if (glActiveTexture == nullptr)
        {
            JUCE_INSTANTIATE_GL_EXTENSION (glActiveTexture);
            JUCE_INSTANTIATE_GL_EXTENSION (glClientActiveTexture);
        }
    }
   #else
    void initialiseMultiTextureExtensions() {}
   #endif
}

//==============================================================================
struct OpenGLTarget
{
    OpenGLTarget (GLuint frameBufferID_, int width_, int height_) noexcept
        : frameBuffer (nullptr), frameBufferID (frameBufferID_),
          x (0), y (0), width (width_), height (height_)
    {
    }

    OpenGLTarget (OpenGLFrameBuffer& frameBuffer_, const Point<int>& origin) noexcept
        : frameBuffer (&frameBuffer_), frameBufferID (0), x (origin.x), y (origin.y),
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

    static void applyFlippedMatrix (const int x, const int y, const int width, const int height)
    {
        glMatrixMode (GL_PROJECTION);
        glLoadIdentity();

       #if JUCE_OPENGL_ES
        glOrthof ((GLfloat) x, (GLfloat) (x + width), (GLfloat) (y + height), (GLfloat) y, 0.0f, 1.0f);
       #else
        glOrtho (x, x + width, y + height, y, 0, 1);
       #endif

        glViewport (0, 0, width, height);
    }

    OpenGLFrameBuffer* frameBuffer;
    GLuint frameBufferID;
    int x, y, width, height;
};

//==============================================================================
class PositionedTexture
{
public:
    PositionedTexture (OpenGLTexture& texture, EdgeTable& et, const Rectangle<int>& clip_)
    {
        et.clipToRectangle (clip_);

        EdgeTableData data (et);

        texture.loadAlpha (data.data, data.area.getWidth(), data.area.getHeight());
        textureID = texture.getTextureID();

        clip = et.getMaximumBounds();
        area = data.area;
    }

    PositionedTexture (GLuint textureID_, const Rectangle<int> area_, const Rectangle<int> clip_)
        : textureID (textureID_), area (area_), clip (clip_)
    {
    }

    template <typename ValueType>
    void getTextureCoordAt (ValueType x, ValueType y, GLfloat& resultX, GLfloat& resultY) const noexcept
    {
        resultX = (x - area.getX()) / (float) area.getWidth();
        resultY = (area.getBottom() - y) / (float) area.getHeight();
    }

    void enable (GLenum multitextureIndex, const GLfloat* textureCoords) const
    {
        glActiveTexture (multitextureIndex);
        glClientActiveTexture (multitextureIndex);
        glBindTexture (GL_TEXTURE_2D, textureID);
        glEnable (GL_TEXTURE_2D);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
    }

    void enable (GLenum multitextureIndex, const Rectangle<int>* const area, GLfloat* const textureCoords) const noexcept
    {
        if (area != nullptr)
        {
            getTextureCoordAt (area->getX(),     area->getY(),      textureCoords[0], textureCoords[1]);
            getTextureCoordAt (area->getRight(), area->getY(),      textureCoords[2], textureCoords[3]);
            getTextureCoordAt (area->getX(),     area->getBottom(), textureCoords[4], textureCoords[5]);
            getTextureCoordAt (area->getRight(), area->getBottom(), textureCoords[6], textureCoords[7]);
        }

        enable (multitextureIndex, textureCoords);
    }

    GLuint textureID;
    Rectangle<int> area, clip;

private:
    struct EdgeTableData
    {
        EdgeTableData (const EdgeTable& et)
            : area (et.getMaximumBounds().withSize (nextPowerOfTwo (et.getMaximumBounds().getWidth()),
                                                    nextPowerOfTwo (et.getMaximumBounds().getHeight())))
        {
            data.calloc (area.getWidth() * area.getHeight());
            et.iterate (*this);
        }

        inline void setEdgeTableYPos (const int y) noexcept
        {
            currentLine = data + (area.getBottom() - 1 - y) * area.getWidth() - area.getX();
        }

        inline void handleEdgeTablePixel (const int x, const int alphaLevel) const noexcept
        {
            currentLine[x] = (uint8) alphaLevel;
        }

        inline void handleEdgeTablePixelFull (const int x) const noexcept
        {
            currentLine[x] = 255;
        }

        inline void handleEdgeTableLine (int x, int width, const int alphaLevel) const noexcept
        {
            memset (currentLine + x, (uint8) alphaLevel, width);
        }

        inline void handleEdgeTableLineFull (int x, int width) const noexcept
        {
            memset (currentLine + x, 255, width);
        }

        HeapBlock<uint8> data;
        const Rectangle<int> area;

    private:
        uint8* currentLine;

        JUCE_DECLARE_NON_COPYABLE (EdgeTableData);
    };
};

//==============================================================================
class GradientTexture
{
public:
    GradientTexture() : needsRefresh (true) {}

    enum { textureSize = 256 };

    void reset() noexcept
    {
        needsRefresh = true;
    }

    void bind (const ColourGradient& gradient)
    {
        if (needsRefresh)
        {
            needsRefresh = false;
            PixelARGB lookup [textureSize];
            gradient.createLookupTable (lookup, textureSize);
            texture.loadARGB (lookup, textureSize, 1);
        }

        texture.bind();
    }

private:
    OpenGLTexture texture;
    bool needsRefresh;
};

//==============================================================================
namespace
{
    struct TargetSaver
    {
        TargetSaver()
            : oldFramebuffer (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
        {
            glGetIntegerv (GL_VIEWPORT, oldViewport);
            glPushMatrix();
        }

        ~TargetSaver()
        {
            OpenGLFrameBuffer::setCurrentFrameBufferTarget (oldFramebuffer);
            glPopMatrix();
            glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
        }

    private:
        GLuint oldFramebuffer;
        GLint oldViewport[4];
    };

    struct TemporaryColourModulationMode
    {
        TemporaryColourModulationMode()    { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR); }
        ~TemporaryColourModulationMode()   { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA); }
    };

    void fillRectangleList (const RectangleList& list)
    {
        GLfloat vertices [8];
        glEnableClientState (GL_VERTEX_ARRAY);
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

    void fillRectangleList (const RectangleList& list, const Rectangle<int>& clip)
    {
        GLfloat vertices [8];
        glEnableClientState (GL_VERTEX_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);

        for (RectangleList::Iterator i (list); i.next();)
        {
            const Rectangle<int> r (i.getRectangle()->getIntersection (clip));
            vertices[0] = vertices[4] = (GLfloat) r.getX();
            vertices[1] = vertices[3] = (GLfloat) r.getY();
            vertices[2] = vertices[6] = (GLfloat) r.getRight();
            vertices[5] = vertices[7] = (GLfloat) r.getBottom();

            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    inline void setColour (const float alpha) noexcept
    {
        glColor4f (alpha, alpha, alpha, alpha);
    }

    inline void setPremultipliedColour (const Colour& c) noexcept
    {
        const PixelARGB p (c.getPixelARGB());
        OpenGLHelpers::setColour (Colour (p.getARGB()));
    }

    void disableMultiTexture (GLenum level)
    {
        glActiveTexture (level);
        glDisable (GL_TEXTURE_2D);
    }

    void disableMultiTexture()
    {
        disableMultiTexture (GL_TEXTURE2);
        disableMultiTexture (GL_TEXTURE1);
        disableMultiTexture (GL_TEXTURE0);
    }

    void enableSingleTexture()
    {
        disableMultiTexture (GL_TEXTURE2);
        disableMultiTexture (GL_TEXTURE1);
        glActiveTexture (GL_TEXTURE0);
        glClientActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
    }

    void resetMultiTextureMode (GLenum index, const bool forRGBTextures)
    {
        glActiveTexture (index);
        glClientActiveTexture (index);
        glDisable (GL_TEXTURE_2D);
        glTexEnvi (GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
        glTexEnvi (GL_TEXTURE_ENV, GL_SRC0_RGB, GL_PREVIOUS);
        glTexEnvi (GL_TEXTURE_ENV, GL_SRC1_RGB, GL_TEXTURE);
        glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
        glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, forRGBTextures ? GL_SRC_COLOR : GL_SRC_ALPHA);
        glTexEnvi (GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
        glTexEnvi (GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_PREVIOUS);
        glTexEnvi (GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
        glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
        glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void resetMultiTextureModes (const bool forRGBTextures)
    {
        resetMultiTextureMode (GL_TEXTURE2, forRGBTextures);
        resetMultiTextureMode (GL_TEXTURE1, forRGBTextures);
        resetMultiTextureMode (GL_TEXTURE0, forRGBTextures);
    }

    void setPremultipliedBlendingMode() noexcept
    {
        glEnable (GL_BLEND);
        glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    void setBlendMode (const bool replaceExistingContents) noexcept
    {
        if (replaceExistingContents)
            glDisable (GL_BLEND);
        else
            setPremultipliedBlendingMode();
    }

    void prepareMasks (const PositionedTexture* const mask1, const PositionedTexture* const mask2,
                       GLfloat* const textureCoords1, GLfloat* const textureCoords2, const Rectangle<int>* const area)
    {
        if (mask1 != nullptr)
        {
            mask1->enable (GL_TEXTURE0, area, textureCoords1);

            if (mask2 != nullptr)
            {
                mask2->enable (GL_TEXTURE1, area, textureCoords2);

                glActiveTexture (GL_TEXTURE2);
                glClientActiveTexture (GL_TEXTURE2);
            }
            else
            {
                disableMultiTexture (GL_TEXTURE2);
                glActiveTexture (GL_TEXTURE1);
                glClientActiveTexture (GL_TEXTURE1);
            }
        }
        else
        {
            disableMultiTexture (GL_TEXTURE2);
            disableMultiTexture (GL_TEXTURE1);
            glActiveTexture (GL_TEXTURE0);
            glClientActiveTexture (GL_TEXTURE0);
        }
    }

    void renderImage (const OpenGLTarget& target, const OpenGLTextureFromImage& image,
                      const Rectangle<int>& clip, const AffineTransform& transform, float alpha,
                      const PositionedTexture* mask1, const PositionedTexture* mask2,
                      const bool replaceExistingContents, const bool isTiled)
    {
        setBlendMode (replaceExistingContents);
        GLfloat textureCoords1[8], textureCoords2[8];

        if ((! isTiled) || (isPowerOfTwo (image.imageWidth) && isPowerOfTwo (image.imageHeight)))
        {
            prepareMasks (mask1, mask2, textureCoords1, textureCoords2, &clip);

            glEnable (GL_TEXTURE_2D);
            glBindTexture (GL_TEXTURE_2D, image.textureID);
            glEnableClientState (GL_VERTEX_ARRAY);
            glEnableClientState (GL_TEXTURE_COORD_ARRAY);
            TemporaryColourModulationMode tmm;
            setColour (alpha);

            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, isTiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);
            glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, isTiled ? GL_REPEAT : GL_CLAMP_TO_EDGE);

            const GLfloat clipX = (GLfloat) clip.getX();
            const GLfloat clipY = (GLfloat) clip.getY();
            const GLfloat clipR = (GLfloat) clip.getRight();
            const GLfloat clipB = (GLfloat) clip.getBottom();

            const GLfloat vertices[]  = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };
            GLfloat textureCoords[]   = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };

            {
                const AffineTransform t (transform.inverted().scaled (image.fullWidthProportion / image.imageWidth,
                                                                      image.fullHeightProportion / image.imageHeight));
                t.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
                t.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

                textureCoords[1] = 1.0f - textureCoords[1];
                textureCoords[3] = 1.0f - textureCoords[3];
                textureCoords[5] = 1.0f - textureCoords[5];
                textureCoords[7] = 1.0f - textureCoords[7];
            }

            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }
        else
        {
            prepareMasks (mask1, mask2, textureCoords1, textureCoords2, nullptr);

            glEnable (GL_TEXTURE_2D);
            glBindTexture (GL_TEXTURE_2D, image.textureID);
            glEnableClientState (GL_VERTEX_ARRAY);
            glEnableClientState (GL_TEXTURE_COORD_ARRAY);
            TemporaryColourModulationMode tmm;
            setColour (alpha);

            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            target.scissor (clip);
            glPushMatrix();
            OpenGLHelpers::applyTransform (transform);

            GLfloat vertices[8];
            const GLfloat textureCoords[] = { 0, 1.0f, image.fullWidthProportion, 1.0f,
                                              0, 1.0f - image.fullHeightProportion, image.fullWidthProportion, 1.0f - image.fullHeightProportion };
            glVertexPointer (2, GL_FLOAT, 0, vertices);
            glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

            const Rectangle<int> targetArea (clip.toFloat().transformed (transform.inverted()).getSmallestIntegerContainer());
            int x = targetArea.getX() - negativeAwareModulo (targetArea.getX(), image.imageWidth);
            int y = targetArea.getY() - negativeAwareModulo (targetArea.getY(), image.imageHeight);
            const int right  = targetArea.getRight();
            const int bottom = targetArea.getBottom();

            while (y < bottom)
            {
                vertices[1] = vertices[3] = (GLfloat) y;
                vertices[5] = vertices[7] = (GLfloat) (y + image.imageHeight);

                for (int x1 = x; x1 < right; x1 += image.imageWidth)
                {
                    vertices[0] = vertices[4] = (GLfloat) x1;
                    vertices[2] = vertices[6] = (GLfloat) (x1 + image.imageWidth);

                    if (mask1 != nullptr)
                    {
                        float t[] = { vertices[0], vertices[1], vertices[2], vertices[3],
                                      vertices[4], vertices[5], vertices[6], vertices[7] };
                        transform.transformPoints (t[0], t[1], t[2], t[3]);
                        transform.transformPoints (t[4], t[5], t[6], t[7]);

                        mask1->getTextureCoordAt (t[0], t[1], textureCoords1[0], textureCoords1[1]);
                        mask1->getTextureCoordAt (t[2], t[3], textureCoords1[2], textureCoords1[3]);
                        mask1->getTextureCoordAt (t[4], t[5], textureCoords1[4], textureCoords1[5]);
                        mask1->getTextureCoordAt (t[6], t[7], textureCoords1[6], textureCoords1[7]);

                        if (mask2 != nullptr)
                        {
                            mask2->getTextureCoordAt (t[0], t[1], textureCoords2[0], textureCoords2[1]);
                            mask2->getTextureCoordAt (t[2], t[3], textureCoords2[2], textureCoords2[3]);
                            mask2->getTextureCoordAt (t[4], t[5], textureCoords2[4], textureCoords2[5]);
                            mask2->getTextureCoordAt (t[6], t[7], textureCoords2[6], textureCoords2[7]);
                        }
                    }

                    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
                }

                y += image.imageHeight;
            }

            glPopMatrix();
            glDisable (GL_SCISSOR_TEST);
        }
    }

    void fillWithLinearGradient (GradientTexture& gradientTexture, const Rectangle<int>& rect, const ColourGradient& grad,
                                 const AffineTransform& transform, const PositionedTexture* mask1, const PositionedTexture* mask2)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.x - (grad.point2.y - grad.point1.y) / gradientTexture.textureSize,
                                             grad.point1.y + (grad.point2.x - grad.point1.x) / gradientTexture.textureSize).transformedBy (transform));

        const AffineTransform textureTransform (AffineTransform::fromTargetPoints (p1.x, p1.y,  0.0f, 0.0f,
                                                                                   p2.x, p2.y,  1.0f, 0.0f,
                                                                                   p3.x, p3.y,  0.0f, 1.0f));

        const GLfloat l = (GLfloat) rect.getX();
        const GLfloat r = (GLfloat) rect.getRight();
        const GLfloat t = (GLfloat) rect.getY();
        const GLfloat b = (GLfloat) rect.getBottom();

        const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]  = { l, t, r, t, l, b, r, b };

        textureTransform.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        textureTransform.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        GLfloat textureCoords1[8], textureCoords2[8];
        prepareMasks (mask1, mask2, textureCoords1, textureCoords2, &rect);
        TemporaryColourModulationMode tmm;

        gradientTexture.bind (grad);

        setColour (1.0f);
        OpenGLHelpers::drawTriangleStrip (vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (GradientTexture& gradientTexture, const OpenGLTarget& target, const Rectangle<int>& rect,
                                 const ColourGradient& grad, const AffineTransform& transform,
                                 const PositionedTexture* mask1, const PositionedTexture* mask2)
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
        GLfloat vertices       [4 + numDivisions * 2];
        GLfloat textureCoords1 [4 + numDivisions * 2];
        GLfloat textureCoords2 [4 + numDivisions * 2];
        GLfloat textureCoords3 [4 + numDivisions * 2];

        {
            GLfloat* t = textureCoords1;
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
            *v++ = centre.x;
            *v++ = centre.y;

            const Point<float> first (grad.point1.translated (0, -sourceRadius)
                                                 .transformedBy (transform));
            *v++ = first.x;
            *v++ = first.y;

            for (int i = 1; i < numDivisions; ++i)
            {
                const float angle = i * (float_Pi * 2.0f / numDivisions);
                const Point<float> p (grad.point1.translated (std::sin (angle) * sourceRadius,
                                                              std::cos (angle) * -sourceRadius)
                                                 .transformedBy (transform));
                *v++ = p.x;
                *v++ = p.y;
            }

            *v++ = first.x;
            *v++ = first.y;
        }

        prepareMasks (mask1, mask2, textureCoords2, textureCoords3, nullptr);

        if (mask1 != nullptr)
        {
            for (int i = 0; i < 2 * (numDivisions + 2); i += 2)
                mask1->getTextureCoordAt (vertices[i], vertices[i + 1], textureCoords2[i], textureCoords2[i + 1]);

            if (mask2 != nullptr)
                for (int i = 0; i < 2 * (numDivisions + 2); i += 2)
                    mask2->getTextureCoordAt (vertices[i], vertices[i + 1], textureCoords3[i], textureCoords3[i + 1]);
        }

        gradientTexture.bind (grad);

        target.scissor (rect);
        glEnable (GL_TEXTURE_2D);
        TemporaryColourModulationMode tmm;
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords1);
        setColour (1.0f);
        glDrawArrays (GL_TRIANGLE_FAN, 0, numDivisions + 2);
        glDisable (GL_SCISSOR_TEST);
    }

    void fillTexture (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, GradientTexture& gradientTexture,
                      const PositionedTexture* mask1, const PositionedTexture* mask2, const bool replaceExistingContents)
    {
        jassert (! (mask1 == nullptr && mask2 != nullptr));

        if (fill.isColour())
        {
            GLfloat textureCoords1[8], textureCoords2[8];
            disableMultiTexture (GL_TEXTURE2);

            if (mask1 != nullptr)
            {
                setBlendMode (replaceExistingContents);
                mask1->enable (GL_TEXTURE0, &area, textureCoords1);

                if (mask2 != nullptr)
                    mask2->enable (GL_TEXTURE1, &area, textureCoords2);
                else
                    disableMultiTexture (GL_TEXTURE1);
            }
            else
            {
                setBlendMode (replaceExistingContents || fill.colour.isOpaque());
                disableMultiTexture (GL_TEXTURE1);
                disableMultiTexture (GL_TEXTURE0);
            }

            setPremultipliedColour (fill.colour);
            glEnableClientState (GL_VERTEX_ARRAY);
            OpenGLHelpers::fillRect (area);
        }
        else if (fill.isGradient())
        {
            ColourGradient g2 (*(fill.gradient));
            g2.multiplyOpacity (fill.getOpacity());

            if (g2.point1 == g2.point2)
            {
                fillTexture (target, area, g2.getColourAtPosition (1.0), gradientTexture, mask1, mask2, replaceExistingContents);
            }
            else
            {
                setBlendMode (replaceExistingContents || (mask1 == nullptr && fill.colour.isOpaque() && fill.gradient->isOpaque()));

                if (g2.isRadial)
                    fillWithRadialGradient (gradientTexture, target, area, g2, fill.transform, mask1, mask2);
                else
                    fillWithLinearGradient (gradientTexture, area, g2, fill.transform, mask1, mask2);
            }
        }
        else if (fill.isTiledImage())
        {
            renderImage (target, fill.image, area, fill.transform, fill.colour.getFloatAlpha(),
                         mask1, mask2, replaceExistingContents, true);
        }
    }

    //==============================================================================
    struct VariableAlphaColour
    {
        VariableAlphaColour (const Colour& c) noexcept
            : r (c.getFloatRed()), g (c.getFloatGreen()), b (c.getFloatBlue()),
              alphaScale (c.getFloatAlpha() / 255.0f), lastAlpha (-1)
        {}

        void setForAlpha (const int alpha) noexcept
        {
            if (lastAlpha != alpha)
            {
                lastAlpha = alpha;
                const float a = alpha * alphaScale;
                glColor4f (r * a, g * a, b * a, a);
            }
        }

    private:
        const float r, g, b, alphaScale;
        int lastAlpha;

        JUCE_DECLARE_NON_COPYABLE (VariableAlphaColour);
    };

    //==============================================================================
    struct EdgeTableRenderer
    {
        EdgeTableRenderer (const Colour& c) noexcept
            : colour (c)
        {}

        void draw (const EdgeTable& et)
        {
            glDisableClientState (GL_TEXTURE_COORD_ARRAY);
            glEnableClientState (GL_VERTEX_ARRAY);
            glVertexPointer (2, GL_FLOAT, 0, vertices);

            et.iterate (*this);
        }

        void setEdgeTableYPos (const int y) noexcept
        {
            vertices[1] = vertices[5] = (GLfloat) y;
            vertices[3] = vertices[7] = (GLfloat) (y + 1);
        }

        void handleEdgeTablePixel (const int x, const int alphaLevel) noexcept
        {
            drawHorizontal (x, 1, alphaLevel);
        }

        void handleEdgeTablePixelFull (const int x) noexcept
        {
            drawHorizontal (x, 1, 255);
        }

        void handleEdgeTableLine (const int x, const int width, const int alphaLevel) noexcept
        {
            drawHorizontal (x, width, alphaLevel);
        }

        void handleEdgeTableLineFull (const int x, const int width) noexcept
        {
            drawHorizontal (x, width, 255);
        }

    private:
        GLfloat vertices[8];
        VariableAlphaColour colour;

        void drawHorizontal (int x, const int w, const int alpha) noexcept
        {
            vertices[0] = vertices[2] = (GLfloat) x;
            vertices[4] = vertices[6] = (GLfloat) (x + w);

            colour.setForAlpha (alpha);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }

        JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer);
    };

    //==============================================================================
    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (const Colour& c) noexcept
            : colour (c)
        {}

        void draw (const Rectangle<float>& r)
        {
            glDisableClientState (GL_TEXTURE_COORD_ARRAY);
            glEnableClientState (GL_VERTEX_ARRAY);
            glVertexPointer (2, GL_FLOAT, 0, vertices);

            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (*this);
        }

        void operator() (const int x, const int y, const int w, const int h, const int alpha)
        {
            vertices[0] = vertices[2] = (GLfloat) x;
            vertices[1] = vertices[5] = (GLfloat) y;
            vertices[4] = vertices[6] = (GLfloat) (x + w);
            vertices[3] = vertices[7] = (GLfloat) (y + h);

            colour.setForAlpha (alpha);
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        }

    private:
        GLfloat vertices[8];
        VariableAlphaColour colour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };
}

//==============================================================================
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
    virtual Ptr clipToImageAlpha (const OpenGLTextureFromImage&, const AffineTransform&) = 0;
    virtual Ptr clipToTexture (const PositionedTexture&) = 0;
    virtual Rectangle<int> getClipBounds() const = 0;
    virtual void fillRect (const OpenGLTarget&, const Rectangle<int>& area, const FillType&, GradientTexture&, bool replaceContents) = 0;
    virtual void fillRect (const OpenGLTarget&, const Rectangle<float>& area, const FillType&, GradientTexture&) = 0;
    virtual void fillMask (const OpenGLTarget& target, const Rectangle<int>& area, const PositionedTexture&, const FillType&, GradientTexture&) = 0;
    virtual void fillEdgeTable (const OpenGLTarget& target, EdgeTable& et, const FillType& fill, GradientTexture& gradientTexture) = 0;
    virtual void drawImage (const OpenGLTarget&, const OpenGLTextureFromImage&, const AffineTransform&, float alpha,
                            const Rectangle<int>& clip, const PositionedTexture* mask) = 0;

private:
    JUCE_DECLARE_NON_COPYABLE (ClipRegionBase);
};

//==============================================================================
class ClipRegion_Mask  : public ClipRegionBase
{
public:
    ClipRegion_Mask (const ClipRegion_Mask& other)
        : clip (other.clip),
          maskOrigin (other.clip.getPosition())
    {
        TargetSaver ts;
        mask.initialise (clip.getWidth(), clip.getHeight());

        OpenGLTarget m (mask, maskOrigin);
        m.makeActiveFor2D();
        glDisable (GL_BLEND);
        setColour (1.0f);
        drawFrameBuffer (other.mask, other.maskOrigin);
    }

    explicit ClipRegion_Mask (const RectangleList& r)
        : clip (r.getBounds()),
          maskOrigin (clip.getPosition())
    {
        TargetSaver ts;
        initialiseClear();

        disableMultiTexture();
        setColour (1.0f);
        fillRectangleList (r);
    }

    Ptr clone() const                               { return new ClipRegion_Mask (*this); }
    Rectangle<int> getClipBounds() const            { return clip; }

    Ptr applyClipTo (const Ptr& target)
    {
        return target->clipToTexture (PositionedTexture (mask.getTextureID(), Rectangle<int> (maskOrigin.x, maskOrigin.y,
                                                                                              mask.getWidth(), mask.getHeight()), clip));
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

            TargetSaver ts;
            makeMaskActive();
            disableMultiTexture();
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

        TargetSaver ts;
        makeMaskActive();
        disableMultiTexture();
        glDisable (GL_BLEND);
        setColour (0);
        OpenGLHelpers::fillRect (r);
        return this;
    }

    Ptr clipToPath (const Path& p, const AffineTransform& t)
    {
        EdgeTable et (clip, p, t);

        if (! et.isEmpty())
        {
            OpenGLTexture texture;
            PositionedTexture pt (texture, et, et.getMaximumBounds());
            return clipToTexture (pt);
        }

        return nullptr;
    }

    Ptr clipToTexture (const PositionedTexture& pt)
    {
        clip = clip.getIntersection (pt.clip);

        if (clip.isEmpty())
            return nullptr;

        TargetSaver ts;
        makeMaskActive();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ZERO, GL_SRC_ALPHA);
        setColour (1.0f);
        enableSingleTexture();
        OpenGLHelpers::drawTextureQuad (pt.textureID, pt.area);
        return this;
    }

    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)
    {
        TargetSaver ts;
        makeMaskActive();
        glEnable (GL_BLEND);
        glBlendFunc (GL_ZERO, GL_SRC_ALPHA);

        setColour (1.0f);
        glBindTexture (GL_TEXTURE_2D, image.textureID);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        const GLfloat l = (GLfloat) maskOrigin.x;
        const GLfloat t = (GLfloat) maskOrigin.y;
        const GLfloat r = (GLfloat) (maskOrigin.x + mask.getWidth());
        const GLfloat b = (GLfloat) (maskOrigin.y + mask.getHeight());
        const GLfloat vertices[]  = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]   = { l, t, r, t, l, b, r, b };

        const AffineTransform inv (transform.inverted().scaled (image.fullWidthProportion / image.imageWidth,
                                                                image.fullHeightProportion / image.imageHeight));

        inv.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        inv.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        textureCoords[1] = 1.0f - textureCoords[1];
        textureCoords[3] = 1.0f - textureCoords[3];
        textureCoords[5] = 1.0f - textureCoords[5];
        textureCoords[7] = 1.0f - textureCoords[7];

        OpenGLHelpers::drawTriangleStrip (vertices, textureCoords, 4);

        return this;
    }

    void fillRect (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, GradientTexture& gradientTexture, bool replaceContents)
    {
        jassert (! replaceContents);
        const Rectangle<int> r (clip.getIntersection (area));

        if (! r.isEmpty())
            fillRectInternal (target, r, fill, gradientTexture, false);
    }

    void fillRect (const OpenGLTarget& target, const Rectangle<float>& area, const FillType& fill, GradientTexture& gradientTexture)
    {
        EdgeTable et (area);
        fillEdgeTable (target, et, fill, gradientTexture);
    }

    void fillMask (const OpenGLTarget& target, const Rectangle<int>& area, const PositionedTexture& texture, const FillType& fill, GradientTexture& gradientTexture)
    {
        PositionedTexture pt (mask.getTextureID(), Rectangle<int> (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()), area);
        fillTexture (target, area, fill, gradientTexture, &texture, &pt, false);
    }

    void fillEdgeTable (const OpenGLTarget& target, EdgeTable& et, const FillType& fill, GradientTexture& gradientTexture)
    {
        OpenGLTexture texture;
        PositionedTexture pt (texture, et, clip);

        fillMask (target, pt.clip, pt, fill, gradientTexture);
    }

    void fillRectInternal (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, GradientTexture& gradientTexture, bool replaceContents)
    {
        PositionedTexture pt (mask.getTextureID(), Rectangle<int> (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()), area);
        fillTexture (target, area, fill, gradientTexture, &pt, nullptr, replaceContents);
    }

    void drawImage (const OpenGLTarget& target, const OpenGLTextureFromImage& source, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, const PositionedTexture* mask1)
    {
        const Rectangle<int> bufferArea (clipArea.getIntersection (clip));

        if (! bufferArea.isEmpty())
        {
            PositionedTexture pt (mask.getTextureID(), Rectangle<int> (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()), bufferArea);
            renderImage (target, source, bufferArea, transform, alpha, mask1, &pt, false, false);
        }
    }

private:
    OpenGLFrameBuffer mask;
    Rectangle<int> clip;
    Point<int> maskOrigin;

    void prepareFor2D() const
    {
        OpenGLTarget::applyFlippedMatrix (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight());
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
        mask.initialise (clip.getWidth(), clip.getHeight());
        mask.makeCurrentAndClear();
        glDisable (GL_TEXTURE_2D);
        glDisable (GL_BLEND);
        prepareFor2D();
    }

    void drawFrameBuffer (const OpenGLFrameBuffer& buffer, const Point<int>& topLeft)
    {
        enableSingleTexture();
        OpenGLHelpers::drawTextureQuad (buffer.getTextureID(), Rectangle<int> (topLeft.x, topLeft.y,
                                                                               buffer.getWidth(), buffer.getHeight()));
    }

    ClipRegion_Mask& operator= (const ClipRegion_Mask&);
};


//==============================================================================
class ClipRegion_RectangleList  : public ClipRegionBase
{
public:
    explicit ClipRegion_RectangleList (const Rectangle<int>& r) noexcept
        : clip (r)
    {}

    explicit ClipRegion_RectangleList (const RectangleList& r) noexcept
        : clip (r)
    {}

    Ptr clone() const                               { return new ClipRegion_RectangleList (clip); }
    Rectangle<int> getClipBounds() const            { return clip.getBounds(); }
    Ptr applyClipTo (const Ptr& target)             { return target->clipToRectangleList (clip); }

    Ptr clipToRectangle (const Rectangle<int>& r)       { return clip.clipTo (r) ? this : nullptr; }
    Ptr clipToRectangleList (const RectangleList& r)    { return clip.clipTo (r) ? this : nullptr; }
    Ptr excludeClipRectangle (const Rectangle<int>& r)  { clip.subtract (r); return clip.isEmpty() ? nullptr : this; }

    Ptr clipToTexture (const PositionedTexture& t)                      { return toMask()->clipToTexture (t); }
    Ptr clipToPath (const Path& p, const AffineTransform& transform)    { return toMask()->clipToPath (p, transform); }
    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)    { return toMask()->clipToImageAlpha (image, transform); }

    void fillRect (const OpenGLTarget& target, const Rectangle<int>& area, const FillType& fill, GradientTexture& gradientTexture, bool replaceContents)
    {
        if (fill.isColour())
        {
            disableMultiTexture();
            setBlendMode (replaceContents || fill.colour.isOpaque());
            setPremultipliedColour (fill.colour);
            fillRectangleList (clip, area);
        }
        else
        {
            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (area));

                if (! r.isEmpty())
                    fillTexture (target, r, fill, gradientTexture, nullptr, nullptr, replaceContents);
            }
        }
    }

    void fillRect (const OpenGLTarget& target, const Rectangle<float>& area, const FillType& fill, GradientTexture& gradientTexture)
    {
        if (fill.isColour())
        {
            disableMultiTexture();
            setPremultipliedBlendingMode();
            setPremultipliedColour (fill.colour);

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<float> r (i.getRectangle()->toFloat().getIntersection (area));

                if (! r.isEmpty())
                {
                    FloatRectangleRenderer frr (fill.colour);
                    frr.draw (r);
                }
            }
        }
        else
        {
            EdgeTable et (area);
            fillEdgeTable (target, et, fill, gradientTexture);
        }
    }

    void drawImage (const OpenGLTarget& target, const OpenGLTextureFromImage& source, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, const PositionedTexture* mask)
    {
        for (RectangleList::Iterator i (clip); i.next();)
        {
            const Rectangle<int> bufferArea (i.getRectangle()->getIntersection (clipArea));

            if (! bufferArea.isEmpty())
                renderImage (target, source, bufferArea, transform, alpha, mask, nullptr, false, false);
        }
    }

    void fillEdgeTable (const OpenGLTarget& target, EdgeTable& et, const FillType& fill, GradientTexture& gradientTexture)
    {
        if (fill.isColour())
        {
            setPremultipliedBlendingMode();
            disableMultiTexture (GL_TEXTURE2);
            disableMultiTexture (GL_TEXTURE1);
            disableMultiTexture (GL_TEXTURE0);

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (et.getMaximumBounds()));

                if (! r.isEmpty())
                {
                    target.scissor (r);

                    EdgeTableRenderer etr (fill.colour);
                    etr.draw (et);
                }
            }

            glDisable (GL_SCISSOR_TEST);
        }
        else
        {
            OpenGLTexture texture;
            PositionedTexture pt (texture, et, clip.getBounds());

            fillMask (target, pt.clip, pt, fill, gradientTexture);
        }
    }

    void fillMask (const OpenGLTarget& target, const Rectangle<int>& area, const PositionedTexture& texture, const FillType& fill, GradientTexture& gradientTexture)
    {
        fillTexture (target, area, fill, gradientTexture, &texture, nullptr, false);
    }

private:
    RectangleList clip;

    Ptr toMask() const
    {
        return new ClipRegion_Mask (clip);
    }

    ClipRegion_RectangleList& operator= (const ClipRegion_RectangleList&);
};


//==============================================================================
class OpenGLRenderer::SavedState
{
public:
    SavedState (const OpenGLTarget& target_)
        : clip (new ClipRegion_RectangleList (Rectangle<int> (target_.width, target_.height))),
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
            const Rectangle<int> clipBounds (clip->getClipBounds());

            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->target = OpenGLTarget (*OpenGLImageType::getFrameBufferFrom (s->transparencyLayer), clipBounds.getPosition());
            s->transparencyLayerAlpha = opacity;
            s->cloneClipIfMultiplyReferenced();

            s->target.makeActiveFor2D();
        }

        return s;
    }

    void endTransparencyLayer (SavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            target.makeActiveFor2D();
            const Rectangle<int> clipBounds (clip->getClipBounds());

            clip->drawImage (target, finishedLayerState.transparencyLayer,
                             AffineTransform::translation ((float) clipBounds.getX(), (float) clipBounds.getY()),
                             finishedLayerState.transparencyLayerAlpha, clipBounds, nullptr);
        }
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, const bool replaceContents)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                clip->fillRect (target, r.translated (transform.xOffset, transform.yOffset),
                                getFillType(), gradientTexture, replaceContents);
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
                const Rectangle<float> c (r.translated ((float) transform.xOffset, (float) transform.yOffset)
                                           .getIntersection (clip->getClipBounds().toFloat()));

                if (! c.isEmpty())
                    clip->fillRect (target, c, getFillType(), gradientTexture);
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
            EdgeTable et (clip->getClipBounds(), path, transform.getTransformWith (t));
            fillEdgeTable (et);
        }
    }

    void drawGlyph (int glyphNumber, const AffineTransform& t)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated && t.isOnlyTranslation())
            {
                RenderingHelpers::GlyphCache <CachedGlyphEdgeTable, SavedState>::getInstance()
                    .drawGlyph (*this, font, glyphNumber,
                                transform.xOffset + t.getTranslationX(),
                                transform.yOffset + t.getTranslationY());
            }
            else
            {
                const float fontHeight = font.getHeight();

                const ScopedPointer<EdgeTable> et (font.getTypeface()->getEdgeTableForGlyph
                        (glyphNumber, transform.getTransformWith (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                                  .followedBy (t))));

                if (et != nullptr)
                    fillEdgeTable (*et);
            }
        }
    }

    void fillEdgeTable (const EdgeTable& et, const float x, const int y)
    {
        if (clip != nullptr)
        {
            EdgeTable et2 (et);
            et2.translate (x, y);

            fillEdgeTable (et2);
        }
    }

    void drawLine (const Line <float>& line)
    {
        Path p;
        p.addLineSegment (line, 1.0f);
        fillPath (p, AffineTransform::identity);
    }

    //==============================================================================
    void drawImage (const Image& image, const AffineTransform& trans)
    {
        if (clip == nullptr || fillType.colour.isTransparent())
            return;

        const Rectangle<int> clipBounds (clip->getClipBounds());
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

                clip->drawImage (target, image, t, alpha, Rectangle<int> (tx, ty, image.getWidth(), image.getHeight()), nullptr);
                return;
            }
        }

        if (! t.isSingularity())
        {
            Path p;
            p.addRectangle (image.getBounds());

            OpenGLTexture texture;
            EdgeTable et (clipBounds, p, t);
            PositionedTexture pt (texture, et, clipBounds);

            clip->drawImage (target, image, t, alpha, clipBounds, &pt);
        }
    }

    void setFillType (const FillType& newFill)
    {
        fillType = newFill;
        gradientTexture.reset();
    }

    //==============================================================================
    ClipRegionBase::Ptr clip;
    RenderingHelpers::TranslationOrTransform transform;
    Font font;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;
    OpenGLTarget target;

private:
    float transparencyLayerAlpha;
    Image transparencyLayer;
    GradientTexture gradientTexture;

    void cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    FillType getFillType() const
    {
        return fillType.transformed (transform.getTransform());
    }

    void fillEdgeTable (EdgeTable& et)
    {
        clip->fillEdgeTable (target, et, getFillType(), gradientTexture);
    }

    class CachedGlyphEdgeTable
    {
    public:
        CachedGlyphEdgeTable() : glyph (0), lastAccessCount (0) {}

        void draw (OpenGLRenderer::SavedState& state, float x, const float y) const
        {
            if (snapToIntegerCoordinate)
                x = std::floor (x + 0.5f);

            if (edgeTable != nullptr)
                state.fillEdgeTable (*edgeTable, x, roundToInt (y));
        }

        void generate (const Font& newFont, const int glyphNumber)
        {
            font = newFont;
            snapToIntegerCoordinate = newFont.getTypeface()->isHinted();
            glyph = glyphNumber;

            const float fontHeight = font.getHeight();
            edgeTable = font.getTypeface()->getEdgeTableForGlyph (glyphNumber,
                                                                  AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                                #if JUCE_MAC || JUCE_IOS
                                                                                  .translated (0.0f, -0.5f)
                                                                                #endif
                                                                  );
        }

        Font font;
        int glyph, lastAccessCount;
        bool snapToIntegerCoordinate;

    private:
        ScopedPointer <EdgeTable> edgeTable;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedGlyphEdgeTable);
    };

    SavedState& operator= (const SavedState&);
};

//==============================================================================
OpenGLRenderer::OpenGLRenderer (OpenGLComponent& target)
    : stack (new SavedState (OpenGLTarget (target.getFrameBufferID(), target.getWidth(), target.getHeight())))
{
    initialise();
}

OpenGLRenderer::OpenGLRenderer (OpenGLFrameBuffer& target)
    : stack (new SavedState (OpenGLTarget (target, Point<int>())))
{
    initialise();
}

OpenGLRenderer::OpenGLRenderer (unsigned int frameBufferID, int width, int height)
    : stack (new SavedState (OpenGLTarget (frameBufferID, width, height)))
{
    initialise();
}

void OpenGLRenderer::initialise()
{
    // This object can only be created and used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();
    initialiseMultiTextureExtensions();
    stack->target.makeActiveFor2D();
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    resetMultiTextureModes (false);
}

OpenGLRenderer::~OpenGLRenderer()
{
    OpenGLFrameBuffer::setCurrentFrameBufferTarget (previousFrameBufferTarget);
    resetMultiTextureModes (true);
}

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
void OpenGLRenderer::setFill (const FillType& fillType)                             { stack->setFillType (fillType); }
void OpenGLRenderer::setOpacity (float newOpacity)                                  { stack->fillType.setOpacity (newOpacity); }
void OpenGLRenderer::setInterpolationQuality (Graphics::ResamplingQuality quality)  { stack->interpolationQuality = quality; }
void OpenGLRenderer::fillRect (const Rectangle<int>& r, bool replace)               { stack->fillRect (r, replace); }
void OpenGLRenderer::fillPath (const Path& path, const AffineTransform& t)          { stack->fillPath (path, t); }
void OpenGLRenderer::drawImage (const Image& im, const AffineTransform& t)          { stack->drawImage (im, t); }
void OpenGLRenderer::drawVerticalLine (int x, float top, float bottom)              { if (top < bottom) stack->fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top)); }
void OpenGLRenderer::drawHorizontalLine (int y, float left, float right)            { if (left < right) stack->fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f)); }
void OpenGLRenderer::drawGlyph (int glyphNumber, const AffineTransform& t)          { stack->drawGlyph (glyphNumber, t); }
void OpenGLRenderer::drawLine (const Line <float>& line)                            { stack->drawLine (line); }
void OpenGLRenderer::setFont (const Font& newFont)                                  { stack->font = newFont; }
Font OpenGLRenderer::getFont()                                                      { return stack->font; }

END_JUCE_NAMESPACE
