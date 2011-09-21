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

BEGIN_JUCE_NAMESPACE

#if JUCE_WINDOWS
enum
{
    GL_FRAMEBUFFER_EXT = 0x8D40,
    GL_RENDERBUFFER_EXT = 0x8D41,
    GL_COLOR_ATTACHMENT0_EXT = 0x8CE0,
    GL_DEPTH_ATTACHMENT_EXT = 0x8D00,
    GL_STENCIL_ATTACHMENT_EXT = 0x8D20,
    GL_FRAMEBUFFER_COMPLETE_EXT = 0x8CD5,
    GL_DEPTH24_STENCIL8_EXT = 0x88F0,
    GL_RENDERBUFFER_DEPTH_SIZE_EXT = 0x8D54
};
#endif

#if JUCE_WINDOWS || JUCE_LINUX

#define FRAMEBUFFER_FUNCTION_LIST(USE_FUNCTION) \
    USE_FUNCTION (glIsRenderbufferEXT,                          GLboolean, (GLuint renderbuffer))\
    USE_FUNCTION (glBindRenderbufferEXT,                        void, (GLenum target, GLuint renderbuffer))\
    USE_FUNCTION (glDeleteRenderbuffersEXT,                     void, (GLsizei n, const GLuint *renderbuffers))\
    USE_FUNCTION (glGenRenderbuffersEXT,                        void, (GLsizei n, GLuint *renderbuffers))\
    USE_FUNCTION (glRenderbufferStorageEXT,                     void, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height))\
    USE_FUNCTION (glGetRenderbufferParameterivEXT,              void, (GLenum target, GLenum pname, GLint* params))\
    USE_FUNCTION (glIsFramebufferEXT,                           GLboolean, (GLuint framebuffer))\
    USE_FUNCTION (glBindFramebufferEXT,                         void, (GLenum target, GLuint framebuffer))\
    USE_FUNCTION (glDeleteFramebuffersEXT,                      void, (GLsizei n, const GLuint *framebuffers))\
    USE_FUNCTION (glGenFramebuffersEXT,                         void, (GLsizei n, GLuint *framebuffers))\
    USE_FUNCTION (glCheckFramebufferStatusEXT,                  GLenum, (GLenum target))\
    USE_FUNCTION (glFramebufferTexture1DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
    USE_FUNCTION (glFramebufferTexture2DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
    USE_FUNCTION (glFramebufferTexture3DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))\
    USE_FUNCTION (glFramebufferRenderbufferEXT,                 void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))\
    USE_FUNCTION (glGetFramebufferAttachmentParameterivEXT,     void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))\
    USE_FUNCTION (glGenerateMipmapEXT,                          void, (GLenum target))\

#if JUCE_WINDOWS
 #define APICALLTYPE __stdcall
#else
 #define APICALLTYPE
#endif

#define DECLARE_FUNCTION(name, returnType, params) \
  typedef returnType (APICALLTYPE * type_ ## name) params; static type_ ## name name;
FRAMEBUFFER_FUNCTION_LIST (DECLARE_FUNCTION)
#undef DECLARE_FUNCTION

static bool framebufferFunctionsInitialised = false;

static void initialiseFrameBufferFunctions()
{
    if (! framebufferFunctionsInitialised)
    {
        framebufferFunctionsInitialised = true;

       #if JUCE_LINUX
        #define lookUpFunction(name) glXGetProcAddress ((const GLubyte*) name)
       #else
        #define lookUpFunction(name) wglGetProcAddress (name)
       #endif

       #define FIND_FUNCTION(name, returnType, params) name = (type_ ## name) lookUpFunction (#name);
        FRAMEBUFFER_FUNCTION_LIST (FIND_FUNCTION)
       #undef FIND_FUNCTION
    }
}

#undef FRAMEBUFFER_FUNCTION_LIST

//==============================================================================
#elif JUCE_OPENGL_ES

#define glIsRenderbufferEXT                         glIsRenderbufferOES
#define glBindRenderbufferEXT                       glBindRenderbufferOES
#define glDeleteRenderbuffersEXT                    glDeleteRenderbuffersOES
#define glGenRenderbuffersEXT                       glGenRenderbuffersOES
#define glRenderbufferStorageEXT                    glRenderbufferStorageOES
#define glGetRenderbufferParameterivEXT             glGetRenderbufferParameterivOES
#define glIsFramebufferEXT                          glIsFramebufferOES
#define glBindFramebufferEXT                        glBindFramebufferOES
#define glDeleteFramebuffersEXT                     glDeleteFramebuffersOES
#define glGenFramebuffersEXT                        glGenFramebuffersOES
#define glCheckFramebufferStatusEXT                 glCheckFramebufferStatusOES
#define glFramebufferTexture1DEXT                   glFramebufferTexture1DOES
#define glFramebufferTexture2DEXT                   glFramebufferTexture2DOES
#define glFramebufferTexture3DEXT                   glFramebufferTexture3DOES
#define glFramebufferRenderbufferEXT                glFramebufferRenderbufferOES
#define glGetFramebufferAttachmentParameterivEXT    glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT                         glGenerateMipmapOES

#define GL_FRAMEBUFFER_EXT                          GL_FRAMEBUFFER_OES
#define GL_RGBA8                                    GL_RGBA
#define GL_COLOR_ATTACHMENT0_EXT                    GL_COLOR_ATTACHMENT0_OES
#define GL_RENDERBUFFER_EXT                         GL_RENDERBUFFER_OES
#define GL_DEPTH24_STENCIL8_EXT                     GL_DEPTH24_STENCIL8_OES
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT              GL_RENDERBUFFER_DEPTH_SIZE_OES
#define GL_DEPTH_ATTACHMENT_EXT                     GL_DEPTH_ATTACHMENT_OES
#define GL_STENCIL_ATTACHMENT_EXT                   GL_STENCIL_ATTACHMENT_OES
#define GL_FRAMEBUFFER_COMPLETE_EXT                 GL_FRAMEBUFFER_COMPLETE_OES
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT              GL_FRAMEBUFFER_UNSUPPORTED_OES
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES
#endif

//==============================================================================
class OpenGLFrameBuffer::Pimpl
{
public:
    Pimpl (const int width_, const int height_,
           const bool wantsDepthBuffer, const bool wantsStencilBuffer,
           const GLenum textureType = GL_TEXTURE_2D)
        : width (width_),
          height (height_),
          textureID (0),
          frameBufferHandle (0),
          depthOrStencilBuffer (0),
          hasDepthBuffer (false),
          hasStencilBuffer (false),
          ok (false)
    {
       #if JUCE_WINDOWS || JUCE_LINUX
        initialiseFrameBufferFunctions();

        if (glGenFramebuffersEXT == nullptr)
            return;
       #endif

        resetGLErrorState();

        glGenFramebuffersEXT (1, &frameBufferHandle);
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, frameBufferHandle);

        glGenTextures (1, &textureID);
        glBindTexture (textureType, textureID);
        glTexImage2D (textureType, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glTexParameterf (textureType, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf (textureType, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameterf (textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureType, textureID, 0);

        if (wantsDepthBuffer || wantsStencilBuffer)
        {
            glGenRenderbuffersEXT (1, &depthOrStencilBuffer);
            glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, depthOrStencilBuffer);
            jassert (glIsRenderbufferEXT (depthOrStencilBuffer));

            if (wantsDepthBuffer && wantsStencilBuffer)
            {
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width, height);

                GLint params = 0;
                glGetRenderbufferParameterivEXT (GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &params);

                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);
                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);

                hasDepthBuffer = true;
                hasStencilBuffer = true;
            }
            else
            {
               #if JUCE_OPENGL_ES
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
               #else
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
               #endif

                GLint params = 0;
                glGetRenderbufferParameterivEXT (GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &params);
                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);

                hasDepthBuffer = true;
            }
        }

        ok = checkStatus();

        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
    }

    ~Pimpl()
    {
        if (textureID != 0)
            glDeleteTextures (1, &textureID);

        if (depthOrStencilBuffer != 0)
            glDeleteRenderbuffersEXT (1, &depthOrStencilBuffer);

        if (frameBufferHandle != 0)
            glDeleteFramebuffersEXT (1, &frameBufferHandle);
    }

    bool bind()    { return bind (frameBufferHandle); }
    bool unbind()  { return bind (0); }

    const int width, height;
    GLuint textureID, frameBufferHandle, depthOrStencilBuffer;
    bool hasDepthBuffer, hasStencilBuffer, ok;

private:
    bool bind (GLuint buffer)
    {
        if (ok)
        {
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, buffer);
            ok = checkStatus();
        }

        return ok;
    }

    static bool checkStatus() noexcept
    {
        const GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);

        return status == GL_NO_ERROR
            || status == GL_FRAMEBUFFER_COMPLETE_EXT;
    }

    static void resetGLErrorState()
    {
        while (glGetError() != GL_NO_ERROR) {}
    }

    JUCE_DECLARE_NON_COPYABLE (Pimpl);
};


//==============================================================================
OpenGLFrameBuffer::OpenGLFrameBuffer() {}
OpenGLFrameBuffer::~OpenGLFrameBuffer() {}

bool OpenGLFrameBuffer::initialise (int width, int height)
{
    release();
    pimpl = new Pimpl (width, height, true, false);

    if (! pimpl->ok)
        pimpl = nullptr;

    return pimpl != nullptr;
}

void OpenGLFrameBuffer::release()
{
    pimpl = nullptr;
}

int OpenGLFrameBuffer::getWidth() const noexcept            { return pimpl != nullptr ? pimpl->width : 0; }
int OpenGLFrameBuffer::getHeight() const noexcept           { return pimpl != nullptr ? pimpl->height : 0; }
GLuint OpenGLFrameBuffer::getTextureID() const noexcept     { return pimpl != nullptr ? pimpl->textureID : 0; }

bool OpenGLFrameBuffer::makeCurrentTarget()
{
    return pimpl != nullptr && pimpl->bind();
}

void OpenGLFrameBuffer::releaseCurrentTarget()
{
    if (pimpl != nullptr)
        pimpl->unbind();
}

void OpenGLFrameBuffer::clear (const Colour& colour)
{
    if (makeCurrentTarget())
    {
        OpenGLHelpers::clear (colour);
        releaseCurrentTarget();
    }
}

END_JUCE_NAMESPACE
