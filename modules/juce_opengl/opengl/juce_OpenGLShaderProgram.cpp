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

OpenGLShaderProgram::OpenGLShaderProgram (const OpenGLContext& c) noexcept
    : context (c), programID (0)
{
}

OpenGLShaderProgram::~OpenGLShaderProgram() noexcept
{
    release();
}

GLuint OpenGLShaderProgram::getProgramID() const noexcept
{
    // This method can only be used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    if (programID == 0)
        programID = context.extensions.glCreateProgram();

    return programID;
}

void OpenGLShaderProgram::release() noexcept
{
    if (programID != 0)
    {
        context.extensions.glDeleteProgram (programID);
        programID = 0;
    }
}

double OpenGLShaderProgram::getLanguageVersion()
{
    return String::fromUTF8 ((const char*) glGetString (GL_SHADING_LANGUAGE_VERSION))
            .retainCharacters("1234567890.").getDoubleValue();
}

bool OpenGLShaderProgram::addShader (const String& code, GLenum type)
{
    GLuint shaderID = context.extensions.glCreateShader (type);

    const GLchar* c = code.toRawUTF8();
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

    context.extensions.glAttachShader (getProgramID(), shaderID);
    context.extensions.glDeleteShader (shaderID);
    JUCE_CHECK_OPENGL_ERROR
    return true;
}

bool OpenGLShaderProgram::addVertexShader (const String& code)    { return addShader (code, GL_VERTEX_SHADER); }
bool OpenGLShaderProgram::addFragmentShader (const String& code)  { return addShader (code, GL_FRAGMENT_SHADER); }

bool OpenGLShaderProgram::link() noexcept
{
    // This method can only be used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    GLuint progID = getProgramID();

    context.extensions.glLinkProgram (progID);

    GLint status = GL_FALSE;
    context.extensions.glGetProgramiv (progID, GL_LINK_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLchar infoLog [16384];
        GLsizei infoLogLength = 0;
        context.extensions.glGetProgramInfoLog (progID, sizeof (infoLog), &infoLogLength, infoLog);
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
    : uniformID (program.context.extensions.glGetUniformLocation (program.getProgramID(), name)), context (program.context)
{
   #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert (uniformID >= 0);
   #endif
}

OpenGLShaderProgram::Attribute::Attribute (const OpenGLShaderProgram& program, const char* name)
    : attributeID ((GLuint) program.context.extensions.glGetAttribLocation (program.getProgramID(), name))
{
   #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert ((GLint) attributeID >= 0);
   #endif
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
