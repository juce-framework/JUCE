/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

OpenGLShaderProgram::OpenGLShaderProgram (const OpenGLContext& c) noexcept  : context (c)
{
}

OpenGLShaderProgram::~OpenGLShaderProgram() noexcept
{
    release();
}

GLuint OpenGLShaderProgram::getProgramID() const noexcept
{
    if (programID == 0)
    {
        // This method should only be called when the current thread has an active OpenGL context.
        jassert (OpenGLHelpers::isContextActive());

        programID = context.extensions.glCreateProgram();
    }

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

    if (status == (GLint) GL_FALSE)
    {
        std::vector<GLchar> infoLog (16384);
        GLsizei infoLogLength = 0;
        context.extensions.glGetShaderInfoLog (shaderID, (GLsizei) infoLog.size(), &infoLogLength, infoLog.data());
        errorLog = String (infoLog.data(), (size_t) infoLogLength);

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

    if (status == (GLint) GL_FALSE)
    {
        std::vector<GLchar> infoLog (16384);
        GLsizei infoLogLength = 0;
        context.extensions.glGetProgramInfoLog (progID, (GLsizei) infoLog.size(), &infoLogLength, infoLog.data());
        errorLog = String (infoLog.data(), (size_t) infoLogLength);

       #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
        // Your GLSL code contained link errors!
        // Hopefully this compile log should help to explain what went wrong.
        DBG (errorLog);
        jassertfalse;
       #endif
    }

    JUCE_CHECK_OPENGL_ERROR
    return status != (GLint) GL_FALSE;
}

void OpenGLShaderProgram::use() const noexcept
{
    // The shader program must have been successfully linked when this method is called!
    jassert (programID != 0);

    context.extensions.glUseProgram (programID);
}

GLint OpenGLShaderProgram::getUniformIDFromName (const char* uniformName) const noexcept
{
    // The shader program must be active when this method is called!
    jassert (programID != 0);

    return (GLint) context.extensions.glGetUniformLocation (programID, uniformName);
}

void OpenGLShaderProgram::setUniform (const char* name, GLfloat n1) noexcept                                       { context.extensions.glUniform1f  (getUniformIDFromName (name), n1); }
void OpenGLShaderProgram::setUniform (const char* name, GLint n1) noexcept                                         { context.extensions.glUniform1i  (getUniformIDFromName (name), n1); }
void OpenGLShaderProgram::setUniform (const char* name, GLfloat n1, GLfloat n2) noexcept                           { context.extensions.glUniform2f  (getUniformIDFromName (name), n1, n2); }
void OpenGLShaderProgram::setUniform (const char* name, GLfloat n1, GLfloat n2, GLfloat n3) noexcept               { context.extensions.glUniform3f  (getUniformIDFromName (name), n1, n2, n3); }
void OpenGLShaderProgram::setUniform (const char* name, GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4) noexcept   { context.extensions.glUniform4f  (getUniformIDFromName (name), n1, n2, n3, n4); }
void OpenGLShaderProgram::setUniform (const char* name, GLint n1, GLint n2, GLint n3, GLint n4) noexcept           { context.extensions.glUniform4i  (getUniformIDFromName (name), n1, n2, n3, n4); }
void OpenGLShaderProgram::setUniform (const char* name, const GLfloat* values, GLsizei numValues) noexcept         { context.extensions.glUniform1fv (getUniformIDFromName (name), numValues, values); }
void OpenGLShaderProgram::setUniformMat2 (const char* name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix2fv (getUniformIDFromName (name), num, trns, v); }
void OpenGLShaderProgram::setUniformMat3 (const char* name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix3fv (getUniformIDFromName (name), num, trns, v); }
void OpenGLShaderProgram::setUniformMat4 (const char* name, const GLfloat* v, GLint num, GLboolean trns) noexcept  { context.extensions.glUniformMatrix4fv (getUniformIDFromName (name), num, trns, v); }

//==============================================================================
OpenGLShaderProgram::Attribute::Attribute (const OpenGLShaderProgram& program, const char* name)
    : attributeID ((GLuint) program.context.extensions.glGetAttribLocation (program.getProgramID(), name))
{
   #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert ((GLint) attributeID >= 0);
   #endif
}

//==============================================================================
OpenGLShaderProgram::Uniform::Uniform (const OpenGLShaderProgram& program, const char* const name)
    : uniformID (program.context.extensions.glGetUniformLocation (program.getProgramID(), name)), context (program.context)
{
   #if JUCE_DEBUG && ! JUCE_DONT_ASSERT_ON_GLSL_COMPILE_ERROR
    jassert (uniformID >= 0);
   #endif
}

void OpenGLShaderProgram::Uniform::set (GLfloat n1) const noexcept                                    { context.extensions.glUniform1f (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLint n1) const noexcept                                      { context.extensions.glUniform1i (uniformID, n1); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2) const noexcept                        { context.extensions.glUniform2f (uniformID, n1, n2); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept            { context.extensions.glUniform3f (uniformID, n1, n2, n3); }
void OpenGLShaderProgram::Uniform::set (GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4) const noexcept  { context.extensions.glUniform4f (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept        { context.extensions.glUniform4i (uniformID, n1, n2, n3, n4); }
void OpenGLShaderProgram::Uniform::set (const GLfloat* values, GLsizei numValues) const noexcept      { context.extensions.glUniform1fv (uniformID, numValues, values); }

void OpenGLShaderProgram::Uniform::setMatrix2 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix2fv (uniformID, num, trns, v); }
void OpenGLShaderProgram::Uniform::setMatrix3 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix3fv (uniformID, num, trns, v); }
void OpenGLShaderProgram::Uniform::setMatrix4 (const GLfloat* v, GLint num, GLboolean trns) const noexcept { context.extensions.glUniformMatrix4fv (uniformID, num, trns, v); }

} // namespace juce
