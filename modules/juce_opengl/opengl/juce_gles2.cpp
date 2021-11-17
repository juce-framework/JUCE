/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

// This file was generated automatically using data from the opengl-registry
// https://github.com/KhronosGroup/OpenGL-Registry


// This file's corresponding header contains a reference to a function pointer
// for each command in the GL spec. The functions from earlier GL versions are
// (pretty much) guaranteed to be available in the platform GL library. For
// these functions, the references will be initialised to point directly at the
// library implementations. This behaviour is somewhat configurable:
// preprocessor defs of the form `JUCE_STATIC_LINK_<some gl version>` will
// ensure that the functions from a particular GL version are linked
// statically. Of course, this may fail to link if the platform doesn't
// implement the requested GL version. Any GL versions that are not explicitly
// requested for static linking, along with all known GL extensions, are loaded
// at runtime using gl::loadFunctions(). Again, these functions can be accessed
// via the references in the header.

// You should be aware that *any* of the functions declared in the header may
// be nullptr if the implementation does not supply that function. If you
// depend on specific GL features/extensions, it's probably a good idea to
// check each function pointer to ensure that the function was loaded
// successfully.


#define JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0 \
    X (void        , glActiveTexture, (GLenum texture)) \
    X (void        , glAttachShader, (GLuint program, GLuint shader)) \
    X (void        , glBindAttribLocation, (GLuint program, GLuint index, const GLchar *name)) \
    X (void        , glBindBuffer, (GLenum target, GLuint buffer)) \
    X (void        , glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
    X (void        , glBindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
    X (void        , glBindTexture, (GLenum target, GLuint texture)) \
    X (void        , glBlendColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glBlendEquation, (GLenum mode)) \
    X (void        , glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunc, (GLenum sfactor, GLenum dfactor)) \
    X (void        , glBlendFuncSeparate, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)) \
    X (void        , glBufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage)) \
    X (void        , glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (GLenum      , glCheckFramebufferStatus, (GLenum target)) \
    X (void        , glClear, (GLbitfield mask)) \
    X (void        , glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glClearDepthf, (GLfloat d)) \
    X (void        , glClearStencil, (GLint s)) \
    X (void        , glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) \
    X (void        , glCompileShader, (GLuint shader)) \
    X (void        , glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (void        , glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (GLuint      , glCreateProgram, ()) \
    X (GLuint      , glCreateShader, (GLenum type)) \
    X (void        , glCullFace, (GLenum mode)) \
    X (void        , glDeleteBuffers, (GLsizei n, const GLuint *buffers)) \
    X (void        , glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers)) \
    X (void        , glDeleteProgram, (GLuint program)) \
    X (void        , glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers)) \
    X (void        , glDeleteShader, (GLuint shader)) \
    X (void        , glDeleteTextures, (GLsizei n, const GLuint *textures)) \
    X (void        , glDepthFunc, (GLenum func)) \
    X (void        , glDepthMask, (GLboolean flag)) \
    X (void        , glDepthRangef, (GLfloat n, GLfloat f)) \
    X (void        , glDetachShader, (GLuint program, GLuint shader)) \
    X (void        , glDisable, (GLenum cap)) \
    X (void        , glDisableVertexAttribArray, (GLuint index)) \
    X (void        , glDrawArrays, (GLenum mode, GLint first, GLsizei count)) \
    X (void        , glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void *indices)) \
    X (void        , glEnable, (GLenum cap)) \
    X (void        , glEnableVertexAttribArray, (GLuint index)) \
    X (void        , glFinish, ()) \
    X (void        , glFlush, ()) \
    X (void        , glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (void        , glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glFrontFace, (GLenum mode)) \
    X (void        , glGenBuffers, (GLsizei n, GLuint *buffers)) \
    X (void        , glGenerateMipmap, (GLenum target)) \
    X (void        , glGenFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (void        , glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (void        , glGenTextures, (GLsizei n, GLuint *textures)) \
    X (void        , glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (void        , glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (void        , glGetAttachedShaders, (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)) \
    X (GLint       , glGetAttribLocation, (GLuint program, const GLchar *name)) \
    X (void        , glGetBooleanv, (GLenum pname, GLboolean *data)) \
    X (void        , glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (GLenum      , glGetError, ()) \
    X (void        , glGetFloatv, (GLenum pname, GLfloat *data)) \
    X (void        , glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (void        , glGetIntegerv, (GLenum pname, GLint *data)) \
    X (void        , glGetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
    X (void        , glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
    X (void        , glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)) \
    X (void        , glGetShaderSource, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)) \
    X (const GLubyte *, glGetString, (GLenum name)) \
    X (void        , glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetUniformfv, (GLuint program, GLint location, GLfloat *params)) \
    X (void        , glGetUniformiv, (GLuint program, GLint location, GLint *params)) \
    X (GLint       , glGetUniformLocation, (GLuint program, const GLchar *name)) \
    X (void        , glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetVertexAttribiv, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribPointerv, (GLuint index, GLenum pname, void **pointer)) \
    X (void        , glHint, (GLenum target, GLenum mode)) \
    X (GLboolean   , glIsBuffer, (GLuint buffer)) \
    X (GLboolean   , glIsEnabled, (GLenum cap)) \
    X (GLboolean   , glIsFramebuffer, (GLuint framebuffer)) \
    X (GLboolean   , glIsProgram, (GLuint program)) \
    X (GLboolean   , glIsRenderbuffer, (GLuint renderbuffer)) \
    X (GLboolean   , glIsShader, (GLuint shader)) \
    X (GLboolean   , glIsTexture, (GLuint texture)) \
    X (void        , glLineWidth, (GLfloat width)) \
    X (void        , glLinkProgram, (GLuint program)) \
    X (void        , glPixelStorei, (GLenum pname, GLint param)) \
    X (void        , glPolygonOffset, (GLfloat factor, GLfloat units)) \
    X (void        , glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)) \
    X (void        , glReleaseShaderCompiler, ()) \
    X (void        , glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glSampleCoverage, (GLfloat value, GLboolean invert)) \
    X (void        , glScissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glShaderBinary, (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length)) \
    X (void        , glShaderSource, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)) \
    X (void        , glStencilFunc, (GLenum func, GLint ref, GLuint mask)) \
    X (void        , glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask)) \
    X (void        , glStencilMask, (GLuint mask)) \
    X (void        , glStencilMaskSeparate, (GLenum face, GLuint mask)) \
    X (void        , glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass)) \
    X (void        , glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (void        , glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexParameterf, (GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glTexParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glTexParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glUniform1f, (GLint location, GLfloat v0)) \
    X (void        , glUniform1fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform1i, (GLint location, GLint v0)) \
    X (void        , glUniform1iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glUniform2fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform2i, (GLint location, GLint v0, GLint v1)) \
    X (void        , glUniform2iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glUniform3fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glUniform3iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glUniform4fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glUniform4iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUseProgram, (GLuint program)) \
    X (void        , glValidateProgram, (GLuint program)) \
    X (void        , glVertexAttrib1f, (GLuint index, GLfloat x)) \
    X (void        , glVertexAttrib1fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib2f, (GLuint index, GLfloat x, GLfloat y)) \
    X (void        , glVertexAttrib2fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib3f, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertexAttrib3fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib4f, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertexAttrib4fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)) \
    X (void        , glViewport, (GLint x, GLint y, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0 \
    X (void        , glReadBuffer, (GLenum src)) \
    X (void        , glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)) \
    X (void        , glTexImage3D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glCompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glGenQueries, (GLsizei n, GLuint *ids)) \
    X (void        , glDeleteQueries, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQuery, (GLuint id)) \
    X (void        , glBeginQuery, (GLenum target, GLuint id)) \
    X (void        , glEndQuery, (GLenum target)) \
    X (void        , glGetQueryiv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params)) \
    X (GLboolean   , glUnmapBuffer, (GLenum target)) \
    X (void        , glGetBufferPointerv, (GLenum target, GLenum pname, void **params)) \
    X (void        , glDrawBuffers, (GLsizei n, const GLenum *bufs)) \
    X (void        , glUniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (void        , glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (void *      , glMapBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (void        , glFlushMappedBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length)) \
    X (void        , glBindVertexArray, (GLuint array)) \
    X (void        , glDeleteVertexArrays, (GLsizei n, const GLuint *arrays)) \
    X (void        , glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArray, (GLuint array)) \
    X (void        , glGetIntegeri_v, (GLenum target, GLuint index, GLint *data)) \
    X (void        , glBeginTransformFeedback, (GLenum primitiveMode)) \
    X (void        , glEndTransformFeedback, ()) \
    X (void        , glBindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glBindBufferBase, (GLenum target, GLuint index, GLuint buffer)) \
    X (void        , glTransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (void        , glGetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (void        , glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glGetVertexAttribIiv, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribIuiv, (GLuint index, GLenum pname, GLuint *params)) \
    X (void        , glVertexAttribI4i, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glVertexAttribI4ui, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glVertexAttribI4iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI4uiv, (GLuint index, const GLuint *v)) \
    X (void        , glGetUniformuiv, (GLuint program, GLint location, GLuint *params)) \
    X (GLint       , glGetFragDataLocation, (GLuint program, const GLchar *name)) \
    X (void        , glUniform1ui, (GLint location, GLuint v0)) \
    X (void        , glUniform2ui, (GLint location, GLuint v0, GLuint v1)) \
    X (void        , glUniform3ui, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glUniform4ui, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glUniform1uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform2uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform3uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform4uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (void        , glClearBufferuiv, (GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (void        , glClearBufferfv, (GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (void        , glClearBufferfi, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (const GLubyte *, glGetStringi, (GLenum name, GLuint index)) \
    X (void        , glCopyBufferSubData, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (void        , glGetUniformIndices, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)) \
    X (void        , glGetActiveUniformsiv, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)) \
    X (GLuint      , glGetUniformBlockIndex, (GLuint program, const GLchar *uniformBlockName)) \
    X (void        , glGetActiveUniformBlockiv, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)) \
    X (void        , glGetActiveUniformBlockName, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)) \
    X (void        , glUniformBlockBinding, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding)) \
    X (void        , glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X (void        , glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)) \
    X (GLsync      , glFenceSync, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSync, (GLsync sync)) \
    X (void        , glDeleteSync, (GLsync sync)) \
    X (GLenum      , glClientWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glGetInteger64v, (GLenum pname, GLint64 *data)) \
    X (void        , glGetSynciv, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (void        , glGetInteger64i_v, (GLenum target, GLuint index, GLint64 *data)) \
    X (void        , glGetBufferParameteri64v, (GLenum target, GLenum pname, GLint64 *params)) \
    X (void        , glGenSamplers, (GLsizei count, GLuint *samplers)) \
    X (void        , glDeleteSamplers, (GLsizei count, const GLuint *samplers)) \
    X (GLboolean   , glIsSampler, (GLuint sampler)) \
    X (void        , glBindSampler, (GLuint unit, GLuint sampler)) \
    X (void        , glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
    X (void        , glSamplerParameteriv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
    X (void        , glSamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat *param)) \
    X (void        , glGetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat *params)) \
    X (void        , glVertexAttribDivisor, (GLuint index, GLuint divisor)) \
    X (void        , glBindTransformFeedback, (GLenum target, GLuint id)) \
    X (void        , glDeleteTransformFeedbacks, (GLsizei n, const GLuint *ids)) \
    X (void        , glGenTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedback, (GLuint id)) \
    X (void        , glPauseTransformFeedback, ()) \
    X (void        , glResumeTransformFeedback, ()) \
    X (void        , glGetProgramBinary, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)) \
    X (void        , glProgramBinary, (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)) \
    X (void        , glProgramParameteri, (GLuint program, GLenum pname, GLint value)) \
    X (void        , glInvalidateFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments)) \
    X (void        , glInvalidateSubFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glTexStorage2D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTexStorage3D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glGetInternalformativ, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1 \
    X (void        , glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)) \
    X (void        , glDispatchComputeIndirect, (GLintptr indirect)) \
    X (void        , glDrawArraysIndirect, (GLenum mode, const void *indirect)) \
    X (void        , glDrawElementsIndirect, (GLenum mode, GLenum type, const void *indirect)) \
    X (void        , glFramebufferParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glGetFramebufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint *params)) \
    X (GLuint      , glGetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (void        , glGetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (void        , glGetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params)) \
    X (GLint       , glGetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (void        , glUseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (void        , glActiveShaderProgram, (GLuint pipeline, GLuint program)) \
    X (GLuint      , glCreateShaderProgramv, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (void        , glBindProgramPipeline, (GLuint pipeline)) \
    X (void        , glDeleteProgramPipelines, (GLsizei n, const GLuint *pipelines)) \
    X (void        , glGenProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (GLboolean   , glIsProgramPipeline, (GLuint pipeline)) \
    X (void        , glGetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (void        , glProgramUniform1i, (GLuint program, GLint location, GLint v0)) \
    X (void        , glProgramUniform2i, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (void        , glProgramUniform3i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glProgramUniform4i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glProgramUniform1ui, (GLuint program, GLint location, GLuint v0)) \
    X (void        , glProgramUniform2ui, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (void        , glProgramUniform3ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glProgramUniform4ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glProgramUniform1f, (GLuint program, GLint location, GLfloat v0)) \
    X (void        , glProgramUniform2f, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glProgramUniform3f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glProgramUniform4f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glProgramUniform1iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform2iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform3iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform4iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform1uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform2uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform3uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform4uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform1fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform2fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform3fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform4fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glValidateProgramPipeline, (GLuint pipeline)) \
    X (void        , glGetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)) \
    X (void        , glGetBooleani_v, (GLenum target, GLuint index, GLboolean *data)) \
    X (void        , glMemoryBarrier, (GLbitfield barriers)) \
    X (void        , glMemoryBarrierByRegion, (GLbitfield barriers)) \
    X (void        , glTexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (void        , glGetMultisamplefv, (GLenum pname, GLuint index, GLfloat *val)) \
    X (void        , glSampleMaski, (GLuint maskNumber, GLbitfield mask)) \
    X (void        , glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (void        , glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (void        , glBindVertexBuffer, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (void        , glVertexAttribFormat, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (void        , glVertexAttribIFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexAttribBinding, (GLuint attribindex, GLuint bindingindex)) \
    X (void        , glVertexBindingDivisor, (GLuint bindingindex, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2 \
    X (void        , glBlendBarrier, ()) \
    X (void        , glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (void        , glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (void        , glDebugMessageInsert, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (void        , glDebugMessageCallback, (GLDEBUGPROC callback, const void *userParam)) \
    X (GLuint      , glGetDebugMessageLog, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (void        , glPushDebugGroup, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (void        , glPopDebugGroup, ()) \
    X (void        , glObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectLabel, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (void        , glObjectPtrLabel, (const void *ptr, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectPtrLabel, (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (void        , glGetPointerv, (GLenum pname, void **params)) \
    X (void        , glEnablei, (GLenum target, GLuint index)) \
    X (void        , glDisablei, (GLenum target, GLuint index)) \
    X (void        , glBlendEquationi, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparatei, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunci, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparatei, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (void        , glColorMaski, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnabledi, (GLenum target, GLuint index)) \
    X (void        , glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)) \
    X (void        , glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glPrimitiveBoundingBox, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW)) \
    X (GLenum      , glGetGraphicsResetStatus, ()) \
    X (void        , glReadnPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)) \
    X (void        , glGetnUniformfv, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (void        , glGetnUniformiv, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (void        , glGetnUniformuiv, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (void        , glMinSampleShading, (GLfloat value)) \
    X (void        , glPatchParameteri, (GLenum pname, GLint value)) \
    X (void        , glTexParameterIiv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexParameterIuiv, (GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTexParameterIiv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexParameterIuiv, (GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glSamplerParameterIiv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterIuiv, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (void        , glGetSamplerParameterIiv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterIuiv, (GLuint sampler, GLenum pname, GLuint *params)) \
    X (void        , glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glTexBufferRange, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))

#define JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    X (void        , glRenderbufferStorageMultisampleAdvancedAMD, (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glNamedRenderbufferStorageMultisampleAdvancedAMD, (GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_AMD_performance_monitor \
    X (void        , glGetPerfMonitorGroupsAMD, (GLint *numGroups, GLsizei groupsSize, GLuint *groups)) \
    X (void        , glGetPerfMonitorCountersAMD, (GLuint group, GLint *numCounters, GLint *maxActiveCounters, GLsizei counterSize, GLuint *counters)) \
    X (void        , glGetPerfMonitorGroupStringAMD, (GLuint group, GLsizei bufSize, GLsizei *length, GLchar *groupString)) \
    X (void        , glGetPerfMonitorCounterStringAMD, (GLuint group, GLuint counter, GLsizei bufSize, GLsizei *length, GLchar *counterString)) \
    X (void        , glGetPerfMonitorCounterInfoAMD, (GLuint group, GLuint counter, GLenum pname, void *data)) \
    X (void        , glGenPerfMonitorsAMD, (GLsizei n, GLuint *monitors)) \
    X (void        , glDeletePerfMonitorsAMD, (GLsizei n, GLuint *monitors)) \
    X (void        , glSelectPerfMonitorCountersAMD, (GLuint monitor, GLboolean enable, GLuint group, GLint numCounters, GLuint *counterList)) \
    X (void        , glBeginPerfMonitorAMD, (GLuint monitor)) \
    X (void        , glEndPerfMonitorAMD, (GLuint monitor)) \
    X (void        , glGetPerfMonitorCounterDataAMD, (GLuint monitor, GLenum pname, GLsizei dataSize, GLuint *data, GLint *bytesWritten))

#define JUCE_GL_FUNCTIONS_GL_ANGLE_framebuffer_blit \
    X (void        , glBlitFramebufferANGLE, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define JUCE_GL_FUNCTIONS_GL_ANGLE_framebuffer_multisample \
    X (void        , glRenderbufferStorageMultisampleANGLE, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_ANGLE_instanced_arrays \
    X (void        , glDrawArraysInstancedANGLE, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (void        , glDrawElementsInstancedANGLE, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount)) \
    X (void        , glVertexAttribDivisorANGLE, (GLuint index, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_ANGLE_translated_shader_source \
    X (void        , glGetTranslatedShaderSourceANGLE, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source))

#define JUCE_GL_FUNCTIONS_GL_APPLE_copy_texture_levels \
    X (void        , glCopyTextureLevelsAPPLE, (GLuint destinationTexture, GLuint sourceTexture, GLint sourceBaseLevel, GLsizei sourceLevelCount))

#define JUCE_GL_FUNCTIONS_GL_APPLE_framebuffer_multisample \
    X (void        , glRenderbufferStorageMultisampleAPPLE, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glResolveMultisampleFramebufferAPPLE, ())

#define JUCE_GL_FUNCTIONS_GL_APPLE_sync \
    X (GLsync      , glFenceSyncAPPLE, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSyncAPPLE, (GLsync sync)) \
    X (void        , glDeleteSyncAPPLE, (GLsync sync)) \
    X (GLenum      , glClientWaitSyncAPPLE, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glWaitSyncAPPLE, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glGetInteger64vAPPLE, (GLenum pname, GLint64 *params)) \
    X (void        , glGetSyncivAPPLE, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values))

#define JUCE_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    X (void        , glEGLImageTargetTexStorageEXT, (GLenum target, GLeglImageOES image, const GLint* attrib_list)) \
    X (void        , glEGLImageTargetTextureStorageEXT, (GLuint texture, GLeglImageOES image, const GLint* attrib_list))

#define JUCE_GL_FUNCTIONS_GL_EXT_base_instance \
    X (void        , glDrawArraysInstancedBaseInstanceEXT, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)) \
    X (void        , glDrawElementsInstancedBaseInstanceEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)) \
    X (void        , glDrawElementsInstancedBaseVertexBaseInstanceEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_func_extended \
    X (void        , glBindFragDataLocationIndexedEXT, (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)) \
    X (void        , glBindFragDataLocationEXT, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetProgramResourceLocationIndexEXT, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (GLint       , glGetFragDataIndexEXT, (GLuint program, const GLchar *name))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_minmax \
    X (void        , glBlendEquationEXT, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_EXT_buffer_storage \
    X (void        , glBufferStorageEXT, (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_EXT_clear_texture \
    X (void        , glClearTexImageEXT, (GLuint texture, GLint level, GLenum format, GLenum type, const void *data)) \
    X (void        , glClearTexSubImageEXT, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data))

#define JUCE_GL_FUNCTIONS_GL_EXT_clip_control \
    X (void        , glClipControlEXT, (GLenum origin, GLenum depth))

#define JUCE_GL_FUNCTIONS_GL_EXT_copy_image \
    X (void        , glCopyImageSubDataEXT, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth))

#define JUCE_GL_FUNCTIONS_GL_EXT_debug_label \
    X (void        , glLabelObjectEXT, (GLenum type, GLuint object, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectLabelEXT, (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label))

#define JUCE_GL_FUNCTIONS_GL_EXT_debug_marker \
    X (void        , glInsertEventMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (void        , glPushGroupMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (void        , glPopGroupMarkerEXT, ())

#define JUCE_GL_FUNCTIONS_GL_EXT_discard_framebuffer \
    X (void        , glDiscardFramebufferEXT, (GLenum target, GLsizei numAttachments, const GLenum *attachments))

#define JUCE_GL_FUNCTIONS_GL_EXT_disjoint_timer_query \
    X (void        , glGenQueriesEXT, (GLsizei n, GLuint *ids)) \
    X (void        , glDeleteQueriesEXT, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQueryEXT, (GLuint id)) \
    X (void        , glBeginQueryEXT, (GLenum target, GLuint id)) \
    X (void        , glEndQueryEXT, (GLenum target)) \
    X (void        , glQueryCounterEXT, (GLuint id, GLenum target)) \
    X (void        , glGetQueryivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectivEXT, (GLuint id, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectuivEXT, (GLuint id, GLenum pname, GLuint *params)) \
    X (void        , glGetQueryObjecti64vEXT, (GLuint id, GLenum pname, GLint64 *params)) \
    X (void        , glGetQueryObjectui64vEXT, (GLuint id, GLenum pname, GLuint64 *params)) \
    X (void        , glGetInteger64vEXT, (GLenum pname, GLint64 *data))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers \
    X (void        , glDrawBuffersEXT, (GLsizei n, const GLenum *bufs))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers_indexed \
    X (void        , glEnableiEXT, (GLenum target, GLuint index)) \
    X (void        , glDisableiEXT, (GLenum target, GLuint index)) \
    X (void        , glBlendEquationiEXT, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparateiEXT, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunciEXT, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparateiEXT, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (void        , glColorMaskiEXT, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnablediEXT, (GLenum target, GLuint index))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_elements_base_vertex \
    X (void        , glDrawElementsBaseVertexEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawRangeElementsBaseVertexEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawElementsInstancedBaseVertexEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)) \
    X (void        , glMultiDrawElementsBaseVertexEXT, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_instanced \
    X (void        , glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount)) \
    X (void        , glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_transform_feedback \
    X (void        , glDrawTransformFeedbackEXT, (GLenum mode, GLuint id)) \
    X (void        , glDrawTransformFeedbackInstancedEXT, (GLenum mode, GLuint id, GLsizei instancecount))

#define JUCE_GL_FUNCTIONS_GL_EXT_external_buffer \
    X (void        , glBufferStorageExternalEXT, (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags)) \
    X (void        , glNamedBufferStorageExternalEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_EXT_geometry_shader \
    X (void        , glFramebufferTextureEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level))

#define JUCE_GL_FUNCTIONS_GL_EXT_instanced_arrays \
    X (void        , glVertexAttribDivisorEXT, (GLuint index, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_EXT_map_buffer_range \
    X (void *      , glMapBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (void        , glFlushMappedBufferRangeEXT, (GLenum target, GLintptr offset, GLsizeiptr length))

#define JUCE_GL_FUNCTIONS_GL_EXT_memory_object \
    X (void        , glGetUnsignedBytevEXT, (GLenum pname, GLubyte *data)) \
    X (void        , glGetUnsignedBytei_vEXT, (GLenum target, GLuint index, GLubyte *data)) \
    X (void        , glDeleteMemoryObjectsEXT, (GLsizei n, const GLuint *memoryObjects)) \
    X (GLboolean   , glIsMemoryObjectEXT, (GLuint memoryObject)) \
    X (void        , glCreateMemoryObjectsEXT, (GLsizei n, GLuint *memoryObjects)) \
    X (void        , glMemoryObjectParameterivEXT, (GLuint memoryObject, GLenum pname, const GLint *params)) \
    X (void        , glGetMemoryObjectParameterivEXT, (GLuint memoryObject, GLenum pname, GLint *params)) \
    X (void        , glTexStorageMem2DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset)) \
    X (void        , glTexStorageMem2DMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (void        , glTexStorageMem3DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset)) \
    X (void        , glTexStorageMem3DMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (void        , glBufferStorageMemEXT, (GLenum target, GLsizeiptr size, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureStorageMem2DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureStorageMem2DMultisampleEXT, (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureStorageMem3DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureStorageMem3DMultisampleEXT, (GLuint texture, GLsizei samples, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations, GLuint memory, GLuint64 offset)) \
    X (void        , glNamedBufferStorageMemEXT, (GLuint buffer, GLsizeiptr size, GLuint memory, GLuint64 offset)) \
    X (void        , glTexStorageMem1DEXT, (GLenum target, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureStorageMem1DEXT, (GLuint texture, GLsizei levels, GLenum internalFormat, GLsizei width, GLuint memory, GLuint64 offset))

#define JUCE_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    X (void        , glImportMemoryFdEXT, (GLuint memory, GLuint64 size, GLenum handleType, GLint fd))

#define JUCE_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    X (void        , glImportMemoryWin32HandleEXT, (GLuint memory, GLuint64 size, GLenum handleType, void *handle)) \
    X (void        , glImportMemoryWin32NameEXT, (GLuint memory, GLuint64 size, GLenum handleType, const void *name))

#define JUCE_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    X (void        , glMultiDrawArraysEXT, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)) \
    X (void        , glMultiDrawElementsEXT, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_EXT_multi_draw_indirect \
    X (void        , glMultiDrawArraysIndirectEXT, (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)) \
    X (void        , glMultiDrawElementsIndirectEXT, (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride))

#define JUCE_GL_FUNCTIONS_GL_EXT_multisampled_render_to_texture \
    X (void        , glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glFramebufferTexture2DMultisampleEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))

#define JUCE_GL_FUNCTIONS_GL_EXT_multiview_draw_buffers \
    X (void        , glReadBufferIndexedEXT, (GLenum src, GLint index)) \
    X (void        , glDrawBuffersIndexedEXT, (GLint n, const GLenum *location, const GLint *indices)) \
    X (void        , glGetIntegeri_vEXT, (GLenum target, GLuint index, GLint *data))

#define JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    X (void        , glPolygonOffsetClampEXT, (GLfloat factor, GLfloat units, GLfloat clamp))

#define JUCE_GL_FUNCTIONS_GL_EXT_primitive_bounding_box \
    X (void        , glPrimitiveBoundingBoxEXT, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define JUCE_GL_FUNCTIONS_GL_EXT_raster_multisample \
    X (void        , glRasterSamplesEXT, (GLuint samples, GLboolean fixedsamplelocations))

#define JUCE_GL_FUNCTIONS_GL_EXT_robustness \
    X (GLenum      , glGetGraphicsResetStatusEXT, ()) \
    X (void        , glReadnPixelsEXT, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)) \
    X (void        , glGetnUniformfvEXT, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (void        , glGetnUniformivEXT, (GLuint program, GLint location, GLsizei bufSize, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_semaphore \
    X (void        , glGenSemaphoresEXT, (GLsizei n, GLuint *semaphores)) \
    X (void        , glDeleteSemaphoresEXT, (GLsizei n, const GLuint *semaphores)) \
    X (GLboolean   , glIsSemaphoreEXT, (GLuint semaphore)) \
    X (void        , glSemaphoreParameterui64vEXT, (GLuint semaphore, GLenum pname, const GLuint64 *params)) \
    X (void        , glGetSemaphoreParameterui64vEXT, (GLuint semaphore, GLenum pname, GLuint64 *params)) \
    X (void        , glWaitSemaphoreEXT, (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *srcLayouts)) \
    X (void        , glSignalSemaphoreEXT, (GLuint semaphore, GLuint numBufferBarriers, const GLuint *buffers, GLuint numTextureBarriers, const GLuint *textures, const GLenum *dstLayouts))

#define JUCE_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    X (void        , glImportSemaphoreFdEXT, (GLuint semaphore, GLenum handleType, GLint fd))

#define JUCE_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    X (void        , glImportSemaphoreWin32HandleEXT, (GLuint semaphore, GLenum handleType, void *handle)) \
    X (void        , glImportSemaphoreWin32NameEXT, (GLuint semaphore, GLenum handleType, const void *name))

#define JUCE_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    X (void        , glUseShaderProgramEXT, (GLenum type, GLuint program)) \
    X (void        , glActiveProgramEXT, (GLuint program)) \
    X (GLuint      , glCreateShaderProgramEXT, (GLenum type, const GLchar *string)) \
    X (void        , glActiveShaderProgramEXT, (GLuint pipeline, GLuint program)) \
    X (void        , glBindProgramPipelineEXT, (GLuint pipeline)) \
    X (GLuint      , glCreateShaderProgramvEXT, (GLenum type, GLsizei count, const GLchar **strings)) \
    X (void        , glDeleteProgramPipelinesEXT, (GLsizei n, const GLuint *pipelines)) \
    X (void        , glGenProgramPipelinesEXT, (GLsizei n, GLuint *pipelines)) \
    X (void        , glGetProgramPipelineInfoLogEXT, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetProgramPipelineivEXT, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsProgramPipelineEXT, (GLuint pipeline)) \
    X (void        , glProgramParameteriEXT, (GLuint program, GLenum pname, GLint value)) \
    X (void        , glProgramUniform1fEXT, (GLuint program, GLint location, GLfloat v0)) \
    X (void        , glProgramUniform1fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform1iEXT, (GLuint program, GLint location, GLint v0)) \
    X (void        , glProgramUniform1ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform2fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glProgramUniform2fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform2iEXT, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (void        , glProgramUniform2ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform3fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glProgramUniform3fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform3iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glProgramUniform3ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform4fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glProgramUniform4fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform4iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glProgramUniform4ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniformMatrix2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUseProgramStagesEXT, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (void        , glValidateProgramPipelineEXT, (GLuint pipeline)) \
    X (void        , glProgramUniform1uiEXT, (GLuint program, GLint location, GLuint v0)) \
    X (void        , glProgramUniform2uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (void        , glProgramUniform3uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glProgramUniform4uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glProgramUniform1uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform2uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform3uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform4uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniformMatrix2x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define JUCE_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    X (void        , glFramebufferFetchBarrierEXT, ())

#define JUCE_GL_FUNCTIONS_GL_EXT_shader_pixel_local_storage2 \
    X (void        , glFramebufferPixelLocalStorageSizeEXT, (GLuint target, GLsizei size)) \
    X (GLsizei     , glGetFramebufferPixelLocalStorageSizeEXT, (GLuint target)) \
    X (void        , glClearPixelLocalStorageuiEXT, (GLsizei offset, GLsizei n, const GLuint *values))

#define JUCE_GL_FUNCTIONS_GL_EXT_sparse_texture \
    X (void        , glTexPageCommitmentEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit))

#define JUCE_GL_FUNCTIONS_GL_EXT_tessellation_shader \
    X (void        , glPatchParameteriEXT, (GLenum pname, GLint value))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_border_clamp \
    X (void        , glTexParameterIivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexParameterIuivEXT, (GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTexParameterIivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexParameterIuivEXT, (GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glSamplerParameterIivEXT, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterIuivEXT, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (void        , glGetSamplerParameterIivEXT, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterIuivEXT, (GLuint sampler, GLenum pname, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_buffer \
    X (void        , glTexBufferEXT, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glTexBufferRangeEXT, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_storage \
    X (void        , glTexStorage1DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTexStorage2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTexStorage3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glTextureStorage1DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTextureStorage2DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTextureStorage3DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_view \
    X (void        , glTextureViewEXT, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers))

#define JUCE_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    X (void        , glCreateSemaphoresNV, (GLsizei n, GLuint *semaphores)) \
    X (void        , glSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, const GLint *params)) \
    X (void        , glGetSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    X (GLboolean   , glAcquireKeyedMutexWin32EXT, (GLuint memory, GLuint64 key, GLuint timeout)) \
    X (GLboolean   , glReleaseKeyedMutexWin32EXT, (GLuint memory, GLuint64 key))

#define JUCE_GL_FUNCTIONS_GL_EXT_window_rectangles \
    X (void        , glWindowRectanglesEXT, (GLenum mode, GLsizei count, const GLint *box))

#define JUCE_GL_FUNCTIONS_GL_IMG_bindless_texture \
    X (GLuint64    , glGetTextureHandleIMG, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleIMG, (GLuint texture, GLuint sampler)) \
    X (void        , glUniformHandleui64IMG, (GLint location, GLuint64 value)) \
    X (void        , glUniformHandleui64vIMG, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniformHandleui64IMG, (GLuint program, GLint location, GLuint64 value)) \
    X (void        , glProgramUniformHandleui64vIMG, (GLuint program, GLint location, GLsizei count, const GLuint64 *values))

#define JUCE_GL_FUNCTIONS_GL_IMG_framebuffer_downsample \
    X (void        , glFramebufferTexture2DDownsampleIMG, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint xscale, GLint yscale)) \
    X (void        , glFramebufferTextureLayerDownsampleIMG, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer, GLint xscale, GLint yscale))

#define JUCE_GL_FUNCTIONS_GL_IMG_multisampled_render_to_texture \
    X (void        , glRenderbufferStorageMultisampleIMG, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glFramebufferTexture2DMultisampleIMG, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples))

#define JUCE_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    X (void        , glApplyFramebufferAttachmentCMAAINTEL, ())

#define JUCE_GL_FUNCTIONS_GL_INTEL_performance_query \
    X (void        , glBeginPerfQueryINTEL, (GLuint queryHandle)) \
    X (void        , glCreatePerfQueryINTEL, (GLuint queryId, GLuint *queryHandle)) \
    X (void        , glDeletePerfQueryINTEL, (GLuint queryHandle)) \
    X (void        , glEndPerfQueryINTEL, (GLuint queryHandle)) \
    X (void        , glGetFirstPerfQueryIdINTEL, (GLuint *queryId)) \
    X (void        , glGetNextPerfQueryIdINTEL, (GLuint queryId, GLuint *nextQueryId)) \
    X (void        , glGetPerfCounterInfoINTEL, (GLuint queryId, GLuint counterId, GLuint counterNameLength, GLchar *counterName, GLuint counterDescLength, GLchar *counterDesc, GLuint *counterOffset, GLuint *counterDataSize, GLuint *counterTypeEnum, GLuint *counterDataTypeEnum, GLuint64 *rawCounterMaxValue)) \
    X (void        , glGetPerfQueryDataINTEL, (GLuint queryHandle, GLuint flags, GLsizei dataSize, void *data, GLuint *bytesWritten)) \
    X (void        , glGetPerfQueryIdByNameINTEL, (GLchar *queryName, GLuint *queryId)) \
    X (void        , glGetPerfQueryInfoINTEL, (GLuint queryId, GLuint queryNameLength, GLchar *queryName, GLuint *dataSize, GLuint *noCounters, GLuint *noInstances, GLuint *capsMask))

#define JUCE_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    X (void        , glBlendBarrierKHR, ())

#define JUCE_GL_FUNCTIONS_GL_KHR_debug \
    X (void        , glDebugMessageControlKHR, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (void        , glDebugMessageInsertKHR, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (void        , glDebugMessageCallbackKHR, (GLDEBUGPROCKHR callback, const void *userParam)) \
    X (GLuint      , glGetDebugMessageLogKHR, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (void        , glPushDebugGroupKHR, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (void        , glPopDebugGroupKHR, ()) \
    X (void        , glObjectLabelKHR, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectLabelKHR, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (void        , glObjectPtrLabelKHR, (const void *ptr, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectPtrLabelKHR, (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (void        , glGetPointervKHR, (GLenum pname, void **params))

#define JUCE_GL_FUNCTIONS_GL_KHR_robustness \
    X (GLenum      , glGetGraphicsResetStatusKHR, ()) \
    X (void        , glReadnPixelsKHR, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)) \
    X (void        , glGetnUniformfvKHR, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (void        , glGetnUniformivKHR, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (void        , glGetnUniformuivKHR, (GLuint program, GLint location, GLsizei bufSize, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    X (void        , glMaxShaderCompilerThreadsKHR, (GLuint count))

#define JUCE_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    X (void        , glFramebufferParameteriMESA, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glGetFramebufferParameterivMESA, (GLenum target, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_bindless_texture \
    X (GLuint64    , glGetTextureHandleNV, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleNV, (GLuint texture, GLuint sampler)) \
    X (void        , glMakeTextureHandleResidentNV, (GLuint64 handle)) \
    X (void        , glMakeTextureHandleNonResidentNV, (GLuint64 handle)) \
    X (GLuint64    , glGetImageHandleNV, (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)) \
    X (void        , glMakeImageHandleResidentNV, (GLuint64 handle, GLenum access)) \
    X (void        , glMakeImageHandleNonResidentNV, (GLuint64 handle)) \
    X (void        , glUniformHandleui64NV, (GLint location, GLuint64 value)) \
    X (void        , glUniformHandleui64vNV, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniformHandleui64NV, (GLuint program, GLint location, GLuint64 value)) \
    X (void        , glProgramUniformHandleui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64 *values)) \
    X (GLboolean   , glIsTextureHandleResidentNV, (GLuint64 handle)) \
    X (GLboolean   , glIsImageHandleResidentNV, (GLuint64 handle))

#define JUCE_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    X (void        , glBlendParameteriNV, (GLenum pname, GLint value)) \
    X (void        , glBlendBarrierNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    X (void        , glViewportPositionWScaleNV, (GLuint index, GLfloat xcoeff, GLfloat ycoeff))

#define JUCE_GL_FUNCTIONS_GL_NV_conditional_render \
    X (void        , glBeginConditionalRenderNV, (GLuint id, GLenum mode)) \
    X (void        , glEndConditionalRenderNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_conservative_raster \
    X (void        , glSubpixelPrecisionBiasNV, (GLuint xbits, GLuint ybits))

#define JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    X (void        , glConservativeRasterParameteriNV, (GLenum pname, GLint param))

#define JUCE_GL_FUNCTIONS_GL_NV_copy_buffer \
    X (void        , glCopyBufferSubDataNV, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size))

#define JUCE_GL_FUNCTIONS_GL_NV_coverage_sample \
    X (void        , glCoverageMaskNV, (GLboolean mask)) \
    X (void        , glCoverageOperationNV, (GLenum operation))

#define JUCE_GL_FUNCTIONS_GL_NV_draw_buffers \
    X (void        , glDrawBuffersNV, (GLsizei n, const GLenum *bufs))

#define JUCE_GL_FUNCTIONS_GL_NV_draw_instanced \
    X (void        , glDrawArraysInstancedNV, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (void        , glDrawElementsInstancedNV, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    X (void        , glDrawVkImageNV, (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1)) \
    X (GLVULKANPROCNV, glGetVkProcAddrNV, (const GLchar *name)) \
    X (void        , glWaitVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (void        , glSignalVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (void        , glSignalVkFenceNV, (GLuint64 vkFence))

#define JUCE_GL_FUNCTIONS_GL_NV_fence \
    X (void        , glDeleteFencesNV, (GLsizei n, const GLuint *fences)) \
    X (void        , glGenFencesNV, (GLsizei n, GLuint *fences)) \
    X (GLboolean   , glIsFenceNV, (GLuint fence)) \
    X (GLboolean   , glTestFenceNV, (GLuint fence)) \
    X (void        , glGetFenceivNV, (GLuint fence, GLenum pname, GLint *params)) \
    X (void        , glFinishFenceNV, (GLuint fence)) \
    X (void        , glSetFenceNV, (GLuint fence, GLenum condition))

#define JUCE_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    X (void        , glFragmentCoverageColorNV, (GLuint color))

#define JUCE_GL_FUNCTIONS_GL_NV_framebuffer_blit \
    X (void        , glBlitFramebufferNV, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define JUCE_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    X (void        , glCoverageModulationTableNV, (GLsizei n, const GLfloat *v)) \
    X (void        , glGetCoverageModulationTableNV, (GLsizei bufSize, GLfloat *v)) \
    X (void        , glCoverageModulationNV, (GLenum components))

#define JUCE_GL_FUNCTIONS_GL_NV_framebuffer_multisample \
    X (void        , glRenderbufferStorageMultisampleNV, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_NV_gpu_shader5 \
    X (void        , glUniform1i64NV, (GLint location, GLint64EXT x)) \
    X (void        , glUniform2i64NV, (GLint location, GLint64EXT x, GLint64EXT y)) \
    X (void        , glUniform3i64NV, (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (void        , glUniform4i64NV, (GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (void        , glUniform1i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glUniform2i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glUniform3i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glUniform4i64vNV, (GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glUniform1ui64NV, (GLint location, GLuint64EXT x)) \
    X (void        , glUniform2ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y)) \
    X (void        , glUniform3ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (void        , glUniform4ui64NV, (GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (void        , glUniform1ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glUniform2ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glUniform3ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glUniform4ui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glGetUniformi64vNV, (GLuint program, GLint location, GLint64EXT *params)) \
    X (void        , glProgramUniform1i64NV, (GLuint program, GLint location, GLint64EXT x)) \
    X (void        , glProgramUniform2i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y)) \
    X (void        , glProgramUniform3i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (void        , glProgramUniform4i64NV, (GLuint program, GLint location, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (void        , glProgramUniform1i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glProgramUniform2i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glProgramUniform3i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glProgramUniform4i64vNV, (GLuint program, GLint location, GLsizei count, const GLint64EXT *value)) \
    X (void        , glProgramUniform1ui64NV, (GLuint program, GLint location, GLuint64EXT x)) \
    X (void        , glProgramUniform2ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y)) \
    X (void        , glProgramUniform3ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (void        , glProgramUniform4ui64NV, (GLuint program, GLint location, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (void        , glProgramUniform1ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glProgramUniform2ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glProgramUniform3ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glProgramUniform4ui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value))

#define JUCE_GL_FUNCTIONS_GL_NV_instanced_arrays \
    X (void        , glVertexAttribDivisorNV, (GLuint index, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    X (void        , glGetInternalformatSampleivNV, (GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_memory_attachment \
    X (void        , glGetMemoryObjectDetachedResourcesuivNV, (GLuint memory, GLenum pname, GLint first, GLsizei count, GLuint *params)) \
    X (void        , glResetMemoryObjectParameterNV, (GLuint memory, GLenum pname)) \
    X (void        , glTexAttachMemoryNV, (GLenum target, GLuint memory, GLuint64 offset)) \
    X (void        , glBufferAttachMemoryNV, (GLenum target, GLuint memory, GLuint64 offset)) \
    X (void        , glTextureAttachMemoryNV, (GLuint texture, GLuint memory, GLuint64 offset)) \
    X (void        , glNamedBufferAttachMemoryNV, (GLuint buffer, GLuint memory, GLuint64 offset))

#define JUCE_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    X (void        , glBufferPageCommitmentMemNV, (GLenum target, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)) \
    X (void        , glTexPageCommitmentMemNV, (GLenum target, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit)) \
    X (void        , glNamedBufferPageCommitmentMemNV, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLuint memory, GLuint64 memOffset, GLboolean commit)) \
    X (void        , glTexturePageCommitmentMemNV, (GLuint texture, GLint layer, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLuint memory, GLuint64 offset, GLboolean commit))

#define JUCE_GL_FUNCTIONS_GL_NV_mesh_shader \
    X (void        , glDrawMeshTasksNV, (GLuint first, GLuint count)) \
    X (void        , glDrawMeshTasksIndirectNV, (GLintptr indirect)) \
    X (void        , glMultiDrawMeshTasksIndirectNV, (GLintptr indirect, GLsizei drawcount, GLsizei stride)) \
    X (void        , glMultiDrawMeshTasksIndirectCountNV, (GLintptr indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))

#define JUCE_GL_FUNCTIONS_GL_NV_non_square_matrices \
    X (void        , glUniformMatrix2x3fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x2fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix2x4fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x2fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x4fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x3fvNV, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define JUCE_GL_FUNCTIONS_GL_NV_path_rendering \
    X (GLuint      , glGenPathsNV, (GLsizei range)) \
    X (void        , glDeletePathsNV, (GLuint path, GLsizei range)) \
    X (GLboolean   , glIsPathNV, (GLuint path)) \
    X (void        , glPathCommandsNV, (GLuint path, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords)) \
    X (void        , glPathCoordsNV, (GLuint path, GLsizei numCoords, GLenum coordType, const void *coords)) \
    X (void        , glPathSubCommandsNV, (GLuint path, GLsizei commandStart, GLsizei commandsToDelete, GLsizei numCommands, const GLubyte *commands, GLsizei numCoords, GLenum coordType, const void *coords)) \
    X (void        , glPathSubCoordsNV, (GLuint path, GLsizei coordStart, GLsizei numCoords, GLenum coordType, const void *coords)) \
    X (void        , glPathStringNV, (GLuint path, GLenum format, GLsizei length, const void *pathString)) \
    X (void        , glPathGlyphsNV, (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLsizei numGlyphs, GLenum type, const void *charcodes, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (void        , glPathGlyphRangeNV, (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyph, GLsizei numGlyphs, GLenum handleMissingGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (void        , glWeightPathsNV, (GLuint resultPath, GLsizei numPaths, const GLuint *paths, const GLfloat *weights)) \
    X (void        , glCopyPathNV, (GLuint resultPath, GLuint srcPath)) \
    X (void        , glInterpolatePathsNV, (GLuint resultPath, GLuint pathA, GLuint pathB, GLfloat weight)) \
    X (void        , glTransformPathNV, (GLuint resultPath, GLuint srcPath, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glPathParameterivNV, (GLuint path, GLenum pname, const GLint *value)) \
    X (void        , glPathParameteriNV, (GLuint path, GLenum pname, GLint value)) \
    X (void        , glPathParameterfvNV, (GLuint path, GLenum pname, const GLfloat *value)) \
    X (void        , glPathParameterfNV, (GLuint path, GLenum pname, GLfloat value)) \
    X (void        , glPathDashArrayNV, (GLuint path, GLsizei dashCount, const GLfloat *dashArray)) \
    X (void        , glPathStencilFuncNV, (GLenum func, GLint ref, GLuint mask)) \
    X (void        , glPathStencilDepthOffsetNV, (GLfloat factor, GLfloat units)) \
    X (void        , glStencilFillPathNV, (GLuint path, GLenum fillMode, GLuint mask)) \
    X (void        , glStencilStrokePathNV, (GLuint path, GLint reference, GLuint mask)) \
    X (void        , glStencilFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glStencilStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glPathCoverDepthFuncNV, (GLenum func)) \
    X (void        , glCoverFillPathNV, (GLuint path, GLenum coverMode)) \
    X (void        , glCoverStrokePathNV, (GLuint path, GLenum coverMode)) \
    X (void        , glCoverFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glCoverStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glGetPathParameterivNV, (GLuint path, GLenum pname, GLint *value)) \
    X (void        , glGetPathParameterfvNV, (GLuint path, GLenum pname, GLfloat *value)) \
    X (void        , glGetPathCommandsNV, (GLuint path, GLubyte *commands)) \
    X (void        , glGetPathCoordsNV, (GLuint path, GLfloat *coords)) \
    X (void        , glGetPathDashArrayNV, (GLuint path, GLfloat *dashArray)) \
    X (void        , glGetPathMetricsNV, (GLbitfield metricQueryMask, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLsizei stride, GLfloat *metrics)) \
    X (void        , glGetPathMetricRangeNV, (GLbitfield metricQueryMask, GLuint firstPathName, GLsizei numPaths, GLsizei stride, GLfloat *metrics)) \
    X (void        , glGetPathSpacingNV, (GLenum pathListMode, GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLfloat advanceScale, GLfloat kerningScale, GLenum transformType, GLfloat *returnedSpacing)) \
    X (GLboolean   , glIsPointInFillPathNV, (GLuint path, GLuint mask, GLfloat x, GLfloat y)) \
    X (GLboolean   , glIsPointInStrokePathNV, (GLuint path, GLfloat x, GLfloat y)) \
    X (GLfloat     , glGetPathLengthNV, (GLuint path, GLsizei startSegment, GLsizei numSegments)) \
    X (GLboolean   , glPointAlongPathNV, (GLuint path, GLsizei startSegment, GLsizei numSegments, GLfloat distance, GLfloat *x, GLfloat *y, GLfloat *tangentX, GLfloat *tangentY)) \
    X (void        , glMatrixLoad3x2fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glMatrixLoad3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glMatrixLoadTranspose3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glMatrixMult3x2fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glMatrixMult3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glMatrixMultTranspose3x3fNV, (GLenum matrixMode, const GLfloat *m)) \
    X (void        , glStencilThenCoverFillPathNV, (GLuint path, GLenum fillMode, GLuint mask, GLenum coverMode)) \
    X (void        , glStencilThenCoverStrokePathNV, (GLuint path, GLint reference, GLuint mask, GLenum coverMode)) \
    X (void        , glStencilThenCoverFillPathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLenum fillMode, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (void        , glStencilThenCoverStrokePathInstancedNV, (GLsizei numPaths, GLenum pathNameType, const void *paths, GLuint pathBase, GLint reference, GLuint mask, GLenum coverMode, GLenum transformType, const GLfloat *transformValues)) \
    X (GLenum      , glPathGlyphIndexRangeNV, (GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint pathParameterTemplate, GLfloat emScale, GLuint *baseAndCount)) \
    X (GLenum      , glPathGlyphIndexArrayNV, (GLuint firstPathName, GLenum fontTarget, const void *fontName, GLbitfield fontStyle, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (GLenum      , glPathMemoryGlyphIndexArrayNV, (GLuint firstPathName, GLenum fontTarget, GLsizeiptr fontSize, const void *fontData, GLsizei faceIndex, GLuint firstGlyphIndex, GLsizei numGlyphs, GLuint pathParameterTemplate, GLfloat emScale)) \
    X (void        , glProgramPathFragmentInputGenNV, (GLuint program, GLint location, GLenum genMode, GLint components, const GLfloat *coeffs)) \
    X (void        , glGetProgramResourcefvNV, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLfloat *params)) \
    X (void        , glPathColorGenNV, (GLenum color, GLenum genMode, GLenum colorFormat, const GLfloat *coeffs)) \
    X (void        , glPathTexGenNV, (GLenum texCoordSet, GLenum genMode, GLint components, const GLfloat *coeffs)) \
    X (void        , glPathFogGenNV, (GLenum genMode)) \
    X (void        , glGetPathColorGenivNV, (GLenum color, GLenum pname, GLint *value)) \
    X (void        , glGetPathColorGenfvNV, (GLenum color, GLenum pname, GLfloat *value)) \
    X (void        , glGetPathTexGenivNV, (GLenum texCoordSet, GLenum pname, GLint *value)) \
    X (void        , glGetPathTexGenfvNV, (GLenum texCoordSet, GLenum pname, GLfloat *value)) \
    X (void        , glMatrixFrustumEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glMatrixLoadIdentityEXT, (GLenum mode)) \
    X (void        , glMatrixLoadTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixLoadTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixLoadfEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixLoaddEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixMultTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixMultTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixMultfEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixMultdEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixOrthoEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glMatrixPopEXT, (GLenum mode)) \
    X (void        , glMatrixPushEXT, (GLenum mode)) \
    X (void        , glMatrixRotatefEXT, (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixRotatedEXT, (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glMatrixScalefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixScaledEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glMatrixTranslatefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixTranslatedEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z))

#define JUCE_GL_FUNCTIONS_GL_NV_polygon_mode \
    X (void        , glPolygonModeNV, (GLenum face, GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_NV_read_buffer \
    X (void        , glReadBufferNV, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_NV_sample_locations \
    X (void        , glFramebufferSampleLocationsfvNV, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glNamedFramebufferSampleLocationsfvNV, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glResolveDepthValuesNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    X (void        , glScissorExclusiveNV, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glScissorExclusiveArrayvNV, (GLuint first, GLsizei count, const GLint *v))

#define JUCE_GL_FUNCTIONS_GL_NV_shading_rate_image \
    X (void        , glBindShadingRateImageNV, (GLuint texture)) \
    X (void        , glGetShadingRateImagePaletteNV, (GLuint viewport, GLuint entry, GLenum *rate)) \
    X (void        , glGetShadingRateSampleLocationivNV, (GLenum rate, GLuint samples, GLuint index, GLint *location)) \
    X (void        , glShadingRateImageBarrierNV, (GLboolean synchronize)) \
    X (void        , glShadingRateImagePaletteNV, (GLuint viewport, GLuint first, GLsizei count, const GLenum *rates)) \
    X (void        , glShadingRateSampleOrderNV, (GLenum order)) \
    X (void        , glShadingRateSampleOrderCustomNV, (GLenum rate, GLuint samples, const GLint *locations))

#define JUCE_GL_FUNCTIONS_GL_NV_viewport_array \
    X (void        , glViewportArrayvNV, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glViewportIndexedfNV, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (void        , glViewportIndexedfvNV, (GLuint index, const GLfloat *v)) \
    X (void        , glScissorArrayvNV, (GLuint first, GLsizei count, const GLint *v)) \
    X (void        , glScissorIndexedNV, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (void        , glScissorIndexedvNV, (GLuint index, const GLint *v)) \
    X (void        , glDepthRangeArrayfvNV, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glDepthRangeIndexedfNV, (GLuint index, GLfloat n, GLfloat f)) \
    X (void        , glGetFloati_vNV, (GLenum target, GLuint index, GLfloat *data)) \
    X (void        , glEnableiNV, (GLenum target, GLuint index)) \
    X (void        , glDisableiNV, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnablediNV, (GLenum target, GLuint index))

#define JUCE_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    X (void        , glViewportSwizzleNV, (GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew))

#define JUCE_GL_FUNCTIONS_GL_OES_EGL_image \
    X (void        , glEGLImageTargetTexture2DOES, (GLenum target, GLeglImageOES image)) \
    X (void        , glEGLImageTargetRenderbufferStorageOES, (GLenum target, GLeglImageOES image))

#define JUCE_GL_FUNCTIONS_GL_OES_copy_image \
    X (void        , glCopyImageSubDataOES, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth))

#define JUCE_GL_FUNCTIONS_GL_OES_draw_buffers_indexed \
    X (void        , glEnableiOES, (GLenum target, GLuint index)) \
    X (void        , glDisableiOES, (GLenum target, GLuint index)) \
    X (void        , glBlendEquationiOES, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparateiOES, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunciOES, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparateiOES, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (void        , glColorMaskiOES, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (GLboolean   , glIsEnablediOES, (GLenum target, GLuint index))

#define JUCE_GL_FUNCTIONS_GL_OES_draw_elements_base_vertex \
    X (void        , glDrawElementsBaseVertexOES, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawRangeElementsBaseVertexOES, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawElementsInstancedBaseVertexOES, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex))

#define JUCE_GL_FUNCTIONS_GL_OES_geometry_shader \
    X (void        , glFramebufferTextureOES, (GLenum target, GLenum attachment, GLuint texture, GLint level))

#define JUCE_GL_FUNCTIONS_GL_OES_get_program_binary \
    X (void        , glGetProgramBinaryOES, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)) \
    X (void        , glProgramBinaryOES, (GLuint program, GLenum binaryFormat, const void *binary, GLint length))

#define JUCE_GL_FUNCTIONS_GL_OES_mapbuffer \
    X (void *      , glMapBufferOES, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBufferOES, (GLenum target)) \
    X (void        , glGetBufferPointervOES, (GLenum target, GLenum pname, void **params))

#define JUCE_GL_FUNCTIONS_GL_OES_primitive_bounding_box \
    X (void        , glPrimitiveBoundingBoxOES, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define JUCE_GL_FUNCTIONS_GL_OES_sample_shading \
    X (void        , glMinSampleShadingOES, (GLfloat value))

#define JUCE_GL_FUNCTIONS_GL_OES_tessellation_shader \
    X (void        , glPatchParameteriOES, (GLenum pname, GLint value))

#define JUCE_GL_FUNCTIONS_GL_OES_texture_3D \
    X (void        , glTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glCompressedTexImage3DOES, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage3DOES, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glFramebufferTexture3DOES, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))

#define JUCE_GL_FUNCTIONS_GL_OES_texture_border_clamp \
    X (void        , glTexParameterIivOES, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexParameterIuivOES, (GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTexParameterIivOES, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexParameterIuivOES, (GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glSamplerParameterIivOES, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterIuivOES, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (void        , glGetSamplerParameterIivOES, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterIuivOES, (GLuint sampler, GLenum pname, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_OES_texture_buffer \
    X (void        , glTexBufferOES, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glTexBufferRangeOES, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size))

#define JUCE_GL_FUNCTIONS_GL_OES_texture_storage_multisample_2d_array \
    X (void        , glTexStorage3DMultisampleOES, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations))

#define JUCE_GL_FUNCTIONS_GL_OES_texture_view \
    X (void        , glTextureViewOES, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers))

#define JUCE_GL_FUNCTIONS_GL_OES_vertex_array_object \
    X (void        , glBindVertexArrayOES, (GLuint array)) \
    X (void        , glDeleteVertexArraysOES, (GLsizei n, const GLuint *arrays)) \
    X (void        , glGenVertexArraysOES, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArrayOES, (GLuint array))

#define JUCE_GL_FUNCTIONS_GL_OES_viewport_array \
    X (void        , glViewportArrayvOES, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glViewportIndexedfOES, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (void        , glViewportIndexedfvOES, (GLuint index, const GLfloat *v)) \
    X (void        , glScissorArrayvOES, (GLuint first, GLsizei count, const GLint *v)) \
    X (void        , glScissorIndexedOES, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (void        , glScissorIndexedvOES, (GLuint index, const GLint *v)) \
    X (void        , glDepthRangeArrayfvOES, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glDepthRangeIndexedfOES, (GLuint index, GLfloat n, GLfloat f)) \
    X (void        , glGetFloati_vOES, (GLenum target, GLuint index, GLfloat *data))

#define JUCE_GL_FUNCTIONS_GL_OVR_multiview \
    X (void        , glFramebufferTextureMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews))

#define JUCE_GL_FUNCTIONS_GL_OVR_multiview_multisampled_render_to_texture \
    X (void        , glFramebufferTextureMultisampleMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLsizei samples, GLint baseViewIndex, GLsizei numViews))

#define JUCE_GL_FUNCTIONS_GL_QCOM_alpha_test \
    X (void        , glAlphaFuncQCOM, (GLenum func, GLclampf ref))

#define JUCE_GL_FUNCTIONS_GL_QCOM_driver_control \
    X (void        , glGetDriverControlsQCOM, (GLint *num, GLsizei size, GLuint *driverControls)) \
    X (void        , glGetDriverControlStringQCOM, (GLuint driverControl, GLsizei bufSize, GLsizei *length, GLchar *driverControlString)) \
    X (void        , glEnableDriverControlQCOM, (GLuint driverControl)) \
    X (void        , glDisableDriverControlQCOM, (GLuint driverControl))

#define JUCE_GL_FUNCTIONS_GL_QCOM_extended_get \
    X (void        , glExtGetTexturesQCOM, (GLuint *textures, GLint maxTextures, GLint *numTextures)) \
    X (void        , glExtGetBuffersQCOM, (GLuint *buffers, GLint maxBuffers, GLint *numBuffers)) \
    X (void        , glExtGetRenderbuffersQCOM, (GLuint *renderbuffers, GLint maxRenderbuffers, GLint *numRenderbuffers)) \
    X (void        , glExtGetFramebuffersQCOM, (GLuint *framebuffers, GLint maxFramebuffers, GLint *numFramebuffers)) \
    X (void        , glExtGetTexLevelParameterivQCOM, (GLuint texture, GLenum face, GLint level, GLenum pname, GLint *params)) \
    X (void        , glExtTexObjectStateOverrideiQCOM, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glExtGetTexSubImageQCOM, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, void *texels)) \
    X (void        , glExtGetBufferPointervQCOM, (GLenum target, void **params))

#define JUCE_GL_FUNCTIONS_GL_QCOM_extended_get2 \
    X (void        , glExtGetShadersQCOM, (GLuint *shaders, GLint maxShaders, GLint *numShaders)) \
    X (void        , glExtGetProgramsQCOM, (GLuint *programs, GLint maxPrograms, GLint *numPrograms)) \
    X (GLboolean   , glExtIsProgramBinaryQCOM, (GLuint program)) \
    X (void        , glExtGetProgramBinarySourceQCOM, (GLuint program, GLenum shadertype, GLchar *source, GLint *length))

#define JUCE_GL_FUNCTIONS_GL_QCOM_framebuffer_foveated \
    X (void        , glFramebufferFoveationConfigQCOM, (GLuint framebuffer, GLuint numLayers, GLuint focalPointsPerLayer, GLuint requestedFeatures, GLuint *providedFeatures)) \
    X (void        , glFramebufferFoveationParametersQCOM, (GLuint framebuffer, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea))

#define JUCE_GL_FUNCTIONS_GL_QCOM_motion_estimation \
    X (void        , glTexEstimateMotionQCOM, (GLuint ref, GLuint target, GLuint output)) \
    X (void        , glTexEstimateMotionRegionsQCOM, (GLuint ref, GLuint target, GLuint output, GLuint mask))

#define JUCE_GL_FUNCTIONS_GL_QCOM_frame_extrapolation \
    X (void        , glExtrapolateTex2DQCOM, (GLuint src1, GLuint src2, GLuint output, GLfloat scaleFactor))

#define JUCE_GL_FUNCTIONS_GL_QCOM_texture_foveated \
    X (void        , glTextureFoveationParametersQCOM, (GLuint texture, GLuint layer, GLuint focalPoint, GLfloat focalX, GLfloat focalY, GLfloat gainX, GLfloat gainY, GLfloat foveaArea))

#define JUCE_GL_FUNCTIONS_GL_QCOM_shader_framebuffer_fetch_noncoherent \
    X (void        , glFramebufferFetchBarrierQCOM, ())

#define JUCE_GL_FUNCTIONS_GL_QCOM_shading_rate \
    X (void        , glShadingRateQCOM, (GLenum rate))

#define JUCE_GL_FUNCTIONS_GL_QCOM_tiled_rendering \
    X (void        , glStartTilingQCOM, (GLuint x, GLuint y, GLuint width, GLuint height, GLbitfield preserveMask)) \
    X (void        , glEndTilingQCOM, (GLbitfield preserveMask))


#if JUCE_STATIC_LINK_GL_ES_VERSION_2_0
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0
#endif

#if JUCE_STATIC_LINK_GL_ES_VERSION_3_0
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0
#endif

#if JUCE_STATIC_LINK_GL_ES_VERSION_3_1
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1
#endif

#if JUCE_STATIC_LINK_GL_ES_VERSION_3_2
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC
 #define JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2
#endif


#define JUCE_STATIC_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_STATIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_STATIC

#define JUCE_DYNAMIC_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_2_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_1_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_ES_VERSION_3_2_DYNAMIC

#define JUCE_EXTENSION_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    JUCE_GL_FUNCTIONS_GL_AMD_performance_monitor \
    JUCE_GL_FUNCTIONS_GL_ANGLE_framebuffer_blit \
    JUCE_GL_FUNCTIONS_GL_ANGLE_framebuffer_multisample \
    JUCE_GL_FUNCTIONS_GL_ANGLE_instanced_arrays \
    JUCE_GL_FUNCTIONS_GL_ANGLE_translated_shader_source \
    JUCE_GL_FUNCTIONS_GL_APPLE_copy_texture_levels \
    JUCE_GL_FUNCTIONS_GL_APPLE_framebuffer_multisample \
    JUCE_GL_FUNCTIONS_GL_APPLE_sync \
    JUCE_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    JUCE_GL_FUNCTIONS_GL_EXT_base_instance \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_func_extended \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_minmax \
    JUCE_GL_FUNCTIONS_GL_EXT_buffer_storage \
    JUCE_GL_FUNCTIONS_GL_EXT_clear_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_clip_control \
    JUCE_GL_FUNCTIONS_GL_EXT_copy_image \
    JUCE_GL_FUNCTIONS_GL_EXT_debug_label \
    JUCE_GL_FUNCTIONS_GL_EXT_debug_marker \
    JUCE_GL_FUNCTIONS_GL_EXT_discard_framebuffer \
    JUCE_GL_FUNCTIONS_GL_EXT_disjoint_timer_query \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers_indexed \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_elements_base_vertex \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_instanced \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_transform_feedback \
    JUCE_GL_FUNCTIONS_GL_EXT_external_buffer \
    JUCE_GL_FUNCTIONS_GL_EXT_geometry_shader \
    JUCE_GL_FUNCTIONS_GL_EXT_instanced_arrays \
    JUCE_GL_FUNCTIONS_GL_EXT_map_buffer_range \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    JUCE_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    JUCE_GL_FUNCTIONS_GL_EXT_multi_draw_indirect \
    JUCE_GL_FUNCTIONS_GL_EXT_multisampled_render_to_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_multiview_draw_buffers \
    JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    JUCE_GL_FUNCTIONS_GL_EXT_primitive_bounding_box \
    JUCE_GL_FUNCTIONS_GL_EXT_raster_multisample \
    JUCE_GL_FUNCTIONS_GL_EXT_robustness \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    JUCE_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    JUCE_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    JUCE_GL_FUNCTIONS_GL_EXT_shader_pixel_local_storage2 \
    JUCE_GL_FUNCTIONS_GL_EXT_sparse_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_tessellation_shader \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_border_clamp \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_buffer \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_storage \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_view \
    JUCE_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    JUCE_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    JUCE_GL_FUNCTIONS_GL_EXT_window_rectangles \
    JUCE_GL_FUNCTIONS_GL_IMG_bindless_texture \
    JUCE_GL_FUNCTIONS_GL_IMG_framebuffer_downsample \
    JUCE_GL_FUNCTIONS_GL_IMG_multisampled_render_to_texture \
    JUCE_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    JUCE_GL_FUNCTIONS_GL_INTEL_performance_query \
    JUCE_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    JUCE_GL_FUNCTIONS_GL_KHR_debug \
    JUCE_GL_FUNCTIONS_GL_KHR_robustness \
    JUCE_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    JUCE_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    JUCE_GL_FUNCTIONS_GL_NV_bindless_texture \
    JUCE_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    JUCE_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    JUCE_GL_FUNCTIONS_GL_NV_conditional_render \
    JUCE_GL_FUNCTIONS_GL_NV_conservative_raster \
    JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    JUCE_GL_FUNCTIONS_GL_NV_copy_buffer \
    JUCE_GL_FUNCTIONS_GL_NV_coverage_sample \
    JUCE_GL_FUNCTIONS_GL_NV_draw_buffers \
    JUCE_GL_FUNCTIONS_GL_NV_draw_instanced \
    JUCE_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    JUCE_GL_FUNCTIONS_GL_NV_fence \
    JUCE_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    JUCE_GL_FUNCTIONS_GL_NV_framebuffer_blit \
    JUCE_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    JUCE_GL_FUNCTIONS_GL_NV_framebuffer_multisample \
    JUCE_GL_FUNCTIONS_GL_NV_gpu_shader5 \
    JUCE_GL_FUNCTIONS_GL_NV_instanced_arrays \
    JUCE_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    JUCE_GL_FUNCTIONS_GL_NV_memory_attachment \
    JUCE_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    JUCE_GL_FUNCTIONS_GL_NV_mesh_shader \
    JUCE_GL_FUNCTIONS_GL_NV_non_square_matrices \
    JUCE_GL_FUNCTIONS_GL_NV_path_rendering \
    JUCE_GL_FUNCTIONS_GL_NV_polygon_mode \
    JUCE_GL_FUNCTIONS_GL_NV_read_buffer \
    JUCE_GL_FUNCTIONS_GL_NV_sample_locations \
    JUCE_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    JUCE_GL_FUNCTIONS_GL_NV_shading_rate_image \
    JUCE_GL_FUNCTIONS_GL_NV_viewport_array \
    JUCE_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    JUCE_GL_FUNCTIONS_GL_OES_EGL_image \
    JUCE_GL_FUNCTIONS_GL_OES_copy_image \
    JUCE_GL_FUNCTIONS_GL_OES_draw_buffers_indexed \
    JUCE_GL_FUNCTIONS_GL_OES_draw_elements_base_vertex \
    JUCE_GL_FUNCTIONS_GL_OES_geometry_shader \
    JUCE_GL_FUNCTIONS_GL_OES_get_program_binary \
    JUCE_GL_FUNCTIONS_GL_OES_mapbuffer \
    JUCE_GL_FUNCTIONS_GL_OES_primitive_bounding_box \
    JUCE_GL_FUNCTIONS_GL_OES_sample_shading \
    JUCE_GL_FUNCTIONS_GL_OES_tessellation_shader \
    JUCE_GL_FUNCTIONS_GL_OES_texture_3D \
    JUCE_GL_FUNCTIONS_GL_OES_texture_border_clamp \
    JUCE_GL_FUNCTIONS_GL_OES_texture_buffer \
    JUCE_GL_FUNCTIONS_GL_OES_texture_storage_multisample_2d_array \
    JUCE_GL_FUNCTIONS_GL_OES_texture_view \
    JUCE_GL_FUNCTIONS_GL_OES_vertex_array_object \
    JUCE_GL_FUNCTIONS_GL_OES_viewport_array \
    JUCE_GL_FUNCTIONS_GL_OVR_multiview \
    JUCE_GL_FUNCTIONS_GL_OVR_multiview_multisampled_render_to_texture \
    JUCE_GL_FUNCTIONS_GL_QCOM_alpha_test \
    JUCE_GL_FUNCTIONS_GL_QCOM_driver_control \
    JUCE_GL_FUNCTIONS_GL_QCOM_extended_get \
    JUCE_GL_FUNCTIONS_GL_QCOM_extended_get2 \
    JUCE_GL_FUNCTIONS_GL_QCOM_framebuffer_foveated \
    JUCE_GL_FUNCTIONS_GL_QCOM_motion_estimation \
    JUCE_GL_FUNCTIONS_GL_QCOM_frame_extrapolation \
    JUCE_GL_FUNCTIONS_GL_QCOM_texture_foveated \
    JUCE_GL_FUNCTIONS_GL_QCOM_shader_framebuffer_fetch_noncoherent \
    JUCE_GL_FUNCTIONS_GL_QCOM_shading_rate \
    JUCE_GL_FUNCTIONS_GL_QCOM_tiled_rendering

#define X(returns, name, params) \
    extern "C" KHRONOS_APICALL returns KHRONOS_APIENTRY name params; \
    returns (KHRONOS_APIENTRY* const& ::juce::gl::name) params = ::name;
JUCE_STATIC_GL_FUNCTIONS
#undef X

#define X(returns, name, params) \
    static returns (KHRONOS_APIENTRY* juce_ ## name) params = nullptr; \
    returns (KHRONOS_APIENTRY* const& ::juce::gl::name) params = juce_ ## name;
JUCE_DYNAMIC_GL_FUNCTIONS
JUCE_EXTENSION_GL_FUNCTIONS
#undef X

void juce::gl::loadFunctions()
{
   #define X(returns, name, params) \
       juce_ ## name = reinterpret_cast<returns (KHRONOS_APIENTRY*) params> (::juce::OpenGLHelpers::getExtensionFunction (#name));
    JUCE_DYNAMIC_GL_FUNCTIONS
   #undef X
}

void juce::gl::loadExtensions()
{
   #define X(returns, name, params) \
       juce_ ## name = reinterpret_cast<returns (KHRONOS_APIENTRY*) params> (::juce::OpenGLHelpers::getExtensionFunction (#name));
    JUCE_EXTENSION_GL_FUNCTIONS
   #undef X
}

#undef JUCE_STATIC_GL_FUNCTIONS
#undef JUCE_DYNAMIC_GL_FUNCTIONS
#undef JUCE_EXTENSION_GL_FUNCTIONS
