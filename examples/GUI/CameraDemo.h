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
 vendor:           juce
 website:          http://juce.com
 description:      Showcases camera features.

 dependencies:     juce_core, juce_cryptography, juce_data_structures, juce_events,
                   juce_graphics, juce_gui_basics, juce_gui_extra, juce_video
 exporters:        xcode_mac, vs2017, linux_make

 moduleFlags:      JUCE_USE_CAMERA=1

 type:             Component
 mainClass:        CameraDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class CameraDemo  : public Component,
                    private CameraDevice::Listener,
                    private AsyncUpdater
{
public:
    CameraDemo()
    {
        setOpaque (true);

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
        auto previewArea = r.removeFromTop (r.getHeight() / 2);

        if (cameraPreviewComp.get() != nullptr)
            cameraPreviewComp->setBounds (previewArea);

        r.removeFromTop (4);
        lastSnapshot.setBounds (r);
    }


private:
    //==============================================================================
    ScopedPointer<CameraDevice> cameraDevice;
    ScopedPointer<Component> cameraPreviewComp;
    ImageComponent lastSnapshot;

    ComboBox cameraSelectorComboBox  { "Camera" };
    TextButton snapshotButton        { "Take a snapshot" };
    TextButton recordMovieButton     { "Record a movie (to your desktop)..." };
    bool recordingMovie = false;

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
        cameraDevice     .reset();
        cameraPreviewComp.reset();
        recordingMovie = false;

        if (cameraSelectorComboBox.getSelectedId() > 1)
        {
            // Try to open the user's choice of camera..
            cameraDevice.reset (CameraDevice::openDevice (cameraSelectorComboBox.getSelectedId() - 2));

            // and if it worked, create a preview component for it..
            if (cameraDevice.get() != nullptr)
            {
                cameraPreviewComp.reset (cameraDevice->createViewerComponent());
                addAndMakeVisible (cameraPreviewComp.get());
            }
        }

        snapshotButton   .setEnabled (cameraDevice.get() != nullptr);
        recordMovieButton.setEnabled (cameraDevice.get() != nullptr);
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

                auto file = File::getSpecialLocation (File::userDesktopDirectory)
                                 .getNonexistentChildFile ("JuceCameraDemo", CameraDevice::getFileExtension());

                cameraDevice->startRecordingToFile (file);
                recordMovieButton.setButtonText ("Stop Recording");
            }
            else
            {
                // Already recording, so stop...
                recordingMovie = false;
                cameraDevice->stopRecording();
                recordMovieButton.setButtonText ("Start recording (to a file on your desktop)");
            }
        }
    }

    void takeSnapshot()
    {
        // When the user clicks the snapshot button, we'll attach ourselves to
        // the camera as a listener, and wait for an image to arrive...
        cameraDevice->addListener (this);
    }

    // This is called by the camera device when a new image arrives
    void imageReceived (const Image& image) override
    {
        // In this app we just want to take one image, so as soon as this happens,
        // we'll unregister ourselves as a listener.
        if (cameraDevice.get() != nullptr)
            cameraDevice->removeListener (this);

        // This callback won't be on the message thread, so to get the image back to
        // the message thread, we'll stash a pointer to it (which is reference-counted in
        // a thead-safe way), and trigger an async callback which will then display the
        // new image..
        incomingImage = image;
        triggerAsyncUpdate();
    }

    Image incomingImage;

    void handleAsyncUpdate() override
    {
        if (incomingImage.isValid())
            lastSnapshot.setImage (incomingImage);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDemo)
};
