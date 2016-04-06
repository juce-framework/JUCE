/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef PROJUCER_LIVECODEBUILDERDLL_H_INCLUDED
#define PROJUCER_LIVECODEBUILDERDLL_H_INCLUDED


extern "C"
{
    typedef void* LiveCodeBuilder;
    typedef bool (*SendMessageFunction) (void* userInfo, const void* data, size_t dataSize);
    typedef void (*CrashCallbackFunction) (const char* crashDescription);
    typedef void (*QuitCallbackFunction)();
    typedef void (*SetPropertyFunction) (const char* key, const char* value);
    typedef void (*GetPropertyFunction) (const char* key, char* value, size_t size);
    typedef void (*LoginCallbackFunction) (void* userInfo, const char* errorMessage, const char* username, const char* apiKey);

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
        X (projucer_deleteBuilder,  void, (LiveCodeBuilder)) \
        X (projucer_login,          void, (const char* userLoginName, const char* userPassword, bool remainLoggedIn, LoginCallbackFunction, void* callbackUserInfo)) \
        X (projucer_logout,         void, ()) \
        X (projucer_isLoggedIn,     bool, ()) \
        X (projucer_getLoginName,   void, (char*)) \
        X (projucer_hasLicense,     bool, (const char* featureName)) \
        X (projucer_hasLiveCodingLicence, bool, ())

}


#endif  // PROJUCER_LIVECODEBUILDERDLL_H_INCLUDED
