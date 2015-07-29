/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#if ! JUCE_QUICKTIME
 #error "To support cameras in OSX you'll need to enable the JUCE_QUICKTIME flag"
#endif

extern Image juce_createImageFromCIImage (CIImage*, int w, int h);

struct CameraDevice::Pimpl
{
    Pimpl (const String&, const int index, int /*minWidth*/, int /*minHeight*/, int /*maxWidth*/, int /*maxHeight*/)
        : input (nil),
          audioDevice (nil),
          audioInput (nil),
          session (nil),
          fileOutput (nil),
          imageOutput (nil),
          firstPresentationTime (0),
          averageTimeOffset (0),
          isRecording (false)
    {
        JUCE_AUTORELEASEPOOL
        {
            session = [[QTCaptureSession alloc] init];

            NSArray* devs = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];
            device = (QTCaptureDevice*) [devs objectAtIndex: index];

            static DelegateClass cls;
            callbackDelegate = [cls.createInstance() init];
            DelegateClass::setOwner (callbackDelegate, this);

            NSError* err = nil;
            [device retain];
            [device open: &err];

            if (err == nil)
            {
                input      = [[QTCaptureDeviceInput alloc] initWithDevice: device];
                audioInput = [[QTCaptureDeviceInput alloc] initWithDevice: device];

                [session addInput: input error: &err];

                if (err == nil)
                {
                    resetFile();

                    imageOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
                    [imageOutput setDelegate: callbackDelegate];

                    if (err == nil)
                    {
                        [session startRunning];
                        return;
                    }
                }
            }

            openingError = nsStringToJuce ([err description]);
            DBG (openingError);
        }
    }

    ~Pimpl()
    {
        [session stopRunning];
        [session removeOutput: imageOutput];

        [session release];
        [input release];
        [device release];
        [audioDevice release];
        [audioInput release];
        [fileOutput release];
        [imageOutput release];
        [callbackDelegate release];
    }

    bool openedOk() const noexcept       { return openingError.isEmpty(); }

    void resetFile()
    {
        [fileOutput recordToOutputFileURL: nil];
        [session removeOutput: fileOutput];
        [fileOutput release];
        fileOutput = [[QTCaptureMovieFileOutput alloc] init];

        [session removeInput: audioInput];
        [audioInput release];
        audioInput = nil;
        [audioDevice release];
        audioDevice = nil;

        [fileOutput setDelegate: callbackDelegate];
    }

    void addDefaultAudioInput()
    {
        NSError* err = nil;
        audioDevice = [QTCaptureDevice defaultInputDeviceWithMediaType: QTMediaTypeSound];

        if ([audioDevice open: &err])
            [audioDevice retain];
        else
            audioDevice = nil;

        if (audioDevice != nil)
        {
            audioInput = [[QTCaptureDeviceInput alloc] initWithDevice: audioDevice];
            [session addInput: audioInput error: &err];
        }
    }

    void startRecordingToFile (const File& file, int quality)
    {
        stopRecording();

        firstPresentationTime = 0;
        file.deleteFile();

        // In some versions of QT (e.g. on 10.5), if you record video without audio, the speed comes
        // out wrong, so we'll put some audio in there too..,
        addDefaultAudioInput();

        [session addOutput: fileOutput error: nil];

        NSEnumerator* connectionEnumerator = [[fileOutput connections] objectEnumerator];

        for (;;)
        {
            QTCaptureConnection* connection = [connectionEnumerator nextObject];
            if (connection == nil)
                break;

            QTCompressionOptions* options = nil;
            NSString* mediaType = [connection mediaType];

            if ([mediaType isEqualToString: QTMediaTypeVideo])
                options = [QTCompressionOptions compressionOptionsWithIdentifier:
                                quality >= 1 ? nsStringLiteral ("QTCompressionOptionsSD480SizeH264Video")
                                             : nsStringLiteral ("QTCompressionOptions240SizeH264Video")];
            else if ([mediaType isEqualToString: QTMediaTypeSound])
                options = [QTCompressionOptions compressionOptionsWithIdentifier: nsStringLiteral ("QTCompressionOptionsHighQualityAACAudio")];

            [fileOutput setCompressionOptions: options forConnection: connection];
        }

        [fileOutput recordToOutputFileURL: [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())]];
        isRecording = true;
    }

    void stopRecording()
    {
        if (isRecording)
        {
            resetFile();
            isRecording = false;
        }
    }

    Time getTimeOfFirstRecordedFrame() const
    {
        return firstPresentationTime != 0 ? Time (firstPresentationTime + averageTimeOffset)
                                          : Time();
    }

    void addListener (CameraDevice::Listener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            [session addOutput: imageOutput error: nil];

        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraDevice::Listener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeFirstMatchingValue (listenerToRemove);

        if (listeners.size() == 0)
            [session removeOutput: imageOutput];
    }

    void callListeners (CIImage* frame, int w, int h)
    {
        Image image (juce_createImageFromCIImage (frame, w, h));

        const ScopedLock sl (listenerLock);

        for (int i = listeners.size(); --i >= 0;)
        {
            CameraDevice::Listener* const l = listeners[i];

            if (l != nullptr)
                l->imageReceived (image);
        }
    }

    void captureBuffer (QTSampleBuffer* sampleBuffer)
    {
        const Time now (Time::getCurrentTime());

       #if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_5
        NSNumber* hosttime = (NSNumber*) [sampleBuffer attributeForKey: QTSampleBufferHostTimeAttribute];
       #else
        NSNumber* hosttime = (NSNumber*) [sampleBuffer attributeForKey: nsStringLiteral ("hostTime")];
       #endif

        int64 presentationTime = (hosttime != nil)
                ? ((int64) AudioConvertHostTimeToNanos ([hosttime unsignedLongLongValue]) / 1000000 + 40)
                : (([sampleBuffer presentationTime].timeValue * 1000) / [sampleBuffer presentationTime].timeScale + 50);

        const int64 timeDiff = now.toMilliseconds() - presentationTime;

        if (firstPresentationTime == 0)
        {
            firstPresentationTime = presentationTime;
            averageTimeOffset = timeDiff;
        }
        else
        {
            averageTimeOffset = (averageTimeOffset * 120 + timeDiff * 8) / 128;
        }
    }

    static StringArray getAvailableDevices()
    {
        StringArray results;
        NSArray* devs = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];

        for (int i = 0; i < (int) [devs count]; ++i)
        {
            QTCaptureDevice* dev = (QTCaptureDevice*) [devs objectAtIndex: i];
            results.add (nsStringToJuce ([dev localizedDisplayName]));
        }

        return results;
    }

    QTCaptureDevice* device;
    QTCaptureDevice* audioDevice;
    QTCaptureDeviceInput* input;
    QTCaptureDeviceInput* audioInput;
    QTCaptureSession* session;
    QTCaptureMovieFileOutput* fileOutput;
    QTCaptureDecompressedVideoOutput* imageOutput;
    NSObject* callbackDelegate;
    String openingError;
    int64 firstPresentationTime, averageTimeOffset;
    bool isRecording;

    Array<CameraDevice::Listener*> listeners;
    CriticalSection listenerLock;

private:
    //==============================================================================
    struct DelegateClass  : public ObjCClass<NSObject>
    {
        DelegateClass()  : ObjCClass<NSObject> ("JUCEAppDelegate_")
        {
            addIvar<Pimpl*> ("owner");

            addMethod (@selector (captureOutput:didOutputVideoFrame:withSampleBuffer:fromConnection:),
                       didOutputVideoFrame, "v@:@", @encode (CVImageBufferRef), "@@");
            addMethod (@selector (captureOutput:didOutputSampleBuffer:fromConnection:),
                       didOutputVideoFrame, "v@:@@@");

            registerClass();
        }

        static void setOwner (id self, Pimpl* owner)   { object_setInstanceVariable (self, "owner", owner); }
        static Pimpl* getOwner (id self)               { return getIvar<Pimpl*> (self, "owner"); }

    private:
        static void didOutputVideoFrame (id self, SEL, QTCaptureOutput*, CVImageBufferRef videoFrame,
                                         QTSampleBuffer*, QTCaptureConnection*)
        {
            Pimpl* const internal = getOwner (self);

            if (internal->listeners.size() > 0)
            {
                JUCE_AUTORELEASEPOOL
                {
                    internal->callListeners ([CIImage imageWithCVImageBuffer: videoFrame],
                                             (int) CVPixelBufferGetWidth (videoFrame),
                                             (int) CVPixelBufferGetHeight (videoFrame));
                }
            }
        }

        static void didOutputSampleBuffer (id self, SEL, QTCaptureFileOutput*, QTSampleBuffer* sampleBuffer, QTCaptureConnection*)
        {
            getOwner (self)->captureBuffer (sampleBuffer);
        }
    };

    JUCE_DECLARE_NON_COPYABLE (Pimpl)
};

struct CameraDevice::ViewerComponent  : public NSViewComponent
{
    ViewerComponent (CameraDevice& d)
    {
        JUCE_AUTORELEASEPOOL
        {
            captureView = [[QTCaptureView alloc] init];
            [captureView setCaptureSession: d.pimpl->session];

            setSize (640, 480);
            setView (captureView);
        }
    }

    ~ViewerComponent()
    {
        setView (nil);
        [captureView setCaptureSession: nil];
        [captureView release];
    }

    QTCaptureView* captureView;

    JUCE_DECLARE_NON_COPYABLE (ViewerComponent)
};

String CameraDevice::getFileExtension()
{
    return ".mov";
}
