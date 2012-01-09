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

OpenGLPixelFormat::OpenGLPixelFormat (const int bitsPerRGBComponent,
                                      const int alphaBits_,
                                      const int depthBufferBits_,
                                      const int stencilBufferBits_) noexcept
    : redBits (bitsPerRGBComponent),
      greenBits (bitsPerRGBComponent),
      blueBits (bitsPerRGBComponent),
      alphaBits (alphaBits_),
      depthBufferBits (depthBufferBits_),
      stencilBufferBits (stencilBufferBits_),
      accumulationBufferRedBits (0),
      accumulationBufferGreenBits (0),
      accumulationBufferBlueBits (0),
      accumulationBufferAlphaBits (0),
      multisamplingLevel (0)
{
}

OpenGLPixelFormat::OpenGLPixelFormat (const OpenGLPixelFormat& other) noexcept
    : redBits (other.redBits),
      greenBits (other.greenBits),
      blueBits (other.blueBits),
      alphaBits (other.alphaBits),
      depthBufferBits (other.depthBufferBits),
      stencilBufferBits (other.stencilBufferBits),
      accumulationBufferRedBits (other.accumulationBufferRedBits),
      accumulationBufferGreenBits (other.accumulationBufferGreenBits),
      accumulationBufferBlueBits (other.accumulationBufferBlueBits),
      accumulationBufferAlphaBits (other.accumulationBufferAlphaBits),
      multisamplingLevel (other.multisamplingLevel)
{
}

OpenGLPixelFormat& OpenGLPixelFormat::operator= (const OpenGLPixelFormat& other) noexcept
{
    redBits = other.redBits;
    greenBits = other.greenBits;
    blueBits = other.blueBits;
    alphaBits = other.alphaBits;
    depthBufferBits = other.depthBufferBits;
    stencilBufferBits = other.stencilBufferBits;
    accumulationBufferRedBits = other.accumulationBufferRedBits;
    accumulationBufferGreenBits = other.accumulationBufferGreenBits;
    accumulationBufferBlueBits = other.accumulationBufferBlueBits;
    accumulationBufferAlphaBits = other.accumulationBufferAlphaBits;
    multisamplingLevel = other.multisamplingLevel;
    return *this;
}

bool OpenGLPixelFormat::operator== (const OpenGLPixelFormat& other) const noexcept
{
    return redBits == other.redBits
            && greenBits == other.greenBits
            && blueBits == other.blueBits
            && alphaBits == other.alphaBits
            && depthBufferBits == other.depthBufferBits
            && stencilBufferBits == other.stencilBufferBits
            && accumulationBufferRedBits == other.accumulationBufferRedBits
            && accumulationBufferGreenBits == other.accumulationBufferGreenBits
            && accumulationBufferBlueBits == other.accumulationBufferBlueBits
            && accumulationBufferAlphaBits == other.accumulationBufferAlphaBits
            && multisamplingLevel == other.multisamplingLevel;
}

//==============================================================================
static Array<OpenGLContext*> knownContexts;

OpenGLContext::OpenGLContext() noexcept
    : shaderLanguageVersion (0)
{
    knownContexts.add (this);
}

OpenGLContext::~OpenGLContext()
{
    knownContexts.removeValue (this);
}

OpenGLContext* OpenGLContext::getCurrentContext()
{
    for (int i = knownContexts.size(); --i >= 0;)
    {
        OpenGLContext* const oglc = knownContexts.getUnchecked(i);

        if (oglc->isActive())
            return oglc;
    }

    return nullptr;
}

#if JUCE_USE_OPENGL_SHADERS
double OpenGLContext::getShaderLanguageVersion()
{
    if (shaderLanguageVersion == 0)
        shaderLanguageVersion = OpenGLShaderProgram::getLanguageVersion();

    return shaderLanguageVersion;
}
#endif

void OpenGLContext::copyTexture (const Rectangle<int>& targetClipArea,
                                 const Rectangle<int>& anchorPosAndTextureSize,
                                 float alpha)
{
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);
    glColor4f (1.0f, 1.0f, 1.0f, alpha);

    const GLshort y = (GLshort) (getHeight() - anchorPosAndTextureSize.getHeight());
    const GLshort width = (GLshort) anchorPosAndTextureSize.getWidth();
    const GLshort bottom = (GLshort) (y + anchorPosAndTextureSize.getHeight());
    const GLshort vertices[] = { 0, y, width, y, 0, bottom, width, bottom };

   #if JUCE_USE_OPENGL_SHADERS
    if (getShaderLanguageVersion() > 1.199)
    {
        struct OverlayShaderProgram  : public ReferenceCountedObject
        {
            OverlayShaderProgram (OpenGLContext& context)
                : program (context),
                  builder (program),
                  params (program)
            {}

            static const OverlayShaderProgram& select (OpenGLContext& context)
            {
                static const Identifier programValueID ("juceGLComponentOverlayShader");
                OverlayShaderProgram* program = dynamic_cast <OverlayShaderProgram*> (context.properties [programValueID].getObject());

                if (program == nullptr)
                {
                    program = new OverlayShaderProgram (context);
                    context.properties.set (programValueID, var (program));
                }

                program->program.use();
                return *program;
            }

            struct ProgramBuilder
            {
                ProgramBuilder (OpenGLShaderProgram& program)
                {
                    program.addShader ("attribute vec2 position;"
                                       "uniform vec2 screenSize;"
                                       "void main()"
                                       "{"
                                       " vec2 scaled = position / (0.5 * screenSize.xy);"
                                       " gl_Position = vec4 (scaled.x - 1.0, 1.0 - scaled.y, 0, 1.0);"
                                       "}",
                                       GL_VERTEX_SHADER);

                    program.addShader ("#version 120\n"
                                       "uniform sampler2D imageTexture;"
                                       "uniform float matrix[6];"
                                       "void main()"
                                       "{"
                                        "vec2 texturePos = mat2 (matrix[0], matrix[3], matrix[1], matrix[4]) * gl_FragCoord.xy"
                                                          " + vec2 (matrix[2], matrix[5]);"
                                        "gl_FragColor = gl_Color.a * texture2D (imageTexture, vec2 (texturePos.x, 1.0 - texturePos.y));"
                                       "}",
                                       GL_FRAGMENT_SHADER);
                    program.link();
                }
            };

            struct Params
            {
                Params (OpenGLShaderProgram& program)
                    : positionAttribute (program, "position"),
                      screenSize (program, "screenSize"),
                      imageTexture (program, "imageTexture"),
                      matrix (program, "matrix")
                {}

                void set (const int targetWidth, const int targetHeight, const Rectangle<float>& anchorPosAndTextureSize) const
                {
                    const AffineTransform t (AffineTransform::translation (-anchorPosAndTextureSize.getX(),
                                                                           -anchorPosAndTextureSize.getY())
                                                .followedBy (AffineTransform::verticalFlip (targetHeight))
                                                .inverted().scaled (1.0f / anchorPosAndTextureSize.getWidth(),
                                                                    1.0f / anchorPosAndTextureSize.getHeight()));

                    const GLfloat m[] = { t.mat00, t.mat01, t.mat02, t.mat10, t.mat11, t.mat12 };
                    matrix.set (m, 6);
                    imageTexture.set (0);
                    screenSize.set (targetWidth, targetHeight);
                }

                OpenGLShaderProgram::Attribute positionAttribute;
                OpenGLShaderProgram::Uniform screenSize, imageTexture, matrix;
            };

            OpenGLShaderProgram program;
            ProgramBuilder builder;
            Params params;
        };

        const OverlayShaderProgram& program = OverlayShaderProgram::select (*this);
        program.params.set (getWidth(), getHeight(), anchorPosAndTextureSize.toFloat());

        extensions.glVertexAttribPointer (program.params.positionAttribute.attributeID, 2, GL_SHORT, GL_FALSE, 4, vertices);
        extensions.glEnableVertexAttribArray (program.params.positionAttribute.attributeID);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        extensions.glUseProgram (0);
    }
    #if JUCE_USE_OPENGL_FIXED_FUNCTION
    else
    #endif
   #endif

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    {
        OpenGLHelpers::prepareFor2D (getWidth(), getHeight());

        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);

        const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
        glVertexPointer (2, GL_SHORT, 0, vertices);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    }
   #endif
}
