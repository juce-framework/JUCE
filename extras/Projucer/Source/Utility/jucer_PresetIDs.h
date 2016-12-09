/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCER_PRESETIDS_H_INCLUDED
#define JUCER_PRESETIDS_H_INCLUDED


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
    DECLARE_ID (include);
    DECLARE_ID (info);
    DECLARE_ID (description);
    DECLARE_ID (companyName);
    DECLARE_ID (companyWebsite);
    DECLARE_ID (companyEmail);
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
    DECLARE_ID (libraryPath);
    DECLARE_ID (customXcodeFlags);
    DECLARE_ID (customXcassetsFolder);
    DECLARE_ID (customXcodeResourceFolders);
    DECLARE_ID (cppLanguageStandard);
    DECLARE_ID (cppLibType);
    DECLARE_ID (codeSigningIdentity);
    DECLARE_ID (fastMath);
    DECLARE_ID (linkTimeOptimisation);
    DECLARE_ID (stripLocalSymbols);
    DECLARE_ID (xcodeVstBinaryLocation);
    DECLARE_ID (xcodeVst3BinaryLocation);
    DECLARE_ID (xcodeAudioUnitBinaryLocation);
    DECLARE_ID (xcodeRtasBinaryLocation);
    DECLARE_ID (xcodeAaxBinaryLocation);
    DECLARE_ID (osxSDK);
    DECLARE_ID (osxCompatibility);
    DECLARE_ID (osxArchitecture);
    DECLARE_ID (iosCompatibility);
    DECLARE_ID (extraFrameworks);
    DECLARE_ID (extraDLLs);
    DECLARE_ID (winArchitecture);
    DECLARE_ID (winWarningLevel);
    DECLARE_ID (warningsAreErrors);
    DECLARE_ID (linuxArchitecture);
    DECLARE_ID (toolset);
    DECLARE_ID (IPPLibrary);
    DECLARE_ID (msvcModuleDefinitionFile);
    DECLARE_ID (bigIcon);
    DECLARE_ID (smallIcon);
    DECLARE_ID (jucerVersion);
    DECLARE_ID (prebuildCommand);
    DECLARE_ID (postbuildCommand);
    DECLARE_ID (internalPostBuildComamnd);
    DECLARE_ID (generateManifest);
    DECLARE_ID (useRuntimeLibDLL);
    DECLARE_ID (wholeProgramOptimisation);
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
    DECLARE_ID (showAllCode);
    DECLARE_ID (useLocalCopy);
    DECLARE_ID (overwriteOnSave);
    DECLARE_ID (microphonePermissionNeeded);
    DECLARE_ID (androidActivityClass);
    DECLARE_ID (androidActivitySubClassName);
    DECLARE_ID (androidVersionCode);
    DECLARE_ID (androidSDKPath);
    DECLARE_ID (androidNDKPath);
    DECLARE_ID (androidInternetNeeded);
    DECLARE_ID (androidArchitectures);
    DECLARE_ID (androidBluetoothNeeded);
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
    DECLARE_ID (iosScreenOrientation);
    DECLARE_ID (iosInAppPurchases);
    DECLARE_ID (iosBackgroundAudio);
    DECLARE_ID (iosBackgroundBle);
    DECLARE_ID (iosDevelopmentTeamID);
    DECLARE_ID (buildToolsVersion);
    DECLARE_ID (gradleVersion);
    DECLARE_ID (gradleWrapperVersion);
    DECLARE_ID (gradleToolchain);
    DECLARE_ID (gradleToolchainVersion);
    DECLARE_ID (linuxExtraPkgConfig);
    DECLARE_ID (font);
    DECLARE_ID (colour);
    DECLARE_ID (userNotes);
    DECLARE_ID (maxBinaryFileSize);
    DECLARE_ID (includeBinaryInAppConfig);
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

#endif   // JUCER_PRESETIDS_H_INCLUDED
