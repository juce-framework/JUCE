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

#include "../jucedemo_headers.h"

#if JUCE_USE_CAMERA


//==============================================================================
class CameraDemo  : public Component,
                    public ComboBoxListener,
                    public ButtonListener,
                    public CameraImageListener
{
public:
    //==============================================================================
    CameraDemo()
    {
        setName (T("Camera"));

        cameraDevice = 0;
        cameraPreviewComp = 0;
        lastSnapshot = 0;
        recordingMovie = false;

        addAndMakeVisible (cameraSelectorComboBox = new ComboBox (T("Camera")));
        createListOfCameras();
        cameraSelectorComboBox->setSelectedId (1);
        cameraSelectorComboBox->addListener (this);

        addAndMakeVisible (snapshotButton = new TextButton (T("Take a snapshot")));
        snapshotButton->addButtonListener (this);
        snapshotButton->setEnabled (false);

        addAndMakeVisible (recordMovieButton = new TextButton (T("Record a movie file (to your desktop)...")));
        recordMovieButton->addButtonListener (this);
        recordMovieButton->setEnabled (false);

        cameraSelectorComboBox->setSelectedId (2);
    }

    ~CameraDemo()
    {
        deleteAllChildren();
        delete cameraDevice;
        delete lastSnapshot;
    }

    void paint (Graphics& g)
    {
        if (lastSnapshot != 0)
            g.drawImageWithin (lastSnapshot,
                               getWidth() / 2 + 10, 40,
                               getWidth() / 2 - 20, getHeight() - 50,
                               RectanglePlacement::centred, false);
    }

    void resized()
    {
        cameraSelectorComboBox->setBounds (10, 4, 250, 24);
        snapshotButton->changeWidthToFitText (24);
        snapshotButton->setTopLeftPosition (cameraSelectorComboBox->getRight() + 20, 4);
        recordMovieButton->changeWidthToFitText (24);
        recordMovieButton->setTopLeftPosition (snapshotButton->getRight() + 20, 4);

        if (cameraPreviewComp != 0)
            cameraPreviewComp->setBounds (10, 40, getWidth() / 2 - 20, getHeight() - 50);
    }

    void comboBoxChanged (ComboBox*)
    {
        // This is called when the user chooses a camera from the drop-down list.
        deleteAndZero (cameraDevice);
        deleteAndZero (cameraPreviewComp);
        recordingMovie = false;

        if (cameraSelectorComboBox->getSelectedId() > 1)
        {
            // Try to open the user's choice of camera..
            cameraDevice = CameraDevice::openDevice (cameraSelectorComboBox->getSelectedId() - 2);

            // and if it worked, create a preview component for it..
            if (cameraDevice != 0)
                addAndMakeVisible (cameraPreviewComp = cameraDevice->createViewerComponent());
        }

        snapshotButton->setEnabled (cameraDevice != 0);
        recordMovieButton->setEnabled (cameraDevice != 0);
        resized();
    }

    void createListOfCameras()
    {
        cameraSelectorComboBox->clear();
        cameraSelectorComboBox->addItem ("No camera", 1);
        cameraSelectorComboBox->addSeparator();

        StringArray cameras = CameraDevice::getAvailableDevices();

        for (int i = 0; i < cameras.size(); ++i)
            cameraSelectorComboBox->addItem (cameras[i], i + 2);
    }

    void buttonClicked (Button* b)
    {
        if (cameraDevice != 0)
        {
            if (b == recordMovieButton)
            {
                // The user has clicked the record movie button..
                if (! recordingMovie)
                {
                    // Start recording to a file on the user's desktop..
                    recordingMovie = true;

                    File file (File::getSpecialLocation (File::userDesktopDirectory)
                                .getNonexistentChildFile (T("JuceCameraDemo"),
                                                          CameraDevice::getFileExtension()));

                    cameraDevice->startRecordingToFile (file);
                    recordMovieButton->setButtonText (T("Stop Recording"));
                }
                else
                {
                    // Already recording, so stop...
                    recordingMovie = false;
                    cameraDevice->stopRecording();
                    recordMovieButton->setButtonText (T("Start recording (to a file on your desktop)"));
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
    void imageReceived (Image& image)
    {
        // In this app we just want to take one image, so as soon as this happens,
        // we'll unregister ourselves as a listener.
        if (cameraDevice != 0)
            cameraDevice->removeListener (this);

        // This callback won't be on the message thread, so need to lock it before using
        // data that may already be in use..
        const MessageManagerLock mm;
        deleteAndZero (lastSnapshot);
        lastSnapshot = image.createCopy();
        repaint();
    }

private:
    //==============================================================================
    CameraDevice* cameraDevice;

    ComboBox* cameraSelectorComboBox;
    TextButton* snapshotButton;
    TextButton* recordMovieButton;
    Component* cameraPreviewComp;
    bool recordingMovie;

    Image* lastSnapshot;
};


//==============================================================================
Component* createCameraDemo()
{
    return new CameraDemo();
}

#endif
