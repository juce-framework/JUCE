/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
#define UNITY_AUDIO_PLUGIN_API_VERSION 0x010401

#if JUCE_WINDOWS
 #define UNITY_INTERFACE_API __stdcall
 #define UNITY_INTERFACE_EXPORT __declspec (dllexport)
#else
 #define UNITY_INTERFACE_API
 #define UNITY_INTERFACE_EXPORT __attribute__ ((visibility ("default")))
#endif

//==============================================================================
struct UnityAudioEffectState;

typedef int  (UNITY_INTERFACE_API * createCallback)              (UnityAudioEffectState* state);
typedef int  (UNITY_INTERFACE_API * releaseCallback)             (UnityAudioEffectState* state);
typedef int  (UNITY_INTERFACE_API * resetCallback)               (UnityAudioEffectState* state);

typedef int  (UNITY_INTERFACE_API * processCallback)             (UnityAudioEffectState* state, float* inBuffer, float* outBuffer, unsigned int bufferSize,
                                                                  int numInChannels, int numOutChannels);

typedef int  (UNITY_INTERFACE_API * setPositionCallback)         (UnityAudioEffectState* state, unsigned int pos);

typedef int  (UNITY_INTERFACE_API * setFloatParameterCallback)   (UnityAudioEffectState* state, int index, float value);
typedef int  (UNITY_INTERFACE_API * getFloatParameterCallback)   (UnityAudioEffectState* state, int index, float* value, char* valuestr);
typedef int  (UNITY_INTERFACE_API * getFloatBufferCallback)      (UnityAudioEffectState* state, const char* name, float* buffer, int numsamples);

typedef int  (UNITY_INTERFACE_API * distanceAttenuationCallback) (UnityAudioEffectState* state, float distanceIn, float attenuationIn, float* attenuationOut);

typedef void (UNITY_INTERFACE_API * renderCallback)              (int eventId);

//==============================================================================
enum UnityAudioEffectDefinitionFlags
{
    isSideChainTarget = 1,
    isSpatializer = 2,
    isAmbisonicDecoder = 4,
    appliesDistanceAttenuation = 8
};

enum UnityAudioEffectStateFlags
{
    stateIsPlaying = 1,
    stateIsPaused = 2,
    stateIsMuted = 8,
    statIsSideChainTarget = 16
};

enum UnityEventModifiers
{
    shift = 1,
    control = 2,
    alt = 4,
    command = 8,
    numeric = 16,
    capsLock = 32,
    functionKey = 64
};

//==============================================================================
#ifndef DOXYGEN

struct UnityAudioSpatializerData
{
    float                          listenerMatrix[16];
    float                          sourceMatrix[16];
    float                          spatialBlend;
    float                          reverbZoneMix;
    float                          spread;
    float                          stereoPan;
    distanceAttenuationCallback    attenuationCallback;
    float                          minDistance;
    float                          maxDistance;
};

struct UnityAudioAmbisonicData
{
    float                          listenerMatrix[16];
    float                          sourceMatrix[16];
    float                          spatialBlend;
    float                          reverbZoneMix;
    float                          spread;
    float                          stereoPan;
    distanceAttenuationCallback    attenuationCallback;
    int                            ambisonicOutChannels;
    float                          volume;
};

struct UnityAudioEffectState
{
    juce::uint32               structSize;
    juce::uint32               sampleRate;
    juce::uint64               dspCurrentTick;
    juce::uint64               dspPreviousTick;
    float*                     sidechainBuffer;
    void*                      effectData;
    juce::uint32               flags;
    void*                      internal;

    UnityAudioSpatializerData* spatializerData;
    juce::uint32               dspBufferSize;
    juce::uint32               hostAPIVersion;

    UnityAudioAmbisonicData*   ambisonicData;

    template <typename T>
    inline T* getEffectData() const
    {
        jassert (effectData != nullptr);
        jassert (internal != nullptr);

        return (T*) effectData;
    }
};

struct UnityAudioParameterDefinition
{
    char        name[16];
    char        unit[16];
    const char* description;
    float       min;
    float       max;
    float       defaultVal;
    float       displayScale;
    float       displayExponent;
};

struct UnityAudioEffectDefinition
{
    juce::uint32                   structSize;
    juce::uint32                   parameterStructSize;
    juce::uint32                   apiVersion;
    juce::uint32                   pluginVersion;
    juce::uint32                   channels;
    juce::uint32                   numParameters;
    juce::uint64                   flags;
    char                           name[32];
    createCallback                 create;
    releaseCallback                release;
    resetCallback                  reset;
    processCallback                process;
    setPositionCallback            setPosition;
    UnityAudioParameterDefinition* parameterDefintions;
    setFloatParameterCallback      setFloatParameter;
    getFloatParameterCallback      getFloatParameter;
    getFloatBufferCallback         getFloatBuffer;
};

#endif

//==============================================================================
// Unity callback
extern "C" UNITY_INTERFACE_EXPORT int  UNITY_INTERFACE_API UnityGetAudioEffectDefinitions (UnityAudioEffectDefinition*** definitionsPtr);

// GUI script callbacks
extern "C" UNITY_INTERFACE_EXPORT renderCallback UNITY_INTERFACE_API getRenderCallback();

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityInitialiseTexture (int id, void* textureHandle, int w, int h);

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseDown (int id, float x, float y, UnityEventModifiers mods, int button);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseDrag (int id, float x, float y, UnityEventModifiers mods, int button);
extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityMouseUp   (int id, float x, float y, UnityEventModifiers mods);

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unityKeyEvent (int id, int code, UnityEventModifiers mods, const char* name);

extern "C" UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API unitySetScreenBounds (int id, float x, float y, float w, float h);
