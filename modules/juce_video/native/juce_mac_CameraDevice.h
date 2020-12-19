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

struct CameraDevice::Pimpl
{
   #if defined (MAC_OS_X_VERSION_10_15) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_15
    #define JUCE_USE_NEW_APPLE_CAMERA_API 1
   #else
    #define JUCE_USE_NEW_APPLE_CAMERA_API 0
   #endif

   #if JUCE_USE_NEW_APPLE_CAMERA_API
    class PostCatalinaPhotoOutput
    {
    public:
        PostCatalinaPhotoOutput()
        {
            static PhotoOutputDelegateClass cls;
            delegate.reset ([cls.createInstance() init]);
        }

        void addImageCapture (AVCaptureSession* s)
        {
            if (imageOutput != nil)
                return;

            imageOutput = [[AVCapturePhotoOutput alloc] init];
            [s addOutput: imageOutput];
        }

        void removeImageCapture (AVCaptureSession* s)
        {
            if (imageOutput == nil)
                return;

            [s removeOutput: imageOutput];
            [imageOutput release];
            imageOutput = nil;
        }

        NSArray<AVCaptureConnection*>* getConnections() const
        {
            if (imageOutput != nil)
                return imageOutput.connections;

            return nil;
        }

        void triggerImageCapture (Pimpl& p)
        {
            if (imageOutput == nil)
                return;

            PhotoOutputDelegateClass::setOwner (delegate.get(), &p);

            [imageOutput capturePhotoWithSettings: [AVCapturePhotoSettings photoSettings]
                                         delegate: id<AVCapturePhotoCaptureDelegate> (delegate.get())];
        }

        static NSArray* getAvailableDevices()
        {
            auto* discovery = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes: @[AVCaptureDeviceTypeBuiltInWideAngleCamera,
                                                                                                  AVCaptureDeviceTypeExternalUnknown]
                                                                                     mediaType: AVMediaTypeVideo
                                                                                      position: AVCaptureDevicePositionUnspecified];
            return [discovery devices];
        }

    private:
        class PhotoOutputDelegateClass : public ObjCClass<NSObject>
        {
        public:
            PhotoOutputDelegateClass() : ObjCClass<NSObject> ("PhotoOutputDelegateClass_")
            {
                addMethod (@selector (captureOutput:didFinishProcessingPhoto:error:), didFinishProcessingPhoto, "v@:@@@");
                addIvar<Pimpl*> ("owner");
                registerClass();
            }

            static void didFinishProcessingPhoto (id self, SEL, AVCapturePhotoOutput*, AVCapturePhoto* photo, NSError* error)
            {
                if (error != nil)
                {
                    String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                    ignoreUnused (errorString);

                    JUCE_CAMERA_LOG ("Still picture capture failed, error: " + errorString);
                    jassertfalse;

                    return;
                }

                auto* imageData = [photo fileDataRepresentation];
                auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                getOwner (self).imageCaptureFinished (image);
            }

            static Pimpl& getOwner (id self) { return *getIvar<Pimpl*> (self, "owner"); }
            static void setOwner (id self, Pimpl* t) { object_setInstanceVariable (self, "owner", t); }
        };

        AVCapturePhotoOutput* imageOutput = nil;
        std::unique_ptr<NSObject, NSObjectDeleter> delegate;
    };
   #else
    struct PreCatalinaStillImageOutput
    {
    public:
        void addImageCapture (AVCaptureSession* s)
        {
            if (imageOutput != nil)
                return;

            const auto codecType =
                                  #if defined (MAC_OS_X_VERSION_10_13) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_13
                                   AVVideoCodecTypeJPEG;
                                  #else
                                   AVVideoCodecJPEG;
                                  #endif

            imageOutput = [[AVCaptureStillImageOutput alloc] init];
            auto imageSettings = [[NSDictionary alloc] initWithObjectsAndKeys: codecType, AVVideoCodecKey, nil];
            [imageOutput setOutputSettings: imageSettings];
            [imageSettings release];
            [s addOutput: imageOutput];
        }

        void removeImageCapture (AVCaptureSession* s)
        {
            if (imageOutput == nil)
                return;

            [s removeOutput: imageOutput];
            [imageOutput release];
            imageOutput = nil;
        }

        NSArray<AVCaptureConnection*>* getConnections() const
        {
            if (imageOutput != nil)
                return imageOutput.connections;

            return nil;
        }

        void triggerImageCapture (Pimpl& p)
        {
            if (auto* videoConnection = p.getVideoConnection())
            {
                [imageOutput captureStillImageAsynchronouslyFromConnection: videoConnection
                                                         completionHandler: ^(CMSampleBufferRef sampleBuffer, NSError* error)
                {
                    if (error != nil)
                    {
                        JUCE_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToJuce (error.localizedDescription));
                        jassertfalse;
                        return;
                    }

                    auto* imageData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation: sampleBuffer];
                    auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);
                    p.imageCaptureFinished (image);
                }];
            }
        }

        static NSArray* getAvailableDevices()
        {
            return [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
        }

    private:
        AVCaptureStillImageOutput* imageOutput = nil;
    };
   #endif

    Pimpl (CameraDevice& ownerToUse, const String& deviceNameToUse, int /*index*/,
           int /*minWidth*/, int /*minHeight*/,
           int /*maxWidth*/, int /*maxHeight*/,
           bool useHighQuality)
        : owner (ownerToUse),
          deviceName (deviceNameToUse)
    {
        session = [[AVCaptureSession alloc] init];

        session.sessionPreset = useHighQuality ? AVCaptureSessionPresetHigh
                                               : AVCaptureSessionPresetMedium;

        refreshConnections();

        static DelegateClass cls;
        callbackDelegate = (id<AVCaptureFileOutputRecordingDelegate>) [cls.createInstance() init];
        DelegateClass::setOwner (callbackDelegate, this);

        JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
        [[NSNotificationCenter defaultCenter] addObserver: callbackDelegate
                                                 selector: @selector (captureSessionRuntimeError:)
                                                     name: AVCaptureSessionRuntimeErrorNotification
                                                   object: session];
        JUCE_END_IGNORE_WARNINGS_GCC_LIKE
    }

    ~Pimpl()
    {
        [[NSNotificationCenter defaultCenter] removeObserver: callbackDelegate];

        [session stopRunning];
        removeInput();
        removeImageCapture();
        removeMovieCapture();
        [session release];
        [callbackDelegate release];
    }

    //==============================================================================
    bool openedOk() const noexcept       { return openingError.isEmpty(); }

    void startSession()
    {
        if (! [session isRunning])
            [session startRunning];
    }

    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr)
        {
            jassertfalse;
            return;
        }

        pictureTakenCallback = std::move (pictureTakenCallbackToUse);

        triggerImageCapture();
    }

    void startRecordingToFile (const File& file, int /*quality*/)
    {
        stopRecording();
        refreshIfNeeded();
        firstPresentationTime = Time::getCurrentTime();
        file.deleteFile();

        startSession();
        isRecording = true;
        [fileOutput startRecordingToOutputFileURL: createNSURLFromFile (file)
                                recordingDelegate: callbackDelegate];
    }

    void stopRecording()
    {
        if (isRecording)
        {
            [fileOutput stopRecording];
            isRecording = false;
        }
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstPresentationTime;
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.add (listenerToAdd);

        if (listeners.size() == 1)
            triggerImageCapture();
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);
    }

    static StringArray getAvailableDevices()
    {
        auto* devices = decltype (imageOutput)::getAvailableDevices();

        StringArray results;

        for (AVCaptureDevice* device : devices)
            results.add (nsStringToJuce ([device localizedName]));

        return results;
    }

    AVCaptureSession* getCaptureSession()
    {
        return session;
    }

    NSView* createVideoCapturePreview()
    {
        // The video preview must be created before the capture session is
        // started. Make sure you haven't called `addListener`,
        // `startRecordingToFile`, or `takeStillPicture` before calling this
        // function.
        jassert (! [session isRunning]);
        startSession();

        JUCE_AUTORELEASEPOOL
        {
            NSView* view = [[NSView alloc] init];
            [view setLayer: [AVCaptureVideoPreviewLayer layerWithSession: getCaptureSession()]];
            return view;
        }
    }

private:
    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("JUCECameraDelegate_")
        {
            addIvar<Pimpl*> ("owner");
            addProtocol (@protocol (AVCaptureFileOutputRecordingDelegate));

            addMethod (@selector (captureOutput:didStartRecordingToOutputFileAtURL:  fromConnections:),       didStartRecordingToOutputFileAtURL,   "v@:@@@");
            addMethod (@selector (captureOutput:didPauseRecordingToOutputFileAtURL:  fromConnections:),       didPauseRecordingToOutputFileAtURL,   "v@:@@@");
            addMethod (@selector (captureOutput:didResumeRecordingToOutputFileAtURL: fromConnections:),       didResumeRecordingToOutputFileAtURL,  "v@:@@@");
            addMethod (@selector (captureOutput:willFinishRecordingToOutputFileAtURL:fromConnections:error:), willFinishRecordingToOutputFileAtURL, "v@:@@@@");

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            addMethod (@selector (captureSessionRuntimeError:), sessionRuntimeError, "v@:@");
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

            registerClass();
        }

        static void setOwner (id self, Pimpl* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static Pimpl& getOwner (id self)               { return *getIvar<Pimpl*> (self, "owner"); }

    private:
        static void didStartRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static void didPauseRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static void didResumeRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*) {}
        static void willFinishRecordingToOutputFileAtURL (id, SEL, AVCaptureFileOutput*, NSURL*, NSArray*, NSError*) {}

        static void sessionRuntimeError (id self, SEL, NSNotification* notification)
        {
            JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

            NSError* error = notification.userInfo[AVCaptureSessionErrorKey];
            auto errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
            getOwner (self).cameraSessionRuntimeError (errorString);
        }
    };

    //==============================================================================
    void addImageCapture()
    {
        imageOutput.addImageCapture (session);
    }

    void addMovieCapture()
    {
        if (fileOutput == nil)
        {
            fileOutput = [[AVCaptureMovieFileOutput alloc] init];
            [session addOutput: fileOutput];
        }
    }

    void removeImageCapture()
    {
        imageOutput.removeImageCapture (session);
    }

    void removeMovieCapture()
    {
        if (fileOutput != nil)
        {
            [session removeOutput: fileOutput];
            [fileOutput release];
            fileOutput = nil;
        }
    }

    void removeCurrentSessionVideoInputs()
    {
        if (session != nil)
        {
            NSArray<AVCaptureDeviceInput*>* inputs = session.inputs;

            for (AVCaptureDeviceInput* input : inputs)
                if ([input.device hasMediaType: AVMediaTypeVideo])
                    [session removeInput:input];
        }
    }

    void addInput()
    {
        if (currentInput == nil)
        {
            auto* availableDevices = decltype (imageOutput)::getAvailableDevices();

            for (AVCaptureDevice* device : availableDevices)
            {
                if (deviceName == nsStringToJuce ([device localizedName]))
                {
                    removeCurrentSessionVideoInputs();

                    NSError* err = nil;
                    AVCaptureDeviceInput* inputDevice = [[AVCaptureDeviceInput alloc] initWithDevice: device
                                                                                               error: &err];

                    jassert (err == nil);

                    if ([session canAddInput: inputDevice])
                    {
                        [session addInput: inputDevice];
                        currentInput = inputDevice;
                    }
                    else
                    {
                        jassertfalse;
                        [inputDevice release];
                    }

                    return;
                }
            }
        }
    }

    void removeInput()
    {
        if (currentInput != nil)
        {
            [session removeInput: currentInput];
            [currentInput release];
            currentInput = nil;
        }
    }

    void refreshConnections()
    {
        [session beginConfiguration];
        removeInput();
        removeImageCapture();
        removeMovieCapture();
        addInput();
        addImageCapture();
        addMovieCapture();
        [session commitConfiguration];
    }

    void refreshIfNeeded()
    {
        if (getVideoConnection() == nullptr)
            refreshConnections();
    }

    AVCaptureConnection* getVideoConnection() const
    {
        auto* connections = imageOutput.getConnections();

        if (connections != nil)
            for (AVCaptureConnection* connection in connections)
                if ([connection isActive] && [connection isEnabled])
                    for (AVCaptureInputPort* port in [connection inputPorts])
                        if ([[port mediaType] isEqual: AVMediaTypeVideo])
                            return connection;

        return nil;
    }

    void imageCaptureFinished (const Image& image)
    {
        handleImageCapture (image);

        WeakReference<Pimpl> weakRef (this);
        MessageManager::callAsync ([weakRef, image]() mutable
        {
            if (weakRef != nullptr && weakRef->pictureTakenCallback != nullptr)
                weakRef->pictureTakenCallback (image);
        });
    }

    void handleImageCapture (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });

        if (! listeners.isEmpty())
            triggerImageCapture();
    }

    void triggerImageCapture()
    {
        refreshIfNeeded();

        startSession();

        if (auto* videoConnection = getVideoConnection())
            imageOutput.triggerImageCapture (*this);
    }

    void cameraSessionRuntimeError (const String& error)
    {
        JUCE_CAMERA_LOG ("cameraSessionRuntimeError(), error = " + error);

        if (owner.onErrorOccurred != nullptr)
            owner.onErrorOccurred (error);
    }

    //==============================================================================
    CameraDevice& owner;
    String deviceName;

    AVCaptureSession* session = nil;
    AVCaptureMovieFileOutput* fileOutput = nil;
   #if JUCE_USE_NEW_APPLE_CAMERA_API
    PostCatalinaPhotoOutput imageOutput;
   #else
    PreCatalinaStillImageOutput imageOutput;
   #endif
    AVCaptureDeviceInput* currentInput = nil;

    id<AVCaptureFileOutputRecordingDelegate> callbackDelegate = nil;
    String openingError;
    Time firstPresentationTime;
    bool isRecording = false;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    std::function<void (const Image&)> pictureTakenCallback = nullptr;

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (Pimpl)
    JUCE_DECLARE_NON_COPYABLE       (Pimpl)
};

//==============================================================================
struct CameraDevice::ViewerComponent  : public NSViewComponent
{
    ViewerComponent (CameraDevice& device)
    {
        setView (device.pimpl->createVideoCapturePreview());
    }

    ~ViewerComponent()
    {
        setView (nil);
    }

    JUCE_DECLARE_NON_COPYABLE (ViewerComponent)
};

String CameraDevice::getFileExtension()
{
    return ".mov";
}

#undef JUCE_USE_NEW_APPLE_CAMERA_API
