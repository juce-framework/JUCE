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

#include "jucer_LicenseState.h"

//==============================================================================
class LicenseController
{
public:
    LicenseController() = default;

    //==============================================================================
    LicenseState getCurrentState() const noexcept
    {
        return state;
    }

    void setState (const LicenseState& newState)
    {
        state = newState;
        licenseStateToSettings (state, getGlobalProperties());

        stateListeners.call ([] (LicenseStateListener& l) { l.licenseStateChanged(); });
    }

    void resetState()
    {
        setState ({});
    }

    static LicenseState getGPLState()
    {
        static auto logoImage = []() -> Image
        {
            if (auto logo = Drawable::createFromImageData (BinaryData::gpl_logo_svg, BinaryData::gpl_logo_svgSize))
            {
                auto bounds = logo->getDrawableBounds();

                Image image (Image::ARGB, roundToInt (bounds.getWidth()), roundToInt (bounds.getHeight()), true);
                Graphics g (image);
                logo->draw (g, 1.0f);

                return image;
            }

            jassertfalse;
            return {};
        }();

        return { LicenseState::Type::gpl, {}, {}, logoImage };
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

    static Image avatarFromLicenseState (const String& licenseState)
    {
        MemoryOutputStream imageData;
        Base64::convertFromBase64 (imageData, licenseState);

        return ImageFileFormat::loadFrom (imageData.getData(), imageData.getDataSize());
    }

    static String avatarToLicenseState (Image avatarImage)
    {
        MemoryOutputStream imageData;

        if (avatarImage.isValid() && PNGImageFormat().writeImageToStream (avatarImage, imageData))
            return Base64::toBase64 (imageData.getData(), imageData.getDataSize());

        return {};
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
                             licenseXml->getChildElementAllSubText ("authToken", {}),
                             licenseXml->getChildElementAllSubText ("username", {}),
                             avatarFromLicenseState (licenseXml->getStringAttribute ("avatar", {})) };
                }();

                licenseStateToSettings (stateFromOldSettings, props);

                return stateFromOldSettings;
            }

            return { getLicenseTypeFromValue (licenseXml->getStringAttribute ("type", {})),
                     licenseXml->getStringAttribute ("authToken", {}),
                     licenseXml->getStringAttribute ("username", {}),
                     avatarFromLicenseState (licenseXml->getStringAttribute ("avatar", {})) };
        }

        return {};
    }

    static void licenseStateToSettings (const LicenseState& state, PropertiesFile& props)
    {
        props.removeValue ("license");

        if (state.isValid())
        {
            XmlElement licenseXml ("license");

            if (auto* typeString = getLicenseStateValue (state.type))
                licenseXml.setAttribute ("type", typeString);

            licenseXml.setAttribute ("authToken", state.authToken);
            licenseXml.setAttribute ("username",  state.username);
            licenseXml.setAttribute ("avatar",    avatarToLicenseState (state.avatar));

            props.setValue ("license", &licenseXml);
        }

        props.saveIfNeeded();
    }

    //==============================================================================
   #if JUCER_ENABLE_GPL_MODE
    LicenseState state = getGPLState();
   #else
    LicenseState state = licenseStateFromSettings (getGlobalProperties());
   #endif

    ListenerList<LicenseStateListener> stateListeners;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseController)
};
