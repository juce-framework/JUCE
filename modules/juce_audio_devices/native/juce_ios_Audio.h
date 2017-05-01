/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

class iOSAudioIODeviceType;

class iOSAudioIODevice : public AudioIODevice
{
public:
    //==============================================================================
    String open (const BigInteger&, const BigInteger&, double, int) override;
    void close() override;

    void start (AudioIODeviceCallback*) override;
    void stop() override;

    Array<double> getAvailableSampleRates() override;
    Array<int> getAvailableBufferSizes() override;
    bool setAudioPreprocessingEnabled (bool) override;

    //==============================================================================
    bool isPlaying() override                             { return isRunning && callback != nullptr; }
    bool isOpen() override                                { return isRunning; }
    String getLastError() override                        { return lastError; };

    //==============================================================================
    StringArray getOutputChannelNames() override          { return { "Left", "Right" }; }
    StringArray getInputChannelNames() override           { return audioInputIsAvailable ? getOutputChannelNames() : StringArray(); }
    int getDefaultBufferSize() override                   { return defaultBufferSize; }
    int getCurrentBufferSizeSamples() override            { return actualBufferSize; }
    double getCurrentSampleRate() override                { return sampleRate; }
    int getCurrentBitDepth() override                     { return 16; }
    BigInteger getActiveOutputChannels() const override   { return activeOutputChans; }
    BigInteger getActiveInputChannels() const override    { return activeInputChans; }
    int getOutputLatencyInSamples() override;
    int getInputLatencyInSamples() override;

    //==============================================================================
    void handleStatusChange (bool enabled, const char* reason);
    void handleRouteChange (const char* reason);

    //==============================================================================
    virtual void setMidiMessageCollector (MidiMessageCollector* collector)     { messageCollector = collector; }
    virtual AudioPlayHead* getAudioPlayHead() const;

    //==============================================================================
    virtual bool isInterAppAudioConnected() const                              { return interAppAudioConnected; }
   #if JUCE_MODULE_AVAILABLE_juce_graphics
    virtual Image getIcon (int size);
   #endif
    virtual void switchApplication();
private:
    //==============================================================================
    void updateSampleRateAndAudioInput();

    //==============================================================================
    friend class iOSAudioIODeviceType;
    iOSAudioIODevice (const String& deviceName);

    //==============================================================================
    const int defaultBufferSize;
    double sampleRate;
    int numInputChannels, numOutputChannels;
    int preferredBufferSize, actualBufferSize;
    bool isRunning;
    String lastError;

    bool audioInputIsAvailable, interAppAudioConnected;
    BigInteger activeOutputChans, activeInputChans;

    AudioIODeviceCallback* callback;
    MidiMessageCollector* messageCollector;

    class Pimpl;
    friend class Pimpl;
    ScopedPointer<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE (iOSAudioIODevice)
};
