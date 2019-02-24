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

//==============================================================================
/**
    Manages an OpenGL shader program.

    @tags{OpenGL}
*/
class JUCE_API  OpenGLShaderProgram
{
public:
    /** Creates a shader for use in a particular GL context. */
    OpenGLShaderProgram (const OpenGLContext&) noexcept;

    /** Destructor. */
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

    //==============================================================================
    //  Methods for setting shader uniforms without using a Uniform object (see below).
    //  You must make sure this shader is the currently bound one before setting uniforms
    //  with these functions.

    /** Get the uniform ID from the variable name */
    GLint getUniformIDFromName (const char* uniformName) const noexcept;

    /** Sets a float uniform. */
    void setUniform (const char* uniformName, GLfloat value) noexcept;
    /** Sets an int uniform. */
    void setUniform (const char* uniformName, GLint value) noexcept;
    /** Sets a vec2 uniform. */
    void setUniform (const char* uniformName, GLfloat x, GLfloat y) noexcept;
    /** Sets a vec3 uniform. */
    void setUniform (const char* uniformName, GLfloat x, GLfloat y, GLfloat z) noexcept;
    /** Sets a vec4 uniform. */
    void setUniform (const char* uniformName, GLfloat x, GLfloat y, GLfloat z, GLfloat w) noexcept;
    /** Sets a vec4 uniform. */
    void setUniform (const char* uniformName, GLint x, GLint y, GLint z, GLint w) noexcept;
    /** Sets a vector float uniform. */
    void setUniform (const char* uniformName, const GLfloat* values, GLsizei numValues) noexcept;
    /** Sets a 2x2 matrix float uniform. */
    void setUniformMat2 (const char* uniformName, const GLfloat* values, GLint count, GLboolean transpose) noexcept;
    /** Sets a 3x3 matrix float uniform. */
    void setUniformMat3 (const char* uniformName, const GLfloat* values, GLint count, GLboolean transpose) noexcept;
    /** Sets a 4x4 matrix float uniform. */
    void setUniformMat4 (const char* uniformName, const GLfloat* values, GLint count, GLboolean transpose) noexcept;

    //==============================================================================
    /** Represents an openGL uniform value.
        After a program has been linked, you can create Uniform objects to let you
        set the uniforms that your shaders use.

        Be careful not to call the set() functions unless the appropriate program
        is loaded into the current context.
    */
    struct JUCE_API  Uniform
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
        void set (GLfloat n1, GLfloat n2, GLfloat n3, GLfloat n4) const noexcept;
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

    //==============================================================================
    /** Represents an openGL vertex attribute value.
        After a program has been linked, you can create Attribute objects to let you
        set the attributes that your vertex shaders use.
    */
    struct JUCE_API  Attribute
    {
        /** Initialises an attribute.
            The program must have been successfully linked when this
            constructor is called.
        */
        Attribute (const OpenGLShaderProgram&, const char* attributeName);

        /** The attribute's ID number.
            If the uniform couldn't be found, this value will be < 0.
        */
        GLuint attributeID;
    };

    /** The ID number of the compiled program. */
    GLuint getProgramID() const noexcept;

private:
    const OpenGLContext& context;
    mutable GLuint programID = 0;
    String errorLog;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLShaderProgram)
};

} // namespace juce
