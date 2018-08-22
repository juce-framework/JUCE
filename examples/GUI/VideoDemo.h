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

 name:             VideoDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Plays video files.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra, juce_video
 exporters:        xcode_mac, vs2017, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        VideoDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

#if JUCE_MAC || JUCE_WINDOWS
//==============================================================================
// so that we can easily have two video windows each with a file browser, wrap this up as a class..
class MovieComponentWithFileBrowser  : public Component,
                                       public DragAndDropTarget,
                                       private FilenameComponentListener
{
public:
    MovieComponentWithFileBrowser()
        : videoComp (true)
    {
        addAndMakeVisible (videoComp);

        addAndMakeVisible (fileChooser);
        fileChooser.addListener (this);
        fileChooser.setBrowseButtonText ("browse");
    }

    void setFile (const File& file)
    {
        fileChooser.setCurrentFile (file, true);
    }

    void paintOverChildren (Graphics& g) override
    {
        if (isDragOver)
        {
            g.setColour (Colours::red);
            g.drawRect (fileChooser.getBounds(), 2);
        }
    }

    void resized() override
    {
        videoComp.setBounds (getLocalBounds().reduced (10));
    }

    bool isInterestedInDragSource (const SourceDetails&) override   { return true; }

    void itemDragEnter (const SourceDetails&) override
    {
        isDragOver = true;
        repaint();
    }

    void itemDragExit (const SourceDetails&) override
    {
        isDragOver = false;
        repaint();
    }

    void itemDropped (const SourceDetails& dragSourceDetails) override
    {
        setFile (dragSourceDetails.description.toString());
        isDragOver = false;
        repaint();
    }

private:
    VideoComponent videoComp;

    bool isDragOver = false;
    FilenameComponent fileChooser  { "movie", {}, true, false, false, "*", {}, "(choose a video file to play)"};

    void filenameComponentChanged (FilenameComponent*) override
    {
        auto url = URL (fileChooser.getCurrentFile());

        // this is called when the user changes the filename in the file chooser box
        auto result = videoComp.load (url);
        videoLoadingFinished (url, result);
    }

    void videoLoadingFinished (const URL& url, Result result)
    {
        ignoreUnused (url);

        if (result.wasOk())
        {
            // loaded the file ok, so let's start it playing..

            videoComp.play();
            resized(); // update to reflect the video's aspect ratio
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Couldn't load the file!",
                                              result.getErrorMessage());
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MovieComponentWithFileBrowser)
};

//==============================================================================
class VideoDemo   : public Component,
                    public DragAndDropContainer,
                    private FileBrowserListener
{
public:
    VideoDemo()
    {
        setOpaque (true);

        movieList.setDirectory (File::getSpecialLocation (File::userMoviesDirectory), true, true);
        directoryThread.startThread (1);

        fileTree.addListener (this);
        fileTree.setColour (FileTreeComponent::backgroundColourId, Colours::lightgrey.withAlpha (0.6f));
        addAndMakeVisible (fileTree);

        addAndMakeVisible (resizerBar);

        loadLeftButton .onClick = [this] { movieCompLeft .setFile (fileTree.getSelectedFile (0)); };
        loadRightButton.onClick = [this] { movieCompRight.setFile (fileTree.getSelectedFile (0)); };

        addAndMakeVisible (loadLeftButton);
        addAndMakeVisible (loadRightButton);

        addAndMakeVisible (movieCompLeft);
        addAndMakeVisible (movieCompRight);

        // we have to set up our StretchableLayoutManager so it know the limits and preferred sizes of it's contents
        stretchableManager.setItemLayout (0,            // for the fileTree
                                          -0.1, -0.9,   // must be between 50 pixels and 90% of the available space
                                          -0.3);        // and its preferred size is 30% of the total available space

        stretchableManager.setItemLayout (1,            // for the resize bar
                                          5, 5, 5);     // hard limit to 5 pixels

        stretchableManager.setItemLayout (2,            // for the movie components
                                          -0.1, -0.9,   // size must be between 50 pixels and 90% of the available space
                                          -0.7);        // and its preferred size is 70% of the total available space

        setSize (500, 500);
    }

    ~VideoDemo()
    {
        fileTree.removeListener (this);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        // make a list of two of our child components that we want to reposition
        Component* comps[] = { &fileTree, &resizerBar, nullptr };

        // this will position the 3 components, one above the other, to fit
        // vertically into the rectangle provided.
        stretchableManager.layOutComponents (comps, 3,
                                             0, 0, getWidth(), getHeight(),
                                             true, true);

        // now position out two video components in the space that's left
        auto area = getLocalBounds().removeFromBottom (getHeight() - resizerBar.getBottom());

        {
            auto buttonArea = area.removeFromTop (30);
            loadLeftButton .setBounds (buttonArea.removeFromLeft (buttonArea.getWidth() / 2).reduced (5));
            loadRightButton.setBounds (buttonArea.reduced (5));
        }

        movieCompLeft .setBounds (area.removeFromLeft (area.getWidth() / 2).reduced (5));
        movieCompRight.setBounds (area.reduced (5));
    }

private:
    std::unique_ptr<FileChooser> fileChooser;
    WildcardFileFilter moviesWildcardFilter  { "*", "*", "Movies File Filter" };
    TimeSliceThread directoryThread          { "Movie File Scanner Thread" };
    DirectoryContentsList movieList          { &moviesWildcardFilter, directoryThread };
    FileTreeComponent fileTree               { movieList };

    StretchableLayoutManager stretchableManager;
    StretchableLayoutResizerBar resizerBar   { &stretchableManager, 1, false };

    TextButton loadLeftButton   { "Load Left" },
               loadRightButton  { "Load Right" };
    MovieComponentWithFileBrowser movieCompLeft, movieCompRight;

    void selectionChanged() override
    {
        // we're just going to update the drag description of out tree so that rows can be dragged onto the file players
        fileTree.setDragAndDropDescription (fileTree.getSelectedFile().getFullPathName());
    }

    void fileClicked (const File&, const MouseEvent&) override {}
    void fileDoubleClicked (const File&)              override {}
    void browserRootChanged (const File&)             override {}

    void selectVideoFile()
    {
        fileChooser.reset (new FileChooser ("Choose a file to open...", File::getCurrentWorkingDirectory(),
                                            "*", false));

        fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                  [this] (const FileChooser& chooser)
                                  {
                                      String chosen;
                                      auto results = chooser.getURLResults();

                                      // TODO: support non local files too
                                      if (results.size() > 0)
                                          movieCompLeft.setFile (results[0].getLocalFile());
                                  });
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
};
#elif JUCE_IOS || JUCE_ANDROID
//==============================================================================
class VideoDemo   : public Component,
                    private Timer
{
public:
    VideoDemo()
        : videoCompWithNativeControls (true),
          videoCompNoNativeControls (false)
    {
        loadLocalButton  .onClick = [this] { selectVideoFile(); };
        loadUrlButton    .onClick = [this] { showVideoUrlPrompt(); };
        seekToStartButton.onClick = [this] { seekVideoToStart(); };
        playButton       .onClick = [this] { playVideo(); };
        pauseButton      .onClick = [this] { pauseVideo(); };
        unloadButton     .onClick = [this] { unloadVideoFile(); };

        volumeLabel         .setColour (Label::textColourId, Colours::white);
        currentPositionLabel.setColour (Label::textColourId, Colours::white);

        volumeLabel         .setJustificationType (Justification::right);
        currentPositionLabel.setJustificationType (Justification::right);

        volumeSlider  .setRange (0.0, 1.0);
        positionSlider.setRange (0.0, 1.0);

        volumeSlider  .setSliderSnapsToMousePosition (false);
        positionSlider.setSliderSnapsToMousePosition (false);

        volumeSlider.setSkewFactor (1.5);
        volumeSlider.setValue (1.0, dontSendNotification);
       #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
        curVideoComp->onGlobalMediaVolumeChanged = [this]() { volumeSlider.setValue (curVideoComp->getAudioVolume(), dontSendNotification); };
       #endif

        volumeSlider  .onValueChange = [this]() { curVideoComp->setAudioVolume ((float) volumeSlider.getValue()); };
        positionSlider.onValueChange = [this]() { seekVideoToNormalisedPosition (positionSlider.getValue()); };

        positionSlider.onDragStart = [this]()
                                     {
                                         positionSliderDragging = true;
                                         wasPlayingBeforeDragStart = curVideoComp->isPlaying();

                                         if (wasPlayingBeforeDragStart)
                                             curVideoComp->stop();
                                     };

        positionSlider.onDragEnd   = [this]()
                                     {
                                         if (wasPlayingBeforeDragStart)
                                             curVideoComp->play();

                                         wasPlayingBeforeDragStart = false;

                                         // Ensure the slider does not temporarily jump back on consecutive timer callback.
                                         Timer::callAfterDelay (500, [this]() { positionSliderDragging = false; });
                                     };

        playSpeedComboBox.addItem ("25%", 25);
        playSpeedComboBox.addItem ("50%", 50);
        playSpeedComboBox.addItem ("100%", 100);
        playSpeedComboBox.addItem ("200%", 200);
        playSpeedComboBox.addItem ("400%", 400);
        playSpeedComboBox.setSelectedId (100, dontSendNotification);
        playSpeedComboBox.onChange = [this]() { curVideoComp->setPlaySpeed (playSpeedComboBox.getSelectedId() / 100.0); };

        setTransportControlsEnabled (false);

        addAndMakeVisible (loadLocalButton);
        addAndMakeVisible (loadUrlButton);
        addAndMakeVisible (volumeLabel);
        addAndMakeVisible (volumeSlider);
        addChildComponent (videoCompWithNativeControls);
        addChildComponent (videoCompNoNativeControls);
        addAndMakeVisible (positionSlider);
        addAndMakeVisible (currentPositionLabel);

        addAndMakeVisible (playSpeedComboBox);
        addAndMakeVisible (seekToStartButton);
        addAndMakeVisible (playButton);
        addAndMakeVisible (unloadButton);
        addChildComponent (pauseButton);

        setSize (500, 500);

        RuntimePermissions::request (RuntimePermissions::readExternalStorage,
                                     [] (bool granted)
                                     {
                                         if (! granted)
                                         {
                                             AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                                               "Permissions warning",
                                                                               "External storage access permission not granted, some files"
                                                                               " may be inaccessible.");
                                         }
                                     });

        setPortraitOrientationEnabled (true);
    }

    ~VideoDemo()
    {
        curVideoComp->onPlaybackStarted = nullptr;
        curVideoComp->onPlaybackStopped = nullptr;
        curVideoComp->onErrorOccurred   = nullptr;
        curVideoComp->onGlobalMediaVolumeChanged = nullptr;

        setPortraitOrientationEnabled (false);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds();

        int marginSize = 5;
        int buttonHeight = 20;

        area.reduce (0, marginSize);

        auto topArea = area.removeFromTop (buttonHeight);
        loadLocalButton.setBounds (topArea.removeFromLeft (topArea.getWidth() / 6));
        loadUrlButton.setBounds (topArea.removeFromLeft (loadLocalButton.getWidth()));
        volumeLabel.setBounds (topArea.removeFromLeft (loadLocalButton.getWidth()));
        volumeSlider.setBounds (topArea.reduced (10, 0));

        auto transportArea = area.removeFromBottom (buttonHeight);
        auto positionArea  = area.removeFromBottom (buttonHeight).reduced (marginSize, 0);

        playSpeedComboBox.setBounds (transportArea.removeFromLeft (jmax (50, transportArea.getWidth() / 5)));

        auto controlWidth = transportArea.getWidth() / 3;

        currentPositionLabel.setBounds (positionArea.removeFromRight (jmax (150, controlWidth)));
        positionSlider.setBounds (positionArea);

        seekToStartButton.setBounds (transportArea.removeFromLeft (controlWidth));
        playButton       .setBounds (transportArea.removeFromLeft (controlWidth));
        unloadButton     .setBounds (transportArea.removeFromLeft (controlWidth));
        pauseButton.setBounds (playButton.getBounds());

        area.removeFromTop (marginSize);
        area.removeFromBottom (marginSize);

        videoCompWithNativeControls.setBounds (area);
        videoCompNoNativeControls.setBounds (area);

        if (positionSlider.getWidth() > 0)
            positionSlider.setMouseDragSensitivity (positionSlider.getWidth());
    }

private:
    TextButton loadLocalButton { "Load Local" };
    TextButton loadUrlButton { "Load URL" };
    Label volumeLabel { "volumeLabel", "Vol:" };
    Slider volumeSlider { Slider::LinearHorizontal, Slider::NoTextBox };

    VideoComponent videoCompWithNativeControls;
    VideoComponent videoCompNoNativeControls;
   #if JUCE_IOS || JUCE_MAC
    VideoComponent* curVideoComp = &videoCompWithNativeControls;
   #else
    VideoComponent* curVideoComp = &videoCompNoNativeControls;
   #endif
    bool isFirstSetup = true;

    Slider positionSlider { Slider::LinearHorizontal, Slider::NoTextBox };
    bool positionSliderDragging = false;
    bool wasPlayingBeforeDragStart = false;

    Label currentPositionLabel { "currentPositionLabel", "-:- / -:-" };

    ComboBox playSpeedComboBox { "playSpeedComboBox" };
    TextButton seekToStartButton { "|<" };
    TextButton playButton { "Play" };
    TextButton pauseButton { "Pause" };
    TextButton unloadButton { "Unload" };

    std::unique_ptr<FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VideoDemo)
    JUCE_DECLARE_WEAK_REFERENCEABLE (VideoDemo)

    //==============================================================================
    void setPortraitOrientationEnabled (bool shouldBeEnabled)
    {
        auto allowedOrientations = Desktop::getInstance().getOrientationsEnabled();

        if (shouldBeEnabled)
            allowedOrientations |= Desktop::upright;
        else
            allowedOrientations &= ~Desktop::upright;

        Desktop::getInstance().setOrientationsEnabled (allowedOrientations);
    }

    void setTransportControlsEnabled (bool shouldBeEnabled)
    {
        positionSlider   .setEnabled (shouldBeEnabled);
        playSpeedComboBox.setEnabled (shouldBeEnabled);
        seekToStartButton.setEnabled (shouldBeEnabled);
        playButton       .setEnabled (shouldBeEnabled);
        unloadButton     .setEnabled (shouldBeEnabled);
        pauseButton      .setEnabled (shouldBeEnabled);
    }

    void selectVideoFile()
    {
        fileChooser.reset (new FileChooser ("Choose a video file to open...", File::getCurrentWorkingDirectory(),
                                            "*", true));

        fileChooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectFiles,
                                  [this] (const FileChooser& chooser)
                                  {
                                      auto results = chooser.getURLResults();

                                      if (results.size() > 0)
                                          loadVideo (results[0]);
                                  });
    }

    void loadVideo (const URL& url)
    {
        unloadVideoFile();

       #if JUCE_IOS || JUCE_MAC
        askIfUseNativeControls (url);
       #else
        loadUrl (url);
        setupVideoComp (false);
       #endif
    }

    void askIfUseNativeControls (const URL& url)
    {
        auto* aw = new AlertWindow ("Choose viewer type", {}, AlertWindow::NoIcon);

        aw->addButton ("Yes", 1, KeyPress (KeyPress::returnKey));
        aw->addButton ("No", 0, KeyPress (KeyPress::escapeKey));
        aw->addTextBlock ("Do you want to use the viewer with native controls?");

        auto callback = ModalCallbackFunction::forComponent (videoViewerTypeChosen, this, url);
        aw->enterModalState (true, callback, true);
    }

    static void videoViewerTypeChosen (int result, VideoDemo* owner, URL url)
    {
        if (owner != nullptr)
        {
            owner->setupVideoComp (result != 0);
            owner->loadUrl (url);
        }
    }

    void setupVideoComp (bool useNativeViewerWithNativeControls)
    {
        auto* oldVideoComp = curVideoComp;

        if (useNativeViewerWithNativeControls)
            curVideoComp = &videoCompWithNativeControls;
        else
            curVideoComp = &videoCompNoNativeControls;

        if (isFirstSetup || oldVideoComp != curVideoComp)
        {
            oldVideoComp->onPlaybackStarted = nullptr;
            oldVideoComp->onPlaybackStopped = nullptr;
            oldVideoComp->onErrorOccurred   = nullptr;
            oldVideoComp->setVisible (false);

            curVideoComp->onPlaybackStarted = [this]() { processPlaybackStarted(); };
            curVideoComp->onPlaybackStopped = [this]() { processPlaybackPaused(); };
            curVideoComp->onErrorOccurred   = [this](const String& errorMessage) { errorOccurred (errorMessage); };
            curVideoComp->setVisible (true);

           #if JUCE_SYNC_VIDEO_VOLUME_WITH_OS_MEDIA_VOLUME
            oldVideoComp->onGlobalMediaVolumeChanged = nullptr;
            curVideoComp->onGlobalMediaVolumeChanged = [this]() { volumeSlider.setValue (curVideoComp->getAudioVolume(), dontSendNotification); };
           #endif
        }

        isFirstSetup = false;
    }

    void loadUrl (const URL& url)
    {
        curVideoComp->loadAsync (url, [this] (const URL& u, Result r) { videoLoadingFinished (u, r); });
    }

    void showVideoUrlPrompt()
    {
        auto* aw = new AlertWindow ("Enter URL for video to load", {}, AlertWindow::NoIcon);

        aw->addButton ("OK", 1, KeyPress (KeyPress::returnKey));
        aw->addButton ("Cancel", 0, KeyPress (KeyPress::escapeKey));
        aw->addTextEditor ("videoUrlTextEditor", "https://www.rmp-streaming.com/media/bbb-360p.mp4");

        auto callback = ModalCallbackFunction::forComponent (videoUrlPromptClosed, this, Component::SafePointer<AlertWindow> (aw));
        aw->enterModalState (true, callback, true);
    }

    static void videoUrlPromptClosed (int result, VideoDemo* owner, Component::SafePointer<AlertWindow> aw)
    {
        if (result != 0 && owner != nullptr && aw != nullptr)
        {
            auto url = aw->getTextEditorContents ("videoUrlTextEditor");

            if (url.isNotEmpty())
                owner->loadVideo (url);
        }
    }

    void videoLoadingFinished (const URL& url, Result result)
    {
        ignoreUnused (url);

        if (result.wasOk())
        {
            resized(); // update to reflect the video's aspect ratio

            setTransportControlsEnabled (true);

            currentPositionLabel.setText (getPositionString (0.0, curVideoComp->getVideoDuration()), sendNotification);
            positionSlider.setValue (0.0, dontSendNotification);
            playSpeedComboBox.setSelectedId (100, dontSendNotification);
        }
        else
        {
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                              "Couldn't load the file!",
                                              result.getErrorMessage());
        }
    }

    static String getPositionString (double playPositionSeconds, double durationSeconds)
    {
        auto positionMs = static_cast<int> (1000 * playPositionSeconds);
        int posMinutes = positionMs / 60000;
        int posSeconds = (positionMs % 60000) / 1000;
        int posMillis = positionMs % 1000;

        auto totalMs = static_cast<int> (1000 * durationSeconds);
        int totMinutes = totalMs / 60000;
        int totSeconds = (totalMs % 60000) / 1000;
        int totMillis = totalMs % 1000;

        return String::formatted ("%02d:%02d:%03d / %02d:%02d:%03d",
                                  posMinutes, posSeconds, posMillis,
                                  totMinutes, totSeconds, totMillis);
    }

    void updatePositionSliderAndLabel()
    {
        auto position = curVideoComp->getPlayPosition();
        auto duration = curVideoComp->getVideoDuration();

        currentPositionLabel.setText (getPositionString (position, duration), sendNotification);

        if (! positionSliderDragging)
            positionSlider.setValue (duration != 0 ? (position / duration) : 0.0, dontSendNotification);
    }

    void seekVideoToStart()
    {
        seekVideoToNormalisedPosition (0.0);
    }

    void seekVideoToNormalisedPosition (double normalisedPos)
    {
        normalisedPos = jlimit (0.0, 1.0, normalisedPos);

        auto duration = curVideoComp->getVideoDuration();
        auto newPos = jlimit (0.0, duration, duration * normalisedPos);

        curVideoComp->setPlayPosition (newPos);
        currentPositionLabel.setText (getPositionString (newPos, curVideoComp->getVideoDuration()), sendNotification);
        positionSlider.setValue (normalisedPos, dontSendNotification);
    }

    void playVideo()
    {
        curVideoComp->play();
    }

    void processPlaybackStarted()
    {
        playButton.setVisible (false);
        pauseButton.setVisible (true);

        startTimer (20);
    }

    void pauseVideo()
    {
        curVideoComp->stop();
    }

    void processPlaybackPaused()
    {
        // On seeking to a new pos, the playback may be temporarily paused.
        if (positionSliderDragging)
            return;

        pauseButton.setVisible (false);
        playButton.setVisible (true);
    }

    void errorOccurred (const String& errorMessage)
    {
        AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon,
                                          "An error has occurred",
                                          errorMessage + ", video will be unloaded.");

        unloadVideoFile();
    }

    void unloadVideoFile()
    {
        curVideoComp->closeVideo();

        setTransportControlsEnabled (false);
        stopTimer();

        pauseButton.setVisible (false);
        playButton.setVisible (true);

        currentPositionLabel.setText ("-:- / -:-", sendNotification);
        positionSlider.setValue (0.0, dontSendNotification);
    }

    void timerCallback() override
    {
        updatePositionSliderAndLabel();
    }
};
#elif JUCE_LINUX
 #error "This demo is not supported on Linux!"
#endif
