/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
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

extern void (*clearOpenGLGlyphCache)(); // declared in juce_graphics

namespace OpenGLRendering
{

struct TextureInfo
{
    GLuint textureID;
    int imageWidth, imageHeight;
    float fullWidthProportion, fullHeightProportion;
};

//==============================================================================
// This list persists in the OpenGLContext, and will re-use cached textures which
// are created from Images.
struct CachedImageList  : public ReferenceCountedObject,
                          private ImagePixelData::Listener
{
    CachedImageList (OpenGLContext& c) noexcept
        : context (c), totalSize (0), maxCacheSize (c.getImageCacheSize()) {}

    static CachedImageList* get (OpenGLContext& c)
    {
        const char cacheValueID[] = "CachedImages";
        CachedImageList* list = static_cast<CachedImageList*> (c.getAssociatedObject (cacheValueID));

        if (list == nullptr)
        {
            list = new CachedImageList (c);
            c.setAssociatedObject (cacheValueID, list);
        }

        return list;
    }

    TextureInfo getTextureFor (const Image& image)
    {
        ImagePixelData* const pixelData = image.getPixelData();

        CachedImage* c = findCachedImage (pixelData);

        if (c == nullptr)
        {
            if (OpenGLFrameBuffer* const fb = OpenGLImageType::getFrameBufferFrom (image))
            {
                TextureInfo t;
                t.textureID = fb->getTextureID();
                t.imageWidth = image.getWidth();
                t.imageHeight = image.getHeight();
                t.fullWidthProportion  = 1.0f;
                t.fullHeightProportion = 1.0f;

                return t;
            }

            c = images.add (new CachedImage (*this, pixelData));
            totalSize += c->imageSize;

            while (totalSize > maxCacheSize && images.size() > 1 && totalSize > 0)
                removeOldestItem();
        }

        return c->getTextureInfo();
    }

    struct CachedImage
    {
        CachedImage (CachedImageList& list, ImagePixelData* im)
            : owner (list), pixelData (im),
              lastUsed (Time::getCurrentTime()),
              imageSize ((size_t) (im->width * im->height)),
              textureNeedsReloading (true)
        {
            pixelData->listeners.add (&owner);
        }

        ~CachedImage()
        {
            if (pixelData != nullptr)
                pixelData->listeners.remove (&owner);
        }

        TextureInfo getTextureInfo()
        {
            TextureInfo t;

            if (textureNeedsReloading)
            {
                textureNeedsReloading = false;
                texture.loadImage (Image (pixelData));
            }

            t.textureID = texture.getTextureID();
            t.imageWidth = pixelData->width;
            t.imageHeight = pixelData->height;
            t.fullWidthProportion  = t.imageWidth  / (float) texture.getWidth();
            t.fullHeightProportion = t.imageHeight / (float) texture.getHeight();

            lastUsed = Time::getCurrentTime();

            return t;
        }

        CachedImageList& owner;
        ImagePixelData* pixelData;
        OpenGLTexture texture;
        Time lastUsed;
        const size_t imageSize;
        bool textureNeedsReloading;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
    };

    typedef ReferenceCountedObjectPtr<CachedImageList> Ptr;

private:
    OpenGLContext& context;
    OwnedArray<CachedImage> images;
    size_t totalSize, maxCacheSize;

    bool canUseContext() const noexcept
    {
        return OpenGLContext::getCurrentContext() == &context;
    }

    void imageDataChanged (ImagePixelData* im) override
    {
        if (CachedImage* c = findCachedImage (im))
            c->textureNeedsReloading = true;
    }

    void imageDataBeingDeleted (ImagePixelData* im) override
    {
        for (int i = images.size(); --i >= 0;)
        {
            CachedImage& ci = *images.getUnchecked(i);

            if (ci.pixelData == im)
            {
                if (canUseContext())
                {
                    totalSize -= ci.imageSize;
                    images.remove (i);
                }
                else
                {
                    ci.pixelData = nullptr;
                }

                break;
            }
        }
    }

    CachedImage* findCachedImage (ImagePixelData* const pixelData) const
    {
        for (int i = 0; i < images.size(); ++i)
        {
            CachedImage* c = images.getUnchecked(i);

            if (c->pixelData == pixelData)
                return c;
        }

        return nullptr;
    }

    void removeOldestItem()
    {
        CachedImage* oldest = nullptr;

        for (int i = 0; i < images.size(); ++i)
        {
            CachedImage* c = images.getUnchecked(i);

            if (oldest == nullptr || c->lastUsed < oldest->lastUsed)
                oldest = c;
        }

        if (oldest != nullptr)
        {
            totalSize -= oldest->imageSize;
            images.removeObject (oldest);
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImageList)
};


//==============================================================================
struct Target
{
    Target (OpenGLContext& c, GLuint fbID, int width, int height) noexcept
        : context (c), frameBufferID (fbID), bounds (width, height)
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

//==============================================================================
struct PositionedTexture
{
    PositionedTexture (OpenGLTexture& texture, const EdgeTable& et, Rectangle<int> clipRegion)
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

    PositionedTexture (GLuint texture, Rectangle<int> r, Rectangle<int> clipRegion) noexcept
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
            memset (currentLine + x, (uint8) alphaLevel, (size_t) width);
        }

        inline void handleEdgeTableLineFull (int x, int width) const noexcept
        {
            memset (currentLine + x, 255, (size_t) width);
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLine (x, width, alphaLevel);
            }
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            while (--height >= 0)
            {
                setEdgeTableYPos (y++);
                handleEdgeTableLineFull (x, width);
            }
        }

        HeapBlock<uint8> data;
        const Rectangle<int> area;

    private:
        uint8* currentLine;

        JUCE_DECLARE_NON_COPYABLE (EdgeTableAlphaMap)
    };
};

//==============================================================================
struct ShaderPrograms  : public ReferenceCountedObject
{
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
        ShaderProgramHolder (OpenGLContext& context, const char* fragmentShader, const char* vertexShader)
            : program (context)
        {
            JUCE_CHECK_OPENGL_ERROR

            if (vertexShader == nullptr)
                vertexShader = "attribute vec2 position;"
                               "attribute vec4 colour;"
                               "uniform vec4 screenBounds;"
                               "varying " JUCE_MEDIUMP " vec4 frontColour;"
                               "varying " JUCE_HIGHP " vec2 pixelPos;"
                               "void main()"
                               "{"
                                 "frontColour = colour;"
                                 "vec2 adjustedPos = position - screenBounds.xy;"
                                 "pixelPos = adjustedPos;"
                                 "vec2 scaledPos = adjustedPos / screenBounds.zw;"
                                 "gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);"
                               "}";

            if (program.addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (vertexShader))
                 && program.addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (fragmentShader))
                 && program.link())
            {
                JUCE_CHECK_OPENGL_ERROR
            }
            else
            {
                lastError = program.getLastError();
            }
        }

        OpenGLShaderProgram program;
        String lastError;
    };

    struct ShaderBase   : public ShaderProgramHolder
    {
        ShaderBase (OpenGLContext& context, const char* fragmentShader, const char* vertexShader = nullptr)
            : ShaderProgramHolder (context, fragmentShader, vertexShader),
              positionAttribute (program, "position"),
              colourAttribute (program, "colour"),
              screenBounds (program, "screenBounds")
        {}

        void set2DBounds (Rectangle<float> bounds)
        {
            screenBounds.set (bounds.getX(), bounds.getY(), 0.5f * bounds.getWidth(), 0.5f * bounds.getHeight());
        }

        void bindAttributes (OpenGLContext& context)
        {
            context.extensions.glVertexAttribPointer ((GLuint) positionAttribute.attributeID, 2, GL_SHORT, GL_FALSE, 8, (void*) 0);
            context.extensions.glVertexAttribPointer ((GLuint) colourAttribute.attributeID, 4, GL_UNSIGNED_BYTE, GL_TRUE, 8, (void*) 4);
            context.extensions.glEnableVertexAttribArray ((GLuint) positionAttribute.attributeID);
            context.extensions.glEnableVertexAttribArray ((GLuint) colourAttribute.attributeID);
        }

        void unbindAttributes (OpenGLContext& context)
        {
            context.extensions.glDisableVertexAttribArray ((GLuint) positionAttribute.attributeID);
            context.extensions.glDisableVertexAttribArray ((GLuint) colourAttribute.attributeID);
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

        void setBounds (Rectangle<int> area, const Target& target, GLint textureIndex) const
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

        void setMatrix (Point<float> p1, Point<float> p2, Point<float> p3)
        {
            auto t = AffineTransform::fromTargetPoints (p1, Point<float>(),
                                                        p2, Point<float> (1.0f, 0.0f),
                                                        p3, Point<float> (0.0f, 1.0f));
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
                                          "uniform " JUCE_MEDIUMP " vec4 gradientInfo;" \
                                          JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
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

        void setMatrix (const AffineTransform& trans, int imageWidth, int imageHeight,
                        float fullWidthProportion, float fullHeightProportion,
                        float targetX, float targetY, bool isForTiling) const
        {
            auto t = trans.translated (-targetX, -targetY)
                          .inverted().scaled (fullWidthProportion / imageWidth,
                                              fullHeightProportion / imageHeight);

            const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            matrix.set (m, 6);

            if (isForTiling)
            {
                fullWidthProportion -= 0.5f / imageWidth;
                fullHeightProportion -= 0.5f / imageHeight;
            }

            imageLimits.set (fullWidthProportion, fullHeightProportion);
        }

        void setMatrix (const AffineTransform& trans, const TextureInfo& textureInfo,
                        float targetX, float targetY, bool isForTiling) const
        {
            setMatrix (trans,
                       textureInfo.imageWidth, textureInfo.imageHeight,
                       textureInfo.fullWidthProportion, textureInfo.fullHeightProportion,
                       targetX, targetY, isForTiling);
        }

        OpenGLShaderProgram::Uniform imageTexture, matrix, imageLimits;
    };

    #define JUCE_DECLARE_IMAGE_UNIFORMS "uniform sampler2D imageTexture;" \
                                        "uniform " JUCE_MEDIUMP " vec2 imageLimits;" \
                                        JUCE_DECLARE_MATRIX_UNIFORM JUCE_DECLARE_VARYING_COLOUR JUCE_DECLARE_VARYING_PIXELPOS
    #define JUCE_GET_IMAGE_PIXEL        "texture2D (imageTexture, vec2 (texturePos.x, 1.0 - texturePos.y))"
    #define JUCE_CLAMP_TEXTURE_COORD    JUCE_HIGHP " vec2 texturePos = clamp (" JUCE_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits);"
    #define JUCE_MOD_TEXTURE_COORD      JUCE_HIGHP " vec2 texturePos = mod (" JUCE_MATRIX_TIMES_FRAGCOORD ", imageLimits);"

    struct ImageProgram  : public ShaderBase
    {
        ImageProgram (OpenGLContext& context)
            : ShaderBase (context, JUCE_DECLARE_VARYING_COLOUR
                          "uniform sampler2D imageTexture;"
                          "varying " JUCE_HIGHP " vec2 texturePos;"
                          "void main()"
                          "{"
                            "gl_FragColor = frontColour.a * " JUCE_GET_IMAGE_PIXEL ";"
                          "}",
                          "uniform " JUCE_MEDIUMP " vec2 imageLimits;"
                          JUCE_DECLARE_MATRIX_UNIFORM
                          "attribute vec2 position;"
                          "attribute vec4 colour;"
                          "uniform vec4 screenBounds;"
                          "varying " JUCE_MEDIUMP " vec4 frontColour;"
                          "varying " JUCE_HIGHP " vec2 texturePos;"
                          "void main()"
                          "{"
                            "frontColour = colour;"
                            "vec2 adjustedPos = position - screenBounds.xy;"
                            "vec2 pixelPos = adjustedPos;"
                            "texturePos = clamp (" JUCE_MATRIX_TIMES_FRAGCOORD ", vec2 (0, 0), imageLimits);"
                            "vec2 scaledPos = adjustedPos / screenBounds.zw;"
                            "gl_Position = vec4 (scaledPos.x - 1.0, 1.0 - scaledPos.y, 0, 1.0);"
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

        template <typename QuadQueueType>
        void setPremultipliedBlendingMode (QuadQueueType& quadQueue) noexcept
        {
            setBlendFunc (quadQueue, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
        }

        template <typename QuadQueueType>
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

        template <typename QuadQueueType>
        void disableBlend (QuadQueueType& quadQueue) noexcept
        {
            if (blendingEnabled)
            {
                quadQueue.flush();
                blendingEnabled = false;
                glDisable (GL_BLEND);
            }
        }

        template <typename QuadQueueType>
        void setBlendMode (QuadQueueType& quadQueue, bool replaceExistingContents) noexcept
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
    template <typename QuadQueueType>
    struct EdgeTableRenderer
    {
        EdgeTableRenderer (QuadQueueType& q, PixelARGB c) noexcept
            : quadQueue (q), colour (c)
        {}

        void setEdgeTableYPos (int y) noexcept
        {
            currentY = y;
        }

        void handleEdgeTablePixel (int x, int alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, 1, 1, c);
        }

        void handleEdgeTablePixelFull (int x) noexcept
        {
            quadQueue.add (x, currentY, 1, 1, colour);
        }

        void handleEdgeTableLine (int x, int width, int alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, currentY, width, 1, c);
        }

        void handleEdgeTableLineFull (int x, int width) noexcept
        {
            quadQueue.add (x, currentY, width, 1, colour);
        }

        void handleEdgeTableRectangle (int x, int y, int width, int height, int alphaLevel) noexcept
        {
            auto c = colour;
            c.multiplyAlpha (alphaLevel);
            quadQueue.add (x, y, width, height, c);
        }

        void handleEdgeTableRectangleFull (int x, int y, int width, int height) noexcept
        {
            quadQueue.add (x, y, width, height, colour);
        }

    private:
        QuadQueueType& quadQueue;
        const PixelARGB colour;
        int currentY;

        JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer)
    };

    template <typename QuadQueueType>
    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (QuadQueueType& q, PixelARGB c) noexcept
            : quadQueue (q), colour (c)
        {}

        void operator() (int x, int y, int w, int h, int alpha) noexcept
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

        template <typename QuadQueueType>
        void setTexturesEnabled (QuadQueueType& quadQueue, int textureIndexMask) noexcept
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

        template <typename QuadQueueType>
        void disableTextures (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 0);
        }

        template <typename QuadQueueType>
        void setSingleTextureMode (QuadQueueType& quadQueue) noexcept
        {
            setTexturesEnabled (quadQueue, 1);
            setActiveTexture (0);
        }

        template <typename QuadQueueType>
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

        void setActiveTexture (int index) noexcept
        {
            if (currentActiveTexture != index)
            {
                currentActiveTexture = index;
                context.extensions.glActiveTexture ((GLenum) (GL_TEXTURE0 + index));
                JUCE_CHECK_OPENGL_ERROR
            }
        }

        void bindTexture (GLuint textureID) noexcept
        {
            jassert (currentActiveTexture >= 0);

            if (currentTextureID[currentActiveTexture] != textureID)
            {
                currentTextureID[currentActiveTexture] = textureID;
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
        GLuint currentTextureID[3];
        int texturesEnabled, currentActiveTexture;
        const OpenGLContext& context;

        ActiveTextures& operator= (const ActiveTextures&);
    };

    //==============================================================================
    struct TextureCache
    {
        TextureCache() noexcept {}

        OpenGLTexture* getTexture (ActiveTextures& activeTextures, int w, int h)
        {
            if (textures.size() < numTexturesToCache)
            {
                activeTextures.clear();
                return new OpenGLTexture();
            }

            for (int i = 0; i < numTexturesToCache - 2; ++i)
            {
                auto* t = textures.getUnchecked(i);

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
                PixelARGB lookup[gradientTextureSize];
                gradient.createLookupTable (lookup, gradientTextureSize);
                gradientTextures.getUnchecked (activeGradientIndex)->loadARGB (lookup, gradientTextureSize, 1);
            }

            activeTextures.bindTexture (gradientTextures.getUnchecked (activeGradientIndex)->getTextureID());
        }

        enum { gradientTextureSize = 256 };

    private:
        enum { numTexturesToCache = 8, numGradientTexturesToCache = 10 };
        OwnedArray<OpenGLTexture> textures, gradientTextures;
        int activeGradientIndex = 0;
        bool gradientNeedsRefresh = true;
    };

    //==============================================================================
    struct ShaderQuadQueue
    {
        ShaderQuadQueue (const OpenGLContext& c) noexcept  : context (c)
        {}

        ~ShaderQuadQueue() noexcept
        {
            static_assert (sizeof (VertexInfo) == 8, "Sanity check VertexInfo size");
            context.extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
            context.extensions.glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, 0);
            context.extensions.glDeleteBuffers (2, buffers);
        }

        void initialise() noexcept
        {
            JUCE_CHECK_OPENGL_ERROR

           #if JUCE_ANDROID || JUCE_IOS
            int numQuads = maxNumQuads;
           #else
            GLint maxIndices = 0;
            glGetIntegerv (GL_MAX_ELEMENTS_INDICES, &maxIndices);
            auto numQuads = jmin ((int) maxNumQuads, (int) maxIndices / 6);
            maxVertices = numQuads * 4 - 4;
           #endif

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

        void add (int x, int y, int w, int h, PixelARGB colour) noexcept
        {
            jassert (w > 0 && h > 0);

            auto* v = vertexData + numVertices;
            v[0].x = v[2].x = (GLshort) x;
            v[0].y = v[1].y = (GLshort) y;
            v[1].x = v[3].x = (GLshort) (x + w);
            v[2].y = v[3].y = (GLshort) (y + h);

           #if JUCE_BIG_ENDIAN
            auto rgba = (GLuint) ((colour.getRed() << 24) | (colour.getGreen() << 16)
                                | (colour.getBlue() << 8) |  colour.getAlpha());
           #else
            auto rgba = (GLuint) ((colour.getAlpha() << 24) | (colour.getBlue() << 16)
                                | (colour.getGreen() << 8) |  colour.getRed());
           #endif

            v[0].colour = rgba;
            v[1].colour = rgba;
            v[2].colour = rgba;
            v[3].colour = rgba;

            numVertices += 4;

            if (numVertices > maxVertices)
                draw();
        }

        void add (Rectangle<int> r, PixelARGB colour) noexcept
        {
            add (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }

        void add (Rectangle<float> r, PixelARGB colour) noexcept
        {
            FloatRectangleRenderer<ShaderQuadQueue> frr (*this, colour);
            RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
        }

        void add (const RectangleList<int>& list, PixelARGB colour) noexcept
        {
            for (auto& i : list)
                add (i, colour);
        }

        void add (const RectangleList<int>& list, Rectangle<int> clip, PixelARGB colour) noexcept
        {
            for (auto& i : list)
            {
                auto r = i.getIntersection (clip);

                if (! r.isEmpty())
                    add (r, colour);
            }
        }

        template <typename IteratorType>
        void add (const IteratorType& et, PixelARGB colour)
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

        enum { maxNumQuads = 256 };

        GLuint buffers[2];
        VertexInfo vertexData[maxNumQuads * 4];
        GLushort indexData[maxNumQuads * 6];
        const OpenGLContext& context;
        int numVertices = 0;

       #if JUCE_ANDROID || JUCE_IOS
        enum { maxVertices = maxNumQuads * 4 - 4 };
       #else
        int maxVertices = 0;
       #endif

        void draw() noexcept
        {
            context.extensions.glBufferSubData (GL_ARRAY_BUFFER, 0, (GLsizeiptr) ((size_t) numVertices * sizeof (VertexInfo)), vertexData);
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
        CurrentShader (OpenGLContext& c) noexcept  : context (c)
        {
            auto programValueID = "GraphicsContextPrograms";
            programs = static_cast<ShaderPrograms*> (context.getAssociatedObject (programValueID));

            if (programs == nullptr)
            {
                programs = new ShaderPrograms (context);
                context.setAssociatedObject (programValueID, programs);
            }
        }

        ~CurrentShader()
        {
            jassert (activeShader == nullptr);
        }

        void setShader (Rectangle<int> bounds, ShaderQuadQueue& quadQueue, ShaderPrograms::ShaderBase& shader)
        {
            if (activeShader != &shader)
            {
                clearShader (quadQueue);

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
        ShaderPrograms::ShaderBase* activeShader = nullptr;
        Rectangle<int> currentBounds;

        CurrentShader& operator= (const CurrentShader&);
    };
};

//==============================================================================
struct GLState
{
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
        cachedImageList = CachedImageList::get (t.context);
        JUCE_CHECK_OPENGL_ERROR
    }

    ~GLState()
    {
        flush();
        target.context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
    }

    void flush()
    {
        shaderQuadQueue.flush();
        currentShader.clearShader (shaderQuadQueue);
        JUCE_CHECK_OPENGL_ERROR
    }

    void setShader (ShaderPrograms::ShaderBase& shader)
    {
        currentShader.setShader (target, shaderQuadQueue, shader);
        JUCE_CHECK_OPENGL_ERROR
    }

    void setShaderForGradientFill (const ColourGradient& g, const AffineTransform& transform,
                                   int maskTextureID, const Rectangle<int>* maskArea)
    {
        JUCE_CHECK_OPENGL_ERROR
        activeTextures.disableTextures (shaderQuadQueue);
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);
        JUCE_CHECK_OPENGL_ERROR

        if (maskArea != nullptr)
        {
            activeTextures.setTexturesEnabled (shaderQuadQueue, 3);
            activeTextures.setActiveTexture (1);
            activeTextures.bindTexture ((GLuint) maskTextureID);
            activeTextures.setActiveTexture (0);
            textureCache.bindTextureForGradient (activeTextures, g);
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            textureCache.bindTextureForGradient (activeTextures, g);
        }

        auto t = transform.translated (0.5f - target.bounds.getX(),
                                       0.5f - target.bounds.getY());
        auto p1 = g.point1.transformedBy (t);
        auto p2 = g.point2.transformedBy (t);
        auto p3 = Point<float> (g.point1.x + (g.point2.y - g.point1.y),
                                g.point1.y - (g.point2.x - g.point1.x)).transformedBy (t);

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

    void setShaderForTiledImageFill (const TextureInfo& textureInfo, const AffineTransform& transform,
                                     int maskTextureID, const Rectangle<int>* maskArea, bool isTiledFill)
    {
        blendMode.setPremultipliedBlendingMode (shaderQuadQueue);

        ShaderPrograms* const programs = currentShader.programs;

        const ShaderPrograms::MaskedShaderParams* maskParams = nullptr;
        const ShaderPrograms::ImageParams* imageParams;

        if (maskArea != nullptr)
        {
            activeTextures.setTwoTextureMode (shaderQuadQueue, textureInfo.textureID, (GLuint) maskTextureID);

            if (isTiledFill)
            {
                setShader (programs->tiledImageMasked);
                imageParams = &programs->tiledImageMasked.imageParams;
                maskParams  = &programs->tiledImageMasked.maskParams;
            }
            else
            {
                setShader (programs->imageMasked);
                imageParams = &programs->imageMasked.imageParams;
                maskParams  = &programs->imageMasked.maskParams;
            }
        }
        else
        {
            activeTextures.setSingleTextureMode (shaderQuadQueue);
            activeTextures.bindTexture (textureInfo.textureID);

            if (isTiledFill)
            {
                setShader (programs->tiledImage);
                imageParams = &programs->tiledImage.imageParams;
            }
            else
            {
                setShader (programs->image);
                imageParams = &programs->image.imageParams;
            }
        }

        imageParams->setMatrix (transform, textureInfo, (float) target.bounds.getX(), (float) target.bounds.getY(), isTiledFill);

        if (maskParams != nullptr)
            maskParams->setBounds (*maskArea, target, 1);
    }

    Target target;

    StateHelpers::BlendingMode blendMode;
    StateHelpers::ActiveTextures activeTextures;
    StateHelpers::TextureCache textureCache;
    StateHelpers::CurrentShader currentShader;
    StateHelpers::ShaderQuadQueue shaderQuadQueue;

    CachedImageList::Ptr cachedImageList;

private:
    GLuint previousFrameBufferTarget;
};

//==============================================================================
struct SavedState  : public RenderingHelpers::SavedStateBase<SavedState>
{
    typedef RenderingHelpers::SavedStateBase<SavedState> BaseClass;

    SavedState (GLState* s)  : BaseClass (s->target.bounds), state (s)
    {}

    SavedState (const SavedState& other)
        : BaseClass (other), font (other.font), state (other.state),
          transparencyLayer (other.transparencyLayer),
          previousTarget (createCopyIfNotNull (other.previousTarget.get()))
    {}

    SavedState* beginTransparencyLayer (float opacity)
    {
        auto* s = new SavedState (*this);

        if (clip != nullptr)
        {
            auto clipBounds = clip->getClipBounds();

            state->flush();
            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->previousTarget.reset (new Target (state->target));
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
            finishedLayerState.previousTarget.reset();

            state->target.makeActive();
            auto clipBounds = clip->getClipBounds();

            clip->renderImageUntransformed (*this, finishedLayerState.transparencyLayer,
                                            (int) (finishedLayerState.transparencyLayerAlpha * 255.0f),
                                            clipBounds.getX(), clipBounds.getY(), false);
        }
    }

    typedef RenderingHelpers::GlyphCache<RenderingHelpers::CachedGlyphEdgeTable<SavedState>, SavedState> GlyphCacheType;

    void drawGlyph (int glyphNumber, const AffineTransform& trans)
    {
        if (clip != nullptr)
        {
            if (trans.isOnlyTranslation() && ! transform.isRotated)
            {
                auto& cache = GlyphCacheType::getInstance();
                Point<float> pos (trans.getTranslationX(), trans.getTranslationY());

                if (transform.isOnlyTranslated)
                {
                    cache.drawGlyph (*this, font, glyphNumber, pos + transform.offset.toFloat());
                }
                else
                {
                    pos = transform.transformed (pos);

                    Font f (font);
                    f.setHeight (font.getHeight() * transform.complexTransform.mat11);

                    auto xScale = transform.complexTransform.mat00 / transform.complexTransform.mat11;

                    if (std::abs (xScale - 1.0f) > 0.01f)
                        f.setHorizontalScale (xScale);

                    cache.drawGlyph (*this, f, glyphNumber, pos);
                }
            }
            else
            {
                auto fontHeight = font.getHeight();

                auto t = transform.getTransformWith (AffineTransform::scale (fontHeight * font.getHorizontalScale(), fontHeight)
                                                                     .followedBy (trans));

                const ScopedPointer<EdgeTable> et (font.getTypeface()->getEdgeTableForGlyph (glyphNumber, t, fontHeight));

                if (et != nullptr)
                    fillShape (new EdgeTableRegionType (*et), false);
            }
        }
    }

    Rectangle<int> getMaximumBounds() const     { return state->target.bounds; }

    void setFillType (const FillType& newFill)
    {
        BaseClass::setFillType (newFill);
        state->textureCache.resetGradient();
    }

    //==============================================================================
    template <typename IteratorType>
    void renderImageTransformed (IteratorType& iter, const Image& src, int alpha,
                                 const AffineTransform& trans, Graphics::ResamplingQuality, bool tiledFill) const
    {
        state->shaderQuadQueue.flush();
        state->setShaderForTiledImageFill (state->cachedImageList->getTextureFor (src), trans, 0, nullptr, tiledFill);

        state->shaderQuadQueue.add (iter, PixelARGB ((uint8) alpha, (uint8) alpha, (uint8) alpha, (uint8) alpha));
        state->shaderQuadQueue.flush();

        state->currentShader.clearShader (state->shaderQuadQueue);
    }

    template <typename IteratorType>
    void renderImageUntransformed (IteratorType& iter, const Image& src, int alpha, int x, int y, bool tiledFill) const
    {
        renderImageTransformed (iter, src, alpha, AffineTransform::translation ((float) x, (float) y),
                                Graphics::lowResamplingQuality, tiledFill);
    }

    template <typename IteratorType>
    void fillWithSolidColour (IteratorType& iter, PixelARGB colour, bool replaceContents) const
    {
        if (! isUsingCustomShader)
        {
            state->activeTextures.disableTextures (state->shaderQuadQueue);
            state->blendMode.setBlendMode (state->shaderQuadQueue, replaceContents);
            state->setShader (state->currentShader.programs->solidColourProgram);
        }

        state->shaderQuadQueue.add (iter, colour);
    }

    template <typename IteratorType>
    void fillWithGradient (IteratorType& iter, ColourGradient& gradient, const AffineTransform& trans, bool /*isIdentity*/) const
    {
        state->setShaderForGradientFill (gradient, trans, 0, nullptr);
        state->shaderQuadQueue.add (iter, fillType.colour.getPixelARGB());
    }

    void fillRectWithCustomShader (OpenGLRendering::ShaderPrograms::ShaderBase& shader, Rectangle<int> area)
    {
        state->setShader (shader);
        isUsingCustomShader = true;

        fillRect (area, true);

        isUsingCustomShader = false;
        state->currentShader.clearShader (state->shaderQuadQueue);
    }

    //==============================================================================
    Font font;
    GLState* state;
    bool isUsingCustomShader = false;

private:
    Image transparencyLayer;
    ScopedPointer<Target> previousTarget;

    SavedState& operator= (const SavedState&);
};


//==============================================================================
struct ShaderContext   : public RenderingHelpers::StackBasedLowLevelGraphicsContext<SavedState>
{
    ShaderContext (const Target& target)  : glState (target)
    {
        stack.initialise (new SavedState (&glState));
    }

    void fillRectWithCustomShader (ShaderPrograms::ShaderBase& shader, Rectangle<int> area)
    {
        static_cast<SavedState&> (*stack).fillRectWithCustomShader (shader, area);
    }

    GLState glState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShaderContext)
};

struct NonShaderContext   : public LowLevelGraphicsSoftwareRenderer
{
    NonShaderContext (const Target& t, const Image& im)
        : LowLevelGraphicsSoftwareRenderer (im), target (t), image (im)
    {}

    ~NonShaderContext()
    {
        JUCE_CHECK_OPENGL_ERROR
        auto previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();

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

static void clearOpenGLGlyphCacheCallback()
{
    SavedState::GlyphCacheType::getInstance().reset();
}

static LowLevelGraphicsContext* createOpenGLContext (const Target& target)
{
    clearOpenGLGlyphCache = clearOpenGLGlyphCacheCallback;

    if (target.context.areShadersAvailable())
        return new ShaderContext (target);

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
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, target, {}));
}

LowLevelGraphicsContext* createOpenGLGraphicsContext (OpenGLContext& context, unsigned int frameBufferID, int width, int height)
{
    return OpenGLRendering::createOpenGLContext (OpenGLRendering::Target (context, frameBufferID, width, height));
}

//==============================================================================
struct CustomProgram  : public ReferenceCountedObject,
                        public OpenGLRendering::ShaderPrograms::ShaderBase
{
    CustomProgram (OpenGLRendering::ShaderContext& c, const String& fragmentShader)
        : ShaderBase (c.glState.target.context, fragmentShader.toRawUTF8())
    {
    }

    static CustomProgram* get (const String& hashName)
    {
        if (auto* c = OpenGLContext::getCurrentContext())
            return static_cast<CustomProgram*> (c->getAssociatedObject (hashName.toRawUTF8()));

        return nullptr;
    }

    static CustomProgram* getOrCreate (LowLevelGraphicsContext& gc, const String& hashName, const String& code, String& errorMessage)
    {
        if (auto* c = get (hashName))
            return c;

        if (auto* sc = dynamic_cast<OpenGLRendering::ShaderContext*> (&gc))
        {
            ReferenceCountedObjectPtr<CustomProgram> c (new CustomProgram (*sc, code));
            errorMessage = c->lastError;

            if (errorMessage.isEmpty())
            {
                if (OpenGLContext* context = OpenGLContext::getCurrentContext())
                {
                    context->setAssociatedObject (hashName.toRawUTF8(), c);
                    return c;
                }
            }
        }

        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomProgram)
};

OpenGLGraphicsContextCustomShader::OpenGLGraphicsContextCustomShader (const String& fragmentShaderCode)
    : code (String (JUCE_DECLARE_VARYING_COLOUR
                    JUCE_DECLARE_VARYING_PIXELPOS
                    "\n#define pixelAlpha frontColour.a\n") + fragmentShaderCode),
      hashName (String::toHexString (fragmentShaderCode.hashCode64()) + "_shader")
{
}

OpenGLGraphicsContextCustomShader::~OpenGLGraphicsContextCustomShader()
{
    if (OpenGLContext* context = OpenGLContext::getCurrentContext())
        context->setAssociatedObject (hashName.toRawUTF8(), nullptr);
}

OpenGLShaderProgram* OpenGLGraphicsContextCustomShader::getProgram (LowLevelGraphicsContext& gc) const
{
    String errorMessage;

    if (CustomProgram* c = CustomProgram::getOrCreate (gc, hashName, code, errorMessage))
        return &(c->program);

    return nullptr;
}

void OpenGLGraphicsContextCustomShader::fillRect (LowLevelGraphicsContext& gc, Rectangle<int> area) const
{
    String errorMessage;

    if (OpenGLRendering::ShaderContext* sc = dynamic_cast<OpenGLRendering::ShaderContext*> (&gc))
        if (CustomProgram* c = CustomProgram::getOrCreate (gc, hashName, code, errorMessage))
            sc->fillRectWithCustomShader (*c, area);
}

Result OpenGLGraphicsContextCustomShader::checkCompilation (LowLevelGraphicsContext& gc)
{
    String errorMessage;

    if (CustomProgram::getOrCreate (gc, hashName, code, errorMessage) != nullptr)
        return Result::ok();

    return Result::fail (errorMessage);
}

} // namespace juce
