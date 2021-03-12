/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

//==============================================================================
namespace LicenseHelpers
{
    inline LicenseState::Type licenseTypeForString (const String& licenseString)
    {
        if (licenseString == "juce-pro")       return LicenseState::Type::pro;
        if (licenseString == "juce-indie")     return LicenseState::Type::indie;
        if (licenseString == "juce-edu")       return LicenseState::Type::educational;
        if (licenseString == "juce-personal")  return LicenseState::Type::personal;

        jassertfalse; // unknown type
        return LicenseState::Type::none;
    }

    using LicenseVersionAndType = std::pair<int, LicenseState::Type>;

    inline LicenseVersionAndType findBestLicense (std::vector<LicenseVersionAndType>&& licenses)
    {
        if (licenses.size() == 1)
            return licenses[0];

        auto getValueForLicenceType = [] (LicenseState::Type type)
        {
            switch (type)
            {
                case LicenseState::Type::pro:          return 4;
                case LicenseState::Type::indie:        return 3;
                case LicenseState::Type::educational:  return 2;
                case LicenseState::Type::personal:     return 1;
                case LicenseState::Type::gpl:
                case LicenseState::Type::none:
                default:                               return -1;
            }
        };

        std::sort (licenses.begin(), licenses.end(),
                   [getValueForLicenceType] (const LicenseVersionAndType& l1, const LicenseVersionAndType& l2)
                   {
                       if (l1.first > l2.first)
                           return true;

                       if (l1.first == l2.first)
                           return getValueForLicenceType (l1.second) > getValueForLicenceType (l2.second);

                       return false;
                   });

        auto findFirstLicense = [&licenses] (bool isPaid)
        {
            auto iter = std::find_if (licenses.begin(), licenses.end(),
                                      [isPaid] (const LicenseVersionAndType& l)
                                      {
                                          auto proOrIndie = (l.second == LicenseState::Type::pro || l.second == LicenseState::Type::indie);
                                          return isPaid ? proOrIndie : ! proOrIndie;
                                      });

            return iter != licenses.end() ? *iter
                                          : LicenseVersionAndType();
        };

        auto newestPaid = findFirstLicense (true);
        auto newestFree = findFirstLicense (false);

        if (newestPaid.first >= projucerMajorVersion || newestPaid.first >= newestFree.first)
            return newestPaid;

        return newestFree;
    }
}

//==============================================================================
class LicenseQueryThread
{
public:
    enum class ErrorType
    {
        busy,
        cancelled,
        connectionError,
        webResponseError
    };

    using ErrorMessageAndType = std::pair<String, ErrorType>;
    using LicenseQueryCallback = std::function<void (ErrorMessageAndType, LicenseState)>;

    //==============================================================================
    LicenseQueryThread() = default;

    void checkLicenseValidity (const LicenseState& state, LicenseQueryCallback completionCallback)
    {
        if (jobPool.getNumJobs() > 0)
        {
            completionCallback ({ {}, ErrorType::busy }, {});
            return;
        }

        jobPool.addJob ([this, state, completionCallback]
        {
            auto updatedState = state;

            auto result = runTask (std::make_unique<UserLicenseQuery> (state.authToken), updatedState);

            WeakReference<LicenseQueryThread> weakThis (this);
            MessageManager::callAsync ([weakThis, result, updatedState, completionCallback]
            {
                if (weakThis != nullptr)
                    completionCallback (result, updatedState);
            });
        });
    }

    void doSignIn (const String& email, const String& password, LicenseQueryCallback completionCallback)
    {
        cancelRunningJobs();

        jobPool.addJob ([this, email, password, completionCallback]
        {
            LicenseState state;

            auto result = runTask (std::make_unique<UserLogin> (email, password), state);

            if (result == ErrorMessageAndType())
                result = runTask (std::make_unique<UserLicenseQuery> (state.authToken), state);

            if (result != ErrorMessageAndType())
                state = {};

            WeakReference<LicenseQueryThread> weakThis (this);
            MessageManager::callAsync ([weakThis, result, state, completionCallback]
            {
                if (weakThis != nullptr)
                    completionCallback (result, state);
            });
        });
    }

    void cancelRunningJobs()
    {
        jobPool.removeAllJobs (true, 500);
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
            licenseState.username = json.getProperty ("user", {}).getProperty ("username", {}).toString();

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
                std::vector<LicenseHelpers::LicenseVersionAndType> licenses;

                for (auto& license : *licensesJson)
                {
                    auto version = license.getProperty ("product_version", {}).toString().trim();
                    auto type    = license.getProperty ("licence_type", {}).toString();
                    auto status  = license.getProperty ("status", {}).toString();

                    if (status == "active" && type.isNotEmpty() && version.isNotEmpty())
                        licenses.push_back ({ version.getIntValue(), LicenseHelpers::licenseTypeForString (type) });
                }

                if (! licenses.empty())
                {
                    auto bestLicense = LicenseHelpers::findBestLicense (std::move (licenses));

                    licenseState.version = bestLicense.first;
                    licenseState.type = bestLicense.second;
                }

                return true;
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

    static ErrorMessageAndType runTask (std::unique_ptr<AccountEnquiryBase> accountEnquiryTask, LicenseState& state)
    {
        const ErrorMessageAndType cancelledError ("Cancelled.", ErrorType::cancelled);
        const String endpointURL ("https://api.juce.com/api/v1");

        URL url (endpointURL + accountEnquiryTask->getEndpointURLSuffix());

        auto isPOST = accountEnquiryTask->isPOSTLikeRequest();

        if (isPOST)
            url = url.withPOSTData (postDataStringAsJSON (accountEnquiryTask->getParameterNamesAndValues()));

        if (ThreadPoolJob::getCurrentThreadPoolJob()->shouldExit())
            return cancelledError;

        int statusCode = 0;
        auto urlStream = url.createInputStream (URL::InputStreamOptions (isPOST ? URL::ParameterHandling::inPostData
                                                                                : URL::ParameterHandling::inAddress)
                                                  .withExtraHeaders (accountEnquiryTask->getExtraHeaders())
                                                  .withConnectionTimeoutMs (5000)
                                                  .withStatusCode (&statusCode));

        if (urlStream == nullptr)
            return { "Failed to connect to the web server.", ErrorType::connectionError };

        if (statusCode != accountEnquiryTask->getSuccessCode())
            return { accountEnquiryTask->errorCodeToString (statusCode), ErrorType::webResponseError };

        if (ThreadPoolJob::getCurrentThreadPoolJob()->shouldExit())
            return cancelledError;

        String response;

        for (;;)
        {
            char buffer [8192] = "";
            auto num = urlStream->read (buffer, sizeof (buffer));

            if (ThreadPoolJob::getCurrentThreadPoolJob()->shouldExit())
                return cancelledError;

            if (num <= 0)
                break;

            response += buffer;
        }

        if (ThreadPoolJob::getCurrentThreadPoolJob()->shouldExit())
            return cancelledError;

        if (! accountEnquiryTask->parseServerResponse (response, state))
            return { "Failed to parse server response.", ErrorType::webResponseError };

        return {};
    }

    //==============================================================================
    ThreadPool jobPool { 1 };

    //==============================================================================
    JUCE_DECLARE_WEAK_REFERENCEABLE (LicenseQueryThread)
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseQueryThread)
};
