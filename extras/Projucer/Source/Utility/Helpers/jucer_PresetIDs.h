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

#pragma once


//==============================================================================
// Handy list of static Identifiers..
namespace Ids
{
    #define DECLARE_ID(name)      const Identifier name (#name)

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
    DECLARE_ID (displaySplashScreen);
    DECLARE_ID (reportAppUsage);
    DECLARE_ID (splashScreenColour);
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
    DECLARE_ID (defaultJuceModulePath);
    DECLARE_ID (defaultUserModulePath);
    DECLARE_ID (vst3Folder);
    DECLARE_ID (rtasFolder);
    DECLARE_ID (auFolder);
    DECLARE_ID (vst3Path);
    DECLARE_ID (rtasPath);
    DECLARE_ID (aaxPath);
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
    DECLARE_ID (optimisation);
    DECLARE_ID (defines);
    DECLARE_ID (headerPath);
    DECLARE_ID (systemHeaderPath);
    DECLARE_ID (liveWindowsTargetPlatformVersion);
    DECLARE_ID (libraryPath);
    DECLARE_ID (customXcodeFlags);
    DECLARE_ID (customXcassetsFolder);
    DECLARE_ID (customXcodeResourceFolders);
    DECLARE_ID (plistPreprocessorDefinitions);
    DECLARE_ID (cppLanguageStandard);
    DECLARE_ID (enableGNUExtensions);
    DECLARE_ID (cppLibType);
    DECLARE_ID (codeSigningIdentity);
    DECLARE_ID (fastMath);
    DECLARE_ID (linkTimeOptimisation);
    DECLARE_ID (vstBinaryLocation);
    DECLARE_ID (vst3BinaryLocation);
    DECLARE_ID (auBinaryLocation);
    DECLARE_ID (rtasBinaryLocation);
    DECLARE_ID (aaxBinaryLocation);
    DECLARE_ID (enablePluginBinaryCopyStep);
    DECLARE_ID (stripLocalSymbols);
    DECLARE_ID (osxSDK);
    DECLARE_ID (osxCompatibility);
    DECLARE_ID (osxArchitecture);
    DECLARE_ID (iosCompatibility);
    DECLARE_ID (extraFrameworks);
    DECLARE_ID (extraDLLs);
    DECLARE_ID (winArchitecture);
    DECLARE_ID (winWarningLevel);
    DECLARE_ID (msvcManifestFile);
    DECLARE_ID (warningsAreErrors);
    DECLARE_ID (linuxArchitecture);
    DECLARE_ID (linuxCodeBlocksArchitecture);
    DECLARE_ID (windowsCodeBlocksArchitecture);
    DECLARE_ID (toolset);
    DECLARE_ID (windowsTargetPlatformVersion);
    DECLARE_ID (debugInformationFormat);
    DECLARE_ID (IPPLibrary);
    DECLARE_ID (msvcModuleDefinitionFile);
    DECLARE_ID (bigIcon);
    DECLARE_ID (smallIcon);
    DECLARE_ID (jucerVersion);
    DECLARE_ID (prebuildCommand);
    DECLARE_ID (postbuildCommand);
    DECLARE_ID (generateManifest);
    DECLARE_ID (useRuntimeLibDLL);
    DECLARE_ID (enableIncrementalLinking);
    DECLARE_ID (buildVST);
    DECLARE_ID (bundleIdentifier);
    DECLARE_ID (aaxIdentifier);
    DECLARE_ID (aaxCategory);
    DECLARE_ID (aaxFolder);
    DECLARE_ID (compile);
    DECLARE_ID (noWarnings);
    DECLARE_ID (resource);
    DECLARE_ID (xcodeResource);
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
    DECLARE_ID (microphonePermissionNeeded);
    DECLARE_ID (androidRepositories);
    DECLARE_ID (androidDependencies);
    DECLARE_ID (androidAdditionalXmlValueResources);
    DECLARE_ID (androidAdditionalRawValueResources);
    DECLARE_ID (androidActivityClass);
    DECLARE_ID (androidActivitySubClassName);
    DECLARE_ID (androidVersionCode);
    DECLARE_ID (androidSDKPath);
    DECLARE_ID (androidNDKPath);
    DECLARE_ID (androidInternetNeeded);
    DECLARE_ID (androidArchitectures);
    DECLARE_ID (androidManifestCustomXmlElements);
    DECLARE_ID (androidCustomStringXmlElements);
    DECLARE_ID (androidBluetoothNeeded);
    DECLARE_ID (androidExternalReadNeeded);
    DECLARE_ID (androidExternalWriteNeeded);
    DECLARE_ID (androidInAppBilling);
    DECLARE_ID (androidVibratePermissionNeeded);
    DECLARE_ID (androidEnableRemoteNotifications);
    DECLARE_ID (androidRemoteNotificationsConfigFile);
    DECLARE_ID (androidMinimumSDK);
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
    DECLARE_ID (iosDeviceFamily);
    const Identifier iPhoneScreenOrientation ("iosScreenOrientation"); // old name is confusing
    DECLARE_ID (iPadScreenOrientation);
    DECLARE_ID (iosScreenOrientation);
    DECLARE_ID (iosInAppPurchases);
    DECLARE_ID (iosBackgroundAudio);
    DECLARE_ID (iosBackgroundBle);
    DECLARE_ID (iosPushNotifications);
    DECLARE_ID (iosAppGroups);
    DECLARE_ID (iosDevelopmentTeamID);
    DECLARE_ID (iosAppGroupsId);
    DECLARE_ID (iosAppExtensionDuplicateResourcesFolder);
    DECLARE_ID (buildToolsVersion);
    DECLARE_ID (gradleVersion);
    const Identifier androidPluginVersion ("gradleWrapperVersion"); // old name is very confusing, but we need to remain backward compatible
    DECLARE_ID (gradleToolchain);
    DECLARE_ID (gradleToolchainVersion);
    DECLARE_ID (linuxExtraPkgConfig);
    DECLARE_ID (clionMakefileEnabled);
    DECLARE_ID (clionXcodeEnabled);
    DECLARE_ID (clionCodeBlocksEnabled);
    DECLARE_ID (font);
    DECLARE_ID (colour);
    DECLARE_ID (userNotes);
    DECLARE_ID (maxBinaryFileSize);
    DECLARE_ID (includeBinaryInAppConfig);
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

    const Identifier ID ("id");
    const Identifier ID_uppercase ("ID");
    const Identifier class_ ("class");

    #undef DECLARE_ID
}
