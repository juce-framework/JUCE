/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

struct CameraDevice::Pimpl
{
    using InternalOpenCameraResultCallback = std::function<void (const String& /*cameraId*/, const String& /*error*/)>;

    Pimpl (CameraDevice& ownerToUse, const String& cameraIdToUse, int /*index*/,
           int /*minWidth*/, int /*minHeight*/, int /*maxWidth*/, int /*maxHeight*/,
           bool useHighQuality)
        : owner (ownerToUse),
          cameraId (cameraIdToUse),
          captureSession (*this, useHighQuality)
    {
    }

    String getCameraId() const noexcept { return cameraId; }

    void open (InternalOpenCameraResultCallback cameraOpenCallbackToUse)
    {
        cameraOpenCallback = std::move (cameraOpenCallbackToUse);

        if (cameraOpenCallback == nullptr)
        {
            // A valid camera open callback must be passed.
            jassertfalse;
            return;
        }

        [AVCaptureDevice requestAccessForMediaType: AVMediaTypeVideo
                                 completionHandler: ^(BOOL granted)
         {
             // Access to video is required for camera to work,
             // black images will be produced otherwise!
             jassert (granted);

             ignoreUnused (granted);
         }];

        [AVCaptureDevice requestAccessForMediaType: AVMediaTypeAudio
                                 completionHandler: ^(BOOL granted)
         {
             // Access to audio is required for camera to work,
             // silence will be produced otherwise!
             jassert (granted);

             ignoreUnused (granted);
         }];

        captureSession.startSessionForDeviceWithId (cameraId);
    }

    bool openedOk() const noexcept { return captureSession.openedOk(); }

    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallbackToUse)
    {
        if (pictureTakenCallbackToUse == nullptr)
        {
            jassertfalse;
            return;
        }

        pictureTakenCallback = std::move (pictureTakenCallbackToUse);

        triggerStillPictureCapture();
    }

    void startRecordingToFile (const File& file, int /*quality*/)
    {
        file.deleteFile();

        captureSession.startRecording (file);
    }

    void stopRecording()
    {
        captureSession.stopRecording();
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return captureSession.getTimeOfFirstRecordedFrame();
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;

        JUCE_CAMERA_LOG ("Available camera devices: ");

        for (AVCaptureDevice* device in getDevices())
        {
            JUCE_CAMERA_LOG ("Device start----------------------------------");
            printDebugCameraInfo (device);
            JUCE_CAMERA_LOG ("Device end----------------------------------");

            results.add (nsStringToJuce (device.uniqueID));
        }

        return results;
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);
        listeners.add (listenerToAdd);

        if (listeners.size() == 1)
            triggerStillPictureCapture();
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.remove (listenerToRemove);
    }

private:
    static NSArray<AVCaptureDevice*>* getDevices()
    {
       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        if (iosVersion.major >= 10)
        {
            std::unique_ptr<NSMutableArray<AVCaptureDeviceType>, NSObjectDeleter> deviceTypes ([[NSMutableArray alloc] initWithCapacity: 2]);

            [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInWideAngleCamera];
            [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTelephotoCamera];

            if ((iosVersion.major == 10 && iosVersion.minor >= 2) || iosVersion.major >= 11)
                [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInDualCamera];

            if ((iosVersion.major == 11 && iosVersion.minor >= 1) || iosVersion.major >= 12)
                [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTrueDepthCamera];

            auto discoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes: deviceTypes.get()
                                                                                           mediaType: AVMediaTypeVideo
                                                                                            position: AVCaptureDevicePositionUnspecified];

            return [discoverySession devices];
        }
       #endif

        return [AVCaptureDevice devicesWithMediaType: AVMediaTypeVideo];
    }

    //==============================================================================
    static void printDebugCameraInfo (AVCaptureDevice* device)
    {
        auto position = device.position;

        String positionString = position == AVCaptureDevicePositionBack
                              ? "Back"
                              : position == AVCaptureDevicePositionFront
                                         ? "Front"
                                         : "Unspecified";

        JUCE_CAMERA_LOG ("Position: " + positionString);
        JUCE_CAMERA_LOG ("Model ID: " + nsStringToJuce (device.modelID));
        JUCE_CAMERA_LOG ("Localized name: " + nsStringToJuce (device.localizedName));
        JUCE_CAMERA_LOG ("Unique ID: " + nsStringToJuce (device.uniqueID));
        JUCE_CAMERA_LOG ("Lens aperture: " + String (device.lensAperture));

        JUCE_CAMERA_LOG ("Has flash: " + String ((int)device.hasFlash));
        JUCE_CAMERA_LOG ("Supports flash always on: " + String ((int)[device isFlashModeSupported: AVCaptureFlashModeOn]));
        JUCE_CAMERA_LOG ("Supports auto flash: " + String ((int)[device isFlashModeSupported: AVCaptureFlashModeAuto]));

        JUCE_CAMERA_LOG ("Has torch: " + String ((int)device.hasTorch));
        JUCE_CAMERA_LOG ("Supports torch always on: " + String ((int)[device isTorchModeSupported: AVCaptureTorchModeOn]));
        JUCE_CAMERA_LOG ("Supports auto torch: " + String ((int)[device isTorchModeSupported: AVCaptureTorchModeAuto]));

        JUCE_CAMERA_LOG ("Low light boost supported: " + String ((int)device.lowLightBoostEnabled));

        JUCE_CAMERA_LOG ("Supports auto white balance: " + String ((int)[device isWhiteBalanceModeSupported: AVCaptureWhiteBalanceModeAutoWhiteBalance]));
        JUCE_CAMERA_LOG ("Supports continuous auto white balance: " + String ((int)[device isWhiteBalanceModeSupported: AVCaptureWhiteBalanceModeContinuousAutoWhiteBalance]));

        JUCE_CAMERA_LOG ("Supports auto focus: " + String ((int)[device isFocusModeSupported: AVCaptureFocusModeAutoFocus]));
        JUCE_CAMERA_LOG ("Supports continuous auto focus: " + String ((int)[device isFocusModeSupported: AVCaptureFocusModeContinuousAutoFocus]));
        JUCE_CAMERA_LOG ("Supports point of interest focus: " + String ((int)device.focusPointOfInterestSupported));
        JUCE_CAMERA_LOG ("Smooth auto focus supported: " + String ((int)device.smoothAutoFocusSupported));
        JUCE_CAMERA_LOG ("Auto focus range restriction supported: " + String ((int)device.autoFocusRangeRestrictionSupported));

        JUCE_CAMERA_LOG ("Supports auto exposure: " + String ((int)[device isExposureModeSupported: AVCaptureExposureModeAutoExpose]));
        JUCE_CAMERA_LOG ("Supports continuous auto exposure: " + String ((int)[device isExposureModeSupported: AVCaptureExposureModeContinuousAutoExposure]));
        JUCE_CAMERA_LOG ("Supports custom exposure: " + String ((int)[device isExposureModeSupported: AVCaptureExposureModeCustom]));
        JUCE_CAMERA_LOG ("Supports point of interest exposure: " + String ((int)device.exposurePointOfInterestSupported));

       #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
        if (iosVersion.major >= 10)
        {
            JUCE_CAMERA_LOG ("Device type: " + nsStringToJuce (device.deviceType));
            JUCE_CAMERA_LOG ("Locking focus with custom lens position supported: " + String ((int)device.lockingFocusWithCustomLensPositionSupported));
        }
       #endif

       #if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
        if (iosVersion.major >= 11)
        {
            JUCE_CAMERA_LOG ("Min available video zoom factor: " + String (device.minAvailableVideoZoomFactor));
            JUCE_CAMERA_LOG ("Max available video zoom factor: " + String (device.maxAvailableVideoZoomFactor));
            JUCE_CAMERA_LOG ("Dual camera switch over video zoom factor: " + String (device.dualCameraSwitchOverVideoZoomFactor));
        }
       #endif

        JUCE_CAMERA_LOG ("Capture formats start-------------------");
        for (AVCaptureDeviceFormat* format in device.formats)
        {
            JUCE_CAMERA_LOG ("Capture format start------");
            printDebugCameraFormatInfo (format);
            JUCE_CAMERA_LOG ("Capture format end------");
        }
        JUCE_CAMERA_LOG ("Capture formats end-------------------");
    }

    static void printDebugCameraFormatInfo (AVCaptureDeviceFormat* format)
    {
        JUCE_CAMERA_LOG ("Media type: " + nsStringToJuce (format.mediaType));

        String colourSpaces;

        for (NSNumber* number in format.supportedColorSpaces)
        {
            switch ([number intValue])
            {
                case AVCaptureColorSpace_sRGB:   colourSpaces << "sRGB ";  break;
                case AVCaptureColorSpace_P3_D65: colourSpaces << "P3_D65 "; break;
                default: break;
            }
        }

        JUCE_CAMERA_LOG ("Supported colour spaces: " + colourSpaces);

        JUCE_CAMERA_LOG ("Video field of view: " + String (format.videoFieldOfView));
        JUCE_CAMERA_LOG ("Video max zoom factor: " + String (format.videoMaxZoomFactor));
        JUCE_CAMERA_LOG ("Video zoom factor upscale threshold: " + String (format.videoZoomFactorUpscaleThreshold));

        String videoFrameRateRangesString = "Video supported frame rate ranges: ";

        for (AVFrameRateRange* range in format.videoSupportedFrameRateRanges)
            videoFrameRateRangesString << frameRateRangeToString (range);
        JUCE_CAMERA_LOG (videoFrameRateRangesString);

        JUCE_CAMERA_LOG ("Video binned: " + String (int(format.videoBinned)));

       #if defined (__IPHONE_8_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_8_0
        if (iosVersion.major >= 8)
        {
            JUCE_CAMERA_LOG ("Video HDR supported: " + String (int (format.videoHDRSupported)));
            JUCE_CAMERA_LOG ("High resolution still image dimensions: " + getHighResStillImgDimensionsString (format.highResolutionStillImageDimensions));
            JUCE_CAMERA_LOG ("Min ISO: " + String (format.minISO));
            JUCE_CAMERA_LOG ("Max ISO: " + String (format.maxISO));
            JUCE_CAMERA_LOG ("Min exposure duration: " + cmTimeToString (format.minExposureDuration));

            String autoFocusSystemString;
            switch (format.autoFocusSystem)
            {
                case AVCaptureAutoFocusSystemPhaseDetection:    autoFocusSystemString = "PhaseDetection";    break;
                case AVCaptureAutoFocusSystemContrastDetection: autoFocusSystemString = "ContrastDetection"; break;
                default: autoFocusSystemString = "None";
            }
            JUCE_CAMERA_LOG ("Auto focus system: " + autoFocusSystemString);

            JUCE_CAMERA_LOG ("Standard (iOS 5.0) video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeStandard]));
            JUCE_CAMERA_LOG ("Cinematic video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeCinematic]));
            JUCE_CAMERA_LOG ("Auto video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeAuto]));
        }
       #endif

       #if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
        if (iosVersion.major >= 11)
        {
            JUCE_CAMERA_LOG ("Min zoom factor for depth data delivery: " + String (format.videoMinZoomFactorForDepthDataDelivery));
            JUCE_CAMERA_LOG ("Max zoom factor for depth data delivery: " + String (format.videoMaxZoomFactorForDepthDataDelivery));
        }
       #endif
    }

    static String getHighResStillImgDimensionsString (CMVideoDimensions d)
    {
        return "[" + String (d.width) + " " + String (d.height) + "]";
    }

    static String cmTimeToString (CMTime time)
    {
        CFStringRef timeDesc = CMTimeCopyDescription (NULL, time);
        String result = String::fromCFString (timeDesc);

        CFRelease (timeDesc);
        return result;
    }

    static String frameRateRangeToString (AVFrameRateRange* range)
    {
        String result;
        result << "[minFrameDuration: " + cmTimeToString (range.minFrameDuration);
        result << " maxFrameDuration: " + cmTimeToString (range.maxFrameDuration);
        result << " minFrameRate: " + String (range.minFrameRate);
        result << " maxFrameRate: " + String (range.maxFrameRate) << "] ";

        return result;
    }

    //==============================================================================
    class CaptureSession
    {
    public:
        CaptureSession (Pimpl& ownerToUse, bool useHighQuality)
            : owner (ownerToUse),
              captureSessionQueue (dispatch_queue_create ("JuceCameraDeviceBackgroundDispatchQueue", DISPATCH_QUEUE_SERIAL)),
              captureSession ([[AVCaptureSession alloc] init]),
              delegate (nullptr),
              stillPictureTaker (*this),
              videoRecorder (*this)
        {
            static SessionDelegateClass cls;
            delegate.reset ([cls.createInstance() init]);
            SessionDelegateClass::setOwner (delegate.get(), this);

          #pragma clang diagnostic push
          #pragma clang diagnostic ignored "-Wundeclared-selector"
            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStartRunning:)
                                                         name: AVCaptureSessionDidStartRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStopRunning:)
                                                         name: AVCaptureSessionDidStopRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionRuntimeError:)
                                                         name: AVCaptureSessionRuntimeErrorNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionWasInterrupted:)
                                                         name: AVCaptureSessionWasInterruptedNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionInterruptionEnded:)
                                                         name: AVCaptureSessionInterruptionEndedNotification
                                                       object: captureSession.get()];
           #pragma clang diagnostic pop

            dispatch_async (captureSessionQueue,^
                            {
                                [captureSession.get() setSessionPreset: useHighQuality ? AVCaptureSessionPresetHigh
                                                                                       : AVCaptureSessionPresetMedium];
                            });

            ++numCaptureSessions;
        }

        ~CaptureSession()
        {
            [[NSNotificationCenter defaultCenter] removeObserver: delegate.get()];

            stopRecording();

            if (--numCaptureSessions == 0)
            {
                dispatch_async (captureSessionQueue, ^
                                {
                                    if (captureSession.get().running)
                                        [captureSession.get() stopRunning];

                                    sessionClosedEvent.signal();
                                });

                sessionClosedEvent.wait (-1);
            }
        }

        bool openedOk() const noexcept { return sessionStarted; }

        void startSessionForDeviceWithId (const String& cameraIdToUse)
        {
            dispatch_async (captureSessionQueue,^
                            {
                                cameraDevice = [AVCaptureDevice deviceWithUniqueID: juceStringToNS (cameraIdToUse)];
                                auto audioDevice = [AVCaptureDevice defaultDeviceWithMediaType: AVMediaTypeAudio];

                                [captureSession.get() beginConfiguration];

                                // This will add just video...
                                auto error = addInputToDevice (cameraDevice);

                                if (error.isNotEmpty())
                                {
                                    WeakReference<CaptureSession> weakRef (this);

                                    MessageManager::callAsync ([weakRef, error]() mutable
                                    {
                                        if (weakRef != nullptr)
                                            weakRef->owner.cameraOpenCallback ({}, error);
                                    });

                                    return;
                                }

                                // ... so add audio explicitly here
                                error = addInputToDevice (audioDevice);

                                if (error.isNotEmpty())
                                {
                                    WeakReference<CaptureSession> weakRef (this);

                                    MessageManager::callAsync ([weakRef, error]() mutable
                                    {
                                        if (weakRef != nullptr)
                                            weakRef->owner.cameraOpenCallback ({}, error);
                                    });

                                    return;
                                }

                                [captureSession.get() commitConfiguration];

                                if (! captureSession.get().running)
                                    [captureSession.get() startRunning];
                            });
        }

        AVCaptureVideoPreviewLayer* createPreviewLayer()
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassertfalse;
                return nullptr;
            }

            previewLayer = [AVCaptureVideoPreviewLayer layerWithSession: captureSession.get()];
            return previewLayer;
        }

        void takeStillPicture()
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassert (openedOk());
                return;
            }

            stillPictureTaker.takePicture (previewLayer.connection.videoOrientation);
        }

        void startRecording (const File& file)
        {
            if (! openedOk())
            {
                // A session must be started first!
                jassertfalse;
                return;
            }

            if (file.existsAsFile())
            {
                // File overwriting is not supported by iOS video recorder, the target
                // file must not exist.
                jassertfalse;
                return;
            }

            videoRecorder.startRecording (file, previewLayer.connection.videoOrientation);
        }

        void stopRecording()
        {
            videoRecorder.stopRecording();
        }

        Time getTimeOfFirstRecordedFrame() const
        {
            return videoRecorder.getTimeOfFirstRecordedFrame();
        }

        JUCE_DECLARE_WEAK_REFERENCEABLE (CaptureSession)

    private:
        String addInputToDevice (AVCaptureDevice* device)
        {
            NSError* error = nil;

            auto input = [AVCaptureDeviceInput deviceInputWithDevice: device
                                                               error: &error];

            if (error != nil)
                return nsStringToJuce (error.localizedDescription);

            if (! [captureSession.get() canAddInput: input])
                return "Could not add input to camera session.";

            [captureSession.get() addInput: input];
            return {};
        }

        //==============================================================================
        struct SessionDelegateClass    : public ObjCClass<NSObject>
        {
            SessionDelegateClass()  : ObjCClass<NSObject> ("SessionDelegateClass_")
            {
               #pragma clang diagnostic push
               #pragma clang diagnostic ignored "-Wundeclared-selector"
                addMethod (@selector (sessionDidStartRunning:),   started,           "v@:@");
                addMethod (@selector (sessionDidStopRunning:),    stopped,           "v@:@");
                addMethod (@selector (sessionRuntimeError:),      runtimeError,      "v@:@");
                addMethod (@selector (sessionWasInterrupted:),    interrupted,       "v@:@");
                addMethod (@selector (sessionInterruptionEnded:), interruptionEnded, "v@:@");
               #pragma clang diagnostic pop

                addIvar<CaptureSession*> ("owner");

                registerClass();
            }

            //==============================================================================
            static CaptureSession& getOwner (id self)         { return *getIvar<CaptureSession*> (self, "owner"); }
            static void setOwner (id self, CaptureSession* s) { object_setInstanceVariable (self, "owner", s); }

        private:
            //==============================================================================
            static void started (id self, SEL, NSNotification* notification)
            {
                JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                ignoreUnused (notification);

                dispatch_async (dispatch_get_main_queue(),
                                ^{
                                    getOwner (self).cameraSessionStarted();
                                });
            }

            static void stopped (id, SEL, NSNotification* notification)
            {
                JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                ignoreUnused (notification);
            }

            static void runtimeError (id self, SEL, NSNotification* notification)
            {
                JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                dispatch_async (dispatch_get_main_queue(),
                                ^{
                                    NSError* error = notification.userInfo[AVCaptureSessionErrorKey];
                                    auto errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                                    getOwner (self).cameraSessionRuntimeError (errorString);
                                });
            }

            static void interrupted (id, SEL, NSNotification* notification)
            {
                JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                ignoreUnused (notification);
            }

            static void interruptionEnded (id, SEL, NSNotification* notification)
            {
                JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                ignoreUnused (notification);
            }
        };

        //==============================================================================
        class StillPictureTaker
        {
        public:
            StillPictureTaker (CaptureSession& cs)
                : captureSession (cs),
                  captureOutput (createCaptureOutput()),
                  photoOutputDelegate (nullptr)
            {
                if (Pimpl::getIOSVersion().major >= 10)
                {
                    static PhotoOutputDelegateClass cls;
                    photoOutputDelegate.reset ([cls.createInstance() init]);
                    PhotoOutputDelegateClass::setOwner (photoOutputDelegate.get(), this);
                }

                captureSession.addOutputIfPossible (captureOutput);
            }

            void takePicture (AVCaptureVideoOrientation orientationToUse)
            {
                if (takingPicture)
                {
                    // Picture taking already in progress!
                    jassertfalse;
                    return;
                }

                takingPicture = true;

                printImageOutputDebugInfo (captureOutput);

                if (auto* connection = findVideoConnection (captureOutput))
                {
                   #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
                    if (Pimpl::getIOSVersion().major >= 10 && [captureOutput isKindOfClass: [AVCapturePhotoOutput class]])
                    {
                        auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;
                        auto outputConnection = [photoOutput connectionWithMediaType: AVMediaTypeVideo];
                        outputConnection.videoOrientation = orientationToUse;

                        [photoOutput capturePhotoWithSettings: [AVCapturePhotoSettings photoSettings]
                                                     delegate: id<AVCapturePhotoCaptureDelegate> (photoOutputDelegate.get())];

                        return;
                    }
                   #endif

                    auto* stillImageOutput = (AVCaptureStillImageOutput*) captureOutput;
                    auto outputConnection = [stillImageOutput connectionWithMediaType: AVMediaTypeVideo];
                    outputConnection.videoOrientation = orientationToUse;

                    [stillImageOutput captureStillImageAsynchronouslyFromConnection: connection completionHandler:
                         ^(CMSampleBufferRef imageSampleBuffer, NSError* error)
                         {
                             if (error != nil)
                             {
                                 JUCE_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToJuce (error.localizedDescription));
                                 jassertfalse;
                                 return;
                             }

                             NSData* imageData = [AVCaptureStillImageOutput jpegStillImageNSDataRepresentation: imageSampleBuffer];

                             auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                             callListeners (image);

                             MessageManager::callAsync ([this, image]() { notifyPictureTaken (image); });
                         }];
                }
                else
                {
                    // Could not find a connection of video type
                    jassertfalse;
                }
            }

        private:
            static AVCaptureOutput* createCaptureOutput()
            {
               #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
                if (Pimpl::getIOSVersion().major >= 10)
                    return [AVCapturePhotoOutput new];
               #endif

                return [AVCaptureStillImageOutput new];
            }

            static void printImageOutputDebugInfo (AVCaptureOutput* captureOutput)
            {
               #if defined (__IPHONE_10_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_10_0
                if (Pimpl::getIOSVersion().major >= 10 && [captureOutput isKindOfClass: [AVCapturePhotoOutput class]])
                {
                    auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;

                    String typesString;

                    for (AVVideoCodecType type in photoOutput.availablePhotoCodecTypes)
                        typesString << nsStringToJuce (type) << " ";

                    JUCE_CAMERA_LOG ("Available image codec types: " + typesString);

                    JUCE_CAMERA_LOG ("Still image stabilization supported: " + String ((int) photoOutput.stillImageStabilizationSupported));
                    JUCE_CAMERA_LOG ("Dual camera fusion supported: " + String ((int) photoOutput.dualCameraFusionSupported));
                    JUCE_CAMERA_LOG ("Supports flash: "      + String ((int) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeOn)]));
                    JUCE_CAMERA_LOG ("Supports auto flash: " + String ((int) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeAuto)]));
                    JUCE_CAMERA_LOG ("Max bracketed photo count: " + String (photoOutput.maxBracketedCapturePhotoCount));
                    JUCE_CAMERA_LOG ("Lens stabilization during bracketed capture supported: " + String ((int) photoOutput.lensStabilizationDuringBracketedCaptureSupported));
                    JUCE_CAMERA_LOG ("Live photo capture supported: " + String ((int) photoOutput.livePhotoCaptureSupported));


                   #if defined (__IPHONE_11_0) && __IPHONE_OS_VERSION_MAX_ALLOWED >= __IPHONE_11_0
                    if (Pimpl::getIOSVersion().major >= 11)
                    {
                        typesString.clear();

                        for (AVFileType type in photoOutput.availablePhotoFileTypes)
                            typesString << nsStringToJuce (type) << " ";

                        JUCE_CAMERA_LOG ("Available photo file types: " + typesString);

                        typesString.clear();

                        for (AVFileType type in photoOutput.availableRawPhotoFileTypes)
                            typesString << nsStringToJuce (type) << " ";

                        JUCE_CAMERA_LOG ("Available RAW photo file types: " + typesString);

                        typesString.clear();

                        for (AVFileType type in photoOutput.availableLivePhotoVideoCodecTypes)
                            typesString << nsStringToJuce (type) << " ";

                        JUCE_CAMERA_LOG ("Available live photo video codec types: " + typesString);

                        JUCE_CAMERA_LOG ("Dual camera dual photo delivery supported: " + String ((int) photoOutput.dualCameraDualPhotoDeliverySupported));
                        JUCE_CAMERA_LOG ("Camera calibration data delivery supported: " + String ((int) photoOutput.cameraCalibrationDataDeliverySupported));
                        JUCE_CAMERA_LOG ("Depth data delivery supported: " + String ((int) photoOutput.depthDataDeliverySupported));
                    }
                   #endif

                    return;
                }
               #endif

                auto* stillImageOutput = (AVCaptureStillImageOutput*) captureOutput;

                String typesString;

                for (AVVideoCodecType type in stillImageOutput.availableImageDataCodecTypes)
                    typesString << nsStringToJuce (type) << " ";

                JUCE_CAMERA_LOG ("Available image codec types: " + typesString);
                JUCE_CAMERA_LOG ("Still image stabilization supported: " + String ((int) stillImageOutput.stillImageStabilizationSupported));
                JUCE_CAMERA_LOG ("Automatically enableds still image stabilization when available: " + String ((int) stillImageOutput.automaticallyEnablesStillImageStabilizationWhenAvailable));

                JUCE_CAMERA_LOG ("Output settings for image output: " + nsStringToJuce ([stillImageOutput.outputSettings description]));
            }

            //==============================================================================
            static AVCaptureConnection* findVideoConnection (AVCaptureOutput* output)
            {
                for (AVCaptureConnection* connection in output.connections)
                    for (AVCaptureInputPort* port in connection.inputPorts)
                        if ([port.mediaType isEqual: AVMediaTypeVideo])
                            return connection;

                return nullptr;
            }

            //==============================================================================
            class PhotoOutputDelegateClass : public ObjCClass<NSObject>
            {
            public:
                PhotoOutputDelegateClass() : ObjCClass<NSObject> ("PhotoOutputDelegateClass_")
                {
                    addMethod (@selector (captureOutput:willBeginCaptureForResolvedSettings:),       willBeginCaptureForSettings, "v@:@@");
                    addMethod (@selector (captureOutput:willCapturePhotoForResolvedSettings:),       willCaptureForSettings,      "v@:@@");
                    addMethod (@selector (captureOutput:didCapturePhotoForResolvedSettings:),        didCaptureForSettings,       "v@:@@");
                    addMethod (@selector (captureOutput:didFinishCaptureForResolvedSettings:error:), didFinishCaptureForSettings, "v@:@@@");

                    if (Pimpl::getIOSVersion().major >= 11)
                        addMethod (@selector (captureOutput:didFinishProcessingPhoto:error:), didFinishProcessingPhoto, "v@:@@@");
                    else
                        addMethod (@selector (captureOutput:didFinishProcessingPhotoSampleBuffer:previewPhotoSampleBuffer:resolvedSettings:bracketSettings:error:), didFinishProcessingPhotoSampleBuffer, "v@:@@@@@@");

                    addIvar<StillPictureTaker*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static StillPictureTaker& getOwner (id self) { return *getIvar<StillPictureTaker*> (self, "owner"); }
                static void setOwner (id self, StillPictureTaker* t) { object_setInstanceVariable (self, "owner", t); }

            private:
                static void willBeginCaptureForSettings (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                {
                    JUCE_CAMERA_LOG ("willBeginCaptureForSettings()");
                }

                static void willCaptureForSettings (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                {
                    JUCE_CAMERA_LOG ("willCaptureForSettings()");
                }

                static void didCaptureForSettings (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                {
                    JUCE_CAMERA_LOG ("didCaptureForSettings()");
                }

                static void didFinishCaptureForSettings (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*, NSError* error)
                {
                    String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                    ignoreUnused (errorString);

                    JUCE_CAMERA_LOG ("didFinishCaptureForSettings(), error = " + errorString);
                }

                static void didFinishProcessingPhoto (id self, SEL, AVCapturePhotoOutput*, AVCapturePhoto* capturePhoto, NSError* error)
                {
                    String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                    ignoreUnused (errorString);

                    JUCE_CAMERA_LOG ("didFinishProcessingPhoto(), error = " + errorString);

                    if (error != nil)
                    {
                        JUCE_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToJuce (error.localizedDescription));
                        jassertfalse;
                        return;
                    }

                    auto* imageOrientation = (NSNumber *) capturePhoto.metadata[(NSString*) kCGImagePropertyOrientation];

                    auto* uiImage = getImageWithCorrectOrientation ((CGImagePropertyOrientation) imageOrientation.unsignedIntValue,
                                                                    [capturePhoto CGImageRepresentation]);

                    auto* imageData = UIImageJPEGRepresentation (uiImage, 0.f);

                    auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                    getOwner (self).callListeners (image);

                    MessageManager::callAsync ([self, image]() { getOwner (self).notifyPictureTaken (image); });
                }

                static UIImage* getImageWithCorrectOrientation (CGImagePropertyOrientation imageOrientation,
                                                                CGImageRef imageData)
                {
                    auto origWidth  = CGImageGetWidth (imageData);
                    auto origHeight = CGImageGetHeight (imageData);

                    auto targetSize = getTargetImageDimensionFor (imageOrientation, imageData);

                    UIGraphicsBeginImageContext (targetSize);
                    CGContextRef context = UIGraphicsGetCurrentContext();

                    switch (imageOrientation)
                    {
                        case kCGImagePropertyOrientationUp:
                            CGContextScaleCTM (context, 1.0, -1.0);
                            CGContextTranslateCTM (context, 0.0, -targetSize.height);
                            break;
                        case kCGImagePropertyOrientationRight:
                            CGContextRotateCTM (context, 90 * MathConstants<CGFloat>::pi / 180);
                            CGContextScaleCTM (context, targetSize.height / origHeight, -targetSize.width / origWidth);
                            break;
                        case kCGImagePropertyOrientationDown:
                            CGContextTranslateCTM (context, targetSize.width, 0.0);
                            CGContextScaleCTM (context, -1.0, 1.0);
                            break;
                        case kCGImagePropertyOrientationLeft:
                            CGContextRotateCTM (context, -90 * MathConstants<CGFloat>::pi / 180);
                            CGContextScaleCTM (context, targetSize.height / origHeight, -targetSize.width / origWidth);
                            CGContextTranslateCTM (context, -targetSize.width, -targetSize.height);
                            break;
                        default:
                            // Not implemented.
                            jassertfalse;
                            break;
                    }

                    CGContextDrawImage (context, CGRectMake (0, 0, targetSize.width, targetSize.height), imageData);

                    UIImage* correctedImage = UIGraphicsGetImageFromCurrentImageContext();
                    UIGraphicsEndImageContext();

                    return correctedImage;
                }

                static CGSize getTargetImageDimensionFor (CGImagePropertyOrientation imageOrientation,
                                                          CGImageRef imageData)
                {
                    auto width = CGImageGetWidth (imageData);
                    auto height = CGImageGetHeight (imageData);

                    switch (imageOrientation)
                    {
                        case kCGImagePropertyOrientationUp:
                        case kCGImagePropertyOrientationUpMirrored:
                        case kCGImagePropertyOrientationDown:
                        case kCGImagePropertyOrientationDownMirrored:
                            return CGSizeMake ((CGFloat) width, (CGFloat) height);
                        case kCGImagePropertyOrientationRight:
                        case kCGImagePropertyOrientationRightMirrored:
                        case kCGImagePropertyOrientationLeft:
                        case kCGImagePropertyOrientationLeftMirrored:
                            return CGSizeMake ((CGFloat) height, (CGFloat) width);
                    }

                    jassertfalse;
                    return CGSizeMake ((CGFloat) width, (CGFloat) height);
                }

                static void didFinishProcessingPhotoSampleBuffer (id self, SEL, AVCapturePhotoOutput*,
                                                                  CMSampleBufferRef imageBuffer, CMSampleBufferRef imagePreviewBuffer,
                                                                  AVCaptureResolvedPhotoSettings*, AVCaptureBracketedStillImageSettings*,
                                                                  NSError* error)
                {
                    String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                    ignoreUnused (errorString);

                    JUCE_CAMERA_LOG ("didFinishProcessingPhotoSampleBuffer(), error = " + errorString);

                    if (error != nil)
                    {
                        JUCE_CAMERA_LOG ("Still picture capture failed, error: " + nsStringToJuce (error.localizedDescription));
                        jassertfalse;
                        return;
                    }

                    NSData* origImageData = [AVCapturePhotoOutput JPEGPhotoDataRepresentationForJPEGSampleBuffer: imageBuffer previewPhotoSampleBuffer: imagePreviewBuffer];
                    auto origImage = [UIImage imageWithData: origImageData];
                    auto imageOrientation = uiImageOrientationToCGImageOrientation (origImage.imageOrientation);

                    auto* uiImage = getImageWithCorrectOrientation (imageOrientation, origImage.CGImage);

                    auto* imageData = UIImageJPEGRepresentation (uiImage, 0.f);

                    auto image = ImageFileFormat::loadFrom (imageData.bytes, (size_t) imageData.length);

                    getOwner (self).callListeners (image);

                    MessageManager::callAsync ([self, image]() { getOwner (self).notifyPictureTaken (image); });
                }

                static CGImagePropertyOrientation uiImageOrientationToCGImageOrientation (UIImageOrientation orientation)
                {
                    switch (orientation)
                    {
                        case UIImageOrientationUp:            return kCGImagePropertyOrientationUp;
                        case UIImageOrientationDown:          return kCGImagePropertyOrientationDown;
                        case UIImageOrientationLeft:          return kCGImagePropertyOrientationLeft;
                        case UIImageOrientationRight:         return kCGImagePropertyOrientationRight;
                        case UIImageOrientationUpMirrored:    return kCGImagePropertyOrientationUpMirrored;
                        case UIImageOrientationDownMirrored:  return kCGImagePropertyOrientationDownMirrored;
                        case UIImageOrientationLeftMirrored:  return kCGImagePropertyOrientationLeftMirrored;
                        case UIImageOrientationRightMirrored: return kCGImagePropertyOrientationRightMirrored;
                    }
                }
            };

            //==============================================================================
            void callListeners (const Image& image)
            {
                captureSession.callListeners (image);
            }

            void notifyPictureTaken (const Image& image)
            {
                takingPicture = false;

                captureSession.notifyPictureTaken (image);
            }

            CaptureSession& captureSession;
            AVCaptureOutput* captureOutput;

            std::unique_ptr<NSObject, NSObjectDeleter> photoOutputDelegate;

            bool takingPicture = false;
        };

        //==============================================================================
        // NB: FileOutputRecordingDelegateClass callbacks can be called from any thread (incl.
        // the message thread), so waiting for an event when stopping recording is not an
        // option and VideoRecorder must be alive at all times in order to get stopped
        // recording callback.
        class VideoRecorder
        {
        public:
            VideoRecorder (CaptureSession& captureSession)
                : movieFileOutput ([AVCaptureMovieFileOutput new]),
                  delegate (nullptr)
            {
                static FileOutputRecordingDelegateClass cls;
                delegate.reset ([cls.createInstance() init]);
                FileOutputRecordingDelegateClass::setOwner (delegate.get(), this);

                captureSession.addOutputIfPossible (movieFileOutput);
            }

            ~VideoRecorder()
            {
                stopRecording();

                // Shutting down a device while recording will stop the recording
                // abruptly and the recording will be lost.
                jassert (! recordingInProgress);
            }

            void startRecording (const File& file, AVCaptureVideoOrientation orientationToUse)
            {
                if (Pimpl::getIOSVersion().major >= 10)
                    printVideoOutputDebugInfo (movieFileOutput);

                auto url = [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())
                                      isDirectory: NO];

                auto outputConnection = [movieFileOutput connectionWithMediaType: AVMediaTypeVideo];
                outputConnection.videoOrientation = orientationToUse;

                [movieFileOutput startRecordingToOutputFileURL: url recordingDelegate: delegate.get()];
            }

            void stopRecording()
            {
                [movieFileOutput stopRecording];
            }

            Time getTimeOfFirstRecordedFrame() const
            {
                return Time (firstRecordedFrameTimeMs.get());
            }

        private:
            static void printVideoOutputDebugInfo (AVCaptureMovieFileOutput* output)
            {
                ignoreUnused (output);

                JUCE_CAMERA_LOG ("Available video codec types:");

               #if JUCE_CAMERA_LOG_ENABLED
                for (AVVideoCodecType type in output.availableVideoCodecTypes)
                    JUCE_CAMERA_LOG (nsStringToJuce (type));
               #endif

                JUCE_CAMERA_LOG ("Output settings per video connection:");

               #if JUCE_CAMERA_LOG_ENABLED
                for (AVCaptureConnection* connection in output.connections)
                    JUCE_CAMERA_LOG (nsStringToJuce ([[output outputSettingsForConnection: connection] description]));
               #endif
            }

            //==============================================================================
            struct FileOutputRecordingDelegateClass    : public ObjCClass<NSObject<AVCaptureFileOutputRecordingDelegate>>
            {
                FileOutputRecordingDelegateClass()  : ObjCClass<NSObject<AVCaptureFileOutputRecordingDelegate>> ("FileOutputRecordingDelegateClass_")
                {
                    addMethod (@selector (captureOutput:didStartRecordingToOutputFileAtURL:fromConnections:),        started, "v@:@@@");
                    addMethod (@selector (captureOutput:didFinishRecordingToOutputFileAtURL:fromConnections:error:), stopped, "v@:@@@@");

                    addIvar<VideoRecorder*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static VideoRecorder& getOwner (id self)         { return *getIvar<VideoRecorder*> (self, "owner"); }
                static void setOwner (id self, VideoRecorder* r) { object_setInstanceVariable (self, "owner", r); }

            private:
                static void started (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*)
                {
                    JUCE_CAMERA_LOG ("Started recording");

                    getOwner (self).firstRecordedFrameTimeMs.set (Time::getCurrentTime().toMilliseconds());
                    getOwner (self).recordingInProgress = true;
                }

                static void stopped (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*, NSError* error)
                {
                    String errorString;
                    bool recordingPlayable = true;

                    // There might have been an error in the recording, yet there may be a playable file...
                    if ([error code] != noErr)
                    {
                        id value = [[error userInfo] objectForKey: AVErrorRecordingSuccessfullyFinishedKey];

                        if (value != nil && ! [value boolValue])
                            recordingPlayable = false;

                        errorString = nsStringToJuce (error.localizedDescription) + ", playable: " + String ((int) recordingPlayable);
                    }

                    JUCE_CAMERA_LOG ("Stopped recording, error = " + errorString);

                    getOwner (self).recordingInProgress = false;
                }
            };

            AVCaptureMovieFileOutput* movieFileOutput;
            std::unique_ptr<NSObject<AVCaptureFileOutputRecordingDelegate>, NSObjectDeleter> delegate;
            bool recordingInProgress = false;
            Atomic<int64> firstRecordedFrameTimeMs { 0 };
        };

        //==============================================================================
        void addOutputIfPossible (AVCaptureOutput* output)
        {
            dispatch_async (captureSessionQueue,^
                            {
                                if ([captureSession.get() canAddOutput: output])
                                {
                                    [captureSession.get() beginConfiguration];
                                    [captureSession.get() addOutput: output];
                                    [captureSession.get() commitConfiguration];

                                    return;
                                }

                                // Can't add output to camera session!
                                jassertfalse;
                            });
        }

        //==============================================================================
        void cameraSessionStarted()
        {
            sessionStarted = true;

            owner.cameraSessionStarted();
        }

        void cameraSessionRuntimeError (const String& error)
        {
            owner.cameraSessionRuntimeError (error);
        }

        void callListeners (const Image& image)
        {
            owner.callListeners (image);
        }

        void notifyPictureTaken (const Image& image)
        {
            owner.notifyPictureTaken (image);
        }

        Pimpl& owner;

        dispatch_queue_t captureSessionQueue;
        std::unique_ptr<AVCaptureSession, NSObjectDeleter> captureSession;
        std::unique_ptr<NSObject, NSObjectDeleter> delegate;

        StillPictureTaker stillPictureTaker;
        VideoRecorder videoRecorder;

        AVCaptureDevice* cameraDevice = nil;
        AVCaptureVideoPreviewLayer* previewLayer = nil;

        bool sessionStarted = false;

        WaitableEvent sessionClosedEvent;

        static int numCaptureSessions;
    };

    //==============================================================================
    void cameraSessionStarted()
    {
        JUCE_CAMERA_LOG ("cameraSessionStarted()");

        cameraOpenCallback (cameraId, {});
    }

    void cameraSessionRuntimeError (const String& error)
    {
        JUCE_CAMERA_LOG ("cameraSessionRuntimeError(), error = " + error);

        if (! notifiedOfCameraOpening)
        {
            cameraOpenCallback ({}, error);
        }
        else
        {
            if (owner.onErrorOccurred != nullptr)
                owner.onErrorOccurred (error);
        }
    }

    void callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });
    }

    void notifyPictureTaken (const Image& image)
    {
        JUCE_CAMERA_LOG ("notifyPictureTaken()");

        if (pictureTakenCallback != nullptr)
            pictureTakenCallback (image);
    }

    //==============================================================================
    void triggerStillPictureCapture()
    {
        captureSession.takeStillPicture();
    }

    //==============================================================================
    CameraDevice& owner;
    String cameraId;
    InternalOpenCameraResultCallback cameraOpenCallback;

    CriticalSection listenerLock;
    ListenerList<Listener> listeners;

    std::function<void (const Image&)> pictureTakenCallback;

    CaptureSession captureSession;

    bool notifiedOfCameraOpening = false;

    //==============================================================================
    struct IOSVersion
    {
        int major;
        int minor;
    };

    static IOSVersion getIOSVersion()
    {
        auto processInfo = [NSProcessInfo processInfo];

        if (! [processInfo respondsToSelector: @selector (operatingSystemVersion)])
            return {7, 0};   // Below 8.0 in fact, but only care that it's below 8

        return { (int)[processInfo operatingSystemVersion].majorVersion,
                 (int)[processInfo operatingSystemVersion].minorVersion };
    }

    static IOSVersion iosVersion;

    friend struct CameraDevice::ViewerComponent;

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

CameraDevice::Pimpl::IOSVersion CameraDevice::Pimpl::iosVersion = CameraDevice::Pimpl::getIOSVersion();
int CameraDevice::Pimpl::CaptureSession::numCaptureSessions = 0;

//==============================================================================
struct CameraDevice::ViewerComponent  : public UIViewComponent
{
    //==============================================================================
    struct JuceCameraDeviceViewerClass    : public ObjCClass<UIView>
    {
        JuceCameraDeviceViewerClass()  : ObjCClass<UIView> ("JuceCameraDeviceViewerClass_")
        {
            addMethod (@selector (layoutSubviews), layoutSubviews, "v@:");

            registerClass();
        }

    private:
        static void layoutSubviews (id self, SEL)
        {
            sendSuperclassMessage (self, @selector (layoutSubviews));

            UIView* asUIView = (UIView*) self;

            updateOrientation (self);

            if (auto* previewLayer = getPreviewLayer (self))
                previewLayer.frame = asUIView.bounds;
        }

        static AVCaptureVideoPreviewLayer* getPreviewLayer (id self)
        {
            UIView* asUIView = (UIView*) self;

            if (asUIView.layer.sublayers != nil && [asUIView.layer.sublayers count] > 0)
                if ([asUIView.layer.sublayers[0] isKindOfClass: [AVCaptureVideoPreviewLayer class]])
                     return (AVCaptureVideoPreviewLayer*) asUIView.layer.sublayers[0];

            return nil;
        }

        static void updateOrientation (id self)
        {
            if (auto* previewLayer = getPreviewLayer (self))
            {
                UIDeviceOrientation o = [UIDevice currentDevice].orientation;

                if (UIDeviceOrientationIsPortrait (o) || UIDeviceOrientationIsLandscape (o))
                {
                    if (previewLayer.connection != nil)
                        previewLayer.connection.videoOrientation = (AVCaptureVideoOrientation) o;
                }
            }
        }
    };

    ViewerComponent (CameraDevice& device)
    {
        static JuceCameraDeviceViewerClass cls;

        // Initial size that can be overriden later.
        setSize (640, 480);

        auto view = [cls.createInstance() init];
        setView (view);

        auto* previewLayer = device.pimpl->captureSession.createPreviewLayer();
        previewLayer.frame = view.bounds;

        UIInterfaceOrientation statusBarOrientation = [UIApplication sharedApplication].statusBarOrientation;
        AVCaptureVideoOrientation videoOrientation = statusBarOrientation != UIInterfaceOrientationUnknown
                                                   ? (AVCaptureVideoOrientation) statusBarOrientation
                                                   : AVCaptureVideoOrientationPortrait;

        previewLayer.connection.videoOrientation = videoOrientation;

        [view.layer addSublayer: previewLayer];
    }
};

//==============================================================================
String CameraDevice::getFileExtension()
{
    return ".mov";
}
