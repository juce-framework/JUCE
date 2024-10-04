/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_BASE_FUNCTIONS \
    X (glActiveTexture) \
    X (glBindBuffer) \
    X (glDeleteBuffers) \
    X (glGenBuffers) \
    X (glBufferData) \
    X (glBufferSubData) \
    X (glCreateProgram) \
    X (glDeleteProgram) \
    X (glCreateShader) \
    X (glDeleteShader) \
    X (glShaderSource) \
    X (glCompileShader) \
    X (glAttachShader) \
    X (glLinkProgram) \
    X (glUseProgram) \
    X (glGetShaderiv) \
    X (glGetShaderInfoLog) \
    X (glGetProgramInfoLog) \
    X (glGetProgramiv) \
    X (glGetUniformLocation) \
    X (glGetAttribLocation) \
    X (glVertexAttribPointer) \
    X (glEnableVertexAttribArray) \
    X (glDisableVertexAttribArray) \
    X (glUniform1f) \
    X (glUniform1i) \
    X (glUniform2f) \
    X (glUniform3f) \
    X (glUniform4f) \
    X (glUniform4i) \
    X (glUniform1fv) \
    X (glUniformMatrix2fv) \
    X (glUniformMatrix3fv) \
    X (glUniformMatrix4fv) \
    X (glBindAttribLocation)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_EXTENSION_FUNCTIONS \
    X (glIsRenderbuffer) \
    X (glBindRenderbuffer) \
    X (glDeleteRenderbuffers) \
    X (glGenRenderbuffers) \
    X (glRenderbufferStorage) \
    X (glGetRenderbufferParameteriv) \
    X (glIsFramebuffer) \
    X (glBindFramebuffer) \
    X (glDeleteFramebuffers) \
    X (glGenFramebuffers) \
    X (glCheckFramebufferStatus) \
    X (glFramebufferTexture2D) \
    X (glFramebufferRenderbuffer) \
    X (glGetFramebufferAttachmentParameteriv)

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_VERTEXBUFFER_FUNCTIONS \
    X (glGenVertexArrays) \
    X (glDeleteVertexArrays) \
    X (glBindVertexArray)

/** This class contains a generated list of OpenGL extension functions, which are either dynamically loaded
    for a specific GL context, or simply call-through to the appropriate OS function where available.

    This class is provided for backwards compatibility. In new code, you should prefer to use
    functions from the juce::gl namespace. By importing all these symbols with
    `using namespace ::juce::gl;`, all GL enumerations and functions will be made available at
    global scope. This may be helpful if you need to write code with C source compatibility, or
    which is compatible with a different extension-loading library.
    All the normal guidance about `using namespace` should still apply - don't do this in a header,
    or at all if you can possibly avoid it!

    @tags{OpenGL}
*/
struct OpenGLExtensionFunctions
{
    //==============================================================================
   #ifndef DOXYGEN
    [[deprecated ("A more complete set of GL commands can be found in the juce::gl namespace. "
                  "You should use juce::gl::loadFunctions() to load GL functions.")]]
    static void initialise();
   #endif

   #if JUCE_WINDOWS && ! defined (DOXYGEN)
    typedef char GLchar;
    typedef pointer_sized_int GLsizeiptr;
    typedef pointer_sized_int GLintptr;
   #endif

   #define X(name) static decltype (::juce::gl::name)& name;
    JUCE_GL_BASE_FUNCTIONS
    JUCE_GL_EXTENSION_FUNCTIONS
    JUCE_GL_VERTEXBUFFER_FUNCTIONS
   #undef X
};

enum MissingOpenGLDefinitions
{
   #if JUCE_ANDROID
    JUCE_RGBA_FORMAT                = ::juce::gl::GL_RGBA,
   #else
    JUCE_RGBA_FORMAT                = ::juce::gl::GL_BGRA_EXT,
   #endif
};

} // namespace juce
