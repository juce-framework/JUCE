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

#pragma once

#include "DSPDemo.h"
#include "MainComponent.h"

//==============================================================================
class DSPSamplesApplication  : public JUCEApplication,
                               private TimeSliceThread,
                               private Value::Listener,
                               private ChangeListener
{
public:
    //==============================================================================
    DSPSamplesApplication();

    const String getApplicationName() override       { return ProjectInfo::projectName; }
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }

    //==============================================================================
    void initialise (const String&) override;
    void shutdown() override;

    //==============================================================================
    static DSPSamplesApplication& getApp();

    //==============================================================================
    bool loadFile (const File&);
    void togglePlay();
    void stop();
    void init();
    void play();
    void setLooping (bool);

    //==============================================================================
    void setCurrentDemo (int index, bool force = false);
    int getCurrentDemoIndex() const                                         { return demoIndex; }
    const std::vector<DSPDemoParameterBase*>& getCurrentDemoParameters()    { return currentDemo->getParameters(); }

    AudioDeviceManager& getDeviceManager()        { return audioDeviceManager; }
    AudioFormatManager& getFormatManager()        { return formatManager; }
    AudioTransportSource* getTransportSource()    { return transportSource; }

    Value& getPlayState()                         { return playState; }
    Value& getLoopState()                         { return loopState; }

    //==============================================================================
    struct MainWindow  : public DocumentWindow
    {
        MainWindow (String name)
            : DocumentWindow (name,
                              Desktop::getInstance().getDefaultLookAndFeel()
                                                    .findColour (ResizableWindow::backgroundColourId),
                              DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (mainComponent = new MainContentComponent(), true);

           #if JUCE_ANDROID || JUCE_IOS
            setFullScreen (true);
           #else
            centreWithSize (getWidth(), getHeight());
            setResizable (true, false);
            setResizeLimits (500, 400, 32000, 32000);
           #endif
            setVisible (true);
        }

        void closeButtonPressed() override
        {
            JUCEApplication::getInstance()->systemRequestedQuit();
        }

        void setTransportSource (AudioTransportSource* source)
        {
            mainComponent->getThumbnailComponent().setTransportSource (source);
        }

        void initParameters()
        {
            mainComponent->initParameters();
        }

    private:
        ScopedPointer<MainContentComponent> mainComponent;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
    };


private:
    //==============================================================================
    void valueChanged (Value&) override;
    void changeListenerCallback (ChangeBroadcaster*) override;

    //==============================================================================
    AudioDeviceManager audioDeviceManager;
    AudioFormatManager formatManager;
    Value playState { var (false) };
    Value loopState { var (false) };

    double currentSampleRate = 44100.0;
    uint32 currentBlockSize = 512;
    uint32 currentNumChannels = 2;

    ScopedPointer<AudioFormatReader> reader;
    ScopedPointer<AudioFormatReaderSource> readerSource;
    ScopedPointer<AudioTransportSource> transportSource;
    ScopedPointer<DSPDemoBase> currentDemo;

    AudioSourcePlayer audioSourcePlayer;

    ScopedPointer<MainWindow> mainWindow;

    int demoIndex = -1;

    AudioBuffer<float> fileReadBuffer;
};
