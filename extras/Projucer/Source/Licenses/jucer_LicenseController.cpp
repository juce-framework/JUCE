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
#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "../ProjectSaving/jucer_ProjectExporter.h"
#include "../Project/UI/jucer_HeaderComponent.h"
#include "jucer_LicenseController.h"

#if ! JUCER_ENABLE_GPL_MODE
 #include "jucer_LicenseWebview.h"
 #include "jucer_LicenseThread.h"
#endif

//==============================================================================
const char* LicenseState::licenseTypeToString (LicenseState::Type type)
{
    switch (type)
    {
        case Type::notLoggedIn:         return "<notLoggedIn>";
        case Type::noLicenseChosenYet:  return "<noLicenseChosenYet>";
        case Type::GPL:                 return "JUCE GPL";
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
        case LicenseState::Type::GPL:       return "GPL";
        case LicenseState::Type::personal:  return "personal";
        case LicenseState::Type::edu:       return "edu";
        case LicenseState::Type::indie:     return "indie";
        case LicenseState::Type::pro:       return "pro";
        default:                            return nullptr;
    }
}

static LicenseState::Type getLicenseTypeFromValue (const String& d)
{
    if (d == getLicenseStateValue (LicenseState::Type::GPL))       return LicenseState::Type::GPL;
    if (d == getLicenseStateValue (LicenseState::Type::personal))  return LicenseState::Type::personal;
    if (d == getLicenseStateValue (LicenseState::Type::edu))       return LicenseState::Type::edu;
    if (d == getLicenseStateValue (LicenseState::Type::indie))     return LicenseState::Type::indie;
    if (d == getLicenseStateValue (LicenseState::Type::pro))       return LicenseState::Type::pro;
    return LicenseState::Type::noLicenseChosenYet;
}

static const char* getApplicationUsageDataStateValue (LicenseState::ApplicationUsageData type)
{
    switch (type)
    {
        case LicenseState::ApplicationUsageData::enabled:   return "enabled";
        case LicenseState::ApplicationUsageData::disabled:  return "disabled";
        default:                                            return "notChosen";
    }
}

static LicenseState::ApplicationUsageData getApplicationUsageDataTypeFromValue (const String& value)
{
    if (value == getApplicationUsageDataStateValue (LicenseState::ApplicationUsageData::enabled))   return LicenseState::ApplicationUsageData::enabled;
    if (value == getApplicationUsageDataStateValue (LicenseState::ApplicationUsageData::disabled))  return LicenseState::ApplicationUsageData::disabled;
    return LicenseState::ApplicationUsageData::notChosenYet;
}

#if ! JUCER_ENABLE_GPL_MODE
struct LicenseController::ModalCompletionCallback : ModalComponentManager::Callback
{
    ModalCompletionCallback (LicenseController& controller) : owner (controller) {}
    void modalStateFinished (int returnValue) override       { owner.modalStateFinished (returnValue); }
    LicenseController& owner;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ModalCompletionCallback)
};
#endif

//==============================================================================
LicenseController::LicenseController()
    : state (licenseStateFromSettings (ProjucerApplication::getApp().settings->getGlobalProperties()))
{
   #if JUCER_ENABLE_GPL_MODE
    state.type     = LicenseState::Type::GPL;
    state.username = "GPL mode";
   #endif
}

LicenseController::~LicenseController()
{
   #if ! JUCER_ENABLE_GPL_MODE
    thread.reset();
    closeWebview (-1);
   #endif
}

LicenseState LicenseController::getState() const noexcept
{
    LicenseState projucerState = state;

    // if the user has never logged in before and the user is running from command line
    // then we have no way to ask the user to log in, so fallback to GPL mode
    if (guiNotInitialisedYet
        && (state.type == LicenseState::Type::notLoggedIn
         || state.type == LicenseState::Type::noLicenseChosenYet))
    {
        projucerState.type = LicenseState::Type::GPL;
        projucerState.username = "GPL mode";
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

   #if ! JUCER_ENABLE_GPL_MODE
    if (thread == nullptr)
        thread.reset (new LicenseThread (*this, false));
   #endif
}

void LicenseController::logout()
{
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

   #if ! JUCER_ENABLE_GPL_MODE
    thread.reset();
    updateState ({});

   #if ! JUCE_LINUX
    WebBrowserComponent::clearCookies();
   #endif

    thread.reset (new LicenseThread (*this, false));
   #endif
}

void LicenseController::chooseNewLicense()
{
    jassert (MessageManager::getInstance()->isThisTheMessageThread());

   #if ! JUCER_ENABLE_GPL_MODE
    thread.reset();
    thread.reset (new LicenseThread (*this, true));
   #endif
}

void LicenseController::setApplicationUsageDataState (LicenseState::ApplicationUsageData newState)
{
    if (state.applicationUsageDataState != newState)
    {
        state.applicationUsageDataState = newState;
        ProjucerApplication::getApp().setAnalyticsEnabled (newState == LicenseState::ApplicationUsageData::enabled);

        updateState (state);
    }
}

//==============================================================================
#if ! JUCER_ENABLE_GPL_MODE
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
       #if ! JUCE_LINUX
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
#endif

void LicenseController::updateState (const LicenseState& newState)
{
    auto& props = ProjucerApplication::getApp().settings->getGlobalProperties();

    auto oldLicenseType = state.type;

    state = newState;
    licenseStateToSettings (state, props);
    auto stateParam = getState();
    listeners.call ([&] (StateChangedCallback& l) { l.licenseStateChanged (stateParam); });

    if (oldLicenseType != state.type)
    {
        StringPairArray data;
        data.set ("label", state.licenseTypeToString (state.type));

        Analytics::getInstance()->logEvent ("License Type", data, ProjucerAnalyticsEvent::userEvent);
    }
}

LicenseState LicenseController::licenseStateFromOldSettings (XmlElement* licenseXml)
{
    LicenseState result;
    result.type                        = getLicenseTypeFromValue    (licenseXml->getChildElementAllSubText ("type", {}));
    result.applicationUsageDataState   = getApplicationUsageDataTypeFromValue (licenseXml->getChildElementAllSubText ("applicationUsageData", {}));
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
    std::unique_ptr<XmlElement> licenseXml (props.getXmlValue ("license"));

    if (licenseXml != nullptr)
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
        result.applicationUsageDataState   = getApplicationUsageDataTypeFromValue (licenseXml->getStringAttribute ("applicationUsageData", {}));
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

        licenseXml.setAttribute ("applicationUsageData", getApplicationUsageDataStateValue (state.applicationUsageDataState));
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
