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
