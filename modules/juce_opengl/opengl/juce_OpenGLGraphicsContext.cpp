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
    {}

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

        EdgeTableAlphaMap alphaMap (et);

        texture.loadAlpha (alphaMap.data, alphaMap.area.getWidth(), alphaMap.area.getHeight());
        textureID = texture.getTextureID();

        clip = et.getMaximumBounds();
        area = alphaMap.area;
    }

    PositionedTexture (GLuint textureID_, const Rectangle<int> area_, const Rectangle<int> clip_) noexcept
        : textureID (textureID_), area (area_), clip (clip_)
    {}

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

    GLuint textureID;
    Rectangle<int> area, clip;

private:
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
namespace
{
    struct TemporaryColourModulationMode
    {
        TemporaryColourModulationMode()    { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR); }
        ~TemporaryColourModulationMode()   { glTexEnvi (GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_ALPHA); }
    };
}

//==============================================================================
#if JUCE_USE_OPENGL_SHADERS
class GLShaderProgram
{
public:
    GLShaderProgram() noexcept
        : program (glCreateProgram())
    {
    }

    ~GLShaderProgram() noexcept
    {
        glDeleteProgram (program);
    }

    void addShader (const GLchar* const code, GLenum type)
    {
        GLuint shaderID = glCreateShader (type);
        glShaderSource (shaderID, 1, (const GLchar**) &code, nullptr);
        glCompileShader (shaderID);

       #if JUCE_DEBUG
        GLint status = 0;
        glGetShaderiv (shaderID, GL_COMPILE_STATUS, &status);

        if (status == GL_FALSE)
        {
            GLchar infoLog [16384];
            GLsizei infologLength = 0;
            glGetShaderInfoLog (shaderID, sizeof (infoLog), &infologLength, infoLog);
            DBG (String (infoLog, infologLength));
            jassertfalse;
        }
       #endif

        glAttachShader (program, shaderID);
        glDeleteShader (shaderID);
     }

    void link() noexcept
    {
        glLinkProgram (program);

       #if JUCE_DEBUG
        GLint status = 0;
        glGetProgramiv (program, GL_LINK_STATUS, &status);
        jassert (status != GL_FALSE);
       #endif
    }

    struct Uniform
    {
        Uniform (const GLShaderProgram& program, const GLchar* name)
            : uniformID (glGetUniformLocation (program.program, name))
        {
            jassert (uniformID >= 0);
        }

        void set (GLfloat n1) const noexcept                                    { glUniform1f (uniformID, n1); }
        void set (GLint n1) const noexcept                                      { glUniform1i (uniformID, n1); }
        void set (GLfloat n1, GLfloat n2) const noexcept                        { glUniform2f (uniformID, n1, n2); }
        void set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept            { glUniform3f (uniformID, n1, n2, n3); }
        void set (GLfloat n1, GLfloat n2, GLfloat n3, float n4) const noexcept  { glUniform4f (uniformID, n1, n2, n3, n4); }
        void set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept        { glUniform4i (uniformID, n1, n2, n3, n4); }

        void set (const AffineTransform& t) const noexcept
        {
            const GLfloat f[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
            glUniformMatrix2x3fv (uniformID, 1, false, f);
        }

        GLint uniformID;
    };

    GLuint program;
};

struct ShaderPrograms
{
    ShaderPrograms()
    {
        String v ((const char*) glGetString (GL_SHADING_LANGUAGE_VERSION));
        v = v.upToFirstOccurrenceOf (" ", false, false);
        areShadersSupported = (v.getDoubleValue() >= 1.199);
    }

    bool areShadersSupported;

    struct ShaderBase
    {
        ShaderBase (const char* fragmentShader)
        {
            addShader (fragmentShader, GL_FRAGMENT_SHADER);
            link();
        }

        GLShaderProgram program;
    };

    struct SolidColourMaskedProgram  : public ShaderBase
    {
        SolidColourMaskedProgram()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D maskTexture;"
                          "uniform ivec4 maskBounds;"
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  vec2 maskPos;"
                          "  maskPos.x = (gl_FragCoord.x - maskBounds.x) / maskBounds.z;"
                          "  maskPos.y = 1.0 - (gl_FragCoord.y - maskBounds.y) / maskBounds.w;"
                          "  gl_FragColor = gl_Color * texture2D (maskTexture, maskPos).w;"
                          "}"),
              maskTexture (program, "maskTexture"),
              maskBounds (program, "maskBounds")
        {
        }

        GLShaderProgram::Uniform maskTexture, maskBounds;
    };

    struct RadialGradientProgram  : public ShaderBase
    {
        RadialGradientProgram()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform mat2x3 matrix;"
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = length (vec2 (matrix[0][0] * gl_FragCoord.x + matrix[0][1] * gl_FragCoord.y + matrix[0][2],"
                          "                             matrix[1][0] * gl_FragCoord.x + matrix[1][1] * gl_FragCoord.y + matrix[1][2]));"
                          "  gl_FragColor = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              matrix (program, "matrix")
        {
        }

        GLShaderProgram::Uniform gradientTexture, matrix;
    };

    struct RadialGradientMaskedProgram  : public ShaderBase
    {
        RadialGradientMaskedProgram()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform mat2x3 matrix;"
                          "uniform sampler2D maskTexture;"
                          "uniform ivec4 maskBounds;"
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = length (vec2 (matrix[0][0] * gl_FragCoord.x + matrix[0][1] * gl_FragCoord.y + matrix[0][2],"
                          "                             matrix[1][0] * gl_FragCoord.x + matrix[1][1] * gl_FragCoord.y + matrix[1][2]));"
                          "  vec4 result = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          ""
                          "  vec2 maskPos;"
                          "  maskPos.x = (gl_FragCoord.x - maskBounds.x) / maskBounds.z;"
                          "  maskPos.y = 1.0 - (gl_FragCoord.y - maskBounds.y) / maskBounds.w;"
                          "  result *= texture2D (maskTexture, maskPos).w;"
                          ""
                          "  gl_FragColor = result;"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              matrix (program, "matrix"),
              maskTexture (program, "maskTexture"),
              maskBounds (program, "maskBounds")
        {
        }

        GLShaderProgram::Uniform gradientTexture, matrix;
        GLShaderProgram::Uniform maskTexture, maskBounds;
    };

    struct LinearGradient1Program  : public ShaderBase
    {
        LinearGradient1Program()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform vec4 gradientInfo;"  // x = x1, y = y1, z = (y2 - y1) / (x2 - x1), w = length
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = (gl_FragCoord.y - (gradientInfo.y + (gradientInfo.z * (gl_FragCoord.x - gradientInfo.x)))) / gradientInfo.w;"
                          "  gl_FragColor = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo")
        {
        }

        GLShaderProgram::Uniform gradientTexture, gradientInfo;
    };

    struct LinearGradient1MaskedProgram  : public ShaderBase
    {
        LinearGradient1MaskedProgram()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform vec3 gradientInfo;"  // x = (x2 - x1) / (y2 - y1), y = x1, z = length
                          "uniform sampler2D maskTexture;"
                          "uniform ivec4 maskBounds;"
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = (gl_FragCoord.y - (gradientInfo.y + (gradientInfo.x * (gl_FragCoord.x - gradientInfo.y)))) / gradientInfo.z;"
                          "  vec4 result = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          ""
                          "  vec2 maskPos;"
                          "  maskPos.x = (gl_FragCoord.x - maskBounds.x) / maskBounds.z;"
                          "  maskPos.y = 1.0 - (gl_FragCoord.y - maskBounds.y) / maskBounds.w;"
                          "  result *= texture2D (maskTexture, maskPos).w;"
                          "  gl_FragColor = result;"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo"),
              maskTexture (program, "maskTexture"),
              maskBounds (program, "maskBounds")
        {
        }

        GLShaderProgram::Uniform gradientTexture, gradientInfo;
        GLShaderProgram::Uniform maskTexture, maskBounds;
    };

    struct LinearGradient2Program  : public ShaderBase
    {
        LinearGradient2Program()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform vec4 gradientInfo;"  // x = x1, y = y1, z = (x2 - x1) / (y2 - y1), y = y1, w = length
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = (gl_FragCoord.x - (gradientInfo.x + (gradientInfo.z * (gl_FragCoord.y - gradientInfo.y)))) / gradientInfo.w;"
                          "  gl_FragColor = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo")
        {
        }

        GLShaderProgram::Uniform gradientTexture, gradientInfo;
    };

    struct LinearGradient2MaskedProgram  : public ShaderBase
    {
        LinearGradient2MaskedProgram()
            : ShaderBase ("#version 120\n"
                          "uniform sampler2D gradientTexture;"
                          "uniform vec3 gradientInfo;"  // x = (y2 - y1) / (x2 - x1), y = y1, z = length
                          "uniform sampler2D maskTexture;"
                          "uniform ivec4 maskBounds;"
                          "const float textureY = 0.5;"
                          ""
                          "void main()"
                          "{"
                          "  float dist = (gl_FragCoord.x - (gradientInfo.y + (gradientInfo.x * (gl_FragCoord.y - gradientInfo.y)))) / gradientInfo.z;"
                          "  vec4 result = gl_Color.w * texture2D (gradientTexture, vec2 (dist, textureY));"
                          ""
                          "  vec2 maskPos;"
                          "  maskPos.x = (gl_FragCoord.x - maskBounds.x) / maskBounds.z;"
                          "  maskPos.y = 1.0 - (gl_FragCoord.y - maskBounds.y) / maskBounds.w;"
                          "  result *= texture2D (maskTexture, maskPos).w;"
                          "  gl_FragColor = result;"
                          "}"),
              gradientTexture (program, "gradientTexture"),
              gradientInfo (program, "gradientInfo"),
              maskTexture (program, "maskTexture"),
              maskBounds (program, "maskBounds")
        {
        }

        GLShaderProgram::Uniform gradientTexture, gradientInfo;
        GLShaderProgram::Uniform maskTexture, maskBounds;
    };

    SolidColourMaskedProgram solidColourMasked;
    RadialGradientProgram radialGradient;
    RadialGradientMaskedProgram radialGradientMasked;
    LinearGradient1Program linearGradient1;
    LinearGradient1MaskedProgram linearGradient1Masked;
    LinearGradient2Program linearGradient2;
    LinearGradient2MaskedProgram linearGradient2Masked;
};

//static ScopedPointer<ShaderPrograms> programs;
#endif

class OpenGLRenderer::GLState
{
public:
    GLState (const OpenGLTarget& target_) noexcept
        : target (target_),
          previousFrameBufferTarget (OpenGLFrameBuffer::getCurrentFrameBufferTarget()),
          texturesEnabled (0),
          currentActiveTexture (0),
          blendingEnabled (false),
          blendEnabled (false),
          srcFunction (0), dstFunction (0),
          currentColour (0),
          quadQueueActive (false),
          numVertices (0),
          activeGradientIndex (0),
          gradientNeedsRefresh (true)
         #if JUCE_USE_OPENGL_SHADERS
          , activeShader (nullptr)
         #endif
    {
        // This object can only be created and used when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        initialiseMultiTextureExtensions();
        target.makeActiveFor2D();
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glEnableClientState (GL_VERTEX_ARRAY);
        glDisable (GL_BLEND);
        glColor4f (0, 0, 0, 0);

        for (int i = 3; --i >= 0;)
        {
            setActiveTexture (i);
            glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        }

        clearSelectedTextures();
        resetMultiTextureModes (false);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

       #if JUCE_USE_OPENGL_SHADERS
        if (programs == nullptr)
            programs = new ShaderPrograms();

        canUseShaders = programs->areShadersSupported;
       #endif
    }

    ~GLState()
    {
        flushQuads();
        OpenGLFrameBuffer::setCurrentFrameBufferTarget (previousFrameBufferTarget);
        resetMultiTextureModes (true);
        glFlush();
    }

   #if JUCE_USE_OPENGL_SHADERS
    bool shadersAvailable() const
    {
        return canUseShaders;
    }
   #endif

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

    void setPremultipliedBlendingMode() noexcept
    {
        setBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }

    void setBlendFunc (GLenum src, GLenum dst)
    {
        if (! blendingEnabled)
        {
            flushQuads();
            blendingEnabled = true;
            glEnable (GL_BLEND);
        }

        if (srcFunction != src || dstFunction != dst)
        {
            flushQuads();
            srcFunction = src;
            dstFunction = dst;
            glBlendFunc (src, dst);
        }
    }

    void disableBlend() noexcept
    {
        if (blendingEnabled)
        {
            flushQuads();
            blendingEnabled = false;
            glDisable (GL_BLEND);
        }
    }

    void setBlendMode (const bool replaceExistingContents) noexcept
    {
        if (replaceExistingContents)
            disableBlend();
        else
            setPremultipliedBlendingMode();
    }

    void scissor (const Rectangle<int>& r)
    {
        flushQuads();
        target.scissor (r);
    }

    void disableScissor()
    {
        flushQuads();
        glDisable (GL_SCISSOR_TEST);
    }

    OpenGLTexture* getTexture (int w, int h)
    {
        if (textures.size() < numTexturesToCache)
        {
            clearSelectedTextures();
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

    void releaseTexture (OpenGLTexture* texture)
    {
        currentTextureID [currentActiveTexture] = 0;
        textures.add (texture);
    }

    void resetGradient() noexcept
    {
        gradientNeedsRefresh = true;
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
            GLint t = 0;
            glGetIntegerv (GL_TEXTURE_BINDING_2D, &t);
            jassert (t == (GLint) textureID);
        }
    }

    void bindTextureForGradient (const ColourGradient& gradient)
    {
        if (gradientNeedsRefresh)
        {
            gradientNeedsRefresh = false;

            if (gradientTextures.size() < numGradientTexturesToCache)
            {
                activeGradientIndex = gradientTextures.size();
                clearSelectedTextures();
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

        bindTexture (gradientTextures.getUnchecked (activeGradientIndex)->getTextureID());
    }

    void setTexturesEnabled (const int textureIndexMask) noexcept
    {
        if (texturesEnabled != textureIndexMask)
        {
            flushQuads();

            for (int i = 3; --i >= 0;)
            {
                if ((texturesEnabled & (1 << i)) != (textureIndexMask & (1 << i)))
                {
                    setActiveTexture (i);

                    if ((textureIndexMask & (1 << i)) != 0)
                        glEnable (GL_TEXTURE_2D);
                    else
                        glDisable (GL_TEXTURE_2D);
                }
            }

            texturesEnabled = textureIndexMask;
        }
    }

    void setActiveTexture (const int index) noexcept
    {
        if (currentActiveTexture != index)
        {
            currentActiveTexture = index;
            glActiveTexture (GL_TEXTURE0 + index);
            glClientActiveTexture (GL_TEXTURE0 + index);
        }
    }

    void setSingleTextureMode() noexcept
    {
        setTexturesEnabled (1);
        setActiveTexture (0);
    }

    void prepareMasks (const PositionedTexture* const mask1, const PositionedTexture* const mask2,
                       GLfloat* const textureCoords1, GLfloat* const textureCoords2, const Rectangle<int>* const area)
    {
        if (mask1 != nullptr)
        {
            setTexturesEnabled (mask2 != nullptr ? 7 : 3);
            setActiveTexture (0);
            mask1->prepareTextureCoords (area, textureCoords1);
            bindTexture (mask1->textureID);
            setActiveTexture (1);

            if (mask2 != nullptr)
            {
                mask2->prepareTextureCoords (area, textureCoords2);
                bindTexture (mask2->textureID);
                setActiveTexture (2);
            }
        }
        else
        {
            setSingleTextureMode();
        }
    }

    void checkQuadQueueActive()
    {
        if (! quadQueueActive)
        {
            jassert (numVertices == 0);
            setTexturesEnabled (0);
            glEnableClientState (GL_COLOR_ARRAY);
            glVertexPointer (2, GL_SHORT, 0, vertices);
            glColorPointer (4, GL_UNSIGNED_BYTE, 0, colours);
            setSolidColour();
            quadQueueActive = true; // (careful to do this last, as the preceding calls may change it)
        }
    }

    void addQuadToList (const int x, const int y, const int w, const int h, const PixelARGB& colour) noexcept
    {
        jassert (quadQueueActive && w > 0 && h > 0);

        const GLshort x1 = (GLshort) x;
        const GLshort y1 = (GLshort) y;
        const GLshort x2 = (GLshort) (x + w);
        const GLshort y2 = (GLshort) (y + h);

        GLshort* const v = vertices + numVertices * 2;
        v[0] = x1;  v[1] = y1;  v[2] = x2;  v[3] = y1;  v[4]  = x1;  v[5]  = y2;
        v[6] = x2;  v[7] = y1;  v[8] = x1;  v[9] = y2;  v[10] = x2;  v[11] = y2;

        const uint32 rgba = colour.getInRGBAMemoryOrder();

        uint32* const c = colours + numVertices;
        for (int i = 0; i < 6; ++i)
            c[i] = rgba;

        numVertices += 6;

        if (numVertices > maxVerticesPerBlock - 6)
        {
            glDrawArrays (GL_TRIANGLES, 0, numVertices);
            numVertices = 0;
        }
    }

    void fillRect (const Rectangle<float>& r, const PixelARGB& colour) noexcept
    {
        jassert (! r.isEmpty());

        FloatRectangleRenderer frr (*this, colour);
        RenderingHelpers::FloatRectangleRasterisingInfo (r).iterate (frr);
    }

    void fillRectangleList (const RectangleList& list, const PixelARGB& colour)
    {
        checkQuadQueueActive();

        for (RectangleList::Iterator i (list); i.next();)
            addQuadToList (i.getRectangle()->getX(), i.getRectangle()->getY(),
                           i.getRectangle()->getWidth(), i.getRectangle()->getHeight(), colour);
    }

    void fillRectangleList (const RectangleList& list, const Rectangle<int>& clip, const PixelARGB& colour)
    {
        checkQuadQueueActive();

        for (RectangleList::Iterator i (list); i.next();)
        {
            const Rectangle<int> r (i.getRectangle()->getIntersection (clip));

            if (! r.isEmpty())
                addQuadToList (r.getX(), r.getY(), r.getWidth(), r.getHeight(), colour);
        }
    }

    void drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords, const int numVertices) noexcept
    {
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
        glDrawArrays (GL_TRIANGLE_STRIP, 0, numVertices);
    }

    void fillEdgeTable (const EdgeTable& et, const PixelARGB& colour)
    {
        EdgeTableRenderer etr (*this, colour);
        et.iterate (etr);
    }

    void renderImage (const OpenGLTextureFromImage& image,
                      const Rectangle<int>& clip, const AffineTransform& transform, float alpha,
                      const PositionedTexture* mask1, const PositionedTexture* mask2,
                      const bool replaceExistingContents, const bool isTiled)
    {
        flushQuads();
        setBlendMode (replaceExistingContents);
        setColour (alpha);
        GLfloat textureCoords1[8], textureCoords2[8];

        if ((! isTiled) || (isPowerOfTwo (image.imageWidth) && isPowerOfTwo (image.imageHeight)))
        {
            prepareMasks (mask1, mask2, textureCoords1, textureCoords2, &clip);

            bindTexture (image.textureID);
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

            bindTexture (image.textureID);
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
                setBlendMode (replaceExistingContents);
                setTexturesEnabled (mask2 != nullptr ? 3 : 1);

                setActiveTexture (0);
                mask1->prepareTextureCoords (&area, textureCoords1);
                bindTexture (mask1->textureID);

                if (mask2 != nullptr)
                {
                    setActiveTexture (1);
                    mask2->prepareTextureCoords (&area, textureCoords2);
                    bindTexture (mask2->textureID);
                }
            }
            else
            {
                setBlendMode (replaceExistingContents || fill.colour.isOpaque());
                setTexturesEnabled (0);
            }

            setPremultipliedColour (fill.colour);
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
                setBlendMode (replaceExistingContents || (mask1 == nullptr && fill.colour.isOpaque() && fill.gradient->isOpaque()));

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

   #if JUCE_USE_OPENGL_SHADERS
    void setShaderForGradientFill (const FillType& fill)
    {
        setPremultipliedBlendingMode();
        setSingleTextureMode();
        setSolidColour();

        const ColourGradient& g = *fill.gradient;
        bindTextureForGradient (g);

        const AffineTransform t (fill.transform.followedBy (AffineTransform::verticalFlip (target.height)));
        Point<float> p1 (g.point1.transformedBy (t));
        const Point<float> p2 (g.point2.transformedBy (t));
        const Point<float> p3 (Point<float> (g.point1.x + (g.point2.y - g.point1.y),
                                             g.point1.y - (g.point2.x - g.point1.x)).transformedBy (t));

        if (g.isRadial)
        {
            setShader (&(programs->radialGradient.program));
            programs->radialGradient.matrix.set (AffineTransform::fromTargetPoints (p1.x, p1.y,  0.0f, 0.0f,
                                                                                    p2.x, p2.y,  1.0f, 0.0f,
                                                                                    p3.x, p3.y,  0.0f, 1.0f));
        }
        else
        {
            p1 = Line<float> (p1, p3).findNearestPointTo (p2);
            const Point<float> delta (p2.x - p1.x, p1.y - p2.y);

            if (std::abs (delta.x) < std::abs (delta.y))
            {
                setShader (&(programs->linearGradient1.program));
                const float grad = delta.x / delta.y;
                programs->linearGradient1.gradientInfo.set (p1.x, p1.y, grad, (p2.y - grad * p2.x) - (p1.y - grad * p1.x));
            }
            else
            {
                setShader (&(programs->linearGradient2.program));
                const float grad = delta.y / delta.x;
                programs->linearGradient2.gradientInfo.set (p1.x, p1.y, grad, (p2.x - grad * p2.y) - (p1.x - grad * p1.y));
            }
        }
    }
   #endif

    void flushQuads() noexcept
    {
        if (quadQueueActive)
        {
            if (numVertices > 0)
            {
                glDrawArrays (GL_TRIANGLES, 0, numVertices);
                numVertices = 0;
            }

            quadQueueActive = false;
            glDisableClientState (GL_COLOR_ARRAY);
        }
    }

    void clearSelectedTextures() noexcept
    {
        for (int i = 0; i < numElementsInArray (currentTextureID); ++i)
            currentTextureID[i] = 0;
    }

   #if JUCE_USE_OPENGL_SHADERS
    void setShader (GLShaderProgram* newShader)
    {
        if (activeShader != newShader)
        {
            flushQuads();
            activeShader = newShader;
            glUseProgram (newShader != nullptr ? newShader->program : 0);
        }
    }
   #endif

    OpenGLTarget target;

private:
    enum
    {
        maxVerticesPerBlock = 192,
        gradientTextureSize = 256,
        numTexturesToCache = 8,
        numGradientTexturesToCache = 6
    };

    GLuint previousFrameBufferTarget;
    GLuint currentTextureID [3];
    int texturesEnabled, currentActiveTexture;
    bool blendingEnabled, blendEnabled;
    GLenum srcFunction, dstFunction;
    PixelARGB currentColour;

    bool quadQueueActive;
    GLshort vertices [maxVerticesPerBlock * 2];
    uint32 colours [maxVerticesPerBlock];
    int numVertices;

    OwnedArray<OpenGLTexture> textures, gradientTextures;
    int activeGradientIndex;
    bool gradientNeedsRefresh;

   #if JUCE_USE_OPENGL_SHADERS
    GLShaderProgram* activeShader;
    bool canUseShaders;
   #endif

    void resetMultiTextureMode (int index, const bool forRGBTextures)
    {
        setActiveTexture (index);
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
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    void resetMultiTextureModes (const bool forRGBTextures)
    {
        resetMultiTextureMode (2, forRGBTextures);
        resetMultiTextureMode (1, forRGBTextures);
        resetMultiTextureMode (0, forRGBTextures);
    }

    struct EdgeTableRenderer
    {
        EdgeTableRenderer (GLState& state_, const PixelARGB& colour_) noexcept
            : state (state_), colour (colour_)
        {
            state.checkQuadQueueActive();
        }

        void setEdgeTableYPos (const int y) noexcept
        {
            currentY = y;
        }

        void handleEdgeTablePixel (const int x, const int alphaLevel) noexcept
        {
            PixelARGB c (colour);
            c.multiplyAlpha (alphaLevel);
            state.addQuadToList (x, currentY, 1, 1, c);
        }

        void handleEdgeTablePixelFull (const int x) noexcept
        {
            state.addQuadToList (x, currentY, 1, 1, colour);
        }

        void handleEdgeTableLine (const int x, const int width, const int alphaLevel) noexcept
        {
            PixelARGB c (colour);
            c.multiplyAlpha (alphaLevel);
            state.addQuadToList (x, currentY, width, 1, c);
        }

        void handleEdgeTableLineFull (const int x, const int width) noexcept
        {
            state.addQuadToList (x, currentY, width, 1, colour);
        }

    private:
        GLState& state;
        const PixelARGB colour;
        int currentY;

        JUCE_DECLARE_NON_COPYABLE (EdgeTableRenderer);
    };

    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (GLState& state_, const PixelARGB& colour_) noexcept
            : state (state_), colour (colour_)
        {
            state.checkQuadQueueActive();
        }

        void operator() (const int x, const int y, const int w, const int h, const int alpha) noexcept
        {
            if (w > 0 && h > 0)
            {
                PixelARGB c (colour);
                c.multiplyAlpha (alpha);
                state.addQuadToList (x, y, w, h, c);
            }
        }

    private:
        GLState& state;
        const PixelARGB colour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };

    void fillWithLinearGradient (const Rectangle<int>& rect, const ColourGradient& grad, const AffineTransform& transform,
                                 const PositionedTexture* mask1, const PositionedTexture* mask2)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.x - (grad.point2.y - grad.point1.y) / gradientTextureSize,
                                             grad.point1.y + (grad.point2.x - grad.point1.x) / gradientTextureSize).transformedBy (transform));

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

        bindTextureForGradient (grad);

        setSolidColour();
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
        bindTextureForGradient (grad);
        setSolidColour();
        TemporaryColourModulationMode tmm;
        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords1);
        glDrawArrays (GL_TRIANGLE_FAN, 0, numDivisions + 2);
        disableScissor();
    }
};

//==============================================================================
class ClipRegion_Mask;

//==============================================================================
class ClipRegionBase  : public SingleThreadedReferenceCountedObject
{
public:
    ClipRegionBase (OpenGLRenderer::GLState& state_) noexcept : state (state_) {}
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
    virtual void fillRect (const Rectangle<int>& area, const FillType&, bool replaceContents) = 0;
    virtual void fillRect (const Rectangle<float>& area, const FillType&) = 0;
    virtual void fillEdgeTable (EdgeTable& et, const FillType& fill) = 0;
    virtual void drawImage (const OpenGLTextureFromImage&, const AffineTransform&, float alpha,
                            const Rectangle<int>& clip, const PositionedTexture* mask) = 0;

    OpenGLRenderer::GLState& state;

private:
    JUCE_DECLARE_NON_COPYABLE (ClipRegionBase);
};

//==============================================================================
class ClipRegion_Mask  : public ClipRegionBase
{
public:
    ClipRegion_Mask (const ClipRegion_Mask& other)
        : ClipRegionBase (other.state),
          clip (other.clip),
          maskOrigin (other.clip.getPosition())
    {
        OpenGLTarget::TargetSaver ts;
        state.flushQuads();
        state.setSingleTextureMode();
        state.clearSelectedTextures();
        mask.initialise (clip.getWidth(), clip.getHeight());

        OpenGLTarget m (mask, maskOrigin);
        m.makeActiveFor2D();
        state.disableBlend();
        state.setSolidColour();
        state.setSingleTextureMode();
        OpenGLHelpers::drawTextureQuad (other.mask.getTextureID(), other.getMaskArea());
    }

    explicit ClipRegion_Mask (OpenGLRenderer::GLState& state_, const RectangleList& r)
        : ClipRegionBase (state_),
          clip (r.getBounds()),
          maskOrigin (clip.getPosition())
    {
        OpenGLTarget::TargetSaver ts;
        initialiseClear();
        state.disableBlend();
        state.fillRectangleList (r, PixelARGB (0xffffffff));
        state.flushQuads();
    }

    Ptr clone() const                       { return new ClipRegion_Mask (*this); }
    Rectangle<int> getClipBounds() const    { return clip; }

    Ptr applyClipTo (const Ptr& target)
    {
        return target->clipToTexture (PositionedTexture (mask.getTextureID(), getMaskArea(), clip));
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

            OpenGLTarget::TargetSaver ts;
            makeMaskActive();
            state.disableBlend();
            state.fillRectangleList (excluded, PixelARGB (0));
            state.flushQuads();
        }

        return this;
    }

    Ptr excludeClipRectangle (const Rectangle<int>& r)
    {
        if (r.contains (clip))
            return nullptr;

        OpenGLTarget::TargetSaver ts;
        makeMaskActive();
        state.setTexturesEnabled (0);
        state.disableBlend();
        state.setColour (PixelARGB (0));
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

        OpenGLTarget::TargetSaver ts;
        makeMaskActive();
        state.setBlendFunc (GL_ZERO, GL_SRC_ALPHA);
        state.setSolidColour();
        state.setSingleTextureMode();
        OpenGLHelpers::drawTextureQuad (pt.textureID, pt.area);
        return this;
    }

    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)
    {
        OpenGLTarget::TargetSaver ts;
        makeMaskActive();
        state.setBlendFunc (GL_ZERO, GL_SRC_ALPHA);
        state.setSolidColour();
        state.setSingleTextureMode();
        state.bindTexture (image.textureID);

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
            OpenGLTexture* texture = state.getTexture (r.getWidth(), r.getHeight());
            PositionedTexture pt1 (*texture, et, r);
            PositionedTexture pt2 (mask.getTextureID(), getMaskArea(), r);
            state.fillTexture (r, fill, &pt2, &pt1, false);
            state.releaseTexture (texture);
        }
    }

    void fillRectInternal (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        PositionedTexture pt (mask.getTextureID(), getMaskArea(), area);
        state.fillTexture (area, fill, &pt, nullptr, replaceContents);
    }

    void drawImage (const OpenGLTextureFromImage& source, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, const PositionedTexture* mask1)
    {
        const Rectangle<int> bufferArea (clipArea.getIntersection (clip));

        if (! bufferArea.isEmpty())
        {
            PositionedTexture pt (mask.getTextureID(), getMaskArea(), bufferArea);
            state.renderImage (source, bufferArea, transform, alpha, &pt, mask1, false, false);
        }
    }

private:
    OpenGLFrameBuffer mask;
    Rectangle<int> clip;
    Point<int> maskOrigin;

    Rectangle<int> getMaskArea() const noexcept { return Rectangle<int> (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()); }
    void prepareFor2D() const    { OpenGLTarget::applyFlippedMatrix (maskOrigin.x, maskOrigin.y, mask.getWidth(), mask.getHeight()); }

    void makeMaskActive()
    {
        state.flushQuads();
        const bool b = mask.makeCurrentRenderingTarget();
        (void) b; jassert (b);
        prepareFor2D();
    }

    void initialiseClear()
    {
        state.flushQuads();
        jassert (! clip.isEmpty());
        state.setSingleTextureMode();
        state.clearSelectedTextures();
        mask.initialise (clip.getWidth(), clip.getHeight());
        mask.makeCurrentAndClear();
        state.setTexturesEnabled (0);
        state.disableBlend();
        prepareFor2D();
    }

    struct FloatRectangleRenderer
    {
        FloatRectangleRenderer (ClipRegion_Mask& mask_, const FillType& fill_) noexcept
            : mask (mask_), fill (fill_), originalColour (fill_.colour)
        {}

        void operator() (const int x, const int y, const int w, const int h, const int alpha) noexcept
        {
            if (w > 0 && h > 0)
            {
                fill.colour = originalColour.withMultipliedAlpha (alpha / 255.0f);
                mask.fillRect (Rectangle<int> (x, y, w, h), fill, false);
            }
        }

    private:
        ClipRegion_Mask& mask;
        FillType fill;
        const Colour originalColour;

        JUCE_DECLARE_NON_COPYABLE (FloatRectangleRenderer);
    };

    ClipRegion_Mask& operator= (const ClipRegion_Mask&);
};


//==============================================================================
class ClipRegion_RectangleList  : public ClipRegionBase
{
public:
    explicit ClipRegion_RectangleList (OpenGLRenderer::GLState& state_, const Rectangle<int>& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    explicit ClipRegion_RectangleList (OpenGLRenderer::GLState& state_, const RectangleList& r) noexcept
        : ClipRegionBase (state_), clip (r)
    {}

    Ptr clone() const                               { return new ClipRegion_RectangleList (state, clip); }
    Rectangle<int> getClipBounds() const            { return clip.getBounds(); }
    Ptr applyClipTo (const Ptr& target)             { return target->clipToRectangleList (clip); }

    Ptr clipToRectangle (const Rectangle<int>& r)       { return clip.clipTo (r) ? this : nullptr; }
    Ptr clipToRectangleList (const RectangleList& r)    { return clip.clipTo (r) ? this : nullptr; }
    Ptr excludeClipRectangle (const Rectangle<int>& r)  { clip.subtract (r); return clip.isEmpty() ? nullptr : this; }

    Ptr clipToTexture (const PositionedTexture& t)                                                  { return toMask()->clipToTexture (t); }
    Ptr clipToPath (const Path& p, const AffineTransform& transform)                                { return toMask()->clipToPath (p, transform); }
    Ptr clipToImageAlpha (const OpenGLTextureFromImage& image, const AffineTransform& transform)    { return toMask()->clipToImageAlpha (image, transform); }

    void fillRect (const Rectangle<int>& area, const FillType& fill, bool replaceContents)
    {
        if (fill.isColour())
        {
            state.setTexturesEnabled (0);
            state.setBlendMode (replaceContents || fill.colour.isOpaque());
            state.fillRectangleList (clip, area, fill.colour.getPixelARGB());
        }
       #if JUCE_USE_OPENGL_SHADERS
        else if (state.shadersAvailable() && fill.isGradient())
        {
            state.setShaderForGradientFill (fill);
            state.fillRectangleList (clip, area, fill.colour.getPixelARGB());
            state.setShader (nullptr);
        }
       #endif
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
            state.setTexturesEnabled (0);
            state.setPremultipliedBlendingMode();

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<float> r (i.getRectangle()->toFloat().getIntersection (area));
                if (! r.isEmpty())
                    state.fillRect (r, fill.colour.getPixelARGB());
            }
        }
       #if JUCE_USE_OPENGL_SHADERS
        else if (state.shadersAvailable() && fill.isGradient())
        {
            state.setShaderForGradientFill (fill);

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<float> r (i.getRectangle()->toFloat().getIntersection (area));
                if (! r.isEmpty())
                    state.fillRect (r, fill.colour.getPixelARGB());
            }

            state.setShader (nullptr);
        }
       #endif
        else
        {
            EdgeTable et (area);
            fillEdgeTable (et, fill);
        }
    }

    void drawImage (const OpenGLTextureFromImage& source, const AffineTransform& transform,
                    float alpha, const Rectangle<int>& clipArea, const PositionedTexture* mask)
    {
        for (RectangleList::Iterator i (clip); i.next();)
        {
            const Rectangle<int> bufferArea (i.getRectangle()->getIntersection (clipArea));

            if (! bufferArea.isEmpty())
                state.renderImage (source, bufferArea, transform, alpha, mask, nullptr, false, false);
        }
    }

    void fillEdgeTable (EdgeTable& et, const FillType& fill)
    {
        if (fill.isColour())
        {
            state.setPremultipliedBlendingMode();

            if (clip.containsRectangle (et.getMaximumBounds()))
            {
                state.fillEdgeTable (et, fill.colour.getPixelARGB());
            }
            else
            {
                EdgeTable et2 (clip);
                et2.clipToEdgeTable (et);
                state.fillEdgeTable (et2, fill.colour.getPixelARGB());
            }
        }
       #if JUCE_USE_OPENGL_SHADERS
        else if (state.shadersAvailable() && fill.isGradient())
        {
            state.setShaderForGradientFill (fill);

            if (clip.containsRectangle (et.getMaximumBounds()))
            {
                state.fillEdgeTable (et, fill.colour.getPixelARGB());
            }
            else
            {
                EdgeTable et2 (clip);
                et2.clipToEdgeTable (et);
                state.fillEdgeTable (et2, fill.colour.getPixelARGB());
            }

            state.setShader (nullptr);
        }
       #endif
        else
        {
            OpenGLTexture* texture = state.getTexture (clip.getBounds().getWidth(), clip.getBounds().getHeight());
            PositionedTexture pt (*texture, et, clip.getBounds());

            for (RectangleList::Iterator i (clip); i.next();)
            {
                const Rectangle<int> r (i.getRectangle()->getIntersection (pt.clip));
                if (! r.isEmpty())
                    state.fillTexture (r, fill, &pt, nullptr, false);
            }

            state.releaseTexture (texture);
        }
    }

private:
    RectangleList clip;

    Ptr toMask() const
    {
        return new ClipRegion_Mask (state, clip);
    }

    ClipRegion_RectangleList& operator= (const ClipRegion_RectangleList&);
};


//==============================================================================
class OpenGLRenderer::SavedState
{
public:
    SavedState (GLState* const state_)
        : clip (new ClipRegion_RectangleList (*state_, Rectangle<int> (state_->target.width, state_->target.height))),
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

            state->flushQuads();
            s->transparencyLayer = Image (OpenGLImageType().create (Image::ARGB, clipBounds.getWidth(), clipBounds.getHeight(), true));
            s->previousTarget = new OpenGLTarget (state->target);
            state->target = OpenGLTarget (*OpenGLImageType::getFrameBufferFrom (s->transparencyLayer), clipBounds.getPosition());
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

            state->flushQuads();
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

                clip->drawImage (image, t, alpha, Rectangle<int> (tx, ty, image.getWidth(), image.getHeight()), nullptr);
                return;
            }
        }

        if (! t.isSingularity())
        {
            Path p;
            p.addRectangle (image.getBounds());

            OpenGLTexture* texture = state->getTexture (clipBounds.getWidth(), clipBounds.getHeight());
            EdgeTable et (clipBounds, p, t);
            PositionedTexture pt (*texture, et, clipBounds);

            clip->drawImage (image, t, alpha, clipBounds, &pt);
            state->releaseTexture (texture);
        }
    }

    void setFillType (const FillType& newFill)
    {
        fillType = newFill;
        state->resetGradient();
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

    void fillEdgeTable (EdgeTable& et)
    {
        clip->fillEdgeTable (et, getFillType());
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
    : glState (new GLState (OpenGLTarget (target.getFrameBufferID(), target.getWidth(), target.getHeight()))),
      stack (new SavedState (glState))
{
}

OpenGLRenderer::OpenGLRenderer (OpenGLFrameBuffer& target)
    : glState (new GLState (OpenGLTarget (target, Point<int>()))),
      stack (new SavedState (glState))
{
}

OpenGLRenderer::OpenGLRenderer (unsigned int frameBufferID, int width, int height)
    : glState (new GLState (OpenGLTarget (frameBufferID, width, height))),
      stack (new SavedState (glState))
{
}

OpenGLRenderer::~OpenGLRenderer()
{
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
