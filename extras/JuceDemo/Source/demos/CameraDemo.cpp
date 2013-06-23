/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#include "../jucedemo_headers.h"

#if JUCE_USE_CAMERA


//==============================================================================
class CameraDemo  : public Component,
                    public ComboBoxListener,
                    public ButtonListener,
                    public CameraDevice::Listener
{
public:
    //==============================================================================
    CameraDemo()
        : cameraSelectorComboBox ("Camera"),
          snapshotButton ("Take a snapshot"),
          recordMovieButton ("Record a movie file (to your desktop)..."),
          recordingMovie (false)
    {
        setName ("Camera");

        addAndMakeVisible (&cameraSelectorComboBox);
        createListOfCameras();
        cameraSelectorComboBox.setSelectedId (1);
        cameraSelectorComboBox.addListener (this);

        addAndMakeVisible (&snapshotButton);
        snapshotButton.addListener (this);
        snapshotButton.setEnabled (false);

        addAndMakeVisible (&recordMovieButton);
        recordMovieButton.addListener (this);
        recordMovieButton.setEnabled (false);

        cameraSelectorComboBox.setSelectedId (2);
    }

    ~CameraDemo()
    {
    }

    void paint (Graphics& g)
    {
        g.drawImageWithin (lastSnapshot,
                           getWidth() / 2 + 10, 40,
                           getWidth() / 2 - 20, getHeight() - 50,
                           RectanglePlacement::centred, false);
    }

    void resized()
    {
        cameraSelectorComboBox.setBounds (10, 4, 250, 24);
        snapshotButton.changeWidthToFitText (24);
        snapshotButton.setTopLeftPosition (cameraSelectorComboBox.getRight() + 20, 4);
        recordMovieButton.changeWidthToFitText (24);
        recordMovieButton.setTopLeftPosition (snapshotButton.getRight() + 20, 4);

        if (cameraPreviewComp != 0)
            cameraPreviewComp->setBounds (10, 40, getWidth() / 2 - 20, getHeight() - 50);
    }

    void comboBoxChanged (ComboBox*)
    {
        // This is called when the user chooses a camera from the drop-down list.
        cameraDevice = 0;
        cameraPreviewComp = 0;
        recordingMovie = false;

        if (cameraSelectorComboBox.getSelectedId() > 1)
        {
            // Try to open the user's choice of camera..
            cameraDevice = CameraDevice::openDevice (cameraSelectorComboBox.getSelectedId() - 2);

            // and if it worked, create a preview component for it..
            if (cameraDevice != 0)
                addAndMakeVisible (cameraPreviewComp = cameraDevice->createViewerComponent());
        }

        snapshotButton.setEnabled (cameraDevice != 0);
        recordMovieButton.setEnabled (cameraDevice != 0);
        resized();
    }

    void createListOfCameras()
    {
        cameraSelectorComboBox.clear();
        cameraSelectorComboBox.addItem ("No camera", 1);
        cameraSelectorComboBox.addSeparator();

        StringArray cameras = CameraDevice::getAvailableDevices();

        for (int i = 0; i < cameras.size(); ++i)
            cameraSelectorComboBox.addItem (cameras[i], i + 2);
    }

    void buttonClicked (Button* b)
    {
        if (cameraDevice != 0)
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
    void imageReceived (const Image& image)
    {
        // In this app we just want to take one image, so as soon as this happens,
        // we'll unregister ourselves as a listener.
        if (cameraDevice != 0)
            cameraDevice->removeListener (this);

        // This callback won't be on the message thread, so need to lock it before using
        // data that may already be in use..
        const MessageManagerLock mm;
        lastSnapshot = image;
        repaint();
    }

private:
    //==============================================================================
    ScopedPointer<CameraDevice> cameraDevice;
    ScopedPointer<Component> cameraPreviewComp;
    Image lastSnapshot;

    ComboBox cameraSelectorComboBox;
    TextButton snapshotButton;
    TextButton recordMovieButton;
    bool recordingMovie;
};


//==============================================================================
Component* createCameraDemo()
{
    return new CameraDemo();
}

#endif
