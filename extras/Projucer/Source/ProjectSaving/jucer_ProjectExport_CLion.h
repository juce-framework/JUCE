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

#include "jucer_ProjectExport_CodeBlocks.h"
#include "jucer_ProjectExport_Make.h"
#include "jucer_ProjectExport_Xcode.h"

//==============================================================================
class CLionProjectExporter  : public ProjectExporter
{
protected:
    //==============================================================================
    class CLionBuildConfiguration  : public BuildConfiguration
    {
    public:
        CLionBuildConfiguration (Project& p, const ValueTree& settings, const ProjectExporter& e)
            : BuildConfiguration (p, settings, e)
        {
        }

        void createConfigProperties (PropertyListBuilder&) override         {}
        String getModuleLibraryArchName() const override                    { return {}; }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& tree) const override
    {
        return *new CLionBuildConfiguration (project, tree, *this);
    }

public:
    //==============================================================================
    static const char* getName()                { return "CLion (beta)"; }
    static const char* getValueTreeTypeName()   { return "CLION"; }

    static CLionProjectExporter* createForSettings (Project& projectToUse, const ValueTree& settingsToUse)
    {
        if (settingsToUse.hasType (getValueTreeTypeName()))
            return new CLionProjectExporter (projectToUse, settingsToUse);

        return nullptr;
    }

    static bool isExporterSupported (const ProjectExporter& exporter)
    {
        return exporter.isMakefile()
            || (exporter.isXcode() && ! exporter.isiOS())
            || (exporter.isCodeBlocks() && exporter.isWindows());
    }

    //==============================================================================
    CLionProjectExporter (Project& p, const ValueTree& t)   : ProjectExporter (p, t)
    {
        name = getName();

        targetLocationValue.setDefault (getDefaultBuildsRootFolder() + getTargetFolderForExporter (getValueTreeTypeName()));
    }

    //==============================================================================
    bool usesMMFiles() const override                         { return false; }
    bool canCopeWithDuplicateFiles() override                 { return false; }
    bool supportsUserDefinedConfigurations() const override   { return false; }

    bool isXcode() const override                             { return false; }
    bool isVisualStudio() const override                      { return false; }
    bool isCodeBlocks() const override                        { return false; }
    bool isMakefile() const override                          { return false; }
    bool isAndroidStudio() const override                     { return false; }
    bool isCLion() const override                             { return true; }

    bool isAndroid() const override                           { return false; }
    bool isWindows() const override                           { return false; }
    bool isLinux() const override                             { return false; }
    bool isOSX() const override                               { return false; }
    bool isiOS() const override                               { return false; }

    bool supportsTargetType (ProjectType::Target::Type) const override   { return true; }

    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override {}

    //==============================================================================
    bool canLaunchProject() override
    {
       #if JUCE_MAC
        static Identifier exporterName ("XCODE_MAC");
       #elif JUCE_WINDOWS
        static Identifier exporterName ("CODEBLOCKS_WINDOWS");
       #elif JUCE_LINUX
        static Identifier exporterName ("LINUX_MAKE");
       #else
        static Identifier exporterName;
       #endif

        if (getProject().getExporters().getChildWithName (exporterName).isValid())
            return getCLionExecutableOrApp().exists();

        return false;
    }

    bool launchProject() override
    {
        return getCLionExecutableOrApp().startAsProcess (getTargetFolder().getFullPathName().quoted());
    }

    String getDescription() override
    {
        String description;

        description << "The " << getName() << " exporter produces a single CMakeLists.txt file with "
                    << "multiple platform dependent sections, where the configuration for each section "
                    << "is inherited from other exporters added to this project." << newLine
                    << newLine
                    << "The exporters which provide the CLion configuration for the corresponding platform are:" << newLine
                    << newLine;

        for (auto& exporterName : getExporterNames())
        {
            std::unique_ptr<ProjectExporter> exporter (createNewExporter (getProject(), exporterName));

            if (isExporterSupported (*exporter))
                description << exporter->getName() << newLine;
        }

        description << newLine
                    << "Add these exporters to the project to enable CLion builds." << newLine
                    << newLine
                    << "Not all features of all the exporters are currently supported. Notable omissions are AUv3 "
                    << "plug-ins, embedding resources and fat binaries on MacOS. On Windows the CLion exporter "
                    << "requires a GCC-based compiler like MinGW.";

        return description;
    }

    void createExporterProperties (PropertyListBuilder& properties) override
    {
        for (Project::ExporterIterator exporter (getProject()); exporter.next();)
            if (isExporterSupported (*exporter))
                properties.add (new BooleanPropertyComponent (getExporterEnabledValue (*exporter), "Import settings from exporter", exporter->getName()),
                                "If this is enabled then settings from the corresponding exporter will "
                                "be used in the generated CMakeLists.txt");
    }

    void createDefaultConfigs() override {}

    void create (const OwnedArray<LibraryModule>&) const override
    {
        MemoryOutputStream out;
        out.setNewLineString ("\n");

        out << "# Automatically generated CMakeLists, created by the Projucer" << newLine
            << "# Do not edit this file! Your changes will be overwritten when you re-save the Projucer project!" << newLine
            << newLine;

        out << "cmake_minimum_required (VERSION 3.4.1)" << newLine
            << newLine;

        out << "if (NOT CMAKE_BUILD_TYPE)" << newLine
            << "    set (CMAKE_BUILD_TYPE \"Debug\"  CACHE STRING \"Choose the type of build.\" FORCE)" << newLine
            << "endif (NOT CMAKE_BUILD_TYPE)" << newLine
            << newLine;

        // We'll append to this later.
        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("CMakeLists.txt"), out);

        // CMake has stopped adding PkgInfo files to bundles, so we need to do it manually
        MemoryOutputStream pkgInfoOut;
        pkgInfoOut << "BNDL????";
        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("PkgInfo"), out);
    }

    void writeCMakeListsExporterSection (ProjectExporter* exporter) const
    {
        if (! (isExporterSupported (*exporter) && isExporterEnabled (*exporter)))
            return;

        MemoryBlock existingContent;
        getTargetFolder().getChildFile ("CMakeLists.txt").loadFileAsData (existingContent);

        MemoryOutputStream out (existingContent, true);
        out.setNewLineString ("\n");

        out << "###############################################################################" << newLine
            << "# " << exporter->getName() << newLine
            << "###############################################################################" << newLine
            << newLine;

        if (auto* makefileExporter = dynamic_cast<MakefileProjectExporter*> (exporter))
        {
            out << "if (UNIX AND NOT APPLE)" << newLine << newLine;
            writeCMakeListsMakefileSection (out, *makefileExporter);
        }
        else if (auto* xcodeExporter = dynamic_cast<XcodeProjectExporter*> (exporter))
        {
            out << "if (APPLE)" << newLine << newLine;
            writeCMakeListsXcodeSection (out, *xcodeExporter);
        }
        else if (auto* codeBlocksExporter = dynamic_cast<CodeBlocksProjectExporter*> (exporter))
        {
            out << "if (WIN32)" << newLine << newLine;
            writeCMakeListsCodeBlocksSection (out, *codeBlocksExporter);
        }

        out << "endif()" << newLine << newLine;

        overwriteFileIfDifferentOrThrow (getTargetFolder().getChildFile ("CMakeLists.txt"), out);
    }

private:
    //==============================================================================
    static File getCLionExecutableOrApp()
    {
        File clionExeOrApp (getAppSettings()
                            .getStoredPath (Ids::clionExePath, TargetOS::getThisOS()).get()
                            .toString()
                            .replace ("${user.home}", File::getSpecialLocation (File::userHomeDirectory).getFullPathName()));

       #if JUCE_MAC
        if (clionExeOrApp.getFullPathName().endsWith ("/Contents/MacOS/clion"))
            clionExeOrApp = clionExeOrApp.getParentDirectory()
                                         .getParentDirectory()
                                         .getParentDirectory();
       #endif

        return clionExeOrApp;
    }

    //==============================================================================
    Identifier getExporterEnabledId (const ProjectExporter& exporter) const
    {
        jassert (isExporterSupported (exporter));

        if      (exporter.isMakefile())   return Ids::clionMakefileEnabled;
        else if (exporter.isXcode())      return Ids::clionXcodeEnabled;
        else if (exporter.isCodeBlocks()) return Ids::clionCodeBlocksEnabled;

        jassertfalse;
        return {};
    }

    bool isExporterEnabled (const ProjectExporter& exporter) const
    {
        auto setting = settings[getExporterEnabledId (exporter)];
        return setting.isVoid() || setting;
    }

    Value getExporterEnabledValue (const ProjectExporter& exporter)
    {
        auto enabledID = getExporterEnabledId (exporter);
        getSetting (enabledID) = isExporterEnabled (exporter);
        return getSetting (enabledID);
    }

    //==============================================================================
    static bool isWindowsAbsolutePath (const String& path)
    {
        return path.length() > 1 && path[1] == ':';
    }

    static bool isUnixAbsolutePath (const String& path)
    {
        return path.isNotEmpty() && (path[0] == '/' || path[0] == '~' || path.startsWith ("$ENV{HOME}"));
    }

    //==============================================================================
    static String setCMakeVariable (const String& variableName, const String& value)
    {
        return "set (" + variableName + " \"" + value + "\")";
    }

    static String addToCMakeVariable (const String& variableName, const String& value)
    {
        return setCMakeVariable (variableName, "${" + variableName + "} " + value);
    }

    static String getTargetVarName (ProjectType::Target& target)
    {
        return String (target.getName()).toUpperCase().replaceCharacter (L' ', L'_');
    }

    template <class Target, class Exporter>
    void getFileInfoList (Target& target, Exporter& exporter, const Project::Item& projectItem, std::vector<std::tuple<String, bool, String>>& fileInfoList) const
    {
        auto targetType = (getProject().isAudioPluginProject() ? target.type : Target::Type::SharedCodeTarget);

        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                getFileInfoList (target, exporter, projectItem.getChild(i), fileInfoList);
        }
        else if (projectItem.shouldBeAddedToTargetProject() && projectItem.shouldBeAddedToTargetExporter (*this)
                 && getProject().getTargetTypeFromFilePath (projectItem.getFile(), true) == targetType )
        {
            auto path = RelativePath (projectItem.getFile(), exporter.getTargetFolder(), RelativePath::buildTargetFolder).toUnixStyle();

            fileInfoList.push_back (std::make_tuple (path, projectItem.shouldBeCompiled(),
                                                     exporter.compilerFlagSchemesMap[projectItem.getCompilerFlagSchemeString()].get().toString()));
        }
    }

    template <class Exporter>
    void writeCMakeTargets (OutputStream& out, Exporter& exporter) const
    {
        for (auto* target : exporter.targets)
        {
            if (target->type == ProjectType::Target::Type::AggregateTarget
             || target->type == ProjectType::Target::Type::AudioUnitv3PlugIn)
                continue;

            String functionName;
            StringArray properties;

            switch (target->getTargetFileType())
            {
                case ProjectType::Target::TargetFileType::executable:
                    functionName = "add_executable";

                    if (exporter.isCodeBlocks() && exporter.isWindows()
                          && target->type != ProjectType::Target::Type::ConsoleApp)
                        properties.add ("WIN32");

                    break;
                case ProjectType::Target::TargetFileType::staticLibrary:
                case ProjectType::Target::TargetFileType::sharedLibraryOrDLL:
                case ProjectType::Target::TargetFileType::pluginBundle:
                    functionName = "add_library";

                    if (target->getTargetFileType() == ProjectType::Target::TargetFileType::staticLibrary)
                        properties.add ("STATIC");
                    else if (target->getTargetFileType() == ProjectType::Target::TargetFileType::sharedLibraryOrDLL)
                        properties.add ("SHARED");
                    else
                        properties.add ("MODULE");

                    break;
                default:
                    continue;
            }

            out << functionName << " (" << getTargetVarName (*target);

            if (! properties.isEmpty())
                out << " " << properties.joinIntoString (" ");

            out << newLine;

            std::vector<std::tuple<String, bool, String>> fileInfoList;
            for (auto& group : exporter.getAllGroups())
                getFileInfoList (*target, exporter, group, fileInfoList);

            for (auto& fileInfo : fileInfoList)
                out << "    " << std::get<0> (fileInfo).quoted() << newLine;

            auto isCMakeBundle = exporter.isXcode() && target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle;
            auto pkgInfoPath = String ("PkgInfo").quoted();

            if (isCMakeBundle)
                out << "    " << pkgInfoPath << newLine;

            auto xcodeIcnsFilePath = [&] () -> String
            {
                if (exporter.isXcode() && target->getTargetFileType() == ProjectType::Target::TargetFileType::executable)
                {
                    StringArray pathComponents = { "..", "MacOSX", "Icon.icns" };
                    auto xcodeIcnsFile = getTargetFolder();

                    for (auto& comp : pathComponents)
                        xcodeIcnsFile = xcodeIcnsFile.getChildFile (comp);

                    if (xcodeIcnsFile.existsAsFile())
                        return pathComponents.joinIntoString ("/").quoted();
                }

                return {};
            }();

            if (xcodeIcnsFilePath.isNotEmpty())
                out << "    " << xcodeIcnsFilePath << newLine;

            if (exporter.isCodeBlocks() && target->getTargetFileType() == ProjectType::Target::TargetFileType::executable)
            {
                StringArray pathComponents = { "..", "CodeBlocksWindows", "resources.rc" };
                auto windowsRcFile = getTargetFolder();

                for (auto& comp : pathComponents)
                    windowsRcFile = windowsRcFile.getChildFile (comp);

                if (windowsRcFile.existsAsFile())
                    out << "    " << pathComponents.joinIntoString ("/").quoted() << newLine;
            }

            out << ")" << newLine << newLine;

            if (isCMakeBundle)
                out << "set_source_files_properties (" << pkgInfoPath << " PROPERTIES MACOSX_PACKAGE_LOCATION .)" << newLine;

            if (xcodeIcnsFilePath.isNotEmpty())
                out << "set_source_files_properties (" << xcodeIcnsFilePath << " PROPERTIES MACOSX_PACKAGE_LOCATION \"Resources\")" << newLine;

            for (auto& fileInfo : fileInfoList)
            {
                if (std::get<1> (fileInfo))
                {
                    auto extraCompilerFlags = std::get<2> (fileInfo);

                    if (extraCompilerFlags.isNotEmpty())
                        out << "set_source_files_properties(" << std::get<0> (fileInfo).quoted() << " PROPERTIES COMPILE_FLAGS " << extraCompilerFlags << " )" << newLine;
                }
                else
                {
                    out << "set_source_files_properties (" << std::get<0> (fileInfo).quoted() << " PROPERTIES HEADER_FILE_ONLY TRUE)" << newLine;
                }
            }

            out << newLine;
        }
    }

    //==============================================================================
    void writeCMakeListsMakefileSection (OutputStream& out, MakefileProjectExporter& exporter) const
    {
        out << "project (" << getProject().getProjectNameString().quoted() << " C CXX)" << newLine
            << newLine;

        out << "find_package (PkgConfig REQUIRED)" << newLine;

        StringArray cmakePkgconfigPackages;

        for (auto& package : exporter.getPackages())
        {
            cmakePkgconfigPackages.add (package.toUpperCase());
            out << "pkg_search_module (" << cmakePkgconfigPackages.strings.getLast() << " REQUIRED " << package << ")" << newLine;
        }

        out << newLine;

        writeCMakeTargets (out, exporter);

        for (auto* target : exporter.targets)
        {
            if (target->type == ProjectType::Target::Type::AggregateTarget)
                continue;

            if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle)
                out << "set_target_properties (" << getTargetVarName (*target) << " PROPERTIES PREFIX \"\")" << newLine;

            out << "set_target_properties (" << getTargetVarName (*target) << " PROPERTIES SUFFIX \"" << target->getTargetFileSuffix() << "\")" << newLine
                << newLine;
        }

        for (ProjectExporter::ConstConfigIterator c (exporter); c.next();)
        {
            auto& config = dynamic_cast<const MakefileProjectExporter::MakeBuildConfiguration&> (*c);

            out << "#------------------------------------------------------------------------------" << newLine
                << "# Config: " << config.getName() << newLine
                << "#------------------------------------------------------------------------------" << newLine
                << newLine;

            auto buildTypeCondition = String ("CMAKE_BUILD_TYPE STREQUAL " + config.getName());
            out << "if (" << buildTypeCondition << ")" << newLine
                << newLine;

            out << "execute_process (COMMAND uname -m OUTPUT_VARIABLE JUCE_ARCH_LABEL OUTPUT_STRIP_TRAILING_WHITESPACE)" << newLine
                << newLine;

            out << "include_directories (" << newLine;

            for (auto& path : exporter.getHeaderSearchPaths (config))
                out << "    " << path.quoted() << newLine;

            for (auto& package : cmakePkgconfigPackages)
                out << "    ${" << package << "_INCLUDE_DIRS}" << newLine;

            out << ")" << newLine << newLine;

            StringArray cmakeFoundLibraries;

            for (auto& library : exporter.getLibraryNames (config))
            {
                String cmakeLibraryID (library.toUpperCase());
                cmakeFoundLibraries.add (String ("${") + cmakeLibraryID + "}");
                out << "find_library (" << cmakeLibraryID << " " << library << newLine;

                for (auto& path : exporter.getLibrarySearchPaths (config))
                    out << "    " << path.quoted() << newLine;

                out << ")" << newLine
                    << newLine;
            }

            for (auto* target : exporter.targets)
            {
                if (target->type == ProjectType::Target::Type::AggregateTarget)
                    continue;

                auto targetVarName = getTargetVarName (*target);

                out << "set_target_properties (" << targetVarName << " PROPERTIES" << newLine
                    << "    OUTPUT_NAME "  <<  config.getTargetBinaryNameString().quoted() << newLine;

                auto cxxStandard = project.getCppStandardString();

                if (cxxStandard == "latest")
                    cxxStandard = "17";

                out << "    CXX_STANDARD " << cxxStandard << newLine;

                if (! shouldUseGNUExtensions())
                    out << "    CXX_EXTENSIONS OFF" << newLine;

                out << ")" << newLine << newLine;

                auto defines = exporter.getDefines (config);
                defines.addArray (target->getDefines (config));

                out << "target_compile_definitions (" << targetVarName << " PRIVATE" << newLine;

                for (auto& key : defines.getAllKeys())
                    out << "    " << key << "=" << defines[key] << newLine;

                out << ")" << newLine << newLine;

                auto targetFlags = target->getCompilerFlags();

                if (! targetFlags.isEmpty())
                {
                    out << "target_compile_options (" << targetVarName << " PRIVATE" << newLine;

                    for (auto& flag : targetFlags)
                        out << "    " << flag << newLine;

                    out << ")" << newLine << newLine;
                }

                out << "target_link_libraries (" << targetVarName << " PRIVATE" << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                 || target->type == ProjectType::Target::Type::StandalonePlugIn)
                    out << "    SHARED_CODE" << newLine;

                out << "    " << exporter.getArchFlags (config) << newLine;

                for (auto& flag : target->getLinkerFlags())
                    out << "    " << flag << newLine;

                for (auto& flag : exporter.getLinkerFlags (config))
                    out << "    " << flag << newLine;

                for (auto& lib : cmakeFoundLibraries)
                    out << "    " << lib << newLine;

                for (auto& package : cmakePkgconfigPackages)
                    out << "    ${" << package << "_LIBRARIES}" << newLine;

                out << ")" << newLine << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                    || target->type == ProjectType::Target::Type::StandalonePlugIn)
                    out << "add_dependencies (" << targetVarName << " " << "SHARED_CODE)" << newLine << newLine;
            }

            StringArray cFlags;
            cFlags.add (exporter.getArchFlags (config));
            cFlags.addArray (exporter.getCPreprocessorFlags (config));
            cFlags.addArray (exporter.getCFlags (config));
            out << addToCMakeVariable ("CMAKE_C_FLAGS", cFlags.joinIntoString (" ")) << newLine;

            String cxxFlags;

            for (auto& flag : exporter.getCXXFlags())
                if (! flag.startsWith ("-std="))
                    cxxFlags += " " + flag;

            out << addToCMakeVariable ("CMAKE_CXX_FLAGS", "${CMAKE_C_FLAGS} " + cxxFlags) << newLine
                << newLine;

            out << "endif (" << buildTypeCondition << ")" << newLine
                << newLine;
        }
    }

    //==============================================================================
    void writeCMakeListsCodeBlocksSection (OutputStream& out, CodeBlocksProjectExporter& exporter) const
    {
        out << "project (" << getProject().getProjectNameString().quoted() << " C CXX)" << newLine
            << newLine;

        writeCMakeTargets (out, exporter);

        for (auto* target : exporter.targets)
        {
            if (target->type == ProjectType::Target::Type::AggregateTarget)
                continue;

            out << "set_target_properties (" << getTargetVarName (*target) << " PROPERTIES PREFIX \"\")" << newLine
                << "set_target_properties (" << getTargetVarName (*target) << " PROPERTIES SUFFIX " << target->getTargetSuffix().quoted() << ")" << newLine
                << newLine;
        }

        for (ProjectExporter::ConstConfigIterator c (exporter); c.next();)
        {
            auto& config = dynamic_cast<const CodeBlocksProjectExporter::CodeBlocksBuildConfiguration&> (*c);

            out << "#------------------------------------------------------------------------------" << newLine
                << "# Config: " << config.getName() << newLine
                << "#------------------------------------------------------------------------------" << newLine
                << newLine;

            auto buildTypeCondition = String ("CMAKE_BUILD_TYPE STREQUAL " + config.getName());
            out << "if (" << buildTypeCondition << ")" << newLine
                << newLine;

            out << "include_directories (" << newLine;

            for (auto& path : exporter.getIncludePaths (config))
                out << "    " << path.replace ("\\", "/").quoted() << newLine;

            out << ")" << newLine << newLine;

            for (auto* target : exporter.targets)
            {
                if (target->type == ProjectType::Target::Type::AggregateTarget)
                    continue;

                auto targetVarName = getTargetVarName (*target);

                out << "set_target_properties (" << targetVarName << " PROPERTIES" << newLine
                    << "    OUTPUT_NAME "  <<  config.getTargetBinaryNameString().quoted() << newLine;

                auto cxxStandard = project.getCppStandardString();

                if (cxxStandard == "latest")
                    cxxStandard = "17";

                out << "    CXX_STANDARD " << cxxStandard << newLine;

                if (! shouldUseGNUExtensions())
                    out << "    CXX_EXTENSIONS OFF" << newLine;

                out << ")" << newLine << newLine;

                out << "target_compile_definitions (" << targetVarName << " PRIVATE" << newLine;

                for (auto& def : exporter.getDefines (config, *target))
                    out << "    " << def << newLine;

                out << ")" << newLine << newLine;

                out << "target_compile_options (" << targetVarName << " PRIVATE" << newLine;

                for (auto& option : exporter.getCompilerFlags (config, *target))
                    if (! option.startsWith ("-std="))
                        out << "    " << option.quoted() << newLine;

                out << ")" << newLine << newLine;

                out << "target_link_libraries (" << targetVarName << " PRIVATE" << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                    || target->type == ProjectType::Target::Type::StandalonePlugIn)
                    out << "    SHARED_CODE" << newLine
                        << "    -L." << newLine;

                for (auto& path : exporter.getLinkerSearchPaths (config, *target))
                {
                    out << "    \"-L\\\"";

                    if (! isWindowsAbsolutePath (path))
                        out << "${CMAKE_CURRENT_SOURCE_DIR}/";

                    out << path.replace ("\\", "/").unquoted() << "\\\"\"" << newLine;
                }

                for (auto& flag : exporter.getLinkerFlags (config, *target))
                    out << "    " << flag << newLine;

                for (auto& flag : exporter.getProjectLinkerLibs())
                    out << "    -l" << flag << newLine;

                for (auto& lib : exporter.mingwLibs)
                    out << "    -l" << lib << newLine;

                out << ")" << newLine << newLine;
            }

            out << addToCMakeVariable ("CMAKE_CXX_FLAGS", exporter.getProjectCompilerOptions().joinIntoString (" ")) << newLine
                << addToCMakeVariable ("CMAKE_C_FLAGS", "${CMAKE_CXX_FLAGS}") << newLine
                << newLine;

            out << "endif (" << buildTypeCondition << ")" << newLine
                << newLine;
        }
    }

    //==============================================================================
    void writeCMakeListsXcodeSection (OutputStream& out, XcodeProjectExporter& exporter) const
    {
        // We need to find out the SDK root before defining the project. Unfortunately this is
        // set per-target in the Xcode project, but we want it per-configuration.
        for (ProjectExporter::ConstConfigIterator c (exporter); c.next();)
        {
            auto& config = dynamic_cast<const XcodeProjectExporter::XcodeBuildConfiguration&> (*c);

            for (auto* target : exporter.targets)
            {
                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::macOSAppex
                      || target->type == ProjectType::Target::Type::AggregateTarget
                      || target->type == ProjectType::Target::Type::AudioUnitv3PlugIn)
                    continue;

                auto targetAttributes = target->getTargetSettings (config);
                auto targetAttributeKeys = targetAttributes.getAllKeys();

                if (targetAttributes.getAllKeys().contains ("SDKROOT"))
                {
                    out << "if (CMAKE_BUILD_TYPE STREQUAL " + config.getName() << ")" << newLine
                        << "    set (CMAKE_OSX_SYSROOT " << targetAttributes["SDKROOT"] << ")" << newLine
                        << "endif()" << newLine << newLine;
                    break;
                }
            }
        }

        out << "project (" << getProject().getProjectNameString().quoted() << " C CXX)" << newLine << newLine;

        writeCMakeTargets (out, exporter);

        for (auto* target : exporter.targets)
        {
            if (target->getTargetFileType() == ProjectType::Target::TargetFileType::macOSAppex
                || target->type == ProjectType::Target::Type::AggregateTarget
                || target->type == ProjectType::Target::Type::AudioUnitv3PlugIn)
                continue;

            if (target->type == ProjectType::Target::Type::AudioUnitPlugIn)
                out << "find_program (RC_COMPILER Rez NO_DEFAULT_PATHS PATHS \"/Applications/Xcode.app/Contents/Developer/usr/bin\")" << newLine
                    << "if (NOT RC_COMPILER)" << newLine
                    << "    message (WARNING \"failed to find Rez; older resource-based AU plug-ins may not work correctly\")" << newLine
                    << "endif (NOT RC_COMPILER)" << newLine  << newLine;

            if (target->getTargetFileType() == ProjectType::Target::TargetFileType::staticLibrary
                    || target->getTargetFileType() == ProjectType::Target::TargetFileType::sharedLibraryOrDLL)
                out << "set_target_properties (" << getTargetVarName (*target) << " PROPERTIES SUFFIX \"" << target->xcodeBundleExtension << "\")" << newLine
                    << newLine;
        }

        for (ProjectExporter::ConstConfigIterator c (exporter); c.next();)
        {
            auto& config = dynamic_cast<const XcodeProjectExporter::XcodeBuildConfiguration&> (*c);

            out << "#------------------------------------------------------------------------------" << newLine
                << "# Config: " << config.getName() << newLine
                << "#------------------------------------------------------------------------------" << newLine
                << newLine;

            auto buildTypeCondition = String ("CMAKE_BUILD_TYPE STREQUAL " + config.getName());
            out << "if (" << buildTypeCondition << ")" << newLine
                << newLine;

            out << "execute_process (COMMAND uname -m OUTPUT_VARIABLE JUCE_ARCH_LABEL OUTPUT_STRIP_TRAILING_WHITESPACE)" << newLine
                << newLine;

            auto configSettings = exporter.getProjectSettings (config);
            auto configSettingsKeys = configSettings.getAllKeys();

            auto binaryName = config.getTargetBinaryNameString();

            if (configSettingsKeys.contains ("PRODUCT_NAME"))
                binaryName = configSettings["PRODUCT_NAME"].unquoted();

            for (auto* target : exporter.targets)
            {
                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::macOSAppex
                    || target->type == ProjectType::Target::Type::AggregateTarget
                    || target->type == ProjectType::Target::Type::AudioUnitv3PlugIn)
                    continue;

                auto targetVarName = getTargetVarName (*target);

                auto targetAttributes = target->getTargetSettings (config);
                auto targetAttributeKeys = targetAttributes.getAllKeys();

                StringArray headerSearchPaths;

                if (targetAttributeKeys.contains ("HEADER_SEARCH_PATHS"))
                {
                    auto paths = targetAttributes["HEADER_SEARCH_PATHS"].trim().substring (1).dropLastCharacters (1);
                    paths = paths.replace ("\"$(inherited)\"", {})
                                 .replace ("$(HOME)", "$ENV{HOME}")
                                 .replace ("~", "$ENV{HOME}");
                    headerSearchPaths.addTokens (paths, ",\"\t\\", {});
                    headerSearchPaths.removeEmptyStrings();
                    targetAttributeKeys.removeString ("HEADER_SEARCH_PATHS");
                }

                out << "target_include_directories (" << targetVarName << " PRIVATE" << newLine;

                for (auto& path : headerSearchPaths)
                    out << "    " << path.quoted() << newLine;

                out << ")" << newLine << newLine;

                StringArray defines;

                if (targetAttributeKeys.contains ("GCC_PREPROCESSOR_DEFINITIONS"))
                {
                    defines.addTokens (targetAttributes["GCC_PREPROCESSOR_DEFINITIONS"], "() ,\t", {});
                    defines.removeEmptyStrings();
                    targetAttributeKeys.removeString ("GCC_PREPROCESSOR_DEFINITIONS");
                }

                out << "target_compile_definitions (" << targetVarName << " PRIVATE" << newLine;

                for (auto& def : defines)
                    out << "    " << def << newLine;

                out << ")" << newLine << newLine;

                StringArray cppFlags;

                String archLabel ("${JUCE_ARCH_LABEL}");

                // Fat binaries are not supported.
                if (targetAttributeKeys.contains ("ARCHS"))
                {
                    auto value = targetAttributes["ARCHS"].unquoted();

                    if  (value.contains ("NATIVE_ARCH_ACTUAL"))
                    {
                        cppFlags.add ("-march=native");
                    }
                    else if (value.contains ("ARCHS_STANDARD_32_BIT"))
                    {
                        archLabel = "i386";
                        cppFlags.add ("-arch x86");
                    }
                    else if (value.contains ("ARCHS_STANDARD_32_64_BIT")
                          || value.contains ("ARCHS_STANDARD_64_BIT"))
                    {
                        archLabel = "x86_64";
                        cppFlags.add ("-arch x86_64");
                    }

                    targetAttributeKeys.removeString ("ARCHS");
                }

                if (targetAttributeKeys.contains ("MACOSX_DEPLOYMENT_TARGET"))
                {
                    cppFlags.add ("-mmacosx-version-min=" + targetAttributes["MACOSX_DEPLOYMENT_TARGET"]);
                    targetAttributeKeys.removeString ("MACOSX_DEPLOYMENT_TARGET");
                }

                if (targetAttributeKeys.contains ("OTHER_CPLUSPLUSFLAGS"))
                {
                    cppFlags.add (targetAttributes["OTHER_CPLUSPLUSFLAGS"].unquoted());
                    targetAttributeKeys.removeString ("OTHER_CPLUSPLUSFLAGS");
                }

                if (targetAttributeKeys.contains ("GCC_OPTIMIZATION_LEVEL"))
                {
                    cppFlags.add ("-O" + targetAttributes["GCC_OPTIMIZATION_LEVEL"]);
                    targetAttributeKeys.removeString ("GCC_OPTIMIZATION_LEVEL");
                }

                if (targetAttributeKeys.contains ("LLVM_LTO"))
                {
                    cppFlags.add ("-flto");
                    targetAttributeKeys.removeString ("LLVM_LTO");
                }

                if (targetAttributeKeys.contains ("GCC_FAST_MATH"))
                {
                    cppFlags.add ("-ffast-math");
                    targetAttributeKeys.removeString ("GCC_FAST_MATH");
                }

                // We'll take this setting from the project
                targetAttributeKeys.removeString ("CLANG_CXX_LANGUAGE_STANDARD");

                if (targetAttributeKeys.contains ("CLANG_CXX_LIBRARY"))
                {
                    cppFlags.add ("-stdlib=" + targetAttributes["CLANG_CXX_LIBRARY"].unquoted());
                    targetAttributeKeys.removeString ("CLANG_CXX_LIBRARY");
                }

                out << "target_compile_options (" << targetVarName << " PRIVATE" << newLine;

                for (auto& flag : cppFlags)
                    out << "    " << flag << newLine;

                out << ")" << newLine << newLine;

                StringArray libSearchPaths;

                if (targetAttributeKeys.contains ("LIBRARY_SEARCH_PATHS"))
                {
                    auto paths = targetAttributes["LIBRARY_SEARCH_PATHS"].trim().substring (1).dropLastCharacters (1);
                    paths = paths.replace ("\"$(inherited)\"", {});
                    paths = paths.replace ("$(HOME)", "$ENV{HOME}");
                    libSearchPaths.addTokens (paths, ",\"\t\\", {});
                    libSearchPaths.removeEmptyStrings();

                    for (auto& libPath : libSearchPaths)
                    {
                        libPath = libPath.replace ("${CURRENT_ARCH}", archLabel);

                        if (! isUnixAbsolutePath (libPath))
                            libPath = "${CMAKE_CURRENT_SOURCE_DIR}/" + libPath;
                    }

                    targetAttributeKeys.removeString ("LIBRARY_SEARCH_PATHS");
                }

                StringArray linkerFlags;

                if (targetAttributeKeys.contains ("OTHER_LDFLAGS"))
                {
                    // CMake adds its own SHARED_CODE library linking flags
                    auto flagsWithReplacedSpaces = targetAttributes["OTHER_LDFLAGS"].unquoted().replace ("\\\\ ", "^^%%^^");
                    linkerFlags.addTokens (flagsWithReplacedSpaces, true);
                    linkerFlags.removeString ("-bundle");
                    linkerFlags.removeString ("-l" + binaryName.replace (" ", "^^%%^^"));

                    for (auto& flag : linkerFlags)
                        flag = flag.replace ("^^%%^^", " ");

                    targetAttributeKeys.removeString ("OTHER_LDFLAGS");
                }

                if (target->type == ProjectType::Target::Type::AudioUnitPlugIn)
                {
                    String rezFlags;

                    if (targetAttributeKeys.contains ("OTHER_REZFLAGS"))
                    {
                        rezFlags = targetAttributes["OTHER_REZFLAGS"];
                        targetAttributeKeys.removeString ("OTHER_REZFLAGS");
                    }

                    for (auto& item : exporter.getAllGroups())
                    {
                        if (item.getName() == ProjectSaver::getJuceCodeGroupName())
                        {
                            auto resSourcesVar = targetVarName + "_REZ_SOURCES";
                            auto resOutputVar = targetVarName + "_REZ_OUTPUT";

                            auto sdkVersion = config.getOSXSDKVersionString().upToFirstOccurrenceOf (" ", false, false);
                            auto sysroot = "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX" + sdkVersion + ".sdk";

                            RelativePath rFile ("JuceLibraryCode/include_juce_audio_plugin_client_AU.r", RelativePath::projectFolder);
                            rFile = rebaseFromProjectFolderToBuildTarget (rFile);

                            out << "if (RC_COMPILER)" << newLine
                                << "    set (" << resSourcesVar << newLine
                                << "        " << ("${CMAKE_CURRENT_SOURCE_DIR}/" + rFile.toUnixStyle()).quoted() << newLine
                                << "    )" << newLine
                                << "    set (" << resOutputVar << " " << ("${CMAKE_CURRENT_BINARY_DIR}/" + binaryName + ".rsrc").quoted() << ")" << newLine
                                << "    target_sources (" << targetVarName << " PRIVATE" << newLine
                                << "        ${" << resSourcesVar << "}" << newLine
                                << "        ${" << resOutputVar << "}" << newLine
                                << "    )" << newLine
                                << "    execute_process (COMMAND" << newLine
                                << "        ${RC_COMPILER}" << newLine
                                << "        " << rezFlags.unquoted().removeCharacters ("\\") << newLine
                                << "        -isysroot " << sysroot.quoted() << newLine;

                            for (auto& path : headerSearchPaths)
                            {
                                out << "        -I \"";

                                if (! isUnixAbsolutePath (path))
                                    out << "${PROJECT_SOURCE_DIR}/";

                                out << path << "\"" << newLine;
                            }

                            out << "        ${" << resSourcesVar << "}" << newLine
                                << "        -o ${" << resOutputVar << "}" << newLine
                                << "    )" << newLine
                                << "    set_source_files_properties (${" << resOutputVar << "} PROPERTIES" << newLine
                                << "        GENERATED TRUE" << newLine
                                << "        MACOSX_PACKAGE_LOCATION Resources" << newLine
                                << "    )" << newLine
                                << "endif (RC_COMPILER)" << newLine  << newLine;
                            break;
                        }
                    }
                }

                if (targetAttributeKeys.contains ("INFOPLIST_FILE"))
                {
                    auto plistFile = exporter.getTargetFolder().getChildFile (targetAttributes["INFOPLIST_FILE"]);

                    if (auto plist = parseXML (plistFile))
                    {
                        if (auto* dict = plist->getChildByName ("dict"))
                        {
                            if (auto* entry = dict->getChildByName ("key"))
                            {
                                while (entry != nullptr)
                                {
                                    if (entry->getAllSubText() == "CFBundleExecutable")
                                    {
                                        if (auto* bundleName = entry->getNextElementWithTagName ("string"))
                                        {
                                            bundleName->deleteAllTextElements();
                                            bundleName->addTextElement (binaryName);
                                        }
                                    }

                                    entry = entry->getNextElementWithTagName ("key");
                                }
                            }
                        }

                        auto updatedPlist = getTargetFolder().getChildFile (config.getName() + "-" + plistFile.getFileName());

                        XmlElement::TextFormat format;
                        format.dtd = "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">";
                        plist->writeTo (updatedPlist, format);

                        targetAttributes.set ("INFOPLIST_FILE", ("${CMAKE_CURRENT_SOURCE_DIR}/" + updatedPlist.getFileName()).quoted());
                    }
                    else
                    {
                        targetAttributeKeys.removeString ("INFOPLIST_FILE");
                    }
                }

                targetAttributeKeys.sort (false);

                out << "set_target_properties (" << targetVarName << " PROPERTIES" << newLine
                    << "    OUTPUT_NAME "  << binaryName.quoted() << newLine;

                auto cxxStandard = project.getCppStandardString();

                if (cxxStandard == "latest")
                    cxxStandard = "17";

                out << "    CXX_STANDARD " << cxxStandard << newLine;

                if (! shouldUseGNUExtensions())
                    out << "    CXX_EXTENSIONS OFF" << newLine;

                for (auto& key : targetAttributeKeys)
                    out << "    XCODE_ATTRIBUTE_" << key << " " << targetAttributes[key] << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::executable
                    || target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle)
                {
                    out << "    MACOSX_BUNDLE_INFO_PLIST "     << targetAttributes.getValue ("INFOPLIST_FILE", "\"\"") << newLine
                        << "    XCODE_ATTRIBUTE_PRODUCT_NAME " << binaryName.quoted() << newLine;

                    if (target->getTargetFileType() == ProjectType::Target::TargetFileType::executable)
                    {
                        out << "    MACOSX_BUNDLE TRUE" << newLine;
                    }
                    else
                    {
                        out << "    BUNDLE TRUE" << newLine
                            << "    BUNDLE_EXTENSION " << targetAttributes.getValue ("WRAPPER_EXTENSION", "\"\"") << newLine
                            << "    XCODE_ATTRIBUTE_MACH_O_TYPE \"mh_bundle\"" << newLine;
                    }
                }

                out << ")" << newLine << newLine;

                out << "target_link_libraries (" << targetVarName << " PRIVATE" << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                        || target->type == ProjectType::Target::Type::StandalonePlugIn)
                    out << "    SHARED_CODE" << newLine;

                for (auto& path : libSearchPaths)
                    out << "    \"-L\\\"" << path << "\\\"\"" << newLine;

                for (auto& flag : linkerFlags)
                    out << "    " << flag.quoted() << newLine;

                for (auto& framework : target->frameworkNames)
                        out << "    \"-framework " << framework << "\"" << newLine;

                out << ")" << newLine << newLine;

                if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                    || target->type == ProjectType::Target::Type::StandalonePlugIn)
                {
                    if (target->getTargetFileType() == ProjectType::Target::TargetFileType::pluginBundle
                        && targetAttributeKeys.contains("INSTALL_PATH"))
                    {
                        auto installPath = targetAttributes["INSTALL_PATH"].unquoted().replace ("$(HOME)", "$ENV{HOME}");
                        auto productFilename = binaryName + (targetAttributeKeys.contains ("WRAPPER_EXTENSION") ? "." + targetAttributes["WRAPPER_EXTENSION"] : String());
                        auto productPath = (installPath + productFilename).quoted();
                        out << "add_custom_command (TARGET " << targetVarName << " POST_BUILD" << newLine
                            << "    COMMAND ${CMAKE_COMMAND} -E remove_directory " << productPath << newLine
                            << "    COMMAND ${CMAKE_COMMAND} -E copy_directory \"${CMAKE_BINARY_DIR}/" << productFilename << "\" " << productPath << newLine
                            << "    COMMENT \"Copying \\\"" << productFilename << "\\\" to \\\"" << installPath.unquoted() << "\\\"\"" << newLine
                            << ")" << newLine << newLine;
                    }
                }
            }

            std::map<String, String> basicWarnings
            {
                { "CLANG_WARN_BOOL_CONVERSION",       "bool-conversion" },
                { "CLANG_WARN_COMMA",                 "comma" },
                { "CLANG_WARN_CONSTANT_CONVERSION",   "constant-conversion" },
                { "CLANG_WARN_EMPTY_BODY",            "empty-body" },
                { "CLANG_WARN_ENUM_CONVERSION",       "enum-conversion" },
                { "CLANG_WARN_INFINITE_RECURSION",    "infinite-recursion" },
                { "CLANG_WARN_INT_CONVERSION",        "int-conversion" },
                { "CLANG_WARN_RANGE_LOOP_ANALYSIS",   "range-loop-analysis" },
                { "CLANG_WARN_STRICT_PROTOTYPES",     "strict-prototypes" },
                { "GCC_WARN_CHECK_SWITCH_STATEMENTS", "switch" },
                { "GCC_WARN_UNUSED_VARIABLE",         "unused-variable" },
                { "GCC_WARN_MISSING_PARENTHESES",     "parentheses" },
                { "GCC_WARN_NON_VIRTUAL_DESTRUCTOR",  "non-virtual-dtor" },
                { "GCC_WARN_64_TO_32_BIT_CONVERSION", "shorten-64-to-32" },
                { "GCC_WARN_UNDECLARED_SELECTOR",     "undeclared-selector" },
                { "GCC_WARN_UNUSED_FUNCTION",         "unused-function" }
            };

            StringArray compilerFlags;

            for (auto& key : configSettingsKeys)
            {
                auto basicWarning = basicWarnings.find (key);

                if (basicWarning != basicWarnings.end())
                {
                    compilerFlags.add (configSettings[key] == "YES" ? "-W" + basicWarning->second : "-Wno-" + basicWarning->second);
                }
                else if (key == "CLANG_WARN_SUSPICIOUS_MOVE"         && configSettings[key] == "YES") compilerFlags.add ("-Wmove");
                else if (key == "CLANG_WARN_UNREACHABLE_CODE"        && configSettings[key] == "YES") compilerFlags.add ("-Wunreachable-code");
                else if (key == "CLANG_WARN__DUPLICATE_METHOD_MATCH" && configSettings[key] == "YES") compilerFlags.add ("-Wduplicate-method-match");
                else if (key == "GCC_INLINES_ARE_PRIVATE_EXTERN"     && configSettings[key] == "YES") compilerFlags.add ("-fvisibility-inlines-hidden");
                else if (key == "GCC_NO_COMMON_BLOCKS"               && configSettings[key] == "YES") compilerFlags.add ("-fno-common");
                else if (key == "GCC_WARN_ABOUT_RETURN_TYPE"         && configSettings[key] != "YES") compilerFlags.add (configSettings[key] == "YES_ERROR" ? "-Werror=return-type" : "-Wno-return-type");
                else if (key == "GCC_WARN_TYPECHECK_CALLS_TO_PRINTF" && configSettings[key] != "YES") compilerFlags.add ("-Wno-format");
                else if (key == "GCC_WARN_UNINITIALIZED_AUTOS")
                {
                    if      (configSettings[key] == "YES")            compilerFlags.add ("-Wuninitialized");
                    else if (configSettings[key] == "YES_AGGRESSIVE") compilerFlags.add ("--Wconditional-uninitialized");
                    else                                              compilerFlags.add (")-Wno-uninitialized");
                }
                else if (key == "WARNING_CFLAGS") compilerFlags.add (configSettings[key].unquoted());
            }

            out << addToCMakeVariable ("CMAKE_CXX_FLAGS", compilerFlags.joinIntoString (" ")) << newLine
                << addToCMakeVariable ("CMAKE_C_FLAGS", "${CMAKE_CXX_FLAGS}") << newLine
                << newLine;

            out << "endif (" << buildTypeCondition << ")" << newLine
                << newLine;
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (CLionProjectExporter)
};
