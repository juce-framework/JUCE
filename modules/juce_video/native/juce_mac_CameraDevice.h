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

#if ! (defined (MAC_OS_X_VERSION_10_16) && MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_16)
 JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-declarations")
 #define JUCE_DEPRECATION_IGNORED 1
#endif

struct CameraDevice::Pimpl
{
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

        SEL runtimeErrorSel = NSSelectorFromString (nsStringLiteral ("captureSessionRuntimeError:"));

        [[NSNotificationCenter defaultCenter] addObserver: callbackDelegate
                                                 selector: runtimeErrorSel
                                                     name: AVCaptureSessionRuntimeErrorNotification
                                                   object: session];
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
        StringArray results;
        NSArray* devices = [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];

        for (AVCaptureDevice* device : devices)
            results.add (nsStringToJuce ([device localizedName]));

        return results;
    }

    AVCaptureSession* getCaptureSession()
    {
        return session;
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

            SEL runtimeErrorSel = NSSelectorFromString (nsStringLiteral ("captureSessionRuntimeError:"));
            addMethod (runtimeErrorSel, sessionRuntimeError, "v@:@");

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
        if (imageOutput == nil)
        {
            imageOutput = [[AVCaptureStillImageOutput alloc] init];
            auto imageSettings = [[NSDictionary alloc] initWithObjectsAndKeys: AVVideoCodecJPEG, AVVideoCodecKey, nil];
            [imageOutput setOutputSettings: imageSettings];
            [imageSettings release];
            [session addOutput: imageOutput];
        }
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
        if (imageOutput != nil)
        {
            [session removeOutput: imageOutput];
            [imageOutput release];
            imageOutput = nil;
        }
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
            NSArray* availableDevices = [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];

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
        if (imageOutput != nil)
            for (AVCaptureConnection* connection in imageOutput.connections)
                if ([connection isActive] && [connection isEnabled])
                    for (AVCaptureInputPort* port in [connection inputPorts])
                        if ([[port mediaType] isEqual: AVMediaTypeVideo])
                            return connection;

        return nil;
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

        if (auto* videoConnection = getVideoConnection())
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

                NSData* imageData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation: sampleBuffer];

                auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                handleImageCapture (image);

                WeakReference<Pimpl> weakRef (this);
                MessageManager::callAsync ([weakRef, image]() mutable
                {
                    if (weakRef != nullptr && weakRef->pictureTakenCallback != nullptr)
                        weakRef->pictureTakenCallback (image);
                });
            }];
        }
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
    AVCaptureStillImageOutput* imageOutput = nil;
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
        JUCE_AUTORELEASEPOOL
        {
            AVCaptureVideoPreviewLayer* previewLayer = [[AVCaptureVideoPreviewLayer alloc] init];
            AVCaptureSession* session = device.pimpl->getCaptureSession();

            [session stopRunning];
            [previewLayer setSession: session];
            [session startRunning];

            NSView* view = [[NSView alloc] init];
            [view setLayer: previewLayer];

            setView (view);
        }
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

#if JUCE_DEPRECATION_IGNORED
 JUCE_END_IGNORE_WARNINGS_GCC_LIKE
#endif
