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
                                 completionHandler: ^([[maybe_unused]] BOOL granted)
         {
             // Access to video is required for camera to work,
             // black images will be produced otherwise!
             jassert (granted);
         }];

        [AVCaptureDevice requestAccessForMediaType: AVMediaTypeAudio
                                 completionHandler: ^([[maybe_unused]] BOOL granted)
         {
             // Access to audio is required for camera to work,
             // silence will be produced otherwise!
             jassert (granted);
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
        std::unique_ptr<NSMutableArray<AVCaptureDeviceType>, NSObjectDeleter> deviceTypes ([[NSMutableArray alloc] initWithCapacity: 2]);

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInWideAngleCamera];
        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTelephotoCamera];

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInDualCamera];

        [deviceTypes.get() addObject: AVCaptureDeviceTypeBuiltInTrueDepthCamera];

        auto discoverySession = [AVCaptureDeviceDiscoverySession discoverySessionWithDeviceTypes: deviceTypes.get()
                                                                                       mediaType: AVMediaTypeVideo
                                                                                        position: AVCaptureDevicePositionUnspecified];

        return [discoverySession devices];
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

        JUCE_CAMERA_LOG ("Device type: " + nsStringToJuce (device.deviceType));
        JUCE_CAMERA_LOG ("Locking focus with custom lens position supported: " + String ((int)device.lockingFocusWithCustomLensPositionSupported));

        JUCE_CAMERA_LOG ("Min available video zoom factor: " + String (device.minAvailableVideoZoomFactor));
        JUCE_CAMERA_LOG ("Max available video zoom factor: " + String (device.maxAvailableVideoZoomFactor));
        JUCE_CAMERA_LOG ("Dual camera switch over video zoom factor: " + String (device.dualCameraSwitchOverVideoZoomFactor));

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

        JUCE_CAMERA_LOG ("Video binned: " + String (int (format.videoBinned)));

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
            case AVCaptureAutoFocusSystemNone:
            default:                                        autoFocusSystemString = "None";
        }
        JUCE_CAMERA_LOG ("Auto focus system: " + autoFocusSystemString);

        JUCE_CAMERA_LOG ("Standard video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeStandard]));
        JUCE_CAMERA_LOG ("Cinematic video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeCinematic]));
        JUCE_CAMERA_LOG ("Auto video stabilization supported: " + String ((int) [format isVideoStabilizationModeSupported: AVCaptureVideoStabilizationModeAuto]));

        JUCE_CAMERA_LOG ("Min zoom factor for depth data delivery: " + String (format.videoMinZoomFactorForDepthDataDelivery));
        JUCE_CAMERA_LOG ("Max zoom factor for depth data delivery: " + String (format.videoMaxZoomFactorForDepthDataDelivery));
    }

    static String getHighResStillImgDimensionsString (CMVideoDimensions d)
    {
        return "[" + String (d.width) + " " + String (d.height) + "]";
    }

    static String cmTimeToString (CMTime time)
    {
        CFUniquePtr<CFStringRef> timeDesc (CMTimeCopyDescription (nullptr, time));
        return String::fromCFString (timeDesc.get());
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

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStartRunning:)
                                                         name: AVCaptureSessionDidStartRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (sessionDidStopRunning:)
                                                         name: AVCaptureSessionDidStopRunningNotification
                                                       object: captureSession.get()];

            [[NSNotificationCenter defaultCenter] addObserver: delegate.get()
                                                     selector: @selector (runtimeError:)
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
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE

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
                                    MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }, error]() mutable
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
                                    MessageManager::callAsync ([weakRef = WeakReference<CaptureSession> { this }, error]() mutable
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

            JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wnullable-to-nonnull-conversion")
            if (! [captureSession.get() canAddInput: input])
                return "Could not add input to camera session.";

            [captureSession.get() addInput: input];
            JUCE_END_IGNORE_WARNINGS_GCC_LIKE
            return {};
        }

        //==============================================================================
        struct SessionDelegateClass    : public ObjCClass<NSObject>
        {
            SessionDelegateClass()  : ObjCClass<NSObject> ("SessionDelegateClass_")
            {
                JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wundeclared-selector")
                addMethod (@selector (sessionDidStartRunning:),
                           [] (id self, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                               dispatch_async (dispatch_get_main_queue(),
                                               ^{
                                                   getOwner (self).cameraSessionStarted();
                                               });
                           });

                addMethod (@selector (sessionDidStopRunning:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));
                           });

                addMethod (@selector (runtimeError:),
                           [] (id self, SEL, NSNotification* notification)
                           {
                               JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));

                               dispatch_async (dispatch_get_main_queue(),
                                               ^{
                                                   NSError* error = notification.userInfo[AVCaptureSessionErrorKey];
                                                   auto errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();
                                                   getOwner (self).cameraSessionRuntimeError (errorString);
                                               });
                           });

                addMethod (@selector (sessionWasInterrupted:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));
                           });

                addMethod (@selector (sessionInterruptionEnded:),
                           [] (id, SEL, [[maybe_unused]] NSNotification* notification)
                           {
                               JUCE_CAMERA_LOG (nsStringToJuce ([notification description]));
                           });

                JUCE_END_IGNORE_WARNINGS_GCC_LIKE

                addIvar<CaptureSession*> ("owner");

                registerClass();
            }

            //==============================================================================
            static CaptureSession& getOwner (id self)         { return *getIvar<CaptureSession*> (self, "owner"); }
            static void setOwner (id self, CaptureSession* s) { object_setInstanceVariable (self, "owner", s); }
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
                static PhotoOutputDelegateClass cls;
                photoOutputDelegate.reset ([cls.createInstance() init]);
                PhotoOutputDelegateClass::setOwner (photoOutputDelegate.get(), this);

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
                    auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;
                    auto outputConnection = [photoOutput connectionWithMediaType: AVMediaTypeVideo];
                    outputConnection.videoOrientation = orientationToUse;

                    [photoOutput capturePhotoWithSettings: [AVCapturePhotoSettings photoSettings]
                                                 delegate: id<AVCapturePhotoCaptureDelegate> (photoOutputDelegate.get())];
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
                return [AVCapturePhotoOutput new];
            }

            static void printImageOutputDebugInfo (AVCaptureOutput* captureOutput)
            {
                auto* photoOutput = (AVCapturePhotoOutput*) captureOutput;

                String typesString;

                for (id type in photoOutput.availablePhotoCodecTypes)
                    typesString << nsStringToJuce (type) << " ";

                JUCE_CAMERA_LOG ("Available image codec types: " + typesString);

                JUCE_CAMERA_LOG ("Still image stabilization supported: " + String ((int) photoOutput.stillImageStabilizationSupported));
                JUCE_CAMERA_LOG ("Dual camera fusion supported: " + String ((int) photoOutput.dualCameraFusionSupported));
                JUCE_CAMERA_LOG ("Supports flash: "      + String ((int) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeOn)]));
                JUCE_CAMERA_LOG ("Supports auto flash: " + String ((int) [photoOutput.supportedFlashModes containsObject: @(AVCaptureFlashModeAuto)]));
                JUCE_CAMERA_LOG ("Max bracketed photo count: " + String (photoOutput.maxBracketedCapturePhotoCount));
                JUCE_CAMERA_LOG ("Lens stabilization during bracketed capture supported: " + String ((int) photoOutput.lensStabilizationDuringBracketedCaptureSupported));
                JUCE_CAMERA_LOG ("Live photo capture supported: " + String ((int) photoOutput.livePhotoCaptureSupported));

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
                    addMethod (@selector (captureOutput:willBeginCaptureForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   JUCE_CAMERA_LOG ("willBeginCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:willCapturePhotoForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   JUCE_CAMERA_LOG ("willCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:didCapturePhotoForResolvedSettings:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*)
                               {
                                   JUCE_CAMERA_LOG ("didCaptureForSettings()");
                               });

                    addMethod (@selector (captureOutput:didFinishCaptureForResolvedSettings:error:),
                               [] (id, SEL, AVCapturePhotoOutput*, AVCaptureResolvedPhotoSettings*, NSError* error)
                               {
                                   [[maybe_unused]] String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();

                                   JUCE_CAMERA_LOG ("didFinishCaptureForSettings(), error = " + errorString);
                               });

                    addMethod (@selector (captureOutput:didFinishProcessingPhoto:error:),
                               [] (id self, SEL, AVCapturePhotoOutput*, AVCapturePhoto* capturePhoto, NSError* error)
                               {
                                   getOwner (self).takingPicture = false;

                                   [[maybe_unused]] String errorString = error != nil ? nsStringToJuce (error.localizedDescription) : String();

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
                               });

                    addIvar<StillPictureTaker*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static StillPictureTaker& getOwner (id self) { return *getIvar<StillPictureTaker*> (self, "owner"); }
                static void setOwner (id self, StillPictureTaker* t) { object_setInstanceVariable (self, "owner", t); }

            private:
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
                            CGContextScaleCTM (context, targetSize.height / (CGFloat) origHeight, -targetSize.width / (CGFloat) origWidth);
                            break;
                        case kCGImagePropertyOrientationDown:
                            CGContextTranslateCTM (context, targetSize.width, 0.0);
                            CGContextScaleCTM (context, -1.0, 1.0);
                            break;
                        case kCGImagePropertyOrientationLeft:
                            CGContextRotateCTM (context, -90 * MathConstants<CGFloat>::pi / 180);
                            CGContextScaleCTM (context, targetSize.height / (CGFloat) origHeight, -targetSize.width / (CGFloat) origWidth);
                            CGContextTranslateCTM (context, -targetSize.width, -targetSize.height);
                            break;
                        case kCGImagePropertyOrientationUpMirrored:
                        case kCGImagePropertyOrientationDownMirrored:
                        case kCGImagePropertyOrientationLeftMirrored:
                        case kCGImagePropertyOrientationRightMirrored:
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
            VideoRecorder (CaptureSession& session)
                : movieFileOutput ([AVCaptureMovieFileOutput new]),
                  delegate (nullptr)
            {
                static FileOutputRecordingDelegateClass cls;
                delegate.reset ([cls.createInstance() init]);
                FileOutputRecordingDelegateClass::setOwner (delegate.get(), this);

                session.addOutputIfPossible (movieFileOutput);
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
            static void printVideoOutputDebugInfo ([[maybe_unused]] AVCaptureMovieFileOutput* output)
            {
                JUCE_CAMERA_LOG ("Available video codec types:");

               #if JUCE_CAMERA_LOG_ENABLED
                for (id type in output.availableVideoCodecTypes)
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
                    addMethod (@selector (captureOutput:didStartRecordingToOutputFileAtURL:fromConnections:),
                               [] (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*)
                               {
                                   JUCE_CAMERA_LOG ("Started recording");

                                   getOwner (self).firstRecordedFrameTimeMs.set (Time::getCurrentTime().toMilliseconds());
                                   getOwner (self).recordingInProgress = true;
                               });

                    addMethod (@selector (captureOutput:didFinishRecordingToOutputFileAtURL:fromConnections:error:),
                               [] (id self, SEL, AVCaptureFileOutput*, NSURL*, NSArray<AVCaptureConnection*>*, NSError* error)
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
                               });

                    addIvar<VideoRecorder*> ("owner");

                    registerClass();
                }

                //==============================================================================
                static VideoRecorder& getOwner (id self)         { return *getIvar<VideoRecorder*> (self, "owner"); }
                static void setOwner (id self, VideoRecorder* r) { object_setInstanceVariable (self, "owner", r); }
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
            cameraOpenCallback ({}, error);
        else
            NullCheckedInvocation::invoke (owner.onErrorOccurred, error);
    }

    void callListeners (const Image& image)
    {
        const ScopedLock sl (listenerLock);
        listeners.call ([=] (Listener& l) { l.imageReceived (image); });

        if (listeners.size() == 1)
            triggerStillPictureCapture();
    }

    void notifyPictureTaken (const Image& image)
    {
        JUCE_CAMERA_LOG ("notifyPictureTaken()");

        NullCheckedInvocation::invoke (pictureTakenCallback, image);
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

    friend struct CameraDevice::ViewerComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

int CameraDevice::Pimpl::CaptureSession::numCaptureSessions = 0;

//==============================================================================
struct CameraDevice::ViewerComponent  : public UIViewComponent
{
    //==============================================================================
    struct JuceCameraDeviceViewerClass    : public ObjCClass<UIView>
    {
        JuceCameraDeviceViewerClass()  : ObjCClass<UIView> ("JuceCameraDeviceViewerClass_")
        {
            addMethod (@selector (layoutSubviews),
                       [] (id self, SEL)
                       {
                           sendSuperclassMessage<void> (self, @selector (layoutSubviews));

                           UIView* asUIView = (UIView*) self;

                           updateOrientation (self);

                           if (auto* previewLayer = getPreviewLayer (self))
                               previewLayer.frame = asUIView.bounds;
                       });

            registerClass();
        }

    private:
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

        // Initial size that can be overridden later.
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
