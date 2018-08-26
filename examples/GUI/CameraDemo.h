/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2017 - ROLI Ltd.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             CameraDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Showcases camera features.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_core, juce_cryptography,
                   juce_data_structures, juce_events, juce_graphics, juce_gui_basics,
                   juce_gui_extra, juce_video
 exporters:        xcode_mac, vs2017, androidstudio, xcode_iphone

 moduleFlags:      JUCE_USE_CAMERA=1, JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        CameraDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class CameraDemo  : public Component
{
public:
    CameraDemo()
    {
        setOpaque (true);

       #if JUCE_ANDROID
        // Android requires exclusive access to the audio device when recording videos.
        audioDeviceManager.closeAudioDevice();
       #endif

        addAndMakeVisible (cameraSelectorComboBox);
        updateCameraList();
        cameraSelectorComboBox.setSelectedId (1);
        cameraSelectorComboBox.onChange = [this] { cameraChanged(); };

        addAndMakeVisible (snapshotButton);
        snapshotButton.onClick = [this] { takeSnapshot(); };
        snapshotButton.setEnabled (false);

        addAndMakeVisible (recordMovieButton);
        recordMovieButton.onClick = [this] { startRecording(); };
        recordMovieButton.setEnabled (false);

        addAndMakeVisible (lastSnapshot);

        cameraSelectorComboBox.setSelectedId (2);

        setSize (500, 500);

       #if JUCE_IOS || JUCE_ANDROID
        setPortraitOrientationEnabled (true);
       #endif
    }

    ~CameraDemo()
    {
       #if JUCE_IOS || JUCE_ANDROID
        setPortraitOrientationEnabled (false);
       #endif

       #if JUCE_ANDROID
        audioDeviceManager.restartLastAudioDevice();
       #endif
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (5);

        auto top = r.removeFromTop (25);
        cameraSelectorComboBox.setBounds (top.removeFromLeft (250));

        r.removeFromTop (4);
        top = r.removeFromTop (25);

        snapshotButton.changeWidthToFitText (24);
        snapshotButton.setBounds (top.removeFromLeft (snapshotButton.getWidth()));
        top.removeFromLeft (4);
        recordMovieButton.changeWidthToFitText (24);
        recordMovieButton.setBounds (top.removeFromLeft (recordMovieButton.getWidth()));

        r.removeFromTop (4);
        auto previewArea = shouldUseLandscapeLayout() ? r.removeFromLeft (r.getWidth() / 2)
                                                      : r.removeFromTop (r.getHeight() / 2);

        if (cameraPreviewComp.get() != nullptr)
            cameraPreviewComp->setBounds (previewArea);

        if (shouldUseLandscapeLayout())
            r.removeFromLeft (4);
        else
            r.removeFromTop (4);

        lastSnapshot.setBounds (r);
    }


private:
    //==============================================================================
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    std::unique_ptr<CameraDevice> cameraDevice;
    std::unique_ptr<Component> cameraPreviewComp;
    ImageComponent lastSnapshot;

    ComboBox cameraSelectorComboBox  { "Camera" };
    TextButton snapshotButton        { "Take a snapshot" };
   #if ! JUCE_ANDROID && ! JUCE_IOS
    TextButton recordMovieButton     { "Record a movie (to your desktop)..." };
   #else
    TextButton recordMovieButton     { "Record a movie" };
   #endif
    bool recordingMovie = false;
    File recordingFile;
    bool contentSharingPending = false;

    void setPortraitOrientationEnabled (bool shouldBeEnabled)
    {
        auto allowedOrientations = Desktop::getInstance().getOrientationsEnabled();

        if (shouldBeEnabled)
            allowedOrientations |= Desktop::upright;
        else
            allowedOrientations &= ~Desktop::upright;

        Desktop::getInstance().setOrientationsEnabled (allowedOrientations);
    }

    bool shouldUseLandscapeLayout() const noexcept
    {
       #if JUCE_ANDROID || JUCE_IOS
        auto orientation = Desktop::getInstance().getCurrentOrientation();
        return orientation == Desktop::rotatedClockwise || orientation == Desktop::rotatedAntiClockwise;
       #else
        return false;
       #endif
    }

    void updateCameraList()
    {
        cameraSelectorComboBox.clear();
        cameraSelectorComboBox.addItem ("No camera", 1);
        cameraSelectorComboBox.addSeparator();

        auto cameras = CameraDevice::getAvailableDevices();

        for (int i = 0; i < cameras.size(); ++i)
            cameraSelectorComboBox.addItem (cameras[i], i + 2);
    }

    void cameraChanged()
    {
        // This is called when the user chooses a camera from the drop-down list.
       #if JUCE_IOS
        // On iOS, when switching camera, open the new camera first, so that it can
        // share the underlying camera session with the old camera. Otherwise, the
        // session would have to be closed first, which can take several seconds.
        if (cameraSelectorComboBox.getSelectedId() == 1)
            cameraDevice.reset();
       #else
        cameraDevice.reset();
       #endif
        cameraPreviewComp.reset();
        recordingMovie = false;

        if (cameraSelectorComboBox.getSelectedId() > 1)
        {
           #if JUCE_ANDROID || JUCE_IOS
            openCameraAsync();
           #else
            cameraDeviceOpenResult (CameraDevice::openDevice (cameraSelectorComboBox.getSelectedId() - 2), {});
           #endif
        }
        else
        {
            snapshotButton   .setEnabled (cameraDevice != nullptr && ! contentSharingPending);
            recordMovieButton.setEnabled (cameraDevice != nullptr && ! contentSharingPending);
            resized();
        }
    }

    void openCameraAsync()
    {
        SafePointer<CameraDemo> safeThis (this);

        CameraDevice::openDeviceAsync (cameraSelectorComboBox.getSelectedId() - 2,
                                       [safeThis] (CameraDevice* device, const String& error) mutable
                                       {
                                           if (safeThis)
                                               safeThis->cameraDeviceOpenResult (device, error);
                                       });
    }

    void cameraDeviceOpenResult (CameraDevice* device, const String& error)
    {
        // If camera opening worked, create a preview component for it..
        cameraDevice.reset (device);

        if (cameraDevice.get() != nullptr)
        {
           #if JUCE_ANDROID
            SafePointer<CameraDemo> safeThis (this);
            cameraDevice->onErrorOccurred = [safeThis] (const String& error) mutable { if (safeThis) safeThis->errorOccurred (error); };
           #endif
            cameraPreviewComp.reset (cameraDevice->createViewerComponent());
            addAndMakeVisible (cameraPreviewComp.get());
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Camera open failed",
                                              "Camera open failed, reason: " + error);
        }

        snapshotButton   .setEnabled (cameraDevice.get() != nullptr && ! contentSharingPending);
        recordMovieButton.setEnabled (cameraDevice.get() != nullptr && ! contentSharingPending);
        resized();
    }

    void startRecording()
    {
        if (cameraDevice.get() != nullptr)
        {
            // The user has clicked the record movie button..
            if (! recordingMovie)
            {
                // Start recording to a file on the user's desktop..
                recordingMovie = true;

               #if JUCE_ANDROID || JUCE_IOS
                recordingFile = File::getSpecialLocation (File::tempDirectory)
               #else
                recordingFile = File::getSpecialLocation (File::userDesktopDirectory)
               #endif
                                 .getNonexistentChildFile ("JuceCameraVideoDemo", CameraDevice::getFileExtension());

               #if JUCE_ANDROID
                // Android does not support taking pictures while recording video.
                snapshotButton.setEnabled (false);
               #endif

                cameraSelectorComboBox.setEnabled (false);
                cameraDevice->startRecordingToFile (recordingFile);
                recordMovieButton.setButtonText ("Stop Recording");
            }
            else
            {
                // Already recording, so stop...
                recordingMovie = false;
                cameraDevice->stopRecording();
               #if ! JUCE_ANDROID && ! JUCE_IOS
                recordMovieButton.setButtonText ("Start recording (to a file on your desktop)");
               #else
                recordMovieButton.setButtonText ("Record a movie");
               #endif
                cameraSelectorComboBox.setEnabled (true);

               #if JUCE_ANDROID
                snapshotButton.setEnabled (true);
               #endif

               #if JUCE_ANDROID || JUCE_IOS
                URL url (recordingFile);

                snapshotButton   .setEnabled (false);
                recordMovieButton.setEnabled (false);
                contentSharingPending = true;

                SafePointer<CameraDemo> safeThis (this);

                juce::ContentSharer::getInstance()->shareFiles ({url},
                                                                [safeThis] (bool success, const String&) mutable
                                                                {
                                                                    if (safeThis)
                                                                        safeThis->sharingFinished (success, false);
                                                                });
               #endif
            }
        }
    }

    void takeSnapshot()
    {
        SafePointer<CameraDemo> safeThis (this);
        cameraDevice->takeStillPicture ([safeThis] (const Image& image) mutable { safeThis->imageReceived (image); });
    }

    // This is called by the camera device when a new image arrives
    void imageReceived (const Image& image)
    {
        if (! image.isValid())
            return;

        lastSnapshot.setImage (image);

       #if JUCE_ANDROID || JUCE_IOS
        auto imageFile = File::getSpecialLocation (File::tempDirectory).getNonexistentChildFile ("JuceCameraPhotoDemo", ".jpg");

        if (auto stream = std::unique_ptr<OutputStream> (imageFile.createOutputStream()))
        {
            if (JPEGImageFormat().writeImageToStream (image, *stream))
            {
                URL url (imageFile);

                snapshotButton   .setEnabled (false);
                recordMovieButton.setEnabled (false);
                contentSharingPending = true;

                SafePointer<CameraDemo> safeThis (this);

                juce::ContentSharer::getInstance()->shareFiles ({url},
                                                                [safeThis] (bool success, const String&) mutable
                                                                {
                                                                    if (safeThis)
                                                                        safeThis->sharingFinished (success, true);
                                                                });
            }
        }
       #endif
    }

    void errorOccurred (const String& error)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "Camera Device Error",
                                          "An error has occurred: " + error + " Camera will be closed.");

        cameraDevice.reset();

        cameraSelectorComboBox.setSelectedId (1);
        snapshotButton   .setEnabled (false);
        recordMovieButton.setEnabled (false);
    }

    void sharingFinished (bool success, bool isCapture)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          isCapture ? "Image sharing result" : "Video sharing result",
                                          success ? "Success!" : "Failed!");

        contentSharingPending = false;
        snapshotButton   .setEnabled (true);
        recordMovieButton.setEnabled (true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDemo)
};
