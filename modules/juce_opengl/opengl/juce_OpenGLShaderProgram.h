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

#ifndef JUCE_OPENGLSHADERPROGRAM_H_INCLUDED
#define JUCE_OPENGLSHADERPROGRAM_H_INCLUDED

//==============================================================================
/**
    Manages an OpenGL shader program.
*/
class JUCE_API  OpenGLShaderProgram
{
public:
    OpenGLShaderProgram (const OpenGLContext&) noexcept;
    ~OpenGLShaderProgram() noexcept;

    /** Returns the version of GLSL that the current context supports.
        E.g.
        @code
        if (OpenGLShaderProgram::getLanguageVersion() > 1.199)
        {
            // ..do something that requires GLSL 1.2 or above..
        }
        @endcode
    */
    static double getLanguageVersion();

    /** Compiles and adds a shader to this program.

        After adding all your shaders, remember to call link() to link them into
        a usable program.

        If your app is built in debug mode, this method will assert if the program
        fails to compile correctly.

        The shaderType parameter could be GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, etc.

        @returns  true if the shader compiled successfully. If not, you can call
                  getLastError() to find out what happened.
    */
    bool addShader (const String& shaderSourceCode, GLenum shaderType);

    /** Compiles and adds a fragment shader to this program.
        This is equivalent to calling addShader() with a type of GL_VERTEX_SHADER.
    */
    bool addVertexShader (const String& shaderSourceCode);

    /** Compiles and adds a fragment shader to this program.
        This is equivalent to calling addShader() with a type of GL_FRAGMENT_SHADER.
    */
    bool addFragmentShader (const String& shaderSourceCode);

    /** Links all the compiled shaders into a usable program.
        If your app is built in debug mode, this method will assert if the program
        fails to link correctly.
        @returns  true if the program linked successfully. If not, you can call
                  getLastError() to find out what happened.
    */
    bool link() noexcept;

    /** Get the output for the last shader compilation or link that failed. */
    const String& getLastError() const noexcept             { return errorLog; }

    /** Selects this program into the current context. */
    void use() const noexcept;

    /** Deletes the program. */
    void release() noexcept;

    /** Represents an openGL uniform value.
        After a program has been linked, you can create Uniform objects to let you
        set the uniforms that your shaders use.

        Be careful not to call the set() functions unless the appropriate program
        is loaded into the current context.
    */
    struct Uniform
    {
        /** Initialises a uniform.
            The program must have been successfully linked when this
            constructor is called.
        */
        Uniform (const OpenGLShaderProgram& program, const char* uniformName);

        /** Sets a float uniform. */
        void set (GLfloat n1) const noexcept;
        /** Sets an int uniform. */
        void set (GLint n1) const noexcept;
        /** Sets a vec2 uniform. */
        void set (GLfloat n1, GLfloat n2) const noexcept;
        /** Sets a vec3 uniform. */
        void set (GLfloat n1, GLfloat n2, GLfloat n3) const noexcept;
        /** Sets a vec4 uniform. */
        void set (GLfloat n1, GLfloat n2, GLfloat n3, float n4) const noexcept;
        /** Sets an ivec4 uniform. */
        void set (GLint n1, GLint n2, GLint n3, GLint n4) const noexcept;
        /** Sets a vector float uniform. */
        void set (const GLfloat* values, int numValues) const noexcept;
        /** Sets a 2x2 matrix float uniform. */
        void setMatrix2 (const GLfloat* values, GLint count, GLboolean transpose) const noexcept;
        /** Sets a 3x3 matrix float uniform. */
        void setMatrix3 (const GLfloat* values, GLint count, GLboolean transpose) const noexcept;
        /** Sets a 4x4 matrix float uniform. */
        void setMatrix4 (const GLfloat* values, GLint count, GLboolean transpose) const noexcept;

        /** The uniform's ID number.
            If the uniform couldn't be found, this value will be < 0.
        */
        GLint uniformID;

    private:
        const OpenGLContext& context;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Uniform)
    };

    /** Represents an openGL vertex attribute value.
        After a program has been linked, you can create Attribute objects to let you
        set the attributes that your vertex shaders use.
    */
    struct Attribute
    {
        /** Initialises an attribute.
            The program must have been successfully linked when this
            constructor is called.
        */
        Attribute (const OpenGLShaderProgram& program, const char* attributeName);

        /** The attribute's ID number.
            If the uniform couldn't be found, this value will be < 0.
        */
        GLuint attributeID;
    };

    /** The ID number of the compiled program. */
    GLuint getProgramID() const noexcept;

private:
    const OpenGLContext& context;
    mutable GLuint programID;
    String errorLog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLShaderProgram)
};

#endif   // JUCE_OPENGLSHADERPROGRAM_H_INCLUDED
