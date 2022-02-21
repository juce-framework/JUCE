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
    String EntitlementOptions::getEntitlementsFileContent() const
    {
        String content =
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">\n"
            "<plist version=\"1.0\">\n"
            "<dict>\n";

        const auto entitlements = getEntitlements();

        for (auto& key : entitlements.getAllKeys())
            content += "\t<key>" + key + "</key>\n\t" + entitlements[key] + "\n";

        return content + "</dict>\n</plist>\n";
    }

    StringPairArray EntitlementOptions::getEntitlements() const
    {
        StringPairArray entitlements;

        if (isiOS)
        {
            if (isAudioPluginProject && shouldEnableIAA)
                entitlements.set ("inter-app-audio", "<true/>");

            if (isiCloudPermissionsEnabled)
            {
                entitlements.set ("com.apple.developer.icloud-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.icloud-services",
                                  "<array>\n"
                                  "        <string>CloudDocuments</string>\n"
                                  "    </array>");

                entitlements.set ("com.apple.developer.ubiquity-container-identifiers",
                                  "<array>\n"
                                  "        <string>iCloud.$(CFBundleIdentifier)</string>\n"
                                  "    </array>");
            }
        }

        if (isPushNotificationsEnabled)
            entitlements.set (isiOS ? "aps-environment"
                                    : "com.apple.developer.aps-environment",
                              "<string>development</string>");

        if (isAppGroupsEnabled)
        {
            auto appGroups = StringArray::fromTokens (appGroupIdString, ";", {});
            String groups = "<array>";

            for (auto group : appGroups)
                groups += "\n\t\t<string>" + group.trim() + "</string>";

            groups += "\n\t</array>";

            entitlements.set ("com.apple.security.application-groups", groups);
        }

        if (isHardenedRuntimeEnabled)
            for (auto& option : hardenedRuntimeOptions)
                entitlements.set (option, "<true/>");

        if (isAppSandboxEnabled || (! isiOS && isAudioPluginProject && type == ProjectType::Target::AudioUnitv3PlugIn))
        {
            entitlements.set ("com.apple.security.app-sandbox", "<true/>");

            if (isAppSandboxInhertianceEnabled)
            {
                // no other sandbox options can be specified if sandbox inheritance is enabled!
                jassert (appSandboxOptions.isEmpty());
                jassert (appSandboxTemporaryPaths.empty());

                entitlements.set ("com.apple.security.inherit", "<true/>");
            }

            if (isAppSandboxEnabled)
            {
                for (auto& option : appSandboxOptions)
                    entitlements.set (option, "<true/>");

                for (auto& option : appSandboxTemporaryPaths)
                {
                    String paths = "<array>";

                    for (const auto& path : option.values)
                        paths += "\n\t\t<string>" + path + "</string>";

                    paths += "\n\t</array>";
                    entitlements.set (option.key, paths);
                }
            }
        }

        if (isNetworkingMulticastEnabled)
            entitlements.set ("com.apple.developer.networking.multicast", "<true/>");

        return entitlements;
    }
}
}
