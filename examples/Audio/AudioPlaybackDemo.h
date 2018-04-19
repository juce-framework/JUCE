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

 name:             AudioPlaybackDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays an audio file.

 dependencies:     juce_audio_basics, juce_audio_devices, juce_audio_formats,
                   juce_audio_processors, juce_audio_utils, juce_core,
                   juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2017, linux_make, androidstudio, xcode_iphone

 type:             Component
 mainClass:        AudioPlaybackDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class DemoThumbnailComp  : public Component,
                           public ChangeListener,
                           public FileDragAndDropTarget,
                           public ChangeBroadcaster,
                           private ScrollBar::Listener,
                           private Timer
{
public:
    DemoThumbnailComp (AudioFormatManager& formatManager,
                       AudioTransportSource& source,
                       Slider& slider)
        : transportSource (source),
          zoomSlider (slider),
          thumbnail (512, formatManager, thumbnailCache)
    {
        thumbnail.addChangeListener (this);

        addAndMakeVisible (scrollbar);
        scrollbar.setRangeLimits (visibleRange);
        scrollbar.setAutoHide (false);
        scrollbar.addListener (this);

        currentPositionMarker.setFill (Colours::white.withAlpha (0.85f));
        addAndMakeVisible (currentPositionMarker);
    }

    ~DemoThumbnailComp()
    {
        scrollbar.removeListener (this);
        thumbnail.removeChangeListener (this);
    }

    void setURL (const URL& url)
    {
        InputSource* inputSource = nullptr;

       #if ! JUCE_IOS
        if (url.isLocalFile())
        {
            inputSource = new FileInputSource (url.getLocalFile());
        }
        else
       #endif
        {
            if (inputSource == nullptr)
                inputSource = new URLInputSource (url);
        }

        if (inputSource != nullptr)
        {
            thumbnail.setSource (inputSource);

            Range<double> newRange (0.0, thumbnail.getTotalLength());
            scrollbar.setRangeLimits (newRange);
            setRange (newRange);

            startTimerHz (40);
        }
    }

    URL getLastDroppedFile() const noexcept { return lastFileDropped; }

    void setZoomFactor (double amount)
    {
        if (thumbnail.getTotalLength() > 0)
        {
            auto newScale = jmax (0.001, thumbnail.getTotalLength() * (1.0 - jlimit (0.0, 0.99, amount)));
            auto timeAtCentre = xToTime (getWidth() / 2.0f);

            setRange ({ timeAtCentre - newScale * 0.5, timeAtCentre + newScale * 0.5 });
        }
    }

    void setRange (Range<double> newRange)
    {
        visibleRange = newRange;
        scrollbar.setCurrentRange (visibleRange);
        updateCursorPosition();
        repaint();
    }

    void setFollowsTransport (bool shouldFollow)
    {
        isFollowingTransport = shouldFollow;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::darkgrey);
        g.setColour (Colours::lightblue);

        if (thumbnail.getTotalLength() > 0.0)
        {
            auto thumbArea = getLocalBounds();

            thumbArea.removeFromBottom (scrollbar.getHeight() + 4);
            thumbnail.drawChannels (g, thumbArea.reduced (2),
                                    visibleRange.getStart(), visibleRange.getEnd(), 1.0f);
        }
        else
        {
            g.setFont (14.0f);
            g.drawFittedText ("(No audio file selected)", getLocalBounds(), Justification::centred, 2);
        }
    }

    void resized() override
    {
        scrollbar.setBounds (getLocalBounds().removeFromBottom (14).reduced (2));
    }

    void changeListenerCallback (ChangeBroadcaster*) override
    {
        // this method is called by the thumbnail when it has changed, so we should repaint it..
        repaint();
    }

    bool isInterestedInFileDrag (const StringArray& /*files*/) override
    {
        return true;
    }

    void filesDropped (const StringArray& files, int /*x*/, int /*y*/) override
    {
        lastFileDropped = URL (File (files[0]));
        sendChangeMessage();
    }

    void mouseDown (const MouseEvent& e) override
    {
        mouseDrag (e);
    }

    void mouseDrag (const MouseEvent& e) override
    {
        if (canMoveTransport())
            transportSource.setPosition (jmax (0.0, xToTime ((float) e.x)));
    }

    void mouseUp (const MouseEvent&) override
    {
        transportSource.start();
    }

    void mouseWheelMove (const MouseEvent&, const MouseWheelDetails& wheel) override
    {
        if (thumbnail.getTotalLength() > 0.0)
        {
            auto newStart = visibleRange.getStart() - wheel.deltaX * (visibleRange.getLength()) / 10.0;
            newStart = jlimit (0.0, jmax (0.0, thumbnail.getTotalLength() - (visibleRange.getLength())), newStart);

            if (canMoveTransport())
                setRange ({ newStart, newStart + visibleRange.getLength() });

            if (wheel.deltaY != 0.0f)
                zoomSlider.setValue (zoomSlider.getValue() - wheel.deltaY);

            repaint();
        }
    }


private:
    AudioTransportSource& transportSource;
    Slider& zoomSlider;
    ScrollBar scrollbar  { false };

    AudioThumbnailCache thumbnailCache  { 5 };
    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport = false;
    URL lastFileDropped;

    DrawableRectangle currentPositionMarker;

    float timeToX (const double time) const
    {
        if (visibleRange.getLength() <= 0)
            return 0;

        return getWidth() * (float) ((time - visibleRange.getStart()) / visibleRange.getLength());
    }

    double xToTime (const float x) const
    {
        return (x / getWidth()) * (visibleRange.getLength()) + visibleRange.getStart();
    }

    bool canMoveTransport() const noexcept
    {
        return ! (isFollowingTransport && transportSource.isPlaying());
    }

    void scrollBarMoved (ScrollBar* scrollBarThatHasMoved, double newRangeStart) override
    {
        if (scrollBarThatHasMoved == &scrollbar)
            if (! (isFollowingTransport && transportSource.isPlaying()))
                setRange (visibleRange.movedToStartAt (newRangeStart));
    }

    void timerCallback() override
    {
        if (canMoveTransport())
            updateCursorPosition();
        else
            setRange (visibleRange.movedToStartAt (transportSource.getCurrentPosition() - (visibleRange.getLength() / 2.0)));
    }

    void updateCursorPosition()
    {
        currentPositionMarker.setVisible (transportSource.isPlaying() || isMouseButtonDown());

        currentPositionMarker.setRectangle (Rectangle<float> (timeToX (transportSource.getCurrentPosition()) - 0.75f, 0,
                                                              1.5f, (float) (getHeight() - scrollbar.getHeight())));
    }
};

//==============================================================================
class AudioPlaybackDemo  : public Component,
                          #if (JUCE_ANDROID || JUCE_IOS)
                           private Button::Listener,
                          #else
                           private FileBrowserListener,
                          #endif
                           private ChangeListener
{
public:
    AudioPlaybackDemo()
    {
        addAndMakeVisible (zoomLabel);
        zoomLabel.setFont (Font (15.00f, Font::plain));
        zoomLabel.setJustificationType (Justification::centredRight);
        zoomLabel.setEditable (false, false, false);
        zoomLabel.setColour (TextEditor::textColourId, Colours::black);
        zoomLabel.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (followTransportButton);
        followTransportButton.onClick = [this] { updateFollowTransportState(); };

        addAndMakeVisible (explanation);
        explanation.setFont (Font (14.00f, Font::plain));
        explanation.setJustificationType (Justification::bottomRight);
        explanation.setEditable (false, false, false);
        explanation.setColour (TextEditor::textColourId, Colours::black);
        explanation.setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (zoomSlider);
        zoomSlider.setRange (0, 1, 0);
        zoomSlider.onValueChange = [this] { thumbnail->setZoomFactor (zoomSlider.getValue()); };
        zoomSlider.setSkewFactor (2);

        thumbnail.reset (new DemoThumbnailComp (formatManager, transportSource, zoomSlider));
        addAndMakeVisible (thumbnail.get());
        thumbnail->addChangeListener (this);

        addAndMakeVisible (startStopButton);
        startStopButton.setColour (TextButton::buttonColourId, Colour (0xff79ed7f));
        startStopButton.setColour (TextButton::textColourOffId, Colours::black);
        startStopButton.onClick = [this] { startOrStop(); };

        // audio setup
        formatManager.registerBasicFormats();

        thread.startThread (3);

       #if (JUCE_ANDROID || JUCE_IOS)
        addAndMakeVisible (chooseFileButton);
        chooseFileButton.addListener (this);
       #else
        addAndMakeVisible (fileTreeComp);

        directoryList.setDirectory (File::getSpecialLocation (File::userHomeDirectory), true, true);

        fileTreeComp.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        fileTreeComp.addListener (this);
       #endif

       #ifndef JUCE_DEMO_RUNNER
        RuntimePermissions::request (RuntimePermissions::recordAudio,
                                     [this] (bool granted)
                                     {
                                         int numInputChannels = granted ? 2 : 0;
                                         audioDeviceManager.initialise (numInputChannels, 2, nullptr, true, {}, nullptr);
                                     });
       #endif

        audioDeviceManager.addAudioCallback (&audioSourcePlayer);
        audioSourcePlayer.setSource (&transportSource);

        setOpaque (true);
        setSize (500, 500);
    }

    ~AudioPlaybackDemo()
    {
        transportSource  .setSource (nullptr);
        audioSourcePlayer.setSource (nullptr);

        audioDeviceManager.removeAudioCallback (&audioSourcePlayer);

       #if (JUCE_ANDROID || JUCE_IOS)
        chooseFileButton.removeListener (this);
       #else
        fileTreeComp.removeListener (this);
       #endif

        thumbnail->removeChangeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto r = getLocalBounds().reduced (4);

        auto controls = r.removeFromBottom (90);
        explanation.setBounds (controls.removeFromRight (controls.getWidth() / 3));

        auto zoom = controls.removeFromTop (25);
        zoomLabel .setBounds (zoom.removeFromLeft (50));
        zoomSlider.setBounds (zoom);

        followTransportButton.setBounds (controls.removeFromTop (25));
        startStopButton      .setBounds (controls);

        r.removeFromBottom (6);
        thumbnail->setBounds (r.removeFromBottom (140));
        r.removeFromBottom (6);

       #if (JUCE_ANDROID || JUCE_IOS)
        chooseFileButton.setBounds (r);
       #else
        fileTreeComp.setBounds (r);
       #endif
    }

private:
    // if this PIP is running inside the demo runner, we'll use the shared device manager instead
   #ifndef JUCE_DEMO_RUNNER
    AudioDeviceManager audioDeviceManager;
   #else
    AudioDeviceManager& audioDeviceManager { getSharedAudioDeviceManager (0, 2) };
   #endif

    AudioFormatManager formatManager;
    TimeSliceThread thread  { "audio file preview" };

   #if (JUCE_ANDROID || JUCE_IOS)
    std::unique_ptr<FileChooser> fileChooser;
    TextButton chooseFileButton {"Choose Audio File...", "Choose an audio file for playback"};
   #else
    DirectoryContentsList directoryList {nullptr, thread};
    FileTreeComponent fileTreeComp {directoryList};
   #endif

    URL currentAudioFile;
    AudioSourcePlayer audioSourcePlayer;
    AudioTransportSource transportSource;
    std::unique_ptr<AudioFormatReaderSource> currentAudioFileSource;

    std::unique_ptr<DemoThumbnailComp> thumbnail;
    Label zoomLabel   { {}, "zoom:" },
          explanation { {}, "Select an audio file in the treeview above, and this page will display its waveform, and let you play it.." };
    Slider zoomSlider                   { Slider::LinearHorizontal, Slider::NoTextBox };
    ToggleButton followTransportButton  { "Follow Transport" };
    TextButton startStopButton          { "Play/Stop" };

    //==============================================================================
    void showAudioResource (URL resource)
    {
        if (loadURLIntoTransport (resource))
            currentAudioFile = static_cast<URL&&> (resource);

        zoomSlider.setValue (0, dontSendNotification);
        thumbnail->setURL (currentAudioFile);
    }

    bool loadURLIntoTransport (const URL& audioURL)
    {
        // unload the previous file source and delete it..
        transportSource.stop();
        transportSource.setSource (nullptr);
        currentAudioFileSource.reset();

        AudioFormatReader* reader = nullptr;

       #if ! JUCE_IOS
        if (audioURL.isLocalFile())
        {
            reader = formatManager.createReaderFor (audioURL.getLocalFile());
        }
        else
       #endif
        {
            if (reader == nullptr)
                reader = formatManager.createReaderFor (audioURL.createInputStream (false));
        }

        if (reader != nullptr)
        {
            currentAudioFileSource.reset (new AudioFormatReaderSource (reader, true));

            // ..and plug it into our transport source
            transportSource.setSource (currentAudioFileSource.get(),
                                       32768,                   // tells it to buffer this many samples ahead
                                       &thread,                 // this is the background thread to use for reading-ahead
                                       reader->sampleRate);     // allows for sample rate correction

            return true;
        }

        return false;
    }

    void startOrStop()
    {
        if (transportSource.isPlaying())
        {
            transportSource.stop();
        }
        else
        {
            transportSource.setPosition (0);
            transportSource.start();
        }
    }

    void updateFollowTransportState()
    {
        thumbnail->setFollowsTransport (followTransportButton.getToggleState());
    }

   #if (JUCE_ANDROID || JUCE_IOS)
    void buttonClicked (Button* btn) override
    {
        if (btn == &chooseFileButton && fileChooser.get() == nullptr)
        {
            SafePointer<AudioPlaybackDemo> safeThis (this);

            if (! RuntimePermissions::isGranted (RuntimePermissions::readExternalStorage))
            {
                RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                             [safeThis] (bool granted) mutable
                                             {
                                                 if (granted)
                                                     safeThis->buttonClicked (&safeThis->chooseFileButton);
                                             });
                return;
            }

            if (FileChooser::isPlatformDialogAvailable())
            {
                fileChooser.reset (new FileChooser ("Select an audio file...", File(), "*.wav;*.mp3;*.aif"));

                fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                          [safeThis] (const FileChooser& fc) mutable
                                          {
                                              if (safeThis != nullptr && fc.getURLResults().size() > 0)
                                              {
                                                  auto u = fc.getURLResult();

                                                  safeThis->showAudioResource (static_cast<URL&&> (u));
                                              }

                                              safeThis->fileChooser = nullptr;
                                          }, nullptr);
            }
            else
            {
                NativeMessageBox::showMessageBoxAsync (AlertWindow::WarningIcon, "Enable Code Signing",
                                                       "You need to enable code-signing for your iOS project and enable \"iCloud Documents\" "
                                                       "permissions to be able to open audio files on your iDevice. See: "
                                                       "https://forum.juce.com/t/native-ios-android-file-choosers");
            }
        }
    }
   #else
    void selectionChanged() override
    {
        showAudioResource (URL (fileTreeComp.getSelectedFile()));
    }

    void fileClicked (const File&, const MouseEvent&) override          {}
    void fileDoubleClicked (const File&) override                       {}
    void browserRootChanged (const File&) override                      {}
   #endif

    void changeListenerCallback (ChangeBroadcaster* source) override
    {
        if (source == thumbnail.get())
            showAudioResource (URL (thumbnail->getLastDroppedFile()));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlaybackDemo)
};
