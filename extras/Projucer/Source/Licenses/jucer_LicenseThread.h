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

//==============================================================================
struct NetWorkerThread   : public Thread,
                           private AsyncUpdater
{
    NetWorkerThread()  : Thread ("License") {}

    ~NetWorkerThread()
    {
        jassert (MessageManager::getInstance()->isThisTheMessageThread());

        signalThreadShouldExit();
        cancelPendingUpdate();
        finished.signal();

        {
            ScopedLock lock (weakReferenceLock);

            if (currentInputStream != nullptr)
                currentInputStream->cancel();
        }

        waitForThreadToExit (-1);
    }

    //==============================================================================
    void executeOnMessageThreadAndBlock (std::function<void()> f, bool signalWhenFinished = true)
    {
        // only call this on the worker thread
        jassert (Thread::getCurrentThreadId() == getThreadId());

        if (! isWaiting)
        {
            ScopedValueSetter<bool> reentrant (isWaiting, true);

            finished.reset();

            if (! threadShouldExit())
            {
                functionToExecute = [signalWhenFinished, f, this] () { f(); if (signalWhenFinished) finished.signal(); };
                triggerAsyncUpdate();
                finished.wait (-1);
            }
        }
        else
        {
            // only one task at a time
            jassertfalse;
            return;
        }
    }

    WebInputStream* getSharedWebInputStream (const URL& url, const bool usePost)
    {
        ScopedLock lock (weakReferenceLock);

        if (threadShouldExit())
            return nullptr;

        jassert (currentInputStream == nullptr);
        return (currentInputStream = new WeakWebInputStream (*this, url, usePost));
    }

    bool isWaiting = false;
    WaitableEvent finished;

private:
    //==============================================================================
    void handleAsyncUpdate() override
    {
        if (functionToExecute)
        {
            std::function<void()> f;
            std::swap (f, functionToExecute);

            if (! threadShouldExit())
                f();
        }
    }

    //==============================================================================
    struct WeakWebInputStream  : public WebInputStream
    {
        WeakWebInputStream (NetWorkerThread& workerThread, const URL& url, const bool usePost)
            : WebInputStream (url, usePost), owner (workerThread) {}

        ~WeakWebInputStream()
        {
            ScopedLock lock (owner.weakReferenceLock);
            owner.currentInputStream = nullptr;
        }

        NetWorkerThread& owner;
        WeakReference<WeakWebInputStream>::Master masterReference;
        friend class WeakReference<WeakWebInputStream>;
    };

    //==============================================================================
    friend struct WeakWebInputStream;

    std::function<void()> functionToExecute;
    CriticalSection weakReferenceLock;
    WebInputStream* currentInputStream = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NetWorkerThread)
};

//==============================================================================
//==============================================================================
//==============================================================================
struct LicenseThread : NetWorkerThread
{
    LicenseThread (LicenseController& licenseController, bool shouldSelectNewLicense)
        : owner (licenseController), selectNewLicense (shouldSelectNewLicense)
    {
        startThread();
    }

    String getAuthToken()
    {
        if (owner.state.authToken.isNotEmpty())
            return owner.state.authToken;

        selectNewLicense = false;
        HashMap<String, String> result;

        if (! queryWebview ("https://auth.roli.com/signin/projucer?redirect=projucer://receive-auth-token?token=",
                            "receive-auth-token", result))
            return {};

        return result["token"];
    }

    // returns true if any information was updated
    void updateUserInfo (LicenseState& stateToUpdate)
    {
        jassert (stateToUpdate.authToken.isNotEmpty());

        auto accessTokenHeader = "x-access-token: " + stateToUpdate.authToken;

        if (ScopedPointer<WebInputStream> shared
              = getSharedWebInputStream (URL ("https://api.roli.com/api/v1/user"), false))
        {
            const int statusCode = shared->withExtraHeaders (accessTokenHeader).getStatusCode();

            if (statusCode == 200)
            {
                var result = JSON::parse (shared->readEntireStreamAsString());
                shared = nullptr;

                auto newState = licenseStateFromJSON (result, stateToUpdate.authToken, stateToUpdate.avatar);

                if (newState.type != LicenseState::Type::notLoggedIn)
                    stateToUpdate = newState;
            }
            else if (statusCode == 401)
            {
                selectNewLicense = false;

                // un-authorised: token has expired
                stateToUpdate = LicenseState();
            }
        }
    }

    void updateLicenseType (LicenseState& stateToUpdate)
    {
        bool requiredWebview = false;
        String licenseChooserPage = "https://juce.com/webviews/select_license";

        jassert (stateToUpdate.authToken.isNotEmpty());
        jassert (stateToUpdate.type != LicenseState::Type::notLoggedIn);

        auto accessTokenHeader = "x-access-token: " + stateToUpdate.authToken;
        StringArray licenses;

        while ((licenses.isEmpty() || selectNewLicense) && ! threadShouldExit())
        {
            static Identifier licenseTypeIdentifier   ("type");
            static Identifier licenseStatusIdentifier ("status");
            static Identifier projucerLicenseTypeIdentifier ("licence_type");
            static Identifier productNameIdentifier ("product_name");
            static Identifier licenseIdentifier ("licence");
            static Identifier serialIdentifier  ("serial_number");
            static Identifier versionIdentifier ("product_version");
            static Identifier searchInternalIdentifier ("search_internal_id");

            if (! selectNewLicense)
            {
                ScopedPointer<WebInputStream> shared = getSharedWebInputStream (URL ("https://api.roli.com/api/v1/user/licences?search_internal_id=com.roli.projucer&version=5"),
                                                                                false);
                if (shared == nullptr)
                    break;

                var json = JSON::parse (shared->withExtraHeaders (accessTokenHeader)
                                          .readEntireStreamAsString());
                shared = nullptr;

                if (auto* jsonLicenses = json.getArray())
                {
                    for (auto& v : *jsonLicenses)
                    {
                        if (auto* obj = v.getDynamicObject())
                        {
                            const String& productType = obj->getProperty (projucerLicenseTypeIdentifier);
                            const String& status      = obj->getProperty (licenseStatusIdentifier);

                            if (productType.isNotEmpty() && (status.isEmpty() || status == "active"))
                                licenses.add (productType);
                        }
                    }
                }
                else
                {
                    // no internet -> then use the last valid license
                    if (stateToUpdate.type != LicenseState::Type::notLoggedIn
                     && stateToUpdate.type != LicenseState::Type::noLicenseChosenYet)
                        return;
                }

                if (! licenses.isEmpty())
                    break;
            }

            // ask the user to select a license
            HashMap<String, String> result;
            requiredWebview = true;

            if (! queryWebview (licenseChooserPage, {}, result))
                break;

            const String& redirectURL       = result["page-redirect"];
            const String& productKey        = result["register-product"];
            const String& chosenLicenseType = result["redeem-licence-type"];

            if (redirectURL.isNotEmpty())
            {
                licenseChooserPage = "https://juce.com/webviews/register-product";
                continue;
            }

            if (productKey.isNotEmpty())
            {
                DynamicObject::Ptr redeamObject = new DynamicObject();
                redeamObject->setProperty (serialIdentifier, productKey);

                String postData (JSON::toString (var (redeamObject.get())));

                ScopedPointer<WebInputStream> shared = getSharedWebInputStream (URL ("https://api.roli.com/api/v1/user/products").withPOSTData (postData),
                                                                                true);
                if (shared == nullptr)
                    break;

                int statusCode = shared->withExtraHeaders (accessTokenHeader)
                                        .withExtraHeaders ("Content-Type: application/json")
                                        .getStatusCode();

                licenseChooserPage =  String ("https://juce.com/webviews/register-product?error=")
                                    + String (statusCode == 404 ? "invalid" : "server");

                if (statusCode == 200)
                    selectNewLicense = false;

                continue;
            }

            if (chosenLicenseType.isNotEmpty())
            {
                // redeem the license
                DynamicObject::Ptr jsonLicenseObject = new DynamicObject();
                jsonLicenseObject->setProperty (projucerLicenseTypeIdentifier, chosenLicenseType);
                jsonLicenseObject->setProperty (versionIdentifier,     5);


                DynamicObject::Ptr jsonLicenseRequest = new DynamicObject();
                jsonLicenseRequest->setProperty (licenseIdentifier, var (jsonLicenseObject.get()));
                jsonLicenseRequest->setProperty (searchInternalIdentifier, "com.roli.projucer");
                jsonLicenseRequest->setProperty (licenseTypeIdentifier, "software");

                String postData (JSON::toString (var (jsonLicenseRequest.get())));
                ScopedPointer<WebInputStream> shared
                    = getSharedWebInputStream (URL ("https://api.roli.com/api/v1/user/products/redeem").withPOSTData (postData), true);

                if (shared != nullptr)
                {
                    int statusCode = shared->withExtraHeaders (accessTokenHeader)
                                            .withExtraHeaders ("Content-Type: application/json")
                                            .getStatusCode();

                    if (statusCode == 200)
                        selectNewLicense = false;

                    continue;
                }
            }

            break;
        }

        HashMap<String, String> result;

        if (requiredWebview && ! threadShouldExit())
            queryWebview ("https://juce.com/webviews/registration-complete", "licence_provisioned", result);

        stateToUpdate.type = getBestLicenseTypeFromLicenses (licenses);
    }

    //==============================================================================
    void run() override
    {
        LicenseState workState (owner.state);

        while (! threadShouldExit())
        {
            workState.authToken = getAuthToken();

            if (workState.authToken.isEmpty())
                return;

            // read the user information
            updateUserInfo (workState);

            if (threadShouldExit())
                return;

            updateIfChanged (workState);

            // if the last step logged us out then retry
            if (workState.authToken.isEmpty())
                continue;

            // check if the license has changed
            updateLicenseType (workState);

            if (threadShouldExit())
                return;

            updateIfChanged (workState);
            closeWebviewOnMessageThread (0);
            finished.wait (60 * 5 * 1000);
        }
    }

    //==============================================================================
    LicenseState licenseStateFromJSON (const var& json, const String& authToken, const Image& fallbackAvatar)
    {
        static Identifier usernameIdentifier  ("username");
        static Identifier emailIdentifier     ("email");
        static Identifier avatarURLIdentifier ("avatar_url");

        LicenseState result;

        if (auto* obj = json.getDynamicObject())
        {
            result.type      = LicenseState::Type::noLicenseChosenYet;
            result.username  = obj->getProperty (usernameIdentifier);
            result.authToken = authToken;
            result.email     = obj->getProperty (emailIdentifier);
            result.avatar    = fallbackAvatar;

            String avatarURL = obj->getProperty (avatarURLIdentifier);

            if (avatarURL.isNotEmpty())
            {
                if (ScopedPointer<WebInputStream> shared = getSharedWebInputStream (URL (avatarURL), false))
                {
                    MemoryBlock mb;
                    shared->readIntoMemoryBlock (mb);

                    result.avatar = ImageFileFormat::loadFrom (mb.getData(), mb.getSize());
                }
            }
        }

        return result;
    }

    //==============================================================================
    bool queryWebview (const String& startURL, const String& valueToQuery, HashMap<String, String>& result)
    {
        executeOnMessageThreadAndBlock ([&] () { owner.queryWebview (startURL, valueToQuery, result); }, false);
        return (! threadShouldExit());
    }

    void closeWebviewOnMessageThread (int result)
    {
        executeOnMessageThreadAndBlock ([this, result] () { owner.closeWebview (result); });
    }

    static bool stringArrayContainsSubstring (const StringArray& stringArray, const String& substring)
    {
        jassert (substring.isNotEmpty());

        for (auto element : stringArray)
            if (element.containsIgnoreCase (substring))
                return true;

        return false;
    }

    static LicenseState::Type getBestLicenseTypeFromLicenses (const StringArray& licenses)
    {
        if      (stringArrayContainsSubstring (licenses, "juce-pro"))       return LicenseState::Type::pro;
        else if (stringArrayContainsSubstring (licenses, "juce-indie"))     return LicenseState::Type::indie;
        else if (stringArrayContainsSubstring (licenses, "juce-personal"))  return LicenseState::Type::personal;
        else if (stringArrayContainsSubstring (licenses, "juce-edu"))       return LicenseState::Type::edu;

        return LicenseState::Type::noLicenseChosenYet;
    }

    void updateIfChanged (const LicenseState& newState)
    {
        LicenseState updatedState (owner.state);
        bool changed = false;
        bool shouldUpdateLicenseType = (newState.type     != LicenseState::Type::noLicenseChosenYet
                                     || updatedState.type == LicenseState::Type::notLoggedIn);

        if (newState.type != LicenseState::Type::notLoggedIn) updatedState.avatar = newState.avatar;

        if (owner.state.type      != newState.type && shouldUpdateLicenseType) { updatedState.type      = newState.type;      changed = true; }
        if (owner.state.authToken != newState.authToken)                       { updatedState.authToken = newState.authToken; changed = true; }
        if (owner.state.username  != newState.username)                        { updatedState.username  = newState.username;  changed = true; }
        if (owner.state.email     != newState.email)                           { updatedState.email     = newState.email;     changed = true; }
        if (owner.state.avatar.isValid() != newState.avatar.isValid())         { changed = true; }

        if (changed)
            executeOnMessageThreadAndBlock ([this, updatedState]() { owner.updateState (updatedState); });
    }

    //==============================================================================
    LicenseController& owner;
    bool selectNewLicense;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseThread)
};
