/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

// (This file gets included by juce_mac_NativeCode.mm, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE && JUCE_QUICKTIME && JUCE_USE_CAMERA

//==============================================================================
#define QTCaptureCallbackDelegate MakeObjCClassName(QTCaptureCallbackDelegate)

class QTCameraDeviceInteral;

END_JUCE_NAMESPACE

@interface QTCaptureCallbackDelegate    : NSObject
{
@public
    CameraDevice* owner;
    QTCameraDeviceInteral* internal;
    Time* firstRecordedTime;
}

- (QTCaptureCallbackDelegate*) initWithOwner: (CameraDevice*) owner internalDev: (QTCameraDeviceInteral*) d;
- (void) dealloc;

- (void) captureOutput: (QTCaptureOutput*) captureOutput
         didOutputVideoFrame: (CVImageBufferRef) videoFrame
         withSampleBuffer: (QTSampleBuffer*) sampleBuffer
         fromConnection: (QTCaptureConnection*) connection;

- (void) captureOutput: (QTCaptureFileOutput*) captureOutput
         didOutputSampleBuffer: (QTSampleBuffer*) sampleBuffer
         fromConnection: (QTCaptureConnection*) connection;
@end

BEGIN_JUCE_NAMESPACE

//==============================================================================
class QTCameraDeviceInteral
{
public:
    QTCameraDeviceInteral (CameraDevice* owner, int index)
    {
        const ScopedAutoReleasePool pool;

        session = [[QTCaptureSession alloc] init];

        NSArray* devs = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];
        device = (QTCaptureDevice*) [devs objectAtIndex: index];
        input = 0;
        audioInput = 0;
        audioDevice = 0;
        fileOutput = 0;
        imageOutput = 0;
        callbackDelegate = [[QTCaptureCallbackDelegate alloc] initWithOwner: owner
                                                                internalDev: this];

        NSError* err = 0;
        [device retain];
        [device open: &err];

        if (err == 0)
        {
            input = [[QTCaptureDeviceInput alloc] initWithDevice: device];
            audioInput = [[QTCaptureDeviceInput alloc] initWithDevice: device];

            [session addInput: input error: &err];

            if (err == 0)
            {
                resetFile();

                imageOutput = [[QTCaptureDecompressedVideoOutput alloc] init];
                [imageOutput setDelegate: callbackDelegate];

                if (err == 0)
                {
                    [session startRunning];
                    return;
                }
            }
        }

        openingError = nsStringToJuce ([err description]);
        DBG (openingError);
    }

    ~QTCameraDeviceInteral()
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

    void resetFile()
    {
        [fileOutput recordToOutputFileURL: nil];
        [session removeOutput: fileOutput];
        [fileOutput release];
        fileOutput = [[QTCaptureMovieFileOutput alloc] init];

        [session removeInput: audioInput];
        [audioInput release];
        audioInput = 0;
        [audioDevice release];
        audioDevice = 0;

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

        if (audioDevice != 0)
        {
            audioInput = [[QTCaptureDeviceInput alloc] initWithDevice: audioDevice];
            [session addInput: audioInput error: &err];
        }
    }

    void addListener (CameraImageListener* listenerToAdd)
    {
        const ScopedLock sl (listenerLock);

        if (listeners.size() == 0)
            [session addOutput: imageOutput error: nil];

        listeners.addIfNotAlreadyThere (listenerToAdd);
    }

    void removeListener (CameraImageListener* listenerToRemove)
    {
        const ScopedLock sl (listenerLock);
        listeners.removeValue (listenerToRemove);

        if (listeners.size() == 0)
            [session removeOutput: imageOutput];
    }

    void callListeners (CIImage* frame, int w, int h)
    {
        CoreGraphicsImage image (Image::ARGB, w, h, false);
        CIContext* cic = [CIContext contextWithCGContext: image.context options: nil];
        [cic drawImage: frame inRect: CGRectMake (0, 0, w, h) fromRect: CGRectMake (0, 0, w, h)];
        CGContextFlush (image.context);

        const ScopedLock sl (listenerLock);

        for (int i = listeners.size(); --i >= 0;)
        {
            CameraImageListener* l = (CameraImageListener*) listeners[i];

            if (l != 0)
                l->imageReceived (image);
        }
    }

    QTCaptureDevice* device;
    QTCaptureDeviceInput* input;
    QTCaptureDevice* audioDevice;
    QTCaptureDeviceInput* audioInput;
    QTCaptureSession* session;
    QTCaptureMovieFileOutput* fileOutput;
    QTCaptureDecompressedVideoOutput* imageOutput;
    QTCaptureCallbackDelegate* callbackDelegate;
    String openingError;

    VoidArray listeners;
    CriticalSection listenerLock;
};

END_JUCE_NAMESPACE
@implementation QTCaptureCallbackDelegate

- (QTCaptureCallbackDelegate*) initWithOwner: (CameraDevice*) owner_
                                 internalDev: (QTCameraDeviceInteral*) d
{
    [super init];
    owner = owner_;
    internal = d;
    firstRecordedTime = 0;
    return self;
}

- (void) dealloc
{
    delete firstRecordedTime;
    [super dealloc];
}

- (void) captureOutput: (QTCaptureOutput*) captureOutput
         didOutputVideoFrame: (CVImageBufferRef) videoFrame
         withSampleBuffer: (QTSampleBuffer*) sampleBuffer
         fromConnection: (QTCaptureConnection*) connection
{
    if (internal->listeners.size() > 0)
    {
        const ScopedAutoReleasePool pool;

        internal->callListeners ([CIImage imageWithCVImageBuffer: videoFrame],
                                 CVPixelBufferGetWidth (videoFrame),
                                 CVPixelBufferGetHeight (videoFrame));
    }
}

- (void) captureOutput: (QTCaptureFileOutput*) captureOutput
         didOutputSampleBuffer: (QTSampleBuffer*) sampleBuffer
         fromConnection: (QTCaptureConnection*) connection
{
    if (firstRecordedTime == 0)
    {
        const Time now (Time::getCurrentTime());
        firstRecordedTime = new Time (now - RelativeTime (0.1));
    }
}

@end
BEGIN_JUCE_NAMESPACE

//==============================================================================
class QTCaptureViewerComp : public NSViewComponent
{
public:
    QTCaptureViewerComp (CameraDevice* const cameraDevice, QTCameraDeviceInteral* const internal)
    {
        const ScopedAutoReleasePool pool;
        captureView = [[QTCaptureView alloc] init];
        [captureView setCaptureSession: internal->session];

        setSize (640, 480); //  xxx need to somehow get the movie size - how?
        setView (captureView);
    }

    ~QTCaptureViewerComp()
    {
        setView (0);
        [captureView setCaptureSession: nil];
        [captureView release];
    }

    QTCaptureView* captureView;
};

//==============================================================================
CameraDevice::CameraDevice (const String& name_, int index)
    : name (name_)
{
    isRecording = false;
    QTCameraDeviceInteral* d = new QTCameraDeviceInteral (this, index);
    internal = d;
}

CameraDevice::~CameraDevice()
{
    stopRecording();
    delete (QTCameraDeviceInteral*) internal;
    internal = 0;
}

Component* CameraDevice::createViewerComponent()
{
    return new QTCaptureViewerComp (this, (QTCameraDeviceInteral*) internal);
}

const String CameraDevice::getFileExtension()
{
    return ".mov";
}

void CameraDevice::startRecordingToFile (const File& file)
{
    stopRecording();

    QTCameraDeviceInteral* const d = (QTCameraDeviceInteral*) internal;
    deleteAndZero (d->callbackDelegate->firstRecordedTime);
    file.deleteFile();

    // In some versions of QT (e.g. on 10.5), if you record video without audio, the speed comes
    // out wrong, so we'll put some audio in there too..,
    d->addDefaultAudioInput();

    [d->session addOutput: d->fileOutput error: nil];

    NSEnumerator* connectionEnumerator = [[d->fileOutput connections] objectEnumerator];

    for (;;)
    {
        QTCaptureConnection* connection = [connectionEnumerator nextObject];
        if (connection == 0)
            break;

        QTCompressionOptions* options = 0;
        NSString* mediaType = [connection mediaType];

        if ([mediaType isEqualToString: QTMediaTypeVideo])
            options = [QTCompressionOptions compressionOptionsWithIdentifier: @"QTCompressionOptionsSD480SizeH264Video"];
        else if ([mediaType isEqualToString: QTMediaTypeSound])
            options = [QTCompressionOptions compressionOptionsWithIdentifier: @"QTCompressionOptionsHighQualityAACAudio"];

        [d->fileOutput setCompressionOptions: options forConnection: connection];
    }

    [d->fileOutput recordToOutputFileURL: [NSURL fileURLWithPath: juceStringToNS (file.getFullPathName())]];
    isRecording = true;
}

const Time CameraDevice::getTimeOfFirstRecordedFrame() const
{
    QTCameraDeviceInteral* const d = (QTCameraDeviceInteral*) internal;
    if (d->callbackDelegate->firstRecordedTime != 0)
        return *d->callbackDelegate->firstRecordedTime;

    return Time();
}

void CameraDevice::stopRecording()
{
    if (isRecording)
    {
        QTCameraDeviceInteral* const d = (QTCameraDeviceInteral*) internal;
        d->resetFile();
        isRecording = false;
    }
}

void CameraDevice::addListener (CameraImageListener* listenerToAdd)
{
    QTCameraDeviceInteral* const d = (QTCameraDeviceInteral*) internal;

    if (listenerToAdd != 0)
        d->addListener (listenerToAdd);
}

void CameraDevice::removeListener (CameraImageListener* listenerToRemove)
{
    QTCameraDeviceInteral* const d = (QTCameraDeviceInteral*) internal;

    if (listenerToRemove != 0)
        d->removeListener (listenerToRemove);
}

//==============================================================================
const StringArray CameraDevice::getAvailableDevices()
{
    const ScopedAutoReleasePool pool;

    StringArray results;
    NSArray* devs = [QTCaptureDevice inputDevicesWithMediaType: QTMediaTypeVideo];

    for (int i = 0; i < [devs count]; ++i)
    {
        QTCaptureDevice* dev = (QTCaptureDevice*) [devs objectAtIndex: i];
        results.add (nsStringToJuce ([dev localizedDisplayName]));
    }

    return results;
}

CameraDevice* CameraDevice::openDevice (int index,
                                        int minWidth, int minHeight,
                                        int maxWidth, int maxHeight)
{
    ScopedPointer <CameraDevice> d (new CameraDevice (getAvailableDevices() [index], index));

    if (((QTCameraDeviceInteral*) (d->internal))->openingError.isEmpty())
        return d.release();

    return 0;
}

#endif
