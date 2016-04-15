/*
 ==============================================================================
 Copyright 2015 by Raw Material Software Ltd.
 ==============================================================================
*/


#ifndef PROJUCER_PROJUCERLICENSES_H_INCLUDED
#define PROJUCER_PROJUCERLICENSES_H_INCLUDED


//==============================================================================
struct ProjucerLicences  : private DeletedAtShutdown
{
    ProjucerLicences()
    {
        dll.initialise (crashCallback, quitCallback, false);
    }

    juce_DeclareSingleton (ProjucerLicences, false);

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
        static_cast<ProjucerLicences*> (userInfo)->callbackFunction (errorMessage, username, apiKey);
    }

    static void crashCallback (const char*) {}
    static void quitCallback() {}
};



#endif  // PROJUCER_PROJUCERLICENSES_H_INCLUDED
