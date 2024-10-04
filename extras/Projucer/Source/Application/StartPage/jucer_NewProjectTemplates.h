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

#include "../../Utility/Helpers/jucer_MiscUtilities.h"

//==============================================================================
namespace NewProjectTemplates
{
    enum class ProjectCategory
    {
        application,
        plugin,
        library
    };

    inline String getProjectCategoryString (ProjectCategory category)
    {
        if (category == ProjectCategory::application)  return "Application";
        if (category == ProjectCategory::plugin)       return "Plug-In";
        if (category == ProjectCategory::library)      return "Library";

        jassertfalse;
        return "Unknown";
    }

    enum class FileCreationOptions
    {
        noFiles,
        main,
        header,
        headerAndCpp,
        processorAndEditor,
        araPluginFiles
    };

    using FilenameAndContent = std::pair<String, String>;
    using OptionAndFilenameAndContent = std::pair<FileCreationOptions, std::vector<FilenameAndContent>>;
    using OptionsAndFiles = std::vector<OptionAndFilenameAndContent>;

    struct ProjectTemplate
    {
        ProjectCategory category;
        String displayName, description, projectTypeString;

        const char* icon;
        StringArray requiredModules;
        OptionsAndFiles fileOptionsAndFiles;
        FileCreationOptions defaultFileOption;

        std::vector<FilenameAndContent> getFilesForOption (FileCreationOptions option) const
        {
            auto iter = std::find_if (fileOptionsAndFiles.begin(), fileOptionsAndFiles.end(),
                                      [option] (const OptionAndFilenameAndContent& opt) { return opt.first == option; });

            if (iter != fileOptionsAndFiles.end())
                return iter->second;

            return {};
        }
    };

    inline bool isApplication (const ProjectTemplate& t) noexcept  { return t.category == ProjectCategory::application; }
    inline bool isPlugin (const ProjectTemplate& t) noexcept       { return t.category == ProjectCategory::plugin; }
    inline bool isLibrary (const ProjectTemplate& t) noexcept      { return t.category == ProjectCategory::library; }

    //==============================================================================
    inline var getVarForFileOption (FileCreationOptions opt)
    {
        if (opt == FileCreationOptions::noFiles)             return "none";
        if (opt == FileCreationOptions::main)                return "main";
        if (opt == FileCreationOptions::header)              return "header";
        if (opt == FileCreationOptions::headerAndCpp)        return "headercpp";
        if (opt == FileCreationOptions::processorAndEditor)  return "processoreditor";
        if (opt == FileCreationOptions::araPluginFiles)      return "arapluginfiles";

        jassertfalse;
        return {};
    }

    inline FileCreationOptions getFileOptionForVar (var opt)
    {
        if (opt == "none")             return FileCreationOptions::noFiles;
        if (opt == "main")             return FileCreationOptions::main;
        if (opt == "header")           return FileCreationOptions::header;
        if (opt == "headercpp")        return FileCreationOptions::headerAndCpp;
        if (opt == "processoreditor")  return FileCreationOptions::processorAndEditor;
        if (opt == "arapluginfiles")   return FileCreationOptions::araPluginFiles;

        jassertfalse;
        return {};
    }

    inline String getStringForFileOption (FileCreationOptions opt)
    {
        if (opt == FileCreationOptions::noFiles)            return "No Files";
        if (opt == FileCreationOptions::main)               return "Main.cpp";
        if (opt == FileCreationOptions::header)             return "Main.cpp + .h";
        if (opt == FileCreationOptions::headerAndCpp)       return "Main.cpp + .h/.cpp ";
        if (opt == FileCreationOptions::processorAndEditor) return "Processor and Editor";
        if (opt == FileCreationOptions::araPluginFiles)     return "ARA Plugin Files";

        jassertfalse;
        return {};
    }

    //==============================================================================
    template <typename... Strings>
    inline StringArray addAndReturn (StringArray arr, Strings... strings)
    {
        arr.addArray ({ strings... });
        return arr;
    }

    inline std::vector<ProjectTemplate> getAllTemplates()
    {
        return
        {
            { ProjectCategory::application,
              "Blank", "Creates a blank JUCE GUI application.",
              build_tools::ProjectType_GUIApp::getTypeName(),
              BinaryData::wizard_GUI_svg,
              getModulesRequiredForComponent(),
              {},
              FileCreationOptions::noFiles
            },

            { ProjectCategory::application,
              "GUI", "Creates a blank JUCE GUI application with a single window component.",
              build_tools::ProjectType_GUIApp::getTypeName(),
              BinaryData::wizard_GUI_svg,
              getModulesRequiredForComponent(),
              {
                { FileCreationOptions::noFiles, {} },
                { FileCreationOptions::main, { { "Main.cpp", "jucer_MainTemplate_NoWindow_cpp" } } },
                { FileCreationOptions::header, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                 { "MainComponent.h", "jucer_ContentCompSimpleTemplate_h" } } },
                { FileCreationOptions::headerAndCpp, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                       { "MainComponent.h", "jucer_ContentCompTemplate_h" },
                                                       { "MainComponent.cpp", "jucer_ContentCompTemplate_cpp" } } }
              },
              FileCreationOptions::headerAndCpp },

            { ProjectCategory::application,
              "Audio", "Creates a blank JUCE GUI application with a single window component and audio and MIDI in/out functions.",
              build_tools::ProjectType_GUIApp::getTypeName(),
              BinaryData::wizard_AudioApp_svg,
              addAndReturn (getModulesRequiredForComponent(), "juce_audio_basics", "juce_audio_devices", "juce_audio_formats",
                                                              "juce_audio_processors", "juce_audio_utils", "juce_gui_extra"),
              {
                { FileCreationOptions::header, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                 { "MainComponent.h", "jucer_AudioComponentSimpleTemplate_h" } } },
                { FileCreationOptions::headerAndCpp, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                       { "MainComponent.h", "jucer_AudioComponentTemplate_h" },
                                                       { "MainComponent.cpp", "jucer_AudioComponentTemplate_cpp" } } }
              },
              FileCreationOptions::headerAndCpp },

            { ProjectCategory::application,
              "Console", "Creates a command-line application without GUI support.",
              build_tools::ProjectType_ConsoleApp::getTypeName(),
              BinaryData::wizard_ConsoleApp_svg,
              getModulesRequiredForConsole(),
              {
                  { FileCreationOptions::noFiles, {} },
                  { FileCreationOptions::main, { { "Main.cpp", "jucer_MainConsoleAppTemplate_cpp" } } }
              },
             FileCreationOptions::main },

            { ProjectCategory::application,
              "Animated", "Creates a JUCE GUI application which draws an animated graphical display.",
              build_tools::ProjectType_GUIApp::getTypeName(),
              BinaryData::wizard_AnimatedApp_svg,
              addAndReturn (getModulesRequiredForComponent(), "juce_gui_extra"),
              {
                { FileCreationOptions::header, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                 { "MainComponent.h", "jucer_AudioComponentSimpleTemplate_h" } } },
                { FileCreationOptions::headerAndCpp, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                       { "MainComponent.h", "jucer_AnimatedComponentTemplate_h" },
                                                       { "MainComponent.cpp", "jucer_AnimatedComponentTemplate_cpp" } } }
              },
              FileCreationOptions::headerAndCpp },

            { ProjectCategory::application,
              "OpenGL", "Creates a blank JUCE application with a single window component. "
                        "This component supports openGL drawing features including 3D model import and GLSL shaders.",
              build_tools::ProjectType_GUIApp::getTypeName(),
              BinaryData::wizard_OpenGL_svg,
              addAndReturn (getModulesRequiredForComponent(), "juce_gui_extra", "juce_opengl"),
              {
                { FileCreationOptions::header, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                 { "MainComponent.h", "jucer_AudioComponentSimpleTemplate_h" } } },
                { FileCreationOptions::headerAndCpp, { { "Main.cpp", "jucer_MainTemplate_Window_cpp" },
                                                       { "MainComponent.h", "jucer_OpenGLComponentTemplate_h" },
                                                       { "MainComponent.cpp", "jucer_OpenGLComponentTemplate_cpp" } } }
              },
              FileCreationOptions::headerAndCpp },

            { ProjectCategory::plugin,
              "Basic", "Creates an audio plug-in with a single window GUI and audio/MIDI IO functions.",
              build_tools::ProjectType_AudioPlugin::getTypeName(),
              BinaryData::wizard_AudioPlugin_svg,
              getModulesRequiredForAudioProcessor(),
              {
                { FileCreationOptions::processorAndEditor, { { "PluginProcessor.cpp", "jucer_AudioPluginFilterTemplate_cpp" },
                                                             { "PluginProcessor.h",   "jucer_AudioPluginFilterTemplate_h" },
                                                             { "PluginEditor.cpp",    "jucer_AudioPluginEditorTemplate_cpp" },
                                                             { "PluginEditor.h",      "jucer_AudioPluginEditorTemplate_h" } } }
              },
              FileCreationOptions::processorAndEditor
            },

            { ProjectCategory::plugin,
              "ARA", "Creates an ARA audio plug-in, augmenting the basic audio plug-in with ARA functionality.",
              build_tools::ProjectType_ARAAudioPlugin::getTypeName(),
              BinaryData::wizard_AudioPlugin_svg,
              getModulesRequiredForAudioProcessor(),
              {
                  { FileCreationOptions::araPluginFiles, { { "PluginProcessor.cpp",               "jucer_AudioPluginFilterTemplate_cpp" },
                                                           { "PluginProcessor.h",                 "jucer_AudioPluginARAFilterTemplate_h" },
                                                           { "PluginEditor.cpp",                  "jucer_AudioPluginARAEditorTemplate_cpp" },
                                                           { "PluginEditor.h",                    "jucer_AudioPluginARAEditorTemplate_h" },
                                                           { "PluginARADocumentController.cpp",   "jucer_AudioPluginARADocumentControllerTemplate_cpp" },
                                                           { "PluginARADocumentController.h",     "jucer_AudioPluginARADocumentControllerTemplate_h" },
                                                           { "PluginARAPlaybackRenderer.cpp",     "jucer_AudioPluginARAPlaybackRendererTemplate_cpp" },
                                                           { "PluginARAPlaybackRenderer.h",       "jucer_AudioPluginARAPlaybackRendererTemplate_h" }} }
              },
              FileCreationOptions::araPluginFiles
            },

            { ProjectCategory::library,
              "Static Library", "Creates a static library.",
              build_tools::ProjectType_StaticLibrary::getTypeName(),
              BinaryData::wizard_StaticLibrary_svg,
              getModulesRequiredForConsole(),
              {},
              FileCreationOptions::noFiles
            },

            { ProjectCategory::library,
              "Dynamic Library", "Creates a dynamic library.",
              build_tools::ProjectType_DLL::getTypeName(),
              BinaryData::wizard_DLL_svg,
              getModulesRequiredForConsole(),
              {},
              FileCreationOptions::noFiles
            }
        };
    }
}
