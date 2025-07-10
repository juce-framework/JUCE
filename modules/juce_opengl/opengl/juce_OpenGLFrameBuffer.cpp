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

/*
    Used on Android to detect when the GL context and associated resources (textures, framebuffers,
    etc.) need to be destroyed/created due to the Surface changing state.
*/
class OpenGLContext::NativeContextListener
{
public:
    virtual ~NativeContextListener() = default;

    virtual void contextWillPause() = 0;
    virtual void contextDidResume() = 0;

    static void addListener (OpenGLContext& ctx, NativeContextListener& l);
    static void removeListener (OpenGLContext& ctx, NativeContextListener& l);
};

class OpenGLFrameBuffer::Pimpl : private OpenGLContext::NativeContextListener
{
public:
    struct SavedState
    {
        int width = 0, height = 0;
        std::vector<PixelARGB> data;

        static constexpr auto order = RowOrder::fromBottomUp;
    };

    ~Pimpl() override
    {
        release();
    }

    bool isValid() const noexcept
    {
        return std::holds_alternative<TransientState> (state);
    }

    bool initialise (OpenGLContext& context, int width, int height)
    {
        jassert (context.isActive()); // The context must be active when creating a framebuffer!

        release();
        auto& transientState = state.emplace<TransientState> (width, height, false, false);

        if (! transientState.createdOk())
            release();

        if (! isValid())
            return false;

        associatedContext = &context;
        NativeContextListener::addListener (*associatedContext, *this);

        return true;
    }

    bool initialise (OpenGLContext& context, const Image& image)
    {
        if (! image.isARGB())
            return initialise (context, image.convertedToFormat (Image::ARGB));

        Image::BitmapData bitmap (image, Image::BitmapData::readOnly);

        return initialise (context, bitmap.width, bitmap.height)
               && writePixels ((const PixelARGB*) bitmap.data, image.getBounds(), RowOrder::fromTopDown);
    }

    bool initialise (OpenGLFrameBuffer& other)
    {
        auto* p = std::get_if<TransientState> (&other.pimpl->state);

        if (p == nullptr || other.pimpl->associatedContext == nullptr)
        {
            release();
            return true;
        }

        const Rectangle<int> area (p->width, p->height);

        if (! initialise (*other.pimpl->associatedContext, area.getWidth(), area.getHeight()))
            return false;

        jassert (associatedContext != nullptr);

        auto* transientState = std::get_if<TransientState> (&state);
        transientState->bind();
        const ScopeGuard unbinder { [transientState] { transientState->unbind(); }};

       #if ! JUCE_ANDROID
        if (! associatedContext->isCoreProfile())
            glEnable (GL_TEXTURE_2D);

        clearGLError();
       #endif
        {
            const ScopedTextureBinding scopedTextureBinding;
            glBindTexture (GL_TEXTURE_2D, p->textureID);
            associatedContext->copyTexture (area, area, area.getWidth(), area.getHeight(), false);
        }

        return true;
    }

    void release()
    {
        if (auto* prev = std::exchange (associatedContext, nullptr))
            NativeContextListener::removeListener (*prev, *this);

        state.emplace<std::monostate>();
    }

    void saveAndRelease()
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
        {
            if (auto toSave = readPixels ({ transientState->width, transientState->height }))
                state.emplace<SavedState> (std::move (*toSave));
        }
    }

    bool reloadSavedCopy (OpenGLContext& context)
    {
        if (auto* savedState = std::get_if<SavedState> (&state))
        {
            auto local = std::move (*savedState);

            if (restore (context, local))
                return true;

            state.emplace<SavedState> (std::move (local));
        }

        return false;
    }

    int getWidth() const noexcept
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
            return transientState->width;

        return 0;
    }

    int getHeight() const noexcept
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
            return transientState->height;

        return 0;
    }

    GLuint getTextureID() const noexcept
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
            return transientState->textureID;

        return 0;
    }

    GLuint getFrameBufferID() const noexcept
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
            return transientState->frameBufferID;

        return 0;
    }

    bool makeCurrentRenderingTarget()
    {
        return makeAndGetCurrentRenderingTarget() != nullptr;
    }

    void releaseAsRenderingTarget()
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
            transientState->unbind();
    }

    void clear (Colour colour)
    {
        auto* transientState = makeAndGetCurrentRenderingTarget();

        if (transientState == nullptr)
            return;

        const ScopeGuard unbinder { [transientState] { transientState->unbind(); }};
        OpenGLHelpers::clear (colour);
    }

    void makeCurrentAndClear()
    {
        auto* transientState = makeAndGetCurrentRenderingTarget();

        if (transientState == nullptr)
            return;

        glClearColor (0, 0, 0, 0);
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    bool readPixels (PixelARGB* target, const Rectangle<int>& area, RowOrder order)
    {
        auto* transientState = makeAndGetCurrentRenderingTarget();

        if (transientState == nullptr)
            return false;

        const ScopeGuard unbinder { [transientState] { transientState->unbind(); }};

        glPixelStorei (GL_PACK_ALIGNMENT, 4);
        glReadPixels (area.getX(), area.getY(), area.getWidth(), area.getHeight(),
                      JUCE_RGBA_FORMAT, GL_UNSIGNED_BYTE, target);

        if (order == RowOrder::fromTopDown)
        {
            auto* end = target + area.getWidth() * area.getHeight();

            for (auto y = 0; y < area.getHeight() / 2; ++y)
            {
                const auto offset = area.getWidth() * y;
                auto* rowA = target + offset;
                auto* rowB = end - offset - area.getWidth();

                for (auto x = 0; x < area.getWidth(); ++x)
                    std::swap (rowA[x], rowB[x]);
            }
        }

        return true;
    }

    bool writePixels (const PixelARGB* data, const Rectangle<int>& area, RowOrder order)
    {
        if (associatedContext == nullptr)
            return false;

        const OpenGLTargetSaver ts;
        auto* transientState = makeAndGetCurrentRenderingTarget();

        if (transientState == nullptr)
            return false;

        glDisable (GL_DEPTH_TEST);
        glDisable (GL_BLEND);
        JUCE_CHECK_OPENGL_ERROR

        OpenGLTexture tex;
        tex.loadARGB (data, area.getWidth(), area.getHeight());

        glViewport (0, 0, transientState->width, transientState->height);
        associatedContext->copyTexture (area,
                                        Rectangle<int> (area.getX(),
                                                        area.getY(),
                                                        tex.getWidth(),
                                                        tex.getHeight()),
                                        transientState->width,
                                        transientState->height,
                                        order == RowOrder::fromTopDown,
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
        TransientState (const int w,
                        const int h,
                        const bool wantsDepthBuffer,
                        const bool wantsStencilBuffer)
            : width (w),
              height (h),
              textureID (0),
              frameBufferID (0),
              depthOrStencilBuffer (0)
        {
            // Framebuffer objects can only be created when the current thread has an active OpenGL
            // context. You'll need to create this object in one of the OpenGLContext's callbacks.
            jassert (OpenGLHelpers::isContextActive());

           #if JUCE_WINDOWS || JUCE_LINUX || JUCE_BSD
            if (gl::glGenFramebuffers == nullptr)
                return;
           #endif

            gl::glGenFramebuffers (1, &frameBufferID);
            bind();
            const ScopeGuard unbinder { [this] { unbind(); }};

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

            gl::glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureID, 0);

            if (wantsDepthBuffer || wantsStencilBuffer)
            {
                gl::glGenRenderbuffers (1, &depthOrStencilBuffer);
                gl::glBindRenderbuffer (GL_RENDERBUFFER, depthOrStencilBuffer);
                jassert (gl::glIsRenderbuffer (depthOrStencilBuffer));

               #if JUCE_OPENGL_ES
                constexpr auto depthComponentConstant = (GLenum) GL_DEPTH_COMPONENT16;
               #else
                constexpr auto depthComponentConstant = (GLenum) GL_DEPTH_COMPONENT;
               #endif

                gl::glRenderbufferStorage (GL_RENDERBUFFER,
                                           (wantsDepthBuffer && wantsStencilBuffer) ? (GLenum) GL_DEPTH24_STENCIL8
                                                                                    : depthComponentConstant,
                                           width,
                                           height);

                GLint params = 0;
                gl::glGetRenderbufferParameteriv (GL_RENDERBUFFER, GL_RENDERBUFFER_DEPTH_SIZE, &params);
                gl::glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthOrStencilBuffer);

                if (wantsStencilBuffer)
                    gl::glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depthOrStencilBuffer);
            }
        }

        ~TransientState()
        {
            if (! OpenGLHelpers::isContextActive())
                return;

            if (textureID != 0)
                gl::glDeleteTextures (1, &textureID);

            if (depthOrStencilBuffer != 0)
                gl::glDeleteRenderbuffers (1, &depthOrStencilBuffer);

            if (frameBufferID != 0)
                gl::glDeleteFramebuffers (1, &frameBufferID);

            JUCE_CHECK_OPENGL_ERROR
        }

        bool createdOk() const
        {
            return frameBufferID != 0 && textureID != 0;
        }

        void bind()
        {
            glGetIntegerv (GL_FRAMEBUFFER_BINDING, &prevFramebuffer);
            gl::glBindFramebuffer (GL_FRAMEBUFFER, frameBufferID);
            JUCE_CHECK_OPENGL_ERROR
        }

        void unbind()
        {
            gl::glBindFramebuffer (GL_FRAMEBUFFER, (GLuint) prevFramebuffer);
            JUCE_CHECK_OPENGL_ERROR
        }

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

        writePixels (savedState.data.data(),
                     Rectangle (savedState.width, savedState.height),
                     SavedState::order);
        return true;
    }

    std::optional<SavedState> readPixels (const Rectangle<int>& area)
    {
        SavedState result { area.getWidth(),
                            area.getHeight(),
                            std::vector<PixelARGB> ((size_t) area.getWidth() * (size_t) area.getHeight()) };

        if (! readPixels (result.data.data(), area, SavedState::order))
            return {};

        return result;
    }

    TransientState* makeAndGetCurrentRenderingTarget()
    {
        if (auto* transientState = std::get_if<TransientState> (&state))
        {
            transientState->bind();
            return transientState;
        }

        // trying to use a framebuffer after saving it with saveAndRelease()! Be sure to call
        // reloadSavedCopy() to put it back into GPU memory before using it..
        jassertfalse;

        return nullptr;
    }

    void contextWillPause() override
    {
        saveAndRelease();
    }

    void contextDidResume() override
    {
        if (associatedContext != nullptr)
            reloadSavedCopy (*associatedContext);
    }

    OpenGLContext* associatedContext = nullptr;
    std::variant<std::monostate, TransientState, SavedState> state;
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

bool OpenGLFrameBuffer::readPixels (PixelARGB* targetData, const Rectangle<int>& sourceArea, RowOrder order)
{
    return pimpl->readPixels (targetData, sourceArea, order);
}

bool OpenGLFrameBuffer::writePixels (const PixelARGB* srcData, const Rectangle<int>& targetArea, RowOrder order)
{
    return pimpl->writePixels (srcData, targetArea, order);
}

GLuint OpenGLFrameBuffer::getCurrentFrameBufferTarget() noexcept
{
    GLint fb = {};
    glGetIntegerv (GL_FRAMEBUFFER_BINDING, &fb);
    return (GLuint) fb;
}

} // namespace juce
