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


#define JUCE_GL_FUNCTIONS_GL_VERSION_1_0 \
    X (void        , glCullFace, (GLenum mode)) \
    X (void        , glFrontFace, (GLenum mode)) \
    X (void        , glHint, (GLenum target, GLenum mode)) \
    X (void        , glLineWidth, (GLfloat width)) \
    X (void        , glPointSize, (GLfloat size)) \
    X (void        , glPolygonMode, (GLenum face, GLenum mode)) \
    X (void        , glScissor, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glTexParameterf, (GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glTexParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glTexParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glTexParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexImage1D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexImage2D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glDrawBuffer, (GLenum buf)) \
    X (void        , glClear, (GLbitfield mask)) \
    X (void        , glClearColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glClearStencil, (GLint s)) \
    X (void        , glClearDepth, (GLdouble depth)) \
    X (void        , glStencilMask, (GLuint mask)) \
    X (void        , glColorMask, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)) \
    X (void        , glDepthMask, (GLboolean flag)) \
    X (void        , glDisable, (GLenum cap)) \
    X (void        , glEnable, (GLenum cap)) \
    X (void        , glFinish, ()) \
    X (void        , glFlush, ()) \
    X (void        , glBlendFunc, (GLenum sfactor, GLenum dfactor)) \
    X (void        , glLogicOp, (GLenum opcode)) \
    X (void        , glStencilFunc, (GLenum func, GLint ref, GLuint mask)) \
    X (void        , glStencilOp, (GLenum fail, GLenum zfail, GLenum zpass)) \
    X (void        , glDepthFunc, (GLenum func)) \
    X (void        , glPixelStoref, (GLenum pname, GLfloat param)) \
    X (void        , glPixelStorei, (GLenum pname, GLint param)) \
    X (void        , glReadBuffer, (GLenum src)) \
    X (void        , glReadPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)) \
    X (void        , glGetBooleanv, (GLenum pname, GLboolean *data)) \
    X (void        , glGetDoublev, (GLenum pname, GLdouble *data)) \
    X (GLenum      , glGetError, ()) \
    X (void        , glGetFloatv, (GLenum pname, GLfloat *data)) \
    X (void        , glGetIntegerv, (GLenum pname, GLint *data)) \
    X (const GLubyte *, glGetString, (GLenum name)) \
    X (void        , glGetTexImage, (GLenum target, GLint level, GLenum format, GLenum type, void *pixels)) \
    X (void        , glGetTexParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetTexParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexLevelParameterfv, (GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (void        , glGetTexLevelParameteriv, (GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsEnabled, (GLenum cap)) \
    X (void        , glDepthRange, (GLdouble n, GLdouble f)) \
    X (void        , glViewport, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glNewList, (GLuint list, GLenum mode)) \
    X (void        , glEndList, ()) \
    X (void        , glCallList, (GLuint list)) \
    X (void        , glCallLists, (GLsizei n, GLenum type, const void *lists)) \
    X (void        , glDeleteLists, (GLuint list, GLsizei range)) \
    X (GLuint      , glGenLists, (GLsizei range)) \
    X (void        , glListBase, (GLuint base)) \
    X (void        , glBegin, (GLenum mode)) \
    X (void        , glBitmap, (GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)) \
    X (void        , glColor3b, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (void        , glColor3bv, (const GLbyte *v)) \
    X (void        , glColor3d, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (void        , glColor3dv, (const GLdouble *v)) \
    X (void        , glColor3f, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (void        , glColor3fv, (const GLfloat *v)) \
    X (void        , glColor3i, (GLint red, GLint green, GLint blue)) \
    X (void        , glColor3iv, (const GLint *v)) \
    X (void        , glColor3s, (GLshort red, GLshort green, GLshort blue)) \
    X (void        , glColor3sv, (const GLshort *v)) \
    X (void        , glColor3ub, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (void        , glColor3ubv, (const GLubyte *v)) \
    X (void        , glColor3ui, (GLuint red, GLuint green, GLuint blue)) \
    X (void        , glColor3uiv, (const GLuint *v)) \
    X (void        , glColor3us, (GLushort red, GLushort green, GLushort blue)) \
    X (void        , glColor3usv, (const GLushort *v)) \
    X (void        , glColor4b, (GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)) \
    X (void        , glColor4bv, (const GLbyte *v)) \
    X (void        , glColor4d, (GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)) \
    X (void        , glColor4dv, (const GLdouble *v)) \
    X (void        , glColor4f, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glColor4fv, (const GLfloat *v)) \
    X (void        , glColor4i, (GLint red, GLint green, GLint blue, GLint alpha)) \
    X (void        , glColor4iv, (const GLint *v)) \
    X (void        , glColor4s, (GLshort red, GLshort green, GLshort blue, GLshort alpha)) \
    X (void        , glColor4sv, (const GLshort *v)) \
    X (void        , glColor4ub, (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)) \
    X (void        , glColor4ubv, (const GLubyte *v)) \
    X (void        , glColor4ui, (GLuint red, GLuint green, GLuint blue, GLuint alpha)) \
    X (void        , glColor4uiv, (const GLuint *v)) \
    X (void        , glColor4us, (GLushort red, GLushort green, GLushort blue, GLushort alpha)) \
    X (void        , glColor4usv, (const GLushort *v)) \
    X (void        , glEdgeFlag, (GLboolean flag)) \
    X (void        , glEdgeFlagv, (const GLboolean *flag)) \
    X (void        , glEnd, ()) \
    X (void        , glIndexd, (GLdouble c)) \
    X (void        , glIndexdv, (const GLdouble *c)) \
    X (void        , glIndexf, (GLfloat c)) \
    X (void        , glIndexfv, (const GLfloat *c)) \
    X (void        , glIndexi, (GLint c)) \
    X (void        , glIndexiv, (const GLint *c)) \
    X (void        , glIndexs, (GLshort c)) \
    X (void        , glIndexsv, (const GLshort *c)) \
    X (void        , glNormal3b, (GLbyte nx, GLbyte ny, GLbyte nz)) \
    X (void        , glNormal3bv, (const GLbyte *v)) \
    X (void        , glNormal3d, (GLdouble nx, GLdouble ny, GLdouble nz)) \
    X (void        , glNormal3dv, (const GLdouble *v)) \
    X (void        , glNormal3f, (GLfloat nx, GLfloat ny, GLfloat nz)) \
    X (void        , glNormal3fv, (const GLfloat *v)) \
    X (void        , glNormal3i, (GLint nx, GLint ny, GLint nz)) \
    X (void        , glNormal3iv, (const GLint *v)) \
    X (void        , glNormal3s, (GLshort nx, GLshort ny, GLshort nz)) \
    X (void        , glNormal3sv, (const GLshort *v)) \
    X (void        , glRasterPos2d, (GLdouble x, GLdouble y)) \
    X (void        , glRasterPos2dv, (const GLdouble *v)) \
    X (void        , glRasterPos2f, (GLfloat x, GLfloat y)) \
    X (void        , glRasterPos2fv, (const GLfloat *v)) \
    X (void        , glRasterPos2i, (GLint x, GLint y)) \
    X (void        , glRasterPos2iv, (const GLint *v)) \
    X (void        , glRasterPos2s, (GLshort x, GLshort y)) \
    X (void        , glRasterPos2sv, (const GLshort *v)) \
    X (void        , glRasterPos3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glRasterPos3dv, (const GLdouble *v)) \
    X (void        , glRasterPos3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glRasterPos3fv, (const GLfloat *v)) \
    X (void        , glRasterPos3i, (GLint x, GLint y, GLint z)) \
    X (void        , glRasterPos3iv, (const GLint *v)) \
    X (void        , glRasterPos3s, (GLshort x, GLshort y, GLshort z)) \
    X (void        , glRasterPos3sv, (const GLshort *v)) \
    X (void        , glRasterPos4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glRasterPos4dv, (const GLdouble *v)) \
    X (void        , glRasterPos4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glRasterPos4fv, (const GLfloat *v)) \
    X (void        , glRasterPos4i, (GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glRasterPos4iv, (const GLint *v)) \
    X (void        , glRasterPos4s, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glRasterPos4sv, (const GLshort *v)) \
    X (void        , glRectd, (GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)) \
    X (void        , glRectdv, (const GLdouble *v1, const GLdouble *v2)) \
    X (void        , glRectf, (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)) \
    X (void        , glRectfv, (const GLfloat *v1, const GLfloat *v2)) \
    X (void        , glRecti, (GLint x1, GLint y1, GLint x2, GLint y2)) \
    X (void        , glRectiv, (const GLint *v1, const GLint *v2)) \
    X (void        , glRects, (GLshort x1, GLshort y1, GLshort x2, GLshort y2)) \
    X (void        , glRectsv, (const GLshort *v1, const GLshort *v2)) \
    X (void        , glTexCoord1d, (GLdouble s)) \
    X (void        , glTexCoord1dv, (const GLdouble *v)) \
    X (void        , glTexCoord1f, (GLfloat s)) \
    X (void        , glTexCoord1fv, (const GLfloat *v)) \
    X (void        , glTexCoord1i, (GLint s)) \
    X (void        , glTexCoord1iv, (const GLint *v)) \
    X (void        , glTexCoord1s, (GLshort s)) \
    X (void        , glTexCoord1sv, (const GLshort *v)) \
    X (void        , glTexCoord2d, (GLdouble s, GLdouble t)) \
    X (void        , glTexCoord2dv, (const GLdouble *v)) \
    X (void        , glTexCoord2f, (GLfloat s, GLfloat t)) \
    X (void        , glTexCoord2fv, (const GLfloat *v)) \
    X (void        , glTexCoord2i, (GLint s, GLint t)) \
    X (void        , glTexCoord2iv, (const GLint *v)) \
    X (void        , glTexCoord2s, (GLshort s, GLshort t)) \
    X (void        , glTexCoord2sv, (const GLshort *v)) \
    X (void        , glTexCoord3d, (GLdouble s, GLdouble t, GLdouble r)) \
    X (void        , glTexCoord3dv, (const GLdouble *v)) \
    X (void        , glTexCoord3f, (GLfloat s, GLfloat t, GLfloat r)) \
    X (void        , glTexCoord3fv, (const GLfloat *v)) \
    X (void        , glTexCoord3i, (GLint s, GLint t, GLint r)) \
    X (void        , glTexCoord3iv, (const GLint *v)) \
    X (void        , glTexCoord3s, (GLshort s, GLshort t, GLshort r)) \
    X (void        , glTexCoord3sv, (const GLshort *v)) \
    X (void        , glTexCoord4d, (GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (void        , glTexCoord4dv, (const GLdouble *v)) \
    X (void        , glTexCoord4f, (GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (void        , glTexCoord4fv, (const GLfloat *v)) \
    X (void        , glTexCoord4i, (GLint s, GLint t, GLint r, GLint q)) \
    X (void        , glTexCoord4iv, (const GLint *v)) \
    X (void        , glTexCoord4s, (GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (void        , glTexCoord4sv, (const GLshort *v)) \
    X (void        , glVertex2d, (GLdouble x, GLdouble y)) \
    X (void        , glVertex2dv, (const GLdouble *v)) \
    X (void        , glVertex2f, (GLfloat x, GLfloat y)) \
    X (void        , glVertex2fv, (const GLfloat *v)) \
    X (void        , glVertex2i, (GLint x, GLint y)) \
    X (void        , glVertex2iv, (const GLint *v)) \
    X (void        , glVertex2s, (GLshort x, GLshort y)) \
    X (void        , glVertex2sv, (const GLshort *v)) \
    X (void        , glVertex3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertex3dv, (const GLdouble *v)) \
    X (void        , glVertex3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertex3fv, (const GLfloat *v)) \
    X (void        , glVertex3i, (GLint x, GLint y, GLint z)) \
    X (void        , glVertex3iv, (const GLint *v)) \
    X (void        , glVertex3s, (GLshort x, GLshort y, GLshort z)) \
    X (void        , glVertex3sv, (const GLshort *v)) \
    X (void        , glVertex4d, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertex4dv, (const GLdouble *v)) \
    X (void        , glVertex4f, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertex4fv, (const GLfloat *v)) \
    X (void        , glVertex4i, (GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glVertex4iv, (const GLint *v)) \
    X (void        , glVertex4s, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glVertex4sv, (const GLshort *v)) \
    X (void        , glClipPlane, (GLenum plane, const GLdouble *equation)) \
    X (void        , glColorMaterial, (GLenum face, GLenum mode)) \
    X (void        , glFogf, (GLenum pname, GLfloat param)) \
    X (void        , glFogfv, (GLenum pname, const GLfloat *params)) \
    X (void        , glFogi, (GLenum pname, GLint param)) \
    X (void        , glFogiv, (GLenum pname, const GLint *params)) \
    X (void        , glLightf, (GLenum light, GLenum pname, GLfloat param)) \
    X (void        , glLightfv, (GLenum light, GLenum pname, const GLfloat *params)) \
    X (void        , glLighti, (GLenum light, GLenum pname, GLint param)) \
    X (void        , glLightiv, (GLenum light, GLenum pname, const GLint *params)) \
    X (void        , glLightModelf, (GLenum pname, GLfloat param)) \
    X (void        , glLightModelfv, (GLenum pname, const GLfloat *params)) \
    X (void        , glLightModeli, (GLenum pname, GLint param)) \
    X (void        , glLightModeliv, (GLenum pname, const GLint *params)) \
    X (void        , glLineStipple, (GLint factor, GLushort pattern)) \
    X (void        , glMaterialf, (GLenum face, GLenum pname, GLfloat param)) \
    X (void        , glMaterialfv, (GLenum face, GLenum pname, const GLfloat *params)) \
    X (void        , glMateriali, (GLenum face, GLenum pname, GLint param)) \
    X (void        , glMaterialiv, (GLenum face, GLenum pname, const GLint *params)) \
    X (void        , glPolygonStipple, (const GLubyte *mask)) \
    X (void        , glShadeModel, (GLenum mode)) \
    X (void        , glTexEnvf, (GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glTexEnvfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glTexEnvi, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glTexEnviv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexGend, (GLenum coord, GLenum pname, GLdouble param)) \
    X (void        , glTexGendv, (GLenum coord, GLenum pname, const GLdouble *params)) \
    X (void        , glTexGenf, (GLenum coord, GLenum pname, GLfloat param)) \
    X (void        , glTexGenfv, (GLenum coord, GLenum pname, const GLfloat *params)) \
    X (void        , glTexGeni, (GLenum coord, GLenum pname, GLint param)) \
    X (void        , glTexGeniv, (GLenum coord, GLenum pname, const GLint *params)) \
    X (void        , glFeedbackBuffer, (GLsizei size, GLenum type, GLfloat *buffer)) \
    X (void        , glSelectBuffer, (GLsizei size, GLuint *buffer)) \
    X (GLint       , glRenderMode, (GLenum mode)) \
    X (void        , glInitNames, ()) \
    X (void        , glLoadName, (GLuint name)) \
    X (void        , glPassThrough, (GLfloat token)) \
    X (void        , glPopName, ()) \
    X (void        , glPushName, (GLuint name)) \
    X (void        , glClearAccum, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glClearIndex, (GLfloat c)) \
    X (void        , glIndexMask, (GLuint mask)) \
    X (void        , glAccum, (GLenum op, GLfloat value)) \
    X (void        , glPopAttrib, ()) \
    X (void        , glPushAttrib, (GLbitfield mask)) \
    X (void        , glMap1d, (GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)) \
    X (void        , glMap1f, (GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)) \
    X (void        , glMap2d, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)) \
    X (void        , glMap2f, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points)) \
    X (void        , glMapGrid1d, (GLint un, GLdouble u1, GLdouble u2)) \
    X (void        , glMapGrid1f, (GLint un, GLfloat u1, GLfloat u2)) \
    X (void        , glMapGrid2d, (GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)) \
    X (void        , glMapGrid2f, (GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2)) \
    X (void        , glEvalCoord1d, (GLdouble u)) \
    X (void        , glEvalCoord1dv, (const GLdouble *u)) \
    X (void        , glEvalCoord1f, (GLfloat u)) \
    X (void        , glEvalCoord1fv, (const GLfloat *u)) \
    X (void        , glEvalCoord2d, (GLdouble u, GLdouble v)) \
    X (void        , glEvalCoord2dv, (const GLdouble *u)) \
    X (void        , glEvalCoord2f, (GLfloat u, GLfloat v)) \
    X (void        , glEvalCoord2fv, (const GLfloat *u)) \
    X (void        , glEvalMesh1, (GLenum mode, GLint i1, GLint i2)) \
    X (void        , glEvalPoint1, (GLint i)) \
    X (void        , glEvalMesh2, (GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)) \
    X (void        , glEvalPoint2, (GLint i, GLint j)) \
    X (void        , glAlphaFunc, (GLenum func, GLfloat ref)) \
    X (void        , glPixelZoom, (GLfloat xfactor, GLfloat yfactor)) \
    X (void        , glPixelTransferf, (GLenum pname, GLfloat param)) \
    X (void        , glPixelTransferi, (GLenum pname, GLint param)) \
    X (void        , glPixelMapfv, (GLenum map, GLsizei mapsize, const GLfloat *values)) \
    X (void        , glPixelMapuiv, (GLenum map, GLsizei mapsize, const GLuint *values)) \
    X (void        , glPixelMapusv, (GLenum map, GLsizei mapsize, const GLushort *values)) \
    X (void        , glCopyPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum type)) \
    X (void        , glDrawPixels, (GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glGetClipPlane, (GLenum plane, GLdouble *equation)) \
    X (void        , glGetLightfv, (GLenum light, GLenum pname, GLfloat *params)) \
    X (void        , glGetLightiv, (GLenum light, GLenum pname, GLint *params)) \
    X (void        , glGetMapdv, (GLenum target, GLenum query, GLdouble *v)) \
    X (void        , glGetMapfv, (GLenum target, GLenum query, GLfloat *v)) \
    X (void        , glGetMapiv, (GLenum target, GLenum query, GLint *v)) \
    X (void        , glGetMaterialfv, (GLenum face, GLenum pname, GLfloat *params)) \
    X (void        , glGetMaterialiv, (GLenum face, GLenum pname, GLint *params)) \
    X (void        , glGetPixelMapfv, (GLenum map, GLfloat *values)) \
    X (void        , glGetPixelMapuiv, (GLenum map, GLuint *values)) \
    X (void        , glGetPixelMapusv, (GLenum map, GLushort *values)) \
    X (void        , glGetPolygonStipple, (GLubyte *mask)) \
    X (void        , glGetTexEnvfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetTexEnviv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexGendv, (GLenum coord, GLenum pname, GLdouble *params)) \
    X (void        , glGetTexGenfv, (GLenum coord, GLenum pname, GLfloat *params)) \
    X (void        , glGetTexGeniv, (GLenum coord, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsList, (GLuint list)) \
    X (void        , glFrustum, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glLoadIdentity, ()) \
    X (void        , glLoadMatrixf, (const GLfloat *m)) \
    X (void        , glLoadMatrixd, (const GLdouble *m)) \
    X (void        , glMatrixMode, (GLenum mode)) \
    X (void        , glMultMatrixf, (const GLfloat *m)) \
    X (void        , glMultMatrixd, (const GLdouble *m)) \
    X (void        , glOrtho, (GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glPopMatrix, ()) \
    X (void        , glPushMatrix, ()) \
    X (void        , glRotated, (GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glRotatef, (GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glScaled, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glScalef, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTranslated, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glTranslatef, (GLfloat x, GLfloat y, GLfloat z))

#define JUCE_GL_FUNCTIONS_GL_VERSION_1_1 \
    X (void        , glDrawArrays, (GLenum mode, GLint first, GLsizei count)) \
    X (void        , glDrawElements, (GLenum mode, GLsizei count, GLenum type, const void *indices)) \
    X (void        , glGetPointerv, (GLenum pname, void **params)) \
    X (void        , glPolygonOffset, (GLfloat factor, GLfloat units)) \
    X (void        , glCopyTexImage1D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (void        , glCopyTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (void        , glCopyTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glBindTexture, (GLenum target, GLuint texture)) \
    X (void        , glDeleteTextures, (GLsizei n, const GLuint *textures)) \
    X (void        , glGenTextures, (GLsizei n, GLuint *textures)) \
    X (GLboolean   , glIsTexture, (GLuint texture)) \
    X (void        , glArrayElement, (GLint i)) \
    X (void        , glColorPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glDisableClientState, (GLenum array)) \
    X (void        , glEdgeFlagPointer, (GLsizei stride, const void *pointer)) \
    X (void        , glEnableClientState, (GLenum array)) \
    X (void        , glIndexPointer, (GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glInterleavedArrays, (GLenum format, GLsizei stride, const void *pointer)) \
    X (void        , glNormalPointer, (GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glTexCoordPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glVertexPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (GLboolean   , glAreTexturesResident, (GLsizei n, const GLuint *textures, GLboolean *residences)) \
    X (void        , glPrioritizeTextures, (GLsizei n, const GLuint *textures, const GLfloat *priorities)) \
    X (void        , glIndexub, (GLubyte c)) \
    X (void        , glIndexubv, (const GLubyte *c)) \
    X (void        , glPopClientAttrib, ()) \
    X (void        , glPushClientAttrib, (GLbitfield mask))

#define JUCE_GL_FUNCTIONS_GL_VERSION_1_2 \
    X (void        , glDrawRangeElements, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices)) \
    X (void        , glTexImage3D, (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_VERSION_1_3 \
    X (void        , glActiveTexture, (GLenum texture)) \
    X (void        , glSampleCoverage, (GLfloat value, GLboolean invert)) \
    X (void        , glCompressedTexImage3D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexImage2D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexImage1D, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage3D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage2D, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage1D, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glGetCompressedTexImage, (GLenum target, GLint level, void *img)) \
    X (void        , glClientActiveTexture, (GLenum texture)) \
    X (void        , glMultiTexCoord1d, (GLenum target, GLdouble s)) \
    X (void        , glMultiTexCoord1dv, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord1f, (GLenum target, GLfloat s)) \
    X (void        , glMultiTexCoord1fv, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord1i, (GLenum target, GLint s)) \
    X (void        , glMultiTexCoord1iv, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord1s, (GLenum target, GLshort s)) \
    X (void        , glMultiTexCoord1sv, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord2d, (GLenum target, GLdouble s, GLdouble t)) \
    X (void        , glMultiTexCoord2dv, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord2f, (GLenum target, GLfloat s, GLfloat t)) \
    X (void        , glMultiTexCoord2fv, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord2i, (GLenum target, GLint s, GLint t)) \
    X (void        , glMultiTexCoord2iv, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord2s, (GLenum target, GLshort s, GLshort t)) \
    X (void        , glMultiTexCoord2sv, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord3d, (GLenum target, GLdouble s, GLdouble t, GLdouble r)) \
    X (void        , glMultiTexCoord3dv, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord3f, (GLenum target, GLfloat s, GLfloat t, GLfloat r)) \
    X (void        , glMultiTexCoord3fv, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord3i, (GLenum target, GLint s, GLint t, GLint r)) \
    X (void        , glMultiTexCoord3iv, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord3s, (GLenum target, GLshort s, GLshort t, GLshort r)) \
    X (void        , glMultiTexCoord3sv, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord4d, (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (void        , glMultiTexCoord4dv, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord4f, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (void        , glMultiTexCoord4fv, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord4i, (GLenum target, GLint s, GLint t, GLint r, GLint q)) \
    X (void        , glMultiTexCoord4iv, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord4s, (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (void        , glMultiTexCoord4sv, (GLenum target, const GLshort *v)) \
    X (void        , glLoadTransposeMatrixf, (const GLfloat *m)) \
    X (void        , glLoadTransposeMatrixd, (const GLdouble *m)) \
    X (void        , glMultTransposeMatrixf, (const GLfloat *m)) \
    X (void        , glMultTransposeMatrixd, (const GLdouble *m))

#define JUCE_GL_FUNCTIONS_GL_VERSION_1_4 \
    X (void        , glBlendFuncSeparate, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha)) \
    X (void        , glMultiDrawArrays, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei drawcount)) \
    X (void        , glMultiDrawElements, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount)) \
    X (void        , glPointParameterf, (GLenum pname, GLfloat param)) \
    X (void        , glPointParameterfv, (GLenum pname, const GLfloat *params)) \
    X (void        , glPointParameteri, (GLenum pname, GLint param)) \
    X (void        , glPointParameteriv, (GLenum pname, const GLint *params)) \
    X (void        , glFogCoordf, (GLfloat coord)) \
    X (void        , glFogCoordfv, (const GLfloat *coord)) \
    X (void        , glFogCoordd, (GLdouble coord)) \
    X (void        , glFogCoorddv, (const GLdouble *coord)) \
    X (void        , glFogCoordPointer, (GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glSecondaryColor3b, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (void        , glSecondaryColor3bv, (const GLbyte *v)) \
    X (void        , glSecondaryColor3d, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (void        , glSecondaryColor3dv, (const GLdouble *v)) \
    X (void        , glSecondaryColor3f, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (void        , glSecondaryColor3fv, (const GLfloat *v)) \
    X (void        , glSecondaryColor3i, (GLint red, GLint green, GLint blue)) \
    X (void        , glSecondaryColor3iv, (const GLint *v)) \
    X (void        , glSecondaryColor3s, (GLshort red, GLshort green, GLshort blue)) \
    X (void        , glSecondaryColor3sv, (const GLshort *v)) \
    X (void        , glSecondaryColor3ub, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (void        , glSecondaryColor3ubv, (const GLubyte *v)) \
    X (void        , glSecondaryColor3ui, (GLuint red, GLuint green, GLuint blue)) \
    X (void        , glSecondaryColor3uiv, (const GLuint *v)) \
    X (void        , glSecondaryColor3us, (GLushort red, GLushort green, GLushort blue)) \
    X (void        , glSecondaryColor3usv, (const GLushort *v)) \
    X (void        , glSecondaryColorPointer, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glWindowPos2d, (GLdouble x, GLdouble y)) \
    X (void        , glWindowPos2dv, (const GLdouble *v)) \
    X (void        , glWindowPos2f, (GLfloat x, GLfloat y)) \
    X (void        , glWindowPos2fv, (const GLfloat *v)) \
    X (void        , glWindowPos2i, (GLint x, GLint y)) \
    X (void        , glWindowPos2iv, (const GLint *v)) \
    X (void        , glWindowPos2s, (GLshort x, GLshort y)) \
    X (void        , glWindowPos2sv, (const GLshort *v)) \
    X (void        , glWindowPos3d, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glWindowPos3dv, (const GLdouble *v)) \
    X (void        , glWindowPos3f, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glWindowPos3fv, (const GLfloat *v)) \
    X (void        , glWindowPos3i, (GLint x, GLint y, GLint z)) \
    X (void        , glWindowPos3iv, (const GLint *v)) \
    X (void        , glWindowPos3s, (GLshort x, GLshort y, GLshort z)) \
    X (void        , glWindowPos3sv, (const GLshort *v)) \
    X (void        , glBlendColor, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)) \
    X (void        , glBlendEquation, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_VERSION_1_5 \
    X (void        , glGenQueries, (GLsizei n, GLuint *ids)) \
    X (void        , glDeleteQueries, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQuery, (GLuint id)) \
    X (void        , glBeginQuery, (GLenum target, GLuint id)) \
    X (void        , glEndQuery, (GLenum target)) \
    X (void        , glGetQueryiv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectiv, (GLuint id, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectuiv, (GLuint id, GLenum pname, GLuint *params)) \
    X (void        , glBindBuffer, (GLenum target, GLuint buffer)) \
    X (void        , glDeleteBuffers, (GLsizei n, const GLuint *buffers)) \
    X (void        , glGenBuffers, (GLsizei n, GLuint *buffers)) \
    X (GLboolean   , glIsBuffer, (GLuint buffer)) \
    X (void        , glBufferData, (GLenum target, GLsizeiptr size, const void *data, GLenum usage)) \
    X (void        , glBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (void        , glGetBufferSubData, (GLenum target, GLintptr offset, GLsizeiptr size, void *data)) \
    X (void *      , glMapBuffer, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBuffer, (GLenum target)) \
    X (void        , glGetBufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetBufferPointerv, (GLenum target, GLenum pname, void **params))

#define JUCE_GL_FUNCTIONS_GL_VERSION_2_0 \
    X (void        , glBlendEquationSeparate, (GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glDrawBuffers, (GLsizei n, const GLenum *bufs)) \
    X (void        , glStencilOpSeparate, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (void        , glStencilFuncSeparate, (GLenum face, GLenum func, GLint ref, GLuint mask)) \
    X (void        , glStencilMaskSeparate, (GLenum face, GLuint mask)) \
    X (void        , glAttachShader, (GLuint program, GLuint shader)) \
    X (void        , glBindAttribLocation, (GLuint program, GLuint index, const GLchar *name)) \
    X (void        , glCompileShader, (GLuint shader)) \
    X (GLuint      , glCreateProgram, ()) \
    X (GLuint      , glCreateShader, (GLenum type)) \
    X (void        , glDeleteProgram, (GLuint program)) \
    X (void        , glDeleteShader, (GLuint shader)) \
    X (void        , glDetachShader, (GLuint program, GLuint shader)) \
    X (void        , glDisableVertexAttribArray, (GLuint index)) \
    X (void        , glEnableVertexAttribArray, (GLuint index)) \
    X (void        , glGetActiveAttrib, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (void        , glGetActiveUniform, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLint *size, GLenum *type, GLchar *name)) \
    X (void        , glGetAttachedShaders, (GLuint program, GLsizei maxCount, GLsizei *count, GLuint *shaders)) \
    X (GLint       , glGetAttribLocation, (GLuint program, const GLchar *name)) \
    X (void        , glGetProgramiv, (GLuint program, GLenum pname, GLint *params)) \
    X (void        , glGetProgramInfoLog, (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetShaderiv, (GLuint shader, GLenum pname, GLint *params)) \
    X (void        , glGetShaderInfoLog, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetShaderSource, (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *source)) \
    X (GLint       , glGetUniformLocation, (GLuint program, const GLchar *name)) \
    X (void        , glGetUniformfv, (GLuint program, GLint location, GLfloat *params)) \
    X (void        , glGetUniformiv, (GLuint program, GLint location, GLint *params)) \
    X (void        , glGetVertexAttribdv, (GLuint index, GLenum pname, GLdouble *params)) \
    X (void        , glGetVertexAttribfv, (GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetVertexAttribiv, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribPointerv, (GLuint index, GLenum pname, void **pointer)) \
    X (GLboolean   , glIsProgram, (GLuint program)) \
    X (GLboolean   , glIsShader, (GLuint shader)) \
    X (void        , glLinkProgram, (GLuint program)) \
    X (void        , glShaderSource, (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length)) \
    X (void        , glUseProgram, (GLuint program)) \
    X (void        , glUniform1f, (GLint location, GLfloat v0)) \
    X (void        , glUniform2f, (GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glUniform3f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glUniform4f, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glUniform1i, (GLint location, GLint v0)) \
    X (void        , glUniform2i, (GLint location, GLint v0, GLint v1)) \
    X (void        , glUniform3i, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glUniform4i, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glUniform1fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform2fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform3fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform4fv, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform1iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform2iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform3iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform4iv, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniformMatrix2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glValidateProgram, (GLuint program)) \
    X (void        , glVertexAttrib1d, (GLuint index, GLdouble x)) \
    X (void        , glVertexAttrib1dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib1f, (GLuint index, GLfloat x)) \
    X (void        , glVertexAttrib1fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib1s, (GLuint index, GLshort x)) \
    X (void        , glVertexAttrib1sv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib2d, (GLuint index, GLdouble x, GLdouble y)) \
    X (void        , glVertexAttrib2dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib2f, (GLuint index, GLfloat x, GLfloat y)) \
    X (void        , glVertexAttrib2fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib2s, (GLuint index, GLshort x, GLshort y)) \
    X (void        , glVertexAttrib2sv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib3d, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexAttrib3dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib3f, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertexAttrib3fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib3s, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (void        , glVertexAttrib3sv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4Nbv, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttrib4Niv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttrib4Nsv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4Nub, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (void        , glVertexAttrib4Nubv, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttrib4Nuiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttrib4Nusv, (GLuint index, const GLushort *v)) \
    X (void        , glVertexAttrib4bv, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttrib4d, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexAttrib4dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib4f, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertexAttrib4fv, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib4iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttrib4s, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glVertexAttrib4sv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4ubv, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttrib4uiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttrib4usv, (GLuint index, const GLushort *v)) \
    X (void        , glVertexAttribPointer, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_VERSION_2_1 \
    X (void        , glUniformMatrix2x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix2x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x2fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3x4fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4x3fv, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value))

#define JUCE_GL_FUNCTIONS_GL_VERSION_3_0 \
    X (void        , glColorMaski, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a)) \
    X (void        , glGetBooleani_v, (GLenum target, GLuint index, GLboolean *data)) \
    X (void        , glGetIntegeri_v, (GLenum target, GLuint index, GLint *data)) \
    X (void        , glEnablei, (GLenum target, GLuint index)) \
    X (void        , glDisablei, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnabledi, (GLenum target, GLuint index)) \
    X (void        , glBeginTransformFeedback, (GLenum primitiveMode)) \
    X (void        , glEndTransformFeedback, ()) \
    X (void        , glBindBufferRange, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glBindBufferBase, (GLenum target, GLuint index, GLuint buffer)) \
    X (void        , glTransformFeedbackVaryings, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (void        , glGetTransformFeedbackVarying, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (void        , glClampColor, (GLenum target, GLenum clamp)) \
    X (void        , glBeginConditionalRender, (GLuint id, GLenum mode)) \
    X (void        , glEndConditionalRender, ()) \
    X (void        , glVertexAttribIPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glGetVertexAttribIiv, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribIuiv, (GLuint index, GLenum pname, GLuint *params)) \
    X (void        , glVertexAttribI1i, (GLuint index, GLint x)) \
    X (void        , glVertexAttribI2i, (GLuint index, GLint x, GLint y)) \
    X (void        , glVertexAttribI3i, (GLuint index, GLint x, GLint y, GLint z)) \
    X (void        , glVertexAttribI4i, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glVertexAttribI1ui, (GLuint index, GLuint x)) \
    X (void        , glVertexAttribI2ui, (GLuint index, GLuint x, GLuint y)) \
    X (void        , glVertexAttribI3ui, (GLuint index, GLuint x, GLuint y, GLuint z)) \
    X (void        , glVertexAttribI4ui, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glVertexAttribI1iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI2iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI3iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI4iv, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI1uiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI2uiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI3uiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI4uiv, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI4bv, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttribI4sv, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttribI4ubv, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttribI4usv, (GLuint index, const GLushort *v)) \
    X (void        , glGetUniformuiv, (GLuint program, GLint location, GLuint *params)) \
    X (void        , glBindFragDataLocation, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetFragDataLocation, (GLuint program, const GLchar *name)) \
    X (void        , glUniform1ui, (GLint location, GLuint v0)) \
    X (void        , glUniform2ui, (GLint location, GLuint v0, GLuint v1)) \
    X (void        , glUniform3ui, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glUniform4ui, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glUniform1uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform2uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform3uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform4uiv, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glTexParameterIiv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexParameterIuiv, (GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTexParameterIiv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexParameterIuiv, (GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glClearBufferiv, (GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (void        , glClearBufferuiv, (GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (void        , glClearBufferfv, (GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (void        , glClearBufferfi, (GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (const GLubyte *, glGetStringi, (GLenum name, GLuint index)) \
    X (GLboolean   , glIsRenderbuffer, (GLuint renderbuffer)) \
    X (void        , glBindRenderbuffer, (GLenum target, GLuint renderbuffer)) \
    X (void        , glDeleteRenderbuffers, (GLsizei n, const GLuint *renderbuffers)) \
    X (void        , glGenRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (void        , glRenderbufferStorage, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glGetRenderbufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsFramebuffer, (GLuint framebuffer)) \
    X (void        , glBindFramebuffer, (GLenum target, GLuint framebuffer)) \
    X (void        , glDeleteFramebuffers, (GLsizei n, const GLuint *framebuffers)) \
    X (void        , glGenFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (GLenum      , glCheckFramebufferStatus, (GLenum target)) \
    X (void        , glFramebufferTexture1D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glFramebufferTexture2D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glFramebufferTexture3D, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (void        , glFramebufferRenderbuffer, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (void        , glGetFramebufferAttachmentParameteriv, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (void        , glGenerateMipmap, (GLenum target)) \
    X (void        , glBlitFramebuffer, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (void        , glRenderbufferStorageMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glFramebufferTextureLayer, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (void *      , glMapBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (void        , glFlushMappedBufferRange, (GLenum target, GLintptr offset, GLsizeiptr length)) \
    X (void        , glBindVertexArray, (GLuint array)) \
    X (void        , glDeleteVertexArrays, (GLsizei n, const GLuint *arrays)) \
    X (void        , glGenVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArray, (GLuint array))

#define JUCE_GL_FUNCTIONS_GL_VERSION_3_1 \
    X (void        , glDrawArraysInstanced, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount)) \
    X (void        , glDrawElementsInstanced, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount)) \
    X (void        , glTexBuffer, (GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glPrimitiveRestartIndex, (GLuint index)) \
    X (void        , glCopyBufferSubData, (GLenum readTarget, GLenum writeTarget, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (void        , glGetUniformIndices, (GLuint program, GLsizei uniformCount, const GLchar *const*uniformNames, GLuint *uniformIndices)) \
    X (void        , glGetActiveUniformsiv, (GLuint program, GLsizei uniformCount, const GLuint *uniformIndices, GLenum pname, GLint *params)) \
    X (void        , glGetActiveUniformName, (GLuint program, GLuint uniformIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformName)) \
    X (GLuint      , glGetUniformBlockIndex, (GLuint program, const GLchar *uniformBlockName)) \
    X (void        , glGetActiveUniformBlockiv, (GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint *params)) \
    X (void        , glGetActiveUniformBlockName, (GLuint program, GLuint uniformBlockIndex, GLsizei bufSize, GLsizei *length, GLchar *uniformBlockName)) \
    X (void        , glUniformBlockBinding, (GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding))

#define JUCE_GL_FUNCTIONS_GL_VERSION_3_2 \
    X (void        , glDrawElementsBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawRangeElementsBaseVertex, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex)) \
    X (void        , glDrawElementsInstancedBaseVertex, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex)) \
    X (void        , glMultiDrawElementsBaseVertex, (GLenum mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei drawcount, const GLint *basevertex)) \
    X (void        , glProvokingVertex, (GLenum mode)) \
    X (GLsync      , glFenceSync, (GLenum condition, GLbitfield flags)) \
    X (GLboolean   , glIsSync, (GLsync sync)) \
    X (void        , glDeleteSync, (GLsync sync)) \
    X (GLenum      , glClientWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glWaitSync, (GLsync sync, GLbitfield flags, GLuint64 timeout)) \
    X (void        , glGetInteger64v, (GLenum pname, GLint64 *data)) \
    X (void        , glGetSynciv, (GLsync sync, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (void        , glGetInteger64i_v, (GLenum target, GLuint index, GLint64 *data)) \
    X (void        , glGetBufferParameteri64v, (GLenum target, GLenum pname, GLint64 *params)) \
    X (void        , glFramebufferTexture, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glTexImage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (void        , glTexImage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (void        , glGetMultisamplefv, (GLenum pname, GLuint index, GLfloat *val)) \
    X (void        , glSampleMaski, (GLuint maskNumber, GLbitfield mask))

#define JUCE_GL_FUNCTIONS_GL_VERSION_3_3 \
    X (void        , glBindFragDataLocationIndexed, (GLuint program, GLuint colorNumber, GLuint index, const GLchar *name)) \
    X (GLint       , glGetFragDataIndex, (GLuint program, const GLchar *name)) \
    X (void        , glGenSamplers, (GLsizei count, GLuint *samplers)) \
    X (void        , glDeleteSamplers, (GLsizei count, const GLuint *samplers)) \
    X (GLboolean   , glIsSampler, (GLuint sampler)) \
    X (void        , glBindSampler, (GLuint unit, GLuint sampler)) \
    X (void        , glSamplerParameteri, (GLuint sampler, GLenum pname, GLint param)) \
    X (void        , glSamplerParameteriv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterf, (GLuint sampler, GLenum pname, GLfloat param)) \
    X (void        , glSamplerParameterfv, (GLuint sampler, GLenum pname, const GLfloat *param)) \
    X (void        , glSamplerParameterIiv, (GLuint sampler, GLenum pname, const GLint *param)) \
    X (void        , glSamplerParameterIuiv, (GLuint sampler, GLenum pname, const GLuint *param)) \
    X (void        , glGetSamplerParameteriv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterIiv, (GLuint sampler, GLenum pname, GLint *params)) \
    X (void        , glGetSamplerParameterfv, (GLuint sampler, GLenum pname, GLfloat *params)) \
    X (void        , glGetSamplerParameterIuiv, (GLuint sampler, GLenum pname, GLuint *params)) \
    X (void        , glQueryCounter, (GLuint id, GLenum target)) \
    X (void        , glGetQueryObjecti64v, (GLuint id, GLenum pname, GLint64 *params)) \
    X (void        , glGetQueryObjectui64v, (GLuint id, GLenum pname, GLuint64 *params)) \
    X (void        , glVertexAttribDivisor, (GLuint index, GLuint divisor)) \
    X (void        , glVertexAttribP1ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (void        , glVertexAttribP1uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (void        , glVertexAttribP2ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (void        , glVertexAttribP2uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (void        , glVertexAttribP3ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (void        , glVertexAttribP3uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (void        , glVertexAttribP4ui, (GLuint index, GLenum type, GLboolean normalized, GLuint value)) \
    X (void        , glVertexAttribP4uiv, (GLuint index, GLenum type, GLboolean normalized, const GLuint *value)) \
    X (void        , glVertexP2ui, (GLenum type, GLuint value)) \
    X (void        , glVertexP2uiv, (GLenum type, const GLuint *value)) \
    X (void        , glVertexP3ui, (GLenum type, GLuint value)) \
    X (void        , glVertexP3uiv, (GLenum type, const GLuint *value)) \
    X (void        , glVertexP4ui, (GLenum type, GLuint value)) \
    X (void        , glVertexP4uiv, (GLenum type, const GLuint *value)) \
    X (void        , glTexCoordP1ui, (GLenum type, GLuint coords)) \
    X (void        , glTexCoordP1uiv, (GLenum type, const GLuint *coords)) \
    X (void        , glTexCoordP2ui, (GLenum type, GLuint coords)) \
    X (void        , glTexCoordP2uiv, (GLenum type, const GLuint *coords)) \
    X (void        , glTexCoordP3ui, (GLenum type, GLuint coords)) \
    X (void        , glTexCoordP3uiv, (GLenum type, const GLuint *coords)) \
    X (void        , glTexCoordP4ui, (GLenum type, GLuint coords)) \
    X (void        , glTexCoordP4uiv, (GLenum type, const GLuint *coords)) \
    X (void        , glMultiTexCoordP1ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (void        , glMultiTexCoordP1uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (void        , glMultiTexCoordP2ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (void        , glMultiTexCoordP2uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (void        , glMultiTexCoordP3ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (void        , glMultiTexCoordP3uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (void        , glMultiTexCoordP4ui, (GLenum texture, GLenum type, GLuint coords)) \
    X (void        , glMultiTexCoordP4uiv, (GLenum texture, GLenum type, const GLuint *coords)) \
    X (void        , glNormalP3ui, (GLenum type, GLuint coords)) \
    X (void        , glNormalP3uiv, (GLenum type, const GLuint *coords)) \
    X (void        , glColorP3ui, (GLenum type, GLuint color)) \
    X (void        , glColorP3uiv, (GLenum type, const GLuint *color)) \
    X (void        , glColorP4ui, (GLenum type, GLuint color)) \
    X (void        , glColorP4uiv, (GLenum type, const GLuint *color)) \
    X (void        , glSecondaryColorP3ui, (GLenum type, GLuint color)) \
    X (void        , glSecondaryColorP3uiv, (GLenum type, const GLuint *color))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_0 \
    X (void        , glMinSampleShading, (GLfloat value)) \
    X (void        , glBlendEquationi, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparatei, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunci, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparatei, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (void        , glDrawArraysIndirect, (GLenum mode, const void *indirect)) \
    X (void        , glDrawElementsIndirect, (GLenum mode, GLenum type, const void *indirect)) \
    X (void        , glUniform1d, (GLint location, GLdouble x)) \
    X (void        , glUniform2d, (GLint location, GLdouble x, GLdouble y)) \
    X (void        , glUniform3d, (GLint location, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glUniform4d, (GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glUniform1dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glUniform2dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glUniform3dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glUniform4dv, (GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glUniformMatrix2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix2x3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix2x4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix3x2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix3x4dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix4x2dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glUniformMatrix4x3dv, (GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glGetUniformdv, (GLuint program, GLint location, GLdouble *params)) \
    X (GLint       , glGetSubroutineUniformLocation, (GLuint program, GLenum shadertype, const GLchar *name)) \
    X (GLuint      , glGetSubroutineIndex, (GLuint program, GLenum shadertype, const GLchar *name)) \
    X (void        , glGetActiveSubroutineUniformiv, (GLuint program, GLenum shadertype, GLuint index, GLenum pname, GLint *values)) \
    X (void        , glGetActiveSubroutineUniformName, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (void        , glGetActiveSubroutineName, (GLuint program, GLenum shadertype, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (void        , glUniformSubroutinesuiv, (GLenum shadertype, GLsizei count, const GLuint *indices)) \
    X (void        , glGetUniformSubroutineuiv, (GLenum shadertype, GLint location, GLuint *params)) \
    X (void        , glGetProgramStageiv, (GLuint program, GLenum shadertype, GLenum pname, GLint *values)) \
    X (void        , glPatchParameteri, (GLenum pname, GLint value)) \
    X (void        , glPatchParameterfv, (GLenum pname, const GLfloat *values)) \
    X (void        , glBindTransformFeedback, (GLenum target, GLuint id)) \
    X (void        , glDeleteTransformFeedbacks, (GLsizei n, const GLuint *ids)) \
    X (void        , glGenTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedback, (GLuint id)) \
    X (void        , glPauseTransformFeedback, ()) \
    X (void        , glResumeTransformFeedback, ()) \
    X (void        , glDrawTransformFeedback, (GLenum mode, GLuint id)) \
    X (void        , glDrawTransformFeedbackStream, (GLenum mode, GLuint id, GLuint stream)) \
    X (void        , glBeginQueryIndexed, (GLenum target, GLuint index, GLuint id)) \
    X (void        , glEndQueryIndexed, (GLenum target, GLuint index)) \
    X (void        , glGetQueryIndexediv, (GLenum target, GLuint index, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_1 \
    X (void        , glReleaseShaderCompiler, ()) \
    X (void        , glShaderBinary, (GLsizei count, const GLuint *shaders, GLenum binaryFormat, const void *binary, GLsizei length)) \
    X (void        , glGetShaderPrecisionFormat, (GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)) \
    X (void        , glDepthRangef, (GLfloat n, GLfloat f)) \
    X (void        , glClearDepthf, (GLfloat d)) \
    X (void        , glGetProgramBinary, (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary)) \
    X (void        , glProgramBinary, (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length)) \
    X (void        , glProgramParameteri, (GLuint program, GLenum pname, GLint value)) \
    X (void        , glUseProgramStages, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (void        , glActiveShaderProgram, (GLuint pipeline, GLuint program)) \
    X (GLuint      , glCreateShaderProgramv, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (void        , glBindProgramPipeline, (GLuint pipeline)) \
    X (void        , glDeleteProgramPipelines, (GLsizei n, const GLuint *pipelines)) \
    X (void        , glGenProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (GLboolean   , glIsProgramPipeline, (GLuint pipeline)) \
    X (void        , glGetProgramPipelineiv, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (void        , glProgramUniform1i, (GLuint program, GLint location, GLint v0)) \
    X (void        , glProgramUniform1iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform1f, (GLuint program, GLint location, GLfloat v0)) \
    X (void        , glProgramUniform1fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform1d, (GLuint program, GLint location, GLdouble v0)) \
    X (void        , glProgramUniform1dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform1ui, (GLuint program, GLint location, GLuint v0)) \
    X (void        , glProgramUniform1uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform2i, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (void        , glProgramUniform2iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform2f, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glProgramUniform2fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform2d, (GLuint program, GLint location, GLdouble v0, GLdouble v1)) \
    X (void        , glProgramUniform2dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform2ui, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (void        , glProgramUniform2uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform3i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glProgramUniform3iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform3f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glProgramUniform3fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform3d, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2)) \
    X (void        , glProgramUniform3dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform3ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glProgramUniform3uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform4i, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glProgramUniform4iv, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform4f, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glProgramUniform4fv, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform4d, (GLuint program, GLint location, GLdouble v0, GLdouble v1, GLdouble v2, GLdouble v3)) \
    X (void        , glProgramUniform4dv, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform4ui, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glProgramUniform4uiv, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniformMatrix2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix2x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x2fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x4fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x3fv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3x2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix2x4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4x2dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3x4dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4x3dv, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glValidateProgramPipeline, (GLuint pipeline)) \
    X (void        , glGetProgramPipelineInfoLog, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glVertexAttribL1d, (GLuint index, GLdouble x)) \
    X (void        , glVertexAttribL2d, (GLuint index, GLdouble x, GLdouble y)) \
    X (void        , glVertexAttribL3d, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexAttribL4d, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexAttribL1dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL2dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL3dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL4dv, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribLPointer, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glGetVertexAttribLdv, (GLuint index, GLenum pname, GLdouble *params)) \
    X (void        , glViewportArrayv, (GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glViewportIndexedf, (GLuint index, GLfloat x, GLfloat y, GLfloat w, GLfloat h)) \
    X (void        , glViewportIndexedfv, (GLuint index, const GLfloat *v)) \
    X (void        , glScissorArrayv, (GLuint first, GLsizei count, const GLint *v)) \
    X (void        , glScissorIndexed, (GLuint index, GLint left, GLint bottom, GLsizei width, GLsizei height)) \
    X (void        , glScissorIndexedv, (GLuint index, const GLint *v)) \
    X (void        , glDepthRangeArrayv, (GLuint first, GLsizei count, const GLdouble *v)) \
    X (void        , glDepthRangeIndexed, (GLuint index, GLdouble n, GLdouble f)) \
    X (void        , glGetFloati_v, (GLenum target, GLuint index, GLfloat *data)) \
    X (void        , glGetDoublei_v, (GLenum target, GLuint index, GLdouble *data))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_2 \
    X (void        , glDrawArraysInstancedBaseInstance, (GLenum mode, GLint first, GLsizei count, GLsizei instancecount, GLuint baseinstance)) \
    X (void        , glDrawElementsInstancedBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLuint baseinstance)) \
    X (void        , glDrawElementsInstancedBaseVertexBaseInstance, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex, GLuint baseinstance)) \
    X (void        , glGetInternalformativ, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint *params)) \
    X (void        , glGetActiveAtomicCounterBufferiv, (GLuint program, GLuint bufferIndex, GLenum pname, GLint *params)) \
    X (void        , glBindImageTexture, (GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)) \
    X (void        , glMemoryBarrier, (GLbitfield barriers)) \
    X (void        , glTexStorage1D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTexStorage2D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTexStorage3D, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glDrawTransformFeedbackInstanced, (GLenum mode, GLuint id, GLsizei instancecount)) \
    X (void        , glDrawTransformFeedbackStreamInstanced, (GLenum mode, GLuint id, GLuint stream, GLsizei instancecount))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_3 \
    X (void        , glClearBufferData, (GLenum target, GLenum internalformat, GLenum format, GLenum type, const void *data)) \
    X (void        , glClearBufferSubData, (GLenum target, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)) \
    X (void        , glDispatchCompute, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z)) \
    X (void        , glDispatchComputeIndirect, (GLintptr indirect)) \
    X (void        , glCopyImageSubData, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (void        , glFramebufferParameteri, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glGetFramebufferParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetInternalformati64v, (GLenum target, GLenum internalformat, GLenum pname, GLsizei count, GLint64 *params)) \
    X (void        , glInvalidateTexSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glInvalidateTexImage, (GLuint texture, GLint level)) \
    X (void        , glInvalidateBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (void        , glInvalidateBufferData, (GLuint buffer)) \
    X (void        , glInvalidateFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments)) \
    X (void        , glInvalidateSubFramebuffer, (GLenum target, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glMultiDrawArraysIndirect, (GLenum mode, const void *indirect, GLsizei drawcount, GLsizei stride)) \
    X (void        , glMultiDrawElementsIndirect, (GLenum mode, GLenum type, const void *indirect, GLsizei drawcount, GLsizei stride)) \
    X (void        , glGetProgramInterfaceiv, (GLuint program, GLenum programInterface, GLenum pname, GLint *params)) \
    X (GLuint      , glGetProgramResourceIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (void        , glGetProgramResourceName, (GLuint program, GLenum programInterface, GLuint index, GLsizei bufSize, GLsizei *length, GLchar *name)) \
    X (void        , glGetProgramResourceiv, (GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei count, GLsizei *length, GLint *params)) \
    X (GLint       , glGetProgramResourceLocation, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (GLint       , glGetProgramResourceLocationIndex, (GLuint program, GLenum programInterface, const GLchar *name)) \
    X (void        , glShaderStorageBlockBinding, (GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding)) \
    X (void        , glTexBufferRange, (GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glTexStorage2DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (void        , glTexStorage3DMultisample, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (void        , glTextureView, (GLuint texture, GLenum target, GLuint origtexture, GLenum internalformat, GLuint minlevel, GLuint numlevels, GLuint minlayer, GLuint numlayers)) \
    X (void        , glBindVertexBuffer, (GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (void        , glVertexAttribFormat, (GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (void        , glVertexAttribIFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexAttribLFormat, (GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexAttribBinding, (GLuint attribindex, GLuint bindingindex)) \
    X (void        , glVertexBindingDivisor, (GLuint bindingindex, GLuint divisor)) \
    X (void        , glDebugMessageControl, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (void        , glDebugMessageInsert, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (void        , glDebugMessageCallback, (GLDEBUGPROC callback, const void *userParam)) \
    X (GLuint      , glGetDebugMessageLog, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog)) \
    X (void        , glPushDebugGroup, (GLenum source, GLuint id, GLsizei length, const GLchar *message)) \
    X (void        , glPopDebugGroup, ()) \
    X (void        , glObjectLabel, (GLenum identifier, GLuint name, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectLabel, (GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label)) \
    X (void        , glObjectPtrLabel, (const void *ptr, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectPtrLabel, (const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_4 \
    X (void        , glBufferStorage, (GLenum target, GLsizeiptr size, const void *data, GLbitfield flags)) \
    X (void        , glClearTexImage, (GLuint texture, GLint level, GLenum format, GLenum type, const void *data)) \
    X (void        , glClearTexSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)) \
    X (void        , glBindBuffersBase, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers)) \
    X (void        , glBindBuffersRange, (GLenum target, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizeiptr *sizes)) \
    X (void        , glBindTextures, (GLuint first, GLsizei count, const GLuint *textures)) \
    X (void        , glBindSamplers, (GLuint first, GLsizei count, const GLuint *samplers)) \
    X (void        , glBindImageTextures, (GLuint first, GLsizei count, const GLuint *textures)) \
    X (void        , glBindVertexBuffers, (GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides))

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_5 \
    X (void        , glClipControl, (GLenum origin, GLenum depth)) \
    X (void        , glCreateTransformFeedbacks, (GLsizei n, GLuint *ids)) \
    X (void        , glTransformFeedbackBufferBase, (GLuint xfb, GLuint index, GLuint buffer)) \
    X (void        , glTransformFeedbackBufferRange, (GLuint xfb, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glGetTransformFeedbackiv, (GLuint xfb, GLenum pname, GLint *param)) \
    X (void        , glGetTransformFeedbacki_v, (GLuint xfb, GLenum pname, GLuint index, GLint *param)) \
    X (void        , glGetTransformFeedbacki64_v, (GLuint xfb, GLenum pname, GLuint index, GLint64 *param)) \
    X (void        , glCreateBuffers, (GLsizei n, GLuint *buffers)) \
    X (void        , glNamedBufferStorage, (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags)) \
    X (void        , glNamedBufferData, (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage)) \
    X (void        , glNamedBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (void        , glCopyNamedBufferSubData, (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (void        , glClearNamedBufferData, (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data)) \
    X (void        , glClearNamedBufferSubData, (GLuint buffer, GLenum internalformat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)) \
    X (void *      , glMapNamedBuffer, (GLuint buffer, GLenum access)) \
    X (void *      , glMapNamedBufferRange, (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (GLboolean   , glUnmapNamedBuffer, (GLuint buffer)) \
    X (void        , glFlushMappedNamedBufferRange, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (void        , glGetNamedBufferParameteriv, (GLuint buffer, GLenum pname, GLint *params)) \
    X (void        , glGetNamedBufferParameteri64v, (GLuint buffer, GLenum pname, GLint64 *params)) \
    X (void        , glGetNamedBufferPointerv, (GLuint buffer, GLenum pname, void **params)) \
    X (void        , glGetNamedBufferSubData, (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data)) \
    X (void        , glCreateFramebuffers, (GLsizei n, GLuint *framebuffers)) \
    X (void        , glNamedFramebufferRenderbuffer, (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (void        , glNamedFramebufferParameteri, (GLuint framebuffer, GLenum pname, GLint param)) \
    X (void        , glNamedFramebufferTexture, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glNamedFramebufferTextureLayer, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (void        , glNamedFramebufferDrawBuffer, (GLuint framebuffer, GLenum buf)) \
    X (void        , glNamedFramebufferDrawBuffers, (GLuint framebuffer, GLsizei n, const GLenum *bufs)) \
    X (void        , glNamedFramebufferReadBuffer, (GLuint framebuffer, GLenum src)) \
    X (void        , glInvalidateNamedFramebufferData, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments)) \
    X (void        , glInvalidateNamedFramebufferSubData, (GLuint framebuffer, GLsizei numAttachments, const GLenum *attachments, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glClearNamedFramebufferiv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLint *value)) \
    X (void        , glClearNamedFramebufferuiv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLuint *value)) \
    X (void        , glClearNamedFramebufferfv, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, const GLfloat *value)) \
    X (void        , glClearNamedFramebufferfi, (GLuint framebuffer, GLenum buffer, GLint drawbuffer, GLfloat depth, GLint stencil)) \
    X (void        , glBlitNamedFramebuffer, (GLuint readFramebuffer, GLuint drawFramebuffer, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (GLenum      , glCheckNamedFramebufferStatus, (GLuint framebuffer, GLenum target)) \
    X (void        , glGetNamedFramebufferParameteriv, (GLuint framebuffer, GLenum pname, GLint *param)) \
    X (void        , glGetNamedFramebufferAttachmentParameteriv, (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)) \
    X (void        , glCreateRenderbuffers, (GLsizei n, GLuint *renderbuffers)) \
    X (void        , glNamedRenderbufferStorage, (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glNamedRenderbufferStorageMultisample, (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glGetNamedRenderbufferParameteriv, (GLuint renderbuffer, GLenum pname, GLint *params)) \
    X (void        , glCreateTextures, (GLenum target, GLsizei n, GLuint *textures)) \
    X (void        , glTextureBuffer, (GLuint texture, GLenum internalformat, GLuint buffer)) \
    X (void        , glTextureBufferRange, (GLuint texture, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glTextureStorage1D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTextureStorage2D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTextureStorage3D, (GLuint texture, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glTextureStorage2DMultisample, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (void        , glTextureStorage3DMultisample, (GLuint texture, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (void        , glTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCompressedTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCopyTextureSubImage1D, (GLuint texture, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyTextureSubImage2D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glCopyTextureSubImage3D, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glTextureParameterf, (GLuint texture, GLenum pname, GLfloat param)) \
    X (void        , glTextureParameterfv, (GLuint texture, GLenum pname, const GLfloat *param)) \
    X (void        , glTextureParameteri, (GLuint texture, GLenum pname, GLint param)) \
    X (void        , glTextureParameterIiv, (GLuint texture, GLenum pname, const GLint *params)) \
    X (void        , glTextureParameterIuiv, (GLuint texture, GLenum pname, const GLuint *params)) \
    X (void        , glTextureParameteriv, (GLuint texture, GLenum pname, const GLint *param)) \
    X (void        , glGenerateTextureMipmap, (GLuint texture)) \
    X (void        , glBindTextureUnit, (GLuint unit, GLuint texture)) \
    X (void        , glGetTextureImage, (GLuint texture, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels)) \
    X (void        , glGetCompressedTextureImage, (GLuint texture, GLint level, GLsizei bufSize, void *pixels)) \
    X (void        , glGetTextureLevelParameterfv, (GLuint texture, GLint level, GLenum pname, GLfloat *params)) \
    X (void        , glGetTextureLevelParameteriv, (GLuint texture, GLint level, GLenum pname, GLint *params)) \
    X (void        , glGetTextureParameterfv, (GLuint texture, GLenum pname, GLfloat *params)) \
    X (void        , glGetTextureParameterIiv, (GLuint texture, GLenum pname, GLint *params)) \
    X (void        , glGetTextureParameterIuiv, (GLuint texture, GLenum pname, GLuint *params)) \
    X (void        , glGetTextureParameteriv, (GLuint texture, GLenum pname, GLint *params)) \
    X (void        , glCreateVertexArrays, (GLsizei n, GLuint *arrays)) \
    X (void        , glDisableVertexArrayAttrib, (GLuint vaobj, GLuint index)) \
    X (void        , glEnableVertexArrayAttrib, (GLuint vaobj, GLuint index)) \
    X (void        , glVertexArrayElementBuffer, (GLuint vaobj, GLuint buffer)) \
    X (void        , glVertexArrayVertexBuffer, (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (void        , glVertexArrayVertexBuffers, (GLuint vaobj, GLuint first, GLsizei count, const GLuint *buffers, const GLintptr *offsets, const GLsizei *strides)) \
    X (void        , glVertexArrayAttribBinding, (GLuint vaobj, GLuint attribindex, GLuint bindingindex)) \
    X (void        , glVertexArrayAttribFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (void        , glVertexArrayAttribIFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexArrayAttribLFormat, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexArrayBindingDivisor, (GLuint vaobj, GLuint bindingindex, GLuint divisor)) \
    X (void        , glGetVertexArrayiv, (GLuint vaobj, GLenum pname, GLint *param)) \
    X (void        , glGetVertexArrayIndexediv, (GLuint vaobj, GLuint index, GLenum pname, GLint *param)) \
    X (void        , glGetVertexArrayIndexed64iv, (GLuint vaobj, GLuint index, GLenum pname, GLint64 *param)) \
    X (void        , glCreateSamplers, (GLsizei n, GLuint *samplers)) \
    X (void        , glCreateProgramPipelines, (GLsizei n, GLuint *pipelines)) \
    X (void        , glCreateQueries, (GLenum target, GLsizei n, GLuint *ids)) \
    X (void        , glGetQueryBufferObjecti64v, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (void        , glGetQueryBufferObjectiv, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (void        , glGetQueryBufferObjectui64v, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (void        , glGetQueryBufferObjectuiv, (GLuint id, GLuint buffer, GLenum pname, GLintptr offset)) \
    X (void        , glMemoryBarrierByRegion, (GLbitfield barriers)) \
    X (void        , glGetTextureSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLsizei bufSize, void *pixels)) \
    X (void        , glGetCompressedTextureSubImage, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei bufSize, void *pixels)) \
    X (GLenum      , glGetGraphicsResetStatus, ()) \
    X (void        , glGetnCompressedTexImage, (GLenum target, GLint lod, GLsizei bufSize, void *pixels)) \
    X (void        , glGetnTexImage, (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *pixels)) \
    X (void        , glGetnUniformdv, (GLuint program, GLint location, GLsizei bufSize, GLdouble *params)) \
    X (void        , glGetnUniformfv, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (void        , glGetnUniformiv, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (void        , glGetnUniformuiv, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (void        , glReadnPixels, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)) \
    X (void        , glGetnMapdv, (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v)) \
    X (void        , glGetnMapfv, (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v)) \
    X (void        , glGetnMapiv, (GLenum target, GLenum query, GLsizei bufSize, GLint *v)) \
    X (void        , glGetnPixelMapfv, (GLenum map, GLsizei bufSize, GLfloat *values)) \
    X (void        , glGetnPixelMapuiv, (GLenum map, GLsizei bufSize, GLuint *values)) \
    X (void        , glGetnPixelMapusv, (GLenum map, GLsizei bufSize, GLushort *values)) \
    X (void        , glGetnPolygonStipple, (GLsizei bufSize, GLubyte *pattern)) \
    X (void        , glGetnColorTable, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table)) \
    X (void        , glGetnConvolutionFilter, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image)) \
    X (void        , glGetnSeparableFilter, (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span)) \
    X (void        , glGetnHistogram, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values)) \
    X (void        , glGetnMinmax, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values)) \
    X (void        , glTextureBarrier, ())

#define JUCE_GL_FUNCTIONS_GL_VERSION_4_6 \
    X (void        , glSpecializeShader, (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue)) \
    X (void        , glMultiDrawArraysIndirectCount, (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (void        , glMultiDrawElementsIndirectCount, (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (void        , glPolygonOffsetClamp, (GLfloat factor, GLfloat units, GLfloat clamp))

#define JUCE_GL_FUNCTIONS_GL_3DFX_tbuffer \
    X (void        , glTbufferMask3DFX, (GLuint mask))

#define JUCE_GL_FUNCTIONS_GL_AMD_debug_output \
    X (void        , glDebugMessageEnableAMD, (GLenum category, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (void        , glDebugMessageInsertAMD, (GLenum category, GLenum severity, GLuint id, GLsizei length, const GLchar *buf)) \
    X (void        , glDebugMessageCallbackAMD, (GLDEBUGPROCAMD callback, void *userParam)) \
    X (GLuint      , glGetDebugMessageLogAMD, (GLuint count, GLsizei bufSize, GLenum *categories, GLenum *severities, GLuint *ids, GLsizei *lengths, GLchar *message))

#define JUCE_GL_FUNCTIONS_GL_AMD_draw_buffers_blend \
    X (void        , glBlendFuncIndexedAMD, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparateIndexedAMD, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)) \
    X (void        , glBlendEquationIndexedAMD, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparateIndexedAMD, (GLuint buf, GLenum modeRGB, GLenum modeAlpha))

#define JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    X (void        , glRenderbufferStorageMultisampleAdvancedAMD, (GLenum target, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glNamedRenderbufferStorageMultisampleAdvancedAMD, (GLuint renderbuffer, GLsizei samples, GLsizei storageSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_sample_positions \
    X (void        , glFramebufferSamplePositionsfvAMD, (GLenum target, GLuint numsamples, GLuint pixelindex, const GLfloat *values)) \
    X (void        , glNamedFramebufferSamplePositionsfvAMD, (GLuint framebuffer, GLuint numsamples, GLuint pixelindex, const GLfloat *values)) \
    X (void        , glGetFramebufferParameterfvAMD, (GLenum target, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values)) \
    X (void        , glGetNamedFramebufferParameterfvAMD, (GLuint framebuffer, GLenum pname, GLuint numsamples, GLuint pixelindex, GLsizei size, GLfloat *values))

#define JUCE_GL_FUNCTIONS_GL_AMD_gpu_shader_int64 \
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
    X (void        , glGetUniformui64vNV, (GLuint program, GLint location, GLuint64EXT *params)) \
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

#define JUCE_GL_FUNCTIONS_GL_AMD_interleaved_elements \
    X (void        , glVertexAttribParameteriAMD, (GLuint index, GLenum pname, GLint param))

#define JUCE_GL_FUNCTIONS_GL_AMD_multi_draw_indirect \
    X (void        , glMultiDrawArraysIndirectAMD, (GLenum mode, const void *indirect, GLsizei primcount, GLsizei stride)) \
    X (void        , glMultiDrawElementsIndirectAMD, (GLenum mode, GLenum type, const void *indirect, GLsizei primcount, GLsizei stride))

#define JUCE_GL_FUNCTIONS_GL_AMD_name_gen_delete \
    X (void        , glGenNamesAMD, (GLenum identifier, GLuint num, GLuint *names)) \
    X (void        , glDeleteNamesAMD, (GLenum identifier, GLuint num, const GLuint *names)) \
    X (GLboolean   , glIsNameAMD, (GLenum identifier, GLuint name))

#define JUCE_GL_FUNCTIONS_GL_AMD_occlusion_query_event \
    X (void        , glQueryObjectParameteruiAMD, (GLenum target, GLuint id, GLenum pname, GLuint param))

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

#define JUCE_GL_FUNCTIONS_GL_AMD_sample_positions \
    X (void        , glSetMultisamplefvAMD, (GLenum pname, GLuint index, const GLfloat *val))

#define JUCE_GL_FUNCTIONS_GL_AMD_sparse_texture \
    X (void        , glTexStorageSparseAMD, (GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags)) \
    X (void        , glTextureStorageSparseAMD, (GLuint texture, GLenum target, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLsizei layers, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_AMD_stencil_operation_extended \
    X (void        , glStencilOpValueAMD, (GLenum face, GLuint value))

#define JUCE_GL_FUNCTIONS_GL_AMD_vertex_shader_tessellator \
    X (void        , glTessellationFactorAMD, (GLfloat factor)) \
    X (void        , glTessellationModeAMD, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_APPLE_element_array \
    X (void        , glElementPointerAPPLE, (GLenum type, const void *pointer)) \
    X (void        , glDrawElementArrayAPPLE, (GLenum mode, GLint first, GLsizei count)) \
    X (void        , glDrawRangeElementArrayAPPLE, (GLenum mode, GLuint start, GLuint end, GLint first, GLsizei count)) \
    X (void        , glMultiDrawElementArrayAPPLE, (GLenum mode, const GLint *first, const GLsizei *count, GLsizei primcount)) \
    X (void        , glMultiDrawRangeElementArrayAPPLE, (GLenum mode, GLuint start, GLuint end, const GLint *first, const GLsizei *count, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_APPLE_fence \
    X (void        , glGenFencesAPPLE, (GLsizei n, GLuint *fences)) \
    X (void        , glDeleteFencesAPPLE, (GLsizei n, const GLuint *fences)) \
    X (void        , glSetFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glIsFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glTestFenceAPPLE, (GLuint fence)) \
    X (void        , glFinishFenceAPPLE, (GLuint fence)) \
    X (GLboolean   , glTestObjectAPPLE, (GLenum object, GLuint name)) \
    X (void        , glFinishObjectAPPLE, (GLenum object, GLint name))

#define JUCE_GL_FUNCTIONS_GL_APPLE_flush_buffer_range \
    X (void        , glBufferParameteriAPPLE, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glFlushMappedBufferRangeAPPLE, (GLenum target, GLintptr offset, GLsizeiptr size))

#define JUCE_GL_FUNCTIONS_GL_APPLE_object_purgeable \
    X (GLenum      , glObjectPurgeableAPPLE, (GLenum objectType, GLuint name, GLenum option)) \
    X (GLenum      , glObjectUnpurgeableAPPLE, (GLenum objectType, GLuint name, GLenum option)) \
    X (void        , glGetObjectParameterivAPPLE, (GLenum objectType, GLuint name, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_APPLE_texture_range \
    X (void        , glTextureRangeAPPLE, (GLenum target, GLsizei length, const void *pointer)) \
    X (void        , glGetTexParameterPointervAPPLE, (GLenum target, GLenum pname, void **params))

#define JUCE_GL_FUNCTIONS_GL_APPLE_vertex_array_object \
    X (void        , glBindVertexArrayAPPLE, (GLuint array)) \
    X (void        , glDeleteVertexArraysAPPLE, (GLsizei n, const GLuint *arrays)) \
    X (void        , glGenVertexArraysAPPLE, (GLsizei n, GLuint *arrays)) \
    X (GLboolean   , glIsVertexArrayAPPLE, (GLuint array))

#define JUCE_GL_FUNCTIONS_GL_APPLE_vertex_array_range \
    X (void        , glVertexArrayRangeAPPLE, (GLsizei length, void *pointer)) \
    X (void        , glFlushVertexArrayRangeAPPLE, (GLsizei length, void *pointer)) \
    X (void        , glVertexArrayParameteriAPPLE, (GLenum pname, GLint param))

#define JUCE_GL_FUNCTIONS_GL_APPLE_vertex_program_evaluators \
    X (void        , glEnableVertexAttribAPPLE, (GLuint index, GLenum pname)) \
    X (void        , glDisableVertexAttribAPPLE, (GLuint index, GLenum pname)) \
    X (GLboolean   , glIsVertexAttribEnabledAPPLE, (GLuint index, GLenum pname)) \
    X (void        , glMapVertexAttrib1dAPPLE, (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble *points)) \
    X (void        , glMapVertexAttrib1fAPPLE, (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat *points)) \
    X (void        , glMapVertexAttrib2dAPPLE, (GLuint index, GLuint size, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble *points)) \
    X (void        , glMapVertexAttrib2fAPPLE, (GLuint index, GLuint size, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat *points))

#define JUCE_GL_FUNCTIONS_GL_ARB_ES3_2_compatibility \
    X (void        , glPrimitiveBoundingBoxARB, (GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW))

#define JUCE_GL_FUNCTIONS_GL_ARB_bindless_texture \
    X (GLuint64    , glGetTextureHandleARB, (GLuint texture)) \
    X (GLuint64    , glGetTextureSamplerHandleARB, (GLuint texture, GLuint sampler)) \
    X (void        , glMakeTextureHandleResidentARB, (GLuint64 handle)) \
    X (void        , glMakeTextureHandleNonResidentARB, (GLuint64 handle)) \
    X (GLuint64    , glGetImageHandleARB, (GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum format)) \
    X (void        , glMakeImageHandleResidentARB, (GLuint64 handle, GLenum access)) \
    X (void        , glMakeImageHandleNonResidentARB, (GLuint64 handle)) \
    X (void        , glUniformHandleui64ARB, (GLint location, GLuint64 value)) \
    X (void        , glUniformHandleui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniformHandleui64ARB, (GLuint program, GLint location, GLuint64 value)) \
    X (void        , glProgramUniformHandleui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *values)) \
    X (GLboolean   , glIsTextureHandleResidentARB, (GLuint64 handle)) \
    X (GLboolean   , glIsImageHandleResidentARB, (GLuint64 handle)) \
    X (void        , glVertexAttribL1ui64ARB, (GLuint index, GLuint64EXT x)) \
    X (void        , glVertexAttribL1ui64vARB, (GLuint index, const GLuint64EXT *v)) \
    X (void        , glGetVertexAttribLui64vARB, (GLuint index, GLenum pname, GLuint64EXT *params))

#define JUCE_GL_FUNCTIONS_GL_ARB_cl_event \
    X (GLsync      , glCreateSyncFromCLeventARB, (struct _cl_context *context, struct _cl_event *event, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_ARB_color_buffer_float \
    X (void        , glClampColorARB, (GLenum target, GLenum clamp))

#define JUCE_GL_FUNCTIONS_GL_ARB_compute_variable_group_size \
    X (void        , glDispatchComputeGroupSizeARB, (GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z, GLuint group_size_x, GLuint group_size_y, GLuint group_size_z))

#define JUCE_GL_FUNCTIONS_GL_ARB_debug_output \
    X (void        , glDebugMessageControlARB, (GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled)) \
    X (void        , glDebugMessageInsertARB, (GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf)) \
    X (void        , glDebugMessageCallbackARB, (GLDEBUGPROCARB callback, const void *userParam)) \
    X (GLuint      , glGetDebugMessageLogARB, (GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog))

#define JUCE_GL_FUNCTIONS_GL_ARB_draw_buffers \
    X (void        , glDrawBuffersARB, (GLsizei n, const GLenum *bufs))

#define JUCE_GL_FUNCTIONS_GL_ARB_draw_buffers_blend \
    X (void        , glBlendEquationiARB, (GLuint buf, GLenum mode)) \
    X (void        , glBlendEquationSeparateiARB, (GLuint buf, GLenum modeRGB, GLenum modeAlpha)) \
    X (void        , glBlendFunciARB, (GLuint buf, GLenum src, GLenum dst)) \
    X (void        , glBlendFuncSeparateiARB, (GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha))

#define JUCE_GL_FUNCTIONS_GL_ARB_draw_instanced \
    X (void        , glDrawArraysInstancedARB, (GLenum mode, GLint first, GLsizei count, GLsizei primcount)) \
    X (void        , glDrawElementsInstancedARB, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_ARB_fragment_program \
    X (void        , glProgramStringARB, (GLenum target, GLenum format, GLsizei len, const void *string)) \
    X (void        , glBindProgramARB, (GLenum target, GLuint program)) \
    X (void        , glDeleteProgramsARB, (GLsizei n, const GLuint *programs)) \
    X (void        , glGenProgramsARB, (GLsizei n, GLuint *programs)) \
    X (void        , glProgramEnvParameter4dARB, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glProgramEnvParameter4dvARB, (GLenum target, GLuint index, const GLdouble *params)) \
    X (void        , glProgramEnvParameter4fARB, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glProgramEnvParameter4fvARB, (GLenum target, GLuint index, const GLfloat *params)) \
    X (void        , glProgramLocalParameter4dARB, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glProgramLocalParameter4dvARB, (GLenum target, GLuint index, const GLdouble *params)) \
    X (void        , glProgramLocalParameter4fARB, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glProgramLocalParameter4fvARB, (GLenum target, GLuint index, const GLfloat *params)) \
    X (void        , glGetProgramEnvParameterdvARB, (GLenum target, GLuint index, GLdouble *params)) \
    X (void        , glGetProgramEnvParameterfvARB, (GLenum target, GLuint index, GLfloat *params)) \
    X (void        , glGetProgramLocalParameterdvARB, (GLenum target, GLuint index, GLdouble *params)) \
    X (void        , glGetProgramLocalParameterfvARB, (GLenum target, GLuint index, GLfloat *params)) \
    X (void        , glGetProgramivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetProgramStringARB, (GLenum target, GLenum pname, void *string)) \
    X (GLboolean   , glIsProgramARB, (GLuint program))

#define JUCE_GL_FUNCTIONS_GL_ARB_geometry_shader4 \
    X (void        , glProgramParameteriARB, (GLuint program, GLenum pname, GLint value)) \
    X (void        , glFramebufferTextureARB, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glFramebufferTextureLayerARB, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (void        , glFramebufferTextureFaceARB, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face))

#define JUCE_GL_FUNCTIONS_GL_ARB_gl_spirv \
    X (void        , glSpecializeShaderARB, (GLuint shader, const GLchar *pEntryPoint, GLuint numSpecializationConstants, const GLuint *pConstantIndex, const GLuint *pConstantValue))

#define JUCE_GL_FUNCTIONS_GL_ARB_gpu_shader_int64 \
    X (void        , glUniform1i64ARB, (GLint location, GLint64 x)) \
    X (void        , glUniform2i64ARB, (GLint location, GLint64 x, GLint64 y)) \
    X (void        , glUniform3i64ARB, (GLint location, GLint64 x, GLint64 y, GLint64 z)) \
    X (void        , glUniform4i64ARB, (GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)) \
    X (void        , glUniform1i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glUniform2i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glUniform3i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glUniform4i64vARB, (GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glUniform1ui64ARB, (GLint location, GLuint64 x)) \
    X (void        , glUniform2ui64ARB, (GLint location, GLuint64 x, GLuint64 y)) \
    X (void        , glUniform3ui64ARB, (GLint location, GLuint64 x, GLuint64 y, GLuint64 z)) \
    X (void        , glUniform4ui64ARB, (GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)) \
    X (void        , glUniform1ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glUniform2ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glUniform3ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glUniform4ui64vARB, (GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glGetUniformi64vARB, (GLuint program, GLint location, GLint64 *params)) \
    X (void        , glGetUniformui64vARB, (GLuint program, GLint location, GLuint64 *params)) \
    X (void        , glGetnUniformi64vARB, (GLuint program, GLint location, GLsizei bufSize, GLint64 *params)) \
    X (void        , glGetnUniformui64vARB, (GLuint program, GLint location, GLsizei bufSize, GLuint64 *params)) \
    X (void        , glProgramUniform1i64ARB, (GLuint program, GLint location, GLint64 x)) \
    X (void        , glProgramUniform2i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y)) \
    X (void        , glProgramUniform3i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z)) \
    X (void        , glProgramUniform4i64ARB, (GLuint program, GLint location, GLint64 x, GLint64 y, GLint64 z, GLint64 w)) \
    X (void        , glProgramUniform1i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glProgramUniform2i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glProgramUniform3i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glProgramUniform4i64vARB, (GLuint program, GLint location, GLsizei count, const GLint64 *value)) \
    X (void        , glProgramUniform1ui64ARB, (GLuint program, GLint location, GLuint64 x)) \
    X (void        , glProgramUniform2ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y)) \
    X (void        , glProgramUniform3ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z)) \
    X (void        , glProgramUniform4ui64ARB, (GLuint program, GLint location, GLuint64 x, GLuint64 y, GLuint64 z, GLuint64 w)) \
    X (void        , glProgramUniform1ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniform2ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniform3ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value)) \
    X (void        , glProgramUniform4ui64vARB, (GLuint program, GLint location, GLsizei count, const GLuint64 *value))

#define JUCE_GL_FUNCTIONS_GL_ARB_imaging \
    X (void        , glColorTable, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table)) \
    X (void        , glColorTableParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glColorTableParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glCopyColorTable, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (void        , glGetColorTable, (GLenum target, GLenum format, GLenum type, void *table)) \
    X (void        , glGetColorTableParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetColorTableParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glColorSubTable, (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data)) \
    X (void        , glCopyColorSubTable, (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width)) \
    X (void        , glConvolutionFilter1D, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image)) \
    X (void        , glConvolutionFilter2D, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image)) \
    X (void        , glConvolutionParameterf, (GLenum target, GLenum pname, GLfloat params)) \
    X (void        , glConvolutionParameterfv, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glConvolutionParameteri, (GLenum target, GLenum pname, GLint params)) \
    X (void        , glConvolutionParameteriv, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glCopyConvolutionFilter1D, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyConvolutionFilter2D, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glGetConvolutionFilter, (GLenum target, GLenum format, GLenum type, void *image)) \
    X (void        , glGetConvolutionParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetConvolutionParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetSeparableFilter, (GLenum target, GLenum format, GLenum type, void *row, void *column, void *span)) \
    X (void        , glSeparableFilter2D, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column)) \
    X (void        , glGetHistogram, (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)) \
    X (void        , glGetHistogramParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetHistogramParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMinmax, (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)) \
    X (void        , glGetMinmaxParameterfv, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetMinmaxParameteriv, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glHistogram, (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)) \
    X (void        , glMinmax, (GLenum target, GLenum internalformat, GLboolean sink)) \
    X (void        , glResetHistogram, (GLenum target)) \
    X (void        , glResetMinmax, (GLenum target))

#define JUCE_GL_FUNCTIONS_GL_ARB_indirect_parameters \
    X (void        , glMultiDrawArraysIndirectCountARB, (GLenum mode, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride)) \
    X (void        , glMultiDrawElementsIndirectCountARB, (GLenum mode, GLenum type, const void *indirect, GLintptr drawcount, GLsizei maxdrawcount, GLsizei stride))

#define JUCE_GL_FUNCTIONS_GL_ARB_instanced_arrays \
    X (void        , glVertexAttribDivisorARB, (GLuint index, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_ARB_matrix_palette \
    X (void        , glCurrentPaletteMatrixARB, (GLint index)) \
    X (void        , glMatrixIndexubvARB, (GLint size, const GLubyte *indices)) \
    X (void        , glMatrixIndexusvARB, (GLint size, const GLushort *indices)) \
    X (void        , glMatrixIndexuivARB, (GLint size, const GLuint *indices)) \
    X (void        , glMatrixIndexPointerARB, (GLint size, GLenum type, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_ARB_multisample \
    X (void        , glSampleCoverageARB, (GLfloat value, GLboolean invert))

#define JUCE_GL_FUNCTIONS_GL_ARB_multitexture \
    X (void        , glActiveTextureARB, (GLenum texture)) \
    X (void        , glClientActiveTextureARB, (GLenum texture)) \
    X (void        , glMultiTexCoord1dARB, (GLenum target, GLdouble s)) \
    X (void        , glMultiTexCoord1dvARB, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord1fARB, (GLenum target, GLfloat s)) \
    X (void        , glMultiTexCoord1fvARB, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord1iARB, (GLenum target, GLint s)) \
    X (void        , glMultiTexCoord1ivARB, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord1sARB, (GLenum target, GLshort s)) \
    X (void        , glMultiTexCoord1svARB, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord2dARB, (GLenum target, GLdouble s, GLdouble t)) \
    X (void        , glMultiTexCoord2dvARB, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord2fARB, (GLenum target, GLfloat s, GLfloat t)) \
    X (void        , glMultiTexCoord2fvARB, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord2iARB, (GLenum target, GLint s, GLint t)) \
    X (void        , glMultiTexCoord2ivARB, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord2sARB, (GLenum target, GLshort s, GLshort t)) \
    X (void        , glMultiTexCoord2svARB, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord3dARB, (GLenum target, GLdouble s, GLdouble t, GLdouble r)) \
    X (void        , glMultiTexCoord3dvARB, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord3fARB, (GLenum target, GLfloat s, GLfloat t, GLfloat r)) \
    X (void        , glMultiTexCoord3fvARB, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord3iARB, (GLenum target, GLint s, GLint t, GLint r)) \
    X (void        , glMultiTexCoord3ivARB, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord3sARB, (GLenum target, GLshort s, GLshort t, GLshort r)) \
    X (void        , glMultiTexCoord3svARB, (GLenum target, const GLshort *v)) \
    X (void        , glMultiTexCoord4dARB, (GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q)) \
    X (void        , glMultiTexCoord4dvARB, (GLenum target, const GLdouble *v)) \
    X (void        , glMultiTexCoord4fARB, (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)) \
    X (void        , glMultiTexCoord4fvARB, (GLenum target, const GLfloat *v)) \
    X (void        , glMultiTexCoord4iARB, (GLenum target, GLint s, GLint t, GLint r, GLint q)) \
    X (void        , glMultiTexCoord4ivARB, (GLenum target, const GLint *v)) \
    X (void        , glMultiTexCoord4sARB, (GLenum target, GLshort s, GLshort t, GLshort r, GLshort q)) \
    X (void        , glMultiTexCoord4svARB, (GLenum target, const GLshort *v))

#define JUCE_GL_FUNCTIONS_GL_ARB_occlusion_query \
    X (void        , glGenQueriesARB, (GLsizei n, GLuint *ids)) \
    X (void        , glDeleteQueriesARB, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsQueryARB, (GLuint id)) \
    X (void        , glBeginQueryARB, (GLenum target, GLuint id)) \
    X (void        , glEndQueryARB, (GLenum target)) \
    X (void        , glGetQueryivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectivARB, (GLuint id, GLenum pname, GLint *params)) \
    X (void        , glGetQueryObjectuivARB, (GLuint id, GLenum pname, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_ARB_parallel_shader_compile \
    X (void        , glMaxShaderCompilerThreadsARB, (GLuint count))

#define JUCE_GL_FUNCTIONS_GL_ARB_point_parameters \
    X (void        , glPointParameterfARB, (GLenum pname, GLfloat param)) \
    X (void        , glPointParameterfvARB, (GLenum pname, const GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_ARB_robustness \
    X (GLenum      , glGetGraphicsResetStatusARB, ()) \
    X (void        , glGetnTexImageARB, (GLenum target, GLint level, GLenum format, GLenum type, GLsizei bufSize, void *img)) \
    X (void        , glReadnPixelsARB, (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data)) \
    X (void        , glGetnCompressedTexImageARB, (GLenum target, GLint lod, GLsizei bufSize, void *img)) \
    X (void        , glGetnUniformfvARB, (GLuint program, GLint location, GLsizei bufSize, GLfloat *params)) \
    X (void        , glGetnUniformivARB, (GLuint program, GLint location, GLsizei bufSize, GLint *params)) \
    X (void        , glGetnUniformuivARB, (GLuint program, GLint location, GLsizei bufSize, GLuint *params)) \
    X (void        , glGetnUniformdvARB, (GLuint program, GLint location, GLsizei bufSize, GLdouble *params)) \
    X (void        , glGetnMapdvARB, (GLenum target, GLenum query, GLsizei bufSize, GLdouble *v)) \
    X (void        , glGetnMapfvARB, (GLenum target, GLenum query, GLsizei bufSize, GLfloat *v)) \
    X (void        , glGetnMapivARB, (GLenum target, GLenum query, GLsizei bufSize, GLint *v)) \
    X (void        , glGetnPixelMapfvARB, (GLenum map, GLsizei bufSize, GLfloat *values)) \
    X (void        , glGetnPixelMapuivARB, (GLenum map, GLsizei bufSize, GLuint *values)) \
    X (void        , glGetnPixelMapusvARB, (GLenum map, GLsizei bufSize, GLushort *values)) \
    X (void        , glGetnPolygonStippleARB, (GLsizei bufSize, GLubyte *pattern)) \
    X (void        , glGetnColorTableARB, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *table)) \
    X (void        , glGetnConvolutionFilterARB, (GLenum target, GLenum format, GLenum type, GLsizei bufSize, void *image)) \
    X (void        , glGetnSeparableFilterARB, (GLenum target, GLenum format, GLenum type, GLsizei rowBufSize, void *row, GLsizei columnBufSize, void *column, void *span)) \
    X (void        , glGetnHistogramARB, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values)) \
    X (void        , glGetnMinmaxARB, (GLenum target, GLboolean reset, GLenum format, GLenum type, GLsizei bufSize, void *values))

#define JUCE_GL_FUNCTIONS_GL_ARB_sample_locations \
    X (void        , glFramebufferSampleLocationsfvARB, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glNamedFramebufferSampleLocationsfvARB, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glEvaluateDepthValuesARB, ())

#define JUCE_GL_FUNCTIONS_GL_ARB_sample_shading \
    X (void        , glMinSampleShadingARB, (GLfloat value))

#define JUCE_GL_FUNCTIONS_GL_ARB_shader_objects \
    X (void        , glDeleteObjectARB, (GLhandleARB obj)) \
    X (GLhandleARB , glGetHandleARB, (GLenum pname)) \
    X (void        , glDetachObjectARB, (GLhandleARB containerObj, GLhandleARB attachedObj)) \
    X (GLhandleARB , glCreateShaderObjectARB, (GLenum shaderType)) \
    X (void        , glShaderSourceARB, (GLhandleARB shaderObj, GLsizei count, const GLcharARB **string, const GLint *length)) \
    X (void        , glCompileShaderARB, (GLhandleARB shaderObj)) \
    X (GLhandleARB , glCreateProgramObjectARB, ()) \
    X (void        , glAttachObjectARB, (GLhandleARB containerObj, GLhandleARB obj)) \
    X (void        , glLinkProgramARB, (GLhandleARB programObj)) \
    X (void        , glUseProgramObjectARB, (GLhandleARB programObj)) \
    X (void        , glValidateProgramARB, (GLhandleARB programObj)) \
    X (void        , glUniform1fARB, (GLint location, GLfloat v0)) \
    X (void        , glUniform2fARB, (GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glUniform3fARB, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glUniform4fARB, (GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glUniform1iARB, (GLint location, GLint v0)) \
    X (void        , glUniform2iARB, (GLint location, GLint v0, GLint v1)) \
    X (void        , glUniform3iARB, (GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glUniform4iARB, (GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glUniform1fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform2fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform3fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform4fvARB, (GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glUniform1ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform2ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform3ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniform4ivARB, (GLint location, GLsizei count, const GLint *value)) \
    X (void        , glUniformMatrix2fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix3fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glUniformMatrix4fvARB, (GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glGetObjectParameterfvARB, (GLhandleARB obj, GLenum pname, GLfloat *params)) \
    X (void        , glGetObjectParameterivARB, (GLhandleARB obj, GLenum pname, GLint *params)) \
    X (void        , glGetInfoLogARB, (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *infoLog)) \
    X (void        , glGetAttachedObjectsARB, (GLhandleARB containerObj, GLsizei maxCount, GLsizei *count, GLhandleARB *obj)) \
    X (GLint       , glGetUniformLocationARB, (GLhandleARB programObj, const GLcharARB *name)) \
    X (void        , glGetActiveUniformARB, (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)) \
    X (void        , glGetUniformfvARB, (GLhandleARB programObj, GLint location, GLfloat *params)) \
    X (void        , glGetUniformivARB, (GLhandleARB programObj, GLint location, GLint *params)) \
    X (void        , glGetShaderSourceARB, (GLhandleARB obj, GLsizei maxLength, GLsizei *length, GLcharARB *source))

#define JUCE_GL_FUNCTIONS_GL_ARB_shading_language_include \
    X (void        , glNamedStringARB, (GLenum type, GLint namelen, const GLchar *name, GLint stringlen, const GLchar *string)) \
    X (void        , glDeleteNamedStringARB, (GLint namelen, const GLchar *name)) \
    X (void        , glCompileShaderIncludeARB, (GLuint shader, GLsizei count, const GLchar *const*path, const GLint *length)) \
    X (GLboolean   , glIsNamedStringARB, (GLint namelen, const GLchar *name)) \
    X (void        , glGetNamedStringARB, (GLint namelen, const GLchar *name, GLsizei bufSize, GLint *stringlen, GLchar *string)) \
    X (void        , glGetNamedStringivARB, (GLint namelen, const GLchar *name, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_ARB_sparse_buffer \
    X (void        , glBufferPageCommitmentARB, (GLenum target, GLintptr offset, GLsizeiptr size, GLboolean commit)) \
    X (void        , glNamedBufferPageCommitmentEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit)) \
    X (void        , glNamedBufferPageCommitmentARB, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLboolean commit))

#define JUCE_GL_FUNCTIONS_GL_ARB_sparse_texture \
    X (void        , glTexPageCommitmentARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit))

#define JUCE_GL_FUNCTIONS_GL_ARB_texture_buffer_object \
    X (void        , glTexBufferARB, (GLenum target, GLenum internalformat, GLuint buffer))

#define JUCE_GL_FUNCTIONS_GL_ARB_texture_compression \
    X (void        , glCompressedTexImage3DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexImage2DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexImage1DARB, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage3DARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage2DARB, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glCompressedTexSubImage1DARB, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *data)) \
    X (void        , glGetCompressedTexImageARB, (GLenum target, GLint level, void *img))

#define JUCE_GL_FUNCTIONS_GL_ARB_transpose_matrix \
    X (void        , glLoadTransposeMatrixfARB, (const GLfloat *m)) \
    X (void        , glLoadTransposeMatrixdARB, (const GLdouble *m)) \
    X (void        , glMultTransposeMatrixfARB, (const GLfloat *m)) \
    X (void        , glMultTransposeMatrixdARB, (const GLdouble *m))

#define JUCE_GL_FUNCTIONS_GL_ARB_vertex_blend \
    X (void        , glWeightbvARB, (GLint size, const GLbyte *weights)) \
    X (void        , glWeightsvARB, (GLint size, const GLshort *weights)) \
    X (void        , glWeightivARB, (GLint size, const GLint *weights)) \
    X (void        , glWeightfvARB, (GLint size, const GLfloat *weights)) \
    X (void        , glWeightdvARB, (GLint size, const GLdouble *weights)) \
    X (void        , glWeightubvARB, (GLint size, const GLubyte *weights)) \
    X (void        , glWeightusvARB, (GLint size, const GLushort *weights)) \
    X (void        , glWeightuivARB, (GLint size, const GLuint *weights)) \
    X (void        , glWeightPointerARB, (GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glVertexBlendARB, (GLint count))

#define JUCE_GL_FUNCTIONS_GL_ARB_vertex_buffer_object \
    X (void        , glBindBufferARB, (GLenum target, GLuint buffer)) \
    X (void        , glDeleteBuffersARB, (GLsizei n, const GLuint *buffers)) \
    X (void        , glGenBuffersARB, (GLsizei n, GLuint *buffers)) \
    X (GLboolean   , glIsBufferARB, (GLuint buffer)) \
    X (void        , glBufferDataARB, (GLenum target, GLsizeiptrARB size, const void *data, GLenum usage)) \
    X (void        , glBufferSubDataARB, (GLenum target, GLintptrARB offset, GLsizeiptrARB size, const void *data)) \
    X (void        , glGetBufferSubDataARB, (GLenum target, GLintptrARB offset, GLsizeiptrARB size, void *data)) \
    X (void *      , glMapBufferARB, (GLenum target, GLenum access)) \
    X (GLboolean   , glUnmapBufferARB, (GLenum target)) \
    X (void        , glGetBufferParameterivARB, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetBufferPointervARB, (GLenum target, GLenum pname, void **params))

#define JUCE_GL_FUNCTIONS_GL_ARB_vertex_program \
    X (void        , glVertexAttrib1dARB, (GLuint index, GLdouble x)) \
    X (void        , glVertexAttrib1dvARB, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib1fARB, (GLuint index, GLfloat x)) \
    X (void        , glVertexAttrib1fvARB, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib1sARB, (GLuint index, GLshort x)) \
    X (void        , glVertexAttrib1svARB, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib2dARB, (GLuint index, GLdouble x, GLdouble y)) \
    X (void        , glVertexAttrib2dvARB, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib2fARB, (GLuint index, GLfloat x, GLfloat y)) \
    X (void        , glVertexAttrib2fvARB, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib2sARB, (GLuint index, GLshort x, GLshort y)) \
    X (void        , glVertexAttrib2svARB, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib3dARB, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexAttrib3dvARB, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib3fARB, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertexAttrib3fvARB, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib3sARB, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (void        , glVertexAttrib3svARB, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4NbvARB, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttrib4NivARB, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttrib4NsvARB, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4NubARB, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (void        , glVertexAttrib4NubvARB, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttrib4NuivARB, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttrib4NusvARB, (GLuint index, const GLushort *v)) \
    X (void        , glVertexAttrib4bvARB, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttrib4dARB, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexAttrib4dvARB, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib4fARB, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertexAttrib4fvARB, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib4ivARB, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttrib4sARB, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glVertexAttrib4svARB, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4ubvARB, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttrib4uivARB, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttrib4usvARB, (GLuint index, const GLushort *v)) \
    X (void        , glVertexAttribPointerARB, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer)) \
    X (void        , glEnableVertexAttribArrayARB, (GLuint index)) \
    X (void        , glDisableVertexAttribArrayARB, (GLuint index)) \
    X (void        , glGetVertexAttribdvARB, (GLuint index, GLenum pname, GLdouble *params)) \
    X (void        , glGetVertexAttribfvARB, (GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetVertexAttribivARB, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribPointervARB, (GLuint index, GLenum pname, void **pointer))

#define JUCE_GL_FUNCTIONS_GL_ARB_vertex_shader \
    X (void        , glBindAttribLocationARB, (GLhandleARB programObj, GLuint index, const GLcharARB *name)) \
    X (void        , glGetActiveAttribARB, (GLhandleARB programObj, GLuint index, GLsizei maxLength, GLsizei *length, GLint *size, GLenum *type, GLcharARB *name)) \
    X (GLint       , glGetAttribLocationARB, (GLhandleARB programObj, const GLcharARB *name))

#define JUCE_GL_FUNCTIONS_GL_ARB_viewport_array \
    X (void        , glDepthRangeArraydvNV, (GLuint first, GLsizei count, const GLdouble *v)) \
    X (void        , glDepthRangeIndexeddNV, (GLuint index, GLdouble n, GLdouble f))

#define JUCE_GL_FUNCTIONS_GL_ARB_window_pos \
    X (void        , glWindowPos2dARB, (GLdouble x, GLdouble y)) \
    X (void        , glWindowPos2dvARB, (const GLdouble *v)) \
    X (void        , glWindowPos2fARB, (GLfloat x, GLfloat y)) \
    X (void        , glWindowPos2fvARB, (const GLfloat *v)) \
    X (void        , glWindowPos2iARB, (GLint x, GLint y)) \
    X (void        , glWindowPos2ivARB, (const GLint *v)) \
    X (void        , glWindowPos2sARB, (GLshort x, GLshort y)) \
    X (void        , glWindowPos2svARB, (const GLshort *v)) \
    X (void        , glWindowPos3dARB, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glWindowPos3dvARB, (const GLdouble *v)) \
    X (void        , glWindowPos3fARB, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glWindowPos3fvARB, (const GLfloat *v)) \
    X (void        , glWindowPos3iARB, (GLint x, GLint y, GLint z)) \
    X (void        , glWindowPos3ivARB, (const GLint *v)) \
    X (void        , glWindowPos3sARB, (GLshort x, GLshort y, GLshort z)) \
    X (void        , glWindowPos3svARB, (const GLshort *v))

#define JUCE_GL_FUNCTIONS_GL_ATI_draw_buffers \
    X (void        , glDrawBuffersATI, (GLsizei n, const GLenum *bufs))

#define JUCE_GL_FUNCTIONS_GL_ATI_element_array \
    X (void        , glElementPointerATI, (GLenum type, const void *pointer)) \
    X (void        , glDrawElementArrayATI, (GLenum mode, GLsizei count)) \
    X (void        , glDrawRangeElementArrayATI, (GLenum mode, GLuint start, GLuint end, GLsizei count))

#define JUCE_GL_FUNCTIONS_GL_ATI_envmap_bumpmap \
    X (void        , glTexBumpParameterivATI, (GLenum pname, const GLint *param)) \
    X (void        , glTexBumpParameterfvATI, (GLenum pname, const GLfloat *param)) \
    X (void        , glGetTexBumpParameterivATI, (GLenum pname, GLint *param)) \
    X (void        , glGetTexBumpParameterfvATI, (GLenum pname, GLfloat *param))

#define JUCE_GL_FUNCTIONS_GL_ATI_fragment_shader \
    X (GLuint      , glGenFragmentShadersATI, (GLuint range)) \
    X (void        , glBindFragmentShaderATI, (GLuint id)) \
    X (void        , glDeleteFragmentShaderATI, (GLuint id)) \
    X (void        , glBeginFragmentShaderATI, ()) \
    X (void        , glEndFragmentShaderATI, ()) \
    X (void        , glPassTexCoordATI, (GLuint dst, GLuint coord, GLenum swizzle)) \
    X (void        , glSampleMapATI, (GLuint dst, GLuint interp, GLenum swizzle)) \
    X (void        , glColorFragmentOp1ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)) \
    X (void        , glColorFragmentOp2ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)) \
    X (void        , glColorFragmentOp3ATI, (GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)) \
    X (void        , glAlphaFragmentOp1ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod)) \
    X (void        , glAlphaFragmentOp2ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod)) \
    X (void        , glAlphaFragmentOp3ATI, (GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod)) \
    X (void        , glSetFragmentShaderConstantATI, (GLuint dst, const GLfloat *value))

#define JUCE_GL_FUNCTIONS_GL_ATI_map_object_buffer \
    X (void *      , glMapObjectBufferATI, (GLuint buffer)) \
    X (void        , glUnmapObjectBufferATI, (GLuint buffer))

#define JUCE_GL_FUNCTIONS_GL_ATI_pn_triangles \
    X (void        , glPNTrianglesiATI, (GLenum pname, GLint param)) \
    X (void        , glPNTrianglesfATI, (GLenum pname, GLfloat param))

#define JUCE_GL_FUNCTIONS_GL_ATI_separate_stencil \
    X (void        , glStencilOpSeparateATI, (GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass)) \
    X (void        , glStencilFuncSeparateATI, (GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask))

#define JUCE_GL_FUNCTIONS_GL_ATI_vertex_array_object \
    X (GLuint      , glNewObjectBufferATI, (GLsizei size, const void *pointer, GLenum usage)) \
    X (GLboolean   , glIsObjectBufferATI, (GLuint buffer)) \
    X (void        , glUpdateObjectBufferATI, (GLuint buffer, GLuint offset, GLsizei size, const void *pointer, GLenum preserve)) \
    X (void        , glGetObjectBufferfvATI, (GLuint buffer, GLenum pname, GLfloat *params)) \
    X (void        , glGetObjectBufferivATI, (GLuint buffer, GLenum pname, GLint *params)) \
    X (void        , glFreeObjectBufferATI, (GLuint buffer)) \
    X (void        , glArrayObjectATI, (GLenum array, GLint size, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (void        , glGetArrayObjectfvATI, (GLenum array, GLenum pname, GLfloat *params)) \
    X (void        , glGetArrayObjectivATI, (GLenum array, GLenum pname, GLint *params)) \
    X (void        , glVariantArrayObjectATI, (GLuint id, GLenum type, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (void        , glGetVariantArrayObjectfvATI, (GLuint id, GLenum pname, GLfloat *params)) \
    X (void        , glGetVariantArrayObjectivATI, (GLuint id, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_ATI_vertex_attrib_array_object \
    X (void        , glVertexAttribArrayObjectATI, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLuint buffer, GLuint offset)) \
    X (void        , glGetVertexAttribArrayObjectfvATI, (GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetVertexAttribArrayObjectivATI, (GLuint index, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_ATI_vertex_streams \
    X (void        , glVertexStream1sATI, (GLenum stream, GLshort x)) \
    X (void        , glVertexStream1svATI, (GLenum stream, const GLshort *coords)) \
    X (void        , glVertexStream1iATI, (GLenum stream, GLint x)) \
    X (void        , glVertexStream1ivATI, (GLenum stream, const GLint *coords)) \
    X (void        , glVertexStream1fATI, (GLenum stream, GLfloat x)) \
    X (void        , glVertexStream1fvATI, (GLenum stream, const GLfloat *coords)) \
    X (void        , glVertexStream1dATI, (GLenum stream, GLdouble x)) \
    X (void        , glVertexStream1dvATI, (GLenum stream, const GLdouble *coords)) \
    X (void        , glVertexStream2sATI, (GLenum stream, GLshort x, GLshort y)) \
    X (void        , glVertexStream2svATI, (GLenum stream, const GLshort *coords)) \
    X (void        , glVertexStream2iATI, (GLenum stream, GLint x, GLint y)) \
    X (void        , glVertexStream2ivATI, (GLenum stream, const GLint *coords)) \
    X (void        , glVertexStream2fATI, (GLenum stream, GLfloat x, GLfloat y)) \
    X (void        , glVertexStream2fvATI, (GLenum stream, const GLfloat *coords)) \
    X (void        , glVertexStream2dATI, (GLenum stream, GLdouble x, GLdouble y)) \
    X (void        , glVertexStream2dvATI, (GLenum stream, const GLdouble *coords)) \
    X (void        , glVertexStream3sATI, (GLenum stream, GLshort x, GLshort y, GLshort z)) \
    X (void        , glVertexStream3svATI, (GLenum stream, const GLshort *coords)) \
    X (void        , glVertexStream3iATI, (GLenum stream, GLint x, GLint y, GLint z)) \
    X (void        , glVertexStream3ivATI, (GLenum stream, const GLint *coords)) \
    X (void        , glVertexStream3fATI, (GLenum stream, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertexStream3fvATI, (GLenum stream, const GLfloat *coords)) \
    X (void        , glVertexStream3dATI, (GLenum stream, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexStream3dvATI, (GLenum stream, const GLdouble *coords)) \
    X (void        , glVertexStream4sATI, (GLenum stream, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glVertexStream4svATI, (GLenum stream, const GLshort *coords)) \
    X (void        , glVertexStream4iATI, (GLenum stream, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glVertexStream4ivATI, (GLenum stream, const GLint *coords)) \
    X (void        , glVertexStream4fATI, (GLenum stream, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertexStream4fvATI, (GLenum stream, const GLfloat *coords)) \
    X (void        , glVertexStream4dATI, (GLenum stream, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexStream4dvATI, (GLenum stream, const GLdouble *coords)) \
    X (void        , glNormalStream3bATI, (GLenum stream, GLbyte nx, GLbyte ny, GLbyte nz)) \
    X (void        , glNormalStream3bvATI, (GLenum stream, const GLbyte *coords)) \
    X (void        , glNormalStream3sATI, (GLenum stream, GLshort nx, GLshort ny, GLshort nz)) \
    X (void        , glNormalStream3svATI, (GLenum stream, const GLshort *coords)) \
    X (void        , glNormalStream3iATI, (GLenum stream, GLint nx, GLint ny, GLint nz)) \
    X (void        , glNormalStream3ivATI, (GLenum stream, const GLint *coords)) \
    X (void        , glNormalStream3fATI, (GLenum stream, GLfloat nx, GLfloat ny, GLfloat nz)) \
    X (void        , glNormalStream3fvATI, (GLenum stream, const GLfloat *coords)) \
    X (void        , glNormalStream3dATI, (GLenum stream, GLdouble nx, GLdouble ny, GLdouble nz)) \
    X (void        , glNormalStream3dvATI, (GLenum stream, const GLdouble *coords)) \
    X (void        , glClientActiveVertexStreamATI, (GLenum stream)) \
    X (void        , glVertexBlendEnviATI, (GLenum pname, GLint param)) \
    X (void        , glVertexBlendEnvfATI, (GLenum pname, GLfloat param))

#define JUCE_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    X (void        , glEGLImageTargetTexStorageEXT, (GLenum target, GLeglImageOES image, const GLint* attrib_list)) \
    X (void        , glEGLImageTargetTextureStorageEXT, (GLuint texture, GLeglImageOES image, const GLint* attrib_list))

#define JUCE_GL_FUNCTIONS_GL_EXT_bindable_uniform \
    X (void        , glUniformBufferEXT, (GLuint program, GLint location, GLuint buffer)) \
    X (GLint       , glGetUniformBufferSizeEXT, (GLuint program, GLint location)) \
    X (GLintptr    , glGetUniformOffsetEXT, (GLuint program, GLint location))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_color \
    X (void        , glBlendColorEXT, (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_equation_separate \
    X (void        , glBlendEquationSeparateEXT, (GLenum modeRGB, GLenum modeAlpha))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_func_separate \
    X (void        , glBlendFuncSeparateEXT, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))

#define JUCE_GL_FUNCTIONS_GL_EXT_blend_minmax \
    X (void        , glBlendEquationEXT, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_EXT_color_subtable \
    X (void        , glColorSubTableEXT, (GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const void *data)) \
    X (void        , glCopyColorSubTableEXT, (GLenum target, GLsizei start, GLint x, GLint y, GLsizei width))

#define JUCE_GL_FUNCTIONS_GL_EXT_compiled_vertex_array \
    X (void        , glLockArraysEXT, (GLint first, GLsizei count)) \
    X (void        , glUnlockArraysEXT, ())

#define JUCE_GL_FUNCTIONS_GL_EXT_convolution \
    X (void        , glConvolutionFilter1DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *image)) \
    X (void        , glConvolutionFilter2DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *image)) \
    X (void        , glConvolutionParameterfEXT, (GLenum target, GLenum pname, GLfloat params)) \
    X (void        , glConvolutionParameterfvEXT, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glConvolutionParameteriEXT, (GLenum target, GLenum pname, GLint params)) \
    X (void        , glConvolutionParameterivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glCopyConvolutionFilter1DEXT, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyConvolutionFilter2DEXT, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glGetConvolutionFilterEXT, (GLenum target, GLenum format, GLenum type, void *image)) \
    X (void        , glGetConvolutionParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetConvolutionParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetSeparableFilterEXT, (GLenum target, GLenum format, GLenum type, void *row, void *column, void *span)) \
    X (void        , glSeparableFilter2DEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *row, const void *column))

#define JUCE_GL_FUNCTIONS_GL_EXT_coordinate_frame \
    X (void        , glTangent3bEXT, (GLbyte tx, GLbyte ty, GLbyte tz)) \
    X (void        , glTangent3bvEXT, (const GLbyte *v)) \
    X (void        , glTangent3dEXT, (GLdouble tx, GLdouble ty, GLdouble tz)) \
    X (void        , glTangent3dvEXT, (const GLdouble *v)) \
    X (void        , glTangent3fEXT, (GLfloat tx, GLfloat ty, GLfloat tz)) \
    X (void        , glTangent3fvEXT, (const GLfloat *v)) \
    X (void        , glTangent3iEXT, (GLint tx, GLint ty, GLint tz)) \
    X (void        , glTangent3ivEXT, (const GLint *v)) \
    X (void        , glTangent3sEXT, (GLshort tx, GLshort ty, GLshort tz)) \
    X (void        , glTangent3svEXT, (const GLshort *v)) \
    X (void        , glBinormal3bEXT, (GLbyte bx, GLbyte by, GLbyte bz)) \
    X (void        , glBinormal3bvEXT, (const GLbyte *v)) \
    X (void        , glBinormal3dEXT, (GLdouble bx, GLdouble by, GLdouble bz)) \
    X (void        , glBinormal3dvEXT, (const GLdouble *v)) \
    X (void        , glBinormal3fEXT, (GLfloat bx, GLfloat by, GLfloat bz)) \
    X (void        , glBinormal3fvEXT, (const GLfloat *v)) \
    X (void        , glBinormal3iEXT, (GLint bx, GLint by, GLint bz)) \
    X (void        , glBinormal3ivEXT, (const GLint *v)) \
    X (void        , glBinormal3sEXT, (GLshort bx, GLshort by, GLshort bz)) \
    X (void        , glBinormal3svEXT, (const GLshort *v)) \
    X (void        , glTangentPointerEXT, (GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glBinormalPointerEXT, (GLenum type, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_EXT_copy_texture \
    X (void        , glCopyTexImage1DEXT, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (void        , glCopyTexImage2DEXT, (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (void        , glCopyTexSubImage1DEXT, (GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyTexSubImage2DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glCopyTexSubImage3DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_EXT_cull_vertex \
    X (void        , glCullParameterdvEXT, (GLenum pname, GLdouble *params)) \
    X (void        , glCullParameterfvEXT, (GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_debug_label \
    X (void        , glLabelObjectEXT, (GLenum type, GLuint object, GLsizei length, const GLchar *label)) \
    X (void        , glGetObjectLabelEXT, (GLenum type, GLuint object, GLsizei bufSize, GLsizei *length, GLchar *label))

#define JUCE_GL_FUNCTIONS_GL_EXT_debug_marker \
    X (void        , glInsertEventMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (void        , glPushGroupMarkerEXT, (GLsizei length, const GLchar *marker)) \
    X (void        , glPopGroupMarkerEXT, ())

#define JUCE_GL_FUNCTIONS_GL_EXT_depth_bounds_test \
    X (void        , glDepthBoundsEXT, (GLclampd zmin, GLclampd zmax))

#define JUCE_GL_FUNCTIONS_GL_EXT_direct_state_access \
    X (void        , glMatrixLoadfEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixLoaddEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixMultfEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixMultdEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixLoadIdentityEXT, (GLenum mode)) \
    X (void        , glMatrixRotatefEXT, (GLenum mode, GLfloat angle, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixRotatedEXT, (GLenum mode, GLdouble angle, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glMatrixScalefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixScaledEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glMatrixTranslatefEXT, (GLenum mode, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glMatrixTranslatedEXT, (GLenum mode, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glMatrixFrustumEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glMatrixOrthoEXT, (GLenum mode, GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar)) \
    X (void        , glMatrixPopEXT, (GLenum mode)) \
    X (void        , glMatrixPushEXT, (GLenum mode)) \
    X (void        , glClientAttribDefaultEXT, (GLbitfield mask)) \
    X (void        , glPushClientAttribDefaultEXT, (GLbitfield mask)) \
    X (void        , glTextureParameterfEXT, (GLuint texture, GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glTextureParameterfvEXT, (GLuint texture, GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glTextureParameteriEXT, (GLuint texture, GLenum target, GLenum pname, GLint param)) \
    X (void        , glTextureParameterivEXT, (GLuint texture, GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (void        , glCopyTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (void        , glCopyTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glGetTextureImageEXT, (GLuint texture, GLenum target, GLint level, GLenum format, GLenum type, void *pixels)) \
    X (void        , glGetTextureParameterfvEXT, (GLuint texture, GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetTextureParameterivEXT, (GLuint texture, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTextureLevelParameterfvEXT, (GLuint texture, GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (void        , glGetTextureLevelParameterivEXT, (GLuint texture, GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (void        , glTextureImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glBindMultiTextureEXT, (GLenum texunit, GLenum target, GLuint texture)) \
    X (void        , glMultiTexCoordPointerEXT, (GLenum texunit, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glMultiTexEnvfEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glMultiTexEnvfvEXT, (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glMultiTexEnviEXT, (GLenum texunit, GLenum target, GLenum pname, GLint param)) \
    X (void        , glMultiTexEnvivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glMultiTexGendEXT, (GLenum texunit, GLenum coord, GLenum pname, GLdouble param)) \
    X (void        , glMultiTexGendvEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLdouble *params)) \
    X (void        , glMultiTexGenfEXT, (GLenum texunit, GLenum coord, GLenum pname, GLfloat param)) \
    X (void        , glMultiTexGenfvEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLfloat *params)) \
    X (void        , glMultiTexGeniEXT, (GLenum texunit, GLenum coord, GLenum pname, GLint param)) \
    X (void        , glMultiTexGenivEXT, (GLenum texunit, GLenum coord, GLenum pname, const GLint *params)) \
    X (void        , glGetMultiTexEnvfvEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetMultiTexEnvivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMultiTexGendvEXT, (GLenum texunit, GLenum coord, GLenum pname, GLdouble *params)) \
    X (void        , glGetMultiTexGenfvEXT, (GLenum texunit, GLenum coord, GLenum pname, GLfloat *params)) \
    X (void        , glGetMultiTexGenivEXT, (GLenum texunit, GLenum coord, GLenum pname, GLint *params)) \
    X (void        , glMultiTexParameteriEXT, (GLenum texunit, GLenum target, GLenum pname, GLint param)) \
    X (void        , glMultiTexParameterivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glMultiTexParameterfEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glMultiTexParameterfvEXT, (GLenum texunit, GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border)) \
    X (void        , glCopyMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border)) \
    X (void        , glCopyMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width)) \
    X (void        , glCopyMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glGetMultiTexImageEXT, (GLenum texunit, GLenum target, GLint level, GLenum format, GLenum type, void *pixels)) \
    X (void        , glGetMultiTexParameterfvEXT, (GLenum texunit, GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetMultiTexParameterivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMultiTexLevelParameterfvEXT, (GLenum texunit, GLenum target, GLint level, GLenum pname, GLfloat *params)) \
    X (void        , glGetMultiTexLevelParameterivEXT, (GLenum texunit, GLenum target, GLint level, GLenum pname, GLint *params)) \
    X (void        , glMultiTexImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glCopyMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glEnableClientStateIndexedEXT, (GLenum array, GLuint index)) \
    X (void        , glDisableClientStateIndexedEXT, (GLenum array, GLuint index)) \
    X (void        , glGetFloatIndexedvEXT, (GLenum target, GLuint index, GLfloat *data)) \
    X (void        , glGetDoubleIndexedvEXT, (GLenum target, GLuint index, GLdouble *data)) \
    X (void        , glGetPointerIndexedvEXT, (GLenum target, GLuint index, void **data)) \
    X (void        , glEnableIndexedEXT, (GLenum target, GLuint index)) \
    X (void        , glDisableIndexedEXT, (GLenum target, GLuint index)) \
    X (GLboolean   , glIsEnabledIndexedEXT, (GLenum target, GLuint index)) \
    X (void        , glGetIntegerIndexedvEXT, (GLenum target, GLuint index, GLint *data)) \
    X (void        , glGetBooleanIndexedvEXT, (GLenum target, GLuint index, GLboolean *data)) \
    X (void        , glCompressedTextureImage3DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedTextureImage2DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedTextureImage1DEXT, (GLuint texture, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedTextureSubImage3DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedTextureSubImage2DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedTextureSubImage1DEXT, (GLuint texture, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glGetCompressedTextureImageEXT, (GLuint texture, GLenum target, GLint lod, void *img)) \
    X (void        , glCompressedMultiTexImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedMultiTexImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedMultiTexImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedMultiTexSubImage3DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedMultiTexSubImage2DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glCompressedMultiTexSubImage1DEXT, (GLenum texunit, GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const void *bits)) \
    X (void        , glGetCompressedMultiTexImageEXT, (GLenum texunit, GLenum target, GLint lod, void *img)) \
    X (void        , glMatrixLoadTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixLoadTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glMatrixMultTransposefEXT, (GLenum mode, const GLfloat *m)) \
    X (void        , glMatrixMultTransposedEXT, (GLenum mode, const GLdouble *m)) \
    X (void        , glNamedBufferDataEXT, (GLuint buffer, GLsizeiptr size, const void *data, GLenum usage)) \
    X (void        , glNamedBufferSubDataEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (void *      , glMapNamedBufferEXT, (GLuint buffer, GLenum access)) \
    X (GLboolean   , glUnmapNamedBufferEXT, (GLuint buffer)) \
    X (void        , glGetNamedBufferParameterivEXT, (GLuint buffer, GLenum pname, GLint *params)) \
    X (void        , glGetNamedBufferPointervEXT, (GLuint buffer, GLenum pname, void **params)) \
    X (void        , glGetNamedBufferSubDataEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, void *data)) \
    X (void        , glProgramUniform1fEXT, (GLuint program, GLint location, GLfloat v0)) \
    X (void        , glProgramUniform2fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1)) \
    X (void        , glProgramUniform3fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2)) \
    X (void        , glProgramUniform4fEXT, (GLuint program, GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3)) \
    X (void        , glProgramUniform1iEXT, (GLuint program, GLint location, GLint v0)) \
    X (void        , glProgramUniform2iEXT, (GLuint program, GLint location, GLint v0, GLint v1)) \
    X (void        , glProgramUniform3iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2)) \
    X (void        , glProgramUniform4iEXT, (GLuint program, GLint location, GLint v0, GLint v1, GLint v2, GLint v3)) \
    X (void        , glProgramUniform1fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform2fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform3fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform4fvEXT, (GLuint program, GLint location, GLsizei count, const GLfloat *value)) \
    X (void        , glProgramUniform1ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform2ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform3ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniform4ivEXT, (GLuint program, GLint location, GLsizei count, const GLint *value)) \
    X (void        , glProgramUniformMatrix2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix2x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x2fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix3x4fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glProgramUniformMatrix4x3fvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value)) \
    X (void        , glTextureBufferEXT, (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glMultiTexBufferEXT, (GLenum texunit, GLenum target, GLenum internalformat, GLuint buffer)) \
    X (void        , glTextureParameterIivEXT, (GLuint texture, GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTextureParameterIuivEXT, (GLuint texture, GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTextureParameterIivEXT, (GLuint texture, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTextureParameterIuivEXT, (GLuint texture, GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glMultiTexParameterIivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glMultiTexParameterIuivEXT, (GLenum texunit, GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetMultiTexParameterIivEXT, (GLenum texunit, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMultiTexParameterIuivEXT, (GLenum texunit, GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glProgramUniform1uiEXT, (GLuint program, GLint location, GLuint v0)) \
    X (void        , glProgramUniform2uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1)) \
    X (void        , glProgramUniform3uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glProgramUniform4uiEXT, (GLuint program, GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glProgramUniform1uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform2uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform3uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glProgramUniform4uivEXT, (GLuint program, GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glNamedProgramLocalParameters4fvEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLfloat *params)) \
    X (void        , glNamedProgramLocalParameterI4iEXT, (GLuint program, GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glNamedProgramLocalParameterI4ivEXT, (GLuint program, GLenum target, GLuint index, const GLint *params)) \
    X (void        , glNamedProgramLocalParametersI4ivEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (void        , glNamedProgramLocalParameterI4uiEXT, (GLuint program, GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glNamedProgramLocalParameterI4uivEXT, (GLuint program, GLenum target, GLuint index, const GLuint *params)) \
    X (void        , glNamedProgramLocalParametersI4uivEXT, (GLuint program, GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (void        , glGetNamedProgramLocalParameterIivEXT, (GLuint program, GLenum target, GLuint index, GLint *params)) \
    X (void        , glGetNamedProgramLocalParameterIuivEXT, (GLuint program, GLenum target, GLuint index, GLuint *params)) \
    X (void        , glEnableClientStateiEXT, (GLenum array, GLuint index)) \
    X (void        , glDisableClientStateiEXT, (GLenum array, GLuint index)) \
    X (void        , glGetFloati_vEXT, (GLenum pname, GLuint index, GLfloat *params)) \
    X (void        , glGetDoublei_vEXT, (GLenum pname, GLuint index, GLdouble *params)) \
    X (void        , glGetPointeri_vEXT, (GLenum pname, GLuint index, void **params)) \
    X (void        , glNamedProgramStringEXT, (GLuint program, GLenum target, GLenum format, GLsizei len, const void *string)) \
    X (void        , glNamedProgramLocalParameter4dEXT, (GLuint program, GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glNamedProgramLocalParameter4dvEXT, (GLuint program, GLenum target, GLuint index, const GLdouble *params)) \
    X (void        , glNamedProgramLocalParameter4fEXT, (GLuint program, GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glNamedProgramLocalParameter4fvEXT, (GLuint program, GLenum target, GLuint index, const GLfloat *params)) \
    X (void        , glGetNamedProgramLocalParameterdvEXT, (GLuint program, GLenum target, GLuint index, GLdouble *params)) \
    X (void        , glGetNamedProgramLocalParameterfvEXT, (GLuint program, GLenum target, GLuint index, GLfloat *params)) \
    X (void        , glGetNamedProgramivEXT, (GLuint program, GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetNamedProgramStringEXT, (GLuint program, GLenum target, GLenum pname, void *string)) \
    X (void        , glNamedRenderbufferStorageEXT, (GLuint renderbuffer, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glGetNamedRenderbufferParameterivEXT, (GLuint renderbuffer, GLenum pname, GLint *params)) \
    X (void        , glNamedRenderbufferStorageMultisampleEXT, (GLuint renderbuffer, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glNamedRenderbufferStorageMultisampleCoverageEXT, (GLuint renderbuffer, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (GLenum      , glCheckNamedFramebufferStatusEXT, (GLuint framebuffer, GLenum target)) \
    X (void        , glNamedFramebufferTexture1DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glNamedFramebufferTexture2DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glNamedFramebufferTexture3DEXT, (GLuint framebuffer, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (void        , glNamedFramebufferRenderbufferEXT, (GLuint framebuffer, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (void        , glGetNamedFramebufferAttachmentParameterivEXT, (GLuint framebuffer, GLenum attachment, GLenum pname, GLint *params)) \
    X (void        , glGenerateTextureMipmapEXT, (GLuint texture, GLenum target)) \
    X (void        , glGenerateMultiTexMipmapEXT, (GLenum texunit, GLenum target)) \
    X (void        , glFramebufferDrawBufferEXT, (GLuint framebuffer, GLenum mode)) \
    X (void        , glFramebufferDrawBuffersEXT, (GLuint framebuffer, GLsizei n, const GLenum *bufs)) \
    X (void        , glFramebufferReadBufferEXT, (GLuint framebuffer, GLenum mode)) \
    X (void        , glGetFramebufferParameterivEXT, (GLuint framebuffer, GLenum pname, GLint *params)) \
    X (void        , glNamedCopyBufferSubDataEXT, (GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (void        , glNamedFramebufferTextureEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glNamedFramebufferTextureLayerEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLint layer)) \
    X (void        , glNamedFramebufferTextureFaceEXT, (GLuint framebuffer, GLenum attachment, GLuint texture, GLint level, GLenum face)) \
    X (void        , glTextureRenderbufferEXT, (GLuint texture, GLenum target, GLuint renderbuffer)) \
    X (void        , glMultiTexRenderbufferEXT, (GLenum texunit, GLenum target, GLuint renderbuffer)) \
    X (void        , glVertexArrayVertexOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayColorOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayEdgeFlagOffsetEXT, (GLuint vaobj, GLuint buffer, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayIndexOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayNormalOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayTexCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayMultiTexCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum texunit, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayFogCoordOffsetEXT, (GLuint vaobj, GLuint buffer, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArraySecondaryColorOffsetEXT, (GLuint vaobj, GLuint buffer, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayVertexAttribOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLintptr offset)) \
    X (void        , glVertexArrayVertexAttribIOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glEnableVertexArrayEXT, (GLuint vaobj, GLenum array)) \
    X (void        , glDisableVertexArrayEXT, (GLuint vaobj, GLenum array)) \
    X (void        , glEnableVertexArrayAttribEXT, (GLuint vaobj, GLuint index)) \
    X (void        , glDisableVertexArrayAttribEXT, (GLuint vaobj, GLuint index)) \
    X (void        , glGetVertexArrayIntegervEXT, (GLuint vaobj, GLenum pname, GLint *param)) \
    X (void        , glGetVertexArrayPointervEXT, (GLuint vaobj, GLenum pname, void **param)) \
    X (void        , glGetVertexArrayIntegeri_vEXT, (GLuint vaobj, GLuint index, GLenum pname, GLint *param)) \
    X (void        , glGetVertexArrayPointeri_vEXT, (GLuint vaobj, GLuint index, GLenum pname, void **param)) \
    X (void *      , glMapNamedBufferRangeEXT, (GLuint buffer, GLintptr offset, GLsizeiptr length, GLbitfield access)) \
    X (void        , glFlushMappedNamedBufferRangeEXT, (GLuint buffer, GLintptr offset, GLsizeiptr length)) \
    X (void        , glNamedBufferStorageEXT, (GLuint buffer, GLsizeiptr size, const void *data, GLbitfield flags)) \
    X (void        , glClearNamedBufferDataEXT, (GLuint buffer, GLenum internalformat, GLenum format, GLenum type, const void *data)) \
    X (void        , glClearNamedBufferSubDataEXT, (GLuint buffer, GLenum internalformat, GLsizeiptr offset, GLsizeiptr size, GLenum format, GLenum type, const void *data)) \
    X (void        , glNamedFramebufferParameteriEXT, (GLuint framebuffer, GLenum pname, GLint param)) \
    X (void        , glGetNamedFramebufferParameterivEXT, (GLuint framebuffer, GLenum pname, GLint *params)) \
    X (void        , glProgramUniform1dEXT, (GLuint program, GLint location, GLdouble x)) \
    X (void        , glProgramUniform2dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y)) \
    X (void        , glProgramUniform3dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glProgramUniform4dEXT, (GLuint program, GLint location, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glProgramUniform1dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform2dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform3dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniform4dvEXT, (GLuint program, GLint location, GLsizei count, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix2x3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix2x4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3x2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix3x4dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4x2dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glProgramUniformMatrix4x3dvEXT, (GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLdouble *value)) \
    X (void        , glTextureBufferRangeEXT, (GLuint texture, GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glTextureStorage1DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTextureStorage2DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTextureStorage3DEXT, (GLuint texture, GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glTextureStorage2DMultisampleEXT, (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLboolean fixedsamplelocations)) \
    X (void        , glTextureStorage3DMultisampleEXT, (GLuint texture, GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations)) \
    X (void        , glVertexArrayBindVertexBufferEXT, (GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride)) \
    X (void        , glVertexArrayVertexAttribFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset)) \
    X (void        , glVertexArrayVertexAttribIFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexArrayVertexAttribLFormatEXT, (GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLuint relativeoffset)) \
    X (void        , glVertexArrayVertexAttribBindingEXT, (GLuint vaobj, GLuint attribindex, GLuint bindingindex)) \
    X (void        , glVertexArrayVertexBindingDivisorEXT, (GLuint vaobj, GLuint bindingindex, GLuint divisor)) \
    X (void        , glVertexArrayVertexAttribLOffsetEXT, (GLuint vaobj, GLuint buffer, GLuint index, GLint size, GLenum type, GLsizei stride, GLintptr offset)) \
    X (void        , glTexturePageCommitmentEXT, (GLuint texture, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLboolean commit)) \
    X (void        , glVertexArrayVertexAttribDivisorEXT, (GLuint vaobj, GLuint index, GLuint divisor))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers2 \
    X (void        , glColorMaskIndexedEXT, (GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_instanced \
    X (void        , glDrawArraysInstancedEXT, (GLenum mode, GLint start, GLsizei count, GLsizei primcount)) \
    X (void        , glDrawElementsInstancedEXT, (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount))

#define JUCE_GL_FUNCTIONS_GL_EXT_draw_range_elements \
    X (void        , glDrawRangeElementsEXT, (GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices))

#define JUCE_GL_FUNCTIONS_GL_EXT_external_buffer \
    X (void        , glBufferStorageExternalEXT, (GLenum target, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags)) \
    X (void        , glNamedBufferStorageExternalEXT, (GLuint buffer, GLintptr offset, GLsizeiptr size, GLeglClientBufferEXT clientBuffer, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_EXT_fog_coord \
    X (void        , glFogCoordfEXT, (GLfloat coord)) \
    X (void        , glFogCoordfvEXT, (const GLfloat *coord)) \
    X (void        , glFogCoorddEXT, (GLdouble coord)) \
    X (void        , glFogCoorddvEXT, (const GLdouble *coord)) \
    X (void        , glFogCoordPointerEXT, (GLenum type, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_blit \
    X (void        , glBlitFramebufferEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter))

#define JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    X (void        , glBlitFramebufferLayersEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (void        , glBlitFramebufferLayerEXT, (GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint srcLayer, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLint dstLayer, GLbitfield mask, GLenum filter))

#define JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_multisample \
    X (void        , glRenderbufferStorageMultisampleEXT, (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_object \
    X (GLboolean   , glIsRenderbufferEXT, (GLuint renderbuffer)) \
    X (void        , glBindRenderbufferEXT, (GLenum target, GLuint renderbuffer)) \
    X (void        , glDeleteRenderbuffersEXT, (GLsizei n, const GLuint *renderbuffers)) \
    X (void        , glGenRenderbuffersEXT, (GLsizei n, GLuint *renderbuffers)) \
    X (void        , glRenderbufferStorageEXT, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glGetRenderbufferParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsFramebufferEXT, (GLuint framebuffer)) \
    X (void        , glBindFramebufferEXT, (GLenum target, GLuint framebuffer)) \
    X (void        , glDeleteFramebuffersEXT, (GLsizei n, const GLuint *framebuffers)) \
    X (void        , glGenFramebuffersEXT, (GLsizei n, GLuint *framebuffers)) \
    X (GLenum      , glCheckFramebufferStatusEXT, (GLenum target)) \
    X (void        , glFramebufferTexture1DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glFramebufferTexture2DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)) \
    X (void        , glFramebufferTexture3DEXT, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset)) \
    X (void        , glFramebufferRenderbufferEXT, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)) \
    X (void        , glGetFramebufferAttachmentParameterivEXT, (GLenum target, GLenum attachment, GLenum pname, GLint *params)) \
    X (void        , glGenerateMipmapEXT, (GLenum target))

#define JUCE_GL_FUNCTIONS_GL_EXT_geometry_shader4 \
    X (void        , glProgramParameteriEXT, (GLuint program, GLenum pname, GLint value))

#define JUCE_GL_FUNCTIONS_GL_EXT_gpu_program_parameters \
    X (void        , glProgramEnvParameters4fvEXT, (GLenum target, GLuint index, GLsizei count, const GLfloat *params)) \
    X (void        , glProgramLocalParameters4fvEXT, (GLenum target, GLuint index, GLsizei count, const GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_gpu_shader4 \
    X (void        , glGetUniformuivEXT, (GLuint program, GLint location, GLuint *params)) \
    X (void        , glBindFragDataLocationEXT, (GLuint program, GLuint color, const GLchar *name)) \
    X (GLint       , glGetFragDataLocationEXT, (GLuint program, const GLchar *name)) \
    X (void        , glUniform1uiEXT, (GLint location, GLuint v0)) \
    X (void        , glUniform2uiEXT, (GLint location, GLuint v0, GLuint v1)) \
    X (void        , glUniform3uiEXT, (GLint location, GLuint v0, GLuint v1, GLuint v2)) \
    X (void        , glUniform4uiEXT, (GLint location, GLuint v0, GLuint v1, GLuint v2, GLuint v3)) \
    X (void        , glUniform1uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform2uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform3uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glUniform4uivEXT, (GLint location, GLsizei count, const GLuint *value)) \
    X (void        , glVertexAttribI1iEXT, (GLuint index, GLint x)) \
    X (void        , glVertexAttribI2iEXT, (GLuint index, GLint x, GLint y)) \
    X (void        , glVertexAttribI3iEXT, (GLuint index, GLint x, GLint y, GLint z)) \
    X (void        , glVertexAttribI4iEXT, (GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glVertexAttribI1uiEXT, (GLuint index, GLuint x)) \
    X (void        , glVertexAttribI2uiEXT, (GLuint index, GLuint x, GLuint y)) \
    X (void        , glVertexAttribI3uiEXT, (GLuint index, GLuint x, GLuint y, GLuint z)) \
    X (void        , glVertexAttribI4uiEXT, (GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glVertexAttribI1ivEXT, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI2ivEXT, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI3ivEXT, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI4ivEXT, (GLuint index, const GLint *v)) \
    X (void        , glVertexAttribI1uivEXT, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI2uivEXT, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI3uivEXT, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI4uivEXT, (GLuint index, const GLuint *v)) \
    X (void        , glVertexAttribI4bvEXT, (GLuint index, const GLbyte *v)) \
    X (void        , glVertexAttribI4svEXT, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttribI4ubvEXT, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttribI4usvEXT, (GLuint index, const GLushort *v)) \
    X (void        , glVertexAttribIPointerEXT, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glGetVertexAttribIivEXT, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribIuivEXT, (GLuint index, GLenum pname, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_histogram \
    X (void        , glGetHistogramEXT, (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)) \
    X (void        , glGetHistogramParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetHistogramParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMinmaxEXT, (GLenum target, GLboolean reset, GLenum format, GLenum type, void *values)) \
    X (void        , glGetMinmaxParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetMinmaxParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glHistogramEXT, (GLenum target, GLsizei width, GLenum internalformat, GLboolean sink)) \
    X (void        , glMinmaxEXT, (GLenum target, GLenum internalformat, GLboolean sink)) \
    X (void        , glResetHistogramEXT, (GLenum target)) \
    X (void        , glResetMinmaxEXT, (GLenum target))

#define JUCE_GL_FUNCTIONS_GL_EXT_index_func \
    X (void        , glIndexFuncEXT, (GLenum func, GLclampf ref))

#define JUCE_GL_FUNCTIONS_GL_EXT_index_material \
    X (void        , glIndexMaterialEXT, (GLenum face, GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_EXT_light_texture \
    X (void        , glApplyTextureEXT, (GLenum mode)) \
    X (void        , glTextureLightEXT, (GLenum pname)) \
    X (void        , glTextureMaterialEXT, (GLenum face, GLenum mode))

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

#define JUCE_GL_FUNCTIONS_GL_EXT_multisample \
    X (void        , glSampleMaskEXT, (GLclampf value, GLboolean invert)) \
    X (void        , glSamplePatternEXT, (GLenum pattern))

#define JUCE_GL_FUNCTIONS_GL_EXT_paletted_texture \
    X (void        , glColorTableEXT, (GLenum target, GLenum internalFormat, GLsizei width, GLenum format, GLenum type, const void *table)) \
    X (void        , glGetColorTableEXT, (GLenum target, GLenum format, GLenum type, void *data)) \
    X (void        , glGetColorTableParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetColorTableParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_pixel_transform \
    X (void        , glPixelTransformParameteriEXT, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glPixelTransformParameterfEXT, (GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glPixelTransformParameterivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glPixelTransformParameterfvEXT, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glGetPixelTransformParameterivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetPixelTransformParameterfvEXT, (GLenum target, GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_point_parameters \
    X (void        , glPointParameterfEXT, (GLenum pname, GLfloat param)) \
    X (void        , glPointParameterfvEXT, (GLenum pname, const GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset \
    X (void        , glPolygonOffsetEXT, (GLfloat factor, GLfloat bias))

#define JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    X (void        , glPolygonOffsetClampEXT, (GLfloat factor, GLfloat units, GLfloat clamp))

#define JUCE_GL_FUNCTIONS_GL_EXT_provoking_vertex \
    X (void        , glProvokingVertexEXT, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_EXT_raster_multisample \
    X (void        , glRasterSamplesEXT, (GLuint samples, GLboolean fixedsamplelocations))

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

#define JUCE_GL_FUNCTIONS_GL_EXT_secondary_color \
    X (void        , glSecondaryColor3bEXT, (GLbyte red, GLbyte green, GLbyte blue)) \
    X (void        , glSecondaryColor3bvEXT, (const GLbyte *v)) \
    X (void        , glSecondaryColor3dEXT, (GLdouble red, GLdouble green, GLdouble blue)) \
    X (void        , glSecondaryColor3dvEXT, (const GLdouble *v)) \
    X (void        , glSecondaryColor3fEXT, (GLfloat red, GLfloat green, GLfloat blue)) \
    X (void        , glSecondaryColor3fvEXT, (const GLfloat *v)) \
    X (void        , glSecondaryColor3iEXT, (GLint red, GLint green, GLint blue)) \
    X (void        , glSecondaryColor3ivEXT, (const GLint *v)) \
    X (void        , glSecondaryColor3sEXT, (GLshort red, GLshort green, GLshort blue)) \
    X (void        , glSecondaryColor3svEXT, (const GLshort *v)) \
    X (void        , glSecondaryColor3ubEXT, (GLubyte red, GLubyte green, GLubyte blue)) \
    X (void        , glSecondaryColor3ubvEXT, (const GLubyte *v)) \
    X (void        , glSecondaryColor3uiEXT, (GLuint red, GLuint green, GLuint blue)) \
    X (void        , glSecondaryColor3uivEXT, (const GLuint *v)) \
    X (void        , glSecondaryColor3usEXT, (GLushort red, GLushort green, GLushort blue)) \
    X (void        , glSecondaryColor3usvEXT, (const GLushort *v)) \
    X (void        , glSecondaryColorPointerEXT, (GLint size, GLenum type, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    X (void        , glUseShaderProgramEXT, (GLenum type, GLuint program)) \
    X (void        , glActiveProgramEXT, (GLuint program)) \
    X (GLuint      , glCreateShaderProgramEXT, (GLenum type, const GLchar *string)) \
    X (void        , glActiveShaderProgramEXT, (GLuint pipeline, GLuint program)) \
    X (void        , glBindProgramPipelineEXT, (GLuint pipeline)) \
    X (GLuint      , glCreateShaderProgramvEXT, (GLenum type, GLsizei count, const GLchar *const*strings)) \
    X (void        , glDeleteProgramPipelinesEXT, (GLsizei n, const GLuint *pipelines)) \
    X (void        , glGenProgramPipelinesEXT, (GLsizei n, GLuint *pipelines)) \
    X (void        , glGetProgramPipelineInfoLogEXT, (GLuint pipeline, GLsizei bufSize, GLsizei *length, GLchar *infoLog)) \
    X (void        , glGetProgramPipelineivEXT, (GLuint pipeline, GLenum pname, GLint *params)) \
    X (GLboolean   , glIsProgramPipelineEXT, (GLuint pipeline)) \
    X (void        , glUseProgramStagesEXT, (GLuint pipeline, GLbitfield stages, GLuint program)) \
    X (void        , glValidateProgramPipelineEXT, (GLuint pipeline))

#define JUCE_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    X (void        , glFramebufferFetchBarrierEXT, ())

#define JUCE_GL_FUNCTIONS_GL_EXT_shader_image_load_store \
    X (void        , glBindImageTextureEXT, (GLuint index, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLint format)) \
    X (void        , glMemoryBarrierEXT, (GLbitfield barriers))

#define JUCE_GL_FUNCTIONS_GL_EXT_stencil_clear_tag \
    X (void        , glStencilClearTagEXT, (GLsizei stencilTagBits, GLuint stencilClearTag))

#define JUCE_GL_FUNCTIONS_GL_EXT_stencil_two_side \
    X (void        , glActiveStencilFaceEXT, (GLenum face))

#define JUCE_GL_FUNCTIONS_GL_EXT_subtexture \
    X (void        , glTexSubImage1DEXT, (GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage2DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture3D \
    X (void        , glTexImage3DEXT, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage3DEXT, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *pixels))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_array \
    X (void        , glFramebufferTextureLayerEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_buffer_object \
    X (void        , glTexBufferEXT, (GLenum target, GLenum internalformat, GLuint buffer))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_integer \
    X (void        , glTexParameterIivEXT, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glTexParameterIuivEXT, (GLenum target, GLenum pname, const GLuint *params)) \
    X (void        , glGetTexParameterIivEXT, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetTexParameterIuivEXT, (GLenum target, GLenum pname, GLuint *params)) \
    X (void        , glClearColorIiEXT, (GLint red, GLint green, GLint blue, GLint alpha)) \
    X (void        , glClearColorIuiEXT, (GLuint red, GLuint green, GLuint blue, GLuint alpha))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_object \
    X (GLboolean   , glAreTexturesResidentEXT, (GLsizei n, const GLuint *textures, GLboolean *residences)) \
    X (void        , glBindTextureEXT, (GLenum target, GLuint texture)) \
    X (void        , glDeleteTexturesEXT, (GLsizei n, const GLuint *textures)) \
    X (void        , glGenTexturesEXT, (GLsizei n, GLuint *textures)) \
    X (GLboolean   , glIsTextureEXT, (GLuint texture)) \
    X (void        , glPrioritizeTexturesEXT, (GLsizei n, const GLuint *textures, const GLclampf *priorities))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_perturb_normal \
    X (void        , glTextureNormalEXT, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_EXT_texture_storage \
    X (void        , glTexStorage1DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width)) \
    X (void        , glTexStorage2DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)) \
    X (void        , glTexStorage3DEXT, (GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth))

#define JUCE_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    X (void        , glCreateSemaphoresNV, (GLsizei n, GLuint *semaphores)) \
    X (void        , glSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, const GLint *params)) \
    X (void        , glGetSemaphoreParameterivNV, (GLuint semaphore, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_timer_query \
    X (void        , glGetQueryObjecti64vEXT, (GLuint id, GLenum pname, GLint64 *params)) \
    X (void        , glGetQueryObjectui64vEXT, (GLuint id, GLenum pname, GLuint64 *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_transform_feedback \
    X (void        , glBeginTransformFeedbackEXT, (GLenum primitiveMode)) \
    X (void        , glEndTransformFeedbackEXT, ()) \
    X (void        , glBindBufferRangeEXT, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glBindBufferOffsetEXT, (GLenum target, GLuint index, GLuint buffer, GLintptr offset)) \
    X (void        , glBindBufferBaseEXT, (GLenum target, GLuint index, GLuint buffer)) \
    X (void        , glTransformFeedbackVaryingsEXT, (GLuint program, GLsizei count, const GLchar *const*varyings, GLenum bufferMode)) \
    X (void        , glGetTransformFeedbackVaryingEXT, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name))

#define JUCE_GL_FUNCTIONS_GL_EXT_vertex_array \
    X (void        , glArrayElementEXT, (GLint i)) \
    X (void        , glColorPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer)) \
    X (void        , glDrawArraysEXT, (GLenum mode, GLint first, GLsizei count)) \
    X (void        , glEdgeFlagPointerEXT, (GLsizei stride, GLsizei count, const GLboolean *pointer)) \
    X (void        , glGetPointervEXT, (GLenum pname, void **params)) \
    X (void        , glIndexPointerEXT, (GLenum type, GLsizei stride, GLsizei count, const void *pointer)) \
    X (void        , glNormalPointerEXT, (GLenum type, GLsizei stride, GLsizei count, const void *pointer)) \
    X (void        , glTexCoordPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer)) \
    X (void        , glVertexPointerEXT, (GLint size, GLenum type, GLsizei stride, GLsizei count, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_EXT_vertex_attrib_64bit \
    X (void        , glVertexAttribL1dEXT, (GLuint index, GLdouble x)) \
    X (void        , glVertexAttribL2dEXT, (GLuint index, GLdouble x, GLdouble y)) \
    X (void        , glVertexAttribL3dEXT, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexAttribL4dEXT, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexAttribL1dvEXT, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL2dvEXT, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL3dvEXT, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribL4dvEXT, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttribLPointerEXT, (GLuint index, GLint size, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glGetVertexAttribLdvEXT, (GLuint index, GLenum pname, GLdouble *params))

#define JUCE_GL_FUNCTIONS_GL_EXT_vertex_shader \
    X (void        , glBeginVertexShaderEXT, ()) \
    X (void        , glEndVertexShaderEXT, ()) \
    X (void        , glBindVertexShaderEXT, (GLuint id)) \
    X (GLuint      , glGenVertexShadersEXT, (GLuint range)) \
    X (void        , glDeleteVertexShaderEXT, (GLuint id)) \
    X (void        , glShaderOp1EXT, (GLenum op, GLuint res, GLuint arg1)) \
    X (void        , glShaderOp2EXT, (GLenum op, GLuint res, GLuint arg1, GLuint arg2)) \
    X (void        , glShaderOp3EXT, (GLenum op, GLuint res, GLuint arg1, GLuint arg2, GLuint arg3)) \
    X (void        , glSwizzleEXT, (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)) \
    X (void        , glWriteMaskEXT, (GLuint res, GLuint in, GLenum outX, GLenum outY, GLenum outZ, GLenum outW)) \
    X (void        , glInsertComponentEXT, (GLuint res, GLuint src, GLuint num)) \
    X (void        , glExtractComponentEXT, (GLuint res, GLuint src, GLuint num)) \
    X (GLuint      , glGenSymbolsEXT, (GLenum datatype, GLenum storagetype, GLenum range, GLuint components)) \
    X (void        , glSetInvariantEXT, (GLuint id, GLenum type, const void *addr)) \
    X (void        , glSetLocalConstantEXT, (GLuint id, GLenum type, const void *addr)) \
    X (void        , glVariantbvEXT, (GLuint id, const GLbyte *addr)) \
    X (void        , glVariantsvEXT, (GLuint id, const GLshort *addr)) \
    X (void        , glVariantivEXT, (GLuint id, const GLint *addr)) \
    X (void        , glVariantfvEXT, (GLuint id, const GLfloat *addr)) \
    X (void        , glVariantdvEXT, (GLuint id, const GLdouble *addr)) \
    X (void        , glVariantubvEXT, (GLuint id, const GLubyte *addr)) \
    X (void        , glVariantusvEXT, (GLuint id, const GLushort *addr)) \
    X (void        , glVariantuivEXT, (GLuint id, const GLuint *addr)) \
    X (void        , glVariantPointerEXT, (GLuint id, GLenum type, GLuint stride, const void *addr)) \
    X (void        , glEnableVariantClientStateEXT, (GLuint id)) \
    X (void        , glDisableVariantClientStateEXT, (GLuint id)) \
    X (GLuint      , glBindLightParameterEXT, (GLenum light, GLenum value)) \
    X (GLuint      , glBindMaterialParameterEXT, (GLenum face, GLenum value)) \
    X (GLuint      , glBindTexGenParameterEXT, (GLenum unit, GLenum coord, GLenum value)) \
    X (GLuint      , glBindTextureUnitParameterEXT, (GLenum unit, GLenum value)) \
    X (GLuint      , glBindParameterEXT, (GLenum value)) \
    X (GLboolean   , glIsVariantEnabledEXT, (GLuint id, GLenum cap)) \
    X (void        , glGetVariantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (void        , glGetVariantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (void        , glGetVariantFloatvEXT, (GLuint id, GLenum value, GLfloat *data)) \
    X (void        , glGetVariantPointervEXT, (GLuint id, GLenum value, void **data)) \
    X (void        , glGetInvariantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (void        , glGetInvariantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (void        , glGetInvariantFloatvEXT, (GLuint id, GLenum value, GLfloat *data)) \
    X (void        , glGetLocalConstantBooleanvEXT, (GLuint id, GLenum value, GLboolean *data)) \
    X (void        , glGetLocalConstantIntegervEXT, (GLuint id, GLenum value, GLint *data)) \
    X (void        , glGetLocalConstantFloatvEXT, (GLuint id, GLenum value, GLfloat *data))

#define JUCE_GL_FUNCTIONS_GL_EXT_vertex_weighting \
    X (void        , glVertexWeightfEXT, (GLfloat weight)) \
    X (void        , glVertexWeightfvEXT, (const GLfloat *weight)) \
    X (void        , glVertexWeightPointerEXT, (GLint size, GLenum type, GLsizei stride, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    X (GLboolean   , glAcquireKeyedMutexWin32EXT, (GLuint memory, GLuint64 key, GLuint timeout)) \
    X (GLboolean   , glReleaseKeyedMutexWin32EXT, (GLuint memory, GLuint64 key))

#define JUCE_GL_FUNCTIONS_GL_EXT_window_rectangles \
    X (void        , glWindowRectanglesEXT, (GLenum mode, GLsizei count, const GLint *box))

#define JUCE_GL_FUNCTIONS_GL_EXT_x11_sync_object \
    X (GLsync      , glImportSyncEXT, (GLenum external_sync_type, GLintptr external_sync, GLbitfield flags))

#define JUCE_GL_FUNCTIONS_GL_GREMEDY_frame_terminator \
    X (void        , glFrameTerminatorGREMEDY, ())

#define JUCE_GL_FUNCTIONS_GL_GREMEDY_string_marker \
    X (void        , glStringMarkerGREMEDY, (GLsizei len, const void *string))

#define JUCE_GL_FUNCTIONS_GL_HP_image_transform \
    X (void        , glImageTransformParameteriHP, (GLenum target, GLenum pname, GLint param)) \
    X (void        , glImageTransformParameterfHP, (GLenum target, GLenum pname, GLfloat param)) \
    X (void        , glImageTransformParameterivHP, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glImageTransformParameterfvHP, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glGetImageTransformParameterivHP, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetImageTransformParameterfvHP, (GLenum target, GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_IBM_multimode_draw_arrays \
    X (void        , glMultiModeDrawArraysIBM, (const GLenum *mode, const GLint *first, const GLsizei *count, GLsizei primcount, GLint modestride)) \
    X (void        , glMultiModeDrawElementsIBM, (const GLenum *mode, const GLsizei *count, GLenum type, const void *const*indices, GLsizei primcount, GLint modestride))

#define JUCE_GL_FUNCTIONS_GL_IBM_static_data \
    X (void        , glFlushStaticDataIBM, (GLenum target))

#define JUCE_GL_FUNCTIONS_GL_IBM_vertex_array_lists \
    X (void        , glColorPointerListIBM, (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glSecondaryColorPointerListIBM, (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glEdgeFlagPointerListIBM, (GLint stride, const GLboolean **pointer, GLint ptrstride)) \
    X (void        , glFogCoordPointerListIBM, (GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glIndexPointerListIBM, (GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glNormalPointerListIBM, (GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glTexCoordPointerListIBM, (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride)) \
    X (void        , glVertexPointerListIBM, (GLint size, GLenum type, GLint stride, const void **pointer, GLint ptrstride))

#define JUCE_GL_FUNCTIONS_GL_INGR_blend_func_separate \
    X (void        , glBlendFuncSeparateINGR, (GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha))

#define JUCE_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    X (void        , glApplyFramebufferAttachmentCMAAINTEL, ())

#define JUCE_GL_FUNCTIONS_GL_INTEL_map_texture \
    X (void        , glSyncTextureINTEL, (GLuint texture)) \
    X (void        , glUnmapTexture2DINTEL, (GLuint texture, GLint level)) \
    X (void *      , glMapTexture2DINTEL, (GLuint texture, GLint level, GLbitfield access, GLint *stride, GLenum *layout))

#define JUCE_GL_FUNCTIONS_GL_INTEL_parallel_arrays \
    X (void        , glVertexPointervINTEL, (GLint size, GLenum type, const void **pointer)) \
    X (void        , glNormalPointervINTEL, (GLenum type, const void **pointer)) \
    X (void        , glColorPointervINTEL, (GLint size, GLenum type, const void **pointer)) \
    X (void        , glTexCoordPointervINTEL, (GLint size, GLenum type, const void **pointer))

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

#define JUCE_GL_FUNCTIONS_GL_MESA_resize_buffers \
    X (void        , glResizeBuffersMESA, ())

#define JUCE_GL_FUNCTIONS_GL_MESA_window_pos \
    X (void        , glWindowPos2dMESA, (GLdouble x, GLdouble y)) \
    X (void        , glWindowPos2dvMESA, (const GLdouble *v)) \
    X (void        , glWindowPos2fMESA, (GLfloat x, GLfloat y)) \
    X (void        , glWindowPos2fvMESA, (const GLfloat *v)) \
    X (void        , glWindowPos2iMESA, (GLint x, GLint y)) \
    X (void        , glWindowPos2ivMESA, (const GLint *v)) \
    X (void        , glWindowPos2sMESA, (GLshort x, GLshort y)) \
    X (void        , glWindowPos2svMESA, (const GLshort *v)) \
    X (void        , glWindowPos3dMESA, (GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glWindowPos3dvMESA, (const GLdouble *v)) \
    X (void        , glWindowPos3fMESA, (GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glWindowPos3fvMESA, (const GLfloat *v)) \
    X (void        , glWindowPos3iMESA, (GLint x, GLint y, GLint z)) \
    X (void        , glWindowPos3ivMESA, (const GLint *v)) \
    X (void        , glWindowPos3sMESA, (GLshort x, GLshort y, GLshort z)) \
    X (void        , glWindowPos3svMESA, (const GLshort *v)) \
    X (void        , glWindowPos4dMESA, (GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glWindowPos4dvMESA, (const GLdouble *v)) \
    X (void        , glWindowPos4fMESA, (GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glWindowPos4fvMESA, (const GLfloat *v)) \
    X (void        , glWindowPos4iMESA, (GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glWindowPos4ivMESA, (const GLint *v)) \
    X (void        , glWindowPos4sMESA, (GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glWindowPos4svMESA, (const GLshort *v))

#define JUCE_GL_FUNCTIONS_GL_NVX_conditional_render \
    X (void        , glBeginConditionalRenderNVX, (GLuint id)) \
    X (void        , glEndConditionalRenderNVX, ())

#define JUCE_GL_FUNCTIONS_GL_NVX_linked_gpu_multicast \
    X (void        , glLGPUNamedBufferSubDataNVX, (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (void        , glLGPUCopyImageSubDataNVX, (GLuint sourceGpu, GLbitfield destinationGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srxY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth)) \
    X (void        , glLGPUInterlockNVX, ())

#define JUCE_GL_FUNCTIONS_GL_NV_alpha_to_coverage_dither_control \
    X (void        , glAlphaToCoverageDitherControlNV, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect \
    X (void        , glMultiDrawArraysIndirectBindlessNV, (GLenum mode, const void *indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount)) \
    X (void        , glMultiDrawElementsIndirectBindlessNV, (GLenum mode, GLenum type, const void *indirect, GLsizei drawCount, GLsizei stride, GLint vertexBufferCount))

#define JUCE_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect_count \
    X (void        , glMultiDrawArraysIndirectBindlessCountNV, (GLenum mode, const void *indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount)) \
    X (void        , glMultiDrawElementsIndirectBindlessCountNV, (GLenum mode, GLenum type, const void *indirect, GLsizei drawCount, GLsizei maxDrawCount, GLsizei stride, GLint vertexBufferCount))

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

#define JUCE_GL_FUNCTIONS_GL_NV_command_list \
    X (void        , glCreateStatesNV, (GLsizei n, GLuint *states)) \
    X (void        , glDeleteStatesNV, (GLsizei n, const GLuint *states)) \
    X (GLboolean   , glIsStateNV, (GLuint state)) \
    X (void        , glStateCaptureNV, (GLuint state, GLenum mode)) \
    X (GLuint      , glGetCommandHeaderNV, (GLenum tokenID, GLuint size)) \
    X (GLushort    , glGetStageIndexNV, (GLenum shadertype)) \
    X (void        , glDrawCommandsNV, (GLenum primitiveMode, GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, GLuint count)) \
    X (void        , glDrawCommandsAddressNV, (GLenum primitiveMode, const GLuint64 *indirects, const GLsizei *sizes, GLuint count)) \
    X (void        , glDrawCommandsStatesNV, (GLuint buffer, const GLintptr *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (void        , glDrawCommandsStatesAddressNV, (const GLuint64 *indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (void        , glCreateCommandListsNV, (GLsizei n, GLuint *lists)) \
    X (void        , glDeleteCommandListsNV, (GLsizei n, const GLuint *lists)) \
    X (GLboolean   , glIsCommandListNV, (GLuint list)) \
    X (void        , glListDrawCommandsStatesClientNV, (GLuint list, GLuint segment, const void **indirects, const GLsizei *sizes, const GLuint *states, const GLuint *fbos, GLuint count)) \
    X (void        , glCommandListSegmentsNV, (GLuint list, GLuint segments)) \
    X (void        , glCompileCommandListNV, (GLuint list)) \
    X (void        , glCallCommandListNV, (GLuint list))

#define JUCE_GL_FUNCTIONS_GL_NV_conditional_render \
    X (void        , glBeginConditionalRenderNV, (GLuint id, GLenum mode)) \
    X (void        , glEndConditionalRenderNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_conservative_raster \
    X (void        , glSubpixelPrecisionBiasNV, (GLuint xbits, GLuint ybits))

#define JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_dilate \
    X (void        , glConservativeRasterParameterfNV, (GLenum pname, GLfloat value))

#define JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    X (void        , glConservativeRasterParameteriNV, (GLenum pname, GLint param))

#define JUCE_GL_FUNCTIONS_GL_NV_copy_image \
    X (void        , glCopyImageSubDataNV, (GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei width, GLsizei height, GLsizei depth))

#define JUCE_GL_FUNCTIONS_GL_NV_depth_buffer_float \
    X (void        , glDepthRangedNV, (GLdouble zNear, GLdouble zFar)) \
    X (void        , glClearDepthdNV, (GLdouble depth)) \
    X (void        , glDepthBoundsdNV, (GLdouble zmin, GLdouble zmax))

#define JUCE_GL_FUNCTIONS_GL_NV_draw_texture \
    X (void        , glDrawTextureNV, (GLuint texture, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1))

#define JUCE_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    X (void        , glDrawVkImageNV, (GLuint64 vkImage, GLuint sampler, GLfloat x0, GLfloat y0, GLfloat x1, GLfloat y1, GLfloat z, GLfloat s0, GLfloat t0, GLfloat s1, GLfloat t1)) \
    X (GLVULKANPROCNV, glGetVkProcAddrNV, (const GLchar *name)) \
    X (void        , glWaitVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (void        , glSignalVkSemaphoreNV, (GLuint64 vkSemaphore)) \
    X (void        , glSignalVkFenceNV, (GLuint64 vkFence))

#define JUCE_GL_FUNCTIONS_GL_NV_evaluators \
    X (void        , glMapControlPointsNV, (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const void *points)) \
    X (void        , glMapParameterivNV, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glMapParameterfvNV, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glGetMapControlPointsNV, (GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, void *points)) \
    X (void        , glGetMapParameterivNV, (GLenum target, GLenum pname, GLint *params)) \
    X (void        , glGetMapParameterfvNV, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetMapAttribParameterivNV, (GLenum target, GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetMapAttribParameterfvNV, (GLenum target, GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glEvalMapsNV, (GLenum target, GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_NV_explicit_multisample \
    X (void        , glGetMultisamplefvNV, (GLenum pname, GLuint index, GLfloat *val)) \
    X (void        , glSampleMaskIndexedNV, (GLuint index, GLbitfield mask)) \
    X (void        , glTexRenderbufferNV, (GLenum target, GLuint renderbuffer))

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

#define JUCE_GL_FUNCTIONS_GL_NV_fragment_program \
    X (void        , glProgramNamedParameter4fNV, (GLuint id, GLsizei len, const GLubyte *name, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glProgramNamedParameter4fvNV, (GLuint id, GLsizei len, const GLubyte *name, const GLfloat *v)) \
    X (void        , glProgramNamedParameter4dNV, (GLuint id, GLsizei len, const GLubyte *name, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glProgramNamedParameter4dvNV, (GLuint id, GLsizei len, const GLubyte *name, const GLdouble *v)) \
    X (void        , glGetProgramNamedParameterfvNV, (GLuint id, GLsizei len, const GLubyte *name, GLfloat *params)) \
    X (void        , glGetProgramNamedParameterdvNV, (GLuint id, GLsizei len, const GLubyte *name, GLdouble *params))

#define JUCE_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    X (void        , glCoverageModulationTableNV, (GLsizei n, const GLfloat *v)) \
    X (void        , glGetCoverageModulationTableNV, (GLsizei bufSize, GLfloat *v)) \
    X (void        , glCoverageModulationNV, (GLenum components))

#define JUCE_GL_FUNCTIONS_GL_NV_framebuffer_multisample_coverage \
    X (void        , glRenderbufferStorageMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLenum internalformat, GLsizei width, GLsizei height))

#define JUCE_GL_FUNCTIONS_GL_NV_geometry_program4 \
    X (void        , glProgramVertexLimitNV, (GLenum target, GLint limit)) \
    X (void        , glFramebufferTextureEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level)) \
    X (void        , glFramebufferTextureFaceEXT, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLenum face))

#define JUCE_GL_FUNCTIONS_GL_NV_gpu_program4 \
    X (void        , glProgramLocalParameterI4iNV, (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glProgramLocalParameterI4ivNV, (GLenum target, GLuint index, const GLint *params)) \
    X (void        , glProgramLocalParametersI4ivNV, (GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (void        , glProgramLocalParameterI4uiNV, (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glProgramLocalParameterI4uivNV, (GLenum target, GLuint index, const GLuint *params)) \
    X (void        , glProgramLocalParametersI4uivNV, (GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (void        , glProgramEnvParameterI4iNV, (GLenum target, GLuint index, GLint x, GLint y, GLint z, GLint w)) \
    X (void        , glProgramEnvParameterI4ivNV, (GLenum target, GLuint index, const GLint *params)) \
    X (void        , glProgramEnvParametersI4ivNV, (GLenum target, GLuint index, GLsizei count, const GLint *params)) \
    X (void        , glProgramEnvParameterI4uiNV, (GLenum target, GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)) \
    X (void        , glProgramEnvParameterI4uivNV, (GLenum target, GLuint index, const GLuint *params)) \
    X (void        , glProgramEnvParametersI4uivNV, (GLenum target, GLuint index, GLsizei count, const GLuint *params)) \
    X (void        , glGetProgramLocalParameterIivNV, (GLenum target, GLuint index, GLint *params)) \
    X (void        , glGetProgramLocalParameterIuivNV, (GLenum target, GLuint index, GLuint *params)) \
    X (void        , glGetProgramEnvParameterIivNV, (GLenum target, GLuint index, GLint *params)) \
    X (void        , glGetProgramEnvParameterIuivNV, (GLenum target, GLuint index, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_gpu_program5 \
    X (void        , glProgramSubroutineParametersuivNV, (GLenum target, GLsizei count, const GLuint *params)) \
    X (void        , glGetProgramSubroutineParameteruivNV, (GLenum target, GLuint index, GLuint *param))

#define JUCE_GL_FUNCTIONS_GL_NV_half_float \
    X (void        , glVertex2hNV, (GLhalfNV x, GLhalfNV y)) \
    X (void        , glVertex2hvNV, (const GLhalfNV *v)) \
    X (void        , glVertex3hNV, (GLhalfNV x, GLhalfNV y, GLhalfNV z)) \
    X (void        , glVertex3hvNV, (const GLhalfNV *v)) \
    X (void        , glVertex4hNV, (GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)) \
    X (void        , glVertex4hvNV, (const GLhalfNV *v)) \
    X (void        , glNormal3hNV, (GLhalfNV nx, GLhalfNV ny, GLhalfNV nz)) \
    X (void        , glNormal3hvNV, (const GLhalfNV *v)) \
    X (void        , glColor3hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue)) \
    X (void        , glColor3hvNV, (const GLhalfNV *v)) \
    X (void        , glColor4hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue, GLhalfNV alpha)) \
    X (void        , glColor4hvNV, (const GLhalfNV *v)) \
    X (void        , glTexCoord1hNV, (GLhalfNV s)) \
    X (void        , glTexCoord1hvNV, (const GLhalfNV *v)) \
    X (void        , glTexCoord2hNV, (GLhalfNV s, GLhalfNV t)) \
    X (void        , glTexCoord2hvNV, (const GLhalfNV *v)) \
    X (void        , glTexCoord3hNV, (GLhalfNV s, GLhalfNV t, GLhalfNV r)) \
    X (void        , glTexCoord3hvNV, (const GLhalfNV *v)) \
    X (void        , glTexCoord4hNV, (GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)) \
    X (void        , glTexCoord4hvNV, (const GLhalfNV *v)) \
    X (void        , glMultiTexCoord1hNV, (GLenum target, GLhalfNV s)) \
    X (void        , glMultiTexCoord1hvNV, (GLenum target, const GLhalfNV *v)) \
    X (void        , glMultiTexCoord2hNV, (GLenum target, GLhalfNV s, GLhalfNV t)) \
    X (void        , glMultiTexCoord2hvNV, (GLenum target, const GLhalfNV *v)) \
    X (void        , glMultiTexCoord3hNV, (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r)) \
    X (void        , glMultiTexCoord3hvNV, (GLenum target, const GLhalfNV *v)) \
    X (void        , glMultiTexCoord4hNV, (GLenum target, GLhalfNV s, GLhalfNV t, GLhalfNV r, GLhalfNV q)) \
    X (void        , glMultiTexCoord4hvNV, (GLenum target, const GLhalfNV *v)) \
    X (void        , glVertexAttrib1hNV, (GLuint index, GLhalfNV x)) \
    X (void        , glVertexAttrib1hvNV, (GLuint index, const GLhalfNV *v)) \
    X (void        , glVertexAttrib2hNV, (GLuint index, GLhalfNV x, GLhalfNV y)) \
    X (void        , glVertexAttrib2hvNV, (GLuint index, const GLhalfNV *v)) \
    X (void        , glVertexAttrib3hNV, (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z)) \
    X (void        , glVertexAttrib3hvNV, (GLuint index, const GLhalfNV *v)) \
    X (void        , glVertexAttrib4hNV, (GLuint index, GLhalfNV x, GLhalfNV y, GLhalfNV z, GLhalfNV w)) \
    X (void        , glVertexAttrib4hvNV, (GLuint index, const GLhalfNV *v)) \
    X (void        , glVertexAttribs1hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (void        , glVertexAttribs2hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (void        , glVertexAttribs3hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (void        , glVertexAttribs4hvNV, (GLuint index, GLsizei n, const GLhalfNV *v)) \
    X (void        , glFogCoordhNV, (GLhalfNV fog)) \
    X (void        , glFogCoordhvNV, (const GLhalfNV *fog)) \
    X (void        , glSecondaryColor3hNV, (GLhalfNV red, GLhalfNV green, GLhalfNV blue)) \
    X (void        , glSecondaryColor3hvNV, (const GLhalfNV *v)) \
    X (void        , glVertexWeighthNV, (GLhalfNV weight)) \
    X (void        , glVertexWeighthvNV, (const GLhalfNV *weight))

#define JUCE_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    X (void        , glGetInternalformatSampleivNV, (GLenum target, GLenum internalformat, GLsizei samples, GLenum pname, GLsizei count, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_gpu_multicast \
    X (void        , glRenderGpuMaskNV, (GLbitfield mask)) \
    X (void        , glMulticastBufferSubDataNV, (GLbitfield gpuMask, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data)) \
    X (void        , glMulticastCopyBufferSubDataNV, (GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size)) \
    X (void        , glMulticastCopyImageSubDataNV, (GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth)) \
    X (void        , glMulticastBlitFramebufferNV, (GLuint srcGpu, GLuint dstGpu, GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter)) \
    X (void        , glMulticastFramebufferSampleLocationsfvNV, (GLuint gpu, GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glMulticastBarrierNV, ()) \
    X (void        , glMulticastWaitSyncNV, (GLuint signalGpu, GLbitfield waitGpuMask)) \
    X (void        , glMulticastGetQueryObjectivNV, (GLuint gpu, GLuint id, GLenum pname, GLint *params)) \
    X (void        , glMulticastGetQueryObjectuivNV, (GLuint gpu, GLuint id, GLenum pname, GLuint *params)) \
    X (void        , glMulticastGetQueryObjecti64vNV, (GLuint gpu, GLuint id, GLenum pname, GLint64 *params)) \
    X (void        , glMulticastGetQueryObjectui64vNV, (GLuint gpu, GLuint id, GLenum pname, GLuint64 *params))

#define JUCE_GL_FUNCTIONS_GL_NVX_gpu_multicast2 \
    X (void        , glUploadGpuMaskNVX, (GLbitfield mask)) \
    X (void        , glMulticastViewportArrayvNVX, (GLuint gpu, GLuint first, GLsizei count, const GLfloat *v)) \
    X (void        , glMulticastViewportPositionWScaleNVX, (GLuint gpu, GLuint index, GLfloat xcoeff, GLfloat ycoeff)) \
    X (void        , glMulticastScissorArrayvNVX, (GLuint gpu, GLuint first, GLsizei count, const GLint *v)) \
    X (GLuint      , glAsyncCopyBufferSubDataNVX, (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *fenceValueArray, GLuint readGpu, GLbitfield writeGpuMask, GLuint readBuffer, GLuint writeBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray)) \
    X (GLuint      , glAsyncCopyImageSubDataNVX, (GLsizei waitSemaphoreCount, const GLuint *waitSemaphoreArray, const GLuint64 *waitValueArray, GLuint srcGpu, GLbitfield dstGpuMask, GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth, GLsizei signalSemaphoreCount, const GLuint *signalSemaphoreArray, const GLuint64 *signalValueArray))

#define JUCE_GL_FUNCTIONS_GL_NVX_progress_fence \
    X (GLuint      , glCreateProgressFenceNVX, ()) \
    X (void        , glSignalSemaphoreui64NVX, (GLuint signalGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray)) \
    X (void        , glWaitSemaphoreui64NVX, (GLuint waitGpu, GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray)) \
    X (void        , glClientWaitSemaphoreui64NVX, (GLsizei fenceObjectCount, const GLuint *semaphoreArray, const GLuint64 *fenceValueArray))

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

#define JUCE_GL_FUNCTIONS_GL_NV_occlusion_query \
    X (void        , glGenOcclusionQueriesNV, (GLsizei n, GLuint *ids)) \
    X (void        , glDeleteOcclusionQueriesNV, (GLsizei n, const GLuint *ids)) \
    X (GLboolean   , glIsOcclusionQueryNV, (GLuint id)) \
    X (void        , glBeginOcclusionQueryNV, (GLuint id)) \
    X (void        , glEndOcclusionQueryNV, ()) \
    X (void        , glGetOcclusionQueryivNV, (GLuint id, GLenum pname, GLint *params)) \
    X (void        , glGetOcclusionQueryuivNV, (GLuint id, GLenum pname, GLuint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_parameter_buffer_object \
    X (void        , glProgramBufferParametersfvNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLfloat *params)) \
    X (void        , glProgramBufferParametersIivNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLint *params)) \
    X (void        , glProgramBufferParametersIuivNV, (GLenum target, GLuint bindingIndex, GLuint wordIndex, GLsizei count, const GLuint *params))

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
    X (void        , glGetPathTexGenfvNV, (GLenum texCoordSet, GLenum pname, GLfloat *value))

#define JUCE_GL_FUNCTIONS_GL_NV_pixel_data_range \
    X (void        , glPixelDataRangeNV, (GLenum target, GLsizei length, const void *pointer)) \
    X (void        , glFlushPixelDataRangeNV, (GLenum target))

#define JUCE_GL_FUNCTIONS_GL_NV_point_sprite \
    X (void        , glPointParameteriNV, (GLenum pname, GLint param)) \
    X (void        , glPointParameterivNV, (GLenum pname, const GLint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_present_video \
    X (void        , glPresentFrameKeyedNV, (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLuint key0, GLenum target1, GLuint fill1, GLuint key1)) \
    X (void        , glPresentFrameDualFillNV, (GLuint video_slot, GLuint64EXT minPresentTime, GLuint beginPresentTimeId, GLuint presentDurationId, GLenum type, GLenum target0, GLuint fill0, GLenum target1, GLuint fill1, GLenum target2, GLuint fill2, GLenum target3, GLuint fill3)) \
    X (void        , glGetVideoivNV, (GLuint video_slot, GLenum pname, GLint *params)) \
    X (void        , glGetVideouivNV, (GLuint video_slot, GLenum pname, GLuint *params)) \
    X (void        , glGetVideoi64vNV, (GLuint video_slot, GLenum pname, GLint64EXT *params)) \
    X (void        , glGetVideoui64vNV, (GLuint video_slot, GLenum pname, GLuint64EXT *params))

#define JUCE_GL_FUNCTIONS_GL_NV_primitive_restart \
    X (void        , glPrimitiveRestartNV, ()) \
    X (void        , glPrimitiveRestartIndexNV, (GLuint index))

#define JUCE_GL_FUNCTIONS_GL_NV_query_resource \
    X (GLint       , glQueryResourceNV, (GLenum queryType, GLint tagId, GLuint count, GLint *buffer))

#define JUCE_GL_FUNCTIONS_GL_NV_query_resource_tag \
    X (void        , glGenQueryResourceTagNV, (GLsizei n, GLint *tagIds)) \
    X (void        , glDeleteQueryResourceTagNV, (GLsizei n, const GLint *tagIds)) \
    X (void        , glQueryResourceTagNV, (GLint tagId, const GLchar *tagString))

#define JUCE_GL_FUNCTIONS_GL_NV_register_combiners \
    X (void        , glCombinerParameterfvNV, (GLenum pname, const GLfloat *params)) \
    X (void        , glCombinerParameterfNV, (GLenum pname, GLfloat param)) \
    X (void        , glCombinerParameterivNV, (GLenum pname, const GLint *params)) \
    X (void        , glCombinerParameteriNV, (GLenum pname, GLint param)) \
    X (void        , glCombinerInputNV, (GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)) \
    X (void        , glCombinerOutputNV, (GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum)) \
    X (void        , glFinalCombinerInputNV, (GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage)) \
    X (void        , glGetCombinerInputParameterfvNV, (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat *params)) \
    X (void        , glGetCombinerInputParameterivNV, (GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint *params)) \
    X (void        , glGetCombinerOutputParameterfvNV, (GLenum stage, GLenum portion, GLenum pname, GLfloat *params)) \
    X (void        , glGetCombinerOutputParameterivNV, (GLenum stage, GLenum portion, GLenum pname, GLint *params)) \
    X (void        , glGetFinalCombinerInputParameterfvNV, (GLenum variable, GLenum pname, GLfloat *params)) \
    X (void        , glGetFinalCombinerInputParameterivNV, (GLenum variable, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_NV_register_combiners2 \
    X (void        , glCombinerStageParameterfvNV, (GLenum stage, GLenum pname, const GLfloat *params)) \
    X (void        , glGetCombinerStageParameterfvNV, (GLenum stage, GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_NV_sample_locations \
    X (void        , glFramebufferSampleLocationsfvNV, (GLenum target, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glNamedFramebufferSampleLocationsfvNV, (GLuint framebuffer, GLuint start, GLsizei count, const GLfloat *v)) \
    X (void        , glResolveDepthValuesNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    X (void        , glScissorExclusiveNV, (GLint x, GLint y, GLsizei width, GLsizei height)) \
    X (void        , glScissorExclusiveArrayvNV, (GLuint first, GLsizei count, const GLint *v))

#define JUCE_GL_FUNCTIONS_GL_NV_shader_buffer_load \
    X (void        , glMakeBufferResidentNV, (GLenum target, GLenum access)) \
    X (void        , glMakeBufferNonResidentNV, (GLenum target)) \
    X (GLboolean   , glIsBufferResidentNV, (GLenum target)) \
    X (void        , glMakeNamedBufferResidentNV, (GLuint buffer, GLenum access)) \
    X (void        , glMakeNamedBufferNonResidentNV, (GLuint buffer)) \
    X (GLboolean   , glIsNamedBufferResidentNV, (GLuint buffer)) \
    X (void        , glGetBufferParameterui64vNV, (GLenum target, GLenum pname, GLuint64EXT *params)) \
    X (void        , glGetNamedBufferParameterui64vNV, (GLuint buffer, GLenum pname, GLuint64EXT *params)) \
    X (void        , glGetIntegerui64vNV, (GLenum value, GLuint64EXT *result)) \
    X (void        , glUniformui64NV, (GLint location, GLuint64EXT value)) \
    X (void        , glUniformui64vNV, (GLint location, GLsizei count, const GLuint64EXT *value)) \
    X (void        , glProgramUniformui64NV, (GLuint program, GLint location, GLuint64EXT value)) \
    X (void        , glProgramUniformui64vNV, (GLuint program, GLint location, GLsizei count, const GLuint64EXT *value))

#define JUCE_GL_FUNCTIONS_GL_NV_shading_rate_image \
    X (void        , glBindShadingRateImageNV, (GLuint texture)) \
    X (void        , glGetShadingRateImagePaletteNV, (GLuint viewport, GLuint entry, GLenum *rate)) \
    X (void        , glGetShadingRateSampleLocationivNV, (GLenum rate, GLuint samples, GLuint index, GLint *location)) \
    X (void        , glShadingRateImageBarrierNV, (GLboolean synchronize)) \
    X (void        , glShadingRateImagePaletteNV, (GLuint viewport, GLuint first, GLsizei count, const GLenum *rates)) \
    X (void        , glShadingRateSampleOrderNV, (GLenum order)) \
    X (void        , glShadingRateSampleOrderCustomNV, (GLenum rate, GLuint samples, const GLint *locations))

#define JUCE_GL_FUNCTIONS_GL_NV_texture_barrier \
    X (void        , glTextureBarrierNV, ())

#define JUCE_GL_FUNCTIONS_GL_NV_texture_multisample \
    X (void        , glTexImage2DMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (void        , glTexImage3DMultisampleCoverageNV, (GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)) \
    X (void        , glTextureImage2DMultisampleNV, (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (void        , glTextureImage3DMultisampleNV, (GLuint texture, GLenum target, GLsizei samples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations)) \
    X (void        , glTextureImage2DMultisampleCoverageNV, (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLboolean fixedSampleLocations)) \
    X (void        , glTextureImage3DMultisampleCoverageNV, (GLuint texture, GLenum target, GLsizei coverageSamples, GLsizei colorSamples, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedSampleLocations))

#define JUCE_GL_FUNCTIONS_GL_NV_transform_feedback \
    X (void        , glBeginTransformFeedbackNV, (GLenum primitiveMode)) \
    X (void        , glEndTransformFeedbackNV, ()) \
    X (void        , glTransformFeedbackAttribsNV, (GLsizei count, const GLint *attribs, GLenum bufferMode)) \
    X (void        , glBindBufferRangeNV, (GLenum target, GLuint index, GLuint buffer, GLintptr offset, GLsizeiptr size)) \
    X (void        , glBindBufferOffsetNV, (GLenum target, GLuint index, GLuint buffer, GLintptr offset)) \
    X (void        , glBindBufferBaseNV, (GLenum target, GLuint index, GLuint buffer)) \
    X (void        , glTransformFeedbackVaryingsNV, (GLuint program, GLsizei count, const GLint *locations, GLenum bufferMode)) \
    X (void        , glActiveVaryingNV, (GLuint program, const GLchar *name)) \
    X (GLint       , glGetVaryingLocationNV, (GLuint program, const GLchar *name)) \
    X (void        , glGetActiveVaryingNV, (GLuint program, GLuint index, GLsizei bufSize, GLsizei *length, GLsizei *size, GLenum *type, GLchar *name)) \
    X (void        , glGetTransformFeedbackVaryingNV, (GLuint program, GLuint index, GLint *location)) \
    X (void        , glTransformFeedbackStreamAttribsNV, (GLsizei count, const GLint *attribs, GLsizei nbuffers, const GLint *bufstreams, GLenum bufferMode))

#define JUCE_GL_FUNCTIONS_GL_NV_transform_feedback2 \
    X (void        , glBindTransformFeedbackNV, (GLenum target, GLuint id)) \
    X (void        , glDeleteTransformFeedbacksNV, (GLsizei n, const GLuint *ids)) \
    X (void        , glGenTransformFeedbacksNV, (GLsizei n, GLuint *ids)) \
    X (GLboolean   , glIsTransformFeedbackNV, (GLuint id)) \
    X (void        , glPauseTransformFeedbackNV, ()) \
    X (void        , glResumeTransformFeedbackNV, ()) \
    X (void        , glDrawTransformFeedbackNV, (GLenum mode, GLuint id))

#define JUCE_GL_FUNCTIONS_GL_NV_vdpau_interop \
    X (void        , glVDPAUInitNV, (const void *vdpDevice, const void *getProcAddress)) \
    X (void        , glVDPAUFiniNV, ()) \
    X (GLvdpauSurfaceNV, glVDPAURegisterVideoSurfaceNV, (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)) \
    X (GLvdpauSurfaceNV, glVDPAURegisterOutputSurfaceNV, (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames)) \
    X (GLboolean   , glVDPAUIsSurfaceNV, (GLvdpauSurfaceNV surface)) \
    X (void        , glVDPAUUnregisterSurfaceNV, (GLvdpauSurfaceNV surface)) \
    X (void        , glVDPAUGetSurfaceivNV, (GLvdpauSurfaceNV surface, GLenum pname, GLsizei count, GLsizei *length, GLint *values)) \
    X (void        , glVDPAUSurfaceAccessNV, (GLvdpauSurfaceNV surface, GLenum access)) \
    X (void        , glVDPAUMapSurfacesNV, (GLsizei numSurfaces, const GLvdpauSurfaceNV *surfaces)) \
    X (void        , glVDPAUUnmapSurfacesNV, (GLsizei numSurface, const GLvdpauSurfaceNV *surfaces))

#define JUCE_GL_FUNCTIONS_GL_NV_vdpau_interop2 \
    X (GLvdpauSurfaceNV, glVDPAURegisterVideoSurfaceWithPictureStructureNV, (const void *vdpSurface, GLenum target, GLsizei numTextureNames, const GLuint *textureNames, GLboolean isFrameStructure))

#define JUCE_GL_FUNCTIONS_GL_NV_vertex_array_range \
    X (void        , glFlushVertexArrayRangeNV, ()) \
    X (void        , glVertexArrayRangeNV, (GLsizei length, const void *pointer))

#define JUCE_GL_FUNCTIONS_GL_NV_vertex_attrib_integer_64bit \
    X (void        , glVertexAttribL1i64NV, (GLuint index, GLint64EXT x)) \
    X (void        , glVertexAttribL2i64NV, (GLuint index, GLint64EXT x, GLint64EXT y)) \
    X (void        , glVertexAttribL3i64NV, (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z)) \
    X (void        , glVertexAttribL4i64NV, (GLuint index, GLint64EXT x, GLint64EXT y, GLint64EXT z, GLint64EXT w)) \
    X (void        , glVertexAttribL1i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (void        , glVertexAttribL2i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (void        , glVertexAttribL3i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (void        , glVertexAttribL4i64vNV, (GLuint index, const GLint64EXT *v)) \
    X (void        , glVertexAttribL1ui64NV, (GLuint index, GLuint64EXT x)) \
    X (void        , glVertexAttribL2ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y)) \
    X (void        , glVertexAttribL3ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z)) \
    X (void        , glVertexAttribL4ui64NV, (GLuint index, GLuint64EXT x, GLuint64EXT y, GLuint64EXT z, GLuint64EXT w)) \
    X (void        , glVertexAttribL1ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (void        , glVertexAttribL2ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (void        , glVertexAttribL3ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (void        , glVertexAttribL4ui64vNV, (GLuint index, const GLuint64EXT *v)) \
    X (void        , glGetVertexAttribLi64vNV, (GLuint index, GLenum pname, GLint64EXT *params)) \
    X (void        , glGetVertexAttribLui64vNV, (GLuint index, GLenum pname, GLuint64EXT *params)) \
    X (void        , glVertexAttribLFormatNV, (GLuint index, GLint size, GLenum type, GLsizei stride))

#define JUCE_GL_FUNCTIONS_GL_NV_vertex_buffer_unified_memory \
    X (void        , glBufferAddressRangeNV, (GLenum pname, GLuint index, GLuint64EXT address, GLsizeiptr length)) \
    X (void        , glVertexFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (void        , glNormalFormatNV, (GLenum type, GLsizei stride)) \
    X (void        , glColorFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (void        , glIndexFormatNV, (GLenum type, GLsizei stride)) \
    X (void        , glTexCoordFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (void        , glEdgeFlagFormatNV, (GLsizei stride)) \
    X (void        , glSecondaryColorFormatNV, (GLint size, GLenum type, GLsizei stride)) \
    X (void        , glFogCoordFormatNV, (GLenum type, GLsizei stride)) \
    X (void        , glVertexAttribFormatNV, (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride)) \
    X (void        , glVertexAttribIFormatNV, (GLuint index, GLint size, GLenum type, GLsizei stride)) \
    X (void        , glGetIntegerui64i_vNV, (GLenum value, GLuint index, GLuint64EXT *result))

#define JUCE_GL_FUNCTIONS_GL_NV_vertex_program \
    X (GLboolean   , glAreProgramsResidentNV, (GLsizei n, const GLuint *programs, GLboolean *residences)) \
    X (void        , glBindProgramNV, (GLenum target, GLuint id)) \
    X (void        , glDeleteProgramsNV, (GLsizei n, const GLuint *programs)) \
    X (void        , glExecuteProgramNV, (GLenum target, GLuint id, const GLfloat *params)) \
    X (void        , glGenProgramsNV, (GLsizei n, GLuint *programs)) \
    X (void        , glGetProgramParameterdvNV, (GLenum target, GLuint index, GLenum pname, GLdouble *params)) \
    X (void        , glGetProgramParameterfvNV, (GLenum target, GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetProgramivNV, (GLuint id, GLenum pname, GLint *params)) \
    X (void        , glGetProgramStringNV, (GLuint id, GLenum pname, GLubyte *program)) \
    X (void        , glGetTrackMatrixivNV, (GLenum target, GLuint address, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribdvNV, (GLuint index, GLenum pname, GLdouble *params)) \
    X (void        , glGetVertexAttribfvNV, (GLuint index, GLenum pname, GLfloat *params)) \
    X (void        , glGetVertexAttribivNV, (GLuint index, GLenum pname, GLint *params)) \
    X (void        , glGetVertexAttribPointervNV, (GLuint index, GLenum pname, void **pointer)) \
    X (GLboolean   , glIsProgramNV, (GLuint id)) \
    X (void        , glLoadProgramNV, (GLenum target, GLuint id, GLsizei len, const GLubyte *program)) \
    X (void        , glProgramParameter4dNV, (GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glProgramParameter4dvNV, (GLenum target, GLuint index, const GLdouble *v)) \
    X (void        , glProgramParameter4fNV, (GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glProgramParameter4fvNV, (GLenum target, GLuint index, const GLfloat *v)) \
    X (void        , glProgramParameters4dvNV, (GLenum target, GLuint index, GLsizei count, const GLdouble *v)) \
    X (void        , glProgramParameters4fvNV, (GLenum target, GLuint index, GLsizei count, const GLfloat *v)) \
    X (void        , glRequestResidentProgramsNV, (GLsizei n, const GLuint *programs)) \
    X (void        , glTrackMatrixNV, (GLenum target, GLuint address, GLenum matrix, GLenum transform)) \
    X (void        , glVertexAttribPointerNV, (GLuint index, GLint fsize, GLenum type, GLsizei stride, const void *pointer)) \
    X (void        , glVertexAttrib1dNV, (GLuint index, GLdouble x)) \
    X (void        , glVertexAttrib1dvNV, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib1fNV, (GLuint index, GLfloat x)) \
    X (void        , glVertexAttrib1fvNV, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib1sNV, (GLuint index, GLshort x)) \
    X (void        , glVertexAttrib1svNV, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib2dNV, (GLuint index, GLdouble x, GLdouble y)) \
    X (void        , glVertexAttrib2dvNV, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib2fNV, (GLuint index, GLfloat x, GLfloat y)) \
    X (void        , glVertexAttrib2fvNV, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib2sNV, (GLuint index, GLshort x, GLshort y)) \
    X (void        , glVertexAttrib2svNV, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib3dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z)) \
    X (void        , glVertexAttrib3dvNV, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib3fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glVertexAttrib3fvNV, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib3sNV, (GLuint index, GLshort x, GLshort y, GLshort z)) \
    X (void        , glVertexAttrib3svNV, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4dNV, (GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w)) \
    X (void        , glVertexAttrib4dvNV, (GLuint index, const GLdouble *v)) \
    X (void        , glVertexAttrib4fNV, (GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glVertexAttrib4fvNV, (GLuint index, const GLfloat *v)) \
    X (void        , glVertexAttrib4sNV, (GLuint index, GLshort x, GLshort y, GLshort z, GLshort w)) \
    X (void        , glVertexAttrib4svNV, (GLuint index, const GLshort *v)) \
    X (void        , glVertexAttrib4ubNV, (GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w)) \
    X (void        , glVertexAttrib4ubvNV, (GLuint index, const GLubyte *v)) \
    X (void        , glVertexAttribs1dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (void        , glVertexAttribs1fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (void        , glVertexAttribs1svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (void        , glVertexAttribs2dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (void        , glVertexAttribs2fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (void        , glVertexAttribs2svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (void        , glVertexAttribs3dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (void        , glVertexAttribs3fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (void        , glVertexAttribs3svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (void        , glVertexAttribs4dvNV, (GLuint index, GLsizei count, const GLdouble *v)) \
    X (void        , glVertexAttribs4fvNV, (GLuint index, GLsizei count, const GLfloat *v)) \
    X (void        , glVertexAttribs4svNV, (GLuint index, GLsizei count, const GLshort *v)) \
    X (void        , glVertexAttribs4ubvNV, (GLuint index, GLsizei count, const GLubyte *v))

#define JUCE_GL_FUNCTIONS_GL_NV_video_capture \
    X (void        , glBeginVideoCaptureNV, (GLuint video_capture_slot)) \
    X (void        , glBindVideoCaptureStreamBufferNV, (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLintptrARB offset)) \
    X (void        , glBindVideoCaptureStreamTextureNV, (GLuint video_capture_slot, GLuint stream, GLenum frame_region, GLenum target, GLuint texture)) \
    X (void        , glEndVideoCaptureNV, (GLuint video_capture_slot)) \
    X (void        , glGetVideoCaptureivNV, (GLuint video_capture_slot, GLenum pname, GLint *params)) \
    X (void        , glGetVideoCaptureStreamivNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLint *params)) \
    X (void        , glGetVideoCaptureStreamfvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLfloat *params)) \
    X (void        , glGetVideoCaptureStreamdvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, GLdouble *params)) \
    X (GLenum      , glVideoCaptureNV, (GLuint video_capture_slot, GLuint *sequence_num, GLuint64EXT *capture_time)) \
    X (void        , glVideoCaptureStreamParameterivNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLint *params)) \
    X (void        , glVideoCaptureStreamParameterfvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLfloat *params)) \
    X (void        , glVideoCaptureStreamParameterdvNV, (GLuint video_capture_slot, GLuint stream, GLenum pname, const GLdouble *params))

#define JUCE_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    X (void        , glViewportSwizzleNV, (GLuint index, GLenum swizzlex, GLenum swizzley, GLenum swizzlez, GLenum swizzlew))

#define JUCE_GL_FUNCTIONS_GL_OES_byte_coordinates \
    X (void        , glMultiTexCoord1bOES, (GLenum texture, GLbyte s)) \
    X (void        , glMultiTexCoord1bvOES, (GLenum texture, const GLbyte *coords)) \
    X (void        , glMultiTexCoord2bOES, (GLenum texture, GLbyte s, GLbyte t)) \
    X (void        , glMultiTexCoord2bvOES, (GLenum texture, const GLbyte *coords)) \
    X (void        , glMultiTexCoord3bOES, (GLenum texture, GLbyte s, GLbyte t, GLbyte r)) \
    X (void        , glMultiTexCoord3bvOES, (GLenum texture, const GLbyte *coords)) \
    X (void        , glMultiTexCoord4bOES, (GLenum texture, GLbyte s, GLbyte t, GLbyte r, GLbyte q)) \
    X (void        , glMultiTexCoord4bvOES, (GLenum texture, const GLbyte *coords)) \
    X (void        , glTexCoord1bOES, (GLbyte s)) \
    X (void        , glTexCoord1bvOES, (const GLbyte *coords)) \
    X (void        , glTexCoord2bOES, (GLbyte s, GLbyte t)) \
    X (void        , glTexCoord2bvOES, (const GLbyte *coords)) \
    X (void        , glTexCoord3bOES, (GLbyte s, GLbyte t, GLbyte r)) \
    X (void        , glTexCoord3bvOES, (const GLbyte *coords)) \
    X (void        , glTexCoord4bOES, (GLbyte s, GLbyte t, GLbyte r, GLbyte q)) \
    X (void        , glTexCoord4bvOES, (const GLbyte *coords)) \
    X (void        , glVertex2bOES, (GLbyte x, GLbyte y)) \
    X (void        , glVertex2bvOES, (const GLbyte *coords)) \
    X (void        , glVertex3bOES, (GLbyte x, GLbyte y, GLbyte z)) \
    X (void        , glVertex3bvOES, (const GLbyte *coords)) \
    X (void        , glVertex4bOES, (GLbyte x, GLbyte y, GLbyte z, GLbyte w)) \
    X (void        , glVertex4bvOES, (const GLbyte *coords))

#define JUCE_GL_FUNCTIONS_GL_OES_fixed_point \
    X (void        , glAlphaFuncxOES, (GLenum func, GLfixed ref)) \
    X (void        , glClearColorxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (void        , glClearDepthxOES, (GLfixed depth)) \
    X (void        , glClipPlanexOES, (GLenum plane, const GLfixed *equation)) \
    X (void        , glColor4xOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (void        , glDepthRangexOES, (GLfixed n, GLfixed f)) \
    X (void        , glFogxOES, (GLenum pname, GLfixed param)) \
    X (void        , glFogxvOES, (GLenum pname, const GLfixed *param)) \
    X (void        , glFrustumxOES, (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)) \
    X (void        , glGetClipPlanexOES, (GLenum plane, GLfixed *equation)) \
    X (void        , glGetFixedvOES, (GLenum pname, GLfixed *params)) \
    X (void        , glGetTexEnvxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (void        , glGetTexParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (void        , glLightModelxOES, (GLenum pname, GLfixed param)) \
    X (void        , glLightModelxvOES, (GLenum pname, const GLfixed *param)) \
    X (void        , glLightxOES, (GLenum light, GLenum pname, GLfixed param)) \
    X (void        , glLightxvOES, (GLenum light, GLenum pname, const GLfixed *params)) \
    X (void        , glLineWidthxOES, (GLfixed width)) \
    X (void        , glLoadMatrixxOES, (const GLfixed *m)) \
    X (void        , glMaterialxOES, (GLenum face, GLenum pname, GLfixed param)) \
    X (void        , glMaterialxvOES, (GLenum face, GLenum pname, const GLfixed *param)) \
    X (void        , glMultMatrixxOES, (const GLfixed *m)) \
    X (void        , glMultiTexCoord4xOES, (GLenum texture, GLfixed s, GLfixed t, GLfixed r, GLfixed q)) \
    X (void        , glNormal3xOES, (GLfixed nx, GLfixed ny, GLfixed nz)) \
    X (void        , glOrthoxOES, (GLfixed l, GLfixed r, GLfixed b, GLfixed t, GLfixed n, GLfixed f)) \
    X (void        , glPointParameterxvOES, (GLenum pname, const GLfixed *params)) \
    X (void        , glPointSizexOES, (GLfixed size)) \
    X (void        , glPolygonOffsetxOES, (GLfixed factor, GLfixed units)) \
    X (void        , glRotatexOES, (GLfixed angle, GLfixed x, GLfixed y, GLfixed z)) \
    X (void        , glScalexOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (void        , glTexEnvxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (void        , glTexEnvxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (void        , glTexParameterxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (void        , glTexParameterxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (void        , glTranslatexOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (void        , glGetLightxvOES, (GLenum light, GLenum pname, GLfixed *params)) \
    X (void        , glGetMaterialxvOES, (GLenum face, GLenum pname, GLfixed *params)) \
    X (void        , glPointParameterxOES, (GLenum pname, GLfixed param)) \
    X (void        , glSampleCoveragexOES, (GLclampx value, GLboolean invert)) \
    X (void        , glAccumxOES, (GLenum op, GLfixed value)) \
    X (void        , glBitmapxOES, (GLsizei width, GLsizei height, GLfixed xorig, GLfixed yorig, GLfixed xmove, GLfixed ymove, const GLubyte *bitmap)) \
    X (void        , glBlendColorxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (void        , glClearAccumxOES, (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)) \
    X (void        , glColor3xOES, (GLfixed red, GLfixed green, GLfixed blue)) \
    X (void        , glColor3xvOES, (const GLfixed *components)) \
    X (void        , glColor4xvOES, (const GLfixed *components)) \
    X (void        , glConvolutionParameterxOES, (GLenum target, GLenum pname, GLfixed param)) \
    X (void        , glConvolutionParameterxvOES, (GLenum target, GLenum pname, const GLfixed *params)) \
    X (void        , glEvalCoord1xOES, (GLfixed u)) \
    X (void        , glEvalCoord1xvOES, (const GLfixed *coords)) \
    X (void        , glEvalCoord2xOES, (GLfixed u, GLfixed v)) \
    X (void        , glEvalCoord2xvOES, (const GLfixed *coords)) \
    X (void        , glFeedbackBufferxOES, (GLsizei n, GLenum type, const GLfixed *buffer)) \
    X (void        , glGetConvolutionParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (void        , glGetHistogramParameterxvOES, (GLenum target, GLenum pname, GLfixed *params)) \
    X (void        , glGetLightxOES, (GLenum light, GLenum pname, GLfixed *params)) \
    X (void        , glGetMapxvOES, (GLenum target, GLenum query, GLfixed *v)) \
    X (void        , glGetMaterialxOES, (GLenum face, GLenum pname, GLfixed param)) \
    X (void        , glGetPixelMapxv, (GLenum map, GLint size, GLfixed *values)) \
    X (void        , glGetTexGenxvOES, (GLenum coord, GLenum pname, GLfixed *params)) \
    X (void        , glGetTexLevelParameterxvOES, (GLenum target, GLint level, GLenum pname, GLfixed *params)) \
    X (void        , glIndexxOES, (GLfixed component)) \
    X (void        , glIndexxvOES, (const GLfixed *component)) \
    X (void        , glLoadTransposeMatrixxOES, (const GLfixed *m)) \
    X (void        , glMap1xOES, (GLenum target, GLfixed u1, GLfixed u2, GLint stride, GLint order, GLfixed points)) \
    X (void        , glMap2xOES, (GLenum target, GLfixed u1, GLfixed u2, GLint ustride, GLint uorder, GLfixed v1, GLfixed v2, GLint vstride, GLint vorder, GLfixed points)) \
    X (void        , glMapGrid1xOES, (GLint n, GLfixed u1, GLfixed u2)) \
    X (void        , glMapGrid2xOES, (GLint n, GLfixed u1, GLfixed u2, GLfixed v1, GLfixed v2)) \
    X (void        , glMultTransposeMatrixxOES, (const GLfixed *m)) \
    X (void        , glMultiTexCoord1xOES, (GLenum texture, GLfixed s)) \
    X (void        , glMultiTexCoord1xvOES, (GLenum texture, const GLfixed *coords)) \
    X (void        , glMultiTexCoord2xOES, (GLenum texture, GLfixed s, GLfixed t)) \
    X (void        , glMultiTexCoord2xvOES, (GLenum texture, const GLfixed *coords)) \
    X (void        , glMultiTexCoord3xOES, (GLenum texture, GLfixed s, GLfixed t, GLfixed r)) \
    X (void        , glMultiTexCoord3xvOES, (GLenum texture, const GLfixed *coords)) \
    X (void        , glMultiTexCoord4xvOES, (GLenum texture, const GLfixed *coords)) \
    X (void        , glNormal3xvOES, (const GLfixed *coords)) \
    X (void        , glPassThroughxOES, (GLfixed token)) \
    X (void        , glPixelMapx, (GLenum map, GLint size, const GLfixed *values)) \
    X (void        , glPixelStorex, (GLenum pname, GLfixed param)) \
    X (void        , glPixelTransferxOES, (GLenum pname, GLfixed param)) \
    X (void        , glPixelZoomxOES, (GLfixed xfactor, GLfixed yfactor)) \
    X (void        , glPrioritizeTexturesxOES, (GLsizei n, const GLuint *textures, const GLfixed *priorities)) \
    X (void        , glRasterPos2xOES, (GLfixed x, GLfixed y)) \
    X (void        , glRasterPos2xvOES, (const GLfixed *coords)) \
    X (void        , glRasterPos3xOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (void        , glRasterPos3xvOES, (const GLfixed *coords)) \
    X (void        , glRasterPos4xOES, (GLfixed x, GLfixed y, GLfixed z, GLfixed w)) \
    X (void        , glRasterPos4xvOES, (const GLfixed *coords)) \
    X (void        , glRectxOES, (GLfixed x1, GLfixed y1, GLfixed x2, GLfixed y2)) \
    X (void        , glRectxvOES, (const GLfixed *v1, const GLfixed *v2)) \
    X (void        , glTexCoord1xOES, (GLfixed s)) \
    X (void        , glTexCoord1xvOES, (const GLfixed *coords)) \
    X (void        , glTexCoord2xOES, (GLfixed s, GLfixed t)) \
    X (void        , glTexCoord2xvOES, (const GLfixed *coords)) \
    X (void        , glTexCoord3xOES, (GLfixed s, GLfixed t, GLfixed r)) \
    X (void        , glTexCoord3xvOES, (const GLfixed *coords)) \
    X (void        , glTexCoord4xOES, (GLfixed s, GLfixed t, GLfixed r, GLfixed q)) \
    X (void        , glTexCoord4xvOES, (const GLfixed *coords)) \
    X (void        , glTexGenxOES, (GLenum coord, GLenum pname, GLfixed param)) \
    X (void        , glTexGenxvOES, (GLenum coord, GLenum pname, const GLfixed *params)) \
    X (void        , glVertex2xOES, (GLfixed x)) \
    X (void        , glVertex2xvOES, (const GLfixed *coords)) \
    X (void        , glVertex3xOES, (GLfixed x, GLfixed y)) \
    X (void        , glVertex3xvOES, (const GLfixed *coords)) \
    X (void        , glVertex4xOES, (GLfixed x, GLfixed y, GLfixed z)) \
    X (void        , glVertex4xvOES, (const GLfixed *coords))

#define JUCE_GL_FUNCTIONS_GL_OES_query_matrix \
    X (GLbitfield  , glQueryMatrixxOES, (GLfixed *mantissa, GLint *exponent))

#define JUCE_GL_FUNCTIONS_GL_OES_single_precision \
    X (void        , glClearDepthfOES, (GLclampf depth)) \
    X (void        , glClipPlanefOES, (GLenum plane, const GLfloat *equation)) \
    X (void        , glDepthRangefOES, (GLclampf n, GLclampf f)) \
    X (void        , glFrustumfOES, (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f)) \
    X (void        , glGetClipPlanefOES, (GLenum plane, GLfloat *equation)) \
    X (void        , glOrthofOES, (GLfloat l, GLfloat r, GLfloat b, GLfloat t, GLfloat n, GLfloat f))

#define JUCE_GL_FUNCTIONS_GL_OVR_multiview \
    X (void        , glFramebufferTextureMultiviewOVR, (GLenum target, GLenum attachment, GLuint texture, GLint level, GLint baseViewIndex, GLsizei numViews))

#define JUCE_GL_FUNCTIONS_GL_PGI_misc_hints \
    X (void        , glHintPGI, (GLenum target, GLint mode))

#define JUCE_GL_FUNCTIONS_GL_SGIS_detail_texture \
    X (void        , glDetailTexFuncSGIS, (GLenum target, GLsizei n, const GLfloat *points)) \
    X (void        , glGetDetailTexFuncSGIS, (GLenum target, GLfloat *points))

#define JUCE_GL_FUNCTIONS_GL_SGIS_fog_function \
    X (void        , glFogFuncSGIS, (GLsizei n, const GLfloat *points)) \
    X (void        , glGetFogFuncSGIS, (GLfloat *points))

#define JUCE_GL_FUNCTIONS_GL_SGIS_multisample \
    X (void        , glSampleMaskSGIS, (GLclampf value, GLboolean invert)) \
    X (void        , glSamplePatternSGIS, (GLenum pattern))

#define JUCE_GL_FUNCTIONS_GL_SGIS_pixel_texture \
    X (void        , glPixelTexGenParameteriSGIS, (GLenum pname, GLint param)) \
    X (void        , glPixelTexGenParameterivSGIS, (GLenum pname, const GLint *params)) \
    X (void        , glPixelTexGenParameterfSGIS, (GLenum pname, GLfloat param)) \
    X (void        , glPixelTexGenParameterfvSGIS, (GLenum pname, const GLfloat *params)) \
    X (void        , glGetPixelTexGenParameterivSGIS, (GLenum pname, GLint *params)) \
    X (void        , glGetPixelTexGenParameterfvSGIS, (GLenum pname, GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_SGIS_point_parameters \
    X (void        , glPointParameterfSGIS, (GLenum pname, GLfloat param)) \
    X (void        , glPointParameterfvSGIS, (GLenum pname, const GLfloat *params))

#define JUCE_GL_FUNCTIONS_GL_SGIS_sharpen_texture \
    X (void        , glSharpenTexFuncSGIS, (GLenum target, GLsizei n, const GLfloat *points)) \
    X (void        , glGetSharpenTexFuncSGIS, (GLenum target, GLfloat *points))

#define JUCE_GL_FUNCTIONS_GL_SGIS_texture4D \
    X (void        , glTexImage4DSGIS, (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const void *pixels)) \
    X (void        , glTexSubImage4DSGIS, (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, const void *pixels))

#define JUCE_GL_FUNCTIONS_GL_SGIS_texture_color_mask \
    X (void        , glTextureColorMaskSGIS, (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha))

#define JUCE_GL_FUNCTIONS_GL_SGIS_texture_filter4 \
    X (void        , glGetTexFilterFuncSGIS, (GLenum target, GLenum filter, GLfloat *weights)) \
    X (void        , glTexFilterFuncSGIS, (GLenum target, GLenum filter, GLsizei n, const GLfloat *weights))

#define JUCE_GL_FUNCTIONS_GL_SGIX_async \
    X (void        , glAsyncMarkerSGIX, (GLuint marker)) \
    X (GLint       , glFinishAsyncSGIX, (GLuint *markerp)) \
    X (GLint       , glPollAsyncSGIX, (GLuint *markerp)) \
    X (GLuint      , glGenAsyncMarkersSGIX, (GLsizei range)) \
    X (void        , glDeleteAsyncMarkersSGIX, (GLuint marker, GLsizei range)) \
    X (GLboolean   , glIsAsyncMarkerSGIX, (GLuint marker))

#define JUCE_GL_FUNCTIONS_GL_SGIX_flush_raster \
    X (void        , glFlushRasterSGIX, ())

#define JUCE_GL_FUNCTIONS_GL_SGIX_fragment_lighting \
    X (void        , glFragmentColorMaterialSGIX, (GLenum face, GLenum mode)) \
    X (void        , glFragmentLightfSGIX, (GLenum light, GLenum pname, GLfloat param)) \
    X (void        , glFragmentLightfvSGIX, (GLenum light, GLenum pname, const GLfloat *params)) \
    X (void        , glFragmentLightiSGIX, (GLenum light, GLenum pname, GLint param)) \
    X (void        , glFragmentLightivSGIX, (GLenum light, GLenum pname, const GLint *params)) \
    X (void        , glFragmentLightModelfSGIX, (GLenum pname, GLfloat param)) \
    X (void        , glFragmentLightModelfvSGIX, (GLenum pname, const GLfloat *params)) \
    X (void        , glFragmentLightModeliSGIX, (GLenum pname, GLint param)) \
    X (void        , glFragmentLightModelivSGIX, (GLenum pname, const GLint *params)) \
    X (void        , glFragmentMaterialfSGIX, (GLenum face, GLenum pname, GLfloat param)) \
    X (void        , glFragmentMaterialfvSGIX, (GLenum face, GLenum pname, const GLfloat *params)) \
    X (void        , glFragmentMaterialiSGIX, (GLenum face, GLenum pname, GLint param)) \
    X (void        , glFragmentMaterialivSGIX, (GLenum face, GLenum pname, const GLint *params)) \
    X (void        , glGetFragmentLightfvSGIX, (GLenum light, GLenum pname, GLfloat *params)) \
    X (void        , glGetFragmentLightivSGIX, (GLenum light, GLenum pname, GLint *params)) \
    X (void        , glGetFragmentMaterialfvSGIX, (GLenum face, GLenum pname, GLfloat *params)) \
    X (void        , glGetFragmentMaterialivSGIX, (GLenum face, GLenum pname, GLint *params)) \
    X (void        , glLightEnviSGIX, (GLenum pname, GLint param))

#define JUCE_GL_FUNCTIONS_GL_SGIX_framezoom \
    X (void        , glFrameZoomSGIX, (GLint factor))

#define JUCE_GL_FUNCTIONS_GL_SGIX_igloo_interface \
    X (void        , glIglooInterfaceSGIX, (GLenum pname, const void *params))

#define JUCE_GL_FUNCTIONS_GL_SGIX_instruments \
    X (GLint       , glGetInstrumentsSGIX, ()) \
    X (void        , glInstrumentsBufferSGIX, (GLsizei size, GLint *buffer)) \
    X (GLint       , glPollInstrumentsSGIX, (GLint *marker_p)) \
    X (void        , glReadInstrumentsSGIX, (GLint marker)) \
    X (void        , glStartInstrumentsSGIX, ()) \
    X (void        , glStopInstrumentsSGIX, (GLint marker))

#define JUCE_GL_FUNCTIONS_GL_SGIX_list_priority \
    X (void        , glGetListParameterfvSGIX, (GLuint list, GLenum pname, GLfloat *params)) \
    X (void        , glGetListParameterivSGIX, (GLuint list, GLenum pname, GLint *params)) \
    X (void        , glListParameterfSGIX, (GLuint list, GLenum pname, GLfloat param)) \
    X (void        , glListParameterfvSGIX, (GLuint list, GLenum pname, const GLfloat *params)) \
    X (void        , glListParameteriSGIX, (GLuint list, GLenum pname, GLint param)) \
    X (void        , glListParameterivSGIX, (GLuint list, GLenum pname, const GLint *params))

#define JUCE_GL_FUNCTIONS_GL_SGIX_pixel_texture \
    X (void        , glPixelTexGenSGIX, (GLenum mode))

#define JUCE_GL_FUNCTIONS_GL_SGIX_polynomial_ffd \
    X (void        , glDeformationMap3dSGIX, (GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble *points)) \
    X (void        , glDeformationMap3fSGIX, (GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat *points)) \
    X (void        , glDeformSGIX, (GLbitfield mask)) \
    X (void        , glLoadIdentityDeformationMapSGIX, (GLbitfield mask))

#define JUCE_GL_FUNCTIONS_GL_SGIX_reference_plane \
    X (void        , glReferencePlaneSGIX, (const GLdouble *equation))

#define JUCE_GL_FUNCTIONS_GL_SGIX_sprite \
    X (void        , glSpriteParameterfSGIX, (GLenum pname, GLfloat param)) \
    X (void        , glSpriteParameterfvSGIX, (GLenum pname, const GLfloat *params)) \
    X (void        , glSpriteParameteriSGIX, (GLenum pname, GLint param)) \
    X (void        , glSpriteParameterivSGIX, (GLenum pname, const GLint *params))

#define JUCE_GL_FUNCTIONS_GL_SGIX_tag_sample_buffer \
    X (void        , glTagSampleBufferSGIX, ())

#define JUCE_GL_FUNCTIONS_GL_SGI_color_table \
    X (void        , glColorTableSGI, (GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const void *table)) \
    X (void        , glColorTableParameterfvSGI, (GLenum target, GLenum pname, const GLfloat *params)) \
    X (void        , glColorTableParameterivSGI, (GLenum target, GLenum pname, const GLint *params)) \
    X (void        , glCopyColorTableSGI, (GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width)) \
    X (void        , glGetColorTableSGI, (GLenum target, GLenum format, GLenum type, void *table)) \
    X (void        , glGetColorTableParameterfvSGI, (GLenum target, GLenum pname, GLfloat *params)) \
    X (void        , glGetColorTableParameterivSGI, (GLenum target, GLenum pname, GLint *params))

#define JUCE_GL_FUNCTIONS_GL_SUNX_constant_data \
    X (void        , glFinishTextureSUNX, ())

#define JUCE_GL_FUNCTIONS_GL_SUN_global_alpha \
    X (void        , glGlobalAlphaFactorbSUN, (GLbyte factor)) \
    X (void        , glGlobalAlphaFactorsSUN, (GLshort factor)) \
    X (void        , glGlobalAlphaFactoriSUN, (GLint factor)) \
    X (void        , glGlobalAlphaFactorfSUN, (GLfloat factor)) \
    X (void        , glGlobalAlphaFactordSUN, (GLdouble factor)) \
    X (void        , glGlobalAlphaFactorubSUN, (GLubyte factor)) \
    X (void        , glGlobalAlphaFactorusSUN, (GLushort factor)) \
    X (void        , glGlobalAlphaFactoruiSUN, (GLuint factor))

#define JUCE_GL_FUNCTIONS_GL_SUN_mesh_array \
    X (void        , glDrawMeshArraysSUN, (GLenum mode, GLint first, GLsizei count, GLsizei width))

#define JUCE_GL_FUNCTIONS_GL_SUN_triangle_list \
    X (void        , glReplacementCodeuiSUN, (GLuint code)) \
    X (void        , glReplacementCodeusSUN, (GLushort code)) \
    X (void        , glReplacementCodeubSUN, (GLubyte code)) \
    X (void        , glReplacementCodeuivSUN, (const GLuint *code)) \
    X (void        , glReplacementCodeusvSUN, (const GLushort *code)) \
    X (void        , glReplacementCodeubvSUN, (const GLubyte *code)) \
    X (void        , glReplacementCodePointerSUN, (GLenum type, GLsizei stride, const void **pointer))

#define JUCE_GL_FUNCTIONS_GL_SUN_vertex \
    X (void        , glColor4ubVertex2fSUN, (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y)) \
    X (void        , glColor4ubVertex2fvSUN, (const GLubyte *c, const GLfloat *v)) \
    X (void        , glColor4ubVertex3fSUN, (GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glColor4ubVertex3fvSUN, (const GLubyte *c, const GLfloat *v)) \
    X (void        , glColor3fVertex3fSUN, (GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glColor3fVertex3fvSUN, (const GLfloat *c, const GLfloat *v)) \
    X (void        , glNormal3fVertex3fSUN, (GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glNormal3fVertex3fvSUN, (const GLfloat *n, const GLfloat *v)) \
    X (void        , glColor4fNormal3fVertex3fSUN, (GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glColor4fNormal3fVertex3fvSUN, (const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (void        , glTexCoord2fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTexCoord2fVertex3fvSUN, (const GLfloat *tc, const GLfloat *v)) \
    X (void        , glTexCoord4fVertex4fSUN, (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glTexCoord4fVertex4fvSUN, (const GLfloat *tc, const GLfloat *v)) \
    X (void        , glTexCoord2fColor4ubVertex3fSUN, (GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTexCoord2fColor4ubVertex3fvSUN, (const GLfloat *tc, const GLubyte *c, const GLfloat *v)) \
    X (void        , glTexCoord2fColor3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTexCoord2fColor3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *v)) \
    X (void        , glTexCoord2fNormal3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTexCoord2fNormal3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *n, const GLfloat *v)) \
    X (void        , glTexCoord2fColor4fNormal3fVertex3fSUN, (GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glTexCoord2fColor4fNormal3fVertex3fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (void        , glTexCoord4fColor4fNormal3fVertex4fSUN, (GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w)) \
    X (void        , glTexCoord4fColor4fNormal3fVertex4fvSUN, (const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (void        , glReplacementCodeuiVertex3fSUN, (GLuint rc, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiVertex3fvSUN, (const GLuint *rc, const GLfloat *v)) \
    X (void        , glReplacementCodeuiColor4ubVertex3fSUN, (GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiColor4ubVertex3fvSUN, (const GLuint *rc, const GLubyte *c, const GLfloat *v)) \
    X (void        , glReplacementCodeuiColor3fVertex3fSUN, (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiColor3fVertex3fvSUN, (const GLuint *rc, const GLfloat *c, const GLfloat *v)) \
    X (void        , glReplacementCodeuiNormal3fVertex3fSUN, (GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *n, const GLfloat *v)) \
    X (void        , glReplacementCodeuiColor4fNormal3fVertex3fSUN, (GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiColor4fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *c, const GLfloat *n, const GLfloat *v)) \
    X (void        , glReplacementCodeuiTexCoord2fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiTexCoord2fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *v)) \
    X (void        , glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *n, const GLfloat *v)) \
    X (void        , glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN, (GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z)) \
    X (void        , glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN, (const GLuint *rc, const GLfloat *tc, const GLfloat *c, const GLfloat *n, const GLfloat *v))


#if JUCE_STATIC_LINK_GL_VERSION_1_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_0_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_0
#endif

#if JUCE_STATIC_LINK_GL_VERSION_1_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_1_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_1_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_1
#endif

#if JUCE_STATIC_LINK_GL_VERSION_1_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_2_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_2_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_2
#endif

#if JUCE_STATIC_LINK_GL_VERSION_1_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_3_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_3_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_3
#endif

#if JUCE_STATIC_LINK_GL_VERSION_1_4
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_4_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_4
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_4_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_4
#endif

#if JUCE_STATIC_LINK_GL_VERSION_1_5
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_5_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_1_5
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_5_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_1_5
#endif

#if JUCE_STATIC_LINK_GL_VERSION_2_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_0_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_2_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_2_0
#endif

#if JUCE_STATIC_LINK_GL_VERSION_2_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_1_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_2_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_1_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_2_1
#endif

#if JUCE_STATIC_LINK_GL_VERSION_3_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_0_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_3_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_3_0
#endif

#if JUCE_STATIC_LINK_GL_VERSION_3_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_1_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_3_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_1_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_3_1
#endif

#if JUCE_STATIC_LINK_GL_VERSION_3_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_2_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_3_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_2_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_3_2
#endif

#if JUCE_STATIC_LINK_GL_VERSION_3_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_3_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_3_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_3_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_3_3
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_0_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_0
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_0_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_0
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_1_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_1
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_1_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_1
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_2_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_2
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_2_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_2
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_3_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_3
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_3_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_3
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_4
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_4_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_4
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_4_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_4
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_5
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_5_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_5
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_5_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_5
#endif

#if JUCE_STATIC_LINK_GL_VERSION_4_6
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_6_STATIC JUCE_GL_FUNCTIONS_GL_VERSION_4_6
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC
#else
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_6_STATIC
 #define JUCE_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC JUCE_GL_FUNCTIONS_GL_VERSION_4_6
#endif


#define JUCE_STATIC_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_1_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_2_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_3_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_4_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_5_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_2_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_2_1_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_1_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_2_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_3_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_0_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_1_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_2_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_3_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_4_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_5_STATIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_6_STATIC

#define JUCE_DYNAMIC_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_1_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_2_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_3_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_4_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_1_5_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_2_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_2_1_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_1_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_2_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_3_3_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_0_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_1_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_2_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_3_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_4_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_5_DYNAMIC \
    JUCE_GL_FUNCTIONS_GL_VERSION_4_6_DYNAMIC

#define JUCE_EXTENSION_GL_FUNCTIONS \
    JUCE_GL_FUNCTIONS_GL_3DFX_tbuffer \
    JUCE_GL_FUNCTIONS_GL_AMD_debug_output \
    JUCE_GL_FUNCTIONS_GL_AMD_draw_buffers_blend \
    JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_multisample_advanced \
    JUCE_GL_FUNCTIONS_GL_AMD_framebuffer_sample_positions \
    JUCE_GL_FUNCTIONS_GL_AMD_gpu_shader_int64 \
    JUCE_GL_FUNCTIONS_GL_AMD_interleaved_elements \
    JUCE_GL_FUNCTIONS_GL_AMD_multi_draw_indirect \
    JUCE_GL_FUNCTIONS_GL_AMD_name_gen_delete \
    JUCE_GL_FUNCTIONS_GL_AMD_occlusion_query_event \
    JUCE_GL_FUNCTIONS_GL_AMD_performance_monitor \
    JUCE_GL_FUNCTIONS_GL_AMD_sample_positions \
    JUCE_GL_FUNCTIONS_GL_AMD_sparse_texture \
    JUCE_GL_FUNCTIONS_GL_AMD_stencil_operation_extended \
    JUCE_GL_FUNCTIONS_GL_AMD_vertex_shader_tessellator \
    JUCE_GL_FUNCTIONS_GL_APPLE_element_array \
    JUCE_GL_FUNCTIONS_GL_APPLE_fence \
    JUCE_GL_FUNCTIONS_GL_APPLE_flush_buffer_range \
    JUCE_GL_FUNCTIONS_GL_APPLE_object_purgeable \
    JUCE_GL_FUNCTIONS_GL_APPLE_texture_range \
    JUCE_GL_FUNCTIONS_GL_APPLE_vertex_array_object \
    JUCE_GL_FUNCTIONS_GL_APPLE_vertex_array_range \
    JUCE_GL_FUNCTIONS_GL_APPLE_vertex_program_evaluators \
    JUCE_GL_FUNCTIONS_GL_ARB_ES3_2_compatibility \
    JUCE_GL_FUNCTIONS_GL_ARB_bindless_texture \
    JUCE_GL_FUNCTIONS_GL_ARB_cl_event \
    JUCE_GL_FUNCTIONS_GL_ARB_color_buffer_float \
    JUCE_GL_FUNCTIONS_GL_ARB_compute_variable_group_size \
    JUCE_GL_FUNCTIONS_GL_ARB_debug_output \
    JUCE_GL_FUNCTIONS_GL_ARB_draw_buffers \
    JUCE_GL_FUNCTIONS_GL_ARB_draw_buffers_blend \
    JUCE_GL_FUNCTIONS_GL_ARB_draw_instanced \
    JUCE_GL_FUNCTIONS_GL_ARB_fragment_program \
    JUCE_GL_FUNCTIONS_GL_ARB_geometry_shader4 \
    JUCE_GL_FUNCTIONS_GL_ARB_gl_spirv \
    JUCE_GL_FUNCTIONS_GL_ARB_gpu_shader_int64 \
    JUCE_GL_FUNCTIONS_GL_ARB_imaging \
    JUCE_GL_FUNCTIONS_GL_ARB_indirect_parameters \
    JUCE_GL_FUNCTIONS_GL_ARB_instanced_arrays \
    JUCE_GL_FUNCTIONS_GL_ARB_matrix_palette \
    JUCE_GL_FUNCTIONS_GL_ARB_multisample \
    JUCE_GL_FUNCTIONS_GL_ARB_multitexture \
    JUCE_GL_FUNCTIONS_GL_ARB_occlusion_query \
    JUCE_GL_FUNCTIONS_GL_ARB_parallel_shader_compile \
    JUCE_GL_FUNCTIONS_GL_ARB_point_parameters \
    JUCE_GL_FUNCTIONS_GL_ARB_robustness \
    JUCE_GL_FUNCTIONS_GL_ARB_sample_locations \
    JUCE_GL_FUNCTIONS_GL_ARB_sample_shading \
    JUCE_GL_FUNCTIONS_GL_ARB_shader_objects \
    JUCE_GL_FUNCTIONS_GL_ARB_shading_language_include \
    JUCE_GL_FUNCTIONS_GL_ARB_sparse_buffer \
    JUCE_GL_FUNCTIONS_GL_ARB_sparse_texture \
    JUCE_GL_FUNCTIONS_GL_ARB_texture_buffer_object \
    JUCE_GL_FUNCTIONS_GL_ARB_texture_compression \
    JUCE_GL_FUNCTIONS_GL_ARB_transpose_matrix \
    JUCE_GL_FUNCTIONS_GL_ARB_vertex_blend \
    JUCE_GL_FUNCTIONS_GL_ARB_vertex_buffer_object \
    JUCE_GL_FUNCTIONS_GL_ARB_vertex_program \
    JUCE_GL_FUNCTIONS_GL_ARB_vertex_shader \
    JUCE_GL_FUNCTIONS_GL_ARB_viewport_array \
    JUCE_GL_FUNCTIONS_GL_ARB_window_pos \
    JUCE_GL_FUNCTIONS_GL_ATI_draw_buffers \
    JUCE_GL_FUNCTIONS_GL_ATI_element_array \
    JUCE_GL_FUNCTIONS_GL_ATI_envmap_bumpmap \
    JUCE_GL_FUNCTIONS_GL_ATI_fragment_shader \
    JUCE_GL_FUNCTIONS_GL_ATI_map_object_buffer \
    JUCE_GL_FUNCTIONS_GL_ATI_pn_triangles \
    JUCE_GL_FUNCTIONS_GL_ATI_separate_stencil \
    JUCE_GL_FUNCTIONS_GL_ATI_vertex_array_object \
    JUCE_GL_FUNCTIONS_GL_ATI_vertex_attrib_array_object \
    JUCE_GL_FUNCTIONS_GL_ATI_vertex_streams \
    JUCE_GL_FUNCTIONS_GL_EXT_EGL_image_storage \
    JUCE_GL_FUNCTIONS_GL_EXT_bindable_uniform \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_color \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_equation_separate \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_func_separate \
    JUCE_GL_FUNCTIONS_GL_EXT_blend_minmax \
    JUCE_GL_FUNCTIONS_GL_EXT_color_subtable \
    JUCE_GL_FUNCTIONS_GL_EXT_compiled_vertex_array \
    JUCE_GL_FUNCTIONS_GL_EXT_convolution \
    JUCE_GL_FUNCTIONS_GL_EXT_coordinate_frame \
    JUCE_GL_FUNCTIONS_GL_EXT_copy_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_cull_vertex \
    JUCE_GL_FUNCTIONS_GL_EXT_debug_label \
    JUCE_GL_FUNCTIONS_GL_EXT_debug_marker \
    JUCE_GL_FUNCTIONS_GL_EXT_depth_bounds_test \
    JUCE_GL_FUNCTIONS_GL_EXT_direct_state_access \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_buffers2 \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_instanced \
    JUCE_GL_FUNCTIONS_GL_EXT_draw_range_elements \
    JUCE_GL_FUNCTIONS_GL_EXT_external_buffer \
    JUCE_GL_FUNCTIONS_GL_EXT_fog_coord \
    JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_blit \
    JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_blit_layers \
    JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_multisample \
    JUCE_GL_FUNCTIONS_GL_EXT_framebuffer_object \
    JUCE_GL_FUNCTIONS_GL_EXT_geometry_shader4 \
    JUCE_GL_FUNCTIONS_GL_EXT_gpu_program_parameters \
    JUCE_GL_FUNCTIONS_GL_EXT_gpu_shader4 \
    JUCE_GL_FUNCTIONS_GL_EXT_histogram \
    JUCE_GL_FUNCTIONS_GL_EXT_index_func \
    JUCE_GL_FUNCTIONS_GL_EXT_index_material \
    JUCE_GL_FUNCTIONS_GL_EXT_light_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object_fd \
    JUCE_GL_FUNCTIONS_GL_EXT_memory_object_win32 \
    JUCE_GL_FUNCTIONS_GL_EXT_multi_draw_arrays \
    JUCE_GL_FUNCTIONS_GL_EXT_multisample \
    JUCE_GL_FUNCTIONS_GL_EXT_paletted_texture \
    JUCE_GL_FUNCTIONS_GL_EXT_pixel_transform \
    JUCE_GL_FUNCTIONS_GL_EXT_point_parameters \
    JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset \
    JUCE_GL_FUNCTIONS_GL_EXT_polygon_offset_clamp \
    JUCE_GL_FUNCTIONS_GL_EXT_provoking_vertex \
    JUCE_GL_FUNCTIONS_GL_EXT_raster_multisample \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore_fd \
    JUCE_GL_FUNCTIONS_GL_EXT_semaphore_win32 \
    JUCE_GL_FUNCTIONS_GL_EXT_secondary_color \
    JUCE_GL_FUNCTIONS_GL_EXT_separate_shader_objects \
    JUCE_GL_FUNCTIONS_GL_EXT_shader_framebuffer_fetch_non_coherent \
    JUCE_GL_FUNCTIONS_GL_EXT_shader_image_load_store \
    JUCE_GL_FUNCTIONS_GL_EXT_stencil_clear_tag \
    JUCE_GL_FUNCTIONS_GL_EXT_stencil_two_side \
    JUCE_GL_FUNCTIONS_GL_EXT_subtexture \
    JUCE_GL_FUNCTIONS_GL_EXT_texture3D \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_array \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_buffer_object \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_integer \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_object \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_perturb_normal \
    JUCE_GL_FUNCTIONS_GL_EXT_texture_storage \
    JUCE_GL_FUNCTIONS_GL_NV_timeline_semaphore \
    JUCE_GL_FUNCTIONS_GL_EXT_timer_query \
    JUCE_GL_FUNCTIONS_GL_EXT_transform_feedback \
    JUCE_GL_FUNCTIONS_GL_EXT_vertex_array \
    JUCE_GL_FUNCTIONS_GL_EXT_vertex_attrib_64bit \
    JUCE_GL_FUNCTIONS_GL_EXT_vertex_shader \
    JUCE_GL_FUNCTIONS_GL_EXT_vertex_weighting \
    JUCE_GL_FUNCTIONS_GL_EXT_win32_keyed_mutex \
    JUCE_GL_FUNCTIONS_GL_EXT_window_rectangles \
    JUCE_GL_FUNCTIONS_GL_EXT_x11_sync_object \
    JUCE_GL_FUNCTIONS_GL_GREMEDY_frame_terminator \
    JUCE_GL_FUNCTIONS_GL_GREMEDY_string_marker \
    JUCE_GL_FUNCTIONS_GL_HP_image_transform \
    JUCE_GL_FUNCTIONS_GL_IBM_multimode_draw_arrays \
    JUCE_GL_FUNCTIONS_GL_IBM_static_data \
    JUCE_GL_FUNCTIONS_GL_IBM_vertex_array_lists \
    JUCE_GL_FUNCTIONS_GL_INGR_blend_func_separate \
    JUCE_GL_FUNCTIONS_GL_INTEL_framebuffer_CMAA \
    JUCE_GL_FUNCTIONS_GL_INTEL_map_texture \
    JUCE_GL_FUNCTIONS_GL_INTEL_parallel_arrays \
    JUCE_GL_FUNCTIONS_GL_INTEL_performance_query \
    JUCE_GL_FUNCTIONS_GL_KHR_blend_equation_advanced \
    JUCE_GL_FUNCTIONS_GL_KHR_debug \
    JUCE_GL_FUNCTIONS_GL_KHR_robustness \
    JUCE_GL_FUNCTIONS_GL_KHR_parallel_shader_compile \
    JUCE_GL_FUNCTIONS_GL_MESA_framebuffer_flip_y \
    JUCE_GL_FUNCTIONS_GL_MESA_resize_buffers \
    JUCE_GL_FUNCTIONS_GL_MESA_window_pos \
    JUCE_GL_FUNCTIONS_GL_NVX_conditional_render \
    JUCE_GL_FUNCTIONS_GL_NVX_linked_gpu_multicast \
    JUCE_GL_FUNCTIONS_GL_NV_alpha_to_coverage_dither_control \
    JUCE_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect \
    JUCE_GL_FUNCTIONS_GL_NV_bindless_multi_draw_indirect_count \
    JUCE_GL_FUNCTIONS_GL_NV_bindless_texture \
    JUCE_GL_FUNCTIONS_GL_NV_blend_equation_advanced \
    JUCE_GL_FUNCTIONS_GL_NV_clip_space_w_scaling \
    JUCE_GL_FUNCTIONS_GL_NV_command_list \
    JUCE_GL_FUNCTIONS_GL_NV_conditional_render \
    JUCE_GL_FUNCTIONS_GL_NV_conservative_raster \
    JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_dilate \
    JUCE_GL_FUNCTIONS_GL_NV_conservative_raster_pre_snap_triangles \
    JUCE_GL_FUNCTIONS_GL_NV_copy_image \
    JUCE_GL_FUNCTIONS_GL_NV_depth_buffer_float \
    JUCE_GL_FUNCTIONS_GL_NV_draw_texture \
    JUCE_GL_FUNCTIONS_GL_NV_draw_vulkan_image \
    JUCE_GL_FUNCTIONS_GL_NV_evaluators \
    JUCE_GL_FUNCTIONS_GL_NV_explicit_multisample \
    JUCE_GL_FUNCTIONS_GL_NV_fence \
    JUCE_GL_FUNCTIONS_GL_NV_fragment_coverage_to_color \
    JUCE_GL_FUNCTIONS_GL_NV_fragment_program \
    JUCE_GL_FUNCTIONS_GL_NV_framebuffer_mixed_samples \
    JUCE_GL_FUNCTIONS_GL_NV_framebuffer_multisample_coverage \
    JUCE_GL_FUNCTIONS_GL_NV_geometry_program4 \
    JUCE_GL_FUNCTIONS_GL_NV_gpu_program4 \
    JUCE_GL_FUNCTIONS_GL_NV_gpu_program5 \
    JUCE_GL_FUNCTIONS_GL_NV_half_float \
    JUCE_GL_FUNCTIONS_GL_NV_internalformat_sample_query \
    JUCE_GL_FUNCTIONS_GL_NV_gpu_multicast \
    JUCE_GL_FUNCTIONS_GL_NVX_gpu_multicast2 \
    JUCE_GL_FUNCTIONS_GL_NVX_progress_fence \
    JUCE_GL_FUNCTIONS_GL_NV_memory_attachment \
    JUCE_GL_FUNCTIONS_GL_NV_memory_object_sparse \
    JUCE_GL_FUNCTIONS_GL_NV_mesh_shader \
    JUCE_GL_FUNCTIONS_GL_NV_occlusion_query \
    JUCE_GL_FUNCTIONS_GL_NV_parameter_buffer_object \
    JUCE_GL_FUNCTIONS_GL_NV_path_rendering \
    JUCE_GL_FUNCTIONS_GL_NV_pixel_data_range \
    JUCE_GL_FUNCTIONS_GL_NV_point_sprite \
    JUCE_GL_FUNCTIONS_GL_NV_present_video \
    JUCE_GL_FUNCTIONS_GL_NV_primitive_restart \
    JUCE_GL_FUNCTIONS_GL_NV_query_resource \
    JUCE_GL_FUNCTIONS_GL_NV_query_resource_tag \
    JUCE_GL_FUNCTIONS_GL_NV_register_combiners \
    JUCE_GL_FUNCTIONS_GL_NV_register_combiners2 \
    JUCE_GL_FUNCTIONS_GL_NV_sample_locations \
    JUCE_GL_FUNCTIONS_GL_NV_scissor_exclusive \
    JUCE_GL_FUNCTIONS_GL_NV_shader_buffer_load \
    JUCE_GL_FUNCTIONS_GL_NV_shading_rate_image \
    JUCE_GL_FUNCTIONS_GL_NV_texture_barrier \
    JUCE_GL_FUNCTIONS_GL_NV_texture_multisample \
    JUCE_GL_FUNCTIONS_GL_NV_transform_feedback \
    JUCE_GL_FUNCTIONS_GL_NV_transform_feedback2 \
    JUCE_GL_FUNCTIONS_GL_NV_vdpau_interop \
    JUCE_GL_FUNCTIONS_GL_NV_vdpau_interop2 \
    JUCE_GL_FUNCTIONS_GL_NV_vertex_array_range \
    JUCE_GL_FUNCTIONS_GL_NV_vertex_attrib_integer_64bit \
    JUCE_GL_FUNCTIONS_GL_NV_vertex_buffer_unified_memory \
    JUCE_GL_FUNCTIONS_GL_NV_vertex_program \
    JUCE_GL_FUNCTIONS_GL_NV_video_capture \
    JUCE_GL_FUNCTIONS_GL_NV_viewport_swizzle \
    JUCE_GL_FUNCTIONS_GL_OES_byte_coordinates \
    JUCE_GL_FUNCTIONS_GL_OES_fixed_point \
    JUCE_GL_FUNCTIONS_GL_OES_query_matrix \
    JUCE_GL_FUNCTIONS_GL_OES_single_precision \
    JUCE_GL_FUNCTIONS_GL_OVR_multiview \
    JUCE_GL_FUNCTIONS_GL_PGI_misc_hints \
    JUCE_GL_FUNCTIONS_GL_SGIS_detail_texture \
    JUCE_GL_FUNCTIONS_GL_SGIS_fog_function \
    JUCE_GL_FUNCTIONS_GL_SGIS_multisample \
    JUCE_GL_FUNCTIONS_GL_SGIS_pixel_texture \
    JUCE_GL_FUNCTIONS_GL_SGIS_point_parameters \
    JUCE_GL_FUNCTIONS_GL_SGIS_sharpen_texture \
    JUCE_GL_FUNCTIONS_GL_SGIS_texture4D \
    JUCE_GL_FUNCTIONS_GL_SGIS_texture_color_mask \
    JUCE_GL_FUNCTIONS_GL_SGIS_texture_filter4 \
    JUCE_GL_FUNCTIONS_GL_SGIX_async \
    JUCE_GL_FUNCTIONS_GL_SGIX_flush_raster \
    JUCE_GL_FUNCTIONS_GL_SGIX_fragment_lighting \
    JUCE_GL_FUNCTIONS_GL_SGIX_framezoom \
    JUCE_GL_FUNCTIONS_GL_SGIX_igloo_interface \
    JUCE_GL_FUNCTIONS_GL_SGIX_instruments \
    JUCE_GL_FUNCTIONS_GL_SGIX_list_priority \
    JUCE_GL_FUNCTIONS_GL_SGIX_pixel_texture \
    JUCE_GL_FUNCTIONS_GL_SGIX_polynomial_ffd \
    JUCE_GL_FUNCTIONS_GL_SGIX_reference_plane \
    JUCE_GL_FUNCTIONS_GL_SGIX_sprite \
    JUCE_GL_FUNCTIONS_GL_SGIX_tag_sample_buffer \
    JUCE_GL_FUNCTIONS_GL_SGI_color_table \
    JUCE_GL_FUNCTIONS_GL_SUNX_constant_data \
    JUCE_GL_FUNCTIONS_GL_SUN_global_alpha \
    JUCE_GL_FUNCTIONS_GL_SUN_mesh_array \
    JUCE_GL_FUNCTIONS_GL_SUN_triangle_list \
    JUCE_GL_FUNCTIONS_GL_SUN_vertex

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
