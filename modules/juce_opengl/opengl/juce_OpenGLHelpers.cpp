/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

void OpenGLHelpers::enableScissorTest (const Rectangle<int>& clip)
{
    glEnable (GL_SCISSOR_TEST);
    glScissor (clip.getX(), clip.getY(), clip.getWidth(), clip.getHeight());
}

String OpenGLHelpers::translateVertexShaderToV3 (const String& code)
{
   #if JUCE_OPENGL3
    if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
        return JUCE_GLSL_VERSION "\n" + code.replace ("attribute", "in")
                                            .replace ("varying", "out");
   #endif

    return code;
}

String OpenGLHelpers::translateFragmentShaderToV3 (const String& code)
{
   #if JUCE_OPENGL3
    if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
        return JUCE_GLSL_VERSION "\n"
               "out vec4 fragColor;\n"
                + code.replace ("varying", "in")
                      .replace ("texture2D", "texture")
                      .replace ("gl_FragColor", "fragColor");
   #endif

    return code;
}
