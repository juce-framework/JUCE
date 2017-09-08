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
    {
        String output;

       #if JUCE_ANDROID
        {
            int numAttributes = 0;

            for (int p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
                numAttributes++;

            int last = 0;

            for (int p = code.indexOf (0, "attribute "); p >= 0; p = code.indexOf (p + 1, "attribute "))
            {
                output += code.substring (last, p) + String ("layout(location=") + String (--numAttributes) + ") in ";

                last = p + 10;
            }

            output += code.substring (last);
        }
       #else
        output = code.replace ("attribute", "in");
       #endif

        return JUCE_GLSL_VERSION "\n" + output.replace ("varying", "out");
    }
   #endif

    return code;
}

String OpenGLHelpers::translateFragmentShaderToV3 (const String& code)
{
   #if JUCE_OPENGL3
    if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
        return JUCE_GLSL_VERSION "\n"
               "out " JUCE_MEDIUMP " vec4 fragColor;\n"
                + code.replace ("varying", "in")
                      .replace ("texture2D", "texture")
                      .replace ("gl_FragColor", "fragColor");
   #endif

    return code;
}

} // namespace juce
