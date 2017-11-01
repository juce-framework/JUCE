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

#include "Main.h"

DSPSamplesApplication::DSPSamplesApplication()
    : TimeSliceThread ("Audio File Reader Thread")
{
    loopState.addListener (this);
}

void DSPSamplesApplication::initialise (const String&)
{
    formatManager.registerBasicFormats();
    audioDeviceManager.addAudioCallback (&audioSourcePlayer);
    audioDeviceManager.initialiseWithDefaultDevices (0, 2);

    setCurrentDemo (0);
    startThread();

    mainWindow = new MainWindow (getApplicationName());
}

void DSPSamplesApplication::shutdown()
{
    signalThreadShouldExit();
    stop();
    audioDeviceManager.removeAudioCallback (&audioSourcePlayer);
    waitForThreadToExit (10000);
    mainWindow.reset();
}

//==============================================================================
DSPSamplesApplication& DSPSamplesApplication::getApp()
{
    auto* app = dynamic_cast<DSPSamplesApplication*> (JUCEApplication::getInstance());
    jassert (app != nullptr);
    return *app;
}

//==============================================================================
bool DSPSamplesApplication::loadFile (const File& fileToPlay)
{
    stop();

    audioSourcePlayer.setSource (nullptr);
    mainWindow->setTransportSource (nullptr);
    transportSource.reset();
    readerSource.reset();

    reader = formatManager.createReaderFor (fileToPlay);

    if (reader != nullptr)
    {
        readerSource = new AudioFormatReaderSource (reader, false);
        readerSource->setLooping (loopState.getValue());

        init();

        return true;
    }

    return false;
}

void DSPSamplesApplication::togglePlay()
{
    if (playState.getValue())
        stop();
    else
        play();
}

void DSPSamplesApplication::stop()
{
    playState = false;

    if (transportSource != nullptr)
    {
        transportSource->stop();
        transportSource->setPosition (0);
    }
}

void DSPSamplesApplication::init()
{
    if (transportSource == nullptr)
    {
        transportSource = new AudioTransportSource();
        transportSource->addChangeListener (this);

        if (readerSource != nullptr)
        {
            if (auto* device = audioDeviceManager.getCurrentAudioDevice())
            {
                transportSource->setSource (readerSource, roundToInt (device->getCurrentSampleRate()), this, reader->sampleRate);

                // tell the main window about this so that it can do the seeking behaviour...
                mainWindow->setTransportSource (transportSource);
            }
        }
    }

    audioSourcePlayer.setSource (nullptr);
    currentDemo.reset();

    if (currentDemo == nullptr)
        if (auto demo = Demo::getList()[demoIndex])
            if (demo->name.isNotEmpty())
                currentDemo = demo->createDemo (*transportSource);

    audioSourcePlayer.setSource (currentDemo);

    if (mainWindow != nullptr)
        mainWindow->initParameters();
}

void DSPSamplesApplication::play()
{
    if (readerSource == nullptr)
        return;

    if (transportSource->getCurrentPosition() >= transportSource->getLengthInSeconds()
         || transportSource->getCurrentPosition() < 0)
        transportSource->setPosition (0);

    transportSource->start();
    playState = true;
}

void DSPSamplesApplication::setLooping (bool shouldLoop)
{
    if (readerSource != nullptr)
        readerSource->setLooping (shouldLoop);
}

void DSPSamplesApplication::changeListenerCallback (ChangeBroadcaster*)
{
    if (playState.getValue() && ! transportSource->isPlaying())
        stop();
}

void DSPSamplesApplication::setCurrentDemo (int index, bool force)
{
    if ((index != demoIndex || force) && isPositiveAndBelow (index, Demo::getList().size()))
    {
        demoIndex = index;
        init();

        if (playState.getValue())
            play();
    }
}

void DSPSamplesApplication::valueChanged (Value& v)
{
    if (readerSource != nullptr)
        readerSource->setLooping (v.getValue());
}

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (DSPSamplesApplication)
