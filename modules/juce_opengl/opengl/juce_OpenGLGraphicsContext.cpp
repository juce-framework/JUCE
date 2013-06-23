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

namespace OpenGLRendering
{

struct Target
{
    Target (OpenGLContext& c, GLuint frameBufferID_, int width, int height) noexcept
        : context (c), frameBufferID (frameBufferID_), bounds (width, height)
    {}

    Target (OpenGLContext& c, OpenGLFrameBuffer& fb, Point<int> origin) noexcept
        : context (c), frameBufferID (fb.getFrameBufferID()),
          bounds (origin.x, origin.y, fb.getWidth(), fb.getHeight())
    {
        jassert (frameBufferID != 0); // trying to render into an uninitialised framebuffer object.
    }

    Target (const Target& other) noexcept
        : context (other.context), frameBufferID (other.frameBufferID), bounds (other.bounds)
    {}

    Target& operator= (const Target& other) noexcept
    {
        frameBufferID = other.frameBufferID;
        bounds = other.bounds;
        return *this;
    }

    void makeActive() const noexcept
    {
       #if JUCE_WINDOWS
        if (context.extensions.glBindFramebuffer != nullptr)
       #endif
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, frameBufferID);

        glViewport (0, 0, bounds.getWidth(), bounds.getHeight());
        glDisable (GL_DEPTH_TEST);
    }

    OpenGLContext& context;
    GLuint frameBufferID;
    Rectangle<int> bounds;
};

#if JUCE_USE_OPENGL_SHADERS

//==============================================================================
class PositionedTexture
{
public:
    PositionedTexture (OpenGLTexture& texture, const EdgeTable& et, const Rectangle<int>& clipRegion)
        : clip (clipRegion.getIntersection (et.getMaximumBounds()))
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

    PositionedTexture (GLuint texture, const Rectangle<int> r, const Rectangle<int> clipRegion) noexcept
        : textureID (texture), area (r), clip (clipRegion)
    {}

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

        JUCE_DECLARE_NON_COPYABLE (EdgeTableAlphaMap)
    };
};

//==============================================================================
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
            JUCE_CHECK_OPENGL_ERROR
            program.addShader ("attribute vec2 position;"
                               "attribute vec4 colour;"
                               "uniform vec4 screenBounds;"
                               "varying " JUCE_MEDIUMP " vec4 frontColour;"
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
            JUCE_CHECK_OPENGL_ERROR
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

        void setBounds (const Rectangle<int>& area, const Target& target, const GLint textureIndex) const
        {
            maskTexture.set (textureIndex);
            maskBounds.set (area.getX() - target.bounds.getX(),
                            area.getY() - target.bounds.getY(),
                            area.getWidth(), area.getHeight());
        }

        OpenGLShaderProgram::Uniform maskTexture, maskBounds;
    };

    //==============================================================================
    #define JUCE_DECLARE_VARYING_COLOUR   "varying " JUCE_MEDIUMP " vec4 frontColour;"
    #define JUCE_DECLARE_VARYING_PIXELPOS "varying " JUCE_HIGHP " vec2 pixelPos;"

    struct SolidColourProgram  : public ShaderBase
    {
        SolidColourProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_COLOUR
                          "void main() { gl_FragColor = frontColour; }")
        {}
    };

   #if JUCE_ANDROID
    #define JUCE_DECLARE_SWIZZLE_FUNCTION "\n" JUCE_MEDIUMP " vec4 swizzleRGBOrder (in " JUCE_MEDIUMP " vec4 c) { return vec4 (c.b, c.g, c.r, c.a); }\n"
   #else
    #define JUCE_DECLARE_SWIZZLE_FUNCTION "\n" JUCE_MEDIUMP " vec4 swizzleRGBOrder (in " JUCE_MEDIUMP " vec4 c) { return c; }\n"
   #endif

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
                          "void main() {"
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

        void setMatrix (const Point<float> p1, const Point<float> p2, const Point<float> p3)
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
    #define JUCE_GET_TEXTURE_COLOUR       "(frontColour.a * swizzleRGBOrder (texture2D (gradientTexture, vec2 (gradientPos, 0.5))))"

    struct RadialGradientProgram  : public ShaderBase
    {
        RadialGradientProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_PIXELPOS
                          JUCE_DECLARE_RADIAL_UNIFORMS JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_SWIZZLE_FUNCTION
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
                          JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
                                          "uniform " JUCE_MEDIUMP " vec4 gradientInfo;" \
                                          JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
    #define JUCE_CALC_LINEAR_GRAD_POS1    JUCE_MEDIUMP " float gradientPos = (pixelPos.y - (gradientInfo.y + (gradientInfo.z * (pixelPos.x - gradientInfo.x)))) / gradientInfo.w;"
    #define JUCE_CALC_LINEAR_GRAD_POS2    JUCE_MEDIUMP " float gradientPos = (pixelPos.x - (gradientInfo.x + (gradientInfo.z * (pixelPos.y - gradientInfo.y)))) / gradientInfo.w;"

    struct LinearGradient1Program  : public ShaderBase
    {
        LinearGradient1Program (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_LINEAR_UNIFORMS  // gradientInfo: x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          JUCE_DECLARE_SWIZZLE_FUNCTION
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
                          JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
                          JUCE_DECLARE_SWIZZLE_FUNCTION
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
                          JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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

            imageLimits.set (fullWidthProportion, fullHeightProportion);
        }

        void setMatrix (const AffineTransform& trans, const OpenGLTextureFromImage& im,
                        const float targetX, const float targetY) const
        {
            setMatrix (trans,
                       im.imageWidth, im.imageHeight,
                       im.fullWidthProportion, im.fullHeightProportion,
                       targetX, targetY);
        }

        OpenGLShaderProgram::Uniform imageTexture, matrix, imageLimits;
    };

    #define JUCE_DECLARE_IMAGE_UNIFORMS "uniform sampler2D imageTexture;" \
                                        "uniform " JUCE_MEDIUMP " vec2 imageLimits;" \
                                        JUCE_DECLARE_MATRIX_UNIFORM JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
    #define JUCE_GET_IMAGE_PIXEL        "swizzleRGBOrder (texture2D (imageTexture, vec2 (texturePos.x, 1.0 - texturePos.y)))"
    #define JUCE_CLAMP_TEXTURE_COORD    JUCE_HIGHP " vec2 texturePos = clamp (" JUCE_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits);"
    #define JUCE_MOD_TEXTURE_COORD      JUCE_HIGHP " vec2 texturePos = mod (" JUCE_MATRIX_TIMES_FRAGCOORD ", imageLimits);"

    struct ImageProgram  : public ShaderBase
    {
        ImageProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_MASK_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
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
            : ShaderBase (context, JUCE_DECLARE_IMAGE_UNIFORMS JUCE_DECLARE_SWIZZLE_FUNCTION
                          "void main()"
                          "{"
                            JUCE_HIGHP " vec2 texturePos = " JUCE_MATRIX_TIMES_FRAGCOORD ";"
                            JUCE_HIGHP " float roundingError = 0.00001;"
                            "if (texturePos.x >= -roundingError"
                                 "&& texturePos.y >= -roundingError"
                                 "&& texturePos.x <= imageLimits.x + roundingError"
                                 "&& texturePos.y <= imageLimits.y + roundingError)"
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

//==============================================================================
struct StateHelpers
{
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
    template <class QuadQueueType>
    struct EdgeTableRenderer
    {
        EdgeTableRenderer (QuadQueueType& q, const PixelARGB c) noexcept
            : quadQueue (q), colour (c)
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

        JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer)
    };

    template <class QuadQueueType>
    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (QuadQueueType& q, const PixelARGB c) noexcept
            : quadQueue (q), colour (c)
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

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer)
    };

    //==============================================================================
    struct ActiveTextures
    {
        ActiveTextures (const OpenGLContext& c) noexcept
            : texturesEnabled (0), currentActiveTexture (-1), context (c)
        {}

        void clear() noexcept
        {
            zeromem (currentTextureID, sizeof (currentTextureID));
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
                        JUCE_CHECK_OPENGL_ERROR

                       #if ! JUCE_ANDROID
                        if ((textureIndexMask & (1 << i)) != 0)
                            glEnable (GL_TEXTURE_2D);
                        else
                        {
                            glDisable (GL_TEXTURE_2D);
                            currentTextureID[i] = 0;
                        }

                        clearGLError();
                       #endif
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
            JUCE_CHECK_OPENGL_ERROR
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
            JUCE_CHECK_OPENGL_ERROR
        }

        void setActiveTexture (const int index) noexcept
        {
            if (currentActiveTexture != index)
            {
                currentActiveTexture = index;
                context.extensions.glActiveTexture (GL_TEXTURE0 + index);
                JUCE_CHECK_OPENGL_ERROR
            }
        }

        void bindTexture (const GLuint textureID) noexcept
        {
            jassert (currentActiveTexture >= 0);

            if (currentTextureID [currentActiveTexture] != textureID)
            {
                currentTextureID [currentActiveTexture] = textureID;
                glBindTexture (GL_TEXTURE_2D, textureID);
                JUCE_CHECK_OPENGL_ERROR
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

                JUCE_CHECK_OPENGL_ERROR;
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

    //==============================================================================
    struct ShaderQuadQueue
    {
        ShaderQuadQueue (const OpenGLContext& c) noexcept
            : context (c), numVertices (0)
        {}

        ~ShaderQuadQueue() noexcept
        {
            static_jassert (sizeof (VertexInfo) == 8);
            context.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
            context.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
            context.extensions.glDeleteBuffers (2, buffers);
        }

        void initialise() noexcept
        {
            JUCE_CHECK_OPENGL_ERROR
            for (int i = 0, v = 0; i < numQuads * 6; i += 6, v += 4)
            {
                indexData[i] = (GLushort) v;
                indexData[i + 1] = indexData[i + 3] = (GLushort) (v + 1);
                indexData[i + 2] = indexData[i + 4] = (GLushort) (v + 2);
                indexData[i + 5] = (GLushort) (v + 3);
            }

            context.extensions.glGenBuffers (2, buffers);
            context.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, buffers[0]);
            context.extensions.glBufferData (GL_ELEMENT_ARRAY_BUFFER, sizeof (indexData), indexData, GL_STATIC_DRAW);
            context.extensions.glBindBuffer (GL_ARRAY_BUFFER, buffers[1]);
            context.extensions.glBufferData (GL_ARRAY_BUFFER, sizeof (vertexData), vertexData, GL_STREAM_DRAW);
            JUCE_CHECK_OPENGL_ERROR
        }

        void add (const int x, const int y, const int w, const int h, const PixelARGB colour) noexcept
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

        void add (const Rectangle<int>& r, const PixelARGB colour) noexcept
        {
            add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }

        void add (const Rectangle<float>& r, const PixelARGB colour) noexcept
        {
            FloatRectangleRenderer<ShaderQuadQueue> frr (*this, colour);
            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
        }

        void add (const RectangleList& list, const PixelARGB colour) noexcept
        {
            for (const Rectangle<int>* i = list.begin(), * const e = list.end(); i != e; ++i)
                add (*i, colour);
        }

        void add (const RectangleList& list, const Rectangle<int>& clip, const PixelARGB colour) noexcept
        {
            for (const Rectangle<int>* i = list.begin(), * const e = list.end(); i != e; ++i)
            {
                const Rectangle<int> r (i->getIntersection (clip));

                if (! r.isEmpty())
                    add (r, colour);
            }
        }

        void add (const EdgeTable& et, const PixelARGB colour)
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

       #if JUCE_MAC || JUCE_ANDROID || JUCE_IOS
        enum { numQuads = 256 };
       #else
        enum { numQuads = 64 }; // (had problems with my drivers segfaulting when these buffers are any larger)
       #endif

        GLuint buffers[2];
        VertexInfo vertexData [numQuads * 4];
        GLushort indexData [numQuads * 6];
        const OpenGLContext& context;
        int numVertices;

        void draw() noexcept
        {
            context.extensions.glBufferSubData (GL_ARRAY_BUFFER, 0, numVertices * sizeof (VertexInfo), vertexData);
            // NB: If you get a random crash in here and are running in a Parallels VM, it seems to be a bug in
            // their driver.. Can't find a workaround unfortunately.
            glDrawElements (GL_TRIANGLES, (numVertices * 3) / 2, GL_UNSIGNED_SHORT, 0);
            JUCE_CHECK_OPENGL_ERROR
            numVertices = 0;
        }

        JUCE_DECLARE_NON_COPYABLE (ShaderQuadQueue)
    };

    //==============================================================================
    struct CurrentShader
    {
        CurrentShader (OpenGLContext& c) noexcept
            : context (c), activeShader (nullptr)
        {
            const char programValueID[] = "GraphicsContextPrograms";
            programs = static_cast <ShaderPrograms*> (context.getAssociatedObject (programValueID));

            if (programs == nullptr)
            {
                programs = new ShaderPrograms (context);
                context.setAssociatedObject (programValueID, programs);
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

                JUCE_CHECK_OPENGL_ERROR
            }
            else if (bounds != currentBounds)
            {
                currentBounds = bounds;
                shader.set2DBounds (bounds.toFloat());
            }
        }

        void setShader (Target& target, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
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

    private:
        ShaderPrograms::ShaderBase* activeShader;
        Rectangle<int> currentBounds;

        CurrentShader& operator= (const CurrentShader&);
    };
};

//==============================================================================
class GLState
{
public:
    GLState (const Target& t) noexcept
        : target (t),
          activeTextures (t.context),
          currentShader (t.context),
          shaderQuadQueue (t.context),
          previousFrameBufferTarget (OpenGLFrameBuffer::getCurrentFrameBufferTarget())
    {
        // This object can only be created and used when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        JUCE_CHECK_OPENGL_ERROR
        target.makeActive();
        blendMode.resync();
        JUCE_CHECK_OPENGL_ERROR
        activeTextures.clear();
        shaderQuadQueue.initialise();
        JUCE_CHECK_OPENGL_ERROR
    }

    ~GLState()
    {
        flush();
        target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
    }

    void flush()
    {
        currentShader.clearShader (shaderQuadQueue);
        shaderQuadQueue.flush();
        JUCE_CHECK_OPENGL_ERROR
    }

    void setShader (ShaderPrograms::ShaderBase& shader)
    {
        currentShader.setShader (target, shaderQuadQueue, shader);
        JUCE_CHECK_OPENGL_ERROR
    }

    void setShaderForGradientFill (const ColourGradient& g, const AffineTransform& transform,
                                   const int maskTextureID, const Rectangle<int>* const maskArea)
    {
        JUCE_CHECK_OPENGL_ERROR
        activeTextures.disableTextures (shaderQuadQueue);
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);
        JUCE_CHECK_OPENGL_ERROR

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

        JUCE_CHECK_OPENGL_ERROR
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
                maskParams  = &programs->imageMasked.maskParams;
            }
            else
            {
                setShader (programs->tiledImageMasked);
                imageParams = &programs->tiledImageMasked.imageParams;
                maskParams  = &programs->tiledImageMasked.maskParams;
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

    Target target;

    StateHelpers::BlendingMode blendMode;
    StateHelpers::ActiveTextures activeTextures;
    StateHelpers::TextureCache textureCache;
    StateHelpers::CurrentShader currentShader;
    StateHelpers::ShaderQuadQueue shaderQuadQueue;

private:
    GLuint previousFrameBufferTarget;
};

//==============================================================================
class ClipRegionBase  : public SingleThreadedReferenceCountedObject
{
public:
    ClipRegionBase (GLState& state_) noexcept : state (state_) {}
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

    GLState& state;

    JUCE_DECLARE_NON_COPYABLE (ClipRegionBase)
};


//==============================================================================
class ClipRegion_Mask  : public ClipRegionBase
{
public:
    ClipRegion_Mask (const ClipRegion_Mask& other)
        : ClipRegionBase (other.state),
          clip (other.clip),
          maskArea (other.clip)
    {
        OpenGLTargetSaver ts (state.target.context);
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

    ClipRegion_Mask (GLState& state_, const RectangleList& r)
        : ClipRegionBase (state_),
          clip (r.getBounds()),
          maskArea (clip)
    {
        OpenGLTargetSaver ts (state.target.context);
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
            return Ptr();

        RectangleList excluded (clip);

        if (excluded.subtract (r))
        {
            if (excluded.getNumRectangles() == 1)
                return excludeClipRectangle (excluded.getRectangle (0));

            OpenGLTargetSaver ts (state.target.context);
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
            return Ptr();

        OpenGLTargetSaver ts (state.target.context);
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
            OpenGLTargetSaver ts (state.target.context);
            state.currentShader.clearShader (state.shaderQuadQueue);
            state.shaderQuadQueue.flush();
            state.activeTextures.clear();

            OpenGLTexture texture;
            PositionedTexture pt (texture, et, clip);
            return clipToTexture (pt);
        }

        return Ptr();
    }

    Ptr clipToTexture (const PositionedTexture& pt)
    {
        clip = clip.getIntersection (pt.clip);

        if (clip.isEmpty())
            return Ptr();

        OpenGLTargetSaver ts (state.target.context);
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
        OpenGLTargetSaver ts (state.target.context);
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
        (void) replaceContents; jassert (! replaceContents);
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
        ShaderFillOperation (const ClipRegion_Mask& clipMask, const FillType& fill, const bool clampTiledImages)
            : state (clipMask.state)
        {
            const GLuint maskTextureID = clipMask.mask.getTextureID();

            if (fill.isColour())
            {
                state.blendMode.setPremultipliedBlendingMode (state.shaderQuadQueue);
                state.activeTextures.setSingleTextureMode (state.shaderQuadQueue);
                state.activeTextures.bindTexture (maskTextureID);

                state.setShader (state.currentShader.programs->solidColourMasked);
                state.currentShader.programs->solidColourMasked.maskParams.setBounds (clipMask.maskArea, state.target, 0);
            }
            else if (fill.isGradient())
            {
                state.setShaderForGradientFill (*fill.gradient, fill.transform, maskTextureID, &clipMask.maskArea);
            }
            else
            {
                jassert (fill.isTiledImage());
                image = new OpenGLTextureFromImage (fill.image);
                state.setShaderForTiledImageFill (*image, fill.transform, maskTextureID, &clipMask.maskArea, clampTiledImages);
            }
        }

        ~ShaderFillOperation()
        {
            state.shaderQuadQueue.flush();
        }

        GLState& state;
        ScopedPointer<OpenGLTextureFromImage> image;

        JUCE_DECLARE_NON_COPYABLE (ShaderFillOperation)
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
        FloatRectangleRenderer (ClipRegion_Mask& owner_, const FillType& fill_) noexcept
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
        ClipRegion_Mask& owner;
        const PixelARGB originalColour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer)
    };

    ClipRegion_Mask& operator= (const ClipRegion_Mask&);
};

//==============================================================================
class ClipRegion_RectangleList  : public ClipRegionBase
{
public:
    ClipRegion_RectangleList (GLState& state_, const Rectangle<int>& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    ClipRegion_RectangleList (GLState& state_, const RectangleList& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    Ptr clone() const       { return new ClipRegion_RectangleList (state, clip); }

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

        for (const Rectangle<int>* i = clip.begin(), * const e = clip.end(); i != e; ++i)
        {
            const Rectangle<float> r (i->toFloat().getIntersection (area));
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

    Rectangle<int> getClipBounds() const                { return clip.getBounds(); }
    Ptr clipToRectangle (const Rectangle<int>& r)       { return clip.clipTo (r) ? this : nullptr; }
    Ptr clipToRectangleList (const RectangleList& r)    { return clip.clipTo (r) ? this : nullptr; }
    Ptr excludeClipRectangle (const Rectangle<int>& r)  { clip.subtract (r); return clip.isEmpty() ? nullptr : this; }

private:
    RectangleList clip;

    Ptr toMask() const    { return new ClipRegion_Mask (state, clip); }

    struct ShaderFillOperation
    {
        ShaderFillOperation (const ClipRegion_RectangleList& clipList, const FillType& fill,
                             const bool replaceContents, const bool clampTiledImages)
            : state (clipList.state)
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
                state.shaderQuadQueue.flush();
                image = new OpenGLTextureFromImage (fill.image);
                state.setShaderForTiledImageFill (*image, fill.transform, 0, nullptr, clampTiledImages);
            }
        }

        ~ShaderFillOperation()
        {
            if (image != nullptr)
                state.shaderQuadQueue.flush();
        }

        GLState& state;
        ScopedPointer<OpenGLTextureFromImage> image;

        JUCE_DECLARE_NON_COPYABLE (ShaderFillOperation)
    };

    JUCE_DECLARE_NON_COPYABLE (ClipRegion_RectangleList)
};

//==============================================================================
class SavedState
{
public:
    SavedState (GLState* const state_)
        : clip (new ClipRegion_RectangleList (*state_, state_->target.bounds)),
          transform (0, 0), interpolationQuality (Graphics::mediumResamplingQuality),
          state (state_), transparencyLayerAlpha (1.0f)
    {}

    SavedState (const SavedState& other)
        : clip (other.clip), transform (other.transform), font (other.font),
          fillType (other.fillType), interpolationQuality (other.interpolationQuality),
          state (other.state), transparencyLayerAlpha (other.transparencyLayerAlpha),
          transparencyLayer (other.transparencyLayer), previousTarget (other.previousTarget.createCopy())
    {}

    bool clipToRectangle (const Rectangle<int>& r)
    {
        if (clip != nullptr)
        {
            if (transform.isOnlyTranslated)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.translated (r));
            }
            else if (transform.isIntegerScaling)
            {
                cloneClipIfMultiplyReferenced();
                clip = clip->clipToRectangle (transform.transformed (r).getSmallestIntegerContainer());
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
            else if (transform.isIntegerScaling)
            {
                cloneClipIfMultiplyReferenced();
                RectangleList scaledList;

                for (const Rectangle<int>* i = r.begin(), * const e = r.end(); i != e; ++i)
                    scaledList.add (transform.transformed (*i).getSmallestIntegerContainer());

                clip = clip->clipToRectangleList (scaledList);
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
            else if (transform.isIntegerScaling)
            {
                clip = clip->excludeClipRectangle (transform.transformed (r).getSmallestIntegerContainer());
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
        SavedState* const s = new SavedState (*this);

        if (clip != nullptr)
        {
            const Rectangle<int> clipBounds (clip->getClipBounds());

            state->flush();
            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->previousTarget = new Target (state->target);
            state->target = Target (state->target.context, *OpenGLImageType::getFrameBufferFrom (s->transparencyLayer), clipBounds.getPosition());
            s->transparencyLayerAlpha = opacity;
            s->cloneClipIfMultiplyReferenced();

            s->state->target.makeActive();
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

            state->target.makeActive();
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
                RenderingHelpers::GlyphCache <RenderingHelpers::CachedGlyphEdgeTable <SavedState>, SavedState>::getInstance()
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

        const AffineTransform t (transform.getTransformWith (trans));
        if (t.isSingularity())
            return;

        const float alpha = fillType.colour.getFloatAlpha();

        float px0 = 0, py0 = 0;
        float px1 = (float) image.getWidth(), py1 = 0;
        float px2 = 0, py2 = (float) image.getHeight();
        t.transformPoints (px0, py0, px1, py1, px2, py2);

        const int ix0 = (int) (px0 * 256.0f);
        const int iy0 = (int) (py0 * 256.0f);
        const int ix1 = (int) (px1 * 256.0f);
        const int iy1 = (int) (py1 * 256.0f);
        const int ix2 = (int) (px2 * 256.0f);
        const int iy2 = (int) (py2 * 256.0f);

        if (((ix0 | iy0 | ix1 | iy1 | ix2 | iy2) & 0xf8) == 0
              && ix0 == ix2 && iy0 == iy1)
        {
            // Non-warping transform can be done as a single rectangle.
            clip->drawImage (image, t, alpha,
                             Rectangle<int> (Point<int> (((ix0 + 128) >> 8),
                                                         ((iy0 + 128) >> 8)),
                                             Point<int> (((ix1 + 128) >> 8),
                                                         ((iy2 + 128) >> 8))), nullptr);
        }
        else
        {
            Path p;
            p.addRectangle (image.getBounds());

            const Rectangle<int> clipBounds (clip->getClipBounds());
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
    ScopedPointer<Target> previousTarget;

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
class ShaderContext   : public LowLevelGraphicsContext
{
public:
    ShaderContext (const Target& target)
       : glState (target), stack (new SavedState (&glState))
    {}

    bool isVectorDevice() const                                         { return false; }
    void setOrigin (int x, int y)                                       { stack->transform.setOrigin (x, y); }
    void addTransform (const AffineTransform& t)                        { stack->transform.addTransform (t); }
    float getScaleFactor()                                              { return stack->transform.getScaleFactor(); }
    Rectangle<int> getClipBounds() const                                { return stack->getClipBounds(); }
    bool isClipEmpty() const                                            { return stack->clip == nullptr; }
    bool clipRegionIntersects (const Rectangle<int>& r)                 { return stack->clipRegionIntersects (r); }
    bool clipToRectangle (const Rectangle<int>& r)                      { return stack->clipToRectangle (r); }
    bool clipToRectangleList (const RectangleList& r)                   { return stack->clipToRectangleList (r); }
    void excludeClipRectangle (const Rectangle<int>& r)                 { stack->excludeClipRectangle (r); }
    void clipToPath (const Path& path, const AffineTransform& t)        { stack->clipToPath (path, t); }
    void clipToImageAlpha (const Image& im, const AffineTransform& t)   { stack->clipToImageAlpha (im, t); }
    void saveState()                                                    { stack.save(); }
    void restoreState()                                                 { stack.restore(); }
    void beginTransparencyLayer (float opacity)                         { stack.beginTransparencyLayer (opacity); }
    void endTransparencyLayer()                                         { stack.endTransparencyLayer(); }
    void setFill (const FillType& fillType)                             { stack->setFillType (fillType); }
    void setOpacity (float newOpacity)                                  { stack->fillType.setOpacity (newOpacity); }
    void setInterpolationQuality (Graphics::ResamplingQuality quality)  { stack->interpolationQuality = quality; }
    void fillRect (const Rectangle<int>& r, bool replace)               { stack->fillRect (r, replace); }
    void fillPath (const Path& path, const AffineTransform& t)          { stack->fillPath (path, t); }
    void drawImage (const Image& im, const AffineTransform& t)          { stack->drawImage (im, t); }
    void drawVerticalLine (int x, float top, float bottom)              { if (top < bottom) stack->fillRect (Rectangle<float> ((float) x, top, 1.0f, bottom - top)); }
    void drawHorizontalLine (int y, float left, float right)            { if (left < right) stack->fillRect (Rectangle<float> (left, (float) y, right - left, 1.0f)); }
    void drawGlyph (int glyphNumber, const AffineTransform& t)          { stack->drawGlyph (glyphNumber, t); }
    void drawLine (const Line <float>& line)                            { stack->drawLine (line); }
    void setFont (const Font& newFont)                                  { stack->font = newFont; }
    const Font& getFont()                                               { return stack->font; }

private:
    GLState glState;
    RenderingHelpers::SavedStateStack<SavedState> stack;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShaderContext)
};

#endif

class NonShaderContext   : public LowLevelGraphicsSoftwareRenderer
{
public:
    NonShaderContext (const Target& target_, const Image& image_)
        : LowLevelGraphicsSoftwareRenderer (image_), target (target_), image (image_)
    {}

    ~NonShaderContext()
    {
        JUCE_CHECK_OPENGL_ERROR
        const GLuint previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();

       #if ! JUCE_ANDROID
        target.context.extensions.glActiveTexture (GL_TEXTURE0);
        glEnable (GL_TEXTURE_2D);
        clearGLError();
       #endif

        OpenGLTexture texture;
        texture.loadImage (image);
        texture.bind();

        target.makeActive();
        target.context.copyTexture (target.bounds, Rectangle<int> (texture.getWidth(),
                                                                   texture.getHeight()),
                                    target.bounds.getWidth(), target.bounds.getHeight(),
                                    false);
        glBindTexture (GL_TEXTURE_2D, 0);

       #if JUCE_WINDOWS
        if (target.context.extensions.glBindFramebuffer != nullptr)
       #endif
            target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);

        JUCE_CHECK_OPENGL_ERROR
    }

private:
    Target target;
    Image image;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NonShaderContext)
};

LowLevelGraphicsContext* createOpenGLContext (const Target&);
LowLevelGraphicsContext* createOpenGLContext (const Target& target)
{
   #if JUCE_USE_OPENGL_SHADERS
    if (target.context.areShadersAvailable())
        return new ShaderContext (target);
   #endif

    Image tempImage (Image::ARGB, target.bounds.getWidth(), target.bounds.getHeight(), true, SoftwareImageType());
    return new NonShaderContext (target, tempImage);
}

}

//==============================================================================
LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context, int width, int height)
{
    return createOpenGLGraphicsContext (context, context.getFrameBufferID(), width, height);
}

LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context, OpenGLFrameBuffer& target)
{
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, target, Point<int>()));
}

LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context, unsigned int frameBufferID, int width, int height)
{
    using namespace OpenGLRendering;
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, frameBufferID, width, height));
}
