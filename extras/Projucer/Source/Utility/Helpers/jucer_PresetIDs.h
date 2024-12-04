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


//==============================================================================
// Handy list of static Identifiers..
namespace Ids
{
    #define DECLARE_ID(name)  const Identifier name (#name)

    DECLARE_ID (name);
    DECLARE_ID (file);
    DECLARE_ID (path);
    DECLARE_ID (text);
    DECLARE_ID (vendor);
    DECLARE_ID (version);
    DECLARE_ID (license);
    DECLARE_ID (minimumCppStandard);
    DECLARE_ID (include);
    DECLARE_ID (info);
    DECLARE_ID (description);
    DECLARE_ID (companyName);
    DECLARE_ID (companyCopyright);
    DECLARE_ID (companyWebsite);
    DECLARE_ID (companyEmail);
    DECLARE_ID (useAppConfig);
    DECLARE_ID (addUsingNamespaceToJuceHeader);
    DECLARE_ID (usePrecompiledHeaderFile);
    DECLARE_ID (precompiledHeaderFile);
    DECLARE_ID (position);
    DECLARE_ID (source);
    DECLARE_ID (width);
    DECLARE_ID (height);
    DECLARE_ID (bounds);
    DECLARE_ID (background);
    DECLARE_ID (initialState);
    DECLARE_ID (targetFolder);
    DECLARE_ID (intermediatesPath);
    DECLARE_ID (modulePaths);
    DECLARE_ID (searchpaths);
    DECLARE_ID (osxFallback);
    DECLARE_ID (windowsFallback);
    DECLARE_ID (linuxFallback);
    DECLARE_ID (jucePath);
    DECLARE_ID (defaultJuceModulePath);
    DECLARE_ID (defaultUserModulePath);
    DECLARE_ID (vstLegacyFolder);
    DECLARE_ID (vst3Folder);
    DECLARE_ID (auFolder);
    DECLARE_ID (vstLegacyPath);
    DECLARE_ID (aaxPath);
    DECLARE_ID (araPath);
    DECLARE_ID (flags);
    DECLARE_ID (line);
    DECLARE_ID (index);
    DECLARE_ID (type);
    DECLARE_ID (time);
    DECLARE_ID (extraCompilerFlags);
    DECLARE_ID (extraLinkerFlags);
    DECLARE_ID (externalLibraries);
    DECLARE_ID (extraDefs);
    DECLARE_ID (projectType);
    DECLARE_ID (isDebug);
    DECLARE_ID (alwaysGenerateDebugSymbols);
    DECLARE_ID (targetName);
    DECLARE_ID (binaryPath);
    DECLARE_ID (recommendedWarnings);
    DECLARE_ID (optimisation);
    DECLARE_ID (defines);
    DECLARE_ID (headerPath);
    DECLARE_ID (systemHeaderPath);
    DECLARE_ID (liveWindowsTargetPlatformVersion);
    DECLARE_ID (libraryPath);
    DECLARE_ID (customXcodeFlags);
    DECLARE_ID (customXcassetsFolder);
    DECLARE_ID (customLaunchStoryboard);
    DECLARE_ID (customXcodeResourceFolders);
    DECLARE_ID (plistPreprocessorDefinitions);
    DECLARE_ID (applicationCategory);
    DECLARE_ID (customPList);
    DECLARE_ID (pListPrefixHeader);
    DECLARE_ID (pListPreprocess);
    DECLARE_ID (UIFileSharingEnabled);
    DECLARE_ID (UISupportsDocumentBrowser);
    DECLARE_ID (UIStatusBarHidden);
    DECLARE_ID (UIRequiresFullScreen);
    DECLARE_ID (documentExtensions);
    DECLARE_ID (keepCustomXcodeSchemes);
    DECLARE_ID (useHeaderMap);
    DECLARE_ID (cppLanguageStandard);
    DECLARE_ID (enableGNUExtensions);
    DECLARE_ID (cppLibType);
    DECLARE_ID (codeSigningIdentity);
    DECLARE_ID (fastMath);
    DECLARE_ID (linkTimeOptimisation);
    DECLARE_ID (vstBinaryLocation);
    DECLARE_ID (vst3BinaryLocation);
    DECLARE_ID (auBinaryLocation);
    DECLARE_ID (aaxBinaryLocation);
    DECLARE_ID (unityPluginBinaryLocation);
    DECLARE_ID (enablePluginBinaryCopyStep);
    DECLARE_ID (stripLocalSymbols);
    DECLARE_ID (macOSBaseSDK);
    DECLARE_ID (macOSDeploymentTarget);
    DECLARE_ID (osxArchitecture);
    DECLARE_ID (iosBaseSDK);
    DECLARE_ID (iosDeploymentTarget);
    DECLARE_ID (xcodeSubprojects);
    DECLARE_ID (extraFrameworks);
    DECLARE_ID (frameworkSearchPaths);
    DECLARE_ID (extraCustomFrameworks);
    DECLARE_ID (embeddedFrameworks);
    DECLARE_ID (extraDLLs);
    DECLARE_ID (winArchitecture);
    DECLARE_ID (winWarningLevel);
    DECLARE_ID (msvcManifestFile);
    DECLARE_ID (warningsAreErrors);
    DECLARE_ID (linuxArchitecture);
    DECLARE_ID (toolset);
    DECLARE_ID (windowsTargetPlatformVersion);
    DECLARE_ID (debugInformationFormat);
    DECLARE_ID (IPPLibrary);
    DECLARE_ID (IPP1ALibrary);
    DECLARE_ID (MKL1ALibrary);
    DECLARE_ID (msvcModuleDefinitionFile);
    DECLARE_ID (bigIcon);
    DECLARE_ID (smallIcon);
    DECLARE_ID (prebuildCommand);
    DECLARE_ID (postbuildCommand);
    DECLARE_ID (useRuntimeLibDLL);
    DECLARE_ID (multiProcessorCompilation);
    DECLARE_ID (enableIncrementalLinking);
    DECLARE_ID (bundleIdentifier);
    DECLARE_ID (aaxIdentifier);
    DECLARE_ID (araFactoryID);
    DECLARE_ID (araDocumentArchiveID);
    DECLARE_ID (araCompatibleArchiveIDs);
    DECLARE_ID (aaxFolder);
    DECLARE_ID (araFolder);
    DECLARE_ID (compile);
    DECLARE_ID (noWarnings);
    DECLARE_ID (skipPCH);
    DECLARE_ID (resource);
    DECLARE_ID (xcodeResource);
    DECLARE_ID (xcodeValidArchs);
    DECLARE_ID (className);
    DECLARE_ID (classDesc);
    DECLARE_ID (controlPoint);
    DECLARE_ID (createCallback);
    DECLARE_ID (parentClasses);
    DECLARE_ID (constructorParams);
    DECLARE_ID (objectConstructionArgs);
    DECLARE_ID (memberInitialisers);
    DECLARE_ID (canBeAggregated);
    DECLARE_ID (rootItemVisible);
    DECLARE_ID (openByDefault);
    DECLARE_ID (locked);
    DECLARE_ID (tooltip);
    DECLARE_ID (memberName);
    DECLARE_ID (markerName);
    DECLARE_ID (focusOrder);
    DECLARE_ID (hidden);
    DECLARE_ID (useStdCall);
    DECLARE_ID (useGlobalPath);
    DECLARE_ID (showAllCode);
    DECLARE_ID (useLocalCopy);
    DECLARE_ID (overwriteOnSave);
    DECLARE_ID (appSandbox);
    DECLARE_ID (appSandboxInheritance);
    DECLARE_ID (appSandboxOptions);
    DECLARE_ID (appSandboxHomeDirRO);
    DECLARE_ID (appSandboxHomeDirRW);
    DECLARE_ID (appSandboxAbsDirRO);
    DECLARE_ID (appSandboxAbsDirRW);
    DECLARE_ID (appSandboxExceptionIOKit);
    DECLARE_ID (hardenedRuntime);
    DECLARE_ID (hardenedRuntimeOptions);
    DECLARE_ID (microphonePermissionNeeded);
    DECLARE_ID (microphonePermissionsText);
    DECLARE_ID (cameraPermissionNeeded);
    DECLARE_ID (cameraPermissionText);
    DECLARE_ID (sendAppleEventsPermissionNeeded);
    DECLARE_ID (sendAppleEventsPermissionText);
    DECLARE_ID (androidJavaLibs);
    DECLARE_ID (androidAdditionalJavaFolders);
    DECLARE_ID (androidAdditionalResourceFolders);
    DECLARE_ID (androidProjectRepositories);
    DECLARE_ID (androidRepositories);
    DECLARE_ID (androidDependencies);
    DECLARE_ID (androidCustomAppBuildGradleContent);
    DECLARE_ID (androidBuildConfigRemoteNotifsConfigFile);
    DECLARE_ID (androidAdditionalXmlValueResources);
    DECLARE_ID (androidAdditionalDrawableResources);
    DECLARE_ID (androidAdditionalRawValueResources);
//    DECLARE_ID (androidActivityClass);  // DEPRECATED!
    const Identifier androidCustomActivityClass ("androidActivitySubClassName"); // old name is very confusing, but we need to remain backward compatible
//    DECLARE_ID (androidActivityBaseClassName); // DEPRECATED!
    DECLARE_ID (androidCustomApplicationClass);
    DECLARE_ID (androidVersionCode);
    DECLARE_ID (androidSDKPath);
    DECLARE_ID (androidOboeRepositoryPath);
    DECLARE_ID (androidInternetNeeded);
    DECLARE_ID (androidArchitectures);
    DECLARE_ID (androidManifestCustomXmlElements);
    DECLARE_ID (androidGradleSettingsContent);
    DECLARE_ID (androidCustomStringXmlElements);
    DECLARE_ID (androidBluetoothNeeded);
    DECLARE_ID (androidBluetoothScanNeeded);
    DECLARE_ID (androidBluetoothAdvertiseNeeded);
    DECLARE_ID (androidBluetoothConnectNeeded);
    DECLARE_ID (androidExternalReadNeeded);
    DECLARE_ID (androidReadMediaAudioPermission);
    DECLARE_ID (androidReadMediaImagesPermission);
    DECLARE_ID (androidReadMediaVideoPermission);
    DECLARE_ID (androidExternalWriteNeeded);
    DECLARE_ID (androidInAppBilling);
    DECLARE_ID (androidVibratePermissionNeeded);
    DECLARE_ID (androidPushNotifications);
    DECLARE_ID (androidEnableRemoteNotifications);
    DECLARE_ID (androidRemoteNotificationsConfigFile);
    DECLARE_ID (androidEnableContentSharing);
    DECLARE_ID (androidMinimumSDK);
    DECLARE_ID (androidTargetSDK);
    DECLARE_ID (androidOtherPermissions);
    DECLARE_ID (androidKeyStore);
    DECLARE_ID (androidKeyStorePass);
    DECLARE_ID (androidKeyAlias);
    DECLARE_ID (androidKeyAliasPass);
    DECLARE_ID (androidTheme);
    DECLARE_ID (androidStaticLibraries);
    DECLARE_ID (androidSharedLibraries);
    DECLARE_ID (androidScreenOrientation);
    DECLARE_ID (androidExtraAssetsFolder);
    DECLARE_ID (androidStudioExePath);
    DECLARE_ID (iosDeviceFamily);
    const Identifier iPhoneScreenOrientation ("iosScreenOrientation"); // old name is confusing
    DECLARE_ID (iPadScreenOrientation);
    DECLARE_ID (iosScreenOrientation);
    DECLARE_ID (iosInAppPurchases);
    DECLARE_ID (iosContentSharing);
    DECLARE_ID (iosBackgroundAudio);
    DECLARE_ID (iosBackgroundBle);
    DECLARE_ID (iosPushNotifications);
    DECLARE_ID (iosAppGroups);
    DECLARE_ID (iCloudPermissions);
    DECLARE_ID (networkingMulticast);
    DECLARE_ID (iosDevelopmentTeamID);
    DECLARE_ID (iosAppGroupsId);
    DECLARE_ID (iosBluetoothPermissionNeeded);
    DECLARE_ID (iosBluetoothPermissionText);
    DECLARE_ID (duplicateAppExResourcesFolder);
    DECLARE_ID (buildToolsVersion);
    DECLARE_ID (gradleVersion);
    const Identifier androidPluginVersion ("gradleWrapperVersion"); // old name is very confusing, but we need to remain backward compatible
    DECLARE_ID (gradleToolchain);
    DECLARE_ID (gradleToolchainVersion);
    DECLARE_ID (gradleClangTidy);
    DECLARE_ID (linuxExtraPkgConfig);
    DECLARE_ID (font);
    DECLARE_ID (colour);
    DECLARE_ID (userNotes);
    DECLARE_ID (maxBinaryFileSize);
    DECLARE_ID (includeBinaryInJuceHeader);
    DECLARE_ID (binaryDataNamespace);
    DECLARE_ID (characterSet);
    DECLARE_ID (JUCERPROJECT);
    DECLARE_ID (MAINGROUP);
    DECLARE_ID (EXPORTFORMATS);
    DECLARE_ID (GROUP);
    DECLARE_ID (FILE);
    DECLARE_ID (MODULES);
    DECLARE_ID (MODULE);
    DECLARE_ID (JUCEOPTIONS);
    DECLARE_ID (CONFIGURATIONS);
    DECLARE_ID (CONFIGURATION);
    DECLARE_ID (MODULEPATHS);
    DECLARE_ID (MODULEPATH);
    DECLARE_ID (PATH);
    DECLARE_ID (userpath);
    DECLARE_ID (systempath);
    DECLARE_ID (utilsCppInclude);
    DECLARE_ID (juceModulesFolder);
    DECLARE_ID (parentActive);
    DECLARE_ID (message);
    DECLARE_ID (start);
    DECLARE_ID (end);
    DECLARE_ID (range);
    DECLARE_ID (location);
    DECLARE_ID (key);
    DECLARE_ID (list);
    DECLARE_ID (METADATA);
    DECLARE_ID (DEPENDENCIES);
    DECLARE_ID (CLASSLIST);
    DECLARE_ID (CLASS);
    DECLARE_ID (MEMBER);
    DECLARE_ID (METHOD);
    DECLARE_ID (LITERALS);
    DECLARE_ID (LITERAL);
    DECLARE_ID (abstract);
    DECLARE_ID (anonymous);
    DECLARE_ID (noDefConstructor);
    DECLARE_ID (returnType);
    DECLARE_ID (numArgs);
    DECLARE_ID (declaration);
    DECLARE_ID (definition);
    DECLARE_ID (classDecl);
    DECLARE_ID (initialisers);
    DECLARE_ID (destructors);
    DECLARE_ID (pluginFormats);
    DECLARE_ID (buildVST);
    DECLARE_ID (buildVST3);
    DECLARE_ID (buildAU);
    DECLARE_ID (buildAUv3);
    DECLARE_ID (buildAAX);
    DECLARE_ID (buildStandalone);
    DECLARE_ID (buildUnity);
    DECLARE_ID (buildLV2);
    DECLARE_ID (enableIAA);
    DECLARE_ID (enableARA);
    DECLARE_ID (pluginName);
    DECLARE_ID (pluginDesc);
    DECLARE_ID (pluginManufacturer);
    DECLARE_ID (pluginManufacturerCode);
    DECLARE_ID (pluginCode);
    DECLARE_ID (pluginChannelConfigs);
    DECLARE_ID (pluginCharacteristicsValue);
    DECLARE_ID (pluginCharacteristics);
    DECLARE_ID (extraPluginFormats);
    DECLARE_ID (pluginIsSynth);
    DECLARE_ID (pluginWantsMidiIn);
    DECLARE_ID (pluginProducesMidiOut);
    DECLARE_ID (pluginIsMidiEffectPlugin);
    DECLARE_ID (pluginEditorRequiresKeys);
    DECLARE_ID (pluginVSTCategory);
    DECLARE_ID (pluginVST3Category);
    DECLARE_ID (pluginAUExportPrefix);
    DECLARE_ID (pluginAUMainType);
    DECLARE_ID (pluginAUIsSandboxSafe);
    DECLARE_ID (pluginAAXCategory);
    DECLARE_ID (pluginAAXDisableBypass);
    DECLARE_ID (pluginAAXDisableMultiMono);
    DECLARE_ID (pluginARAAnalyzableContent);
    DECLARE_ID (pluginARATransformFlags);
    DECLARE_ID (pluginVSTNumMidiInputs);
    DECLARE_ID (pluginVSTNumMidiOutputs);
    DECLARE_ID (suppressPlistResourceUsage);
    DECLARE_ID (useLegacyBuildSystem);
    DECLARE_ID (exporters);
    DECLARE_ID (website);
    DECLARE_ID (mainClass);
    DECLARE_ID (documentControllerClass);
    DECLARE_ID (moduleFlags);
    DECLARE_ID (projectLineFeed);
    DECLARE_ID (compilerFlagSchemes);
    DECLARE_ID (compilerFlagScheme);
    DECLARE_ID (dontQueryForUpdate);
    DECLARE_ID (dontAskAboutJUCEPath);
    DECLARE_ID (postExportShellCommandPosix);
    DECLARE_ID (postExportShellCommandWin);
    DECLARE_ID (jucerFormatVersion);
    DECLARE_ID (buildNumber);
    DECLARE_ID (lv2Uri);
    DECLARE_ID (lv2UriUi);
    DECLARE_ID (lv2BinaryLocation);
    DECLARE_ID (vst3ManifestEnabled);

    DECLARE_ID (osxSDK);
    DECLARE_ID (osxCompatibility);
    DECLARE_ID (iosCompatibility);

    const Identifier ID ("id");
    const Identifier ID_uppercase ("ID");
    const Identifier class_ ("class");
    const Identifier dependencies_ ("dependencies");

    #undef DECLARE_ID
}
