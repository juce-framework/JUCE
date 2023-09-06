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

#include <emscripten/webaudio.h>

#define JUCE_WASM_LOG(a) Logger::writeToLog(a);

namespace juce {
    uint8_t audioThreadStack[4096];
    void onAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
    void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData);
    EM_BOOL audioCallback(int numInputs, const AudioSampleFrame *inputs,
                          int numOutputs, AudioSampleFrame *outputs,
                          int numParams, const AudioParamFrame *params,
                          void *userData);

    EM_BOOL OnCanvasClick(int eventType, const EmscriptenMouseEvent *mouseEvent, void *userData)
    {
        std::cout << "Resume" << std::endl;
        EMSCRIPTEN_WEBAUDIO_T audioContext = (EMSCRIPTEN_WEBAUDIO_T)userData;
        if (emscripten_audio_context_state(audioContext) != AUDIO_CONTEXT_STATE_RUNNING) {
            emscripten_resume_audio_context_sync(audioContext);
        }
        return EM_FALSE;
    }
class WASMAudioIODevice;
struct  WASMAudioIODeviceCallback {
    AudioIODeviceCallback* callback;
    WASMAudioIODevice* device;
    CriticalSection lock;
};
//TODO:wasm_support https://emscripten.org/docs/api_reference/wasm_audio_worklets.html
class WASMAudioIODevice: public AudioIODevice,public Thread {
public:
    WASMAudioIODevice(size_t threadStackSize, const String &deviceName,
                      const String &typeName) : Thread("Web Audio", threadStackSize),
                                                AudioIODevice(deviceName, typeName) {}
    StringArray getOutputChannelNames() override {
        return StringArray("Default Output");
    }

    StringArray getInputChannelNames() override {
        return StringArray("Default Input");
    }
    Array<double> getAvailableSampleRates() override {
        return Array<double>(44100,48000);
    }

    Array<int> getAvailableBufferSizes() override {
        return Array<int>(128);
    }
    int getDefaultBufferSize() override {
        return 128;
    }
    String open(const BigInteger &inputChannels, const BigInteger &outputChannels, double sampleRate,
                int bufferSizeSamples) override {
        if(bufferSizeSamples != getDefaultBufferSize()) {
            JUCE_WASM_LOG("bufferSizeSamples != 128");
            return "";
        }
        options = new EmscriptenWebAudioCreateAttributes();
        options->latencyHint = "playback";
        options->sampleRate = static_cast<uint32_t>(sampleRate);
        context = emscripten_create_audio_context(options);
        return String();
    }
    void close() override {
       emscripten_destroy_audio_context(context);
    }
    bool isOpen() override {
        return emscripten_audio_context_state(context) != AUDIO_CONTEXT_STATE_CLOSED;
    }
    void start(AudioIODeviceCallback *callback) override {
        lastCallback = new WASMAudioIODeviceCallback();
        lastCallback->device = this;
        lastCallback->callback = callback;
        emscripten_start_wasm_audio_worklet_thread_async(context,audioThreadStack,sizeof(audioThreadStack),&onAudioThreadInitialized,lastCallback);
    }
    void stop() override {
        emscripten_destroy_audio_context(context);
    }

    bool isPlaying() override {
        return emscripten_audio_context_state(context) == AUDIO_CONTEXT_STATE_RUNNING;
    }

    String getLastError() override {
        return String();
    }

    int getCurrentBufferSizeSamples() override {
        return 128;
    }

    double getCurrentSampleRate() override {
        return 44100;
    }

    int getCurrentBitDepth() override {
        return 24;
    }

    BigInteger getActiveOutputChannels() const override {
        return BigInteger(0);
    }

    BigInteger getActiveInputChannels() const override {
        return BigInteger(10);
    }

    int getOutputLatencyInSamples() override {
        return 0;
    }

    int getInputLatencyInSamples() override {
        return 0;
    }
    void run() override {
        std::cout << "Starting Run" << std::endl;
    }
private:
    EMSCRIPTEN_WEBAUDIO_T context;
    EmscriptenWebAudioCreateAttributes* options;
    WASMAudioIODeviceCallback *lastCallback;
};

    void onAudioThreadInitialized(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData) {
        std::cout << "onAudioThreadInitialized " << success << std::endl;
        if(!success) {
            std::cout << "Can not init audio thread" << std::endl;
            return;
        }
        WebAudioWorkletProcessorCreateOptions opts = {
                .name = "device",
        };
        emscripten_create_wasm_audio_worklet_processor_async(audioContext, &opts, &AudioWorkletProcessorCreated, userData);

    }

    void AudioWorkletProcessorCreated(EMSCRIPTEN_WEBAUDIO_T audioContext, EM_BOOL success, void *userData)
    {
        std::cout << "AudioWorkletProcessorCreated " << std::endl;
        int outputChannelCounts[1] = { 1 };
        EmscriptenAudioWorkletNodeCreateOptions options = {
                .numberOfInputs = 1,
                .numberOfOutputs = 1,
                .outputChannelCounts = outputChannelCounts
        };
        EMSCRIPTEN_AUDIO_WORKLET_NODE_T wasmAudioWorklet = emscripten_create_wasm_audio_worklet_node(audioContext,"device", &options, &audioCallback, userData);
        EM_ASM({emscriptenGetAudioObject($0).connect(emscriptenGetAudioObject($1).destination)},
               wasmAudioWorklet, audioContext);
    }

    EM_BOOL audioCallback(int numInputs, const AudioSampleFrame *inputs,
                          int numOutputs, AudioSampleFrame *outputs,
                          int numParams, const AudioParamFrame *params,
                          void *userData) {

        juce::AudioSampleBuffer inputBuffer(2,128);
        juce::AudioSampleBuffer outputBuffer(2,128);
        inputBuffer.clear();
        outputBuffer.clear();
        WASMAudioIODeviceCallback* current = (WASMAudioIODeviceCallback*)userData;
        ScopedLock  sl(current->lock);
        current->callback->audioDeviceIOCallbackWithContext(
                    inputBuffer.getArrayOfWritePointers(),
                inputBuffer.getNumChannels(),
                outputBuffer.getArrayOfWritePointers(),
                outputBuffer.getNumChannels(),
                    128,
                    {}
                );
        auto writeableChannel =std::min(2,outputs[0].numberOfChannels);
        auto readPointers = outputBuffer.getArrayOfReadPointers();
        for(int i =0;i < writeableChannel * 128;i++) {
       outputs[0].data[i] = readPointers[i / 128][i];
        }
        return EM_TRUE;
    }

class WasmAudioIODeviceType: public AudioIODeviceType {
    public:
        WasmAudioIODeviceType() : AudioIODeviceType("Web Audio Default Device") {

        }
        void scanForDevices() override {

        }

        StringArray getDeviceNames(bool wantInputNames) const override {
            StringArray names;
            names.add("Web Audio Default Device");
            return names;
        }

        int getDefaultDeviceIndex(bool forInput) const override {
            return 0;
        }

        int getIndexOfDevice(AudioIODevice *device, bool asInput) const override {
            return 0;
        }

        bool hasSeparateInputsAndOutputs() const override {
            return true;
        }

        AudioIODevice *createDevice(const String &outputDeviceName, const String &inputDeviceName) override {
            return new WASMAudioIODevice(sizeof(audioThreadStack),outputDeviceName,inputDeviceName);
        }
    };





}



