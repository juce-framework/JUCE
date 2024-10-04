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

// The following definitions would normally be found in qedit.h, which is not part of the
// Windows SDK, and which is incompatible with newer versions of DirectX.
JUCE_COMCLASS (ISampleGrabberCB, "0579154A-2B53-4994-B0D0-E773148EFF85") : public IUnknown
{
    JUCE_COMCALL SampleCB (double, IMediaSample*) = 0;
    JUCE_COMCALL BufferCB (double, BYTE*, long) = 0;
};

JUCE_COMCLASS (ISampleGrabber, "6B652FFF-11FE-4fce-92AD-0266B5D7C78F") : public IUnknown
{
    JUCE_COMCALL SetOneShot (BOOL) = 0;
    JUCE_COMCALL SetMediaType (const AM_MEDIA_TYPE*) = 0;
    JUCE_COMCALL GetConnectedMediaType (AM_MEDIA_TYPE*) = 0;
    JUCE_COMCALL SetBufferSamples (BOOL) = 0;
    JUCE_COMCALL GetCurrentBuffer (long*, long*) = 0;
    JUCE_COMCALL GetCurrentSample (IMediaSample**) = 0;
    JUCE_COMCALL SetCallback (ISampleGrabberCB*, long) = 0;
};

constexpr CLSID CLSID_NullRenderer  = { 0xC1F400A4, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };
constexpr CLSID CLSID_SampleGrabber = { 0xC1F400A0, 0x3F08, 0x11d3, { 0x9F, 0x0B, 0x00, 0x60, 0x08, 0x03, 0x9E, 0x37 } };

//==============================================================================
struct CameraDevice::Pimpl  : public ChangeBroadcaster
{
    Pimpl (CameraDevice& ownerToUse, const String&, int index,
           int minWidth, int minHeight, int maxWidth, int maxHeight,
           bool /*highQuality*/)
       : owner (ownerToUse)
    {
        HRESULT hr = captureGraphBuilder.CoCreateInstance (CLSID_CaptureGraphBuilder2);
        if (FAILED (hr))
            return;

        filter = enumerateCameras (nullptr, index);
        if (filter == nullptr)
            return;

        hr = graphBuilder.CoCreateInstance (CLSID_FilterGraph);
        if (FAILED (hr))
            return;

        hr = captureGraphBuilder->SetFiltergraph (graphBuilder);
        if (FAILED (hr))
            return;

        mediaControl = graphBuilder.getInterface<IMediaControl>();
        if (mediaControl == nullptr)
            return;

        {
            ComSmartPtr<IAMStreamConfig> streamConfig;

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            hr = captureGraphBuilder->FindInterface (&PIN_CATEGORY_CAPTURE, nullptr, filter,
                                                     __uuidof (IAMStreamConfig), (void**) streamConfig.resetAndGetPointerAddress());
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            if (streamConfig != nullptr)
            {
                getVideoSizes (streamConfig);

                if (! selectVideoSize (streamConfig, minWidth, minHeight, maxWidth, maxHeight))
                    return;
            }
        }

        hr = graphBuilder->AddFilter (filter, _T ("Video Capture"));
        if (FAILED (hr))
            return;

        hr = smartTee.CoCreateInstance (CLSID_SmartTee);
        if (FAILED (hr))
            return;

        hr = graphBuilder->AddFilter (smartTee, _T ("Smart Tee"));
        if (FAILED (hr))
            return;

        if (! connectFilters (filter, smartTee))
            return;

        ComSmartPtr<IBaseFilter> sampleGrabberBase;
        hr = sampleGrabberBase.CoCreateInstance (CLSID_SampleGrabber);
        if (FAILED (hr))
            return;

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
        hr = sampleGrabberBase.QueryInterface (__uuidof (ISampleGrabber), sampleGrabber);
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE

        if (FAILED (hr))
            return;

        {
            AM_MEDIA_TYPE mt = {};
            mt.majortype = MEDIATYPE_Video;
            mt.subtype = MEDIASUBTYPE_RGB24;
            mt.formattype = FORMAT_VideoInfo;
            sampleGrabber->SetMediaType (&mt);
        }

        callback = becomeComSmartPtrOwner (new GrabberCallback (*this));
        hr = sampleGrabber->SetCallback (callback, 1);

        hr = graphBuilder->AddFilter (sampleGrabberBase, _T ("Sample Grabber"));
        if (FAILED (hr))
            return;

        ComSmartPtr<IPin> grabberInputPin;
        if (! (getPin (smartTee, PINDIR_OUTPUT, smartTeeCaptureOutputPin, "capture")
                && getPin (smartTee, PINDIR_OUTPUT, smartTeePreviewOutputPin, "preview")
                && getPin (sampleGrabberBase, PINDIR_INPUT, grabberInputPin)))
            return;

        hr = graphBuilder->Connect (smartTeePreviewOutputPin, grabberInputPin);
        if (FAILED (hr))
            return;

        AM_MEDIA_TYPE mt = {};
        hr = sampleGrabber->GetConnectedMediaType (&mt);

        if (auto* pVih = unalignedPointerCast<VIDEOINFOHEADER*> (mt.pbFormat))
        {
            width = pVih->bmiHeader.biWidth;
            height = pVih->bmiHeader.biHeight;
        }

        ComSmartPtr<IBaseFilter> nullFilter;
        hr = nullFilter.CoCreateInstance (CLSID_NullRenderer);
        hr = graphBuilder->AddFilter (nullFilter, _T ("Null Renderer"));

        if (connectFilters (sampleGrabberBase, nullFilter)
              && addGraphToRot())
        {
            openedSuccessfully = true;
        }
    }

    ~Pimpl()
    {
        if (mediaControl != nullptr)
            mediaControl->Stop();

        removeGraphFromRot();
        disconnectAnyViewers();

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

    bool openedOk() const noexcept       { return openedSuccessfully; }

    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallbackToUse)
    {
        {
            const ScopedLock sl (pictureTakenCallbackLock);

            jassert (pictureTakenCallbackToUse != nullptr);

            if (pictureTakenCallbackToUse == nullptr)
                return;

            pictureTakenCallback = std::move (pictureTakenCallbackToUse);
        }

        addUser();
    }

    void startRecordingToFile (const File& file, int quality)
    {
        addUser();
        isRecording = createFileCaptureFilter (file, quality);
    }

    void stopRecording()
    {
        if (isRecording)
        {
            removeFileCaptureFilter();
            removeUser();
            isRecording = false;
        }
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstRecordedTime;
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            addUser();

        listeners.add (listenerToAdd);
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);

        if (listeners.size() == 0)
            removeUser();
    }

    void callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });
    }

    void notifyPictureTakenIfNeeded (const Image& image)
    {
        {
            const ScopedLock sl (pictureTakenCallbackLock);

            if (pictureTakenCallback == nullptr)
                return;
        }

        MessageManager::callAsync ([weakRef = WeakReference<Pimpl> { this }, image]() mutable
                                   {
                                       if (weakRef == nullptr)
                                           return;

                                       NullCheckedInvocation::invoke (weakRef->pictureTakenCallback, image);
                                       weakRef->pictureTakenCallback = nullptr;
                                   });
    }

    void addUser()
    {
        if (openedSuccessfully && activeUsers++ == 0)
            mediaControl->Run();
    }

    void removeUser()
    {
        if (openedSuccessfully && --activeUsers == 0)
            mediaControl->Stop();
    }

    void handleFrame (double /*time*/, BYTE* buffer, long /*bufferSize*/)
    {
        if (recordNextFrameTime)
        {
            const double defaultCameraLatency = 0.1;

            firstRecordedTime = Time::getCurrentTime() - RelativeTime (defaultCameraLatency);
            recordNextFrameTime = false;

            ComSmartPtr<IPin> pin;
            if (getPin (filter, PINDIR_OUTPUT, pin))
            {
                if (auto pushSource = pin.getInterface<IAMPushSource>())
                {
                    REFERENCE_TIME latency = 0;
                    pushSource->GetLatency (&latency);

                    firstRecordedTime = firstRecordedTime - RelativeTime ((double) latency);
                }
            }
        }

        Image loadingImage (Image::RGB, width, height, true);
        const int lineStride = width * 3;

        {
            const Image::BitmapData destData (loadingImage, 0, 0, width, height, Image::BitmapData::writeOnly);

            for (int i = 0; i < height; ++i)
                memcpy (destData.getLinePointer ((height - 1) - i),
                        buffer + lineStride * i,
                        (size_t) lineStride);
        }

        if (! listeners.isEmpty())
            callListeners (loadingImage);

        notifyPictureTakenIfNeeded (loadingImage);

        sendChangeMessage();

        const ScopedLock sl (imageSwapLock);
        activeImage = loadingImage;
    }

    void drawCurrentImage (Graphics& g, Rectangle<int> area)
    {
        const auto imageToDraw = [this]
        {
            const ScopedLock sl (imageSwapLock);
            return activeImage;
        }();

        Rectangle<int> centred (RectanglePlacement (RectanglePlacement::centred)
                                    .appliedTo (Rectangle<int> (width, height), area));

        RectangleList<int> borders (area);
        borders.subtract (centred);
        g.setColour (Colours::black);
        g.fillRectList (borders);

        g.drawImage (imageToDraw, centred.getX(), centred.getY(),
                     centred.getWidth(), centred.getHeight(), 0, 0, width, height);
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
            if (auto fileSink = asfWriter.getInterface<IFileSinkFilter>())
            {
                hr = fileSink->SetFileName (file.getFullPathName().toWideCharPointer(), nullptr);

                if (SUCCEEDED (hr))
                {
                    hr = graphBuilder->AddFilter (asfWriter, _T ("AsfWriter"));

                    if (SUCCEEDED (hr))
                    {
                        if (auto asfConfig = asfWriter.getInterface<IConfigAsfWriter>())
                        {
                            asfConfig->SetIndexMode (true);
                            ComSmartPtr<IWMProfileManager> profileManager;

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
                            int maxFramesPerSecond = fps[jlimit (0, numElementsInArray (fps) - 1, quality & 0xff)];

                            if (((uint32_t) quality & 0xff000000) != 0) // (internal hacky way to pass explicit frame rates for testing)
                                maxFramesPerSecond = (quality >> 24) & 0xff;

                            prof = prof.replace ("$WIDTH", String (width))
                                       .replace ("$HEIGHT", String (height))
                                       .replace ("$AVGTIMEPERFRAME", String (10000000 / maxFramesPerSecond));

                            ComSmartPtr<IWMProfile> currentProfile;
                            hr = profileManager->LoadProfileByData (prof.toWideCharPointer(), currentProfile.resetAndGetPointerAddress());
                            hr = asfConfig->ConfigureFilterUsingProfile (currentProfile);

                            if (SUCCEEDED (hr))
                            {
                                ComSmartPtr<IPin> asfWriterInputPin;

                                if (getPin (asfWriter, PINDIR_INPUT, asfWriterInputPin, "Video Input 01"))
                                {
                                    hr = graphBuilder->Connect (smartTeeCaptureOutputPin, asfWriterInputPin);

                                    if (SUCCEEDED (hr) && openedSuccessfully && activeUsers > 0
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
        }

        removeFileCaptureFilter();

        if (openedSuccessfully && activeUsers > 0)
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

        if (openedSuccessfully && activeUsers > 0)
            mediaControl->Run();

        previewMaxFPS = 60;
    }

    static ComSmartPtr<IBaseFilter> enumerateCameras (StringArray* names, const int deviceIndexToOpen)
    {
        int index = 0;
        ComSmartPtr<ICreateDevEnum> pDevEnum;

        struct Deleter
        {
            void operator() (IUnknown* ptr) const noexcept { ptr->Release(); }
        };

        using ContextPtr = std::unique_ptr<IBindCtx, Deleter>;

        if (SUCCEEDED (pDevEnum.CoCreateInstance (CLSID_SystemDeviceEnum)))
        {
            ComSmartPtr<IEnumMoniker> enumerator;
            HRESULT hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, enumerator.resetAndGetPointerAddress(), 0);

            if (SUCCEEDED (hr) && enumerator != nullptr)
            {
                ComSmartPtr<IMoniker> moniker;
                ULONG fetched;

                while (enumerator->Next (1, moniker.resetAndGetPointerAddress(), &fetched) == S_OK)
                {
                    auto context = []
                    {
                        IBindCtx* ptr = nullptr;
                        [[maybe_unused]] const auto result = CreateBindCtx (0, &ptr);
                        return ContextPtr (ptr);
                    }();

                    ComSmartPtr<IBaseFilter> captureFilter;
                    JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
                    hr = moniker->BindToObject (context.get(), nullptr, __uuidof (IBaseFilter), (void**) captureFilter.resetAndGetPointerAddress());
                    JUCE_END_IGNORE_WARNINGS_GCC_LIKE

                    if (SUCCEEDED (hr))
                    {
                        ComSmartPtr<IPropertyBag> propertyBag;
                        hr = moniker->BindToStorage (context.get(), nullptr, IID_IPropertyBag, (void**) propertyBag.resetAndGetPointerAddress());

                        if (SUCCEEDED (hr))
                        {
                            VARIANT var;
                            var.vt = VT_BSTR;

                            hr = propertyBag->Read (_T ("FriendlyName"), &var, nullptr);
                            propertyBag = nullptr;

                            if (SUCCEEDED (hr))
                            {
                                if (names != nullptr)
                                    names->add (var.bstrVal);

                                if (index == deviceIndexToOpen)
                                    return captureFilter;

                                ++index;
                            }
                        }
                    }
                }
            }
        }

        return nullptr;
    }

    static StringArray getAvailableDevices()
    {
        StringArray devs;
        enumerateCameras (&devs, -1);
        return devs;
    }

    struct GrabberCallback   : public ComBaseClassHelperBase<ISampleGrabberCB>
    {
        explicit GrabberCallback (Pimpl& p)
            : owner (p) {}

        JUCE_COMRESULT QueryInterface (REFIID refId, void** result) override
        {
            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wlanguage-extension-token")
            if (refId == __uuidof (ISampleGrabberCB))
                return castToType<ISampleGrabberCB> (result);
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            return ComBaseClassHelperBase<ISampleGrabberCB>::QueryInterface (refId, result);
        }

        JUCE_COMRESULT SampleCB (double, IMediaSample*)  override { return E_FAIL; }

        JUCE_COMRESULT BufferCB (double time, BYTE* buffer, long bufferSize) override
        {
            owner.handleFrame (time, buffer, bufferSize);
            return S_OK;
        }

        Pimpl& owner;

        JUCE_DECLARE_NON_COPYABLE (GrabberCallback)
    };

    CameraDevice& owner;

    ComSmartPtr<GrabberCallback> callback;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    CriticalSection pictureTakenCallbackLock;
    std::function<void (const Image&)> pictureTakenCallback;

    bool isRecording = false, openedSuccessfully = false;
    int width = 0, height = 0;
    Time firstRecordedTime;

    Array<ViewerComponent*> viewerComps;

    ComSmartPtr<ICaptureGraphBuilder2> captureGraphBuilder;
    ComSmartPtr<IBaseFilter> filter, smartTee, asfWriter;
    ComSmartPtr<IGraphBuilder> graphBuilder;
    ComSmartPtr<ISampleGrabber> sampleGrabber;
    ComSmartPtr<IMediaControl> mediaControl;
    ComSmartPtr<IPin> smartTeePreviewOutputPin, smartTeeCaptureOutputPin;
    int activeUsers = 0;
    Array<int> widths, heights;
    DWORD graphRegistrationID;

    bool recordNextFrameTime = false;
    int previewMaxFPS = 60;

    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)

private:
    CriticalSection imageSwapLock;
    Image activeImage;

    void getVideoSizes (IAMStreamConfig* const streamConfig)
    {
        widths.clear();
        heights.clear();

        int count = 0, size = 0;
        streamConfig->GetNumberOfCapabilities (&count, &size);

        if (size == (int) sizeof (VIDEO_STREAM_CONFIG_CAPS))
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

        if (size == (int) sizeof (VIDEO_STREAM_CONFIG_CAPS))
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
        ComSmartPtr<IEnumPins> enumerator;
        ComSmartPtr<IPin> pin;

        filter->EnumPins (enumerator.resetAndGetPointerAddress());

        while (enumerator->Next (1, pin.resetAndGetPointerAddress(), nullptr) == S_OK)
        {
            PIN_DIRECTION dir;
            pin->QueryDirection (&dir);

            if (wantedDirection == dir)
            {
                PIN_INFO info = {};
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
        ComSmartPtr<IPin> in, out;

        return getPin (first, PINDIR_OUTPUT, out)
                && getPin (second, PINDIR_INPUT, in)
                && SUCCEEDED (graphBuilder->Connect (out, in));
    }

    bool addGraphToRot()
    {
        ComSmartPtr<IRunningObjectTable> rot;
        if (FAILED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            return false;

        ComSmartPtr<IMoniker> moniker;
        WCHAR buffer[128]{};
        HRESULT hr = CreateItemMoniker (_T ("!"), buffer, moniker.resetAndGetPointerAddress());
        if (FAILED (hr))
            return false;

        graphRegistrationID = 0;
        return SUCCEEDED (rot->Register (0, graphBuilder, moniker, &graphRegistrationID));
    }

    void removeGraphFromRot()
    {
        ComSmartPtr<IRunningObjectTable> rot;

        if (SUCCEEDED (GetRunningObjectTable (0, rot.resetAndGetPointerAddress())))
            rot->Revoke (graphRegistrationID);
    }

    void disconnectAnyViewers();

    static void deleteMediaType (AM_MEDIA_TYPE* const pmt)
    {
        if (pmt->cbFormat != 0)
            CoTaskMemFree ((PVOID) pmt->pbFormat);

        if (pmt->pUnk != nullptr)
            pmt->pUnk->Release();

        CoTaskMemFree (pmt);
    }

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public Component,
                                        public ChangeListener
{
    ViewerComponent (CameraDevice& d)
       : owner (d.pimpl.get()), maxFPS (15), lastRepaintTime (0)
    {
        setOpaque (true);
        owner->addChangeListener (this);
        owner->addUser();
        owner->viewerComps.add (this);
        setSize (owner->width, owner->height);
    }

    ~ViewerComponent() override
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
            owner->drawCurrentImage (g, getLocalBounds());
        else
            g.fillAll();
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        const int64 now = Time::currentTimeMillis();

        if (now >= lastRepaintTime + (1000 / maxFPS))
        {
            lastRepaintTime = now;
            repaint();

            if (owner != nullptr)
                maxFPS = owner->previewMaxFPS;
        }
    }

private:
    Pimpl* owner;
    int maxFPS;
    int64 lastRepaintTime;
};

void CameraDevice::Pimpl::disconnectAnyViewers()
{
    for (int i = viewerComps.size(); --i >= 0;)
        viewerComps.getUnchecked (i)->ownerDeleted();
}

String CameraDevice::getFileExtension()
{
    return ".wmv";
}
