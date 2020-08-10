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

namespace juce
{

#if JUCE_IOS
struct AppInactivityCallback // NB: this is a duplicate of an internal declaration in juce_core
{
    virtual ~AppInactivityCallback() {}
    virtual void appBecomingInactive() = 0;
};

extern Array<AppInactivityCallback*> appBecomingInactiveCallbacks;

// On iOS, all GL calls will crash when the app is running in the background, so
// this prevents them from happening (which some messy locking behaviour)
struct iOSBackgroundProcessCheck  : public AppInactivityCallback
{
    iOSBackgroundProcessCheck()              { isBackgroundProcess(); appBecomingInactiveCallbacks.add (this); }
    ~iOSBackgroundProcessCheck() override    { appBecomingInactiveCallbacks.removeAllInstancesOf (this); }

    bool isBackgroundProcess()
    {
        const bool b = Process::isForegroundProcess();
        isForeground.set (b ? 1 : 0);
        return ! b;
    }

    void appBecomingInactive() override
    {
        int counter = 2000;

        while (--counter > 0 && isForeground.get() != 0)
            Thread::sleep (1);
    }

private:
    Atomic<int> isForeground;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (iOSBackgroundProcessCheck)
};

#endif

#if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
 extern JUCE_API double getScaleFactorForWindow (HWND);
#endif

//==============================================================================
class OpenGLContext::CachedImage  : public CachedComponentImage,
                                    private ThreadPoolJob
{
public:
    CachedImage (OpenGLContext& c, Component& comp,
                 const OpenGLPixelFormat& pixFormat, void* contextToShare)
        : ThreadPoolJob ("OpenGL Rendering"),
          context (c), component (comp)
    {
        nativeContext.reset (new NativeContext (component, pixFormat, contextToShare,
                                                c.useMultisampling, c.versionRequired));

        if (nativeContext->createdOk())
            context.nativeContext = nativeContext.get();
        else
            nativeContext.reset();
    }

    ~CachedImage() override
    {
        stop();
    }

    //==============================================================================
    void start()
    {
        if (nativeContext != nullptr)
        {
            renderThread.reset (new ThreadPool (1));
            resume();
        }
    }

    void stop()
    {
        if (renderThread != nullptr)
        {
            // make sure everything has finished executing
            destroying = true;

            if (workQueue.size() > 0)
            {
                if (! renderThread->contains (this))
                    resume();

                while (workQueue.size() != 0)
                    Thread::sleep (20);
            }

            pause();
            renderThread.reset();
        }

        hasInitialised = false;
    }

    //==============================================================================
    void pause()
    {
        signalJobShouldExit();
        messageManagerLock.abort();

        if (renderThread != nullptr)
        {
            repaintEvent.signal();
            renderThread->removeJob (this, true, -1);
        }
    }

    void resume()
    {
        if (renderThread != nullptr)
            renderThread->addJob (this, false);
    }

    //==============================================================================
    void paint (Graphics&) override
    {
        updateViewportSize (false);
    }

    bool invalidateAll() override
    {
        validArea.clear();
        triggerRepaint();
        return false;
    }

    bool invalidate (const Rectangle<int>& area) override
    {
        validArea.subtract (area.toFloat().transformedBy (transform).getSmallestIntegerContainer());
        triggerRepaint();
        return false;
    }

    void releaseResources() override
    {
        stop();
    }

    void triggerRepaint()
    {
        needsUpdate = 1;
        repaintEvent.signal();
    }

    //==============================================================================
    bool ensureFrameBufferSize()
    {
        auto fbW = cachedImageFrameBuffer.getWidth();
        auto fbH = cachedImageFrameBuffer.getHeight();

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

        auto previousFrameBufferTarget = OpenGLFrameBuffer::getCurrentFrameBufferTarget();
        cachedImageFrameBuffer.makeCurrentRenderingTarget();
        auto imageH = cachedImageFrameBuffer.getHeight();

        for (auto& r : list)
        {
            glScissor (r.getX(), imageH - r.getBottom(), r.getWidth(), r.getHeight());
            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        }

        glDisable (GL_SCISSOR_TEST);
        context.extensions.glBindFramebuffer (GL_FRAMEBUFFER, previousFrameBufferTarget);
        JUCE_CHECK_OPENGL_ERROR
    }

    bool renderFrame()
    {
        MessageManager::Lock::ScopedTryLockType mmLock (messageManagerLock, false);

        auto isUpdatingTestValue = true;
        auto isUpdating = needsUpdate.compare_exchange_strong (isUpdatingTestValue, false);

        if (context.renderComponents && isUpdating)
        {
            // This avoids hogging the message thread when doing intensive rendering.
            if (lastMMLockReleaseTime + 1 >= Time::getMillisecondCounter())
                Thread::sleep (2);

            while (! shouldExit())
            {
                doWorkWhileWaitingForLock (false);

                if (mmLock.retryLock())
                    break;
            }

            if (shouldExit())
                return false;
        }

        if (! context.makeActive())
            return false;

        NativeContext::Locker locker (*nativeContext);

        JUCE_CHECK_OPENGL_ERROR

        doWorkWhileWaitingForLock (true);

        if (context.renderer != nullptr)
        {
            glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
            context.currentRenderScale = scale;
            context.renderer->renderOpenGL();
            clearGLError();

            bindVertexArray();
        }

        if (context.renderComponents)
        {
            if (isUpdating)
            {
                paintComponent();

                if (! hasInitialised)
                    return false;

                messageManagerLock.exit();
                lastMMLockReleaseTime = Time::getMillisecondCounter();
            }

            glViewport (0, 0, viewportArea.getWidth(), viewportArea.getHeight());
            drawComponentBuffer();
        }

        context.swapBuffers();

        OpenGLContext::deactivateCurrentContext();
        return true;
    }

    void updateViewportSize (bool canTriggerUpdate)
    {
        if (auto* peer = component.getPeer())
        {
            auto localBounds = component.getLocalBounds();
            auto displayScale = Desktop::getInstance().getDisplays().findDisplayForRect (component.getTopLevelComponent()->getScreenBounds()).scale;

            auto newArea = peer->getComponent().getLocalArea (&component, localBounds).withZeroOrigin() * displayScale;

           #if JUCE_WINDOWS && JUCE_WIN_PER_MONITOR_DPI_AWARE
            auto newScale = getScaleFactorForWindow (nativeContext->getNativeHandle());
            auto desktopScale = Desktop::getInstance().getGlobalScaleFactor();

            if (! approximatelyEqual (1.0f, desktopScale))
                newScale *= desktopScale;
           #else
            auto newScale = displayScale;
           #endif

            if (scale != newScale || viewportArea != newArea)
            {
                scale = newScale;
                viewportArea = newArea;
                transform = AffineTransform::scale ((float) newArea.getWidth()  / (float) localBounds.getWidth(),
                                                    (float) newArea.getHeight() / (float) localBounds.getHeight());

                nativeContext->updateWindowPosition (peer->getAreaCoveredBy (component));

                if (canTriggerUpdate)
                    invalidateAll();
            }
        }
    }

    void bindVertexArray() noexcept
    {
       #if JUCE_OPENGL3
        if (vertexArrayObject != 0)
            context.extensions.glBindVertexArray (vertexArrayObject);
       #endif
    }

    void checkViewportBounds()
    {
        auto screenBounds = component.getTopLevelComponent()->getScreenBounds();

        if (lastScreenBounds != screenBounds)
            updateViewportSize (true);
    }

    void paintComponent()
    {
        // you mustn't set your own cached image object when attaching a GL context!
        jassert (get (component) == this);

        if (! ensureFrameBufferSize())
            return;

        RectangleList<int> invalid (viewportArea);
        invalid.subtract (validArea);
        validArea = viewportArea;

        if (! invalid.isEmpty())
        {
            clearRegionInFrameBuffer (invalid);

            {
                std::unique_ptr<LowLevelGraphicsContext> g (createOpenGLGraphicsContext (context, cachedImageFrameBuffer));
                g->clipToRectangleList (invalid);
                g->addTransform (transform);

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

       #if JUCE_WINDOWS
        // some stupidly old drivers are missing this function, so try to at least avoid a crash here,
        // but if you hit this assertion you may want to have your own version check before using the
        // component rendering stuff on such old drivers.
        jassert (context.extensions.glActiveTexture != nullptr);
        if (context.extensions.glActiveTexture != nullptr)
       #endif
            context.extensions.glActiveTexture (GL_TEXTURE0);

        glBindTexture (GL_TEXTURE_2D, cachedImageFrameBuffer.getTextureID());
        bindVertexArray();

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
    JobStatus runJob() override
    {
        {
            // Allow the message thread to finish setting-up the context before using it..
            MessageManager::Lock::ScopedTryLockType mmLock (messageManagerLock, false);

            do
            {
                if (shouldExit())
                    return ThreadPoolJob::jobHasFinished;

            } while (! mmLock.retryLock());
        }

        if (! initialiseOnThread())
        {
            hasInitialised = false;

            return ThreadPoolJob::jobHasFinished;
        }

        hasInitialised = true;

        while (! shouldExit())
        {
           #if JUCE_IOS
            if (backgroundProcessCheck.isBackgroundProcess())
            {
                repaintEvent.wait (300);
                continue;
            }
           #endif

            if (shouldExit())
                break;

           #if JUCE_MAC
            if (cvDisplayLinkWrapper != nullptr)
            {
                repaintEvent.wait (-1);
                renderFrame();
            }
            else
           #endif
            if (! renderFrame())
                repaintEvent.wait (5); // failed to render, so avoid a tight fail-loop.
            else if (! context.continuousRepaint && ! shouldExit())
                repaintEvent.wait (-1);
        }

        hasInitialised = false;
        context.makeActive();
        shutdownOnThread();
        OpenGLContext::deactivateCurrentContext();

        return ThreadPoolJob::jobHasFinished;
    }

    bool initialiseOnThread()
    {
        // On android, this can get called twice, so drop any previous state..
        associatedObjectNames.clear();
        associatedObjects.clear();
        cachedImageFrameBuffer.release();

        context.makeActive();

        if (! nativeContext->initialiseOnRenderThread (context))
            return false;

       #if JUCE_ANDROID
        // On android the context may be created in initialiseOnRenderThread
        // and we therefore need to call makeActive again
        context.makeActive();
       #endif

        context.extensions.initialise();

       #if JUCE_OPENGL3
        if (OpenGLShaderProgram::getLanguageVersion() > 1.2)
        {
            context.extensions.glGenVertexArrays (1, &vertexArrayObject);
            bindVertexArray();
        }
       #endif

        glViewport (0, 0, component.getWidth(), component.getHeight());

        nativeContext->setSwapInterval (1);

       #if ! JUCE_OPENGL_ES
        JUCE_CHECK_OPENGL_ERROR
        shadersAvailable = OpenGLShaderProgram::getLanguageVersion() > 0;
        clearGLError();
       #endif

        if (context.renderer != nullptr)
            context.renderer->newOpenGLContextCreated();

       #if JUCE_MAC
        if (context.continuousRepaint)
            cvDisplayLinkWrapper = std::make_unique<CVDisplayLinkWrapper> (this);
       #endif

        return true;
    }

    void shutdownOnThread()
    {
       #if JUCE_MAC
        cvDisplayLinkWrapper = nullptr;
       #endif

        if (context.renderer != nullptr)
            context.renderer->openGLContextClosing();

       #if JUCE_OPENGL3
        if (vertexArrayObject != 0)
            context.extensions.glDeleteVertexArrays (1, &vertexArrayObject);
       #endif

        associatedObjectNames.clear();
        associatedObjects.clear();
        cachedImageFrameBuffer.release();
        nativeContext->shutdownOnRenderThread();
    }

    //==============================================================================
    struct BlockingWorker  : public OpenGLContext::AsyncWorker
    {
        BlockingWorker (OpenGLContext::AsyncWorker::Ptr && workerToUse)
            : originalWorker (std::move (workerToUse))
        {}

        void operator() (OpenGLContext& calleeContext)
        {
            if (originalWorker != nullptr)
                (*originalWorker) (calleeContext);

            finishedSignal.signal();
        }

        void block() noexcept  { finishedSignal.wait(); }

        OpenGLContext::AsyncWorker::Ptr originalWorker;
        WaitableEvent finishedSignal;
    };

    bool doWorkWhileWaitingForLock (bool contextIsAlreadyActive)
    {
        bool contextActivated = false;

        for (OpenGLContext::AsyncWorker::Ptr work = workQueue.removeAndReturn (0);
             work != nullptr && (! shouldExit()); work = workQueue.removeAndReturn (0))
        {
            if ((! contextActivated) && (! contextIsAlreadyActive))
            {
                if (! context.makeActive())
                    break;

                contextActivated = true;
            }

            NativeContext::Locker locker (*nativeContext);

            (*work) (context);
            clearGLError();
        }

        if (contextActivated)
            OpenGLContext::deactivateCurrentContext();

        return shouldExit();
    }

    void execute (OpenGLContext::AsyncWorker::Ptr workerToUse, bool shouldBlock, bool calledFromDestructor = false)
    {
        if (calledFromDestructor || ! destroying)
        {
            if (shouldBlock)
            {
                auto blocker = new BlockingWorker (std::move (workerToUse));
                OpenGLContext::AsyncWorker::Ptr worker (*blocker);
                workQueue.add (worker);

                messageManagerLock.abort();
                context.triggerRepaint();

                blocker->block();
            }
            else
            {
                workQueue.add (std::move (workerToUse));

                messageManagerLock.abort();
                context.triggerRepaint();
            }
        }
        else
        {
            jassertfalse; // you called execute AFTER you detached your openglcontext
        }
    }

    //==============================================================================
    static CachedImage* get (Component& c) noexcept
    {
        return dynamic_cast<CachedImage*> (c.getCachedComponentImage());
    }

    //==============================================================================
    friend class NativeContext;
    std::unique_ptr<NativeContext> nativeContext;

    OpenGLContext& context;
    Component& component;

    OpenGLFrameBuffer cachedImageFrameBuffer;
    RectangleList<int> validArea;
    Rectangle<int> viewportArea, lastScreenBounds;
    double scale = 1.0;
    AffineTransform transform;
   #if JUCE_OPENGL3
    GLuint vertexArrayObject = 0;
   #endif

    StringArray associatedObjectNames;
    ReferenceCountedArray<ReferenceCountedObject> associatedObjects;

    WaitableEvent canPaintNowFlag, finishedPaintingFlag, repaintEvent;
   #if JUCE_OPENGL_ES
    bool shadersAvailable = true;
   #else
    bool shadersAvailable = false;
   #endif
    std::atomic<bool> hasInitialised { false }, needsUpdate { true }, destroying { false };
    uint32 lastMMLockReleaseTime = 0;

   #if JUCE_MAC
    struct CVDisplayLinkWrapper
    {
        CVDisplayLinkWrapper (CachedImage* im)
        {
            CVDisplayLinkCreateWithActiveCGDisplays (&displayLink);
            CVDisplayLinkSetOutputCallback (displayLink, &displayLinkCallback, im);
            CVDisplayLinkStart (displayLink);
        }

        ~CVDisplayLinkWrapper()
        {
            CVDisplayLinkStop (displayLink);
            CVDisplayLinkRelease (displayLink);
        }

        static CVReturn displayLinkCallback (CVDisplayLinkRef, const CVTimeStamp*, const CVTimeStamp*,
                                             CVOptionFlags, CVOptionFlags*, void* displayLinkContext)
        {
            auto* self = (CachedImage*) displayLinkContext;
            self->repaintEvent.signal();
            return kCVReturnSuccess;
        }

        CVDisplayLinkRef displayLink;
    };

    std::unique_ptr<CVDisplayLinkWrapper> cvDisplayLinkWrapper;
   #endif

    std::unique_ptr<ThreadPool> renderThread;
    ReferenceCountedArray<OpenGLContext::AsyncWorker, CriticalSection> workQueue;
    MessageManager::Lock messageManagerLock;

   #if JUCE_IOS
    iOSBackgroundProcessCheck backgroundProcessCheck;
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CachedImage)
};

//==============================================================================
class OpenGLContext::Attachment  : public ComponentMovementWatcher,
                                   private Timer
{
public:
    Attachment (OpenGLContext& c, Component& comp)
       : ComponentMovementWatcher (&comp), context (c)
    {
        if (canBeAttached (comp))
            attach();
    }

    ~Attachment() override
    {
        detach();
    }

    void detach()
    {
        auto& comp = *getComponent();
        stop();
        comp.setCachedComponentImage (nullptr);
        context.nativeContext = nullptr;
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        auto& comp = *getComponent();

        if (isAttached (comp) != canBeAttached (comp))
            componentVisibilityChanged();

        if (comp.getWidth() > 0 && comp.getHeight() > 0
             && context.nativeContext != nullptr)
        {
            if (auto* c = CachedImage::get (comp))
                c->handleResize();

            if (auto* peer = comp.getTopLevelComponent()->getPeer())
                context.nativeContext->updateWindowPosition (peer->getAreaCoveredBy (comp));
        }
    }

    using ComponentMovementWatcher::componentMovedOrResized;

    void componentPeerChanged() override
    {
        detach();
        componentVisibilityChanged();
    }

    void componentVisibilityChanged() override
    {
        auto& comp = *getComponent();

        if (canBeAttached (comp))
        {
            if (isAttached (comp))
                comp.repaint(); // (needed when windows are un-minimised)
            else
                attach();
        }
        else
        {
            detach();
        }
    }

    using ComponentMovementWatcher::componentVisibilityChanged;

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

    void update()
    {
        auto& comp = *getComponent();

        if (canBeAttached (comp))
            start();
        else
            stop();
    }

private:
    OpenGLContext& context;

    bool canBeAttached (const Component& comp) noexcept
    {
        return (! context.overrideCanAttach) && comp.getWidth() > 0 && comp.getHeight() > 0 && isShowingOrMinimised (comp);
    }

    static bool isShowingOrMinimised (const Component& c)
    {
        if (! c.isVisible())
            return false;

        if (auto* p = c.getParentComponent())
            return isShowingOrMinimised (*p);

        return c.getPeer() != nullptr;
    }

    static bool isAttached (const Component& comp) noexcept
    {
        return comp.getCachedComponentImage() != nullptr;
    }

    void attach()
    {
        auto& comp = *getComponent();
        auto* newCachedImage = new CachedImage (context, comp,
                                                context.openGLPixelFormat,
                                                context.contextToShareWith);
        comp.setCachedComponentImage (newCachedImage);

        start();
    }

    void stop()
    {
        stopTimer();

        auto& comp = *getComponent();

       #if JUCE_MAC
        [[(NSView*) comp.getWindowHandle() window] disableScreenUpdatesUntilFlush];
       #endif

        if (auto* oldCachedImage = CachedImage::get (comp))
            oldCachedImage->stop(); // (must stop this before detaching it from the component)
    }

    void start()
    {
        auto& comp = *getComponent();

        if (auto* cachedImage = CachedImage::get (comp))
        {
            cachedImage->start(); // (must wait until this is attached before starting its thread)
            cachedImage->updateViewportSize (true);

            startTimer (400);
        }
    }

    void timerCallback() override
    {
        if (auto* cachedImage = CachedImage::get (*getComponent()))
            cachedImage->checkViewportBounds();
    }
};

//==============================================================================
OpenGLContext::OpenGLContext()
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

    #if JUCE_MAC
     if (auto* component = getTargetComponent())
     {
         detach();
         attachment.reset (new Attachment (*this, *component));
     }
    #endif

    triggerRepaint();
}

void OpenGLContext::setPixelFormat (const OpenGLPixelFormat& preferredPixelFormat) noexcept
{
    // This method must not be called when the context has already been attached!
    // Call it before attaching your context, or use detach() first, before calling this!
    jassert (nativeContext == nullptr);

    openGLPixelFormat = preferredPixelFormat;
}

void OpenGLContext::setTextureMagnificationFilter (OpenGLContext::TextureMagnificationFilter magFilterMode) noexcept
{
    texMagFilter = magFilterMode;
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

void OpenGLContext::setOpenGLVersionRequired (OpenGLVersion v) noexcept
{
    versionRequired = v;
}

void OpenGLContext::attachTo (Component& component)
{
    component.repaint();

    if (getTargetComponent() != &component)
    {
        detach();
        attachment.reset (new Attachment (*this, component));
    }
}

void OpenGLContext::detach()
{
    if (auto* a = attachment.get())
    {
        a->detach(); // must detach before nulling our pointer
        attachment.reset();
    }

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

OpenGLContext* OpenGLContext::getContextAttachedTo (Component& c) noexcept
{
    if (auto* ci = CachedImage::get (c))
        return &(ci->context);

    return nullptr;
}

static ThreadLocalValue<OpenGLContext*> currentThreadActiveContext;

OpenGLContext* OpenGLContext::getCurrentContext()
{
    return currentThreadActiveContext.get();
}

bool OpenGLContext::makeActive() const noexcept
{
    auto& current = currentThreadActiveContext.get();

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
    if (auto* cachedImage = getCachedImage())
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
    if (auto* comp = getTargetComponent())
        return CachedImage::get (*comp);

    return nullptr;
}

bool OpenGLContext::areShadersAvailable() const
{
    auto* c = getCachedImage();
    return c != nullptr && c->shadersAvailable;
}

ReferenceCountedObject* OpenGLContext::getAssociatedObject (const char* name) const
{
    jassert (name != nullptr);

    auto* c = getCachedImage();

    // This method must only be called from an openGL rendering callback.
    jassert (c != nullptr && nativeContext != nullptr);
    jassert (getCurrentContext() != nullptr);

    auto index = c->associatedObjectNames.indexOf (name);
    return index >= 0 ? c->associatedObjects.getUnchecked (index).get() : nullptr;
}

void OpenGLContext::setAssociatedObject (const char* name, ReferenceCountedObject* newObject)
{
    jassert (name != nullptr);

    if (auto* c = getCachedImage())
    {
        // This method must only be called from an openGL rendering callback.
        jassert (nativeContext != nullptr);
        jassert (getCurrentContext() != nullptr);

        const int index = c->associatedObjectNames.indexOf (name);

        if (index >= 0)
        {
            if (newObject != nullptr)
            {
                c->associatedObjects.set (index, newObject);
            }
            else
            {
                c->associatedObjectNames.remove (index);
                c->associatedObjects.remove (index);
            }
        }
        else if (newObject != nullptr)
        {
            c->associatedObjectNames.add (name);
            c->associatedObjects.add (newObject);
        }
    }
}

void OpenGLContext::setImageCacheSize (size_t newSize) noexcept     { imageCacheMaxSize = newSize; }
size_t OpenGLContext::getImageCacheSize() const noexcept            { return imageCacheMaxSize; }

void OpenGLContext::execute (OpenGLContext::AsyncWorker::Ptr workerToUse, bool shouldBlock)
{
    if (auto* c = getCachedImage())
        c->execute (std::move (workerToUse), shouldBlock);
    else
        jassertfalse; // You must have attached the context to a component
}

//==============================================================================
struct DepthTestDisabler
{
    DepthTestDisabler() noexcept
    {
        glGetBooleanv (GL_DEPTH_TEST, &wasEnabled);

        if (wasEnabled)
            glDisable (GL_DEPTH_TEST);
    }

    ~DepthTestDisabler() noexcept
    {
        if (wasEnabled)
            glEnable (GL_DEPTH_TEST);
    }

    GLboolean wasEnabled;
};

//==============================================================================
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

    DepthTestDisabler depthDisabler;

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
                OverlayShaderProgram* program = static_cast<OverlayShaderProgram*> (context.getAssociatedObject (programValueID));

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
                    prog.addVertexShader (OpenGLHelpers::translateVertexShaderToV3 (
                        "attribute " JUCE_HIGHP " vec2 position;"
                        "uniform " JUCE_HIGHP " vec2 screenSize;"
                        "uniform " JUCE_HIGHP " float textureBounds[4];"
                        "uniform " JUCE_HIGHP " vec2 vOffsetAndScale;"
                        "varying " JUCE_HIGHP " vec2 texturePos;"
                        "void main()"
                        "{"
                          JUCE_HIGHP " vec2 scaled = position / (0.5 * screenSize.xy);"
                          "gl_Position = vec4 (scaled.x - 1.0, 1.0 - scaled.y, 0, 1.0);"
                          "texturePos = (position - vec2 (textureBounds[0], textureBounds[1])) / vec2 (textureBounds[2], textureBounds[3]);"
                          "texturePos = vec2 (texturePos.x, vOffsetAndScale.x + vOffsetAndScale.y * texturePos.y);"
                        "}"));

                    prog.addFragmentShader (OpenGLHelpers::translateFragmentShaderToV3 (
                        "uniform sampler2D imageTexture;"
                        "varying " JUCE_HIGHP " vec2 texturePos;"
                        "void main()"
                        "{"
                          "gl_FragColor = texture2D (imageTexture, texturePos);"
                        "}"));

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

        auto left   = (GLshort) targetClipArea.getX();
        auto top    = (GLshort) targetClipArea.getY();
        auto right  = (GLshort) targetClipArea.getRight();
        auto bottom = (GLshort) targetClipArea.getBottom();
        const GLshort vertices[] = { left, bottom, right, bottom, left, top, right, top };

        auto& program = OverlayShaderProgram::select (*this);
        program.params.set ((float) contextWidth, (float) contextHeight, anchorPosAndTextureSize.toFloat(), flippedVertically);

        GLuint vertexBuffer = 0;
        extensions.glGenBuffers (1, &vertexBuffer);
        extensions.glBindBuffer (GL_ARRAY_BUFFER, vertexBuffer);
        extensions.glBufferData (GL_ARRAY_BUFFER, sizeof (vertices), vertices, GL_STATIC_DRAW);

        auto index = (GLuint) program.params.positionAttribute.attributeID;
        extensions.glVertexAttribPointer (index, 2, GL_SHORT, GL_FALSE, 4, nullptr);
        extensions.glEnableVertexAttribArray (index);
        JUCE_CHECK_OPENGL_ERROR

        if (extensions.glCheckFramebufferStatus (GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
        {
            glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);

            extensions.glBindBuffer (GL_ARRAY_BUFFER, 0);
            extensions.glUseProgram (0);
            extensions.glDisableVertexAttribArray (index);
            extensions.glDeleteBuffers (1, &vertexBuffer);
        }
        else
        {
            clearGLError();
        }
    }
    else
    {
        jassert (attachment == nullptr); // Running on an old graphics card!
    }

    JUCE_CHECK_OPENGL_ERROR
}

#if JUCE_ANDROID
EGLDisplay OpenGLContext::NativeContext::display = EGL_NO_DISPLAY;
EGLDisplay OpenGLContext::NativeContext::config;

void OpenGLContext::NativeContext::surfaceCreated (LocalRef<jobject> holder)
{
    ignoreUnused (holder);

    if (auto* cachedImage = CachedImage::get (component))
    {
        if (auto* pool = cachedImage->renderThread.get())
        {
            if (! pool->contains (cachedImage))
            {
                cachedImage->resume();
                cachedImage->context.triggerRepaint();
            }
        }
    }
}

void OpenGLContext::NativeContext::surfaceDestroyed (LocalRef<jobject> holder)
{
    ignoreUnused (holder);

    // unlike the name suggests this will be called just before the
    // surface is destroyed. We need to pause the render thread.
    if (auto* cachedImage = CachedImage::get (component))
    {
        cachedImage->pause();

        if (auto* threadPool = cachedImage->renderThread.get())
            threadPool->waitForJobToFinish (cachedImage, -1);
    }
}
#endif

} // namespace juce
