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

namespace juce
{

#if JUCE_USE_CAMERA || DOXYGEN


//==============================================================================
/**
    Controls any video capture devices that might be available.

    Use getAvailableDevices() to list the devices that are attached to the
    system, then call openDevice() or openDeviceAsync() to open one for use.
    Once you have a CameraDevice object, you can get a viewer component from it,
    and use its methods to stream to a file or capture still-frames.

    @tags{Video}
*/
class JUCE_API  CameraDevice
{
public:
    /** Destructor. */
    virtual ~CameraDevice();

    //==============================================================================
    /** Returns a list of the available cameras on this machine.

        You can open one of these devices by calling openDevice() or openDeviceAsync().
    */
    static StringArray getAvailableDevices();

    /** Synchronously opens a camera device. This function should not be used on iOS or
        Android, use openDeviceAsync() instead.

        The index parameter indicates which of the items returned by getAvailableDevices()
        to open.

        The size constraints allow the method to choose between different resolutions if
        the camera supports this. If the resolution can't be specified (e.g. on the Mac)
        then these will be ignored.

        On Mac, if highQuality is false, then the camera will be opened in preview mode
        which will allow the OS to drop frames if the computer cannot keep up in processing
        the frames.
    */
    static CameraDevice* openDevice (int deviceIndex,
                                     int minWidth = 128, int minHeight = 64,
                                     int maxWidth = 1024, int maxHeight = 768,
                                     bool highQuality = true);

    using OpenCameraResultCallback = std::function<void (CameraDevice*, const String& /*error*/)>;

    /** Asynchronously opens a camera device on iOS (iOS 7+) or Android (API 21+).
        On other platforms, the function will simply call openDevice(). Upon completion,
        resultCallback will be invoked with valid CameraDevice* and an empty error
        String on success, or nullptr CameraDevice and a non-empty error String on failure.

        This is the preferred method of opening a camera device, because it works on all
        platforms, whereas synchronous openDevice() does not work on iOS & Android.

        The index parameter indicates which of the items returned by getAvailableDevices()
        to open.

        The size constraints allow the method to choose between different resolutions if
        the camera supports this. If the resolution can't be specified then these will be
        ignored.

        On iOS, if you want to switch a device, it is more efficient to open a new device
        before closing the older one, because this way both devices can share the same
        underlying camera session. Otherwise, the session needs to be close first, and this
        is a lengthy process that can take several seconds.

        The Android implementation currently supports a maximum recording resolution of
        1080p. Choosing a larger size will result in larger pictures taken, but the video
        will be capped at 1080p.
    */
    static void openDeviceAsync (int deviceIndex,
                                 OpenCameraResultCallback resultCallback,
                                 int minWidth = 128, int minHeight = 64,
                                 int maxWidth = 1024, int maxHeight = 768,
                                 bool highQuality = true);

    //==============================================================================
    /** Returns the name of this device */
    const String& getName() const noexcept          { return name; }

    /** Creates a component that can be used to display a preview of the
        video from this camera.

        Note: While you can change the size of the preview component, the actual
        preview display may be smaller than the size requested, because the correct
        aspect ratio is maintained automatically.
    */
    Component* createViewerComponent();

    //==============================================================================
    /** Triggers a still picture capture. Upon completion, pictureTakenCallback will
        be invoked on a message thread.

        On Android, before calling takeStillPicture(), you need to create a preview with
        createViewerComponent() and you need to make it visible on screen.

        Android does not support simultaneous video recording and still picture capture.
     */
    void takeStillPicture (std::function<void (const Image&)> pictureTakenCallback);

    /** Starts recording video to the specified file.

        You should use getFileExtension() to find out the correct extension to
        use for your filename.

        If the file exists, it will be deleted before the recording starts.

        This method may not start recording instantly, so if you need to know the
        exact time at which the file begins, you can call getTimeOfFirstRecordedFrame()
        after the recording has finished.

        The quality parameter can be 0, 1, or 2, to indicate low, medium, or high. It may
        or may not be used, depending on the driver.

        On Android, before calling startRecordingToFile(), you need to create a preview with
        createViewerComponent() and you need to make it visible on screen.

        The Android camera also requires exclusive access to the audio device, so make sure
        you close any open audio devices with AudioDeviceManager::closeAudioDevice() first.

        Android does not support simultaneous video recording and still picture capture.

        @see AudioDeviceManager::closeAudioDevice, AudioDeviceManager::restartLastAudioDevice
    */
    void startRecordingToFile (const File& file, int quality = 2);

    /** Stops recording, after a call to startRecordingToFile(). */
    void stopRecording();

    /** Returns the file extension that should be used for the files
        that you pass to startRecordingToFile().

        This may be platform-specific, e.g. ".mov" or ".avi".
    */
    static String getFileExtension();

    /** After calling stopRecording(), this method can be called to return the timestamp
        of the first frame that was written to the file.
    */
    Time getTimeOfFirstRecordedFrame() const;

    /** Set this callback to be notified whenever an error occurs. You may need to close
        and reopen the device to be able to use it further. */
    std::function<void (const String& /*error*/)> onErrorOccurred;

    //==============================================================================
    /**
        Receives callbacks with individual frames from a CameraDevice. It is mainly
        useful for processing multiple frames that has to be done as quickly as
        possible. The callbacks can be called from any thread.

        If you just need to take one picture, you should use takeStillPicture() instead.

        @see CameraDevice::addListener
    */
    class JUCE_API  Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}

        /** This method is called when a new image arrives.

            This may be called by any thread, so be careful about thread-safety,
            and make sure that you process the data as quickly as possible to
            avoid glitching!

            Simply add a listener to be continuously notified about new frames becoming
            available and remove the listener when you no longer need new frames.

            If you just need to take one picture, use takeStillPicture() instead.

            @see CameraDevice::takeStillPicture
        */
        virtual void imageReceived (const Image& image) = 0;
    };

    /** Adds a listener to receive images from the camera.

        Be very careful not to delete the listener without first removing it by calling
        removeListener().
    */
    void addListener (Listener* listenerToAdd);

    /** Removes a listener that was previously added with addListener(). */
    void removeListener (Listener* listenerToRemove);

private:
    String name;

    struct Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    struct ViewerComponent;
    friend struct ViewerComponent;

    CameraDevice (const String& name, int index,
                  int minWidth, int minHeight, int maxWidth, int maxHeight, bool highQuality);

   #if JUCE_ANDROID || JUCE_IOS
    class CameraFactory;
   #endif

   #if JUCE_ANDROID
    friend void juce_cameraDeviceStateClosed (int64);
    friend void juce_cameraDeviceStateDisconnected (int64);
    friend void juce_cameraDeviceStateError (int64, int);
    friend void juce_cameraDeviceStateOpened (int64, void*);

    friend void juce_cameraCaptureSessionActive (int64, void*);
    friend void juce_cameraCaptureSessionClosed (int64, void*);
    friend void juce_cameraCaptureSessionConfigureFailed (int64, void*);
    friend void juce_cameraCaptureSessionConfigured (int64, void*);
    friend void juce_cameraCaptureSessionReady (int64, void*);

    friend void juce_cameraCaptureSessionCaptureCompleted (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureFailed (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureProgressed (int64, bool, void*, void*, void*);
    friend void juce_cameraCaptureSessionCaptureSequenceAborted (int64, bool, void*, int);
    friend void juce_cameraCaptureSessionCaptureSequenceCompleted (int64, bool, void*, int, int64);
    friend void juce_cameraCaptureSessionCaptureStarted (int64, bool, void*, void*, int64, int64);

    friend void juce_deviceOrientationChanged (int64, int);
   #endif

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDevice)
};

#endif

} // namespace juce
