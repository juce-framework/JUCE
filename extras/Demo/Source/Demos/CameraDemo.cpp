/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-12 by Raw Material Software Ltd.

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

#include "../JuceDemoHeader.h"

#if JUCE_USE_CAMERA

//==============================================================================
class CameraDemo  : public Component,
                    private ComboBoxListener,
                    private ButtonListener,
                    private CameraDevice::Listener,
                    private AsyncUpdater
{
public:
    CameraDemo()
        : cameraSelectorComboBox ("Camera"),
          snapshotButton ("Take a snapshot"),
          recordMovieButton ("Record a movie (to your desktop)..."),
          recordingMovie (false)
    {
        setOpaque (true);

        addAndMakeVisible (cameraSelectorComboBox);
        updateCameraList();
        cameraSelectorComboBox.setSelectedId (1);
        cameraSelectorComboBox.addListener (this);

        addAndMakeVisible (snapshotButton);
        snapshotButton.addListener (this);
        snapshotButton.setEnabled (false);

        addAndMakeVisible (recordMovieButton);
        recordMovieButton.addListener (this);
        recordMovieButton.setEnabled (false);

        addAndMakeVisible (lastSnapshot);

        cameraSelectorComboBox.setSelectedId (2);
    }

    //==============================================================================
    void paint (Graphics& g) override
    {
        g.fillAll (Colours::black);
    }

    void resized() override
    {
        Rectangle<int> r (getLocalBounds().reduced (5));

        Rectangle<int> top (r.removeFromTop (25));
        cameraSelectorComboBox.setBounds (top.removeFromLeft (250));

        r.removeFromTop (4);
        top = r.removeFromTop (25);

        snapshotButton.changeWidthToFitText (24);
        snapshotButton.setBounds (top.removeFromLeft (snapshotButton.getWidth()));
        top.removeFromLeft (4);
        recordMovieButton.changeWidthToFitText (24);
        recordMovieButton.setBounds (top.removeFromLeft (recordMovieButton.getWidth()));

        r.removeFromTop (4);
        Rectangle<int> previewArea (r.removeFromTop (r.getHeight() / 2));

        if (cameraPreviewComp != nullptr)
            cameraPreviewComp->setBounds (previewArea);

        r.removeFromTop (4);
        lastSnapshot.setBounds (r);
    }


private:
    //==============================================================================
    ScopedPointer<CameraDevice> cameraDevice;
    ScopedPointer<Component> cameraPreviewComp;
    ImageComponent lastSnapshot;

    ComboBox cameraSelectorComboBox;
    TextButton snapshotButton;
    TextButton recordMovieButton;
    bool recordingMovie;

    void updateCameraList()
    {
        cameraSelectorComboBox.clear();
        cameraSelectorComboBox.addItem ("No camera", 1);
        cameraSelectorComboBox.addSeparator();

        StringArray cameras = CameraDevice::getAvailableDevices();

        for (int i = 0; i < cameras.size(); ++i)
            cameraSelectorComboBox.addItem (cameras[i], i + 2);
    }

    void comboBoxChanged (ComboBox*) override
    {
        // This is called when the user chooses a camera from the drop-down list.
        cameraDevice = nullptr;
        cameraPreviewComp = nullptr;
        recordingMovie = false;

        if (cameraSelectorComboBox.getSelectedId() > 1)
        {
            // Try to open the user's choice of camera..
            cameraDevice = CameraDevice::openDevice (cameraSelectorComboBox.getSelectedId() - 2);

            // and if it worked, create a preview component for it..
            if (cameraDevice != nullptr)
                addAndMakeVisible (cameraPreviewComp = cameraDevice->createViewerComponent());
        }

        snapshotButton.setEnabled (cameraDevice != nullptr);
        recordMovieButton.setEnabled (cameraDevice != nullptr);
        resized();
    }

    void buttonClicked (Button* b) override
    {
        if (cameraDevice != nullptr)
        {
            if (b == &recordMovieButton)
            {
                // The user has clicked the record movie button..
                if (! recordingMovie)
                {
                    // Start recording to a file on the user's desktop..
                    recordingMovie = true;

                    File file (File::getSpecialLocation (File::userDesktopDirectory)
                               .getNonexistentChildFile ("JuceCameraDemo",
                                                         CameraDevice::getFileExtension()));

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
            else
            {
                // When the user clicks the snapshot button, we'll attach ourselves to
                // the camera as a listener, and wait for an image to arrive...
                cameraDevice->addListener (this);
            }
        }
    }

    // This is called by the camera device when a new image arrives
    void imageReceived (const Image& image) override
    {
        // In this app we just want to take one image, so as soon as this happens,
        // we'll unregister ourselves as a listener.
        if (cameraDevice != nullptr)
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CameraDemo);
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<CameraDemo> demo ("29 Graphics: Camera Capture");

#endif // JUCE_USE_CAMERA
