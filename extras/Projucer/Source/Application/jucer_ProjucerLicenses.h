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

#ifndef PROJUCER_PROJUCERLICENSES_H_INCLUDED
#define PROJUCER_PROJUCERLICENSES_H_INCLUDED


//==============================================================================
struct ProjucerLicenses  : private DeletedAtShutdown
{
    ProjucerLicenses()
    {
        dll.initialise (crashCallback, quitCallback, false);
    }

    juce_DeclareSingleton (ProjucerLicenses, false);

    //==============================================================================
    struct LoginCallback
    {
        virtual ~LoginCallback() {}

        // always called on message thread
        virtual void loginError (const String& errorMessage, bool hiliteUserID) = 0;
        virtual void loginSuccess (const String& username, const String& apiKey) = 0;
    };

    // Log the user in. This will return immedietely
    void login (const String& userLoginName, const String& userPassword,
                bool remainLoggedIn, LoginCallback* callback)
    {
        if (dll.isLoaded())
        {
            jassert (callback != nullptr);
            jassert (MessageManager::getInstance()->isThisTheMessageThread());

            userCallback = callback;
            dll.projucer_login (userLoginName.toRawUTF8(), userPassword.toRawUTF8(),
                                remainLoggedIn, staticCallbackFunction, this);
        }
        else
        {
            callback->loginError ("The Projucer DLL is missing", false);
        }
    }

    // Log the user out. Only call on the message thread!
    void logout()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        if (dll.isLoaded())
            dll.projucer_logout();
    }

    bool isLoggedIn() const noexcept
    {
        return dll.isLoaded() && dll.projucer_isLoggedIn();
    }

    String getLoginName() const noexcept
    {
        if (dll.isLoaded())
        {
            char name[256] = { 0 };
            dll.projucer_getLoginName (name);
            return String::fromUTF8 (name);
        }

        return String();
    }

    bool isDLLPresent() const
    {
        return dll.isLoaded();
    }

    bool hasLiveCodingLicence() const
    {
        return isDLLPresent() && dll.projucer_hasLiveCodingLicence();
    }

    bool hasFreeToUseLicense() const
    {
        return isDLLPresent() && dll.projucer_hasLicense ("ProjucerFreeToUse");
    }

    bool retryLoadDll()
    {
        dll.tryLoadDll();
        return dll.isLoaded();
    }

private:
    CompileEngineDLL dll;
    LoginCallback* userCallback = nullptr;

    void callbackFunction (const char* errorMessage, const char* username, const char* apiKey)
    {
        jassert (userCallback != nullptr);

        if (userCallback != nullptr)
        {
            if (errorMessage != nullptr)
                userCallback->loginError (String::fromUTF8 (errorMessage), false);
            else
                userCallback->loginSuccess (String::fromUTF8 (username), String::fromUTF8 (apiKey));
        }
    }

    static void staticCallbackFunction (void* userInfo, const char* errorMessage, const char* username, const char* apiKey)
    {
        static_cast<ProjucerLicenses*> (userInfo)->callbackFunction (errorMessage, username, apiKey);
    }

    static void crashCallback (const char*) {}
    static void quitCallback() {}
};



#endif  // PROJUCER_PROJUCERLICENSES_H_INCLUDED
