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

interface ISampleGrabberCB  : public IUnknown
{
    virtual STDMETHODIMP SampleCB (double, IMediaSample*) = 0;
    virtual STDMETHODIMP BufferCB (double, BYTE*, long) = 0;
};

interface ISampleGrabber  : public IUnknown
{
    virtual HRESULT STDMETHODCALLTYPE SetOneShot (BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetMediaType (const AM_MEDIA_TYPE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetConnectedMediaType (AM_MEDIA_TYPE*) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetBufferSamples (BOOL) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentBuffer (long*, long*) = 0;
    virtual HRESULT STDMETHODCALLTYPE GetCurrentSample (IMediaSample**) = 0;
    virtual HRESULT STDMETHODCALLTYPE SetCallback (ISampleGrabberCB*, long) = 0;
};

static const IID IID_ISampleGrabberCB  = { 0x0579154A, 0x2B53, 0x4994, { 0xB0, 0xD0, 0xE7, 0x73, 0x14, 0x8E, 0xFF, 0x85 } };
static const IID IID_ISampleGrabber    = { 0x6B652FFF, 0x11FE, 0x4fce, { 0x92, 0xAD, 0x02, 0x66, 0xB5, 0xD7, 0xC7, 0x8F } };
static const CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
static const CLSID CLSID_NullRenderer  = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

//==============================================================================
class DShowCameraDeviceInteral  : public ChangeBroadcaster
{
public:
    DShowCameraDeviceInteral (CameraDevice* const owner_,
                              const ComSmartPtr <ICaptureGraphBuilder2>& captureGraphBuilder_,
                              const ComSmartPtr <IBaseFilter>& filter_,
                              int minWidth, int minHeight,
                              int maxWidth, int maxHeight)
      : owner (owner_),
        captureGraphBuilder (captureGraphBuilder_),
        filter (filter_),
        ok (false),
        imageNeedsFlipping (false),
        width (0),
        height (0),
        activeUsers (0),
        recordNextFrameTime (false),
        previewMaxFPS (60)
    {
        HRESULT hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);
        if (FAILED (hr))
            return;

        hr = captureGraphBuilder->SetFiltergraph (graphBuilder);
        if (FAILED (hr))
            return;

        hr = graphBuilder.QueryInterface (mediaControl);
        if (FAILED (hr))
            return;

        {
            ComSmartPtr <IAMStreamConfig> streamConfig;

            hr = captureGraphBuilder->FindInterface (&PIN_CATEGORY_CAPTURE, 0, filter,
                                                     IID_IAMStreamConfig, (void**) streamConfig.resetAndGetPointerAddress());

            if (streamConfig != nullptr)
            {
                getVideoSizes (streamConfig);

                if (! selectVideoSize (streamConfig, minWidth, minHeight, maxWidth, maxHeight))
                    return;
            }
        }

        hr = graphBuilder->AddFilter (filter, _T("Video Capture"));
        if (FAILED (hr))
            return;

        hr = smartTee.CoCreateInstance (CLSID_SmartTee);
        if (FAILED (hr))
            return;

        hr = graphBuilder->AddFilter (smartTee, _T("Smart Tee"));
        if (FAILED (hr))
            return;

        if (! connectFilters (filter, smartTee))
            return;

        ComSmartPtr <IBaseFilter> sampleGrabberBase;
        hr = sampleGrabberBase.CoCreateInstance (CLSID_SampleGrabber);
        if (FAILED (hr))
            return;

        hr = sampleGrabberBase.QueryInterface (IID_ISampleGrabber, sampleGrabber);
        if (FAILED (hr))
            return;

        {
            AM_MEDIA_TYPE mt = { 0 };
            mt.majortype = MEDIATYPE_Video;
            mt.subtype = MEDIASUBTYPE_RGB24;
            mt.formattype = FORMAT_VideoInfo;
            sampleGrabber->SetMediaType (&mt);
        }

        callback = new GrabberCallback (*this);
        hr = sampleGrabber->SetCallback (callback, 1);

        hr = graphBuilder->AddFilter (sampleGrabberBase, _T("Sample Grabber"));
        if (FAILED (hr))
            return;

        ComSmartPtr <IPin> grabberInputPin;
        if (! (getPin (smartTee, PINDIR_OUTPUT, smartTeeCaptureOutputPin, "capture")
                && getPin (smartTee, PINDIR_OUTPUT, smartTeePreviewOutputPin, "preview")
                && getPin (sampleGrabberBase, PINDIR_INPUT, grabberInputPin)))
            return;

        hr = graphBuilder->Connect (smartTeePreviewOutputPin, grabberInputPin);
        if (FAILED (hr))
            return;

        AM_MEDIA_TYPE mt = { 0 };
        hr = sampleGrabber->GetConnectedMediaType (&mt);
        VIDEOINFOHEADER* pVih = (VIDEOINFOHEADER*) (mt.pbFormat);
        width = pVih->bmiHeader.biWidth;
        height = pVih->bmiHeader.biHeight;

        ComSmartPtr <IBaseFilter> nullFilter;
        hr = nullFilter.CoCreateInstance (CLSID_NullRenderer);
        hr = graphBuilder->AddFilter (nullFilter, _T("Null Renderer"));

        if (connectFilters (sampleGrabberBase, nullFilter)
              && addGraphToRot())
        {
            activeImage = Image (Image::RGB, width, height, true);
            loadingImage = Image (Image::RGB, width, height, true);

            ok = true;
        }
    }

    ~DShowCameraDeviceInteral()
    {
        if (mediaControl != nullptr)
            mediaControl->Stop();

        removeGraphFromRot();

        for (int i = viewerComps.size(); --i >= 0;)
            viewerComps.getUnchecked(i)->ownerDeleted();

        if (sampleGrabber != nullptr)
        {
            sampleGrabber->SetCallback (nullptr, 0);
            sampleGrabber = nullptr;
        }

        callback = nullptr;
        graphBuilder = nullptr;
        mediaControl = nullptr;
        filter = nullptr;
        captureGraphBuilder = nullptr;
        smartTee = nullptr;
        smartTeePreviewOutputPin = nullptr;
        smartTeeCaptureOutputPin = nullptr;
        asfWriter = nullptr;
    }

    void addUser()
    {
        if (ok && activeUsers++ == 0)
            mediaControl->Run();
    }

    void removeUser()
    {
        if (ok && --activeUsers == 0)
            mediaControl->Stop();
    }

    int getPreviewMaxFPS() const
    {
        return previewMaxFPS;
    }

    void handleFrame (double /*time*/, BYTE* buffer, long /*bufferSize*/)
    {
        if (recordNextFrameTime)
        {
            const double defaultCameraLatency = 0.1;

            firstRecordedTime = Time::getCurrentTime() - RelativeTime (defaultCameraLatency);
            recordNextFrameTime = false;

            ComSmartPtr <IPin> pin;
            if (getPin (filter, PINDIR_OUTPUT, pin))
            {
                ComSmartPtr <IAMPushSource> pushSource;
                HRESULT hr = pin.QueryInterface (pushSource);

                if (pushSource != nullptr)
                {
                    REFERENCE_TIME latency = 0;
                    hr = pushSource->GetLatency (&latency);

                    firstRecordedTime = firstRecordedTime - RelativeTime ((double) latency);
                }
            }
        }

        {
            const int lineStride = width * 3;
            const ScopedLock sl (imageSwapLock);

            {
                const Image::BitmapData destData (loadingImage, 0, 0, width, height, Image::BitmapData::writeOnly);

                for (int i = 0; i < height; ++i)
                    memcpy (destData.getLinePointer ((height - 1) - i),
                            buffer + lineStride * i,
                            lineStride);
            }

            imageNeedsFlipping = true;
        }

        if (listeners.size() > 0)
            callListeners (loadingImage);

        sendChangeMessage();
    }

    void drawCurrentImage (Graphics& g, int x, int y, int w, int h)
    {
        if (imageNeedsFlipping)
        {
            const ScopedLock sl (imageSwapLock);
            std::swap (loadingImage, activeImage);
            imageNeedsFlipping = false;
        }

        RectanglePlacement rp (RectanglePlacement::centred);
        double dx = 0, dy = 0, dw = width, dh = height;
        rp.applyTo (dx, dy, dw, dh, x, y, w, h);
        const int rx = roundToInt (dx), ry = roundToInt (dy);
        const int rw = roundToInt (dw), rh = roundToInt (dh);

        {
            Graphics::ScopedSaveState ss (g);

            g.excludeClipRegion (Rectangle<int> (rx, ry, rw, rh));
            g.fillAll (Colours::black);
        }

        g.drawImage (activeImage, rx, ry, rw, rh, 0, 0, width, height);
    }

    bool createFileCaptureFilter (const File& file, int quality)
    {
        removeFileCaptureFilter();
        file.deleteFile();
        mediaControl->Stop();
        firstRecordedTime = Time();
        recordNextFrameTime = true;
        previewMaxFPS = 60;

        HRESULT hr = asfWriter.CoCreateInstance (CLSID_WMAsfWriter);

        if (SUCCEEDED (hr))
        {
            ComSmartPtr <IFileSinkFilter> fileSink;
            hr = asfWriter.QueryInterface (fileSink);

            if (SUCCEEDED (hr))
            {
                hr = fileSink->SetFileName (file.getFullPathName().toWideCharPointer(), 0);

                if (SUCCEEDED (hr))
                {
                    hr = graphBuilder->AddFilter (asfWriter, _T("AsfWriter"));

                    if (SUCCEEDED (hr))
                    {
                        ComSmartPtr <IConfigAsfWriter> asfConfig;
                        hr = asfWriter.QueryInterface (asfConfig);
                        asfConfig->SetIndexMode (true);
                        ComSmartPtr <IWMProfileManager> profileManager;
                        hr = WMCreateProfileManager (profileManager.resetAndGetPointerAddress());

                        // This gibberish is the DirectShow profile for a video-only wmv file.
                        String prof ("<profile version=\"589824\" storageformat=\"1\" name=\"Quality\" description=\"Quality type for output.\">"
                                       "<streamconfig majortype=\"{73646976-0000-0010-8000-00AA00389B71}\" streamnumber=\"1\" "
                                                     "streamname=\"Video Stream\" inputname=\"Video409\" bitrate=\"894960\" "
                                                     "bufferwindow=\"0\" reliabletransport=\"1\" decodercomplexity=\"AU\" rfc1766langid=\"en-us\">"
                                         "<videomediaprops maxkeyframespacing=\"50000000\" quality=\"90\"/>"
                                         "<wmmediatype subtype=\"{33564D57-0000-0010-8000-00AA00389B71}\" bfixedsizesamples=\"0\" "
                                                      "btemporalcompression=\"1\" lsamplesize=\"0\">"
                                         "<videoinfoheader dwbitrate=\"894960\" dwbiterrorrate=\"0\" avgtimeperframe=\"$AVGTIMEPERFRAME\">"
                                             "<rcsource left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/>"
                                             "<rctarget left=\"0\" top=\"0\" right=\"$WIDTH\" bottom=\"$HEIGHT\"/>"
                                             "<bitmapinfoheader biwidth=\"$WIDTH\" biheight=\"$HEIGHT\" biplanes=\"1\" bibitcount=\"24\" "
                                                               "bicompression=\"WMV3\" bisizeimage=\"0\" bixpelspermeter=\"0\" biypelspermeter=\"0\" "
                                                               "biclrused=\"0\" biclrimportant=\"0\"/>"
                                           "</videoinfoheader>"
                                         "</wmmediatype>"
                                       "</streamconfig>"
                                     "</profile>");

                        const int fps[] = { 10, 15, 30 };
                        int maxFramesPerSecond = fps [jlimit (0, numElementsInArray (fps) - 1, quality & 0xff)];

                        if ((quality & 0xff000000) != 0) // (internal hacky way to pass explicit frame rates for testing)
                            maxFramesPerSecond = (quality >> 24) & 0xff;

                        prof = prof.replace ("$WIDTH", String (width))
                                   .replace ("$HEIGHT", String (height))
                                   .replace ("$AVGTIMEPERFRAME", String (10000000 / maxFramesPerSecond));

                        ComSmartPtr <IWMProfile> currentProfile;
                        hr = profileManager->LoadProfileByData (prof.toWideCharPointer(), currentProfile.resetAndGetPointerAddress());
                        hr = asfConfig->ConfigureFilterUsingProfile (currentProfile);

                        if (SUCCEEDED (hr))
                        {
                            ComSmartPtr <IPin> asfWriterInputPin;

                            if (getPin (asfWriter, PINDIR_INPUT, asfWriterInputPin, "Video Input 01"))
                            {
                                hr = graphBuilder->Connect (smartTeeCaptureOutputPin, asfWriterInputPin);

                                if (SUCCEEDED (hr) && ok && activeUsers > 0
                                     && SUCCEEDED (mediaControl->Run()))
                                {
                                    previewMaxFPS = (quality < 2) ? 15 : 25; // throttle back the preview comps to try to leave the cpu free for encoding

                                    if ((quality & 0x00ff0000) != 0)  // (internal hacky way to pass explicit frame rates for testing)
                                        previewMaxFPS = (quality >> 16) & 0xff;

                                    return true;
                                }
                            }
                        }
                    }
                }
            }
        }

        removeFileCaptureFilter();

        if (ok && activeUsers > 0)
            mediaControl->Run();

        return false;
    }

    void removeFileCaptureFilter()
    {
        mediaControl->Stop();

        if (asfWriter != nullptr)
        {
            graphBuilder->RemoveFilter (asfWriter);
            asfWriter = nullptr;
        }

        if (ok && activeUsers > 0)
            mediaControl->Run();

        previewMaxFPS = 60;
    }

    //==============================================================================
    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            addUser();

        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeFirstMatchingValue (listenerToRemove);

        if (listeners.size() == 0)
            removeUser();
    }

    void callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);

        for (int i = listeners.size(); --i >= 0;)
            if (CameraDevice::Listener* const l = listeners[i])
                l->imageReceived (image);
    }

    //==============================================================================
    class DShowCaptureViewerComp   : public Component,
                                     public ChangeListener
    {
    public:
        DShowCaptureViewerComp (DShowCameraDeviceInteral* const owner_)
            : owner (owner_), maxFPS (15), lastRepaintTime (0)
        {
            setOpaque (true);
            owner->addChangeListener (this);
            owner->addUser();
            owner->viewerComps.add (this);
            setSize (owner->width, owner->height);
        }

        ~DShowCaptureViewerComp()
        {
            if (owner != nullptr)
            {
                owner->viewerComps.removeFirstMatchingValue (this);
                owner->removeUser();
                owner->removeChangeListener (this);
            }
        }

        void ownerDeleted()
        {
            owner = nullptr;
        }

        void paint (Graphics& g) override
        {
            g.setColour (Colours::black);
            g.setImageResamplingQuality (Graphics::lowResamplingQuality);

            if (owner != nullptr)
                owner->drawCurrentImage (g, 0, 0, getWidth(), getHeight());
            else
                g.fillAll (Colours::black);
        }

        void changeListenerCallback (ChangeBroadcaster*) override
        {
            const int64 now = Time::currentTimeMillis();

            if (now >= lastRepaintTime + (1000 / maxFPS))
            {
                lastRepaintTime = now;
                repaint();

                if (owner != nullptr)
                    maxFPS = owner->getPreviewMaxFPS();
            }
        }

    private:
        DShowCameraDeviceInteral* owner;
        int maxFPS;
        int64 lastRepaintTime;
    };

    //==============================================================================
    bool ok;
    int width, height;
    Time firstRecordedTime;

    Array <DShowCaptureViewerComp*> viewerComps;

private:
    CameraDevice* const owner;
    ComSmartPtr <ICaptureGraphBuilder2> captureGraphBuilder;
    ComSmartPtr <IBaseFilter> filter;
    ComSmartPtr <IBaseFilter> smartTee;
    ComSmartPtr <IGraphBuilder> graphBuilder;
    ComSmartPtr <ISampleGrabber> sampleGrabber;
    ComSmartPtr <IMediaControl> mediaControl;
    ComSmartPtr <IPin> smartTeePreviewOutputPin;
    ComSmartPtr <IPin> smartTeeCaptureOutputPin;
    ComSmartPtr <IBaseFilter> asfWriter;
    int activeUsers;
    Array <int> widths, heights;
    DWORD graphRegistrationID;

    CriticalSection imageSwapLock;
    bool imageNeedsFlipping;
    Image loadingImage;
    Image activeImage;

    bool recordNextFrameTime;
    int previewMaxFPS;

    void getVideoSizes (IAMStreamConfig* const streamConfig)
    {
        widths.clear();
        heights.clear();

        int count = 0, size = 0;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            for (int i = 0; i < count; ++i)
            {
                VIDEO_STREAM_CONFIG_CAPS scc;
                AM_MEDIA_TYPE* config;

                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    const int w = scc.InputSize.cx;
                    const int h = scc.InputSize.cy;

                    bool duplicate = false;

                    for (int j = widths.size(); --j >= 0;)
                    {
                        if (w == widths.getUnchecked (j) && h == heights.getUnchecked (j))
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (! duplicate)
                    {
                        DBG ("Camera capture size: " + String (w) + ", " + String (h));
                        widths.add (w);
                        heights.add (h);
                    }

                    deleteMediaType (config);
                }
            }
        }
    }

    bool selectVideoSize (IAMStreamConfig* const streamConfig,
                          const int minWidth, const int minHeight,
                          const int maxWidth, const int maxHeight)
    {
        int count = 0, size = 0, bestArea = 0, bestIndex = -1;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == sizeof (VIDEO_STREAM_CONFIG_CAPS))
        {
            AM_MEDIA_TYPE* config;
            VIDEO_STREAM_CONFIG_CAPS scc;

            for (int i = 0; i < count; ++i)
            {
                HRESULT hr = streamConfig->GetStreamCaps (i, &config, (BYTE*) &scc);

                if (SUCCEEDED (hr))
                {
                    if (scc.InputSize.cx >= minWidth
                         && scc.InputSize.cy >= minHeight
                         && scc.InputSize.cx <= maxWidth
                         && scc.InputSize.cy <= maxHeight)
                    {
                        int area = scc.InputSize.cx * scc.InputSize.cy;
                        if (area > bestArea)
                        {
                            bestIndex = i;
                            bestArea = area;
                        }
                    }

                    deleteMediaType (config);
                }
            }

            if (bestIndex >= 0)
            {
                HRESULT hr = streamConfig->GetStreamCaps (bestIndex, &config, (BYTE*) &scc);

                hr = streamConfig->SetFormat (config);
                deleteMediaType (config);
                return SUCCEEDED (hr);
            }
        }

        return false;
    }

    static bool getPin (IBaseFilter* filter, const PIN_DIRECTION wantedDirection,
                        ComSmartPtr<IPin>& result, const char* pinName = nullptr)
    {
        ComSmartPtr <IEnumPins> enumerator;
        ComSmartPtr <IPin> pin;

        filter->EnumPins (enumerator.resetAndGetPointerAddress());

        while (enumerator->Next (1, pin.resetAndGetPointerAddress(), 0) == S_OK)
        {
            PIN_DIRECTION dir;
            pin->QueryDirection (&dir);

            if (wantedDirection == dir)
            {
                PIN_INFO info = { 0 };
                pin->QueryPinInfo (&info);

                if (pinName == nullptr || String (pinName).equalsIgnoreCase (String (info.achName)))
                {
                    result = pin;
                    return true;
                }
            }
        }

        return false;
    }

    bool connectFilters (IBaseFilter* const first, IBaseFilter* const second) const
    {
        ComSmartPtr <IPin> in, out;

        return getPin (first, PINDIR_OUTPUT, out)
                && getPin (second, PINDIR_INPUT, in)
                && SUCCEEDED (graphBuilder->Connect (out, in));
    }

    bool addGraphToRot()
    {
        ComSmartPtr <IRunningObjectTable> rot;
        if (FAILED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            return false;

        ComSmartPtr <IMoniker> moniker;
        WCHAR buffer[128];
        HRESULT hr = CreateItemMoniker (_T("!"), buffer, moniker.resetAndGetPointerAddress());
        if (FAILED (hr))
            return false;

        graphRegistrationID = 0;
        return SUCCEEDED (rot->Register (0, graphBuilder, moniker, &graphRegistrationID));
    }

    void removeGraphFromRot()
    {
        ComSmartPtr <IRunningObjectTable> rot;

        if (SUCCEEDED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            rot->Revoke (graphRegistrationID);
    }

    static void deleteMediaType (AM_MEDIA_TYPE* const pmt)
    {
        if (pmt->cbFormat != 0)
            CoTaskMemFree ((PVOID) pmt->pbFormat);

        if (pmt->pUnk != nullptr)
            pmt->pUnk->Release();

        CoTaskMemFree (pmt);
    }

    //==============================================================================
    class GrabberCallback   : public ComBaseClassHelperBase <ISampleGrabberCB>
    {
    public:
        GrabberCallback (DShowCameraDeviceInteral& cam)
            : ComBaseClassHelperBase <ISampleGrabberCB> (0), owner (cam) {}

        JUCE_COMRESULT QueryInterface (REFIID refId, void** result)
        {
            if (refId == IID_ISampleGrabberCB)
                return castToType <ISampleGrabberCB> (result);

            return ComBaseClassHelperBase<ISampleGrabberCB>::QueryInterface (refId, result);
        }

        STDMETHODIMP SampleCB (double, IMediaSample*)  { return E_FAIL; }

        STDMETHODIMP BufferCB (double time, BYTE* buffer, long bufferSize)
        {
            owner.handleFrame (time, buffer, bufferSize);
            return S_OK;
        }

    private:
        DShowCameraDeviceInteral& owner;

        JUCE_DECLARE_NON_COPYABLE (GrabberCallback)
    };

    ComSmartPtr <GrabberCallback> callback;
    Array <CameraDevice::Listener*> listeners;
    CriticalSection listenerLock;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (DShowCameraDeviceInteral)
};


//==============================================================================
CameraDevice::CameraDevice (const String& nm, int /*index*/)
    : name (nm)
{
    isRecording = false;
}

CameraDevice::~CameraDevice()
{
    stopRecording();
    delete static_cast <DShowCameraDeviceInteral*> (internal);
    internal = nullptr;
}

Component* CameraDevice::createViewerComponent()
{
    return new DShowCameraDeviceInteral::DShowCaptureViewerComp (static_cast <DShowCameraDeviceInteral*> (internal));
}

String CameraDevice::getFileExtension()
{
    return ".wmv";
}

void CameraDevice::startRecordingToFile (const File& file, int quality)
{
    stopRecording();

    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
    d->addUser();
    isRecording = d->createFileCaptureFilter (file, quality);
}

Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
    return d->firstRecordedTime;
}

void CameraDevice::stopRecording()
{
    if (isRecording)
    {
        DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;
        d->removeFileCaptureFilter();
        d->removeUser();
        isRecording = false;
    }
}

void CameraDevice::addListener (Listener* listenerToAdd)
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;

    if (listenerToAdd != nullptr)
        d->addListener (listenerToAdd);
}

void CameraDevice::removeListener (Listener* listenerToRemove)
{
    DShowCameraDeviceInteral* const d = (DShowCameraDeviceInteral*) internal;

    if (listenerToRemove != nullptr)
        d->removeListener (listenerToRemove);
}


//==============================================================================
namespace
{
    ComSmartPtr <IBaseFilter> enumerateCameras (StringArray* const names,
                                                const int deviceIndexToOpen,
                                                String& name)
    {
        int index = 0;
        ComSmartPtr <IBaseFilter> result;

        ComSmartPtr <ICreateDevEnum> pDevEnum;
        HRESULT hr = pDevEnum.CoCreateInstance (CLSID_SystemDeviceEnum);

        if (SUCCEEDED (hr))
        {
            ComSmartPtr <IEnumMoniker> enumerator;
            hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, enumerator.resetAndGetPointerAddress(), 0);

            if (SUCCEEDED (hr) && enumerator != nullptr)
            {
                ComSmartPtr <IMoniker> moniker;
                ULONG fetched;

                while (enumerator->Next (1, moniker.resetAndGetPointerAddress(), &fetched) == S_OK)
                {
                    ComSmartPtr <IBaseFilter> captureFilter;
                    hr = moniker->BindToObject (0, 0, IID_IBaseFilter, (void**) captureFilter.resetAndGetPointerAddress());

                    if (SUCCEEDED (hr))
                    {
                        ComSmartPtr <IPropertyBag> propertyBag;
                        hr = moniker->BindToStorage (0, 0, IID_IPropertyBag, (void**) propertyBag.resetAndGetPointerAddress());

                        if (SUCCEEDED (hr))
                        {
                            VARIANT var;
                            var.vt = VT_BSTR;

                            hr = propertyBag->Read (_T("FriendlyName"), &var, 0);
                            propertyBag = nullptr;

                            if (SUCCEEDED (hr))
                            {
                                if (names != nullptr)
                                    names->add (var.bstrVal);

                                if (index == deviceIndexToOpen)
                                {
                                    name = var.bstrVal;
                                    result = captureFilter;
                                    break;
                                }

                                ++index;
                            }
                        }
                    }
                }
            }
        }

        return result;
    }
}

StringArray CameraDevice::getAvailableDevices()
{
    StringArray devs;
    String dummy;
    enumerateCameras (&devs, -1, dummy);
    return devs;
}

CameraDevice* CameraDevice::openDevice (int index,
                                        int minWidth, int minHeight,
                                        int maxWidth, int maxHeight)
{
    ComSmartPtr <ICaptureGraphBuilder2> captureGraphBuilder;
    HRESULT hr = captureGraphBuilder.CoCreateInstance (CLSID_CaptureGraphBuilder2);

    if (SUCCEEDED (hr))
    {
        String name;
        const ComSmartPtr <IBaseFilter> filter (enumerateCameras (0, index, name));

        if (filter != nullptr)
        {
            ScopedPointer <CameraDevice> cam (new CameraDevice (name, index));

            DShowCameraDeviceInteral* const intern
                = new DShowCameraDeviceInteral (cam, captureGraphBuilder, filter,
                                                minWidth, minHeight, maxWidth, maxHeight);
            cam->internal = intern;

            if (intern->ok)
                return cam.release();
        }
    }

    return nullptr;
}
