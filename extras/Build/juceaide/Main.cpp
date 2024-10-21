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

#include <juce_build_tools/juce_build_tools.h>

#include <fstream>
#include <unordered_map>

namespace
{

constexpr auto headerTemplate = R"(/*
    IMPORTANT! This file is auto-generated.
    If you alter its contents, your changes may be overwritten!

    This is the header file that your files should include in order to get all the
    JUCE library headers. You should avoid including the JUCE headers directly in
    your own source files, because that wouldn't pick up the correct configuration
    options for your app.

*/

#pragma once

${JUCE_INCLUDES}

#if JUCE_TARGET_HAS_BINARY_DATA
 #include "BinaryData.h"
#endif

#if ! DONT_SET_USING_JUCE_NAMESPACE
 // If your code uses a lot of JUCE classes, then this will obviously save you
 // a lot of typing, but can be disabled by setting DONT_SET_USING_JUCE_NAMESPACE.
 using namespace juce;
#endif

#if ! JUCE_DONT_DECLARE_PROJECTINFO
namespace ProjectInfo
{
    const char* const  projectName    = "${JUCE_EXECUTABLE_NAME}";
    const char* const  companyName    = "${JUCE_COMPANY_NAME}";
    const char* const  versionString  = "${JUCE_PROJECT_VERSION}";
    const int          versionNumber  =  ${JUCE_PROJECT_VERSION_HEX};
}
#endif
)";

int writeBinaryData (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (4);
    const auto namespaceName = args.arguments.removeAndReturn (0);
    const auto headerName    = args.arguments.removeAndReturn (0);
    const auto outFolder     = args.arguments.removeAndReturn (0).resolveAsExistingFolder();
    const auto inputFileList = args.arguments.removeAndReturn (0).resolveAsExistingFile();

    juce::build_tools::ResourceFile resourceFile;

    resourceFile.setClassName (namespaceName.text);
    const auto lineEndings = args.removeOptionIfFound ("--windows") ? "\r\n" : "\n";

    const auto allLines = [&]
    {
        auto lines = juce::StringArray::fromLines (inputFileList.loadFileAsString());
        lines.removeEmptyStrings();
        return lines;
    }();

    for (const auto& arg : allLines)
        resourceFile.addFile (juce::File (arg));

    const auto result = resourceFile.write (0,
                                            lineEndings,
                                            outFolder.getChildFile (headerName.text),
                                            [&outFolder] (int index)
                                            {
                                                return outFolder.getChildFile ("./BinaryData" + juce::String { index + 1 } + ".cpp");
                                            });

    if (result.result.failed())
        juce::ConsoleApplication::fail (result.result.getErrorMessage(), 1);

    return 0;
}

struct IconParseResults
{
    juce::build_tools::Icons icons;
    juce::File output;
};

IconParseResults parseIconArguments (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto output = args.arguments.removeAndReturn (0);

    const auto popDrawable = [&args]() -> std::unique_ptr<juce::Drawable>
    {
        if (args.size() == 0)
            return {};

        const auto firstArgText = args.arguments.removeAndReturn (0).text;
        return juce::Drawable::createFromImageFile (firstArgText);
    };

    auto smallIcon = popDrawable();
    auto bigIcon   = popDrawable();

    return { { std::move (smallIcon), std::move (bigIcon) }, output.text };
}

int writeMacIcon (juce::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    juce::build_tools::writeMacIcon (parsed.icons, parsed.output);
    return 0;
}

int writeiOSAssets (juce::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    juce::build_tools::createXcassetsFolderFromIcons (parsed.icons,
                                                      parsed.output.getParentDirectory(),
                                                      parsed.output.getFileName());
    return 0;
}

int writeWinIcon (juce::ArgumentList&& argumentList)
{
    const auto parsed = parseIconArguments (std::move (argumentList));
    juce::build_tools::writeWinIcon (parsed.icons, parsed.output);
    return 0;
}

std::unordered_map<juce::String, juce::String> parseProjectData (const juce::File& file)
{
    constexpr auto recordSeparator = "\x1e";
    const auto contents = file.loadFileAsString();
    const auto lines    = juce::StringArray::fromTokens (contents, recordSeparator, {});

    std::unordered_map<juce::String, juce::String> result;

    constexpr auto unitSeparator = "\x1f";

    for (const auto& line : lines)
    {
        if (line.isEmpty())
            continue;

        result.emplace (line.upToFirstOccurrenceOf (unitSeparator, false, false),
                        line.fromFirstOccurrenceOf (unitSeparator, false, false));
    }

    return result;
}

juce::String getStringValue (const std::unordered_map<juce::String, juce::String>& dict,
                             juce::StringRef key)
{
    const auto it = dict.find (key);
    return it != dict.cend() ? it->second : juce::String{};
}

bool getBoolValue (const std::unordered_map<juce::String, juce::String>& dict, juce::StringRef key)
{
    const auto str = getStringValue (dict, key);
    return str.equalsIgnoreCase ("yes")
        || str.equalsIgnoreCase ("true")
        || str.equalsIgnoreCase ("1")
        || str.equalsIgnoreCase ("on");
}

struct UpdateField
{
    const std::unordered_map<juce::String, juce::String>& dict;

    void operator() (juce::StringRef key, juce::String& value) const
    {
        value = getStringValue (dict, key);
    }

    void operator() (juce::StringRef key, juce::File& value) const
    {
        value = getStringValue (dict, key);
    }

    void operator() (juce::StringRef key, bool& value) const
    {
        value = getBoolValue (dict, key);
    }

    void operator() (juce::StringRef key, juce::StringArray& value) const
    {
        value = juce::StringArray::fromTokens (getStringValue (dict, key), ";", {});
    }
};

void setIfEmpty (juce::String& field, juce::StringRef fallback)
{
    if (field.isEmpty())
        field = fallback;
}

juce::build_tools::PlistOptions parsePlistOptions (const juce::File& file,
                                                   juce::build_tools::ProjectType::Target::Type type)
{
    if (type == juce::build_tools::ProjectType::Target::ConsoleApp)
        juce::ConsoleApplication::fail ("Deduced project type does not require a plist", 1);

    const auto dict = parseProjectData (file);

    UpdateField updateField { dict };

    juce::build_tools::PlistOptions result;

    updateField ("EXECUTABLE_NAME",                      result.executableName);
    updateField ("PLIST_TO_MERGE",                       result.plistToMerge);
    updateField ("IS_IOS",                               result.iOS);
    updateField ("MICROPHONE_PERMISSION_ENABLED",        result.microphonePermissionEnabled);
    updateField ("MICROPHONE_PERMISSION_TEXT",           result.microphonePermissionText);
    updateField ("CAMERA_PERMISSION_ENABLED",            result.cameraPermissionEnabled);
    updateField ("CAMERA_PERMISSION_TEXT",               result.cameraPermissionText);
    updateField ("BLUETOOTH_PERMISSION_ENABLED",         result.bluetoothPermissionEnabled);
    updateField ("BLUETOOTH_PERMISSION_TEXT",            result.bluetoothPermissionText);
    updateField ("SEND_APPLE_EVENTS_PERMISSION_ENABLED", result.sendAppleEventsPermissionEnabled);
    updateField ("SEND_APPLE_EVENTS_PERMISSION_TEXT",    result.sendAppleEventsPermissionText);
    updateField ("SHOULD_ADD_STORYBOARD",                result.shouldAddStoryboardToProject);
    updateField ("LAUNCH_STORYBOARD_FILE",               result.storyboardName);
    updateField ("PROJECT_NAME",                         result.projectName);
    updateField ("VERSION",                              result.marketingVersion);
    updateField ("BUILD_VERSION",                        result.currentProjectVersion);
    updateField ("COMPANY_COPYRIGHT",                    result.companyCopyright);
    updateField ("DOCUMENT_EXTENSIONS",                  result.documentExtensions);
    updateField ("FILE_SHARING_ENABLED",                 result.fileSharingEnabled);
    updateField ("DOCUMENT_BROWSER_ENABLED",             result.documentBrowserEnabled);
    updateField ("STATUS_BAR_HIDDEN",                    result.statusBarHidden);
    updateField ("REQUIRES_FULL_SCREEN",                 result.requiresFullScreen);
    updateField ("BACKGROUND_AUDIO_ENABLED",             result.backgroundAudioEnabled);
    updateField ("BACKGROUND_BLE_ENABLED",               result.backgroundBleEnabled);
    updateField ("PUSH_NOTIFICATIONS_ENABLED",           result.pushNotificationsEnabled);
    updateField ("PLUGIN_MANUFACTURER_CODE",             result.pluginManufacturerCode);
    updateField ("PLUGIN_CODE",                          result.pluginCode);
    updateField ("IPHONE_SCREEN_ORIENTATIONS",           result.iPhoneScreenOrientations);
    updateField ("IPAD_SCREEN_ORIENTATIONS",             result.iPadScreenOrientations);
    updateField ("PLUGIN_NAME",                          result.pluginName);
    updateField ("PLUGIN_MANUFACTURER",                  result.pluginManufacturer);
    updateField ("PLUGIN_DESCRIPTION",                   result.pluginDescription);
    updateField ("PLUGIN_AU_EXPORT_PREFIX",              result.pluginAUExportPrefix);
    updateField ("PLUGIN_AU_MAIN_TYPE",                  result.auMainType);
    updateField ("IS_AU_SANDBOX_SAFE",                   result.isAuSandboxSafe);
    updateField ("IS_PLUGIN_SYNTH",                      result.isPluginSynth);
    updateField ("IS_PLUGIN_ARA_EFFECT",                 result.isPluginARAEffect);
    updateField ("SUPPRESS_AU_PLIST_RESOURCE_USAGE",     result.suppressResourceUsage);
    updateField ("BUNDLE_ID",                            result.bundleIdentifier);
    updateField ("ICON_FILE",                            result.iconFile);

    result.type = type;

    if (result.storyboardName.isNotEmpty())
        result.storyboardName = result.storyboardName.fromLastOccurrenceOf ("/", false, false)
                                                     .upToLastOccurrenceOf (".storyboard", false, false);

    setIfEmpty (result.microphonePermissionText,
                "This app requires audio input. If you do not have an audio interface connected it will use the built-in microphone.");
    setIfEmpty (result.cameraPermissionText,
                "This app requires access to the camera to function correctly.");
    setIfEmpty (result.bluetoothPermissionText,
                "This app requires access to Bluetooth to function correctly.");
    setIfEmpty (result.sendAppleEventsPermissionText,
                "This app requires the ability to send Apple events to function correctly.");

    result.documentExtensions = result.documentExtensions.replace (";", ",");

    // AUv3 needs a slightly different bundle ID
    if (type == juce::build_tools::ProjectType::Target::Type::AudioUnitv3PlugIn)
    {
        const auto bundleIdSegments = juce::StringArray::fromTokens (result.bundleIdentifier, ".", {});
        jassert (! bundleIdSegments.isEmpty());

        const auto last = bundleIdSegments.isEmpty() ? ""
                                                     : bundleIdSegments[bundleIdSegments.size() - 1];

        result.bundleIdentifier += "." + last + "AUv3";
    }

    return result;
}

int writePlist (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (3);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);
    parsePlistOptions (input.resolveAsExistingFile(),
                       juce::build_tools::ProjectType::Target::typeFromName (kind.text))
        .write (output.resolveAsFile());
    return 0;
}

juce::build_tools::EntitlementOptions parseEntitlementsOptions (const juce::File& file,
                                                                juce::build_tools::ProjectType::Target::Type type)
{
    const auto dict = parseProjectData (file);

    UpdateField updateField { dict };

    juce::build_tools::EntitlementOptions result;

    updateField ("IS_IOS",                          result.isiOS);
    updateField ("IS_PLUGIN",                       result.isAudioPluginProject);
    updateField ("IS_AU_PLUGIN_HOST",               result.isAUPluginHost);
    updateField ("ICLOUD_PERMISSIONS_ENABLED",      result.isiCloudPermissionsEnabled);
    updateField ("PUSH_NOTIFICATIONS_ENABLED",      result.isPushNotificationsEnabled);
    updateField ("APP_GROUPS_ENABLED",              result.isAppGroupsEnabled);
    updateField ("APP_GROUP_IDS",                   result.appGroupIdString);
    updateField ("HARDENED_RUNTIME_ENABLED",        result.isHardenedRuntimeEnabled);
    updateField ("HARDENED_RUNTIME_OPTIONS",        result.hardenedRuntimeOptions);
    updateField ("APP_SANDBOX_ENABLED",             result.isAppSandboxEnabled);
    updateField ("APP_SANDBOX_INHERIT",             result.isAppSandboxInhertianceEnabled);
    updateField ("APP_SANDBOX_OPTIONS",             result.appSandboxOptions);
    updateField ("NETWORK_MULTICAST_ENABLED",       result.isNetworkingMulticastEnabled);

    struct SandboxTemporaryAccessKey
    {
        juce::String cMakeVar, key;
    };

    SandboxTemporaryAccessKey sandboxTemporaryAccessKeys[]
    {
        { "APP_SANDBOX_FILE_ACCESS_HOME_RO", "home-relative-path.read-only" },
        { "APP_SANDBOX_FILE_ACCESS_HOME_RW", "home-relative-path.read-write" },
        { "APP_SANDBOX_FILE_ACCESS_ABS_RO",  "absolute-path.read-only" },
        { "APP_SANDBOX_FILE_ACCESS_ABS_RW",  "absolute-path.read-write" }
    };

    for (const auto& entry : sandboxTemporaryAccessKeys)
    {
        juce::StringArray values;
        updateField (entry.cMakeVar, values);

        if (! values.isEmpty())
            result.appSandboxTemporaryPaths.push_back ({ "com.apple.security.temporary-exception.files." + entry.key,
                                                         std::move (values) });
    }

    {
        juce::StringArray values;
        updateField ("APP_SANDBOX_EXCEPTION_IOKIT", values);

        if (! values.isEmpty())
            result.appSandboxExceptionIOKit = values;
    }

    result.type = type;

    return result;
}

int writeEntitlements (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (3);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto options = parseEntitlementsOptions (input.resolveAsExistingFile(),
                                                   juce::build_tools::ProjectType::Target::typeFromName (kind.text));
    juce::build_tools::overwriteFileIfDifferentOrThrow (output.resolveAsFile(), options.getEntitlementsFileContent());
    return 0;
}

int createAndWrite (const juce::File& file, juce::StringRef text)
{
    if (file.create())
        return file.replaceWithText (text) ? 0 : 1;

    return 1;
}

int writePkgInfo (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto kind   = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto projectType = juce::build_tools::ProjectType::Target::typeFromName (kind.text);
    return createAndWrite (output.resolveAsFile(),
                           juce::build_tools::getXcodePackageType (projectType)
                           + juce::build_tools::getXcodeBundleSignature (projectType));
}

juce::build_tools::ResourceRcOptions parseRcFileOptions (const juce::File& file)
{
    const auto dict = parseProjectData (file);
    UpdateField updateField { dict };

    juce::build_tools::ResourceRcOptions result;

    updateField ("VERSION",           result.version);
    updateField ("COMPANY_NAME",      result.companyName);
    updateField ("COMPANY_COPYRIGHT", result.companyCopyright);
    updateField ("PROJECT_NAME",      result.projectName);
    updateField ("ICON_FILE",         result.icon);

    return result;
}

int writeRcFile (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);
    parseRcFileOptions (input.resolveAsExistingFile()).write (output.resolveAsFile());
    return 0;
}

juce::String createDefineStatements (juce::StringRef definitions)
{
    const auto split = juce::StringArray::fromTokens (definitions, ";", "\"");

    juce::String defineStatements;

    for (const auto& def : split)
    {
        if (! def.startsWith ("JucePlugin_"))
            continue;

        const auto defineName  = def.upToFirstOccurrenceOf ("=", false, false);
        const auto defineValue = def.fromFirstOccurrenceOf ("=", false, false);
        defineStatements += "#define " + defineName + " " + defineValue + '\n';
    }

    return defineStatements;
}

int writeAuPluginDefines (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto dict      = parseProjectData (input.resolveAsExistingFile());
    const auto getString = [&] (juce::StringRef key) { return getStringValue (dict, key); };

    const auto defines = "#pragma once\n" + createDefineStatements (getString ("MODULE_DEFINITIONS"));
    return createAndWrite (output.resolveAsFile(), defines);
}

juce::String createIncludeStatements (juce::StringRef definitions)
{
    const auto split = juce::StringArray::fromTokens (definitions, ";", "\"");

    juce::String includeStatements;

    for (const auto& def : split)
    {
        constexpr auto moduleToken = "JUCE_MODULE_AVAILABLE_";

        if (def.startsWith (moduleToken))
        {
            const auto moduleName = def.fromFirstOccurrenceOf (moduleToken, false, false)
                                       .upToFirstOccurrenceOf ("=", false, false);
            includeStatements += "#include <" + moduleName + "/" + moduleName + ".h>\n";
        }
    }

    return includeStatements;
}

int writeHeader (juce::ArgumentList&& args)
{
    args.checkMinNumArguments (2);
    const auto input  = args.arguments.removeAndReturn (0);
    const auto output = args.arguments.removeAndReturn (0);

    const auto dict = parseProjectData (input.resolveAsExistingFile());

    const auto getString = [&] (juce::StringRef key) { return getStringValue (dict, key); };

    const auto includes      = createIncludeStatements (getString ("MODULE_DEFINITIONS"));
    const auto projectName   = getString ("PROJECT_NAME");
    const auto name          = projectName.isEmpty() ? getString ("EXECUTABLE_NAME") : projectName;
    const auto versionString = getString ("VERSION");

    const auto headerText = juce::String (headerTemplate)
                                .replace ("${JUCE_INCLUDES}", includes)
                                .replace ("${JUCE_EXECUTABLE_NAME}", name)
                                .replace ("${JUCE_COMPANY_NAME}", getString ("COMPANY_NAME"))
                                .replace ("${JUCE_PROJECT_VERSION}", versionString)
                                .replace ("${JUCE_PROJECT_VERSION_HEX}", juce::build_tools::getVersionAsHex (versionString));

    return createAndWrite (output.resolveAsFile(), headerText);
}

int printJUCEVersion (juce::ArgumentList&&)
{
    std::cout << juce::SystemStats::getJUCEVersion() << std::endl;
    return 0;
}

} // namespace

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI libraryInitialiser;

    return juce::ConsoleApplication::invokeCatchingFailures ([argc, argv]
    {
        if (argc < 1)
            juce::ConsoleApplication::fail ("No arguments passed", 1);

        const auto getString = [&] (const char* text)
        {
            return juce::String (juce::CharPointer_UTF8 (text));
        };

        std::vector<juce::String> arguments;
        std::transform (argv, argv + argc, std::back_inserter (arguments), getString);

        juce::ArgumentList argumentList { arguments.front(),
                                          juce::StringArray (arguments.data() + 1, (int) arguments.size() - 1) };

        using Fn = int (*) (juce::ArgumentList&&);

        const std::unordered_map<juce::String, Fn> commands
        {
            { "auplugindefines", writeAuPluginDefines },
            { "binarydata",      writeBinaryData },
            { "entitlements",    writeEntitlements },
            { "header",          writeHeader },
            { "iosassets",       writeiOSAssets },
            { "macicon",         writeMacIcon },
            { "pkginfo",         writePkgInfo },
            { "plist",           writePlist },
            { "rcfile",          writeRcFile },
            { "version",         printJUCEVersion },
            { "winicon",         writeWinIcon }
        };

        argumentList.checkMinNumArguments (1);
        const auto mode = argumentList.arguments.removeAndReturn (0);
        const auto it   = commands.find (mode.text);

        if (it == commands.cend())
            juce::ConsoleApplication::fail ("No matching mode", 1);

        try
        {
            return it->second (std::move (argumentList));
        }
        catch (const juce::build_tools::SaveError& error)
        {
            juce::ConsoleApplication::fail (error.message);
        }
        catch (const std::exception& ex)
        {
            juce::ConsoleApplication::fail (ex.what());
        }
        catch (...)
        {
            juce::ConsoleApplication::fail ("Unhandled exception");
        }

        return 1;
    });

    return 0;
}
