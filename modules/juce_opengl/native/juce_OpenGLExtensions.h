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

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_BASE_FUNCTIONS(USE_FUNCTION) \
    USE_FUNCTION (glActiveTexture,          void, (GLenum p1), (p1))\
    USE_FUNCTION (glBindBuffer,             void, (GLenum p1, GLuint p2), (p1, p2))\
    USE_FUNCTION (glDeleteBuffers,          void, (GLsizei p1, const GLuint* p2), (p1, p2))\
    USE_FUNCTION (glGenBuffers,             void, (GLsizei p1, GLuint* p2), (p1, p2))\
    USE_FUNCTION (glBufferData,             void, (GLenum p1, GLsizeiptr p2, const GLvoid* p3, GLenum p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glBufferSubData,          void, (GLenum p1, GLintptr p2, GLsizeiptr p3, const GLvoid* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glCreateProgram,          GLuint, (), ())\
    USE_FUNCTION (glDeleteProgram,          void, (GLuint p1), (p1))\
    USE_FUNCTION (glCreateShader,           GLuint, (GLenum p1), (p1))\
    USE_FUNCTION (glDeleteShader,           void, (GLuint p1), (p1))\
    USE_FUNCTION (glShaderSource,           void, (GLuint p1, GLsizei p2, const GLchar** p3, const GLint* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glCompileShader,          void, (GLuint p1), (p1))\
    USE_FUNCTION (glAttachShader,           void, (GLuint p1, GLuint p2), (p1, p2))\
    USE_FUNCTION (glLinkProgram,            void, (GLuint p1), (p1))\
    USE_FUNCTION (glUseProgram,             void, (GLuint p1), (p1))\
    USE_FUNCTION (glGetShaderiv,            void, (GLuint p1, GLenum p2, GLint* p3), (p1, p2, p3))\
    USE_FUNCTION (glGetShaderInfoLog,       void, (GLuint p1, GLsizei p2, GLsizei* p3, GLchar* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glGetProgramInfoLog,      void, (GLuint p1, GLsizei p2, GLsizei* p3, GLchar* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glGetProgramiv,           void, (GLuint p1, GLenum p2, GLint* p3), (p1, p2, p3))\
    USE_FUNCTION (glGetUniformLocation,     GLint, (GLuint p1, const GLchar* p2), (p1, p2))\
    USE_FUNCTION (glGetAttribLocation,      GLint, (GLuint p1, const GLchar* p2), (p1, p2))\
    USE_FUNCTION (glVertexAttribPointer,    void, (GLuint p1, GLint p2, GLenum p3, GLboolean p4, GLsizei p5, const GLvoid* p6), (p1, p2, p3, p4, p5, p6))\
    USE_FUNCTION (glEnableVertexAttribArray,  void, (GLuint p1), (p1))\
    USE_FUNCTION (glDisableVertexAttribArray, void, (GLuint p1), (p1))\
    USE_FUNCTION (glUniform1f,              void, (GLint p1, GLfloat p2), (p1, p2))\
    USE_FUNCTION (glUniform1i,              void, (GLint p1, GLint p2), (p1, p2))\
    USE_FUNCTION (glUniform2f,              void, (GLint p1, GLfloat p2, GLfloat p3), (p1, p2, p3))\
    USE_FUNCTION (glUniform3f,              void, (GLint p1, GLfloat p2, GLfloat p3, GLfloat p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glUniform4f,              void, (GLint p1, GLfloat p2, GLfloat p3, GLfloat p4, GLfloat p5), (p1, p2, p3, p4, p5))\
    USE_FUNCTION (glUniform4i,              void, (GLint p1, GLint p2, GLint p3, GLint p4, GLint p5), (p1, p2, p3, p4, p5))\
    USE_FUNCTION (glUniform1fv,             void, (GLint p1, GLsizei p2, const GLfloat* p3), (p1, p2, p3))\
    USE_FUNCTION (glUniformMatrix2fv,       void, (GLint p1, GLsizei p2, GLboolean p3, const GLfloat* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glUniformMatrix3fv,       void, (GLint p1, GLsizei p2, GLboolean p3, const GLfloat* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glUniformMatrix4fv,       void, (GLint p1, GLsizei p2, GLboolean p3, const GLfloat* p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glBindAttribLocation,     void, (GLuint p1, GLuint p2, const GLchar* p3), (p1, p2, p3))\
    USE_FUNCTION (glDrawBuffers,            void, (GLsizei p1, const GLenum* p2), (p1, p2))

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_EXTENSION_FUNCTIONS(USE_FUNCTION) \
    USE_FUNCTION (glIsRenderbuffer,         GLboolean, (GLuint p1), (p1))\
    USE_FUNCTION (glBindRenderbuffer,       void, (GLenum p1, GLuint p2), (p1, p2))\
    USE_FUNCTION (glDeleteRenderbuffers,    void, (GLsizei p1, const GLuint* p2), (p1, p2))\
    USE_FUNCTION (glGenRenderbuffers,       void, (GLsizei p1, GLuint* p2), (p1, p2))\
    USE_FUNCTION (glRenderbufferStorage,    void, (GLenum p1, GLenum p2, GLsizei p3, GLsizei p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glGetRenderbufferParameteriv,  void, (GLenum p1, GLenum p2, GLint* p3), (p1, p2, p3))\
    USE_FUNCTION (glIsFramebuffer,          GLboolean, (GLuint p1), (p1))\
    USE_FUNCTION (glBindFramebuffer,        void, (GLenum p1, GLuint p2), (p1, p2))\
    USE_FUNCTION (glDeleteFramebuffers,     void, (GLsizei p1, const GLuint* p2), (p1, p2))\
    USE_FUNCTION (glGenFramebuffers,        void, (GLsizei p1, GLuint* p2), (p1, p2))\
    USE_FUNCTION (glCheckFramebufferStatus, GLenum, (GLenum p1), (p1))\
    USE_FUNCTION (glFramebufferTexture2D,   void, (GLenum p1, GLenum p2, GLenum p3, GLuint p4, GLint p5), (p1, p2, p3, p4, p5))\
    USE_FUNCTION (glFramebufferRenderbuffer,  void, (GLenum p1, GLenum p2, GLenum p3, GLuint p4), (p1, p2, p3, p4))\
    USE_FUNCTION (glGetFramebufferAttachmentParameteriv, void, (GLenum p1, GLenum p2, GLenum p3, GLint* p4), (p1, p2, p3, p4))

/** @internal This macro contains a list of GL extension functions that need to be dynamically loaded on Windows/Linux.
    @see OpenGLExtensionFunctions
*/
#define JUCE_GL_VERTEXBUFFER_FUNCTIONS(USE_FUNCTION) \
    USE_FUNCTION (glGenVertexArrays,        void, (GLsizei p1, GLuint* p2), (p1, p2))\
    USE_FUNCTION (glDeleteVertexArrays,     void, (GLsizei p1, const GLuint* p2), (p1, p2))\
    USE_FUNCTION (glBindVertexArray,        void, (GLuint p1), (p1))


/** This class contains a generated list of OpenGL extension functions, which are either dynamically loaded
    for a specific GL context, or simply call-through to the appropriate OS function where available.

    @tags{OpenGL}
*/
struct OpenGLExtensionFunctions
{
    void initialise();

   #if JUCE_WINDOWS && ! DOXYGEN
    typedef char GLchar;
    typedef pointer_sized_int GLsizeiptr;
    typedef pointer_sized_int GLintptr;
   #endif

    //==============================================================================
   #if JUCE_WINDOWS
    #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)      typedef returnType (__stdcall *type_ ## name) params; type_ ## name name;
    JUCE_GL_BASE_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    JUCE_GL_EXTENSION_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    #if JUCE_OPENGL3
     JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    #endif

    //==============================================================================
   #elif JUCE_LINUX
    #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)      typedef returnType (*type_ ## name) params; type_ ## name name;
    JUCE_GL_BASE_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    JUCE_GL_EXTENSION_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    #if JUCE_OPENGL3
     JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    #endif

    //==============================================================================
   #elif JUCE_OPENGL_ES
    #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)      static returnType name params noexcept;
    JUCE_GL_BASE_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    JUCE_GL_EXTENSION_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)

    //==============================================================================
   #else
    #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)      inline static returnType name params noexcept { return ::name callparams; }
    JUCE_GL_BASE_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)

    #ifndef GL3_PROTOTYPES
     #undef JUCE_DECLARE_GL_FUNCTION
     #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)     inline static returnType name params noexcept { return ::name ## EXT callparams; }
    #endif
     JUCE_GL_EXTENSION_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)

    #if JUCE_OPENGL3
     #ifndef GL3_PROTOTYPES
      #undef JUCE_DECLARE_GL_FUNCTION
      #define JUCE_DECLARE_GL_FUNCTION(name, returnType, params, callparams)    inline static returnType name params noexcept { return ::name ## APPLE callparams; }
     #endif
     JUCE_GL_VERTEXBUFFER_FUNCTIONS (JUCE_DECLARE_GL_FUNCTION)
    #endif
   #endif

    #undef JUCE_DECLARE_GL_FUNCTION
};

} // namespace juce
