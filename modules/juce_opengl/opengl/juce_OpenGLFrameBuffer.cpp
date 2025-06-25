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

namespace juce
{

class OpenGLFrameBuffer::Pimpl
{
public:
    struct SavedState
    {
        int width = 0, height = 0;
        std::vector<PixelARGB> data;
    };

    bool isValid() const noexcept
    {
        return transientState != nullptr;
    }

    bool initialise (OpenGLContext& context, int width, int height)
    {
        jassert (context.isActive()); // The context must be active when creating a framebuffer!

        transientState.reset();
        transientState.reset (new TransientState (context, width, height, false, false));

        if (! transientState->createdOk())
            transientState.reset();

        return transientState != nullptr;
    }

    bool initialise (OpenGLContext& context, const Image& image)
    {
        if (! image.isARGB())
            return initialise (context, image.convertedToFormat (Image::ARGB));

        Image::BitmapData bitmap (image, Image::BitmapData::readOnly);

        return initialise (context, bitmap.width, bitmap.height)
               && writePixels ((const PixelARGB*) bitmap.data, image.getBounds());
    }

    bool initialise (OpenGLFrameBuffer& other)
    {
        auto* p = other.pimpl->transientState.get();

        if (p == nullptr)
        {
            transientState.reset();
            return true;
        }

        const Rectangle<int> area (transientState->width, transientState->height);

        if (initialise (p->context, area.getWidth(), area.getHeight()))
        {
            transientState->bind();

           #if ! JUCE_ANDROID
            if (! transientState->context.isCoreProfile())
                glEnable (GL_TEXTURE_2D);

            clearGLError();
           #endif
            {
                const ScopedTextureBinding scopedTextureBinding;
                glBindTexture (GL_TEXTURE_2D, p->textureID);
                transientState->context.copyTexture (area, area, area.getWidth(), area.getHeight(), false);
            }

            transientState->unbind();
            return true;
        }

        return false;
    }

    void release()
    {
        transientState.reset();
        savedState.reset();
    }

    void saveAndRelease()
    {
        if (transientState != nullptr)
        {
            savedState.reset();

            if (auto toSave = readPixels ({ transientState->width, transientState->height }))
                savedState = std::make_unique<SavedState> (std::move (*toSave));

            transientState.reset();
        }
    }

    bool reloadSavedCopy (OpenGLContext& context)
    {
        if (savedState != nullptr)
        {
            std::unique_ptr<SavedState> state;
            std::swap (state, savedState);

            if (restore (context, *state))
                return true;

            std::swap (state, savedState);
        }

        return false;
    }

    int getWidth() const noexcept            { return transientState != nullptr ? transientState->width : 0; }
    int getHeight() const noexcept           { return transientState != nullptr ? transientState->height : 0; }
    GLuint getTextureID() const noexcept     { return transientState != nullptr ? transientState->textureID : 0; }

    bool makeCurrentRenderingTarget() const
    {
        // trying to use a framebuffer after saving it with saveAndRelease()! Be sure to call
        // reloadSavedCopy() to put it back into GPU memory before using it..
        jassert (savedState == nullptr);

        if (transientState == nullptr)
            return false;

        transientState->bind();
        return true;
    }

    GLuint getFrameBufferID() const noexcept
    {
        return transientState != nullptr ? transientState->frameBufferID : 0;
    }

    void releaseAsRenderingTarget()
    {
        if (transientState != nullptr)
            transientState->unbind();
    }

    void clear (Colour colour)
    {
        if (makeCurrentRenderingTarget())
        {
            OpenGLHelpers::clear (colour);
            releaseAsRenderingTarget();
        }
    }

    void makeCurrentAndClear()
    {
        if (makeCurrentRenderingTarget())
        {
            glClearColor (0, 0, 0, 0);
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }
    }

    bool readPixels (PixelARGB* target, const Rectangle<int>& area) const
    {
        if (! makeCurrentRenderingTarget())
            return false;

        glPixelStorei (GL_PACK_ALIGNMENT, 4);
        glReadPixels (area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                      JUCE_RGBA_FORMAT, GL_UNSIGNED_BYTE, target);

        transientState->unbind();
        return true;
    }

    bool writePixels (const PixelARGB* data, const Rectangle<int>& area)
    {
        OpenGLTargetSaver ts (transientState->context);

        if (! makeCurrentRenderingTarget())
            return false;

        glDisable (GL_DEPTH_TEST);
        glDisable (GL_BLEND);
        JUCE_CHECK_OPENGL_ERROR

        OpenGLTexture tex;
        tex.loadARGB (data, area.getWidth(), area.getHeight());

        glViewport (0, 0, transientState->width, transientState->height);
        transientState->context.copyTexture (area,
                                             Rectangle<int> (area.getX(),
                                                             area.getY(),
                                                             tex.getWidth(),
                                                             tex.getHeight()),
                                             transientState->width,
                                             transientState->height,
                                             true,
                                             false);

        JUCE_CHECK_OPENGL_ERROR
        return true;
    }

private:
    /*  Stores the currently-bound texture on construction, and re-binds it on destruction. */
    struct ScopedTextureBinding
    {
        ScopedTextureBinding()
        {
            glGetIntegerv (GL_TEXTURE_BINDING_2D, &prev);
            JUCE_CHECK_OPENGL_ERROR
        }

        ~ScopedTextureBinding()
        {
            glBindTexture (GL_TEXTURE_2D, (GLuint) prev);
            JUCE_CHECK_OPENGL_ERROR
        }

        GLint prev{};
    };

    class TransientState
    {
    public:
        TransientState (OpenGLContext& c,
                        const int w,
                        const int h,
                        const bool wantsDepthBuffer,
                        const bool wantsStencilBuffer)
            : context (c),
              width (w),
              height (h),
              textureID (0),
              frameBufferID (0),
              depthOrStencilBuffer (0)
        {
            // Framebuffer objects can only be created when the current thread has an active OpenGL
            // context. You'll need to create this object in one of the OpenGLContext's callbacks.
            jassert (OpenGLHelpers::isContextActive());

           #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
            if (context.extensions.glGenFramebuffers == nullptr)
            return;
           #endif

            context.extensions.glGenFramebuffers (1, &frameBufferID);
            bind();

            {
                const ScopedTextureBinding scopedTextureBinding;

                glGenTextures (1, &textureID);
                glBindTexture (GL_TEXTURE_2D, textureID);
                JUCE_CHECK_OPENGL_ERROR

                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                JUCE_CHECK_OPENGL_ERROR

                glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
                JUCE_CHECK_OPENGL_ERROR
            }

            context.extensions.glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            if (wantsDepthBuffer || wantsStencilBuffer)
            {
                context.extensions.glGenRenderbuffers (1, &depthOrStencilBuffer);
                context.extensions.glBindRenderbuffer (GL_RENDERBUFFER, depthOrStencilBuffer);
                jassert (context.extensions.glIsRenderbuffer (depthOrStencilBuffer));

               #if JUCE_OPENGL_ES
                constexpr auto depthComponentConstant = (GLenum) GL_DEPTH_COMPONENT16;
               #else
                constexpr auto depthComponentConstant = (GLenum) GL_DEPTH_COMPONENT;
               #endif

                context.extensions.glRenderbufferStorage (GL_RENDERBUFFER,
                                                          (wantsDepthBuffer && wantsStencilBuffer) ? (GLenum) GL_DEPTH24_STENCIL8
                                                                                                   : depthComponentConstant,
                                                          width, height);

                GLint params = 0;
                context.extensions.glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_DEPTH_SIZE, &params);
                context.extensions.glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthOrStencilBuffer);

                if (wantsStencilBuffer)
                    context.extensions.glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthOrStencilBuffer);
            }

            unbind();
        }

        ~TransientState()
        {
            if (OpenGLHelpers::isContextActive())
            {
                if (textureID != 0)
                    glDeleteTextures (1, &textureID);

                if (depthOrStencilBuffer != 0)
                    context.extensions.glDeleteRenderbuffers (1, &depthOrStencilBuffer);

                if (frameBufferID != 0)
                    context.extensions.glDeleteFramebuffers (1, &frameBufferID);

                JUCE_CHECK_OPENGL_ERROR
            }
        }

        bool createdOk() const
        {
            return frameBufferID != 0 && textureID != 0;
        }

        void bind()
        {
            glGetIntegerv (GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, frameBufferID);
            JUCE_CHECK_OPENGL_ERROR
        }

        void unbind()
        {
            context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, (GLuint) prevFramebuffer);
            JUCE_CHECK_OPENGL_ERROR
        }

        OpenGLContext& context;
        const int width, height;
        GLuint textureID, frameBufferID, depthOrStencilBuffer;

    private:
        GLint prevFramebuffer{};

        JUCE_DECLARE_NON_COPYABLE (TransientState)
    };

    bool restore (OpenGLContext& context, const SavedState& savedState)
    {
        if (! initialise (context, savedState.width, savedState.height))
            return false;

        writePixels (savedState.data.data(), Rectangle (savedState.width, savedState.height));
        return true;
    }

    std::optional<SavedState> readPixels (const Rectangle<int>& area) const
    {
        SavedState result { area.getWidth(),
                            area.getHeight(),
                            std::vector<PixelARGB> ((size_t) area.getWidth() * (size_t) area.getHeight()) };

        if (! readPixels (result.data.data(), area))
            return {};

        return result;
    }

    std::unique_ptr<TransientState> transientState;
    std::unique_ptr<SavedState> savedState;
};

//==============================================================================
OpenGLFrameBuffer::OpenGLFrameBuffer()
    : pimpl (std::make_unique<Pimpl>())
{
}

OpenGLFrameBuffer::~OpenGLFrameBuffer() = default;

bool OpenGLFrameBuffer::initialise (OpenGLContext& context, int width, int height)
{
return pimpl->initialise (context, width, height);
}

bool OpenGLFrameBuffer::initialise (OpenGLContext& context, const Image& content)
{
    return pimpl->initialise (context, content);
}

bool OpenGLFrameBuffer::initialise (OpenGLFrameBuffer& other)
{
    return pimpl->initialise (other);
}

void OpenGLFrameBuffer::release()
{
    pimpl->release();
}

void OpenGLFrameBuffer::saveAndRelease()
{
    pimpl->saveAndRelease();
}

bool OpenGLFrameBuffer::reloadSavedCopy (OpenGLContext& context)
{
    return pimpl->reloadSavedCopy (context);
}

bool OpenGLFrameBuffer::isValid() const noexcept
{
    return pimpl->isValid();
}

int OpenGLFrameBuffer::getWidth() const noexcept
{
    return pimpl->getWidth();
}

int OpenGLFrameBuffer::getHeight() const noexcept
{
    return pimpl->getHeight();
}

GLuint OpenGLFrameBuffer::getTextureID() const noexcept
{
    return pimpl->getTextureID();
}

bool OpenGLFrameBuffer::makeCurrentRenderingTarget()
{
    return pimpl->makeCurrentRenderingTarget();
}

void OpenGLFrameBuffer::releaseAsRenderingTarget()
{
    pimpl->releaseAsRenderingTarget();
}

GLuint OpenGLFrameBuffer::getFrameBufferID() const noexcept
{
    return pimpl->getFrameBufferID();
}

void OpenGLFrameBuffer::clear (Colour colour)
{
    pimpl->clear (colour);
}

void OpenGLFrameBuffer::makeCurrentAndClear()
{
    pimpl->makeCurrentAndClear();
}

bool OpenGLFrameBuffer::readPixels (PixelARGB* targetData, const Rectangle<int>& sourceArea)
{
    return pimpl->readPixels (targetData, sourceArea);
}

bool OpenGLFrameBuffer::writePixels (const PixelARGB* srcData, const Rectangle<int>& targetArea)
{
    return pimpl->writePixels (srcData, targetArea);
}

GLuint OpenGLFrameBuffer::getCurrentFrameBufferTarget() noexcept
{
    GLint fb = {};
    glGetIntegerv (GL_FRAMEBUFFER_BINDING, &fb);
    return (GLuint) fb;
}

} // namespace juce
