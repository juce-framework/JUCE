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
#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "../ProjectSaving/jucer_ProjectExporter.h"
#include "../Project/UI/jucer_HeaderComponent.h"
#include "jucer_LicenseController.h"
#include "jucer_LicenseWebview.h"
#include "jucer_LicenseThread.h"

//==============================================================================
const char* LicenseState::licenseTypeToString (LicenseState::Type type)
{
    switch (type)
    {
        case Type::notLoggedIn:         return "<notLoggedIn>";
        case Type::noLicenseChosenYet:  return "<noLicenseChosenYet>";
        case Type::agplv3:              return "AGPLv3";
        case Type::personal:            return "JUCE Personal";
        case Type::edu:                 return "JUCE Education";
        case Type::indie:               return "JUCE Indie";
        case Type::pro:                 return "JUCE Pro";
        default:                        return "<unknown>";
    }
}

static const char* getLicenseStateValue (LicenseState::Type type)
{
    switch (type)
    {
        case LicenseState::Type::agplv3:    return "agplv3";
        case LicenseState::Type::personal:  return "personal";
        case LicenseState::Type::edu:       return "edu";
        case LicenseState::Type::indie:     return "indie";
        case LicenseState::Type::pro:       return "pro";
        case LicenseState::Type::notLoggedIn:
        case LicenseState::Type::noLicenseChosenYet:
        default:                            return nullptr;
    }
}

static LicenseState::Type getLicenseTypeFromValue (const String& d)
{
    if (d == getLicenseStateValue (LicenseState::Type::agplv3))    return LicenseState::Type::agplv3;
    if (d == getLicenseStateValue (LicenseState::Type::personal))  return LicenseState::Type::personal;
    if (d == getLicenseStateValue (LicenseState::Type::edu))       return LicenseState::Type::edu;
    if (d == getLicenseStateValue (LicenseState::Type::indie))     return LicenseState::Type::indie;
    if (d == getLicenseStateValue (LicenseState::Type::pro))       return LicenseState::Type::pro;
    return LicenseState::Type::noLicenseChosenYet;
}

struct LicenseController::ModalCompletionCallback final : ModalComponentManager::Callback
{
    ModalCompletionCallback (LicenseController& controller) : owner (controller) {}
    void modalStateFinished (int returnValue) override       { owner.modalStateFinished (returnValue); }
    LicenseController& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalCompletionCallback)
};

//==============================================================================
LicenseController::LicenseController()
    : state (licenseStateFromSettings (ProjucerApplication::getApp().settings->getGlobalProperties()))
{
}

LicenseController::~LicenseController()
{
    thread.reset();
    closeWebview (-1);
}

LicenseState LicenseController::getState() const noexcept
{
    LicenseState projucerState = state;

    // if the user has never logged in before and the user is running from command line
    // then we have no way to ask the user to log in, so fallback to AGPLv3 mode
    if (guiNotInitialisedYet
        && (state.type == LicenseState::Type::notLoggedIn
         || state.type == LicenseState::Type::noLicenseChosenYet))
    {
        projucerState.type = LicenseState::Type::agplv3;
        projucerState.username = "AGPLv3 mode";
    }

    return projucerState;
}

void LicenseController::startWebviewIfNeeded()
{
    if (guiNotInitialisedYet)
    {
        guiNotInitialisedYet = false;
        auto stateParam = getState();
        listeners.call ([&] (StateChangedCallback& l) { l.licenseStateChanged (stateParam); });
    }

    if (thread == nullptr)
        thread.reset (new LicenseThread (*this, false));
}

void LicenseController::logout()
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    thread.reset();
    updateState ({});

   #if ! (JUCE_LINUX || JUCE_BSD)
    WebBrowserComponent::clearCookies();
   #endif

    thread.reset (new LicenseThread (*this, false));
}

void LicenseController::chooseNewLicense()
{
    JUCE_ASSERT_MESSAGE_MANAGER_IS_LOCKED

    thread.reset();
    thread.reset (new LicenseThread (*this, true));
}

//==============================================================================
void LicenseController::closeWebview (int result)
{
    if (licenseWebview != nullptr)
        licenseWebview->exitModalState (result);
}

void LicenseController::modalStateFinished (int result)
{
    licenseWebview = nullptr;

    if (result == -1 && (state.type == LicenseState::Type::notLoggedIn
                          || state.type == LicenseState::Type::noLicenseChosenYet))
        JUCEApplication::getInstance()->systemRequestedQuit();
}

void LicenseController::ensureLicenseWebviewIsOpenWithPage (const String& param)
{
    if (licenseWebview != nullptr)
    {
        licenseWebview->goToURL (param);
        licenseWebview->toFront (true);
    }
    else
    {
       #if ! (JUCE_LINUX || JUCE_BSD)
        WebBrowserComponent::clearCookies();
       #endif

        licenseWebview = new LicenseWebview (new ModalCompletionCallback (*this), param);
    }
}

void LicenseController::queryWebview (const String& startURL, const String& valueToQuery,
                                      HashMap<String, String>& result)
{
    ensureLicenseWebviewIsOpenWithPage (startURL);

    licenseWebview->setPageCallback ([this,valueToQuery,&result] (const String& cmd, const HashMap<String, String>& params)
    {
        if (valueToQuery.isEmpty() || cmd == valueToQuery)
        {
            result.clear();

            for (HashMap<String, String>::Iterator it = params.begin(); it != params.end(); ++it)
                result.set (it.getKey(), it.getValue());

            if (thread != nullptr && ! thread->threadShouldExit())
                thread->finished.signal();
        }
    });

    licenseWebview->setNewWindowCallback ([this, &result] (const String& url)
    {
        if (url.endsWith ("get-juce/indie") || url.endsWith ("get-juce/pro"))
        {
            result.clear();
            result.set ("page-redirect", url);

            if (thread != nullptr && ! thread->threadShouldExit())
                thread->finished.signal();
        }
    });
}

void LicenseController::updateState (const LicenseState& newState)
{
    auto& props = ProjucerApplication::getApp().settings->getGlobalProperties();

    state = newState;
    licenseStateToSettings (state, props);
    auto stateParam = getState();
    listeners.call ([&] (StateChangedCallback& l) { l.licenseStateChanged (stateParam); });
}

LicenseState LicenseController::licenseStateFromOldSettings (XmlElement* licenseXml)
{
    LicenseState result;
    result.type                        = getLicenseTypeFromValue    (licenseXml->getChildElementAllSubText ("type", {}));
    result.username                    = licenseXml->getChildElementAllSubText ("username", {});
    result.email                       = licenseXml->getChildElementAllSubText ("email", {});
    result.authToken                   = licenseXml->getChildElementAllSubText ("authToken", {});

    MemoryOutputStream imageData;
    Base64::convertFromBase64 (imageData, licenseXml->getChildElementAllSubText ("avatar", {}));
    result.avatar = ImageFileFormat::loadFrom (imageData.getData(), imageData.getDataSize());

    return result;
}

LicenseState LicenseController::licenseStateFromSettings (PropertiesFile& props)
{
    if (auto licenseXml = props.getXmlValue ("license"))
    {
        // this is here for backwards compatibility with old-style settings files using XML text elements
        if (licenseXml->getChildElementAllSubText ("type", {}) != String())
        {
            auto stateFromOldSettings = licenseStateFromOldSettings (licenseXml.get());

            licenseStateToSettings (stateFromOldSettings, props);

            return stateFromOldSettings;
        }

        LicenseState result;
        result.type                        = getLicenseTypeFromValue    (licenseXml->getStringAttribute ("type", {}));
        result.username                    = licenseXml->getStringAttribute ("username", {});
        result.email                       = licenseXml->getStringAttribute ("email", {});
        result.authToken                   = licenseXml->getStringAttribute ("authToken", {});

        MemoryOutputStream imageData;
        Base64::convertFromBase64 (imageData, licenseXml->getStringAttribute ("avatar", {}));
        result.avatar = ImageFileFormat::loadFrom (imageData.getData(), imageData.getDataSize());

        return result;
    }

    return {};
}

void LicenseController::licenseStateToSettings (const LicenseState& state, PropertiesFile& props)
{
    props.removeValue ("license");

    if (state.type != LicenseState::Type::notLoggedIn && state.username.isNotEmpty())
    {
        XmlElement licenseXml ("license");

        if (auto* typeString = getLicenseStateValue (state.type))
            licenseXml.setAttribute ("type", typeString);

        licenseXml.setAttribute ("username", state.username);
        licenseXml.setAttribute ("email", state.email);
        licenseXml.setAttribute ("authToken", state.authToken);

        MemoryOutputStream imageData;
        if (state.avatar.isValid() && PNGImageFormat().writeImageToStream (state.avatar, imageData))
            licenseXml.setAttribute ("avatar", Base64::toBase64 (imageData.getData(), imageData.getDataSize()));

        props.setValue ("license", &licenseXml);
    }

    props.saveIfNeeded();
}
