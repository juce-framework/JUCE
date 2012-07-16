/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_PRESETIDS_JUCEHEADER__
#define __JUCER_PRESETIDS_JUCEHEADER__


// Handy list of static Identifiers..
namespace Ids
{
    #define DECLARE_ID(name)      const Identifier name (#name)

    DECLARE_ID (name);
    DECLARE_ID (file);
    DECLARE_ID (text);
    DECLARE_ID (version);
    DECLARE_ID (companyName);
    DECLARE_ID (position);
    DECLARE_ID (source);
    DECLARE_ID (width);
    DECLARE_ID (height);
    DECLARE_ID (background);
    DECLARE_ID (initialState);
    DECLARE_ID (juceFolder);
    DECLARE_ID (targetFolder);
    DECLARE_ID (vstFolder);
    DECLARE_ID (rtasFolder);
    DECLARE_ID (auFolder);
    DECLARE_ID (flags);
    DECLARE_ID (line);
    DECLARE_ID (index);
    DECLARE_ID (type);
    DECLARE_ID (extraCompilerFlags);
    DECLARE_ID (extraLinkerFlags);
    DECLARE_ID (extraDefs);
    DECLARE_ID (projectType);
    DECLARE_ID (libraryType);
    DECLARE_ID (isDebug);
    DECLARE_ID (targetName);
    DECLARE_ID (binaryPath);
    DECLARE_ID (optimisation);
    DECLARE_ID (defines);
    DECLARE_ID (headerPath);
    DECLARE_ID (libraryPath);
    DECLARE_ID (customXcodeFlags);
    DECLARE_ID (cppLibType);
    DECLARE_ID (osxSDK);
    DECLARE_ID (osxCompatibility);
    DECLARE_ID (osxArchitecture);
    DECLARE_ID (iosCompatibility);
    DECLARE_ID (extraFrameworks);
    DECLARE_ID (winArchitecture);
    DECLARE_ID (winWarningLevel);
    DECLARE_ID (msvcModuleDefinitionFile);
    DECLARE_ID (bigIcon);
    DECLARE_ID (smallIcon);
    DECLARE_ID (jucerVersion);
    DECLARE_ID (prebuildCommand);
    DECLARE_ID (postbuildCommand);
    DECLARE_ID (generateManifest);
    DECLARE_ID (wholeProgramOptimisation);
    DECLARE_ID (juceLinkage);
    DECLARE_ID (buildVST);
    DECLARE_ID (bundleIdentifier);
    DECLARE_ID (aaxIdentifier);
    DECLARE_ID (aaxCategory);
    DECLARE_ID (aaxFolder);
    DECLARE_ID (compile);
    DECLARE_ID (noWarnings);
    DECLARE_ID (resource);
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
    DECLARE_ID (androidActivityClass);
    DECLARE_ID (androidSDKPath);
    DECLARE_ID (androidNDKPath);
    DECLARE_ID (androidInternetNeeded);
    DECLARE_ID (androidArchitectures);
    DECLARE_ID (androidCpp11);
    DECLARE_ID (androidMicNeeded);
    DECLARE_ID (androidMinimumSDK);
    DECLARE_ID (androidOtherPermissions);
    DECLARE_ID (androidKeyStore);
    DECLARE_ID (androidKeyStorePass);
    DECLARE_ID (androidKeyAlias);
    DECLARE_ID (androidKeyAliasPass);
    DECLARE_ID (font);
    DECLARE_ID (colour);
    const Identifier class_ ("class");

    #undef DECLARE_ID
}

#endif   // __JUCER_PRESETIDS_JUCEHEADER__
