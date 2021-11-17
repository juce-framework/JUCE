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

#include "jucer_LicenseState.h"
#include "jucer_LicenseQueryThread.h"

//==============================================================================
class LicenseController  : private Timer
{
public:
    LicenseController()
    {
        checkLicense();
    }

    //==============================================================================
    static LicenseState getGPLState()
    {
        return { LicenseState::Type::gpl, projucerMajorVersion, {}, {} };
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
            case LicenseState::Type::gpl:          return "GPL";
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
        if (d == getLicenseStateValue (LicenseState::Type::gpl))          return LicenseState::Type::gpl;
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
        if (state.authToken.isNotEmpty() && ! state.isGPL())
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
   #if JUCER_ENABLE_GPL_MODE
    LicenseState state = getGPLState();
   #else
    LicenseState state = licenseStateFromSettings (getGlobalProperties());
   #endif

    ListenerList<LicenseStateListener> stateListeners;
    LicenseQueryThread licenseQueryThread;
    bool hasRetriedLicenseCheck = false;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseController)
};
