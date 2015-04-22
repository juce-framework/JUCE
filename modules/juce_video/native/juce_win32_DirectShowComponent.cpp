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

namespace DirectShowHelpers
{
    bool checkDShowAvailability()
    {
        ComSmartPtr <IGraphBuilder> graph;
        return SUCCEEDED (graph.CoCreateInstance (CLSID_FilterGraph));
    }

    //======================================================================
    class VideoRenderer
    {
    public:
        VideoRenderer() {}
        virtual ~VideoRenderer() {}

        virtual HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                                ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd) = 0;

        virtual void setVideoWindow (HWND hwnd) = 0;
        virtual void setVideoPosition (HWND hwnd, long videoWidth, long videoHeight) = 0;
        virtual void repaintVideo (HWND hwnd, HDC hdc) = 0;
        virtual void displayModeChanged() = 0;
        virtual HRESULT getVideoSize (long& videoWidth, long& videoHeight) = 0;
    };

    //======================================================================
    class VMR7  : public VideoRenderer
    {
    public:
        VMR7() {}

        HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                        ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd)
        {
            ComSmartPtr <IVMRFilterConfig> filterConfig;

            HRESULT hr = baseFilter.CoCreateInstance (CLSID_VideoMixingRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"VMR-7");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (filterConfig);
            if (SUCCEEDED (hr))   hr = filterConfig->SetRenderingMode (VMRMode_Windowless);
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (windowlessControl);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetVideoClippingWindow (hwnd);
            if (SUCCEEDED (hr))   hr = windowlessControl->SetAspectRatioMode (VMR_ARMODE_LETTER_BOX);

            return hr;
        }

        void setVideoWindow (HWND hwnd)
        {
            windowlessControl->SetVideoClippingWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd, long videoWidth, long videoHeight)
        {
            RECT src, dest;

            SetRect (&src, 0, 0, videoWidth, videoHeight);
            GetClientRect (hwnd, &dest);

            windowlessControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND hwnd, HDC hdc)
        {
            windowlessControl->RepaintVideo (hwnd, hdc);
        }

        void displayModeChanged()
        {
            windowlessControl->DisplayModeChanged();
        }

        HRESULT getVideoSize (long& videoWidth, long& videoHeight)
        {
            return windowlessControl->GetNativeVideoSize (&videoWidth, &videoHeight, nullptr, nullptr);
        }

    private:
        ComSmartPtr <IVMRWindowlessControl> windowlessControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VMR7)
    };


    //======================================================================
#if JUCE_MEDIAFOUNDATION
    class EVR : public VideoRenderer
    {
    public:
        EVR() {}

        HRESULT create (ComSmartPtr <IGraphBuilder>& graphBuilder,
                        ComSmartPtr <IBaseFilter>& baseFilter, HWND hwnd)
        {
            ComSmartPtr <IMFGetService> getService;

            HRESULT hr = baseFilter.CoCreateInstance (CLSID_EnhancedVideoRenderer);

            if (SUCCEEDED (hr))   hr = graphBuilder->AddFilter (baseFilter, L"EVR");
            if (SUCCEEDED (hr))   hr = baseFilter.QueryInterface (getService);
            if (SUCCEEDED (hr))   hr = getService->GetService (MR_VIDEO_RENDER_SERVICE, IID_IMFVideoDisplayControl,
                                                               (LPVOID*) videoDisplayControl.resetAndGetPointerAddress());
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetVideoWindow (hwnd);
            if (SUCCEEDED (hr))   hr = videoDisplayControl->SetAspectRatioMode (MFVideoARMode_PreservePicture);

            return hr;
        }

        void setVideoWindow (HWND hwnd)
        {
            videoDisplayControl->SetVideoWindow (hwnd);
        }

        void setVideoPosition (HWND hwnd, long /*videoWidth*/, long /*videoHeight*/)
        {
            const MFVideoNormalizedRect src = { 0.0f, 0.0f, 1.0f, 1.0f };

            RECT dest;
            GetClientRect (hwnd, &dest);

            videoDisplayControl->SetVideoPosition (&src, &dest);
        }

        void repaintVideo (HWND /*hwnd*/, HDC /*hdc*/)
        {
            videoDisplayControl->RepaintVideo();
        }

        void displayModeChanged() {}

        HRESULT getVideoSize (long& videoWidth, long& videoHeight)
        {
            SIZE sz;
            HRESULT hr = videoDisplayControl->GetNativeVideoSize (&sz, nullptr);
            videoWidth  = sz.cx;
            videoHeight = sz.cy;
            return hr;
        }

    private:
        ComSmartPtr <IMFVideoDisplayControl> videoDisplayControl;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EVR)
    };
#endif
}


//======================================================================
class DirectShowComponent::DirectShowContext    : public AsyncUpdater
{
public:
    DirectShowContext (DirectShowComponent& c, VideoRendererType renderType)
        : component (c),
          hwnd (0),
          hdc (0),
          state (uninitializedState),
          hasVideo (false),
          videoWidth (0),
          videoHeight (0),
          type (renderType),
          needToUpdateViewport (true),
          needToRecreateNativeWindow (false)
    {
        CoInitialize (0);

        if (type == dshowDefault)
        {
            type = dshowVMR7;

           #if JUCE_MEDIAFOUNDATION
            if (SystemStats::getOperatingSystemType() >= SystemStats::WinVista)
                type = dshowEVR;
           #endif
        }
    }

    ~DirectShowContext()
    {
        release();
        CoUninitialize();
    }

    //======================================================================
    void updateWindowPosition (const Rectangle<int>& newBounds)
    {
        nativeWindow->setWindowPosition (newBounds);
    }

    void showWindow (bool shouldBeVisible)
    {
        nativeWindow->showWindow (shouldBeVisible);
    }

    //======================================================================
    void repaint()
    {
        if (hasVideo)
            videoRenderer->repaintVideo (nativeWindow->getHandle(), nativeWindow->getContext());
    }

    void updateVideoPosition()
    {
        if (hasVideo)
            videoRenderer->setVideoPosition (nativeWindow->getHandle(), videoWidth, videoHeight);
    }

    void displayResolutionChanged()
    {
        if (hasVideo)
            videoRenderer->displayModeChanged();
    }

    //======================================================================
    void peerChanged()
    {
        deleteNativeWindow();

        mediaEvent->SetNotifyWindow (0, 0, 0);
        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (nullptr);

        createNativeWindow();

        mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);
        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (hwnd);
    }

    void handleAsyncUpdate() override
    {
        if (hwnd != 0)
        {
            if (needToRecreateNativeWindow)
            {
                peerChanged();
                needToRecreateNativeWindow = false;
            }

            if (needToUpdateViewport)
            {
                updateVideoPosition();
                needToUpdateViewport = false;
            }

            repaint();
        }
        else
        {
            triggerAsyncUpdate();
        }
    }

    void recreateNativeWindowAsync()
    {
        needToRecreateNativeWindow = true;
        triggerAsyncUpdate();
    }

    void updateContextPosition()
    {
        needToUpdateViewport = true;
        triggerAsyncUpdate();
    }

    //======================================================================
    bool loadFile (const String& fileOrURLPath)
    {
        jassert (state == uninitializedState);

        if (! createNativeWindow())
            return false;

        HRESULT hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);

        // basic playback interfaces
        if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaControl);
        if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaPosition);
        if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (mediaEvent);
        if (SUCCEEDED (hr))   hr = graphBuilder.QueryInterface (basicAudio);

        // video renderer interface
        if (SUCCEEDED (hr))
        {
           #if JUCE_MEDIAFOUNDATION
            if (type == dshowEVR)
                videoRenderer = new DirectShowHelpers::EVR();
            else
           #endif
                videoRenderer = new DirectShowHelpers::VMR7();

            hr = videoRenderer->create (graphBuilder, baseFilter, hwnd);
        }

        // build filter graph
        if (SUCCEEDED (hr))
        {
            hr = graphBuilder->RenderFile (fileOrURLPath.toWideCharPointer(), nullptr);

            if (FAILED (hr))
            {
                // Annoyingly, if we don't run the msg loop between failing and deleting the window, the
                // whole OS message-dispatch system gets itself into a state, and refuses to deliver any
                // more messages for the whole app. (That's what happens in Win7, anyway)
                MessageManager::getInstance()->runDispatchLoopUntil (200);
            }
        }

        // remove video renderer if not connected (no video)
        if (SUCCEEDED (hr))
        {
            if (isRendererConnected())
            {
                hasVideo = true;
                hr = videoRenderer->getVideoSize (videoWidth, videoHeight);
            }
            else
            {
                hasVideo = false;
                graphBuilder->RemoveFilter (baseFilter);
                videoRenderer = nullptr;
                baseFilter = nullptr;
            }
        }

        // set window to receive events
        if (SUCCEEDED (hr))
            hr = mediaEvent->SetNotifyWindow ((OAHWND) hwnd, graphEventID, 0);

        if (SUCCEEDED (hr))
        {
            state = stoppedState;
            pause();
            return true;
        }

        // Note that if you're trying to open a file and this method fails, you may
        // just need to install a suitable codec. It seems that by default DirectShow
        // doesn't support a very good range of formats.
        release();
        return false;
    }

    void release()
    {
        if (mediaControl != nullptr)
            mediaControl->Stop();

        if (mediaEvent != nullptr)
            mediaEvent->SetNotifyWindow (0, 0, 0);

        if (videoRenderer != nullptr)
            videoRenderer->setVideoWindow (0);

        hasVideo = false;
        videoRenderer = nullptr;

        baseFilter = nullptr;
        basicAudio = nullptr;
        mediaEvent = nullptr;
        mediaPosition = nullptr;
        mediaControl = nullptr;
        graphBuilder = nullptr;

        state = uninitializedState;

        videoWidth = 0;
        videoHeight = 0;

        if (nativeWindow != nullptr)
            deleteNativeWindow();
    }

    void graphEventProc()
    {
        LONG ec;
        LONG_PTR p1, p2;

        jassert (mediaEvent != nullptr);

        while (SUCCEEDED (mediaEvent->GetEvent (&ec, &p1, &p2, 0)))
        {
            switch (ec)
            {
            case EC_REPAINT:
                component.repaint();
                break;

            case EC_COMPLETE:
                if (component.isLooping())
                    component.goToStart();
                else
                    component.stop();
                break;

            case EC_USERABORT:
            case EC_ERRORABORT:
            case EC_ERRORABORTEX:
                component.closeMovie();
                break;

            default:
                break;
            }

            mediaEvent->FreeEventParams (ec, p1, p2);
        }
    }

    //======================================================================
    void run()
    {
        mediaControl->Run();
        state = runningState;
    }

    void stop()
    {
        mediaControl->Stop();
        state = stoppedState;
    }

    void pause()
    {
        mediaControl->Pause();
        state = pausedState;
    }

    //======================================================================
    bool isInitialised() const noexcept  { return state != uninitializedState; }
    bool isRunning() const noexcept      { return state == runningState; }
    bool isPaused() const noexcept       { return state == pausedState; }
    bool isStopped() const noexcept      { return state == stoppedState; }
    bool containsVideo() const noexcept  { return hasVideo; }
    int getVideoWidth() const noexcept   { return (int) videoWidth; }
    int getVideoHeight() const noexcept  { return (int) videoHeight; }

    //======================================================================
    double getDuration() const
    {
        REFTIME duration;
        mediaPosition->get_Duration (&duration);
        return duration;
    }

    double getPosition() const
    {
        REFTIME seconds;
        mediaPosition->get_CurrentPosition (&seconds);
        return seconds;
    }

    //======================================================================
    void setSpeed (const float newSpeed)        { mediaPosition->put_Rate (newSpeed); }
    void setPosition (const double seconds)     { mediaPosition->put_CurrentPosition (seconds); }
    void setVolume (const float newVolume)      { basicAudio->put_Volume (convertToDShowVolume (newVolume)); }

    // in DirectShow, full volume is 0, silence is -10000
    static long convertToDShowVolume (const float vol) noexcept
    {
        if (vol >= 1.0f) return 0;
        if (vol <= 0.0f) return -10000;

        return roundToInt ((vol * 10000.0f) - 10000.0f);
    }

    float getVolume() const
    {
        long volume;
        basicAudio->get_Volume (&volume);
        return (volume + 10000) / 10000.0f;
    }

private:
    //======================================================================
    enum { graphEventID = WM_APP + 0x43f0 };

    DirectShowComponent& component;
    HWND hwnd;
    HDC hdc;

    enum State { uninitializedState, runningState, pausedState, stoppedState };
    State state;

    bool hasVideo;
    long videoWidth, videoHeight;

    VideoRendererType type;

    ComSmartPtr <IGraphBuilder> graphBuilder;
    ComSmartPtr <IMediaControl> mediaControl;
    ComSmartPtr <IMediaPosition> mediaPosition;
    ComSmartPtr <IMediaEventEx> mediaEvent;
    ComSmartPtr <IBasicAudio> basicAudio;
    ComSmartPtr <IBaseFilter> baseFilter;

    ScopedPointer <DirectShowHelpers::VideoRenderer> videoRenderer;

    bool needToUpdateViewport, needToRecreateNativeWindow;

    //======================================================================
    class NativeWindowClass   : private DeletedAtShutdown
    {
    public:
        bool isRegistered() const noexcept              { return atom != 0; }
        LPCTSTR getWindowClassName() const noexcept     { return (LPCTSTR) MAKELONG (atom, 0); }

        juce_DeclareSingleton_SingleThreaded_Minimal (NativeWindowClass)

    private:
        NativeWindowClass()
            : atom (0)
        {
            String windowClassName ("JUCE_DIRECTSHOW_");
            windowClassName << (int) (Time::currentTimeMillis() & 0x7fffffff);

            HINSTANCE moduleHandle = (HINSTANCE) Process::getCurrentModuleInstanceHandle();

            TCHAR moduleFile [1024] = { 0 };
            GetModuleFileName (moduleHandle, moduleFile, 1024);

            WNDCLASSEX wcex = { 0 };
            wcex.cbSize         = sizeof (wcex);
            wcex.style          = CS_OWNDC;
            wcex.lpfnWndProc    = (WNDPROC) wndProc;
            wcex.lpszClassName  = windowClassName.toWideCharPointer();
            wcex.hInstance      = moduleHandle;

            atom = RegisterClassEx (&wcex);
            jassert (atom != 0);
        }

        ~NativeWindowClass()
        {
            if (atom != 0)
                UnregisterClass (getWindowClassName(), (HINSTANCE) Process::getCurrentModuleInstanceHandle());

            clearSingletonInstance();
        }

        static LRESULT CALLBACK wndProc (HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
        {
            if (DirectShowContext* const c = (DirectShowContext*) GetWindowLongPtr (hwnd, GWLP_USERDATA))
            {
                switch (msg)
                {
                    case WM_NCHITTEST:          return HTTRANSPARENT;
                    case WM_ERASEBKGND:         return 1;
                    case WM_DISPLAYCHANGE:      c->displayResolutionChanged(); break;
                    case graphEventID:          c->graphEventProc(); return 0;
                    default:                    break;
                }
            }

            return DefWindowProc (hwnd, msg, wParam, lParam);
        }

        ATOM atom;

        JUCE_DECLARE_NON_COPYABLE (NativeWindowClass)
    };

    //======================================================================
    class NativeWindow
    {
    public:
        NativeWindow (HWND parentToAddTo, void* const userData)
            : hwnd (0), hdc (0)
        {
            NativeWindowClass* const wc = NativeWindowClass::getInstance();

            if (wc->isRegistered())
            {
                DWORD exstyle = 0;
                DWORD type = WS_CHILD;

                hwnd = CreateWindowEx (exstyle, wc->getWindowClassName(),
                                       L"", type, 0, 0, 0, 0, parentToAddTo, 0,
                                       (HINSTANCE) Process::getCurrentModuleInstanceHandle(), 0);

                if (hwnd != 0)
                {
                    hdc = GetDC (hwnd);
                    SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) userData);
                }
            }

            jassert (hwnd != 0);
        }

        ~NativeWindow()
        {
            if (hwnd != 0)
            {
                SetWindowLongPtr (hwnd, GWLP_USERDATA, (LONG_PTR) 0);
                DestroyWindow (hwnd);
            }
        }

        HWND getHandle() const noexcept   { return hwnd; }
        HDC getContext() const noexcept   { return hdc; }

        void setWindowPosition (const Rectangle<int>& newBounds)
        {
            SetWindowPos (hwnd, 0, newBounds.getX(), newBounds.getY(),
                          newBounds.getWidth(), newBounds.getHeight(),
                          SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOOWNERZORDER);
        }

        void showWindow (const bool shouldBeVisible)
        {
            ShowWindow (hwnd, shouldBeVisible ? SW_SHOWNA : SW_HIDE);
        }

    private:
        HWND hwnd;
        HDC hdc;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NativeWindow)
    };

    ScopedPointer<NativeWindow> nativeWindow;

    //======================================================================
    bool createNativeWindow()
    {
        jassert (nativeWindow == nullptr);

        if (ComponentPeer* const topLevelPeer = component.getTopLevelComponent()->getPeer())
        {
            nativeWindow = new NativeWindow ((HWND) topLevelPeer->getNativeHandle(), this);

            hwnd = nativeWindow->getHandle();

            if (hwnd != 0)
            {
                hdc = GetDC (hwnd);
                component.updateContextPosition();
                component.showContext (component.isShowing());
                return true;
            }
            else
            {
                nativeWindow = nullptr;
            }
        }
        else
        {
            jassertfalse;
        }

        return false;
    }

    void deleteNativeWindow()
    {
        jassert (nativeWindow != nullptr);
        ReleaseDC (hwnd, hdc);
        hwnd = 0;
        hdc = 0;
        nativeWindow = nullptr;
    }

    bool isRendererConnected()
    {
        ComSmartPtr <IEnumPins> enumPins;

        HRESULT hr = baseFilter->EnumPins (enumPins.resetAndGetPointerAddress());

        if (SUCCEEDED (hr))
            hr = enumPins->Reset();

        ComSmartPtr<IPin> pin;

        while (SUCCEEDED (hr)
                && enumPins->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
        {
            ComSmartPtr<IPin> otherPin;

            hr = pin->ConnectedTo (otherPin.resetAndGetPointerAddress());

            if (SUCCEEDED (hr))
            {
                PIN_DIRECTION direction;
                hr = pin->QueryDirection (&direction);

                if (SUCCEEDED (hr) && direction == PINDIR_INPUT)
                    return true;
            }
            else if (hr == VFW_E_NOT_CONNECTED)
            {
                hr = S_OK;
            }
        }

        return false;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowContext)
};

juce_ImplementSingleton_SingleThreaded (DirectShowComponent::DirectShowContext::NativeWindowClass)


//======================================================================
class DirectShowComponent::DirectShowComponentWatcher   : public ComponentMovementWatcher
{
public:
    DirectShowComponentWatcher (DirectShowComponent* const c)
        : ComponentMovementWatcher (c),
          owner (c)
    {
    }

    void componentMovedOrResized (bool /*wasMoved*/, bool /*wasResized*/) override
    {
        if (owner->videoLoaded)
            owner->updateContextPosition();
    }

    void componentPeerChanged() override
    {
        if (owner->videoLoaded)
            owner->recreateNativeWindowAsync();
    }

    void componentVisibilityChanged() override
    {
        if (owner->videoLoaded)
            owner->showContext (owner->isShowing());
    }

private:
    DirectShowComponent* const owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DirectShowComponentWatcher)
};


//======================================================================
DirectShowComponent::DirectShowComponent (VideoRendererType type)
    : videoLoaded (false),
      looping (false)
{
    setOpaque (true);
    context = new DirectShowContext (*this, type);
    componentWatcher = new DirectShowComponentWatcher (this);
}

DirectShowComponent::~DirectShowComponent()
{
    componentWatcher = nullptr;
}

bool DirectShowComponent::isDirectShowAvailable()
{
    static bool isDSAvailable = DirectShowHelpers::checkDShowAvailability();
    return isDSAvailable;
}

void DirectShowComponent::recreateNativeWindowAsync()
{
    context->recreateNativeWindowAsync();
    repaint();
}

void DirectShowComponent::updateContextPosition()
{
    context->updateContextPosition();

    if (getWidth() > 0 && getHeight() > 0)
        if (ComponentPeer* peer = getTopLevelComponent()->getPeer())
            context->updateWindowPosition (peer->getAreaCoveredBy (*this));
}

void DirectShowComponent::showContext (const bool shouldBeVisible)
{
    context->showWindow (shouldBeVisible);
}

void DirectShowComponent::paint (Graphics& g)
{
    if (videoLoaded)
        context->handleUpdateNowIfNeeded();
    else
        g.fillAll (Colours::grey);
}

//======================================================================
bool DirectShowComponent::loadMovie (const String& fileOrURLPath)
{
    closeMovie();

    videoLoaded = context->loadFile (fileOrURLPath);

    if (videoLoaded)
    {
        videoPath = fileOrURLPath;
        context->updateVideoPosition();
    }

    return videoLoaded;
}

bool DirectShowComponent::loadMovie (const File& videoFile)
{
    return loadMovie (videoFile.getFullPathName());
}

bool DirectShowComponent::loadMovie (const URL& videoURL)
{
    return loadMovie (videoURL.toString (false));
}

void DirectShowComponent::closeMovie()
{
    if (videoLoaded)
        context->release();

    videoLoaded = false;
    videoPath.clear();
}

//======================================================================
File DirectShowComponent::getCurrentMoviePath() const           { return videoPath; }
bool DirectShowComponent::isMovieOpen() const                   { return videoLoaded; }
double DirectShowComponent::getMovieDuration() const            { return videoLoaded ? context->getDuration() : 0.0; }
void DirectShowComponent::setLooping (const bool shouldLoop)    { looping = shouldLoop; }
bool DirectShowComponent::isLooping() const                     { return looping; }

void DirectShowComponent::getMovieNormalSize (int &width, int &height) const
{
    width = context->getVideoWidth();
    height = context->getVideoHeight();
}

//======================================================================
void DirectShowComponent::setBoundsWithCorrectAspectRatio (const Rectangle<int>& spaceToFitWithin,
                                                           RectanglePlacement placement)
{
    int normalWidth, normalHeight;
    getMovieNormalSize (normalWidth, normalHeight);

    const Rectangle<int> normalSize (0, 0, normalWidth, normalHeight);

    if (! (spaceToFitWithin.isEmpty() || normalSize.isEmpty()))
        setBounds (placement.appliedTo (normalSize, spaceToFitWithin));
    else
        setBounds (spaceToFitWithin);
}

//======================================================================
void DirectShowComponent::play()
{
    if (videoLoaded)
        context->run();
}

void DirectShowComponent::stop()
{
    if (videoLoaded)
        context->pause();
}

bool DirectShowComponent::isPlaying() const
{
    return context->isRunning();
}

void DirectShowComponent::goToStart()
{
    setPosition (0.0);
}

void DirectShowComponent::setPosition (const double seconds)
{
    if (videoLoaded)
        context->setPosition (seconds);
}

double DirectShowComponent::getPosition() const
{
    return videoLoaded ? context->getPosition() : 0.0;
}

void DirectShowComponent::setSpeed (const float newSpeed)
{
    if (videoLoaded)
        context->setSpeed (newSpeed);
}

void DirectShowComponent::setMovieVolume (const float newVolume)
{
    if (videoLoaded)
        context->setVolume (newVolume);
}

float DirectShowComponent::getMovieVolume() const
{
    return videoLoaded ? context->getVolume() : 0.0f;
}
