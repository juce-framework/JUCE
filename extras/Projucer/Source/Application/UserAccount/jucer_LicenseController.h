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

#include "jucer_LicenseState.h"
#include "jucer_LicenseQueryThread.h"

//==============================================================================
class LicenseController final : private Timer
{
public:
    LicenseController()
    {
        checkLicense();
    }

    //==============================================================================
    static LicenseState getAGPLState()
    {
        return { LicenseState::Type::agplv3, projucerMajorVersion, {}, {} };
    }

    LicenseState getCurrentState() const noexcept
    {
        return state;
    }

    void setState (const LicenseState& newState)
    {
        if (state != newState)
        {
            state = newState;
            licenseStateToSettings (state, getGlobalProperties());

            stateListeners.call ([] (LicenseStateListener& l) { l.licenseStateChanged(); });
        }
    }

    void resetState()
    {
        setState ({});
    }

    void signIn (const String& email, const String& password,
                 std::function<void (const String&)> completionCallback)
    {
        licenseQueryThread.doSignIn (email, password,
                                     [this, completionCallback] (LicenseQueryThread::ErrorMessageAndType error,
                                                                 LicenseState newState)
                                     {
                                         completionCallback (error.first);
                                         setState (newState);
                                     });
    }

    void cancelSignIn()
    {
        licenseQueryThread.cancelRunningJobs();
    }

    //==============================================================================
    struct LicenseStateListener
    {
        virtual ~LicenseStateListener() = default;
        virtual void licenseStateChanged() = 0;
    };

    void addListener (LicenseStateListener* listenerToAdd)
    {
        stateListeners.add (listenerToAdd);
    }

    void removeListener (LicenseStateListener* listenerToRemove)
    {
        stateListeners.remove (listenerToRemove);
    }

private:
    //==============================================================================
    static const char* getLicenseStateValue (LicenseState::Type type)
    {
        switch (type)
        {
            case LicenseState::Type::agplv3:       return "AGPLv3";
            case LicenseState::Type::personal:     return "personal";
            case LicenseState::Type::educational:  return "edu";
            case LicenseState::Type::indie:        return "indie";
            case LicenseState::Type::pro:          return "pro";
            case LicenseState::Type::none:
            default:                               break;
        }

        return nullptr;
    }

    static LicenseState::Type getLicenseTypeFromValue (const String& d)
    {
        if (d == getLicenseStateValue (LicenseState::Type::agplv3))       return LicenseState::Type::agplv3;
        if (d == getLicenseStateValue (LicenseState::Type::personal))     return LicenseState::Type::personal;
        if (d == getLicenseStateValue (LicenseState::Type::educational))  return LicenseState::Type::educational;
        if (d == getLicenseStateValue (LicenseState::Type::indie))        return LicenseState::Type::indie;
        if (d == getLicenseStateValue (LicenseState::Type::pro))          return LicenseState::Type::pro;
        return LicenseState::Type::none;
    }

    static LicenseState licenseStateFromSettings (PropertiesFile& props)
    {
        if (auto licenseXml = props.getXmlValue ("license"))
        {
            // this is here for backwards compatibility with old-style settings files using XML text elements
            if (licenseXml->getChildElementAllSubText ("type", {}).isNotEmpty())
            {
                auto stateFromOldSettings = [&licenseXml]() -> LicenseState
                {
                    return { getLicenseTypeFromValue (licenseXml->getChildElementAllSubText ("type", {})),
                             licenseXml->getChildElementAllSubText ("version", "-1").getIntValue(),
                             licenseXml->getChildElementAllSubText ("username", {}),
                             licenseXml->getChildElementAllSubText ("authToken", {}) };
                }();

                licenseStateToSettings (stateFromOldSettings, props);

                return stateFromOldSettings;
            }

            return { getLicenseTypeFromValue (licenseXml->getStringAttribute ("type", {})),
                     licenseXml->getIntAttribute ("version", -1),
                     licenseXml->getStringAttribute ("username", {}),
                     licenseXml->getStringAttribute ("authToken", {}) };
        }

        return {};
    }

    static void licenseStateToSettings (const LicenseState& state, PropertiesFile& props)
    {
        props.removeValue ("license");

        if (state.isSignedIn())
        {
            XmlElement licenseXml ("license");

            if (auto* typeString = getLicenseStateValue (state.type))
                licenseXml.setAttribute ("type", typeString);

            licenseXml.setAttribute ("version",   state.version);
            licenseXml.setAttribute ("username",  state.username);
            licenseXml.setAttribute ("authToken", state.authToken);

            props.setValue ("license", &licenseXml);
        }

        props.saveIfNeeded();
    }

    //==============================================================================
    void checkLicense()
    {
        if (state.authToken.isNotEmpty() && ! state.isAGPL())
        {
            auto completionCallback = [this] (LicenseQueryThread::ErrorMessageAndType error,
                                              LicenseState updatedState)
            {
                if (error == LicenseQueryThread::ErrorMessageAndType())
                {
                    setState (updatedState);
                }
                else if ((error.second == LicenseQueryThread::ErrorType::busy
                         || error.second == LicenseQueryThread::ErrorType::cancelled
                         || error.second == LicenseQueryThread::ErrorType::connectionError)
                           && ! hasRetriedLicenseCheck)
                {
                    hasRetriedLicenseCheck = true;
                    startTimer (10000);
                }
            };

            licenseQueryThread.checkLicenseValidity (state, std::move (completionCallback));
        }
    }

    void timerCallback() override
    {
        stopTimer();
        checkLicense();
    }

    //==============================================================================
    LicenseState state = licenseStateFromSettings (getGlobalProperties());

    ListenerList<LicenseStateListener> stateListeners;
    LicenseQueryThread licenseQueryThread;
    bool hasRetriedLicenseCheck = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseController)
};
