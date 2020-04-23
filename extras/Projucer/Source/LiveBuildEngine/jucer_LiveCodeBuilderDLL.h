/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


extern "C"
{
    typedef void* LiveCodeBuilder;
    typedef bool (*SendMessageFunction) (void* userInfo, const void* data, size_t dataSize);
    typedef void (*CrashCallbackFunction) (const char* crashDescription);
    typedef void (*QuitCallbackFunction)();
    typedef void (*SetPropertyFunction) (const char* key, const char* value);
    typedef void (*GetPropertyFunction) (const char* key, char* value, size_t size);

    // We've used an X macro to define the DLL functions rather than just declaring them, so that
    // we can load the DLL and its functions dynamically and cope with it not being there.
    // The CompileEngineDLL class is a wrapper that manages finding/loading the DLL and exposing
    // these as callable functions.
    #define LIVE_DLL_FUNCTIONS(X) \
        X (projucer_getVersion,     int, ()) \
        X (projucer_initialise,     void, (CrashCallbackFunction, QuitCallbackFunction, SetPropertyFunction, GetPropertyFunction, bool setupSignals)) \
        X (projucer_shutdown,       void, ()) \
        X (projucer_createBuilder,  LiveCodeBuilder, (SendMessageFunction, void* userInfo, const char* projectID, const char* cacheFolder)) \
        X (projucer_sendMessage,    void, (LiveCodeBuilder, const void* messageData, size_t messageDataSize)) \
        X (projucer_deleteBuilder,  void, (LiveCodeBuilder))

}
