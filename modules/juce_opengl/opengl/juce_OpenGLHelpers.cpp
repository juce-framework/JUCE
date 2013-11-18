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

void OpenGLHelpers::resetErrorState()
{
    while (glGetError() != GL_NO_ERROR) {}
}

void* OpenGLHelpers::getExtensionFunction (const char* functionName)
{
   #if JUCE_WINDOWS
    return (void*) wglGetProcAddress (functionName);
   #elif JUCE_LINUX
    return (void*) glXGetProcAddress ((const GLubyte*) functionName);
   #else
    static void* handle = dlopen (nullptr, RTLD_LAZY);
    return dlsym (handle, functionName);
   #endif
}

bool OpenGLHelpers::isExtensionSupported (const char* const extensionName)
{
    jassert (extensionName != nullptr); // you must supply a genuine string for this.
    jassert (isContextActive()); // An OpenGL context will need to be active before calling this.

    const char* extensions = (const char*) glGetString (GL_EXTENSIONS);
    jassert (extensions != nullptr); // Perhaps you didn't activate an OpenGL context before calling this?

    for (;;)
    {
        const char* found = strstr (extensions, extensionName);

        if (found == nullptr)
            break;

        extensions = found + strlen (extensionName);

        if (extensions[0] == ' ' || extensions[0] == 0)
            return true;
    }

    return false;
}

void OpenGLHelpers::clear (Colour colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#if JUCE_USE_OPENGL_FIXED_FUNCTION
void OpenGLHelpers::setColour (Colour colour)
{
    glColor4f (colour.getFloatRed(), colour.getFloatGreen(),
               colour.getFloatBlue(), colour.getFloatAlpha());
}

void OpenGLHelpers::prepareFor2D (const int width, const int height)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    glOrthof (0.0f, (GLfloat) width, 0.0f, (GLfloat) height, 0.0f, 1.0f);
   #else
    glOrtho  (0.0, width, 0.0, height, 0, 1);
   #endif

    glViewport (0, 0, width, height);
}

void OpenGLHelpers::setPerspective (double fovy, double aspect, double zNear, double zFar)
{
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    const GLfloat ymax = (GLfloat) (zNear * tan (fovy * double_Pi / 360.0));
    const GLfloat ymin = -ymax;

    glFrustumf (ymin * (GLfloat) aspect, ymax * (GLfloat) aspect, ymin, ymax, (GLfloat) zNear, (GLfloat) zFar);
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

void OpenGLHelpers::applyMatrix (const float matrixValues[16])
{
    glMultMatrixf (matrixValues);
}

#if ! JUCE_OPENGL_ES
void OpenGLHelpers::applyMatrix (const double matrixValues[16])
{
    glMultMatrixd (matrixValues);
}
#endif
#endif

void OpenGLHelpers::enableScissorTest (const Rectangle<int>& clip)
{
    glEnable (GL_SCISSOR_TEST);
    glScissor (clip.getX(), clip.getY(), clip.getWidth(), clip.getHeight());
}

#if JUCE_USE_OPENGL_FIXED_FUNCTION
void OpenGLHelpers::drawQuad2D (float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                Colour colour)
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
                                Colour colour)
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

void OpenGLHelpers::drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords, const int numVertices) noexcept
{
   #if ! JUCE_ANDROID
    glEnable (GL_TEXTURE_2D);
    clearGLError();
   #endif
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (2, GL_FLOAT, 0, vertices);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, numVertices);
}

void OpenGLHelpers::drawTriangleStrip (const GLfloat* const vertices, const GLfloat* const textureCoords,
                                       const int numVertices, const GLuint textureID) noexcept
{
    jassert (textureID != 0);
    glBindTexture (GL_TEXTURE_2D, textureID);
    drawTriangleStrip (vertices, textureCoords, numVertices);
    glBindTexture (GL_TEXTURE_2D, 0);
}

void OpenGLHelpers::drawTextureQuad (GLuint textureID, const Rectangle<int>& rect)
{
    const GLfloat l = (GLfloat) rect.getX();
    const GLfloat t = (GLfloat) rect.getY();
    const GLfloat r = (GLfloat) rect.getRight();
    const GLfloat b = (GLfloat) rect.getBottom();

    const GLfloat vertices[]      = { l, t, r, t, l, b, r, b };
    const GLfloat textureCoords[] = { 0, 1.0f, 1.0f, 1.0f, 0, 0, 1.0f, 0 };

    drawTriangleStrip (vertices, textureCoords, 4, textureID);
}

void OpenGLHelpers::fillRectWithTexture (const Rectangle<int>& rect, GLuint textureID, const float alpha)
{
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glColor4f (alpha, alpha, alpha, alpha);

    drawTextureQuad (textureID, rect);
}

void OpenGLHelpers::fillRectWithColour (const Rectangle<int>& rect, Colour colour)
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
    const GLfloat vertices[] = { (GLfloat) rect.getX(),     (GLfloat) rect.getY(),
                                 (GLfloat) rect.getRight(), (GLfloat) rect.getY(),
                                 (GLfloat) rect.getX(),     (GLfloat) rect.getBottom(),
                                 (GLfloat) rect.getRight(), (GLfloat) rect.getBottom() };

    glVertexPointer (2, GL_FLOAT, 0, vertices);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}
#endif

//==============================================================================
OpenGLTextureFromImage::OpenGLTextureFromImage (const Image& image)
    : imageWidth (image.getWidth()),
      imageHeight (image.getHeight())
{
    JUCE_CHECK_OPENGL_ERROR
    if (OpenGLFrameBuffer* const fb = OpenGLImageType::getFrameBufferFrom (image))
    {
        textureID = fb->getTextureID();
        fullWidthProportion  = 1.0f;
        fullHeightProportion = 1.0f;
    }
    else
    {
        texture = new OpenGLTexture();
        texture->loadImage (image);
        textureID = texture->getTextureID();

        fullWidthProportion  = imageWidth  / (float) texture->getWidth();
        fullHeightProportion = imageHeight / (float) texture->getHeight();
    }

    JUCE_CHECK_OPENGL_ERROR
}

OpenGLTextureFromImage::~OpenGLTextureFromImage() {}
