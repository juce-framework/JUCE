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

namespace juce
{
namespace build_tools
{
    //==============================================================================
    static bool keyFoundAndNotSequentialDuplicate (XmlElement& xml, const String& key)
    {
        for (auto* element : xml.getChildWithTagNameIterator ("key"))
        {
            if (element->getAllSubText().trim().equalsIgnoreCase (key))
            {
                if (element->getNextElement() != nullptr && element->getNextElement()->hasTagName ("key"))
                {
                    // found broken plist format (sequential duplicate), fix by removing
                    xml.removeChildElement (element, true);
                    return false;
                }

                // key found (not sequential duplicate)
                return true;
            }
        }

        // key not found
        return false;
    }

    static bool addKeyIfNotFound (XmlElement& xml, const String& key)
    {
        if (! keyFoundAndNotSequentialDuplicate (xml, key))
        {
            xml.createNewChildElement ("key")->addTextElement (key);
            return true;
        }

        return false;
    }

    static void addPlistDictionaryKey (XmlElement& xml, const String& key, const String& value)
    {
        if (addKeyIfNotFound (xml, key))
            xml.createNewChildElement ("string")->addTextElement (value);
    }

    template <size_t N>
    static void addPlistDictionaryKey (XmlElement& xml, const String& key, const char (&value) [N])
    {
        addPlistDictionaryKey (xml, key, String { value });
    }

    static void addPlistDictionaryKey (XmlElement& xml, const String& key, const bool value)
    {
        if (addKeyIfNotFound (xml, key))
            xml.createNewChildElement (value ? "true" : "false");
    }

    static void addPlistDictionaryKey (XmlElement& xml, const String& key, int value)
    {
        if (addKeyIfNotFound (xml, key))
            xml.createNewChildElement ("integer")->addTextElement (String (value));
    }

    //==============================================================================
    static void addArrayToPlist (XmlElement& dict, String arrayKey, const StringArray& arrayElements)
    {
        dict.createNewChildElement ("key")->addTextElement (arrayKey);
        auto* plistStringArray = dict.createNewChildElement ("array");

        for (auto& e : arrayElements)
            plistStringArray->createNewChildElement ("string")->addTextElement (e);
    }

    void PlistOptions::write (const File& infoPlistFile) const
    {
        writeStreamToFile (infoPlistFile, [&] (MemoryOutputStream& mo) { write (mo); });
    }

    void PlistOptions::write (MemoryOutputStream& mo) const
    {
        XmlElement::TextFormat format;
        format.dtd = "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">";
        createXML()->writeTo (mo, format);
    }

    std::unique_ptr<XmlElement> PlistOptions::createXML() const
    {
        auto plist = parseXML (plistToMerge);

        if (plist == nullptr || ! plist->hasTagName ("plist"))
            plist.reset (new XmlElement ("plist"));

        auto* dict = plist->getChildByName ("dict");

        if (dict == nullptr)
            dict = plist->createNewChildElement ("dict");

        if (microphonePermissionEnabled)
            addPlistDictionaryKey (*dict, "NSMicrophoneUsageDescription", microphonePermissionText);

        if (cameraPermissionEnabled)
            addPlistDictionaryKey (*dict, "NSCameraUsageDescription", cameraPermissionText);

        if (bluetoothPermissionEnabled)
            addPlistDictionaryKey (*dict, "NSBluetoothAlwaysUsageDescription", bluetoothPermissionText);

        if (iOS)
        {
            if (bluetoothPermissionEnabled)
                addPlistDictionaryKey (*dict, "NSBluetoothPeripheralUsageDescription", bluetoothPermissionText); // needed for pre iOS 13.0

            addPlistDictionaryKey (*dict, "LSRequiresIPhoneOS", true);
            addPlistDictionaryKey (*dict, "UIViewControllerBasedStatusBarAppearance", true);

            if (shouldAddStoryboardToProject)
                addPlistDictionaryKey (*dict, "UILaunchStoryboardName", storyboardName);
        }
        else
        {
            if (sendAppleEventsPermissionEnabled)
                addPlistDictionaryKey (*dict, "NSAppleEventsUsageDescription", sendAppleEventsPermissionText);
        }

        addPlistDictionaryKey (*dict, "CFBundleExecutable",          executableName);

        if (! iOS) // (NB: on iOS this causes error ITMS-90032 during publishing)
            addPlistDictionaryKey (*dict, "CFBundleIconFile", iconFile.exists() ? iconFile.getFileName() : String());

        addPlistDictionaryKey (*dict, "CFBundleIdentifier",          bundleIdentifier);
        addPlistDictionaryKey (*dict, "CFBundleName",                projectName);

        // needed by NSExtension on iOS
        addPlistDictionaryKey (*dict, "CFBundleDisplayName",         projectName);
        addPlistDictionaryKey (*dict, "CFBundlePackageType",         getXcodePackageType (type));
        addPlistDictionaryKey (*dict, "CFBundleSignature",           getXcodeBundleSignature (type));
        addPlistDictionaryKey (*dict, "CFBundleShortVersionString",  version);
        addPlistDictionaryKey (*dict, "CFBundleVersion",             version);
        addPlistDictionaryKey (*dict, "NSHumanReadableCopyright",    companyCopyright);
        addPlistDictionaryKey (*dict, "NSHighResolutionCapable", true);

        if (applicationCategory.isNotEmpty())
            addPlistDictionaryKey (*dict, "LSApplicationCategoryType", applicationCategory);

        auto replacedDocExtensions = StringArray::fromTokens (replacePreprocessorDefs (allPreprocessorDefs,
                                                                                       documentExtensions), ",", {});
        replacedDocExtensions.trim();
        replacedDocExtensions.removeEmptyStrings (true);

        if (! replacedDocExtensions.isEmpty() && type != ProjectType::Target::AudioUnitv3PlugIn)
        {
            dict->createNewChildElement ("key")->addTextElement ("CFBundleDocumentTypes");
            auto* dict2 = dict->createNewChildElement ("array")->createNewChildElement ("dict");
            XmlElement* arrayTag = nullptr;

            for (auto ex : replacedDocExtensions)
            {
                if (ex.startsWithChar ('.'))
                    ex = ex.substring (1);

                if (arrayTag == nullptr)
                {
                    dict2->createNewChildElement ("key")->addTextElement ("CFBundleTypeExtensions");
                    arrayTag = dict2->createNewChildElement ("array");

                    addPlistDictionaryKey (*dict2, "CFBundleTypeName", ex);
                    addPlistDictionaryKey (*dict2, "CFBundleTypeRole", "Editor");
                    addPlistDictionaryKey (*dict2, "CFBundleTypeIconFile", "Icon");
                    addPlistDictionaryKey (*dict2, "NSPersistentStoreTypeKey", "XML");
                }

                arrayTag->createNewChildElement ("string")->addTextElement (ex);
            }
        }

        if (fileSharingEnabled && type != ProjectType::Target::AudioUnitv3PlugIn)
            addPlistDictionaryKey (*dict, "UIFileSharingEnabled", true);

        if (documentBrowserEnabled)
            addPlistDictionaryKey (*dict, "UISupportsDocumentBrowser", true);

        if (iOS)
        {
            if (type != ProjectType::Target::AudioUnitv3PlugIn)
            {
                if (statusBarHidden)
                    addPlistDictionaryKey (*dict, "UIStatusBarHidden", true);

                addPlistDictionaryKey (*dict, "UIRequiresFullScreen", requiresFullScreen);

                addIosScreenOrientations (*dict);
                addIosBackgroundModes (*dict);
            }

            if (type == ProjectType::Target::StandalonePlugIn && enableIAA)
            {
                XmlElement audioComponentsPlistKey ("key");
                audioComponentsPlistKey.addTextElement ("AudioComponents");

                dict->addChildElement (new XmlElement (audioComponentsPlistKey));

                XmlElement audioComponentsPlistEntry ("array");
                auto* audioComponentsDict = audioComponentsPlistEntry.createNewChildElement ("dict");

                addPlistDictionaryKey (*audioComponentsDict, "name",         IAAPluginName);
                addPlistDictionaryKey (*audioComponentsDict, "manufacturer", pluginManufacturerCode.substring (0, 4));
                addPlistDictionaryKey (*audioComponentsDict, "type",         IAATypeCode);
                addPlistDictionaryKey (*audioComponentsDict, "subtype",      pluginCode.substring (0, 4));
                addPlistDictionaryKey (*audioComponentsDict, "version",      versionAsHex);

                dict->addChildElement (new XmlElement (audioComponentsPlistEntry));
            }
        }

        const auto extraOptions = [&]() -> Array<XmlElement>
        {
            if (type == ProjectType::Target::Type::AudioUnitPlugIn)
                return createExtraAudioUnitTargetPlistOptions();

            if (type == ProjectType::Target::Type::AudioUnitv3PlugIn)
                return createExtraAudioUnitV3TargetPlistOptions();

            return {};
        }();

        for (auto& e : extraOptions)
            dict->addChildElement (new XmlElement (e));

        return plist;
    }

    void PlistOptions::addIosScreenOrientations (XmlElement& dict) const
    {
        addArrayToPlist (dict, "UISupportedInterfaceOrientations", iPhoneScreenOrientations);

        if (iPadScreenOrientations != iPhoneScreenOrientations)
            addArrayToPlist (dict, "UISupportedInterfaceOrientations~ipad", iPadScreenOrientations);
    }

    void PlistOptions::addIosBackgroundModes (XmlElement& dict) const
    {
        StringArray iosBackgroundModes;
        if (backgroundAudioEnabled)     iosBackgroundModes.add ("audio");
        if (backgroundBleEnabled)       iosBackgroundModes.add ("bluetooth-central");
        if (pushNotificationsEnabled)   iosBackgroundModes.add ("remote-notification");

        addArrayToPlist (dict, "UIBackgroundModes", iosBackgroundModes);
    }

    Array<XmlElement> PlistOptions::createExtraAudioUnitTargetPlistOptions() const
    {
        XmlElement plistKey ("key");
        plistKey.addTextElement ("AudioComponents");

        XmlElement plistEntry ("array");
        auto* dict = plistEntry.createNewChildElement ("dict");

        auto truncatedCode = pluginManufacturerCode.substring (0, 4);
        auto pluginSubType = pluginCode.substring (0, 4);

        if (truncatedCode.toLowerCase() == truncatedCode)
        {
            throw SaveError ("AudioUnit plugin code identifiers invalid!\n\n"
                             "You have used only lower case letters in your AU plugin manufacturer identifier. "
                             "You must have at least one uppercase letter in your AU plugin manufacturer "
                             "identifier code.");
        }

        addPlistDictionaryKey (*dict, "name", pluginManufacturer + ": " + pluginName);
        addPlistDictionaryKey (*dict, "description", pluginDescription);
        addPlistDictionaryKey (*dict, "factoryFunction", pluginAUExportPrefix + "Factory");
        addPlistDictionaryKey (*dict, "manufacturer", truncatedCode);
        addPlistDictionaryKey (*dict, "type", auMainType.removeCharacters ("'"));
        addPlistDictionaryKey (*dict, "subtype", pluginSubType);
        addPlistDictionaryKey (*dict, "version", versionAsHex);

        if (isAuSandboxSafe)
        {
            addPlistDictionaryKey (*dict, "sandboxSafe", true);
        }
        else if (! suppressResourceUsage)
        {
            dict->createNewChildElement ("key")->addTextElement ("resourceUsage");
            auto* resourceUsageDict = dict->createNewChildElement ("dict");

            addPlistDictionaryKey (*resourceUsageDict, "network.client", true);
            addPlistDictionaryKey (*resourceUsageDict, "temporary-exception.files.all.read-write", true);
        }

        return { plistKey, plistEntry };
    }

    Array<XmlElement> PlistOptions::createExtraAudioUnitV3TargetPlistOptions() const
    {
        XmlElement plistKey ("key");
        plistKey.addTextElement ("NSExtension");

        XmlElement plistEntry ("dict");

        addPlistDictionaryKey (plistEntry, "NSExtensionPrincipalClass", pluginAUExportPrefix + "FactoryAUv3");
        addPlistDictionaryKey (plistEntry, "NSExtensionPointIdentifier", "com.apple.AudioUnit-UI");
        plistEntry.createNewChildElement ("key")->addTextElement ("NSExtensionAttributes");

        auto* dict = plistEntry.createNewChildElement ("dict");
        dict->createNewChildElement ("key")->addTextElement ("AudioComponents");
        auto* componentArray = dict->createNewChildElement ("array");

        auto* componentDict = componentArray->createNewChildElement ("dict");

        addPlistDictionaryKey (*componentDict, "name", pluginManufacturer + ": " + pluginName);
        addPlistDictionaryKey (*componentDict, "description", pluginDescription);
        addPlistDictionaryKey (*componentDict, "factoryFunction", pluginAUExportPrefix + "FactoryAUv3");
        addPlistDictionaryKey (*componentDict, "manufacturer", pluginManufacturerCode.substring (0, 4));
        addPlistDictionaryKey (*componentDict, "type", auMainType.removeCharacters ("'"));
        addPlistDictionaryKey (*componentDict, "subtype", pluginCode.substring (0, 4));
        addPlistDictionaryKey (*componentDict, "version", versionAsHex);
        addPlistDictionaryKey (*componentDict, "sandboxSafe", true);

        componentDict->createNewChildElement ("key")->addTextElement ("tags");
        auto* tagsArray = componentDict->createNewChildElement ("array");

        tagsArray->createNewChildElement ("string")
                 ->addTextElement (isPluginSynth ? "Synth" : "Effects");

        if (auMainType.removeCharacters ("'") == "aumi")
            tagsArray->createNewChildElement ("string")->addTextElement ("MIDI");

        return { plistKey, plistEntry };
    }
}
}
