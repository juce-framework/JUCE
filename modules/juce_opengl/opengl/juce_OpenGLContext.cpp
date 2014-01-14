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

class OpenGLContext::CachedImage  : public CachedComponentImage,
                                    public Thread
{
public:
    CachedImage (OpenGLContext& c, Component& comp,
                 const OpenGLPixelFormat& pixFormat, void* contextToShare)
        : Thread ("OpenGL Rendering"),
          context (c), component (comp),
          scale (1.0),
         #if JUCE_OPENGL_ES
          shadersAvailable (true),
         #else
          shadersAvailable (false),
         #endif
          hasInitialised (false),
          needsUpdate (1), lastMMLockReleaseTime (0)
    {
        nativeContext = new NativeContext (component, pixFormat, contextToShare, c.useMultisampling);

        if (nativeContext->createdOk())
            context.nativeContext = nativeContext;
        else
            nativeContext = nullptr;
    }

    ~CachedImage()
    {
        stop();
    }

    void start()
    {
       #if ! JUCE_ANDROID
        if (nativeContext != nullptr)
            startThread (6);
       #endif
    }

    void stop()
    {
       #if ! JUCE_ANDROID
        stopThread (10000);
       #endif
        hasInitialised = false;
    }

    //==============================================================================
    void paint (Graphics&) override {}

    bool invalidateAll() override
    {
        validArea.clear();
        triggerRepaint();
        return false;
    }

    bool invalidate (const Rectangle<int>& area) override
    {
        validArea.subtract (area * scale);
        triggerRepaint();
        return false;
    }

    void releaseResources() override {}

    void triggerRepaint()
    {
        needsUpdate = 1;

       #if JUCE_ANDROID
        if (nativeContext != nullptr)
            nativeContext->triggerRepaint();
       #else
        notify();
       #endif
    }

    //==============================================================================
    bool ensureFrameBufferSize()
    {
        const int fbW = cachedImageFrameBuffer.getWidth();
        const int fbH = cachedImageFrameBuffer.getHeight();

        if (fbW != viewportArea.getWidth() || fbH != viewportArea.getHeight() || ! cachedImageFrameBuffer.isValid())
        {
            if (! cachedImageFrameBuffer.initialise (context, viewportArea.getWidth(), viewportArea.getHeight()))
                return false;

            validArea.clear();
            JUCE_CHECK_OPENGL_ERROR
        }

        return true;
    }

    void clearRegionInFrameBuffer (const RectangleList<int>& list)
    {
        glClearColor (0, 0, 0, 0);
        glEnable (GL_SCISSOR_TEST);

        const GLuint previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();
        cachedImageFrameBuffer.makeCurrentRenderingTarget();
        const int imageH = cachedImageFrameBuffer.getHeight();

        for (const Rectangle<int>* i = list.begin(), * const e = list.end(); i != e; ++i)
        {
            glScissor (i->getX(), imageH - i->getBottom(), i->getWidth(), i->getHeight());
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        glDisable (GL_SCISSOR_TEST);
        context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
        JUCE_CHECK_OPENGL_ERROR
    }

    bool renderFrame()
    {
        ScopedPointer<MessageManagerLock> mmLock;

        const bool isUpdating = needsUpdate.compareAndSetBool (0, 1);

        if (context.renderComponents && isUpdating)
        {
            // This avoids hogging the message thread when doing intensive rendering.
            if (lastMMLockReleaseTime + 1 >= Time::getMillisecondCounter())
                wait (2);

            mmLock = new MessageManagerLock (this);  // need to acquire this before locking the context.
            if (! mmLock->lockWasGained())
                return false;
        }

        if (! context.makeActive())
            return false;

        NativeContext::Locker locker (*nativeContext);

        JUCE_CHECK_OPENGL_ERROR

        if (context.renderer != nullptr)
        {
            glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
            context.currentRenderScale = scale;
            context.renderer->renderOpenGL();
            clearGLError();
        }

        if (context.renderComponents)
        {
            if (isUpdating)
            {
                paintComponent();
                mmLock = nullptr;
                lastMMLockReleaseTime = Time::getMillisecondCounter();
            }

            glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
            drawComponentBuffer();
        }

        context.swapBuffers();
        return true;
    }

    void updateViewportSize (bool canTriggerUpdate)
    {
        if (ComponentPeer* peer = component.getPeer())
        {
            const double newScale = Desktop::getInstance().getDisplays()
                                        .getDisplayContaining (component.getScreenBounds().getCentre()).scale;

            Rectangle<int> newArea (peer->getComponent().getLocalArea (&component, component.getLocalBounds())
                                                        .withPosition (0, 0)
                                     * newScale);

            if (scale != newScale || viewportArea != newArea)
            {
                scale = newScale;
                viewportArea = newArea;

                if (canTriggerUpdate)
                    invalidateAll();
            }
        }
    }

    void paintComponent()
    {
        // you mustn't set your own cached image object when attaching a GL context!
        jassert (get (component) == this);

        updateViewportSize (false);

        if (! ensureFrameBufferSize())
            return;

        RectangleList<int> invalid (viewportArea);
        invalid.subtract (validArea);
        validArea = viewportArea;

        if (! invalid.isEmpty())
        {
            clearRegionInFrameBuffer (invalid);

            {
                ScopedPointer<LowLevelGraphicsContext> g (createOpenGLGraphicsContext (context, cachedImageFrameBuffer));
                g->clipToRectangleList (invalid);
                g->addTransform (AffineTransform::scale ((float) scale));

                paintOwner (*g);
                JUCE_CHECK_OPENGL_ERROR
            }

            if (! context.isActive())
                context.makeActive();
        }

        JUCE_CHECK_OPENGL_ERROR
    }

    void drawComponentBuffer()
    {
       #if ! JUCE_ANDROID
        glEnable (GL_TEXTURE_2D);
        clearGLError();
       #endif
        context.extensions.glActiveTexture (GL_TEXTURE0);
        glBindTexture (GL_TEXTURE_2D, cachedImageFrameBuffer.getTextureID());

        const Rectangle<int> cacheBounds (cachedImageFrameBuffer.getWidth(), cachedImageFrameBuffer.getHeight());
        context.copyTexture (cacheBounds, cacheBounds, cacheBounds.getWidth(), cacheBounds.getHeight(), false);
        glBindTexture (GL_TEXTURE_2D, 0);
        JUCE_CHECK_OPENGL_ERROR
    }

    void paintOwner (LowLevelGraphicsContext& llgc)
    {
        Graphics g (llgc);

      #if JUCE_ENABLE_REPAINT_DEBUGGING
       #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
        if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            g.saveState();
        }
       #endif

        JUCE_TRY
        {
            component.paintEntireComponent (g, false);
        }
        JUCE_CATCH_EXCEPTION

      #if JUCE_ENABLE_REPAINT_DEBUGGING
       #ifdef JUCE_IS_REPAINT_DEBUGGING_ACTIVE
        if (JUCE_IS_REPAINT_DEBUGGING_ACTIVE)
       #endif
        {
            // enabling this code will fill all areas that get repainted with a colour overlay, to show
            // clearly when things are being repainted.
            g.restoreState();

            static Random rng;
            g.fillAll (Colour ((uint8) rng.nextInt (255),
                               (uint8) rng.nextInt (255),
                               (uint8) rng.nextInt (255),
                               (uint8) 0x50));
        }
       #endif
    }

    void handleResize()
    {
        updateViewportSize (true);

       #if JUCE_MAC
        if (hasInitialised)
        {
            [nativeContext->view update];
            renderFrame();
        }
       #endif
    }

    //==============================================================================
    void run() override
    {
        {
            // Allow the message thread to finish setting-up the context before using it..
            MessageManagerLock mml (this);
            if (! mml.lockWasGained())
                return;
        }

        initialiseOnThread();

        hasInitialised = true;

        while (! threadShouldExit())
        {
            if (! renderFrame())
                wait (5); // failed to render, so avoid a tight fail-loop.
            else if (! context.continuousRepaint)
                wait (-1);
        }

        shutdownOnThread();
    }

    void initialiseOnThread()
    {
        // On android, this can get called twice, so drop any previous state..
        associatedObjectNames.clear();
        associatedObjects.clear();
        cachedImageFrameBuffer.release();

        context.makeActive();
        nativeContext->initialiseOnRenderThread (context);

        glViewport (0, 0, component.getWidth(), component.getHeight());

        context.extensions.initialise();
        nativeContext->setSwapInterval (1);

       #if JUCE_USE_OPENGL_SHADERS && ! JUCE_OPENGL_ES
        shadersAvailable = OpenGLShaderProgram::getLanguageVersion() > 0;
       #endif

        if (context.renderer != nullptr)
            context.renderer->newOpenGLContextCreated();
    }

    void shutdownOnThread()
    {
        if (context.renderer != nullptr)
            context.renderer->openGLContextClosing();

        cachedImageFrameBuffer.release();
        nativeContext->shutdownOnRenderThread();

        associatedObjectNames.clear();
        associatedObjects.clear();
    }

    //==============================================================================
    static CachedImage* get (Component& c) noexcept
    {
        return dynamic_cast<CachedImage*> (c.getCachedComponentImage());
    }

    //==============================================================================
    ScopedPointer<NativeContext> nativeContext;

    OpenGLContext& context;
    Component& component;

    OpenGLFrameBuffer cachedImageFrameBuffer;
    RectangleList<int> validArea;
    Rectangle<int> viewportArea;
    double scale;

    StringArray associatedObjectNames;
    ReferenceCountedArray<ReferenceCountedObject> associatedObjects;

    WaitableEvent canPaintNowFlag, finishedPaintingFlag;
    bool shadersAvailable, hasInitialised;
    Atomic<int> needsUpdate;
    uint32 lastMMLockReleaseTime;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
};

//==============================================================================
#if JUCE_ANDROID
void OpenGLContext::NativeContext::contextCreatedCallback()
{
    isInsideGLCallback = true;

    if (CachedImage* const c = CachedImage::get (component))
        c->initialiseOnThread();
    else
        jassertfalse;

    isInsideGLCallback = false;
}

void OpenGLContext::NativeContext::renderCallback()
{
    isInsideGLCallback = true;

    if (CachedImage* const c = CachedImage::get (component))
    {
        if (c->context.continuousRepaint)
            c->context.triggerRepaint();

        c->renderFrame();
    }

    isInsideGLCallback = false;
}
#endif

//==============================================================================
class OpenGLContext::Attachment  : public ComponentMovementWatcher
{
public:
    Attachment (OpenGLContext& c, Component& comp)
       : ComponentMovementWatcher (&comp), context (c)
    {
        if (canBeAttached (comp))
            attach();
    }

    ~Attachment()
    {
        detach();
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        Component& comp = *getComponent();

        if (isAttached (comp) != canBeAttached (comp))
            componentVisibilityChanged();

        if (comp.getWidth() > 0 && comp.getHeight() > 0
             && context.nativeContext != nullptr)
        {
            if (CachedImage* const c = CachedImage::get (comp))
                c->handleResize();

            if (ComponentPeer* peer = comp.getTopLevelComponent()->getPeer())
                context.nativeContext->updateWindowPosition (peer->getAreaCoveredBy (comp));
        }
    }

    void componentPeerChanged() override
    {
        detach();
        componentVisibilityChanged();
    }

    void componentVisibilityChanged() override
    {
        Component& comp = *getComponent();

        if (canBeAttached (comp))
        {
            if (! isAttached (comp))
                attach();
        }
        else
        {
            detach();
        }
    }

   #if JUCE_DEBUG || JUCE_LOG_ASSERTIONS
    void componentBeingDeleted (Component& c) override
    {
        /* You must call detach() or delete your OpenGLContext to remove it
           from a component BEFORE deleting the component that it is using!
        */
        jassertfalse;

        ComponentMovementWatcher::componentBeingDeleted (c);
    }
   #endif

private:
    OpenGLContext& context;

    static bool canBeAttached (const Component& comp) noexcept
    {
        return comp.getWidth() > 0 && comp.getHeight() > 0 && isShowingOrMinimised (comp);
    }

    static bool isShowingOrMinimised (const Component& c)
    {
        if (! c.isVisible())
            return false;

        if (Component* p = c.getParentComponent())
            return isShowingOrMinimised (*p);

        return c.getPeer() != nullptr;
    }

    static bool isAttached (const Component& comp) noexcept
    {
        return comp.getCachedComponentImage() != nullptr;
    }

    void attach()
    {
        Component& comp = *getComponent();
        CachedImage* const newCachedImage = new CachedImage (context, comp,
                                                             context.pixelFormat,
                                                             context.contextToShareWith);
        comp.setCachedComponentImage (newCachedImage);
        newCachedImage->start(); // (must wait until this is attached before starting its thread)
        newCachedImage->updateViewportSize (true);
    }

    void detach()
    {
        Component& comp = *getComponent();

       #if JUCE_MAC
        [[(NSView*) comp.getWindowHandle() window] disableScreenUpdatesUntilFlush];
       #endif

        if (CachedImage* const oldCachedImage = CachedImage::get (comp))
            oldCachedImage->stop(); // (must stop this before detaching it from the component)

        comp.setCachedComponentImage (nullptr);
        context.nativeContext = nullptr;
    }
};

//==============================================================================
OpenGLContext::OpenGLContext()
    : nativeContext (nullptr), renderer (nullptr), currentRenderScale (1.0),
      contextToShareWith (nullptr), renderComponents (true),
      useMultisampling (false), continuousRepaint (false)
{
}

OpenGLContext::~OpenGLContext()
{
    detach();
}

void OpenGLContext::setRenderer (OpenGLRenderer* rendererToUse) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    renderer = rendererToUse;
}

void OpenGLContext::setComponentPaintingEnabled (bool shouldPaintComponent) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    renderComponents = shouldPaintComponent;
}

void OpenGLContext::setContinuousRepainting (bool shouldContinuouslyRepaint) noexcept
{
    continuousRepaint = shouldContinuouslyRepaint;
}

void OpenGLContext::setPixelFormat (const OpenGLPixelFormat& preferredPixelFormat) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    pixelFormat = preferredPixelFormat;
}

void OpenGLContext::setNativeSharedContext (void* nativeContextToShareWith) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    contextToShareWith = nativeContextToShareWith;
}

void OpenGLContext::setMultisamplingEnabled (bool b) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    useMultisampling = b;
}

void OpenGLContext::attachTo (Component& component)
{
    component.repaint();

    if (getTargetComponent() != &component)
    {
        detach();
        attachment = new Attachment (*this, component);
    }
}

void OpenGLContext::detach()
{
    attachment = nullptr;
    nativeContext = nullptr;
}

bool OpenGLContext::isAttached() const noexcept
{
    return nativeContext != nullptr;
}

Component* OpenGLContext::getTargetComponent() const noexcept
{
    return attachment != nullptr ? attachment->getComponent() : nullptr;
}

static ThreadLocalValue<OpenGLContext*> currentThreadActiveContext;

OpenGLContext* OpenGLContext::getCurrentContext()
{
    return currentThreadActiveContext.get();
}

bool OpenGLContext::makeActive() const noexcept
{
    OpenGLContext*& current = currentThreadActiveContext.get();

    if (nativeContext != nullptr && nativeContext->makeActive())
    {
        current = const_cast<OpenGLContext*> (this);
        return true;
    }

    current = nullptr;
    return false;
}

bool OpenGLContext::isActive() const noexcept
{
    return nativeContext != nullptr && nativeContext->isActive();
}

void OpenGLContext::deactivateCurrentContext()
{
    NativeContext::deactivateCurrentContext();
    currentThreadActiveContext.get() = nullptr;
}

void OpenGLContext::triggerRepaint()
{
    if (CachedImage* const cachedImage = getCachedImage())
        cachedImage->triggerRepaint();
}

void OpenGLContext::swapBuffers()
{
    if (nativeContext != nullptr)
        nativeContext->swapBuffers();
}

unsigned int OpenGLContext::getFrameBufferID() const noexcept
{
    return nativeContext != nullptr ? nativeContext->getFrameBufferID() : 0;
}

bool OpenGLContext::setSwapInterval (int numFramesPerSwap)
{
    return nativeContext != nullptr && nativeContext->setSwapInterval (numFramesPerSwap);
}

int OpenGLContext::getSwapInterval() const
{
    return nativeContext != nullptr ? nativeContext->getSwapInterval() : 0;
}

void* OpenGLContext::getRawContext() const noexcept
{
    return nativeContext != nullptr ? nativeContext->getRawContext() : nullptr;
}

OpenGLContext::CachedImage* OpenGLContext::getCachedImage() const noexcept
{
    if (Component* const comp = getTargetComponent())
        return CachedImage::get (*comp);

    return nullptr;
}

bool OpenGLContext::areShadersAvailable() const
{
    CachedImage* const c = getCachedImage();
    return c != nullptr && c->shadersAvailable;
}

ReferenceCountedObject* OpenGLContext::getAssociatedObject (const char* name) const
{
    jassert (name != nullptr);

    CachedImage* const c = getCachedImage();

    // This method must only be called from an openGL rendering callback.
    jassert (c != nullptr && nativeContext != nullptr);
    jassert (getCurrentContext() != nullptr);

    const int index = c->associatedObjectNames.indexOf (name);
    return index >= 0 ? c->associatedObjects.getUnchecked (index) : nullptr;
}

void OpenGLContext::setAssociatedObject (const char* name, ReferenceCountedObject* newObject)
{
    jassert (name != nullptr);

    if (CachedImage* const c = getCachedImage())
    {
        // This method must only be called from an openGL rendering callback.
        jassert (nativeContext != nullptr);
        jassert (getCurrentContext() != nullptr);

        const int index = c->associatedObjectNames.indexOf (name);

        if (index >= 0)
        {
            c->associatedObjects.set (index, newObject);
        }
        else
        {
            c->associatedObjectNames.add (name);
            c->associatedObjects.add (newObject);
        }
    }
}

void OpenGLContext::copyTexture (const Rectangle<int>& targetClipArea,
                                 const Rectangle<int>& anchorPosAndTextureSize,
                                 const int contextWidth, const int contextHeight,
                                 bool flippedVertically)
{
    if (contextWidth <= 0 || contextHeight <= 0)
        return;

    JUCE_CHECK_OPENGL_ERROR
    glBlendFunc (GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable (GL_BLEND);

   #if JUCE_USE_OPENGL_SHADERS
    if (areShadersAvailable())
    {
        struct OverlayShaderProgram  : public ReferenceCountedObject
        {
            OverlayShaderProgram (OpenGLContext& context)
                : program (context), builder (program), params (program)
            {}

            static const OverlayShaderProgram& select (OpenGLContext& context)
            {
                static const char programValueID[] = "juceGLComponentOverlayShader";
                OverlayShaderProgram* program = static_cast <OverlayShaderProgram*> (context.getAssociatedObject (programValueID));

                if (program == nullptr)
                {
                    program = new OverlayShaderProgram (context);
                    context.setAssociatedObject (programValueID, program);
                }

                program->program.use();
                return *program;
            }

            struct ProgramBuilder
            {
                ProgramBuilder (OpenGLShaderProgram& prog)
                {
                    prog.addShader ("attribute " JUCE_HIGHP " vec2 position;"
                                    "uniform " JUCE_HIGHP " vec2 screenSize;"
                                    "varying " JUCE_HIGHP " vec2 pixelPos;"
                                    "void main()"
                                    "{"
                                    "pixelPos = position;"
                                    JUCE_HIGHP " vec2 scaled = position / (0.5 * screenSize.xy);"
                                    "gl_Position = vec4 (scaled.x - 1.0, 1.0 - scaled.y, 0, 1.0);"
                                    "}",
                                    GL_VERTEX_SHADER);

                    prog.addShader ("uniform sampler2D imageTexture;"
                                    "uniform " JUCE_HIGHP " float textureBounds[4];"
                                    "uniform " JUCE_HIGHP " vec2 vOffsetAndScale;"
                                    "varying " JUCE_HIGHP " vec2 pixelPos;"
                                    "void main()"
                                    "{"
                                     JUCE_HIGHP " vec2 texturePos = (pixelPos - vec2 (textureBounds[0], textureBounds[1]))"
                                                                      "/ vec2 (textureBounds[2], textureBounds[3]);"
                                     "gl_FragColor = texture2D (imageTexture, vec2 (texturePos.x, vOffsetAndScale.x + vOffsetAndScale.y * texturePos.y));"
                                    "}",
                                    GL_FRAGMENT_SHADER);
                    prog.link();
                }
            };

            struct Params
            {
                Params (OpenGLShaderProgram& prog)
                    : positionAttribute (prog, "position"),
                      screenSize (prog, "screenSize"),
                      imageTexture (prog, "imageTexture"),
                      textureBounds (prog, "textureBounds"),
                      vOffsetAndScale (prog, "vOffsetAndScale")
                {}

                void set (const float targetWidth, const float targetHeight, const Rectangle<float>& bounds, bool flipVertically) const
                {
                    const GLfloat m[] = { bounds.getX(), bounds.getY(), bounds.getWidth(), bounds.getHeight() };
                    textureBounds.set (m, 4);
                    imageTexture.set (0);
                    screenSize.set (targetWidth, targetHeight);

                    vOffsetAndScale.set (flipVertically ? 0.0f : 1.0f,
                                         flipVertically ? 1.0f : -1.0f);
                }

                OpenGLShaderProgram::Attribute positionAttribute;
                OpenGLShaderProgram::Uniform screenSize, imageTexture, textureBounds, vOffsetAndScale;
            };

            OpenGLShaderProgram program;
            ProgramBuilder builder;
            Params params;
        };

        const GLshort left   = (GLshort) targetClipArea.getX();
        const GLshort top    = (GLshort) targetClipArea.getY();
        const GLshort right  = (GLshort) targetClipArea.getRight();
        const GLshort bottom = (GLshort) targetClipArea.getBottom();
        const GLshort vertices[] = { left, bottom, right, bottom, left, top, right, top };

        const OverlayShaderProgram& program = OverlayShaderProgram::select (*this);
        program.params.set ((float) contextWidth, (float) contextHeight, anchorPosAndTextureSize.toFloat(), flippedVertically);

        GLuint vertexBuffer = 0;
        extensions.glGenBuffers (1, &vertexBuffer);
        extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        extensions.glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

        const GLuint index = (GLuint) program.params.positionAttribute.attributeID;
        extensions.glVertexAttribPointer (index, 2, GL_SHORT, GL_FALSE, 4, 0);
        extensions.glEnableVertexAttribArray (index);
        JUCE_CHECK_OPENGL_ERROR

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

        extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
        extensions.glUseProgram (0);
        extensions.glDisableVertexAttribArray (index);
        extensions.glDeleteBuffers (1, &vertexBuffer);
    }
    #if JUCE_USE_OPENGL_FIXED_FUNCTION
    else
    #endif
   #endif

   #if JUCE_USE_OPENGL_FIXED_FUNCTION
    {
        glEnable (GL_SCISSOR_TEST);
        glScissor (targetClipArea.getX(), contextHeight - targetClipArea.getBottom(),
                   targetClipArea.getWidth(), targetClipArea.getHeight());

        JUCE_CHECK_OPENGL_ERROR
        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        OpenGLHelpers::prepareFor2D (contextWidth, contextHeight);
        JUCE_CHECK_OPENGL_ERROR

        const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        const GLshort left   = (GLshort) anchorPosAndTextureSize.getX();
        const GLshort right  = (GLshort) anchorPosAndTextureSize.getRight();
        const GLshort top    = (GLshort) (contextHeight - anchorPosAndTextureSize.getY());
        const GLshort bottom = (GLshort) (contextHeight - anchorPosAndTextureSize.getBottom());
        const GLshort vertices[] = { left, bottom, right, bottom, left, top, right, top };
        glVertexPointer (2, GL_SHORT, 0, vertices);

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
        glDisable (GL_SCISSOR_TEST);
    }
   #endif

    JUCE_CHECK_OPENGL_ERROR
}
