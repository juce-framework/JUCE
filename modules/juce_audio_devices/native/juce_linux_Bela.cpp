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

namespace juce
{

//==============================================================================
class BelaMidiInput
{
public:
    static Array<BelaMidiInput*> midiInputs;

    BelaMidiInput (const String& port, MidiInput* input, MidiInputCallback* callback)
        : midiInput (input), midiPort (port), midiCallback (callback)
    {
        jassert (midiCallback != nullptr);
        midiInputs.add (this);

        buffer.resize (32);
    }

    ~BelaMidiInput()
    {
        stop();
        midiInputs.removeAllInstancesOf (this);
    }

    void start()
    {
        midi.readFrom (midiPort.toRawUTF8());
    }

    void stop()
    {
        midi.enableParser (false);
    }

    void poll()
    {
        size_t receivedBytes = 0;

        for (;;)
        {
            auto data = midi.getInput();

            if (data < 0)
                break;

            buffer[receivedBytes] = (uint8) data;
            receivedBytes++;

            if (receivedBytes == buffer.size())
            {
                pushMidiData (static_cast<int> (receivedBytes));
                receivedBytes = 0;
            }
        }

        if (receivedBytes > 0)
            pushMidiData (receivedBytes);
    }

    static Array<MidiDeviceInfo> getDevices (bool input)
    {
        Array<MidiDeviceInfo> devices;

        for (auto& card : findAllALSACardIDs())
            findMidiDevices (devices, input, card);

        return devices;
    }

    void pushMidiMessage (juce::MidiMessage& message)
    {
        concatenator.pushMidiData (message.getRawData(), message.getRawDataSize(), Time::getMillisecondCounter() * 0.001, midiInput, *midiCallback);
    }

private:
    void pushMidiData (int length)
    {
        concatenator.pushMidiData (buffer.data(), length, Time::getMillisecondCounter() * 0.001, midiInput, *midiCallback);
    }

    std::vector<uint8> buffer;

    static Array<int> findAllALSACardIDs()
    {
        Array<int> cards;
        int card = -1;

        for (;;)
        {
            auto status = snd_card_next (&card);

            if (status != 0 || card < 0)
                break;

            cards.add (card);
        }

        return cards;
    }

    // Adds all midi devices to the devices array of the given input/output type on the given card
    static void findMidiDevices (Array<MidiDeviceInfo>& devices, bool input, int cardNum)
    {
        snd_ctl_t* ctl = nullptr;
        auto status = snd_ctl_open (&ctl, ("hw:" + String (cardNum)).toRawUTF8(), 0);

        if (status < 0)
            return;

        int device = -1;

        for (;;)
        {
            status = snd_ctl_rawmidi_next_device (ctl, &device);

            if (status < 0 || device < 0)
                break;

            snd_rawmidi_info_t* info;
            snd_rawmidi_info_alloca (&info);

            snd_rawmidi_info_set_device (info, device);
            snd_rawmidi_info_set_stream (info, input ? SND_RAWMIDI_STREAM_INPUT
                                                     : SND_RAWMIDI_STREAM_OUTPUT);

            snd_ctl_rawmidi_info (ctl, info);

            auto subCount = snd_rawmidi_info_get_subdevices_count (info);

            for (size_t sub = 0; sub < subCount; ++sub)
            {
                snd_rawmidi_info_set_subdevice (info, sub);

                status = snd_ctl_rawmidi_info (ctl, info);

                if (status == 0)
                {
                    String deviceName ("hw:" + String (cardNum) + "," + String (device) + "," + String (sub));
                    devices.add (MidiDeviceInfo (deviceName, deviceName));
                }
            }
        }

        snd_ctl_close (ctl);
    }

    MidiInput* const midiInput;
    String midiPort;
    MidiInputCallback* const midiCallback;

    Midi midi;
    MidiDataConcatenator concatenator { 512 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BelaMidiInput)
};

Array<BelaMidiInput*> BelaMidiInput::midiInputs;


//==============================================================================
class BelaAudioIODevice   : public AudioIODevice
{
public:
    BelaAudioIODevice()  : AudioIODevice (BelaAudioIODevice::belaTypeName,
                                          BelaAudioIODevice::belaTypeName)
    {
        Bela_defaultSettings (&defaultSettings);

        if (includeAnalogSupport)
            analogInputs.resize (8);
    }

    ~BelaAudioIODevice()
    {
        close();
    }

    //==============================================================================
    StringArray getOutputChannelNames() override           { return { "Out #1", "Out #2" }; }
    StringArray getInputChannelNames() override            { return { "In #1",  "In #2" }; }
    Array<double> getAvailableSampleRates() override       { return { 44100.0 }; }
    Array<int> getAvailableBufferSizes() override          { /* TODO: */ return { getDefaultBufferSize() }; }
    int getDefaultBufferSize() override                    { return defaultSettings.periodSize; }

    //==============================================================================
    String open (const BigInteger& inputChannels,
                 const BigInteger& outputChannels,
                 double sampleRate,
                 int bufferSizeSamples) override
    {
        if (sampleRate != 44100.0 && sampleRate != 0.0)
        {
            lastError = "Bela audio outputs only support 44.1 kHz sample rate";
            return lastError;
        }

        settings = defaultSettings;

        auto numIns = getNumContiguousSetBits (inputChannels);
        auto numOuts = getNumContiguousSetBits (outputChannels);

        settings.useAnalog            = includeAnalogSupport ? 1 : 0;
        settings.useDigital           = 0;
        settings.numAudioInChannels   = numIns;
        settings.numAudioOutChannels  = numOuts;
        settings.detectUnderruns      = 1;
        settings.setup                = setupCallback;
        settings.render               = renderCallback;
        settings.cleanup              = cleanupCallback;
        settings.interleave           = 1;

        if (bufferSizeSamples > 0)
            settings.periodSize = bufferSizeSamples;

        isBelaOpen = false;
        isRunning  = false;
        callback   = nullptr;
        underruns  = 0;

        if (Bela_initAudio (&settings, this) != 0 || ! isBelaOpen)
        {
            lastError = "Bela_initAutio failed";
            return lastError;
        }

        actualNumberOfInputs  = jmin (numIns, actualNumberOfInputs);
        actualNumberOfOutputs = jmin (numOuts, actualNumberOfOutputs);

        audioInBuffer.setSize (actualNumberOfInputs, actualBufferSize);
        channelInBuffer.calloc (actualNumberOfInputs);

        audioOutBuffer.setSize (actualNumberOfOutputs, actualBufferSize);
        channelOutBuffer.calloc (actualNumberOfOutputs);

        return {};
    }

    void close() override
    {
        stop();

        if (isBelaOpen)
        {
            Bela_cleanupAudio();

            isBelaOpen = false;
            callback = nullptr;
            underruns = 0;

            actualBufferSize = 0;
            actualNumberOfInputs = 0;
            actualNumberOfOutputs = 0;

            audioInBuffer.setSize (0, 0);
            channelInBuffer.free();

            audioOutBuffer.setSize (0, 0);
            channelOutBuffer.free();
        }
    }

    bool isOpen() override   { return isBelaOpen; }

    void start (AudioIODeviceCallback* newCallback) override
    {
        if (! isBelaOpen)
            return;

        if (isRunning)
        {
            if (newCallback != callback)
            {
                if (newCallback != nullptr)
                    newCallback->audioDeviceAboutToStart (this);

                {
                    ScopedLock lock (callbackLock);
                    std::swap (callback, newCallback);
                }

                if (newCallback != nullptr)
                    newCallback->audioDeviceStopped();
            }
        }
        else
        {
            audioInBuffer.clear();
            audioOutBuffer.clear();

            callback = newCallback;
            isRunning = (Bela_startAudio() == 0);

            if (callback != nullptr)
            {
                if (isRunning)
                {
                    callback->audioDeviceAboutToStart (this);
                }
                else
                {
                    lastError = "Bela_StartAudio failed";
                    callback->audioDeviceError (lastError);
                }
            }
        }
    }

    void stop() override
    {
        AudioIODeviceCallback* oldCallback = nullptr;

        if (callback != nullptr)
        {
            ScopedLock lock (callbackLock);
            std::swap (callback, oldCallback);
        }

        isRunning = false;
        Bela_stopAudio();

        if (oldCallback != nullptr)
            oldCallback->audioDeviceStopped();
    }

    bool isPlaying() override         { return isRunning; }
    String getLastError() override    { return lastError; }

    //==============================================================================
    int getCurrentBufferSizeSamples() override            { return actualBufferSize; }
    double getCurrentSampleRate() override                { return 44100.0; }
    int getCurrentBitDepth() override                     { return 24; }
    BigInteger getActiveOutputChannels() const override   { BigInteger b; b.setRange (0, actualNumberOfOutputs, true); return b; }
    BigInteger getActiveInputChannels() const override    { BigInteger b; b.setRange (0, actualNumberOfInputs, true);  return b; }
    int getOutputLatencyInSamples() override              { /* TODO */ return 0; }
    int getInputLatencyInSamples() override               { /* TODO */ return 0; }
    int getXRunCount() const noexcept                     { return underruns; }

    //==============================================================================
    static const char* const belaTypeName;

private:

    //==============================================================================
    bool setup (BelaContext& context)
    {
        actualBufferSize      = context.audioFrames;
        actualNumberOfInputs  = context.audioInChannels;
        actualNumberOfOutputs = context.audioOutChannels;
        isBelaOpen = true;
        firstCallback = true;

        ScopedLock lock (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceAboutToStart (this);

        return true;
    }

    void render (BelaContext& context)
    {
        // check for xruns
        calculateXruns (context.audioFramesElapsed, context.audioFrames);

        ScopedLock lock (callbackLock);

        if (includeAnalogSupport)
            updateAnalogInputs (context);

        // Check for and process and midi
        for (auto midiInput : BelaMidiInput::midiInputs)
            midiInput->poll();

        if (callback != nullptr)
        {
            jassert (context.audioFrames <= actualBufferSize);
            auto numSamples = jmin (context.audioFrames, actualBufferSize);
            auto interleaved = ((context.flags & BELA_FLAG_INTERLEAVED) != 0);
            auto numIns  = jmin (actualNumberOfInputs,  (int) context.audioInChannels);
            auto numOuts = jmin (actualNumberOfOutputs, (int) context.audioOutChannels);

            int ch;

            if (interleaved && context.audioInChannels > 1)
            {
                for (ch = 0; ch < numIns; ++ch)
                {
                    using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::NonConst>;
                    using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::Const>;

                    channelInBuffer[ch] = audioInBuffer.getWritePointer (ch);
                    DstSampleType dstData (audioInBuffer.getWritePointer (ch));
                    SrcSampleType srcData (context.audioIn + ch, context.audioInChannels);
                    dstData.convertSamples (srcData, numSamples);
                }
            }
            else
            {
                for (ch = 0; ch < numIns; ++ch)
                    channelInBuffer[ch] = context.audioIn + (ch * numSamples);
            }

            for (; ch < actualNumberOfInputs; ++ch)
            {
                channelInBuffer[ch] = audioInBuffer.getWritePointer(ch);
                zeromem (audioInBuffer.getWritePointer (ch), sizeof (float) * numSamples);
            }

            for (int i = 0; i < actualNumberOfOutputs; ++i)
                channelOutBuffer[i] = ((interleaved && context.audioOutChannels > 1) || static_cast<uint32_t> (i) >= context.audioOutChannels ? audioOutBuffer.getWritePointer (i)
                                                                                                                      : context.audioOut + (i * numSamples));

            callback->audioDeviceIOCallback (channelInBuffer.getData(), actualNumberOfInputs,
                                             channelOutBuffer.getData(), actualNumberOfOutputs,
                                             numSamples);

            if (interleaved && context.audioOutChannels > 1)
            {
                for (int i = 0; i < numOuts; ++i)
                {
                    using DstSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::Interleaved,    AudioData::NonConst>;
                    using SrcSampleType = AudioData::Pointer<AudioData::Float32, AudioData::NativeEndian, AudioData::NonInterleaved, AudioData::Const>;

                    SrcSampleType srcData (channelOutBuffer[i]);
                    DstSampleType dstData (context.audioOut + i, context.audioOutChannels);

                    dstData.convertSamples (srcData, numSamples);
                }
            }
        }
    }

    void cleanup (BelaContext&)
    {
        ScopedLock lock (callbackLock);

        if (callback != nullptr)
            callback->audioDeviceStopped();
    }

    void updateAnalogInputs (BelaContext& context)
    {
        for (size_t i = 0; i < 8; i++)
        {
            auto v = analogRead (&context, 0, static_cast<int> (i));

            if (fabs (analogInputs[i].previousInput - v) > 1.0f/512.0f)
            {
                auto controlValue = int (v * 127.0);

                if (analogInputs[i].previousControlValue != controlValue)
                {
                    // Consider this to have moved
                    analogInputs[i].previousInput        = v;
                    analogInputs[i].previousControlValue = controlValue;

                    auto message = MidiMessage::controllerEvent (1, 16 + i, controlValue);

                    for (auto midiInput : BelaMidiInput::midiInputs)
                        midiInput->pushMidiMessage (message);
                }
            }
        }
    }

    struct AnalogInput
    {
        float previousInput        = 0;
        int   previousControlValue = 0;
    };

    std::vector<AnalogInput> analogInputs;


    //==============================================================================
    uint64_t expectedElapsedAudioSamples = 0;
    int underruns = 0;
    bool firstCallback = false;

    void calculateXruns (uint64_t audioFramesElapsed, uint32_t numSamples)
    {
        if (audioFramesElapsed > expectedElapsedAudioSamples && ! firstCallback)
            ++underruns;

        firstCallback = false;
        expectedElapsedAudioSamples = audioFramesElapsed + numSamples;
    }

    //==============================================================================
    static int getNumContiguousSetBits (const BigInteger& value) noexcept
    {
        int bit = 0;

        while (value[bit])
            ++bit;

        return bit;
    }

    //==============================================================================
    static bool setupCallback   (BelaContext* context, void* userData) noexcept    { return static_cast<BelaAudioIODevice*> (userData)->setup (*context); }
    static void renderCallback  (BelaContext* context, void* userData) noexcept    { static_cast<BelaAudioIODevice*> (userData)->render (*context); }
    static void cleanupCallback (BelaContext* context, void* userData) noexcept    { static_cast<BelaAudioIODevice*> (userData)->cleanup (*context); }

    //==============================================================================
    BelaInitSettings defaultSettings, settings;
    bool isBelaOpen = false, isRunning = false;

    CriticalSection callbackLock;
    AudioIODeviceCallback* callback = nullptr;

    String lastError;
    uint32_t actualBufferSize = 0;
    int actualNumberOfInputs = 0, actualNumberOfOutputs = 0;

    AudioBuffer<float> audioInBuffer, audioOutBuffer;
    HeapBlock<const float*> channelInBuffer;
    HeapBlock<float*> channelOutBuffer;

    bool includeAnalogSupport = true;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BelaAudioIODevice)
};

const char* const BelaAudioIODevice::belaTypeName = "Bela Analog";

//==============================================================================
struct BelaAudioIODeviceType  : public AudioIODeviceType
{
    BelaAudioIODeviceType() : AudioIODeviceType ("Bela") {}

    // TODO: support analog outputs
    StringArray getDeviceNames (bool) const override                       { return StringArray (BelaAudioIODevice::belaTypeName); }
    void scanForDevices() override                                         {}
    int getDefaultDeviceIndex (bool) const override                        { return 0; }
    int getIndexOfDevice (AudioIODevice* device, bool) const override      { return device != nullptr ? 0 : -1; }
    bool hasSeparateInputsAndOutputs() const override                      { return false; }

    AudioIODevice* createDevice (const String& outputName, const String& inputName) override
    {
        if (outputName == BelaAudioIODevice::belaTypeName || inputName == BelaAudioIODevice::belaTypeName)
            return new BelaAudioIODevice();

        return nullptr;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BelaAudioIODeviceType)
};

//==============================================================================
AudioIODeviceType* AudioIODeviceType::createAudioIODeviceType_Bela()
{
    return new BelaAudioIODeviceType();
}

//==============================================================================
MidiInput::MidiInput (const String& deviceName, const String& deviceID)
    : deviceInfo (deviceName, deviceID)
{
}

MidiInput::~MidiInput()   { delete static_cast<BelaMidiInput*> (internal); }
void MidiInput::start()   { static_cast<BelaMidiInput*> (internal)->start(); }
void MidiInput::stop()    { static_cast<BelaMidiInput*> (internal)->stop(); }

Array<MidiDeviceInfo> MidiInput::getAvailableDevices()
{
    return BelaMidiInput::getDevices (true);
}

MidiDeviceInfo MidiInput::getDefaultDevice()
{
    return getAvailableDevices().getFirst();
}

std::unique_ptr<MidiInput> MidiInput::openDevice (const String& deviceIdentifier, MidiInputCallback* callback)
{
    if (deviceIdentifier.isEmpty())
        return {};

    std::unique_ptr<MidiInput> midiInput (new MidiInput (deviceIdentifier, deviceIdentifier));
    midiInput->internal = new BelaMidiInput (deviceIdentifier, midiInput.get(), callback);

    return midiInput;
}

std::unique_ptr<MidiInput> MidiInput::createNewDevice (const String&, MidiInputCallback*)
{
    // N/A on Bela
    jassertfalse;
    return {};
}

StringArray MidiInput::getDevices()
{
    StringArray deviceNames;

    for (auto& d : getAvailableDevices())
        deviceNames.add (d.name);

    return deviceNames;
}

int MidiInput::getDefaultDeviceIndex()
{
    return 0;
}

std::unique_ptr<MidiInput> MidiInput::openDevice (int index, MidiInputCallback* callback)
{
    return openDevice (getAvailableDevices()[index].identifier, callback);
}

//==============================================================================
// TODO: Add Bela MidiOutput support
MidiOutput::~MidiOutput()                                                {}
void MidiOutput::sendMessageNow (const MidiMessage&)                     {}
Array<MidiDeviceInfo> MidiOutput::getAvailableDevices()                  { return {}; }
MidiDeviceInfo MidiOutput::getDefaultDevice()                            { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::openDevice (const String&)       { return {}; }
std::unique_ptr<MidiOutput> MidiOutput::createNewDevice (const String&)  { return {}; }
StringArray MidiOutput::getDevices()                                     { return {}; }
int MidiOutput::getDefaultDeviceIndex()                                  { return 0;}
std::unique_ptr<MidiOutput> MidiOutput::openDevice (int)                 { return {}; }

} // namespace juce
