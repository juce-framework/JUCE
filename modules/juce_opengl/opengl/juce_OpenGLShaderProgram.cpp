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

#if JUCE_USE_OPENGL_SHADERS

OpenGLShaderProgram::OpenGLShaderProgram (const OpenGLContext& c) noexcept
    : context (c)
{
    // This object can only be created and used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    programID = context.extensions.glCreateProgram();
}

OpenGLShaderProgram::~OpenGLShaderProgram() noexcept
{
    context.extensions.glDeleteProgram (programID);
}

double OpenGLShaderProgram::getLanguageVersion()
{
   #if JUCE_OPENGL_ES
    // GLES doesn't support this version number, but that shouldn't matter since
    // on GLES you probably won't need to check it.
    jassertfalse;
    return 0;
   #else
    return String ((const char*) glGetString (GL_SHADING_LANGUAGE_VERSION))
            .upToFirstOccurrenceOf (" ", false, false).getDoubleValue();
   #endif
}

bool OpenGLShaderProgram::addShader (StringRef code, GLenum type)
{
    GLuint shaderID = context.extensions.glCreateShader (type);

   #if JUCE_STRING_UTF_TYPE == 8
    const GLchar* c = code.text;
   #else
    String codeString (code.text);
    const GLchar* c = codeString.toRawUTF8();
   #endif

    context.extensions.glShaderSource (shaderID, 1, &c, nullptr);
    context.extensions.glCompileShader (shaderID);

    GLint status = GL_FALSE;
    context.extensions.glGetShaderiv (shaderID, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar infoLog [16384];
        GLsizei infoLogLength = 0;
        context.extensions.glGetShaderInfoLog (shaderID, sizeof (infoLog), &infoLogLength, infoLog);
        errorLog = String (infoLog, (size_t) infoLogLength);

       #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
        // Your GLSL code contained compile errors!
        // Hopefully this compile log should help to explain what went wrong.
        DBG (errorLog);
        jassertfalse;
       #endif

        return false;
    }

    context.extensions.glAttachShader (programID, shaderID);
    context.extensions.glDeleteShader (shaderID);
    JUCE_CHECK_OPENGL_ERROR
    return true;
}

bool OpenGLShaderProgram::addVertexShader (StringRef code)    { return addShader (code, GL_VERTEX_SHADER); }
bool OpenGLShaderProgram::addFragmentShader (StringRef code)  { return addShader (code, GL_FRAGMENT_SHADER); }

bool OpenGLShaderProgram::link() noexcept
{
    context.extensions.glLinkProgram (programID);

    GLint status = GL_FALSE;
    context.extensions.glGetProgramiv (programID, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar infoLog [16384];
        GLsizei infoLogLength = 0;
        context.extensions.glGetProgramInfoLog (programID, sizeof (infoLog), &infoLogLength, infoLog);
        errorLog = String (infoLog, (size_t) infoLogLength);

       #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
        // Your GLSL code contained link errors!
        // Hopefully this compile log should help to explain what went wrong.
        DBG (errorLog);
        jassertfalse;
       #endif
    }

    JUCE_CHECK_OPENGL_ERROR
    return status != GL_FALSE;
}

void OpenGLShaderProgram::use() const noexcept
{
    context.extensions.glUseProgram (programID);
}

OpenGLShaderProgram::Uniform::Uniform (const OpenGLShaderProgram& program, const char* const name)
    : uniformID (program.context.extensions.glGetUniformLocation (program.programID, name)), context (program.context)
{
    jassert (uniformID >= 0);
}

OpenGLShaderProgram::Attribute::Attribute (const OpenGLShaderProgram& program, const char* name)
    : attributeID (program.context.extensions.glGetAttribLocation (program.programID, name))
{
    jassert (attributeID >= 0);
}

void OpenGLShaderProgram::Uniform::set (GLfloat n1) const noexcept                                    { context.extensions.glUniform1f (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLint n1) const noexcept                                      { context.extensions.glUniform1i (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2) const noexcept                        { context.extensions.glUniform2f (uniformID, n1, n2); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept            { context.extensions.glUniform3f (uniformID, n1, n2, n3); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3, float n4) const noexcept  { context.extensions.glUniform4f (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept        { context.extensions.glUniform4i (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (const GLfloat* values, GLsizei numValues) const noexcept      { context.extensions.glUniform1fv (uniformID, numValues, values); }

void OpenGLShaderProgram::Uniform::setMatrix2 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix2fv (uniformID, num, trns, v); }
void OpenGLShaderProgram::Uniform::setMatrix3 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix3fv (uniformID, num, trns, v); }
void OpenGLShaderProgram::Uniform::setMatrix4 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix4fv (uniformID, num, trns, v); }

#endif
