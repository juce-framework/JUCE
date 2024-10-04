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

#include "../../Application/jucer_Headers.h"
#include "../../ProjectSaving/jucer_ProjectExporter.h"
#include "../../ProjectSaving/jucer_ProjectExport_Xcode.h"
#include "../../ProjectSaving/jucer_ProjectExport_Android.h"
#include "../../ProjectSaving/jucer_ProjectExport_MSVC.h"
#include "jucer_PIPGenerator.h"

//==============================================================================
static void ensureSingleNewLineAfterIncludes (StringArray& lines)
{
    int lastIncludeIndex = -1;

    for (int i = 0; i < lines.size(); ++i)
    {
        if (lines[i].contains ("#include"))
            lastIncludeIndex = i;
    }

    if (lastIncludeIndex != -1)
    {
        auto index = lastIncludeIndex;
        int numNewLines = 0;

        while (++index < lines.size() && lines[index].isEmpty())
            ++numNewLines;

        if (numNewLines > 1)
            lines.removeRange (lastIncludeIndex + 1, numNewLines - 1);
    }
}

static String ensureCorrectWhitespace (StringRef input)
{
    auto lines = StringArray::fromLines (input);
    ensureSingleNewLineAfterIncludes (lines);
    return joinLinesIntoSourceFile (lines);
}

static bool isJUCEExample (const File& pipFile)
{
    const auto numLinesToTest = 10; // license should be at the top of the file so no need to check all lines
    const auto lines = StringArray::fromLines (pipFile.loadFileAsString());

    return std::any_of (lines.begin(),
                        lines.begin() + (std::min (lines.size(), numLinesToTest)),
                        [] (const auto& line)
                        {
                            return line.contains ("This file is part of the JUCE framework examples");
                        });
}

static bool isValidExporterIdentifier (const Identifier& exporterIdentifier)
{
    return ProjectExporter::getTypeInfoForExporter (exporterIdentifier).identifier.toString().isNotEmpty();
}

static bool exporterRequiresExampleAssets (const Identifier& exporterIdentifier, const String& projectName)
{
    return (exporterIdentifier.toString() == XcodeProjectExporter::getValueTreeTypeNameiOS()
            || exporterIdentifier.toString() == AndroidProjectExporter::getValueTreeTypeName())
            || (exporterIdentifier.toString() == XcodeProjectExporter::getValueTreeTypeNameMac() && projectName == "AUv3SynthPlugin");
}

//==============================================================================
PIPGenerator::PIPGenerator (const File& pip, const File& output, const File& jucePath, const File& userPath)
    : pipFile (pip),
      juceModulesPath (jucePath),
      userModulesPath (userPath),
      metadata (parseJUCEHeaderMetadata (pipFile))
{
    if (output != File())
    {
        outputDirectory = output;
        isTemp = false;
    }
    else
    {
        outputDirectory = File::getSpecialLocation (File::SpecialLocationType::tempDirectory).getChildFile ("PIPs");
        isTemp = true;
    }

    auto isClipboard = (pip.getParentDirectory().getFileName() == "Clipboard"
                        && pip.getParentDirectory().getParentDirectory().getFileName() == "PIPs");

    outputDirectory = outputDirectory.getChildFile (metadata[Ids::name].toString()).getNonexistentSibling();
    useLocalCopy = metadata[Ids::useLocalCopy].toString().trim().getIntValue() == 1 || isClipboard;

    if (userModulesPath != File())
    {
        availableUserModules.reset (new AvailableModulesList());
        availableUserModules->scanPaths ({ userModulesPath });
    }
}

//==============================================================================
Result PIPGenerator::createJucerFile()
{
    ValueTree root (Ids::JUCERPROJECT);

    auto result = setProjectSettings (root);

    if (result != Result::ok())
        return result;

    addModules     (root);
    addExporters   (root);
    createFiles    (root);
    setModuleFlags (root);

    auto outputFile = outputDirectory.getChildFile (metadata[Ids::name].toString() + ".jucer");

    if (auto xml = root.createXml())
        if (xml->writeTo (outputFile, {}))
            return Result::ok();

    return Result::fail ("Failed to create .jucer file in " + outputDirectory.getFullPathName());
}

Result PIPGenerator::createMainCpp()
{
    auto outputFile = outputDirectory.getChildFile ("Source").getChildFile ("Main.cpp");

    if (! outputFile.existsAsFile() && (outputFile.create() != Result::ok()))
        return Result::fail ("Failed to create Main.cpp - " + outputFile.getFullPathName());

    outputFile.replaceWithText (getMainFileTextForType());

    return Result::ok();
}

//==============================================================================
void PIPGenerator::addFileToTree (ValueTree& groupTree, const String& name, bool compile, const String& path)
{
    ValueTree file (Ids::FILE);
    file.setProperty (Ids::ID, createAlphaNumericUID(), nullptr);
    file.setProperty (Ids::name, name, nullptr);
    file.setProperty (Ids::compile, compile, nullptr);
    file.setProperty (Ids::resource, 0, nullptr);
    file.setProperty (Ids::file, path, nullptr);

    groupTree.addChild (file, -1, nullptr);
}

void PIPGenerator::createFiles (ValueTree& jucerTree)
{
    auto sourceDir = outputDirectory.getChildFile ("Source");

    if (! sourceDir.exists())
        sourceDir.createDirectory();

    if (useLocalCopy)
        pipFile.copyFileTo (sourceDir.getChildFile (pipFile.getFileName()));

    ValueTree mainGroup (Ids::MAINGROUP);
    mainGroup.setProperty (Ids::ID, createAlphaNumericUID(), nullptr);
    mainGroup.setProperty (Ids::name, metadata[Ids::name], nullptr);

    ValueTree group (Ids::GROUP);
    group.setProperty (Ids::ID, createGUID (sourceDir.getFullPathName() + "_guidpathsaltxhsdf"), nullptr);
    group.setProperty (Ids::name, "Source", nullptr);

    addFileToTree (group, "Main.cpp", true, "Source/Main.cpp");
    addFileToTree (group, pipFile.getFileName(), false, useLocalCopy ? "Source/" + pipFile.getFileName()
                                                                     : pipFile.getFullPathName());

    mainGroup.addChild (group, -1, nullptr);

    if (useLocalCopy)
    {
        auto relativeFiles = replaceRelativeIncludesAndGetFilesToMove();

        if (relativeFiles.size() > 0)
        {
            ValueTree assets (Ids::GROUP);
            assets.setProperty (Ids::ID, createAlphaNumericUID(), nullptr);
            assets.setProperty (Ids::name, "Assets", nullptr);

            for (auto& f : relativeFiles)
                if (copyRelativeFileToLocalSourceDirectory (f))
                    addFileToTree (assets, f.getFileName(), f.getFileExtension() == ".cpp", "Source/" + f.getFileName());

            mainGroup.addChild (assets, -1, nullptr);
        }
    }

    jucerTree.addChild (mainGroup, 0, nullptr);
}

String PIPGenerator::getDocumentControllerClass() const
{
    return metadata.getProperty (Ids::documentControllerClass, "").toString();
}

ValueTree PIPGenerator::createModulePathChild (const String& moduleID)
{
    ValueTree modulePath (Ids::MODULEPATH);

    modulePath.setProperty (Ids::ID, moduleID, nullptr);
    modulePath.setProperty (Ids::path, getPathForModule (moduleID), nullptr);

    return modulePath;
}

ValueTree PIPGenerator::createBuildConfigChild (bool isDebug)
{
    ValueTree child (Ids::CONFIGURATION);

    child.setProperty (Ids::name, isDebug ? "Debug" : "Release", nullptr);
    child.setProperty (Ids::isDebug, isDebug ? 1 : 0, nullptr);
    child.setProperty (Ids::optimisation, isDebug ? 1 : 3, nullptr);
    child.setProperty (Ids::targetName, metadata[Ids::name], nullptr);

    return child;
}

ValueTree PIPGenerator::createExporterChild (const Identifier& exporterIdentifier)
{
    ValueTree exporter (exporterIdentifier);

    exporter.setProperty (Ids::targetFolder, "Builds/" + ProjectExporter::getTypeInfoForExporter (exporterIdentifier).targetFolder, nullptr);

    const Identifier vsExporters[] { MSVCProjectExporterVC2019::getValueTreeTypeName(),
                                     MSVCProjectExporterVC2022::getValueTreeTypeName() };

    if (isJUCEExample (pipFile) && std::find (std::begin (vsExporters), std::end (vsExporters), exporterIdentifier) != std::end (vsExporters))
    {
        exporter.setProperty (Ids::extraCompilerFlags, "/bigobj", nullptr);
    }

    if (isJUCEExample (pipFile) && exporterRequiresExampleAssets (exporterIdentifier, metadata[Ids::name]))
    {
        auto examplesDir = getExamplesDirectory();

        if (examplesDir != File())
        {
            auto assetsDirectoryPath = examplesDir.getChildFile ("Assets").getFullPathName();

            exporter.setProperty (exporterIdentifier.toString() == AndroidProjectExporter::getValueTreeTypeName() ? Ids::androidExtraAssetsFolder
                                                                                                                  : Ids::customXcodeResourceFolders,
                                  assetsDirectoryPath, nullptr);
        }
        else
        {
            // invalid JUCE path
            jassertfalse;
        }
    }

    if (exporterIdentifier.toString() == AndroidProjectExporter::getValueTreeTypeName())
        exporter.setProperty (Ids::androidBluetoothNeeded, true, nullptr);

    {
        ValueTree configs (Ids::CONFIGURATIONS);

        configs.addChild (createBuildConfigChild (true), -1, nullptr);
        configs.addChild (createBuildConfigChild (false), -1, nullptr);

        exporter.addChild (configs, -1, nullptr);
    }

    {
        ValueTree modulePaths (Ids::MODULEPATHS);

        auto modules = StringArray::fromTokens (metadata[Ids::dependencies_].toString(), ",", {});

        for (auto m : modules)
            modulePaths.addChild (createModulePathChild (m.trim()), -1, nullptr);

        exporter.addChild (modulePaths, -1, nullptr);
    }

    return exporter;
}

ValueTree PIPGenerator::createModuleChild (const String& moduleID)
{
    ValueTree module (Ids::MODULE);

    module.setProperty (Ids::ID, moduleID, nullptr);
    module.setProperty (Ids::showAllCode, 1, nullptr);
    module.setProperty (Ids::useLocalCopy, 0, nullptr);
    module.setProperty (Ids::useGlobalPath, (getPathForModule (moduleID).isEmpty() ? 1 : 0), nullptr);

    return module;
}

void PIPGenerator::addExporters (ValueTree& jucerTree)
{
    ValueTree exportersTree (Ids::EXPORTFORMATS);

    auto exporters = StringArray::fromTokens (metadata[Ids::exporters].toString(), ",", {});

    for (auto& e : exporters)
    {
        e = e.trim().toUpperCase();

        if (isValidExporterIdentifier (e))
            exportersTree.addChild (createExporterChild (e), -1, nullptr);
    }

    jucerTree.addChild (exportersTree, -1, nullptr);
}

void PIPGenerator::addModules (ValueTree& jucerTree)
{
    ValueTree modulesTree (Ids::MODULES);

    auto modules = StringArray::fromTokens (metadata[Ids::dependencies_].toString(), ",", {});
    modules.trim();

    for (auto& m : modules)
        modulesTree.addChild (createModuleChild (m.trim()), -1, nullptr);

    jucerTree.addChild (modulesTree, -1, nullptr);
}

Result PIPGenerator::setProjectSettings (ValueTree& jucerTree)
{
    auto setPropertyIfNotEmpty = [&jucerTree] (const Identifier& name, const var& value)
    {
        if (value != var())
            jucerTree.setProperty (name, value, nullptr);
    };

    setPropertyIfNotEmpty (Ids::name, metadata[Ids::name]);
    setPropertyIfNotEmpty (Ids::companyName, metadata[Ids::vendor]);
    setPropertyIfNotEmpty (Ids::version, metadata[Ids::version]);
    setPropertyIfNotEmpty (Ids::userNotes, metadata[Ids::description]);
    setPropertyIfNotEmpty (Ids::companyWebsite, metadata[Ids::website]);

    auto defines = metadata[Ids::defines].toString();

    if (isJUCEExample (pipFile))
    {
        auto examplesDir = getExamplesDirectory();

        if (examplesDir != File())
        {
             defines += ((defines.isEmpty() ? "" : " ") + String ("PIP_JUCE_EXAMPLES_DIRECTORY=")
                         + Base64::toBase64 (examplesDir.getFullPathName()));
        }
        else
        {
            return Result::fail (String ("Invalid JUCE path. Set path to JUCE via ") +
                                 (TargetOS::getThisOS() == TargetOS::osx ? "\"Projucer->Global Paths...\""
                                                                         : "\"File->Global Paths...\"")
                                 + " menu item.");
        }
    }

    setPropertyIfNotEmpty (Ids::defines, defines);

    auto type = metadata[Ids::type].toString();

    if (type == "Console")
    {
        jucerTree.setProperty (Ids::projectType, build_tools::ProjectType_ConsoleApp::getTypeName(), nullptr);
    }
    else if (type == "Component")
    {
        jucerTree.setProperty (Ids::projectType, build_tools::ProjectType_GUIApp::getTypeName(), nullptr);
    }
    else if (type == "AudioProcessor")
    {
        jucerTree.setProperty (Ids::projectType, build_tools::ProjectType_AudioPlugin::getTypeName(), nullptr);
        jucerTree.setProperty (Ids::pluginAUIsSandboxSafe, "1", nullptr);

        setPropertyIfNotEmpty (Ids::pluginManufacturer, metadata[Ids::vendor]);

        StringArray pluginFormatsToBuild (Ids::buildVST3.toString(), Ids::buildAU.toString(), Ids::buildStandalone.toString());
        pluginFormatsToBuild.addArray (getExtraPluginFormatsToBuild());

        if (getDocumentControllerClass().isNotEmpty())
            pluginFormatsToBuild.add (Ids::enableARA.toString());

        jucerTree.setProperty (Ids::pluginFormats, pluginFormatsToBuild.joinIntoString (","), nullptr);

        const auto characteristics = metadata[Ids::pluginCharacteristics].toString();

        if (characteristics.isNotEmpty())
            jucerTree.setProperty (Ids::pluginCharacteristicsValue,
                                   characteristics.removeCharacters (" \t\n\r"),
                                   nullptr);
    }

    jucerTree.setProperty (Ids::useAppConfig, false, nullptr);
    jucerTree.setProperty (Ids::addUsingNamespaceToJuceHeader, true, nullptr);

    return Result::ok();
}

void PIPGenerator::setModuleFlags (ValueTree& jucerTree)
{
    ValueTree options ("JUCEOPTIONS");

    for (auto& option : StringArray::fromTokens (metadata[Ids::moduleFlags].toString(), ",", {}))
    {
        auto name  = option.upToFirstOccurrenceOf ("=", false, true).trim();
        auto value = option.fromFirstOccurrenceOf ("=", false, true).trim();

        options.setProperty (name, (value == "1" ? 1 : 0), nullptr);
    }

    if (metadata[Ids::type].toString() == "AudioProcessor"
          && options.getPropertyPointer ("JUCE_VST3_CAN_REPLACE_VST2") == nullptr)
        options.setProperty ("JUCE_VST3_CAN_REPLACE_VST2", 0, nullptr);

    jucerTree.addChild (options, -1, nullptr);
}

String PIPGenerator::getMainFileTextForType()
{
    const auto type = metadata[Ids::type].toString();
    const auto documentControllerClass = getDocumentControllerClass();

    const auto mainTemplate = [&]
    {
        if (type == "Console")
            return String (BinaryData::PIPConsole_cpp_in);

        if (type == "Component")
            return String (BinaryData::PIPComponent_cpp_in)
                   .replace ("${JUCE_PIP_NAME}",       metadata[Ids::name].toString())
                   .replace ("${PROJECT_VERSION}",     metadata[Ids::version].toString())
                   .replace ("${JUCE_PIP_MAIN_CLASS}", metadata[Ids::mainClass].toString());

        if (type == "AudioProcessor")
        {
            if (documentControllerClass.isNotEmpty())
            {
                return String (BinaryData::PIPAudioProcessorWithARA_cpp_in)
                       .replace ("${JUCE_PIP_MAIN_CLASS}", metadata[Ids::mainClass].toString())
                       .replace ("${JUCE_PIP_DOCUMENTCONTROLLER_CLASS}", documentControllerClass);
            }

            return String (BinaryData::PIPAudioProcessor_cpp_in)
                   .replace ("${JUCE_PIP_MAIN_CLASS}", metadata[Ids::mainClass].toString());
        }

        return String{};
    }();

    if (mainTemplate.isEmpty())
        return {};

    const auto includeFilename = [&]
    {
        if (useLocalCopy) return pipFile.getFileName();
        if (isTemp)       return pipFile.getFullPathName();

        return build_tools::RelativePath (pipFile,
                                          outputDirectory.getChildFile ("Source"),
                                          build_tools::RelativePath::unknown).toUnixStyle();
    }();

    return ensureCorrectWhitespace (mainTemplate.replace ("${JUCE_PIP_HEADER}", includeFilename));
}

//==============================================================================
Array<File> PIPGenerator::replaceRelativeIncludesAndGetFilesToMove()
{
    StringArray lines;
    pipFile.readLines (lines);
    Array<File> files;

    for (auto& line : lines)
    {
        if (line.contains ("#include") && ! line.contains ("JuceLibraryCode"))
        {
            auto path = line.fromFirstOccurrenceOf ("#include", false, false);
            path = path.removeCharacters ("\"").trim();

            if (path.startsWith ("<") && path.endsWith (">"))
                continue;

            auto file = pipFile.getParentDirectory().getChildFile (path);
            files.add (file);

            line = line.replace (path, file.getFileName());
        }
    }

    outputDirectory.getChildFile ("Source")
                   .getChildFile (pipFile.getFileName())
                   .replaceWithText (joinLinesIntoSourceFile (lines));

    return files;
}

bool PIPGenerator::copyRelativeFileToLocalSourceDirectory (const File& fileToCopy) const noexcept
{
    return fileToCopy.copyFileTo (outputDirectory.getChildFile ("Source")
                                                 .getChildFile (fileToCopy.getFileName()));
}

StringArray PIPGenerator::getExtraPluginFormatsToBuild() const
{
    auto tokens = StringArray::fromTokens (metadata[Ids::extraPluginFormats].toString(), ",", {});

    for (auto& token : tokens)
    {
        token = [&]
        {
            if (token == "IAA")
                return Ids::enableIAA.toString();

            return "build" + token;
        }();
    }

    return tokens;
}

String PIPGenerator::getPathForModule (const String& moduleID) const
{
    if (isJUCEModule (moduleID))
    {
        if (juceModulesPath != File())
        {
            if (isTemp)
                return juceModulesPath.getFullPathName();

            return build_tools::RelativePath (juceModulesPath,
                                              outputDirectory,
                                              build_tools::RelativePath::projectFolder).toUnixStyle();
        }
    }
    else if (availableUserModules != nullptr)
    {
        auto moduleRoot = availableUserModules->getModuleWithID (moduleID).second.getParentDirectory();

        if (isTemp)
            return moduleRoot.getFullPathName();

        return build_tools::RelativePath (moduleRoot,
                                          outputDirectory,
                                          build_tools::RelativePath::projectFolder).toUnixStyle();
    }

    return {};
}

File PIPGenerator::getExamplesDirectory() const
{
    if (juceModulesPath != File())
    {
        auto examples = juceModulesPath.getSiblingFile ("examples");

        if (isValidJUCEExamplesDirectory (examples))
            return examples;
    }

    auto examples = File (getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get().toString()).getChildFile ("examples");

    if (isValidJUCEExamplesDirectory (examples))
        return examples;

    return {};
}
