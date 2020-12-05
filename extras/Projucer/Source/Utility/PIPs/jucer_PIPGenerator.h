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

#pragma once

#include "../Helpers/jucer_MiscUtilities.h"
#include "../../Project/Modules/jucer_AvailableModulesList.h"

//==============================================================================
class PIPGenerator
{
public:
    PIPGenerator (const File& pipFile, const File& outputDirectory = {},
                  const File& pathToJUCEModules = {}, const File& pathToUserModules = {});

    //==============================================================================
    bool hasValidPIP() const noexcept                   { return ! metadata[Ids::name].toString().isEmpty(); }
    File getJucerFile() const noexcept                  { return outputDirectory.getChildFile (metadata[Ids::name].toString() + ".jucer"); }
    File getPIPFile() const noexcept                    { return useLocalCopy ? outputDirectory.getChildFile ("Source").getChildFile (pipFile.getFileName()) : pipFile; }

    String getMainClassName() const noexcept            { return metadata[Ids::mainClass]; }

    File getOutputDirectory() const noexcept            { return outputDirectory; }

    //==============================================================================
    Result createJucerFile();
    Result createMainCpp();

private:
    //==============================================================================
    void addFileToTree (ValueTree& groupTree, const String& name, bool compile, const String& path);
    void createFiles (ValueTree& jucerTree);

    ValueTree createModulePathChild (const String& moduleID);
    ValueTree createBuildConfigChild (bool isDebug);
    ValueTree createExporterChild (const Identifier& exporterIdentifier);
    ValueTree createModuleChild (const String& moduleID);

    void addExporters (ValueTree& jucerTree);
    void addModules (ValueTree& jucerTree);

    Result setProjectSettings (ValueTree& jucerTree);

    void setModuleFlags (ValueTree& jucerTree);

    String getMainFileTextForType();

    //==============================================================================
    Array<File> replaceRelativeIncludesAndGetFilesToMove();
    bool copyRelativeFileToLocalSourceDirectory (const File&) const noexcept;

    StringArray getExtraPluginFormatsToBuild() const;

    String getPathForModule (const String&) const;
    File getExamplesDirectory() const;

    //==============================================================================
    File pipFile, outputDirectory, juceModulesPath, userModulesPath;
    std::unique_ptr<AvailableModulesList> availableUserModules;
    var metadata;
    bool isTemp = false, useLocalCopy = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PIPGenerator)
};
