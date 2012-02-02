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

//==============================================================================
struct OpenGLTarget
{
    OpenGLTarget (OpenGLContext& context_, GLuint frameBufferID_, int width, int height) noexcept
        : context (context_), frameBuffer (nullptr), frameBufferID (frameBufferID_), bounds (width, height)
    {}

    OpenGLTarget (OpenGLContext& context_, OpenGLFrameBuffer& frameBuffer_, const Point<int>& origin) noexcept
        : context (context_), frameBuffer (&frameBuffer_), frameBufferID (0),
          bounds (origin.x, origin.y, frameBuffer_.getWidth(), frameBuffer_.getHeight())
    {}

    OpenGLTarget (const OpenGLTarget& other) noexcept
        : context (other.context), frameBuffer (other.frameBuffer),
          frameBufferID (other.frameBufferID), bounds (other.bounds)
    {}

    OpenGLTarget& operator= (const OpenGLTarget& other)
    {
        frameBuffer = other.frameBuffer;
        frameBufferID = other.frameBufferID;
        bounds = other.bounds;
        return *this;
    }

    void makeActiveFor2D() const
    {
        if (frameBuffer != nullptr)
            frameBuffer->makeCurrentRenderingTarget();
        else
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, frameBufferID);

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        applyFlippedMatrix (bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight());
       #else
        glViewport (0, 0, bounds.getWidth(), bounds.getHeight());
       #endif
        glDisable (GL_DEPTH_TEST);
    }

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    void scissor (Rectangle<int> r) const
    {
        r -= bounds.getPosition();
        OpenGLHelpers::enableScissorTest (r.withY (bounds.getHeight() - r.getBottom()));
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
   #endif

    OpenGLContext& context;
    OpenGLFrameBuffer* frameBuffer;
    GLuint frameBufferID;

    Rectangle<int> bounds;
};

//==============================================================================
class PositionedTexture
{
public:
    PositionedTexture (OpenGLTexture& texture, const EdgeTable& et, const Rectangle<int>& clip_)
        : clip (clip_.getIntersection (et.getMaximumBounds()))
    {
        if (clip.contains (et.getMaximumBounds()))
        {
            createMap (texture, et);
        }
        else
        {
            EdgeTable et2 (clip);
            et2.clipToEdgeTable (et);
            createMap (texture, et2);
        }
    }

    PositionedTexture (GLuint textureID_, const Rectangle<int> area_, const Rectangle<int> clip_) noexcept
        : textureID (textureID_), area (area_), clip (clip_)
    {}

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    template <typename ValueType>
    void getTextureCoordAt (ValueType x, ValueType y, GLfloat& resultX, GLfloat& resultY) const noexcept
    {
        resultX = (x - area.getX()) / (float) area.getWidth();
        resultY = (area.getBottom() - y) / (float) area.getHeight();
    }

    void prepareTextureCoords (const Rectangle<int>* const area, GLfloat* const textureCoords) const noexcept
    {
        if (area != nullptr)
        {
            getTextureCoordAt (area->getX(),     area->getY(),      textureCoords[0], textureCoords[1]);
            getTextureCoordAt (area->getRight(), area->getY(),      textureCoords[2], textureCoords[3]);
            getTextureCoordAt (area->getX(),     area->getBottom(), textureCoords[4], textureCoords[5]);
            getTextureCoordAt (area->getRight(), area->getBottom(), textureCoords[6], textureCoords[7]);
        }

        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
    }
   #endif

    GLuint textureID;
    Rectangle<int> area, clip;

private:
    void createMap (OpenGLTexture& texture, const EdgeTable& et)
    {
        EdgeTableAlphaMap alphaMap (et);
        texture.loadAlpha (alphaMap.data, alphaMap.area.getWidth(), alphaMap.area.getHeight());
        textureID = texture.getTextureID();
        area = alphaMap.area;
    }

    struct EdgeTableAlphaMap
    {
        EdgeTableAlphaMap (const EdgeTable& et)
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

        JUCE_DECLARE_NON_COPYABLE (EdgeTableAlphaMap);
    };
};

//==============================================================================
#if JUCE_USE_OPENGL_SHADERS
class ShaderPrograms  : public ReferenceCountedObject
{
public:
    ShaderPrograms (OpenGLContext& context)
        : solidColourProgram (context),
          solidColourMasked (context),
          radialGradient (context),
          radialGradientMasked (context),
          linearGradient1 (context),
          linearGradient1Masked (context),
          linearGradient2 (context),
          linearGradient2Masked (context),
          image (context),
          imageMasked (context),
          tiledImage (context),
          tiledImageMasked (context),
          copyTexture (context),
          maskTexture (context)
    {}

    typedef ReferenceCountedObjectPtr<ShaderPrograms> Ptr;

    //==============================================================================
    struct ShaderProgramHolder
    {
        ShaderProgramHolder (OpenGLContext& context, const char* fragmentShader)
            : program (context)
        {
            program.addShader ("attribute vec2 position;"
                               "attribute vec4 colour;"
                               "uniform vec4 screenBounds;"
                               "varying " JUCE_LOWP " vec4 frontColour;"
                               "varying " JUCE_HIGHP " vec2 pixelPos;"
                               "void main()"
                               "{"
                               " frontColour = colour;"
                               " vec2 adjustedPos = position - screenBounds.xy;"
                               " pixelPos = adjustedPos;"
                               " vec2 scaledPos = adjustedPos / screenBounds.zw;"
                               " gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);"
                               "}", GL_VERTEX_SHADER);

            program.addShader (fragmentShader, GL_FRAGMENT_SHADER);
            program.link();
        }

        OpenGLShaderProgram program;
    };

    struct ShaderBase   : public ShaderProgramHolder
    {
        ShaderBase (OpenGLContext& context, const char* fragmentShader)
            : ShaderProgramHolder (context, fragmentShader),
              positionAttribute (program, "position"),
              colourAttribute (program, "colour"),
              screenBounds (program, "screenBounds")
        {}

        void set2DBounds (const Rectangle<float>& bounds)
        {
            screenBounds.set (bounds.getX(), bounds.getY(), 0.5f * bounds.getWidth(), 0.5f * bounds.getHeight());
        }

        void bindAttributes (OpenGLContext& context)
        {
            context.extensions.glVertexAttribPointer (positionAttribute.attributeID, 2, GL_SHORT, GL_FALSE, 8, (void*) 0);
            context.extensions.glVertexAttribPointer (colourAttribute.attributeID, 4, GL_UNSIGNED_BYTE, GL_TRUE, 8, (void*) 4);
            context.extensions.glEnableVertexAttribArray (positionAttribute.attributeID);
            context.extensions.glEnableVertexAttribArray (colourAttribute.attributeID);
        }

        void unbindAttributes (OpenGLContext& context)
        {
            context.extensions.glDisableVertexAttribArray (positionAttribute.attributeID);
            context.extensions.glDisableVertexAttribArray (colourAttribute.attributeID);
        }

        OpenGLShaderProgram::Attribute positionAttribute, colourAttribute;

    private:
        OpenGLShaderProgram::Uniform screenBounds;
    };

    struct MaskedShaderParams
    {
        MaskedShaderParams (OpenGLShaderProgram& program)
            : maskTexture (program, "maskTexture"),
              maskBounds  (program, "maskBounds")
        {}

        void setBounds (const Rectangle<int>& area, const OpenGLTarget& target, const GLint textureIndex) const
        {
            maskTexture.set (textureIndex);
            maskBounds.set (area.getX() - target.bounds.getX(),
                            area.getY() - target.bounds.getY(),
                            area.getWidth(), area.getHeight());
        }

        OpenGLShaderProgram::Uniform maskTexture, maskBounds;
    };

    //==============================================================================
    #define JUCE_DECLARE_VARYING_COLOUR   "varying " JUCE_LOWP " vec4 frontColour;"
    #define JUCE_DECLARE_VARYING_PIXELPOS "varying " JUCE_HIGHP " vec2 pixelPos;"

    struct SolidColourProgram  : public ShaderBase
    {
        SolidColourProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_COLOUR
                                   "void main()"
                                   "{"
                                   " gl_FragColor = frontColour;"
                                   "}")
        {}
    };

    #define JUCE_DECLARE_MASK_UNIFORMS  "uniform sampler2D maskTexture;" \
                                        "uniform ivec4 maskBounds;"
    #define JUCE_FRAGCOORD_TO_MASK_POS  "vec2 ((pixelPos.x - float (maskBounds.x)) / float (maskBounds.z)," \
                                              "1.0 - (pixelPos.y - float (maskBounds.y)) / float (maskBounds.w))"
    #define JUCE_GET_MASK_ALPHA         "texture2D (maskTexture, " JUCE_FRAGCOORD_TO_MASK_POS ").a"

    struct SolidColourMaskedProgram  : public ShaderBase
    {
        SolidColourMaskedProgram (OpenGLContext& context)
            : ShaderBase (context,
                          JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
                          "void main()"
                          "{"
                            "gl_FragColor = frontColour * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              maskParams (program)
        {}

        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct RadialGradientParams
    {
        RadialGradientParams (OpenGLShaderProgram& program)
            : gradientTexture (program, "gradientTexture"),
              matrix (program, "matrix")
        {}

        void setMatrix (const Point<float>& p1, const Point<float>& p2, const Point<float>& p3)
        {
            const AffineTransform t (AffineTransform::fromTargetPoints (p1.x, p1.y,  0.0f, 0.0f,
                                                                        p2.x, p2.y,  1.0f, 0.0f,
                                                                        p3.x, p3.y,  0.0f, 1.0f));
            const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            matrix.set (m, 6);
        }

        OpenGLShaderProgram::Uniform gradientTexture, matrix;
    };

    #define JUCE_DECLARE_MATRIX_UNIFORM   "uniform " JUCE_HIGHP " float matrix[6];"
    #define JUCE_DECLARE_RADIAL_UNIFORMS  "uniform sampler2D gradientTexture;" JUCE_DECLARE_MATRIX_UNIFORM
    #define JUCE_MATRIX_TIMES_FRAGCOORD   "(mat2 (matrix[0], matrix[3], matrix[1], matrix[4]) * pixelPos" \
                                          " + vec2 (matrix[2], matrix[5]))"
    #define JUCE_GET_TEXTURE_COLOUR       "(frontColour.a * texture2D (gradientTexture, vec2 (gradientPos, 0.5)))"

    struct RadialGradientProgram  : public ShaderBase
    {
        RadialGradientProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_PIXELPOS
                          JUCE_DECLARE_RADIAL_UNIFORMS JUCE_DECLARE_VARYING_COLOUR
                          "void main()"
                          "{"
                            JUCE_MEDIUMP " float gradientPos = length (" JUCE_MATRIX_TIMES_FRAGCOORD ");"
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        RadialGradientParams gradientParams;
    };

    struct RadialGradientMaskedProgram  : public ShaderBase
    {
        RadialGradientMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_PIXELPOS
                          JUCE_DECLARE_RADIAL_UNIFORMS JUCE_DECLARE_VARYING_COLOUR
                          JUCE_DECLARE_MASK_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_MEDIUMP " float gradientPos = length (" JUCE_MATRIX_TIMES_FRAGCOORD ");"
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR " * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        RadialGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct LinearGradientParams
    {
        LinearGradientParams (OpenGLShaderProgram& program)
            : gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo")
        {}

        OpenGLShaderProgram::Uniform gradientTexture, gradientInfo;
    };

    #define JUCE_DECLARE_LINEAR_UNIFORMS  "uniform sampler2D gradientTexture;" \
                                          "uniform " JUCE_MEDIUMP " vec4 gradientInfo;" JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
    #define JUCE_CALC_LINEAR_GRAD_POS1    JUCE_MEDIUMP " float gradientPos = (pixelPos.y - (gradientInfo.y + (gradientInfo.z * (pixelPos.x - gradientInfo.x)))) / gradientInfo.w;"
    #define JUCE_CALC_LINEAR_GRAD_POS2    JUCE_MEDIUMP " float gradientPos = (pixelPos.x - (gradientInfo.x + (gradientInfo.z * (pixelPos.y - gradientInfo.y)))) / gradientInfo.w;"

    struct LinearGradient1Program  : public ShaderBase
    {
        LinearGradient1Program (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          "void main()"
                          "{"
                            JUCE_CALC_LINEAR_GRAD_POS1
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        LinearGradientParams gradientParams;
    };

    struct LinearGradient1MaskedProgram  : public ShaderBase
    {
        LinearGradient1MaskedProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          JUCE_DECLARE_MASK_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_CALC_LINEAR_GRAD_POS1
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR " * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        LinearGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    struct LinearGradient2Program  : public ShaderBase
    {
        LinearGradient2Program (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (x2 - x1) / (y2 - y1), y = y1, w = length
                          "void main()"
                          "{"
                            JUCE_CALC_LINEAR_GRAD_POS2
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR ";"
                          "}"),
              gradientParams (program)
        {}

        LinearGradientParams gradientParams;
    };

    struct LinearGradient2MaskedProgram  : public ShaderBase
    {
        LinearGradient2MaskedProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (x2 - x1) / (y2 - y1), y = y1, w = length
                          JUCE_DECLARE_MASK_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_CALC_LINEAR_GRAD_POS2
                            "gl_FragColor = " JUCE_GET_TEXTURE_COLOUR " * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              gradientParams (program),
              maskParams (program)
        {}

        LinearGradientParams gradientParams;
        MaskedShaderParams maskParams;
    };

    //==============================================================================
    struct ImageParams
    {
        ImageParams (OpenGLShaderProgram& program)
            : imageTexture (program, "imageTexture"),
              matrix (program, "matrix"),
              imageLimits (program, "imageLimits")
        {}

        void setMatrix (const AffineTransform& trans,
                        const int imageWidth, const int imageHeight,
                        const float fullWidthProportion, const float fullHeightProportion,
                        const float targetX, const float targetY) const
        {
            const AffineTransform t (trans.translated (-targetX, -targetY)
                                        .inverted().scaled (fullWidthProportion / imageWidth,
                                                            fullHeightProportion / imageHeight));

            const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            matrix.set (m, 6);

            const float halfPixelX = 0.5f / imageWidth;
            const float halfPixelY = 0.5f / imageHeight;
            imageLimits.set (halfPixelX, halfPixelY,
                             fullWidthProportion - halfPixelX,
                             fullHeightProportion - halfPixelY);
        }

        void setMatrix (const AffineTransform& trans, const OpenGLTextureFromImage& image,
                        const float targetX, const float targetY) const
        {
            setMatrix (trans,
                       image.imageWidth, image.imageHeight,
                       image.fullWidthProportion, image.fullHeightProportion,
                       targetX, targetY);
        }

        OpenGLShaderProgram::Uniform imageTexture, matrix, imageLimits;
    };

    #define JUCE_DECLARE_IMAGE_UNIFORMS "uniform sampler2D imageTexture;" \
                                        "uniform " JUCE_MEDIUMP " vec4 imageLimits;" \
                                        JUCE_DECLARE_MATRIX_UNIFORM JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
    #define JUCE_GET_IMAGE_PIXEL        "texture2D (imageTexture, vec2 (texturePos.x, 1.0 - texturePos.y))"
    #define JUCE_CLAMP_TEXTURE_COORD    JUCE_HIGHP " vec2 texturePos = clamp (" JUCE_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits.zw + imageLimits.xy);"
    #define JUCE_MOD_TEXTURE_COORD      JUCE_HIGHP " vec2 texturePos = clamp (mod (" JUCE_MATRIX_TIMES_FRAGCOORD ", imageLimits.zw + imageLimits.xy), vec2 (0, 0), imageLimits.zw + imageLimits.xy);"

    struct ImageProgram  : public ShaderBase
    {
        ImageProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_CLAMP_TEXTURE_COORD
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL ";"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct ImageMaskedProgram  : public ShaderBase
    {
        ImageMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_MASK_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_CLAMP_TEXTURE_COORD
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL " * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              imageParams (program),
              maskParams (program)
        {}

        ImageParams imageParams;
        MaskedShaderParams maskParams;
    };

    struct TiledImageProgram  : public ShaderBase
    {
        TiledImageProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL ";"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct TiledImageMaskedProgram  : public ShaderBase
    {
        TiledImageMaskedProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_MASK_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL " * " JUCE_GET_MASK_ALPHA ";"
                          "}"),
              imageParams (program),
              maskParams (program)
        {}

        ImageParams imageParams;
        MaskedShaderParams maskParams;
    };

    struct CopyTextureProgram  : public ShaderBase
    {
        CopyTextureProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_MOD_TEXTURE_COORD
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL ";"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    struct MaskTextureProgram  : public ShaderBase
    {
        MaskTextureProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS
                          "void main()"
                          "{"
                            JUCE_HIGHP " vec2 texturePos = " JUCE_MATRIX_TIMES_FRAGCOORD ";"
                            "const float roundingError = 0.00001;"
                            "if (texturePos.x >= imageLimits.x - roundingError"
                                 "&& texturePos.y >= imageLimits.y - roundingError"
                                 "&& texturePos.x <= imageLimits.z + roundingError"
                                 "&& texturePos.y <= imageLimits.w + roundingError)"
                             "gl_FragColor = frontColour * " JUCE_GET_IMAGE_PIXEL ".a;"
                            "else "
                             "gl_FragColor = vec4 (0, 0, 0, 0);"
                          "}"),
              imageParams (program)
        {}

        ImageParams imageParams;
    };

    SolidColourProgram solidColourProgram;
    SolidColourMaskedProgram solidColourMasked;
    RadialGradientProgram radialGradient;
    RadialGradientMaskedProgram radialGradientMasked;
    LinearGradient1Program linearGradient1;
    LinearGradient1MaskedProgram linearGradient1Masked;
    LinearGradient2Program linearGradient2;
    LinearGradient2MaskedProgram linearGradient2Masked;
    ImageProgram image;
    ImageMaskedProgram imageMasked;
    TiledImageProgram tiledImage;
    TiledImageMaskedProgram tiledImageMasked;
    CopyTextureProgram copyTexture;
    MaskTextureProgram maskTexture;
};

#endif

//==============================================================================
struct StateHelpers
{
    struct ActiveTextures;

    //==============================================================================
    struct BlendingMode
    {
        BlendingMode() noexcept
            : blendingEnabled (false), srcFunction (0), dstFunction (0)
        {}

        void resync() noexcept
        {
            glDisable (GL_BLEND);
            srcFunction = dstFunction = 0;
        }

        template <class QuadQueueType>
        void setPremultipliedBlendingMode (QuadQueueType& quadQueue) noexcept
        {
            setBlendFunc (quadQueue, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }

        template <class QuadQueueType>
        void setBlendFunc (QuadQueueType& quadQueue, GLenum src, GLenum dst)
        {
            if (! blendingEnabled)
            {
                quadQueue.flush();
                blendingEnabled = true;
                glEnable (GL_BLEND);
            }

            if (srcFunction != src || dstFunction != dst)
            {
                quadQueue.flush();
                srcFunction = src;
                dstFunction = dst;
                glBlendFunc (src, dst);
            }
        }

        template <class QuadQueueType>
        void disableBlend (QuadQueueType& quadQueue) noexcept
        {
            if (blendingEnabled)
            {
                quadQueue.flush();
                blendingEnabled = false;
                glDisable (GL_BLEND);
            }
        }

        template <class QuadQueueType>
        void setBlendMode (QuadQueueType& quadQueue, const bool replaceExistingContents) noexcept
        {
            if (replaceExistingContents)
                disableBlend (quadQueue);
            else
                setPremultipliedBlendingMode (quadQueue);
        }

    private:
        bool blendingEnabled;
        GLenum srcFunction, dstFunction;
    };

    //==============================================================================
   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    struct CurrentColour
    {
        CurrentColour() noexcept
            : currentColour (0xffffffff)
        {}

        void resync() noexcept
        {
            currentColour = PixelARGB (0xffffffff);
            glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        }

        void setPremultipliedColour (const Colour& c) noexcept
        {
            setColour (c.getPixelARGB());
        }

        void setColour (const float alpha) noexcept
        {
            const uint8 v = (uint8) jmin (255, (int) (alpha * 255.0f));
            setColour (PixelARGB (v, v, v, v));
        }

        void setColour (const PixelARGB& c) noexcept
        {
            if (currentColour.getARGB() != c.getARGB())
            {
                currentColour = c;
                glColor4f (c.getRed()  / 255.0f, c.getGreen() / 255.0f,
                           c.getBlue() / 255.0f, c.getAlpha() / 255.0f);
            }
        }

        void setSolidColour() noexcept
        {
            if (currentColour.getARGB() != 0xffffffff)
            {
                currentColour = PixelARGB (0xffffffff);
                glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

    private:
        PixelARGB currentColour;
    };
   #endif

    //==============================================================================
    template <class QuadQueueType>
    struct EdgeTableRenderer
    {
        EdgeTableRenderer (QuadQueueType& quadQueue_, const PixelARGB& colour_) noexcept
            : quadQueue (quadQueue_), colour (colour_)
        {}

        void setEdgeTableYPos (const int y) noexcept
        {
            currentY = y;
        }

        void handleEdgeTablePixel (const int x, const int alphaLevel) noexcept
        {
            PixelARGB c (colour);
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, 1, 1, c);
        }

        void handleEdgeTablePixelFull (const int x) noexcept
        {
            quadQueue.add (x, currentY, 1, 1, colour);
        }

        void handleEdgeTableLine (const int x, const int width, const int alphaLevel) noexcept
        {
            PixelARGB c (colour);
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, width, 1, c);
        }

        void handleEdgeTableLineFull (const int x, const int width) noexcept
        {
            quadQueue.add (x, currentY, width, 1, colour);
        }

    private:
        QuadQueueType& quadQueue;
        const PixelARGB colour;
        int currentY;

        JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer);
    };

    template <class QuadQueueType>
    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (QuadQueueType& quadQueue_, const PixelARGB& colour_) noexcept
            : quadQueue (quadQueue_), colour (colour_)
        {}

        void operator() (const int x, const int y, const int w, const int h, const int alpha) noexcept
        {
            if (w > 0 && h > 0)
            {
                PixelARGB c (colour);
                c.multiplyAlpha (alpha);
                quadQueue.add (x, y, w, h, c);
            }
        }

    private:
        QuadQueueType& quadQueue;
        const PixelARGB colour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    struct QuadQueue
    {
        QuadQueue() noexcept
            : numIndices (0), numVertices (0), isActive (false)
        {}

        void prepare (ActiveTextures& activeTextures, CurrentColour& currentColour)
        {
            if (! isActive)
            {
                jassert (numIndices == 0 && numVertices == 0);
                activeTextures.disableTextures (*this);
                glEnableClientState (GL_COLOR_ARRAY);
                glVertexPointer (2, GL_SHORT, 0, vertices);
                glColorPointer (4, GL_UNSIGNED_BYTE, 0, colours);
                currentColour.setSolidColour();
                isActive = true; // (careful to do this last, as the preceding calls may change it)
            }
        }

        void add (const int x, const int y, const int w, const int h, const PixelARGB& colour) noexcept
        {
            jassert (isActive && w > 0 && h > 0);

            GLshort* const v = vertices + numVertices * 2;
            v[0] = v[4] = (GLshort) x;
            v[1] = v[3] = (GLshort) y;
            v[2] = v[6] = (GLshort) (x + w);
            v[5] = v[7] = (GLshort) (y + h);

            uint32* const c = colours + numVertices;
            c[0] = c[1] = c[2] = c[3] = colour.getInRGBAMemoryOrder();

            GLubyte* const i = indices + numIndices;
            i[0] = (GLubyte) numVertices;
            i[1] = i[3] = (GLubyte) (numVertices + 1);
            i[2] = i[4] = (GLubyte) (numVertices + 2);
            i[5] = (GLubyte) (numVertices + 3);

            numVertices += 4;
            numIndices += 6;

            if (numIndices > maxVerticesPerBlock - 6)
                draw();
        }

        void add (const Rectangle<float>& r, const PixelARGB& colour) noexcept
        {
            FloatRectangleRenderer<QuadQueue> frr (*this, colour);
            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
        }

        void flush() noexcept
        {
            if (isActive)
            {
                if (numIndices > 0)
                    draw();

                isActive = false;
                glDisableClientState (GL_COLOR_ARRAY);
            }
        }

        void add (const EdgeTable& et, const PixelARGB& colour)
        {
            EdgeTableRenderer<QuadQueue> etr (*this, colour);
            et.iterate (etr);
        }

    private:
        enum { maxVerticesPerBlock = 192 }; // must not go over 256 because the indices are 8-bit.
        GLshort vertices [maxVerticesPerBlock * 2];
        GLubyte indices [maxVerticesPerBlock];
        uint32 colours [maxVerticesPerBlock];
        int numIndices, numVertices;
        bool isActive;

        void draw() noexcept
        {
            glDrawElements (GL_TRIANGLES, numIndices, GL_UNSIGNED_BYTE, indices);
            numIndices = 0;
            numVertices = 0;
        }
    };
   #endif

    //==============================================================================
    struct ActiveTextures
    {
        ActiveTextures (const OpenGLContext& context_) noexcept
            : texturesEnabled (0), currentActiveTexture (0), context (context_)
        {}

        void clear() noexcept
        {
            for (int i = 0; i < numElementsInArray (currentTextureID); ++i)
                currentTextureID[i] = 0;
        }

        void clearCurrent() noexcept
        {
            currentTextureID [currentActiveTexture] = 0;
        }

        template <class QuadQueueType>
        void setTexturesEnabled (QuadQueueType& quadQueue, const int textureIndexMask) noexcept
        {
            if (texturesEnabled != textureIndexMask)
            {
                quadQueue.flush();

                for (int i = 3; --i >= 0;)
                {
                    if ((texturesEnabled & (1 << i)) != (textureIndexMask & (1 << i)))
                    {
                        setActiveTexture (i);

                        if ((textureIndexMask & (1 << i)) != 0)
                            glEnable (GL_TEXTURE_2D);
                        else
                        {
                            glDisable (GL_TEXTURE_2D);
                            currentTextureID[i] = 0;
                        }
                    }
                }

                texturesEnabled = textureIndexMask;
            }
        }

        template <class QuadQueueType>
        void disableTextures (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 0);
        }

        template <class QuadQueueType>
        void setSingleTextureMode (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 1);
            setActiveTexture (0);
        }

        template <class QuadQueueType>
        void setTwoTextureMode (QuadQueueType& quadQueue, GLuint texture1, GLuint texture2)
        {
            setTexturesEnabled (quadQueue, 3);

            if (currentActiveTexture == 0)
            {
                bindTexture (texture1);
                setActiveTexture (1);
                bindTexture (texture2);
            }
            else
            {
                setActiveTexture (1);
                bindTexture (texture2);
                setActiveTexture (0);
                bindTexture (texture1);
            }
        }

        void setActiveTexture (const int index) noexcept
        {
            if (currentActiveTexture != index)
            {
                currentActiveTexture = index;
                context.extensions.glActiveTexture (GL_TEXTURE0 + index);

               #if JUCE_USE_OPENGL_FIXED_FUNCTION
                context.extensions.glClientActiveTexture (GL_TEXTURE0 + index);
               #endif
            }
        }

        void bindTexture (const GLuint textureID) noexcept
        {
            if (currentTextureID [currentActiveTexture] != textureID)
            {
                currentTextureID [currentActiveTexture] = textureID;
                glBindTexture (GL_TEXTURE_2D, textureID);
            }
            else
            {
               #if JUCE_DEBUG
                GLint t = 0;
                glGetIntegerv (GL_TEXTURE_BINDING_2D, &t);
                jassert (t == (GLint) textureID);
               #endif
            }
        }

    private:
        GLuint currentTextureID [3];
        int texturesEnabled, currentActiveTexture;
        const OpenGLContext& context;

        ActiveTextures& operator= (const ActiveTextures&);
    };

    //==============================================================================
    struct TextureCache
    {
        TextureCache() noexcept
            : activeGradientIndex (0), gradientNeedsRefresh (true)
        {}

        OpenGLTexture* getTexture (ActiveTextures& activeTextures, int w, int h)
        {
            if (textures.size() < numTexturesToCache)
            {
                activeTextures.clear();
                return new OpenGLTexture();
            }

            for (int i = 0; i < numTexturesToCache - 2; ++i)
            {
                const OpenGLTexture* const t = textures.getUnchecked(i);
                if (t->getWidth() == w && t->getHeight() == h)
                    return textures.removeAndReturn (i);
            }

            return textures.removeAndReturn (0);
        }

        void releaseTexture (ActiveTextures& activeTextures, OpenGLTexture* texture)
        {
            activeTextures.clearCurrent();
            textures.add (texture);
        }

        void resetGradient() noexcept
        {
            gradientNeedsRefresh = true;
        }

        void bindTextureForGradient (ActiveTextures& activeTextures, const ColourGradient& gradient)
        {
            if (gradientNeedsRefresh)
            {
                gradientNeedsRefresh = false;

                if (gradientTextures.size() < numGradientTexturesToCache)
                {
                    activeGradientIndex = gradientTextures.size();
                    activeTextures.clear();
                    gradientTextures.add (new OpenGLTexture());
                }
                else
                {
                    activeGradientIndex = (activeGradientIndex + 1) % numGradientTexturesToCache;
                }

                PixelARGB lookup [gradientTextureSize];
                gradient.createLookupTable (lookup, gradientTextureSize);
                gradientTextures.getUnchecked (activeGradientIndex)->loadARGB (lookup, gradientTextureSize, 1);
            }

            activeTextures.bindTexture (gradientTextures.getUnchecked (activeGradientIndex)->getTextureID());
        }

        enum { gradientTextureSize = 256 };

    private:
        enum { numTexturesToCache = 8, numGradientTexturesToCache = 10 };
        OwnedArray<OpenGLTexture> textures, gradientTextures;
        int activeGradientIndex;
        bool gradientNeedsRefresh;
    };

   #if JUCE_USE_OPENGL_SHADERS
    //==============================================================================
    struct ShaderQuadQueue
    {
        ShaderQuadQueue (const OpenGLContext& context_) noexcept
            : context (context_), numVertices (0)
        {}

        ~ShaderQuadQueue() noexcept
        {
            static_jassert (sizeof (VertexInfo) == 8);
            context.extensions.glDeleteBuffers (2, buffers);
        }

        void initialise() noexcept
        {
            for (int i = 0, v = 0; i < numQuads * 6; i += 6, v += 4)
            {
                indexData[i] = (GLushort) v;
                indexData[i + 1] = indexData[i + 3] = (GLushort) (v + 1);
                indexData[i + 2] = indexData[i + 4] = (GLushort) (v + 2);
                indexData[i + 5] = (GLushort) (v + 3);
            }

            context.extensions.glGenBuffers (2, buffers);
            context.extensions.glBindBuffer (GL_ARRAY_BUFFER, buffers[0]);
            context.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, buffers[1]);
            context.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indexData), indexData, GL_STATIC_DRAW);
        }

        void add (const int x, const int y, const int w, const int h, const PixelARGB& colour) noexcept
        {
            jassert (w > 0 && h > 0);

            VertexInfo* const v = vertexData + numVertices;
            v[0].x = v[2].x = (GLshort) x;
            v[0].y = v[1].y = (GLshort) y;
            v[1].x = v[3].x = (GLshort) (x + w);
            v[2].y = v[3].y = (GLshort) (y + h);

            const GLuint rgba = colour.getInRGBAMemoryOrder();
            v[0].colour = rgba;
            v[1].colour = rgba;
            v[2].colour = rgba;
            v[3].colour = rgba;

            numVertices += 4;

            if (numVertices > numQuads * 4 - 4)
                draw();
        }

        void add (const Rectangle<int>& r, const PixelARGB& colour) noexcept
        {
            add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }

        void add (const Rectangle<float>& r, const PixelARGB& colour) noexcept
        {
            FloatRectangleRenderer<ShaderQuadQueue> frr (*this, colour);
            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
        }

        void add (const RectangleList& list, const PixelARGB& colour) noexcept
        {
            for (RectangleList::Iterator i (list); i.next();)
                add (*i.getRectangle(), colour);
        }

        void add (const RectangleList& list, const Rectangle<int>& clip, const PixelARGB& colour) noexcept
        {
            for (RectangleList::Iterator i (list); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (clip));

                if (! r.isEmpty())
                    add (r, colour);
            }
        }

        void add (const EdgeTable& et, const PixelARGB& colour)
        {
            EdgeTableRenderer<ShaderQuadQueue> etr (*this, colour);
            et.iterate (etr);
        }

        void flush() noexcept
        {
            if (numVertices > 0)
                draw();
        }

    private:
        struct VertexInfo
        {
            GLshort x, y;
            GLuint colour;
        };

       #if ! JUCE_MAC
        enum { numQuads = 64 }; // (had problems with my drivers segfaulting when these buffers are any larger)
       #else
        enum { numQuads = 8192 };
       #endif

        GLuint buffers[2];
        VertexInfo vertexData [numQuads * 4];
        GLushort indexData [numQuads * 6];
        const OpenGLContext& context;
        int numVertices;

        void draw() noexcept
        {
            context.extensions.glBufferData (GL_ARRAY_BUFFER, numVertices * sizeof (VertexInfo), vertexData, GL_DYNAMIC_DRAW);
            glDrawElements (GL_TRIANGLES, (numVertices * 3) / 2, GL_UNSIGNED_SHORT, 0);
            numVertices = 0;
        }

        ShaderQuadQueue& operator= (const ShaderQuadQueue&);
    };

    //==============================================================================
    struct CurrentShader
    {
        CurrentShader (OpenGLContext& context_) noexcept
            : context (context_),
              canUseShaders (context.areShadersAvailable()),
              activeShader (nullptr)
        {
            const Identifier programValueID ("GraphicsContextPrograms");
            programs = dynamic_cast <ShaderPrograms*> (context.properties [programValueID].getObject());

            if (programs == nullptr && canUseShaders)
            {
                programs = new ShaderPrograms (context);
                context.properties.set (programValueID, var (programs));
            }
        }

        void setShader (const Rectangle<int>& bounds, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
        {
            if (activeShader != &shader)
            {
                quadQueue.flush();
                activeShader = &shader;
                shader.program.use();
                shader.bindAttributes (context);

                currentBounds = bounds;
                shader.set2DBounds (bounds.toFloat());
            }
            else if (bounds != currentBounds)
            {
                currentBounds = bounds;
                shader.set2DBounds (bounds.toFloat());
            }
        }

        void setShader (OpenGLTarget& target, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
        {
            setShader (target.bounds, quadQueue, shader);
        }

        void clearShader (ShaderQuadQueue& quadQueue)
        {
            if (activeShader != nullptr)
            {
                quadQueue.flush();
                activeShader->unbindAttributes (context);
                activeShader = nullptr;
                context.extensions.glUseProgram (0);
            }
        }

        OpenGLContext& context;
        ShaderPrograms::Ptr programs;
        bool canUseShaders;

    private:
        ShaderPrograms::ShaderBase* activeShader;
        Rectangle<int> currentBounds;

        CurrentShader& operator= (const CurrentShader&);
    };
   #endif
};

//==============================================================================
class OpenGLGraphicsContext::GLState
{
public:
    GLState (const OpenGLTarget& target_) noexcept
        : target (target_),
          activeTextures (target_.context),
         #if JUCE_USE_OPENGL_SHADERS
          currentShader (target_.context),
          shaderQuadQueue (target_.context),
         #endif
          previousFrameBufferTarget (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
    {
        // This object can only be created and used when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        target.makeActiveFor2D();
        blendMode.resync();

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        currentColour.resync();
       #endif

       #ifdef GL_COLOR_ARRAY
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        #if JUCE_USE_OPENGL_SHADERS
        if (currentShader.canUseShaders)
        {
            glDisableClientState (GL_VERTEX_ARRAY);
            glDisableClientState (GL_INDEX_ARRAY);

            for (int i = 3; --i >= 0;)
            {
                activeTextures.setActiveTexture (i);
                glDisableClientState (GL_TEXTURE_COORD_ARRAY);
            }
        }
        else
        #endif
        {
            glEnableClientState (GL_VERTEX_ARRAY);

            for (int i = 3; --i >= 0;)
            {
                activeTextures.setActiveTexture (i);
                glEnableClientState (GL_TEXTURE_COORD_ARRAY);
            }
        }
       #endif

        activeTextures.clear();

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        resetMultiTextureModes (false);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
       #endif

       #if JUCE_USE_OPENGL_SHADERS
        shaderQuadQueue.initialise();
       #endif
    }

    ~GLState()
    {
        flush();

        target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        resetMultiTextureModes (true);
       #endif

       #if JUCE_USE_OPENGL_SHADERS && defined (GL_INDEX_ARRAY)
        glDisableClientState (GL_INDEX_ARRAY);
       #endif
    }

    void flush()
    {
       #if JUCE_USE_OPENGL_SHADERS
        currentShader.clearShader (shaderQuadQueue);
        shaderQuadQueue.flush();
       #endif

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        quadQueue.flush();
       #endif
    }

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    void scissor (const Rectangle<int>& r)
    {
        quadQueue.flush();
        target.scissor (r);
    }

    void disableScissor()
    {
        quadQueue.flush();
        glDisable (GL_SCISSOR_TEST);
    }

    void prepareMasks (const PositionedTexture* const mask1, const PositionedTexture* const mask2,
                       GLfloat* const textureCoords1, GLfloat* const textureCoords2, const Rectangle<int>* const area)
    {
        if (mask1 != nullptr)
        {
            activeTextures.setTexturesEnabled (quadQueue, mask2 != nullptr ? 7 : 3);
            activeTextures.setActiveTexture (0);
            mask1->prepareTextureCoords (area, textureCoords1);
            activeTextures.bindTexture (mask1->textureID);
            activeTextures.setActiveTexture (1);

            if (mask2 != nullptr)
            {
                mask2->prepareTextureCoords (area, textureCoords2);
                activeTextures.bindTexture (mask2->textureID);
                activeTextures.setActiveTexture (2);
            }
        }
        else
        {
            activeTextures.setSingleTextureMode (quadQueue);
        }
    }

    void fillRect (const Rectangle<int>& r, const PixelARGB& colour) noexcept
    {
        jassert (! r.isEmpty());

        quadQueue.prepare (activeTextures, currentColour);
        quadQueue.add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
    }

    void fillRect (const Rectangle<float>& r, const PixelARGB& colour) noexcept
    {
        jassert (! r.isEmpty());

        quadQueue.prepare (activeTextures, currentColour);
        quadQueue.add (r, colour);
    }

    void fillRectangleList (const RectangleList& list, const PixelARGB& colour)
    {
        quadQueue.prepare (activeTextures, currentColour);

        for (RectangleList::Iterator i (list); i.next();)
            quadQueue.add (i.getRectangle()->getX(), i.getRectangle()->getY(),
                           i.getRectangle()->getWidth(), i.getRectangle()->getHeight(), colour);
    }

    void fillRectangleList (const RectangleList& list, const Rectangle<int>& clip, const PixelARGB& colour)
    {
        quadQueue.prepare (activeTextures, currentColour);

        for (RectangleList::Iterator i (list); i.next();)
        {
            const Rectangle<int> r (i.getRectangle()->getIntersection (clip));

            if (! r.isEmpty())
                quadQueue.add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }
    }

    void fillEdgeTable (const EdgeTable& et, const PixelARGB& colour)
    {
        quadQueue.prepare (activeTextures, currentColour);
        quadQueue.add (et, colour);
    }

    void drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords, const int numVertices) noexcept
    {
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
        glDrawArrays (GL_TRIANGLE_STRIP, 0, numVertices);
    }

    void renderImage (const OpenGLTextureFromImage& image,
                      const Rectangle<int>& clip, const AffineTransform& transform, float alpha,
                      const PositionedTexture* mask1, const PositionedTexture* mask2,
                      const bool replaceExistingContents, const bool isTiled)
    {
        quadQueue.flush();
        blendMode.setBlendMode (quadQueue, replaceExistingContents);
        currentColour.setColour (alpha);
        GLfloat textureCoords1[8], textureCoords2[8];

        if ((! isTiled) || (isPowerOfTwo (image.imageWidth) && isPowerOfTwo (image.imageHeight)))
        {
            prepareMasks (mask1, mask2, textureCoords1, textureCoords2, &clip);

            activeTextures.bindTexture (image.textureID);
            TemporaryColourModulationMode tmm;

            if (isTiled)
            {
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            }

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

            if (isTiled)
            {
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
        }
        else
        {
            prepareMasks (mask1, mask2, textureCoords1, textureCoords2, nullptr);

            activeTextures.bindTexture (image.textureID);
            TemporaryColourModulationMode tmm;

            scissor (clip);
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
            disableScissor();
        }
    }

    void fillTexture (const Rectangle<int>& area, const FillType& fill,
                      const PositionedTexture* mask1, const PositionedTexture* mask2,
                      const bool replaceExistingContents)
    {
        jassert (! (mask1 == nullptr && mask2 != nullptr));

        if (fill.isColour())
        {
            GLfloat textureCoords1[8], textureCoords2[8];

            if (mask1 != nullptr)
            {
                blendMode.setBlendMode (quadQueue, replaceExistingContents);
                activeTextures.setTexturesEnabled (quadQueue, mask2 != nullptr ? 3 : 1);

                activeTextures.setActiveTexture (0);
                mask1->prepareTextureCoords (&area, textureCoords1);
                activeTextures.bindTexture (mask1->textureID);

                if (mask2 != nullptr)
                {
                    activeTextures.setActiveTexture (1);
                    mask2->prepareTextureCoords (&area, textureCoords2);
                    activeTextures.bindTexture (mask2->textureID);
                }
            }
            else
            {
                blendMode.setBlendMode (quadQueue, replaceExistingContents || fill.colour.isOpaque());
                activeTextures.disableTextures (quadQueue);
            }

            currentColour.setPremultipliedColour (fill.colour);
            OpenGLHelpers::fillRect (area);
        }
        else if (fill.isGradient())
        {
            ColourGradient g2 (*(fill.gradient));
            g2.multiplyOpacity (fill.getOpacity());

            if (g2.point1 == g2.point2)
            {
                fillTexture (area, g2.getColourAtPosition (1.0), mask1, mask2, replaceExistingContents);
            }
            else
            {
                blendMode.setBlendMode (quadQueue, replaceExistingContents || (mask1 == nullptr && fill.colour.isOpaque() && fill.gradient->isOpaque()));

                if (g2.isRadial)
                    fillWithRadialGradient (area, g2, fill.transform, mask1, mask2);
                else
                    fillWithLinearGradient (area, g2, fill.transform, mask1, mask2);
            }
        }
        else if (fill.isTiledImage())
        {
            renderImage (fill.image, area, fill.transform, fill.colour.getFloatAlpha(),
                         mask1, mask2, replaceExistingContents, true);
        }
    }
   #endif

   #if JUCE_USE_OPENGL_SHADERS
    void setShader (ShaderPrograms::ShaderBase& shader)
    {
        currentShader.setShader (target, shaderQuadQueue, shader);
    }

    void setShaderForGradientFill (const ColourGradient& g, const AffineTransform& transform,
                                   const int maskTextureID, const Rectangle<int>* const maskArea)
    {
        activeTextures.disableTextures (shaderQuadQueue);
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);

        if (maskArea != nullptr)
        {
            activeTextures.setTexturesEnabled (shaderQuadQueue, 3);
            activeTextures.setActiveTexture (1);
            activeTextures.bindTexture (maskTextureID);
            activeTextures.setActiveTexture (0);
            textureCache.bindTextureForGradient (activeTextures, g);
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            textureCache.bindTextureForGradient (activeTextures, g);
        }

        const AffineTransform t (transform.translated ((float) -target.bounds.getX(), (float) -target.bounds.getY()));
        Point<float> p1 (g.point1.transformedBy (t));
        const Point<float> p2 (g.point2.transformedBy (t));
        const Point<float> p3 (Point<float> (g.point1.x + (g.point2.y - g.point1.y),
                                             g.point1.y - (g.point2.x - g.point1.x)).transformedBy (t));

        ShaderPrograms* const programs = currentShader.programs;
        const ShaderPrograms::MaskedShaderParams* maskParams = nullptr;

        if (g.isRadial)
        {
            ShaderPrograms::RadialGradientParams* gradientParams;

            if (maskArea == nullptr)
            {
                setShader (programs->radialGradient);
                gradientParams = &programs->radialGradient.gradientParams;
            }
            else
            {
                setShader (programs->radialGradientMasked);
                gradientParams = &programs->radialGradientMasked.gradientParams;
                maskParams = &programs->radialGradientMasked.maskParams;
            }

            gradientParams->setMatrix (p1, p2, p3);
        }
        else
        {
            p1 = Line<float> (p1, p3).findNearestPointTo (p2);
            const Point<float> delta (p2.x - p1.x, p1.y - p2.y);
            const ShaderPrograms::LinearGradientParams* gradientParams;
            float grad, length;

            if (std::abs (delta.x) < std::abs (delta.y))
            {
                if (maskArea == nullptr)
                {
                    setShader (programs->linearGradient1);
                    gradientParams = &(programs->linearGradient1.gradientParams);
                }
                else
                {
                    setShader (programs->linearGradient1Masked);
                    gradientParams = &(programs->linearGradient1Masked.gradientParams);
                    maskParams = &programs->linearGradient1Masked.maskParams;
                }

                grad = delta.x / delta.y;
                length = (p2.y - grad * p2.x) - (p1.y - grad * p1.x);
            }
            else
            {
                if (maskArea == nullptr)
                {
                    setShader (programs->linearGradient2);
                    gradientParams = &(programs->linearGradient2.gradientParams);
                }
                else
                {
                    setShader (programs->linearGradient2Masked);
                    gradientParams = &(programs->linearGradient2Masked.gradientParams);
                    maskParams = &programs->linearGradient2Masked.maskParams;
                }

                grad = delta.y / delta.x;
                length = (p2.x - grad * p2.y) - (p1.x - grad * p1.y);
            }

            gradientParams->gradientInfo.set (p1.x, p1.y, grad, length);
        }

        if (maskParams != nullptr)
            maskParams->setBounds (*maskArea, target, 1);
    }

    void setShaderForTiledImageFill (const OpenGLTextureFromImage& image, const AffineTransform& transform,
                                     const int maskTextureID, const Rectangle<int>* const maskArea, const bool clampTiledImages)
    {
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);

        ShaderPrograms* const programs = currentShader.programs;

        const ShaderPrograms::MaskedShaderParams* maskParams = nullptr;
        const ShaderPrograms::ImageParams* imageParams;

        if (maskArea != nullptr)
        {
            activeTextures.setTwoTextureMode (shaderQuadQueue, image.textureID, maskTextureID);

            if (clampTiledImages)
            {
                setShader (programs->imageMasked);
                imageParams = &programs->imageMasked.imageParams;
                maskParams = &programs->imageMasked.maskParams;
            }
            else
            {
                setShader (programs->tiledImageMasked);
                imageParams = &programs->tiledImageMasked.imageParams;
                maskParams = &programs->tiledImageMasked.maskParams;
            }
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            activeTextures.bindTexture (image.textureID);

            if (clampTiledImages)
            {
                setShader (programs->image);
                imageParams = &programs->image.imageParams;
            }
            else
            {
                setShader (programs->tiledImage);
                imageParams = &programs->tiledImage.imageParams;
            }
        }

        imageParams->setMatrix (transform, image, (float) target.bounds.getX(), (float) target.bounds.getY());

        if (maskParams != nullptr)
            maskParams->setBounds (*maskArea, target, 1);
    }
   #endif

    OpenGLTarget target;

    StateHelpers::BlendingMode blendMode;
    StateHelpers::ActiveTextures activeTextures;
    StateHelpers::TextureCache textureCache;

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    StateHelpers::CurrentColour currentColour;
    StateHelpers::QuadQueue quadQueue;
   #endif

   #if JUCE_USE_OPENGL_SHADERS
    StateHelpers::CurrentShader currentShader;
    StateHelpers::ShaderQuadQueue shaderQuadQueue;
   #endif

private:
    GLuint previousFrameBufferTarget;

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    void resetMultiTextureMode (int index, const bool forRGBTextures)
    {
        activeTextures.setActiveTexture (index);
        glDisable (GL_TEXTURE_2D);
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);
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
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void resetMultiTextureModes (const bool forRGBTextures)
    {
        resetMultiTextureMode (2, forRGBTextures);
        resetMultiTextureMode (1, forRGBTextures);
        resetMultiTextureMode (0, forRGBTextures);
    }

    void fillWithLinearGradient (const Rectangle<int>& rect, const ColourGradient& grad, const AffineTransform& transform,
                                 const PositionedTexture* mask1, const PositionedTexture* mask2)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.x - (grad.point2.y - grad.point1.y) / StateHelpers::TextureCache::gradientTextureSize,
                                             grad.point1.y + (grad.point2.x - grad.point1.x) / StateHelpers::TextureCache::gradientTextureSize)
                                    .transformedBy (transform));

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

        textureCache.bindTextureForGradient (activeTextures, grad);

        currentColour.setSolidColour();
        drawTriangleStrip (vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (const Rectangle<int>& rect, const ColourGradient& grad, const AffineTransform& transform,
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

        scissor (rect);
        textureCache.bindTextureForGradient (activeTextures, grad);
        currentColour.setSolidColour();
        TemporaryColourModulationMode tmm;
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords1);
        glDrawArrays (GL_TRIANGLE_FAN, 0, numDivisions + 2);
        disableScissor();
    }

    struct TemporaryColourModulationMode
    {
        TemporaryColourModulationMode()    { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR); }
        ~TemporaryColourModulationMode()   { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA); }
    };
   #endif
};

//==============================================================================
class ClipRegion_Mask;

//==============================================================================
class ClipRegionBase  : public SingleThreadedReferenceCountedObject
{
public:
    ClipRegionBase (OpenGLGraphicsContext::GLState& state_) noexcept : state (state_) {}
    virtual ~ClipRegionBase() {}

    typedef ReferenceCountedObjectPtr<ClipRegionBase> Ptr;

    virtual Ptr clone() const = 0;
    virtual Ptr clipToRectangle (const Rectangle<int>&) = 0;
    virtual Ptr clipToRectangleList (const RectangleList&) = 0;
    virtual Ptr excludeClipRectangle (const Rectangle<int>&) = 0;
    virtual Ptr clipToPath (const Path& p, const AffineTransform&) = 0;
    virtual Ptr clipToImageAlpha (const OpenGLTextureFromImage&, const AffineTransform&) = 0;
    virtual Ptr clipToTexture (const PositionedTexture&) = 0;
    virtual Rectangle<int> getClipBounds() const = 0;
    virtual void fillRect (const Rectangle<int>& area, const FillType&, bool replaceContents) = 0;
    virtual void fillRect (const Rectangle<float>& area, const FillType&) = 0;
    virtual void fillEdgeTable (EdgeTable& et, const FillType& fill) = 0;
    virtual void drawImage (const Image&, const AffineTransform&, float alpha,
                            const Rectangle<int>& clip, EdgeTable* mask) = 0;

    OpenGLGraphicsContext::GLState& state;

private:
    JUCE_DECLARE_NON_COPYABLE (ClipRegionBase);
};

//==============================================================================
class ClipRegion_RectangleListBase  : public ClipRegionBase
{
public:
    ClipRegion_RectangleListBase (OpenGLGraphicsContext::GLState& state_, const Rectangle<int>& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    ClipRegion_RectangleListBase (OpenGLGraphicsContext::GLState& state_, const RectangleList& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    Rectangle<int> getClipBounds() const            { return clip.getBounds(); }

    Ptr clipToRectangle (const Rectangle<int>& r)       { return clip.clipTo (r) ? this : nullptr; }
    Ptr clipToRectangleList (const RectangleList& r)    { return clip.clipTo (r) ? this : nullptr; }
    Ptr excludeClipRectangle (const Rectangle<int>& r)  { clip.subtract (r); return clip.isEmpty() ? nullptr : this; }

protected:
    RectangleList clip;
};


#if JUCE_USE_OPENGL_FIXED_FUNCTION
//==============================================================================
class ClipRegion_Mask  : public ClipRegionBase
{
public:
    ClipRegion_Mask (const ClipRegion_Mask& other)
        : ClipRegionBase (other.state),
          clip (other.clip),
          maskOrigin (other.clip.getPosition())
    {
        TargetSaver ts (state.target.context);
        state.flush();
        state.activeTextures.setSingleTextureMode (state.quadQueue);
        state.activeTextures.clear();
        mask.initialise (state.target.context, clip.getWidth(), clip.getHeight());

        OpenGLTarget m (state.target.context, mask, maskOrigin);
        m.makeActiveFor2D();
        state.blendMode.disableBlend (state.quadQueue);
        state.currentColour.setSolidColour();
        state.activeTextures.setSingleTextureMode (state.quadQueue);
        OpenGLHelpers::drawTextureQuad (other.mask.getTextureID(), other.getMaskArea());
    }

    ClipRegion_Mask (OpenGLGraphicsContext::GLState& state_, const RectangleList& r)
        : ClipRegionBase (state_),
          clip (r.getBounds()),
          maskOrigin (clip.getPosition())
    {
        TargetSaver ts (state.target.context);
        initialiseClear();
        state.blendMode.disableBlend (state.quadQueue);
        state.fillRectangleList (r, PixelARGB (0xffffffff));
        state.quadQueue.flush();
    }

    Ptr clone() const                       { return new ClipRegion_Mask (*this); }
    Rectangle<int> getClipBounds() const    { return clip; }

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

            TargetSaver ts (state.target.context);
            makeMaskActive();
            state.blendMode.disableBlend (state.quadQueue);
            state.fillRectangleList (excluded, PixelARGB (0));
            state.quadQueue.flush();
        }

        return this;
    }

    Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        if (r.contains (clip))
            return nullptr;

        TargetSaver ts (state.target.context);
        makeMaskActive();
        state.activeTextures.disableTextures (state.quadQueue);
        state.blendMode.disableBlend (state.quadQueue);
        state.currentColour.setColour (PixelARGB (0));
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

        TargetSaver ts (state.target.context);
        makeMaskActive();
        state.blendMode.setBlendFunc (state.quadQueue, GL_ZERO, GL_SRC_ALPHA);
        state.currentColour.setSolidColour();
        state.activeTextures.setSingleTextureMode (state.quadQueue);
        OpenGLHelpers::drawTextureQuad (pt.textureID, pt.area);
        return this;
    }

    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)
    {
        TargetSaver ts (state.target.context);
        makeMaskActive();
        state.blendMode.setBlendFunc (state.quadQueue, GL_ZERO, GL_SRC_ALPHA);
        state.currentColour.setSolidColour();
        state.activeTextures.setSingleTextureMode (state.quadQueue);
        state.activeTextures.bindTexture (image.textureID);

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

        state.drawTriangleStrip (vertices, textureCoords, 4);
        return this;
    }

    void fillRect (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        jassert (! replaceContents);
        const Rectangle<int> r (clip.getIntersection (area));

        if (! r.isEmpty())
            fillRectInternal (r, fill, false);
    }

    void fillRect (const Rectangle<float>& area, const FillType& fill)
    {
        if (fill.isColour())
        {
            FloatRectangleRenderer frr (*this, fill);
            RenderingHelpers::FloatRectangleRasterisingInfo (area).iterate (frr);
        }
        else
        {
            EdgeTable et (area);
            fillEdgeTable (et, fill);
        }
    }

    void fillEdgeTable (EdgeTable& et, const FillType& fill)
    {
        const Rectangle<int> r (et.getMaximumBounds().getIntersection (clip));

        if (! r.isEmpty())
        {
            OpenGLTexture* texture = state.textureCache.getTexture (state.activeTextures, r.getWidth(), r.getHeight());
            PositionedTexture pt1 (*texture, et, r);
            PositionedTexture pt2 (mask.getTextureID(), getMaskArea(), r);
            state.fillTexture (r, fill, &pt2, &pt1, false);
            state.textureCache.releaseTexture (state.activeTextures, texture);
        }
    }

    void fillRectInternal (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        PositionedTexture pt (mask.getTextureID(), getMaskArea(), area);
        state.fillTexture (area, fill, &pt, nullptr, replaceContents);
    }

    void drawImage (const Image& image, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, EdgeTable* et)
    {
        const OpenGLTextureFromImage source (image);
        const Rectangle<int> bufferArea (clipArea.getIntersection (clip));

        if (! bufferArea.isEmpty())
        {
            PositionedTexture pt (mask.getTextureID(), getMaskArea(), bufferArea);

            if (et != nullptr)
            {
                OpenGLTexture* texture = state.textureCache.getTexture (state.activeTextures, clipArea.getWidth(), clipArea.getHeight());
                PositionedTexture mask1 (*texture, *et, clipArea);

                state.renderImage (source, bufferArea, transform, alpha, &pt, &mask1, false, false);

                state.textureCache.releaseTexture (state.activeTextures, texture);
            }
            else
            {
                state.renderImage (source, bufferArea, transform, alpha, &pt, nullptr, false, false);
            }
        }
    }

protected:
    OpenGLFrameBuffer mask;
    Rectangle<int> clip;
    Point<int> maskOrigin;

    Rectangle<int> getMaskArea() const noexcept { return Rectangle<int> (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()); }
    void prepareFor2D() const    { OpenGLTarget::applyFlippedMatrix (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()); }

    void makeMaskActive()
    {
        state.flush();
        const bool b = mask.makeCurrentRenderingTarget();
        (void) b; jassert (b);
        prepareFor2D();
    }

    void initialiseClear()
    {
        state.flush();
        jassert (! clip.isEmpty());
        state.activeTextures.setSingleTextureMode (state.quadQueue);
        state.activeTextures.clear();
        mask.initialise (state.target.context, clip.getWidth(), clip.getHeight());
        mask.makeCurrentAndClear();
        state.activeTextures.disableTextures (state.quadQueue);
        state.blendMode.disableBlend (state.quadQueue);
        prepareFor2D();
    }

    struct TargetSaver
    {
        TargetSaver (const OpenGLContext& context_)
            : context (context_), oldFramebuffer (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
        {
            glGetIntegerv (GL_VIEWPORT, oldViewport);
            glPushMatrix();
        }

        ~TargetSaver()
        {
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, oldFramebuffer);

            glPopMatrix();
            glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
        }

    private:
        const OpenGLContext& context;
        GLuint oldFramebuffer;
        GLint oldViewport[4];

        TargetSaver& operator= (const TargetSaver&);
    };

    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (ClipRegion_Mask& owner_, const FillType& fill_) noexcept
            : owner (owner_), fill (fill_), originalColour (fill_.colour)
        {}

        void operator() (const int x, const int y, const int w, const int h, const int alpha) noexcept
        {
            if (w > 0 && h > 0)
            {
                fill.colour = originalColour.withMultipliedAlpha (alpha / 255.0f);
                owner.fillRect (Rectangle<int> (x, y, w, h), fill, false);
            }
        }

    private:
        ClipRegion_Mask& owner;
        FillType fill;
        const Colour originalColour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };

    ClipRegion_Mask& operator= (const ClipRegion_Mask&);
};

//==============================================================================
class ClipRegion_RectangleList  : public ClipRegion_RectangleListBase
{
public:
    ClipRegion_RectangleList (OpenGLGraphicsContext::GLState& state_, const Rectangle<int>& r) noexcept
        : ClipRegion_RectangleListBase (state_, r)
    {}

    ClipRegion_RectangleList (OpenGLGraphicsContext::GLState& state_, const RectangleList& r) noexcept
        : ClipRegion_RectangleListBase (state_, r)
    {}

    Ptr clone() const                               { return new ClipRegion_RectangleList (state, clip); }

    Ptr clipToTexture (const PositionedTexture& t)                                                  { return toMask()->clipToTexture (t); }
    Ptr clipToPath (const Path& p, const AffineTransform& transform)                                { return toMask()->clipToPath (p, transform); }
    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)    { return toMask()->clipToImageAlpha (image, transform); }

    void fillRect (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        if (fill.isColour())
        {
            state.activeTextures.disableTextures (state.quadQueue);
            state.blendMode.setBlendMode (state.quadQueue, replaceContents || fill.colour.isOpaque());
            state.fillRectangleList (clip, area, fill.colour.getPixelARGB());
        }
        else
        {
            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (area));

                if (! r.isEmpty())
                    state.fillTexture (r, fill, nullptr, nullptr, replaceContents);
            }
        }
    }

    void fillRect (const Rectangle<float>& area, const FillType& fill)
    {
        if (fill.isColour())
        {
            state.activeTextures.disableTextures (state.quadQueue);
            state.blendMode.setPremultipliedBlendingMode (state.quadQueue);

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<float> r (i.getRectangle()->toFloat().getIntersection (area));
                if (! r.isEmpty())
                    state.fillRect (r, fill.colour.getPixelARGB());
            }
        }
        else
        {
            EdgeTable et (area);
            fillEdgeTable (et, fill);
        }
    }

    void drawImage (const Image& image, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, EdgeTable* et)
    {
        const OpenGLTextureFromImage source (image);

        for (RectangleList::Iterator i (clip); i.next();)
        {
            const Rectangle<int> bufferArea (i.getRectangle()->getIntersection (clipArea));

            if (! bufferArea.isEmpty())
            {
                if (et != nullptr)
                {
                    OpenGLTexture* texture = state.textureCache.getTexture (state.activeTextures, clipArea.getWidth(), clipArea.getHeight());
                    PositionedTexture mask (*texture, *et, clipArea);

                    state.renderImage (source, bufferArea, transform, alpha, &mask, nullptr, false, false);

                    state.textureCache.releaseTexture (state.activeTextures, texture);
                }
                else
                {
                    state.renderImage (source, bufferArea, transform, alpha, nullptr, nullptr, false, false);
                }
            }
        }
    }

    void fillEdgeTable (EdgeTable& et, const FillType& fill)
    {
        if (fill.isColour())
        {
            state.blendMode.setPremultipliedBlendingMode (state.quadQueue);

            if (! clip.containsRectangle (et.getMaximumBounds()))
                et.clipToEdgeTable (EdgeTable (clip));

            state.fillEdgeTable (et, fill.colour.getPixelARGB());
        }
        else
        {
            OpenGLTexture* texture = state.textureCache.getTexture (state.activeTextures,
                                                                    clip.getBounds().getWidth(), clip.getBounds().getHeight());
            PositionedTexture pt (*texture, et, clip.getBounds());

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (pt.clip));
                if (! r.isEmpty())
                    state.fillTexture (r, fill, &pt, nullptr, false);
            }

            state.textureCache.releaseTexture (state.activeTextures, texture);
        }
    }

protected:
    Ptr toMask() const    { return new ClipRegion_Mask (state, clip); }

    JUCE_DECLARE_NON_COPYABLE (ClipRegion_RectangleList);
};
#endif

//==============================================================================
#if JUCE_USE_OPENGL_SHADERS

class ClipRegion_Mask_Shader  : public ClipRegionBase
{
public:
    ClipRegion_Mask_Shader (const ClipRegion_Mask_Shader& other)
        : ClipRegionBase (other.state),
          clip (other.clip),
          maskArea (other.clip)
    {
        TargetSaver ts (state.target.context);
        state.currentShader.clearShader (state.shaderQuadQueue);
        state.shaderQuadQueue.flush();
        state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
        state.activeTextures.clear();
        mask.initialise (state.target.context, maskArea.getWidth(), maskArea.getHeight());
        maskArea.setSize (mask.getWidth(), mask.getHeight());
        makeActive();

        state.blendMode.disableBlend (state.shaderQuadQueue);
        state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
        state.activeTextures.bindTexture (other.mask.getTextureID());

        state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->copyTexture);
        state.currentShader.programs->copyTexture.imageParams.imageTexture.set (0);
        state.currentShader.programs->copyTexture.imageParams
            .setMatrix (AffineTransform::translation ((float) other.maskArea.getX(), (float) other.maskArea.getY()),
                        other.maskArea.getWidth(), other.maskArea.getHeight(), 1.0f, 1.0f,
                        (float) maskArea.getX(), (float) maskArea.getY());

        state.shaderQuadQueue.add (clip, PixelARGB (0xffffffff));
        state.shaderQuadQueue.flush();
    }

    ClipRegion_Mask_Shader (OpenGLGraphicsContext::GLState& state_, const RectangleList& r)
        : ClipRegionBase (state_),
          clip (r.getBounds()),
          maskArea (clip)
    {
        TargetSaver ts (state.target.context);
        state.currentShader.clearShader (state.shaderQuadQueue);
        state.shaderQuadQueue.flush();
        state.activeTextures.clear();
        mask.initialise (state.target.context, maskArea.getWidth(), maskArea.getHeight());
        maskArea.setSize (mask.getWidth(), mask.getHeight());
        mask.makeCurrentAndClear();
        makeActive();
        state.blendMode.setBlendMode (state.shaderQuadQueue, true);
        state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->solidColourProgram);
        state.shaderQuadQueue.add (r, PixelARGB (0xffffffff));
        state.shaderQuadQueue.flush();
    }

    Ptr clone() const                       { return new ClipRegion_Mask_Shader (*this); }
    Rectangle<int> getClipBounds() const    { return clip; }

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

            TargetSaver ts (state.target.context);
            makeActive();
            state.blendMode.setBlendMode (state.shaderQuadQueue, true);
            state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->solidColourProgram);
            state.shaderQuadQueue.add (excluded, PixelARGB (0));
            state.shaderQuadQueue.flush();
        }

        return this;
    }

    Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        if (r.contains (clip))
            return nullptr;

        TargetSaver ts (state.target.context);
        makeActive();
        state.blendMode.setBlendMode (state.shaderQuadQueue, true);
        state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->solidColourProgram);
        state.shaderQuadQueue.add (r, PixelARGB (0));
        state.shaderQuadQueue.flush();
        return this;
    }

    Ptr clipToPath (const Path& p, const AffineTransform& t)
    {
        EdgeTable et (clip, p, t);

        if (! et.isEmpty())
        {
            TargetSaver ts (state.target.context);
            state.currentShader.clearShader (state.shaderQuadQueue);
            state.shaderQuadQueue.flush();
            state.activeTextures.clear();

            OpenGLTexture texture;
            PositionedTexture pt (texture, et, clip);
            return clipToTexture (pt);
        }

        return nullptr;
    }

    Ptr clipToTexture (const PositionedTexture& pt)
    {
        clip = clip.getIntersection (pt.clip);

        if (clip.isEmpty())
            return nullptr;

        TargetSaver ts (state.target.context);
        makeActive();

        state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
        state.activeTextures.bindTexture (pt.textureID);

        state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->maskTexture);
        state.currentShader.programs->maskTexture.imageParams.imageTexture.set (0);
        state.currentShader.programs->maskTexture.imageParams
            .setMatrix (AffineTransform::translation ((float) pt.area.getX(), (float) pt.area.getY()),
                        pt.area.getWidth(), pt.area.getHeight(), 1.0f, 1.0f,
                        (float) maskArea.getX(), (float) maskArea.getY());

        state.blendMode.setBlendFunc (state.shaderQuadQueue, GL_ZERO, GL_SRC_ALPHA);
        state.shaderQuadQueue.add (clip, PixelARGB (0xffffffff));
        state.shaderQuadQueue.flush();
        return this;
    }

    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)
    {
        TargetSaver ts (state.target.context);
        makeActive();
        state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
        state.activeTextures.bindTexture (image.textureID);

        state.currentShader.setShader (maskArea, state.shaderQuadQueue, state.currentShader.programs->maskTexture);
        state.currentShader.programs->maskTexture.imageParams.imageTexture.set (0);
        state.currentShader.programs->maskTexture.imageParams
            .setMatrix (transform, image, (float) maskArea.getX(), (float) maskArea.getY());

        state.shaderQuadQueue.add (clip, PixelARGB (0xffffffff));
        state.shaderQuadQueue.flush();
        return this;
    }

    void fillRect (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        jassert (! replaceContents);
        const Rectangle<int> r (clip.getIntersection (area));

        if (! r.isEmpty())
        {
            ShaderFillOperation fillOp (*this, fill, false);
            state.shaderQuadQueue.add (r, fill.colour.getPixelARGB());
        }
    }

    void fillRect (const Rectangle<float>& area, const FillType& fill)
    {
        ShaderFillOperation fillOp (*this, fill, false);

        FloatRectangleRenderer frr (*this, fill);
        RenderingHelpers::FloatRectangleRasterisingInfo (area).iterate (frr);
    }

    void fillEdgeTable (EdgeTable& et, const FillType& fill)
    {
        if (et.getMaximumBounds().intersects (clip))
        {
            if (! clip.contains (et.getMaximumBounds()))
                et.clipToRectangle (clip);

            ShaderFillOperation fillOp (*this, fill, false);
            state.shaderQuadQueue.add (et, fill.colour.getPixelARGB());
        }
    }

    void drawImage (const Image& image, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, EdgeTable* et)
    {
        const Rectangle<int> r (clip.getIntersection (clipArea));

        if (! r.isEmpty())
        {
            const PixelARGB colour (Colours::white.withAlpha (alpha).getPixelARGB());
            ShaderFillOperation fillOp (*this, FillType (image, transform), true);

            if (et != nullptr)
            {
                et->clipToRectangle (r);

                if (! et->isEmpty())
                    state.shaderQuadQueue.add (*et, colour);
            }
            else
            {
                state.shaderQuadQueue.add (r, colour);
            }
        }

        state.currentShader.clearShader (state.shaderQuadQueue);
    }

private:
    OpenGLFrameBuffer mask;
    Rectangle<int> clip, maskArea;

    struct ShaderFillOperation
    {
        ShaderFillOperation (const ClipRegion_Mask_Shader& clip, const FillType& fill, const bool clampTiledImages)
            : state (clip.state)
        {
            const GLuint maskTextureID = clip.mask.getTextureID();

            if (fill.isColour())
            {
                state.blendMode.setPremultipliedBlendingMode (state.shaderQuadQueue);
                state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
                state.activeTextures.bindTexture (maskTextureID);

                state.setShader (state.currentShader.programs->solidColourMasked);
                state.currentShader.programs->solidColourMasked.maskParams.setBounds (clip.maskArea, state.target, 0);
            }
            else if (fill.isGradient())
            {
                state.setShaderForGradientFill (*fill.gradient, fill.transform, maskTextureID, &clip.maskArea);
            }
            else
            {
                jassert (fill.isTiledImage());
                image = new OpenGLTextureFromImage (fill.image);
                state.setShaderForTiledImageFill (*image, fill.transform, maskTextureID, &clip.maskArea, clampTiledImages);
            }
        }

        ~ShaderFillOperation()
        {
            state.shaderQuadQueue.flush();
        }

        OpenGLGraphicsContext::GLState& state;
        ScopedPointer<OpenGLTextureFromImage> image;

        JUCE_DECLARE_NON_COPYABLE (ShaderFillOperation);
    };

    struct TargetSaver
    {
        TargetSaver (const OpenGLContext& context_)
            : context (context_), oldFramebuffer (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
        {
            glGetIntegerv (GL_VIEWPORT, oldViewport);
        }

        ~TargetSaver()
        {
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, oldFramebuffer);
            glViewport (oldViewport[0], oldViewport[1], oldViewport[2], oldViewport[3]);
        }

    private:
        const OpenGLContext& context;
        GLuint oldFramebuffer;
        GLint oldViewport[4];

        TargetSaver& operator= (const TargetSaver&);
    };

    void makeActive()
    {
        state.shaderQuadQueue.flush();
        state.activeTextures.clear();
        mask.makeCurrentRenderingTarget();
        glViewport (0, 0, maskArea.getWidth(), maskArea.getHeight());
    }

    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (ClipRegion_Mask_Shader& owner_, const FillType& fill_) noexcept
            : owner (owner_), originalColour (fill_.colour.getPixelARGB())
        {}

        void operator() (int x, int y, int w, int h, const int alpha) noexcept
        {
            if (owner.clip.intersectRectangle (x, y, w, h))
            {
                PixelARGB col (originalColour);
                col.multiplyAlpha (alpha);
                owner.state.shaderQuadQueue.add (x, y, w, h, col);
            }
        }

    private:
        ClipRegion_Mask_Shader& owner;
        const PixelARGB originalColour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };

    ClipRegion_Mask_Shader& operator= (const ClipRegion_Mask_Shader&);
};

//==============================================================================
class ClipRegion_RectangleList_Shaders  : public ClipRegion_RectangleListBase
{
public:
    ClipRegion_RectangleList_Shaders (OpenGLGraphicsContext::GLState& state_, const Rectangle<int>& r) noexcept
        : ClipRegion_RectangleListBase (state_, r)
    {}

    ClipRegion_RectangleList_Shaders (OpenGLGraphicsContext::GLState& state_, const RectangleList& r) noexcept
        : ClipRegion_RectangleListBase (state_, r)
    {}

    Ptr clone() const       { return new ClipRegion_RectangleList_Shaders (state, clip); }

    Ptr clipToTexture (const PositionedTexture& t)                                                  { return toMask()->clipToTexture (t); }
    Ptr clipToPath (const Path& p, const AffineTransform& transform)                                { return toMask()->clipToPath (p, transform); }
    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)    { return toMask()->clipToImageAlpha (image, transform); }

    void fillRect (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        ShaderFillOperation fillOp (*this, fill, replaceContents || fill.colour.isOpaque(), false);
        state.shaderQuadQueue.add (clip, area, fill.colour.getPixelARGB());
    }

    void fillRect (const Rectangle<float>& area, const FillType& fill)
    {
        const PixelARGB colour (fill.colour.getPixelARGB());
        ShaderFillOperation fillOp (*this, fill, false, false);

        for (RectangleList::Iterator i (clip); i.next();)
        {
            const Rectangle<float> r (i.getRectangle()->toFloat().getIntersection (area));
            if (! r.isEmpty())
                state.shaderQuadQueue.add (r, colour);
        }
    }

    void drawImage (const Image& image, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, EdgeTable* et)
    {
        FillType fill (image, transform);
        const PixelARGB colour (Colours::white.withAlpha (alpha).getPixelARGB());

        ShaderFillOperation fillOp (*this, fill, false, true);

        if (et != nullptr)
        {
            if (! clip.containsRectangle (et->getMaximumBounds()))
                et->clipToEdgeTable (EdgeTable (clip));

            state.shaderQuadQueue.add (*et, colour);
        }
        else
        {
            state.shaderQuadQueue.add (clip, clipArea, colour);
        }

        state.currentShader.clearShader (state.shaderQuadQueue);
    }

    void fillEdgeTable (EdgeTable& et, const FillType& fill)
    {
        if (clip.intersects (et.getMaximumBounds()))
        {
            if (! clip.containsRectangle (et.getMaximumBounds()))
                et.clipToEdgeTable (EdgeTable (clip));

            ShaderFillOperation fillOp (*this, fill, false, true);
            state.shaderQuadQueue.add (et, fill.colour.getPixelARGB());
        }
    }

private:
    Ptr toMask() const    { return new ClipRegion_Mask_Shader (state, clip); }

    struct ShaderFillOperation
    {
        ShaderFillOperation (const ClipRegion_RectangleList_Shaders& clip, const FillType& fill,
                             const bool replaceContents, const bool clampTiledImages)
            : state (clip.state)
        {
            if (fill.isColour())
            {
                state.activeTextures.disableTextures (state.shaderQuadQueue);
                state.blendMode.setBlendMode (state.shaderQuadQueue, replaceContents);
                state.setShader (state.currentShader.programs->solidColourProgram);
            }
            else if (fill.isGradient())
            {
                state.setShaderForGradientFill (*fill.gradient, fill.transform, 0, nullptr);
            }
            else
            {
                jassert (fill.isTiledImage());
                image = new OpenGLTextureFromImage (fill.image);
                state.setShaderForTiledImageFill (*image, fill.transform, 0, nullptr, clampTiledImages);
            }
        }

        ~ShaderFillOperation()
        {
            if (image != nullptr)
                state.shaderQuadQueue.flush();
        }

        OpenGLGraphicsContext::GLState& state;
        ScopedPointer<OpenGLTextureFromImage> image;

        JUCE_DECLARE_NON_COPYABLE (ShaderFillOperation);
    };

    JUCE_DECLARE_NON_COPYABLE (ClipRegion_RectangleList_Shaders);
};
#endif

//==============================================================================
class OpenGLGraphicsContext::SavedState
{
public:
    SavedState (OpenGLGraphicsContext::GLState* const state_)
        : clip (createRectangleClip (*state_, state_->target.bounds)),
          transform (0, 0), interpolationQuality (Graphics::mediumResamplingQuality),
          state (state_), transparencyLayerAlpha (1.0f)
    {
    }

    SavedState (const SavedState& other)
        : clip (other.clip), transform (other.transform), font (other.font),
          fillType (other.fillType), interpolationQuality (other.interpolationQuality),
          state (other.state), transparencyLayerAlpha (other.transparencyLayerAlpha),
          transparencyLayer (other.transparencyLayer), previousTarget (other.previousTarget.createCopy())
    {
    }

    static ClipRegionBase* createRectangleClip (OpenGLGraphicsContext::GLState& state, const Rectangle<int>& clip)
    {
       #if JUCE_USE_OPENGL_SHADERS
        if (state.currentShader.canUseShaders)
            return new ClipRegion_RectangleList_Shaders (state, clip);
       #endif

       #if JUCE_USE_OPENGL_FIXED_FUNCTION
        return new ClipRegion_RectangleList (state, clip);
       #else
        // there's no shader hardware, but we're compiling without the fixed-function pipeline available!
        jassertfalse;
        return nullptr;
       #endif
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

            state->flush();
            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->previousTarget = new OpenGLTarget (state->target);
            state->target = OpenGLTarget (state->target.context, *OpenGLImageType::getFrameBufferFrom (s->transparencyLayer), clipBounds.getPosition());
            s->transparencyLayerAlpha = opacity;
            s->cloneClipIfMultiplyReferenced();

            s->state->target.makeActiveFor2D();
        }

        return s;
    }

    void endTransparencyLayer (SavedState& finishedLayerState)
    {
        if (clip != nullptr)
        {
            jassert (finishedLayerState.previousTarget != nullptr);

            state->flush();
            state->target = *finishedLayerState.previousTarget;
            finishedLayerState.previousTarget = nullptr;

            state->target.makeActiveFor2D();
            const Rectangle<int> clipBounds (clip->getClipBounds());

            clip->drawImage (finishedLayerState.transparencyLayer,
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
                clip->fillRect (r.translated (transform.xOffset, transform.yOffset),
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
                const Rectangle<float> c (r.translated ((float) transform.xOffset, (float) transform.yOffset)
                                           .getIntersection (clip->getClipBounds().toFloat()));

                if (! c.isEmpty())
                    clip->fillRect (c, getFillType());
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
                RenderingHelpers::GlyphCache <RenderingHelpers::CachedGlyphEdgeTable <OpenGLGraphicsContext::SavedState>, SavedState>::getInstance()
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

                clip->drawImage (image, t, alpha, Rectangle<int> (tx, ty, image.getWidth(), image.getHeight()), nullptr);
                return;
            }
        }

        if (! t.isSingularity())
        {
            Path p;
            p.addRectangle (image.getBounds());
            EdgeTable et (clipBounds, p, t);

            clip->drawImage (image, t, alpha, clipBounds, &et);
        }
    }

    void setFillType (const FillType& newFill)
    {
        fillType = newFill;
        state->textureCache.resetGradient();
    }

    //==============================================================================
    ClipRegionBase::Ptr clip;
    RenderingHelpers::TranslationOrTransform transform;
    Font font;
    FillType fillType;
    Graphics::ResamplingQuality interpolationQuality;
    GLState* state;

private:
    float transparencyLayerAlpha;
    Image transparencyLayer;
    ScopedPointer<OpenGLTarget> previousTarget;

    void cloneClipIfMultiplyReferenced()
    {
        if (clip->getReferenceCount() > 1)
            clip = clip->clone();
    }

    FillType getFillType() const
    {
        return fillType.transformed (transform.getTransform());
    }

    void fillEdgeTable (EdgeTable& et) const
    {
        clip->fillEdgeTable (et, getFillType());
    }

    SavedState& operator= (const SavedState&);
};

//==============================================================================
OpenGLGraphicsContext::OpenGLGraphicsContext (OpenGLComponent& target)
    : glState (new GLState (OpenGLTarget (*target.getCurrentContext(), target.getFrameBufferID(), target.getWidth(), target.getHeight()))),
      stack (new SavedState (glState))
{
    jassert (target.getCurrentContext() != nullptr); // must have a valid context when this is called!
}

OpenGLGraphicsContext::OpenGLGraphicsContext (OpenGLContext& context, OpenGLFrameBuffer& target)
    : glState (new GLState (OpenGLTarget (context, target, Point<int>()))),
      stack (new SavedState (glState))
{
}

OpenGLGraphicsContext::OpenGLGraphicsContext (OpenGLContext& context, unsigned int frameBufferID, int width, int height)
    : glState (new GLState (OpenGLTarget (context, frameBufferID, width, height))),
      stack (new SavedState (glState))
{
}

OpenGLGraphicsContext::~OpenGLGraphicsContext() {}

bool OpenGLGraphicsContext::isVectorDevice() const                                         { return false; }
void OpenGLGraphicsContext::setOrigin (int x, int y)                                       { stack->transform.setOrigin (x, y); }
void OpenGLGraphicsContext::addTransform (const AffineTransform& t)                        { stack->transform.addTransform (t); }
float OpenGLGraphicsContext::getScaleFactor()                                              { return stack->transform.getScaleFactor(); }
Rectangle<int> OpenGLGraphicsContext::getClipBounds() const                                { return stack->getClipBounds(); }
bool OpenGLGraphicsContext::isClipEmpty() const                                            { return stack->clip == nullptr; }
bool OpenGLGraphicsContext::clipRegionIntersects (const Rectangle<int>& r)                 { return stack->clipRegionIntersects (r); }
bool OpenGLGraphicsContext::clipToRectangle (const Rectangle<int>& r)                      { return stack->clipToRectangle (r); }
bool OpenGLGraphicsContext::clipToRectangleList (const RectangleList& r)                   { return stack->clipToRectangleList (r); }
void OpenGLGraphicsContext::excludeClipRectangle (const Rectangle<int>& r)                 { stack->excludeClipRectangle (r); }
void OpenGLGraphicsContext::clipToPath (const Path& path, const AffineTransform& t)        { stack->clipToPath (path, t); }
void OpenGLGraphicsContext::clipToImageAlpha (const Image& im, const AffineTransform& t)   { stack->clipToImageAlpha (im, t); }
void OpenGLGraphicsContext::saveState()                                                    { stack.save(); }
void OpenGLGraphicsContext::restoreState()                                                 { stack.restore(); }
void OpenGLGraphicsContext::beginTransparencyLayer (float opacity)                         { stack.beginTransparencyLayer (opacity); }
void OpenGLGraphicsContext::endTransparencyLayer()                                         { stack.endTransparencyLayer(); }
void OpenGLGraphicsContext::setFill (const FillType& fillType)                             { stack->setFillType (fillType); }
void OpenGLGraphicsContext::setOpacity (float newOpacity)                                  { stack->fillType.setOpacity (newOpacity); }
void OpenGLGraphicsContext::setInterpolationQuality (Graphics::ResamplingQuality quality)  { stack->interpolationQuality = quality; }
void OpenGLGraphicsContext::fillRect (const Rectangle<int>& r, bool replace)               { stack->fillRect (r, replace); }
void OpenGLGraphicsContext::fillPath (const Path& path, const AffineTransform& t)          { stack->fillPath (path, t); }
void OpenGLGraphicsContext::drawImage (const Image& im, const AffineTransform& t)          { stack->drawImage (im, t); }
void OpenGLGraphicsContext::drawVerticalLine (int x, float top, float bottom)              { if (top < bottom) stack->fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top)); }
void OpenGLGraphicsContext::drawHorizontalLine (int y, float left, float right)            { if (left < right) stack->fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f)); }
void OpenGLGraphicsContext::drawGlyph (int glyphNumber, const AffineTransform& t)          { stack->drawGlyph (glyphNumber, t); }
void OpenGLGraphicsContext::drawLine (const Line <float>& line)                            { stack->drawLine (line); }
void OpenGLGraphicsContext::setFont (const Font& newFont)                                  { stack->font = newFont; }
const Font& OpenGLGraphicsContext::getFont()                                               { return stack->font; }
