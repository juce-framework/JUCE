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

OpenGLShaderProgram::OpenGLShaderProgram() noexcept
{
    // This object can only be created and used when the current thread has an active OpenGL context.
    jassert (OpenGLHelpers::isContextActive());

    programID = glCreateProgram();
}

OpenGLShaderProgram::~OpenGLShaderProgram() noexcept
{
    glDeleteProgram (programID);
}

double OpenGLShaderProgram::getLanguageVersion()
{
    return String ((const char*) glGetString (GL_SHADING_LANGUAGE_VERSION))
            .upToFirstOccurrenceOf (" ", false, false).getDoubleValue();
}

void OpenGLShaderProgram::addShader (const char* const code, GLenum type)
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

    glAttachShader (programID, shaderID);
    glDeleteShader (shaderID);
}

void OpenGLShaderProgram::link() noexcept
{
    glLinkProgram (programID);

   #if JUCE_DEBUG
    GLint status = 0;
    glGetProgramiv (programID, GL_LINK_STATUS, &status);
    jassert (status != GL_FALSE);
   #endif
}

OpenGLShaderProgram::Uniform::Uniform (const OpenGLShaderProgram& program, const char* const name)
    : uniformID (glGetUniformLocation (program.programID, name))
{
    jassert (uniformID >= 0);
}

OpenGLShaderProgram::Attribute::Attribute (const OpenGLShaderProgram& program, const char* name)
    : attributeID (glGetAttribLocation (program.programID, name))
{
    jassert (attributeID >= 0);
}

void OpenGLShaderProgram::Uniform::set (GLfloat n1) const noexcept                                    { glUniform1f (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLint n1) const noexcept                                      { glUniform1i (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2) const noexcept                        { glUniform2f (uniformID, n1, n2); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept            { glUniform3f (uniformID, n1, n2, n3); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3, float n4) const noexcept  { glUniform4f (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept        { glUniform4i (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (const GLfloat* values, GLsizei numValues) const noexcept      { glUniform1fv (uniformID, numValues, values); }
