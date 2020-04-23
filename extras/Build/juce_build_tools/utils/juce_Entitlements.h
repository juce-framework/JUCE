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

namespace juce
{
namespace build_tools
{
    struct EntitlementOptions final
    {
        String getEntitlementsFileContent() const;

        ProjectType::Target::Type type      = ProjectType::Target::GUIApp;

        bool isiOS                          = false;
        bool isAudioPluginProject           = false;
        bool shouldEnableIAA                = false;
        bool isiCloudPermissionsEnabled     = false;
        bool isPushNotificationsEnabled     = false;
        bool isAppGroupsEnabled             = false;
        bool isHardenedRuntimeEnabled       = false;
        bool isAppSandboxEnabled            = false;
        bool isAppSandboxInhertianceEnabled = false;

        String appGroupIdString;

        StringArray hardenedRuntimeOptions;
        StringArray appSandboxOptions;

    private:
        StringPairArray getEntitlements() const;
    };
}
}
