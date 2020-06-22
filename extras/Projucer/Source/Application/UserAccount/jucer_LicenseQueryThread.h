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


//==============================================================================
class LicenseQueryThread  : public Thread
{
public:
    LicenseQueryThread (const String& userEmail, const String& userPassword,
                        std::function<void (LicenseState, String)>&& cb)
        : Thread ("LicenseQueryThread"),
          email (userEmail),
          password (userPassword),
          completionCallback (std::move (cb))
    {
        startThread();
    }

    ~LicenseQueryThread() override
    {
        signalThreadShouldExit();
        waitForThreadToExit (6000);
    }

    void run() override
    {
        LicenseState state;

        auto errorMessage = runJob (std::make_unique<UserLogin> (email, password), state);

        if (errorMessage.isEmpty())
            errorMessage = runJob (std::make_unique<UserLicenseQuery> (state.authToken), state);

        if (errorMessage.isNotEmpty())
            state = {};

        WeakReference<LicenseQueryThread> weakThis (this);
        MessageManager::callAsync ([this, weakThis, state, errorMessage]
        {
            if (weakThis != nullptr)
                completionCallback (state, errorMessage);
        });
    }

private:
    //==============================================================================
    struct AccountEnquiryBase
    {
        virtual ~AccountEnquiryBase() = default;

        virtual bool isPOSTLikeRequest() const = 0;
        virtual String getEndpointURLSuffix() const = 0;
        virtual StringPairArray getParameterNamesAndValues() const = 0;
        virtual String getExtraHeaders() const = 0;
        virtual int getSuccessCode() const = 0;
        virtual String errorCodeToString (int) const = 0;
        virtual bool parseServerResponse (const String&, LicenseState&) = 0;
    };

    struct UserLogin  : public AccountEnquiryBase
    {
        UserLogin (const String& e, const String& p)
            : userEmail (e), userPassword (p)
        {
        }

        bool isPOSTLikeRequest() const override       { return true; }
        String getEndpointURLSuffix() const override  { return "/authenticate/projucer"; }
        int getSuccessCode() const override           { return 200; }

        StringPairArray getParameterNamesAndValues() const override
        {
            StringPairArray namesAndValues;
            namesAndValues.set ("email", userEmail);
            namesAndValues.set ("password", userPassword);

            return namesAndValues;
        }

        String getExtraHeaders() const override
        {
            return "Content-Type: application/json";
        }

        String errorCodeToString (int errorCode) const override
        {
            switch (errorCode)
            {
                case 400:  return "Please enter your email and password to sign in.";
                case 401:  return "Your email and password are incorrect.";
                case 451:  return "Access denied.";
                default:   return "Something went wrong, please try again.";
            }
        }

        bool parseServerResponse (const String& serverResponse, LicenseState& licenseState) override
        {
            auto json = JSON::parse (serverResponse);

            licenseState.authToken = json.getProperty ("token", {}).toString();
            licenseState.username  = json.getProperty ("user",  {}).getProperty ("username", {}).toString();

            auto avatarURL = json.getProperty ("user", {}).getProperty ("avatar_url", {}).toString();

            if (avatarURL.isNotEmpty())
            {
                URL url (avatarURL);

                if (auto stream = url.createInputStream (false, nullptr, nullptr, {}, 5000))
                {
                    MemoryBlock mb;
                    stream->readIntoMemoryBlock (mb);

                    licenseState.avatar = ImageFileFormat::loadFrom (mb.getData(), mb.getSize());
                }
            }

            return (licenseState.authToken.isNotEmpty() && licenseState.username.isNotEmpty());
        }

        String userEmail, userPassword;
    };

    struct UserLicenseQuery  : public AccountEnquiryBase
    {
        UserLicenseQuery (const String& authToken)
            : userAuthToken (authToken)
        {
        }

        bool isPOSTLikeRequest() const override       { return false; }
        String getEndpointURLSuffix() const override  { return "/user/licences/projucer"; }
        int getSuccessCode() const override           { return 200; }

        StringPairArray getParameterNamesAndValues() const override
        {
            return {};
        }

        String getExtraHeaders() const override
        {
            return "x-access-token: " + userAuthToken;
        }

        String errorCodeToString (int errorCode) const override
        {
            switch (errorCode)
            {
                case 401:  return "User not found or could not be verified.";
                default:   return "User licenses info fetch failed (unknown error).";
            }
        }

        bool parseServerResponse (const String& serverResponse, LicenseState& licenseState) override
        {
            auto json = JSON::parse (serverResponse);

            if (auto* licensesJson = json.getArray())
            {
                StringArray licenseTypes;

                for (auto& license : *licensesJson)
                {
                    auto status = license.getProperty ("status", {}).toString();

                    if (status == "active")
                        licenseTypes.add (license.getProperty ("licence_type", {}).toString());
                }

                licenseTypes.removeEmptyStrings();
                licenseTypes.removeDuplicates (false);

                licenseState.type = [licenseTypes]()
                {
                    if      (licenseTypes.contains ("juce-pro"))       return LicenseState::Type::pro;
                    else if (licenseTypes.contains ("juce-indie"))     return LicenseState::Type::indie;
                    else if (licenseTypes.contains ("juce-personal"))  return LicenseState::Type::personal;
                    else if (licenseTypes.contains ("juce-edu"))       return LicenseState::Type::educational;

                    return LicenseState::Type::none;
                }();

                return (licenseState.type != LicenseState::Type::none);
            }

            return false;
        }

        String userAuthToken;
    };

    //==============================================================================
    static String postDataStringAsJSON (const StringPairArray& parameters)
    {
        DynamicObject::Ptr d (new DynamicObject());

        for (auto& key : parameters.getAllKeys())
            d->setProperty (key, parameters[key]);

        return JSON::toString (var (d.get()));
    }

    String runJob (std::unique_ptr<AccountEnquiryBase> accountEnquiryJob, LicenseState& state)
    {
        const String endpointURL = "https://api.juce.com/api/v1";

        auto url = URL (endpointURL + accountEnquiryJob->getEndpointURLSuffix());

        auto isPOST = accountEnquiryJob->isPOSTLikeRequest();

        if (isPOST)
            url = url.withPOSTData (postDataStringAsJSON (accountEnquiryJob->getParameterNamesAndValues()));

        if (threadShouldExit())
            return "Cancelled.";

        int statusCode = 0;
        auto urlStream = url.createInputStream (isPOST, nullptr, nullptr,
                                                accountEnquiryJob->getExtraHeaders(),
                                                5000, nullptr, &statusCode);

        if (urlStream == nullptr)
            return "Failed to connect to the web server.";

        if (statusCode != accountEnquiryJob->getSuccessCode())
            return accountEnquiryJob->errorCodeToString (statusCode);

        if (threadShouldExit())
            return "Cancelled.";

        String response;

        for (;;)
        {
            char buffer [8192];
            auto num = urlStream->read (buffer, sizeof (buffer));

            if (threadShouldExit())
                return "Cancelled.";

            if (num <= 0)
                break;

            response += buffer;
        }

        if (threadShouldExit())
            return "Cancelled.";

        if (! accountEnquiryJob->parseServerResponse (response, state))
            return "Failed to parse server response.";

        return {};
    }

    //==============================================================================
    const String email, password;
    const std::function<void (LicenseState, String)> completionCallback;

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (LicenseQueryThread)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseQueryThread)
};
