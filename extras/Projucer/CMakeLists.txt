# ==============================================================================
#
#  This file is part of the JUCE framework.
#  Copyright (c) Raw Material Software Limited
#
#  JUCE is an open source framework subject to commercial or open source
#  licensing.
#
#  By downloading, installing, or using the JUCE framework, or combining the
#  JUCE framework with any other source code, object code, content or any other
#  copyrightable work, you agree to the terms of the JUCE End User Licence
#  Agreement, and all incorporated terms including the JUCE Privacy Policy and
#  the JUCE Website Terms of Service, as applicable, which will bind you. If you
#  do not agree to the terms of these agreements, we will not license the JUCE
#  framework to you, and you must discontinue the installation or download
#  process and cease use of the JUCE framework.
#
#  JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
#  JUCE Privacy Policy: https://juce.com/juce-privacy-policy
#  JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/
#
#  Or:
#
#  You may also use this code under the terms of the AGPLv3:
#  https://www.gnu.org/licenses/agpl-3.0.en.html
#
#  THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
#  WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.
#
# ==============================================================================

juce_add_gui_app(Projucer
    BUNDLE_ID                           com.juce.theprojucer
    ICON_BIG                            ${CMAKE_CURRENT_SOURCE_DIR}/Source/BinaryData/Icons/juce_icon.png
    DOCUMENT_EXTENSIONS                 jucer
    NEEDS_CURL                          TRUE
    NEEDS_WEB_BROWSER                   TRUE)

juce_generate_juce_header(Projucer)

target_sources(Projucer PRIVATE
    Source/Application/jucer_AutoUpdater.cpp
    Source/Application/jucer_CommandLine.cpp
    Source/Application/jucer_Main.cpp
    Source/Application/jucer_MainWindow.cpp
    Source/Application/StartPage/jucer_StartPageComponent.cpp
    Source/Application/StartPage/jucer_NewProjectWizard.cpp
    Source/CodeEditor/jucer_DocumentEditorComponent.cpp
    Source/CodeEditor/jucer_OpenDocumentManager.cpp
    Source/CodeEditor/jucer_SourceCodeEditor.cpp
    Source/Project/Modules/jucer_Modules.cpp
    Source/Project/UI/jucer_HeaderComponent.cpp
    Source/Project/jucer_Project.cpp
    Source/ProjectSaving/jucer_ProjectExporter.cpp
    Source/ProjectSaving/jucer_ProjectSaver.cpp
    Source/ProjectSaving/jucer_ResourceFile.cpp
    Source/Settings/jucer_AppearanceSettings.cpp
    Source/Settings/jucer_StoredSettings.cpp
    Source/Utility/Helpers/jucer_CodeHelpers.cpp
    Source/Utility/Helpers/jucer_FileHelpers.cpp
    Source/Utility/Helpers/jucer_MiscUtilities.cpp
    Source/Utility/Helpers/jucer_NewFileWizard.cpp
    Source/Utility/Helpers/jucer_VersionInfo.cpp
    Source/Utility/PIPs/jucer_PIPGenerator.cpp
    Source/Utility/UI/jucer_Icons.cpp
    Source/Utility/UI/jucer_JucerTreeViewBase.cpp
    Source/Utility/UI/jucer_ProjucerLookAndFeel.cpp
    Source/Utility/UI/jucer_SlidingPanelComponent.cpp)

target_compile_definitions(Projucer PRIVATE
    JUCE_ALLOW_STATIC_NULL_VARIABLES=0
    JUCE_LOAD_CURL_SYMBOLS_LAZILY=1
    JUCE_LOG_ASSERTIONS=1
    JUCE_STRICT_REFCOUNTEDPOINTER=1
    JUCE_USE_CURL=1
    JUCE_WEB_BROWSER=0
    # This is a temporary workaround to allow builds to complete on Xcode 15.
    # Add -Wl,-ld_classic to the OTHER_LDFLAGS build setting if you need to
    # deploy to older versions of macOS.
    JUCE_SILENCE_XCODE_15_LINKER_WARNING=1)

juce_add_binary_data(ProjucerData SOURCES
    Source/BinaryData/Icons/background_logo.svg
    Source/BinaryData/Icons/export_android.svg
    Source/BinaryData/Icons/export_linux.svg
    Source/BinaryData/Icons/export_visualStudio.svg
    Source/BinaryData/Icons/export_xcode.svg
    Source/BinaryData/Icons/juce_icon.png
    Source/BinaryData/Icons/wizard_AnimatedApp.svg
    Source/BinaryData/Icons/wizard_AudioApp.svg
    Source/BinaryData/Icons/wizard_AudioPlugin.svg
    Source/BinaryData/Icons/wizard_ConsoleApp.svg
    Source/BinaryData/Icons/wizard_DLL.svg
    Source/BinaryData/Icons/wizard_GUI.svg
    Source/BinaryData/Icons/wizard_Highlight.svg
    Source/BinaryData/Icons/wizard_OpenGL.svg
    Source/BinaryData/Icons/wizard_Openfile.svg
    Source/BinaryData/Icons/wizard_StaticLibrary.svg
    Source/BinaryData/Templates/jucer_AnimatedComponentSimpleTemplate.h
    Source/BinaryData/Templates/jucer_AnimatedComponentTemplate.cpp
    Source/BinaryData/Templates/jucer_AnimatedComponentTemplate.h
    Source/BinaryData/Templates/jucer_AudioComponentSimpleTemplate.h
    Source/BinaryData/Templates/jucer_AudioComponentTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioComponentTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginARADocumentControllerTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginARADocumentControllerTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginARAPlaybackRendererTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginARAPlaybackRendererTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginEditorTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginEditorTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginARAEditorTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginARAEditorTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginFilterTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginFilterTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginARAFilterTemplate.h
    Source/BinaryData/Templates/jucer_ComponentTemplate.cpp
    Source/BinaryData/Templates/jucer_ComponentTemplate.h
    Source/BinaryData/Templates/jucer_ContentCompSimpleTemplate.h
    Source/BinaryData/Templates/jucer_ContentCompTemplate.cpp
    Source/BinaryData/Templates/jucer_ContentCompTemplate.h
    Source/BinaryData/Templates/jucer_InlineComponentTemplate.h
    Source/BinaryData/Templates/jucer_MainConsoleAppTemplate.cpp
    Source/BinaryData/Templates/jucer_MainTemplate_NoWindow.cpp
    Source/BinaryData/Templates/jucer_MainTemplate_Window.cpp
    Source/BinaryData/Templates/jucer_NewComponentTemplate.cpp
    Source/BinaryData/Templates/jucer_NewComponentTemplate.h
    Source/BinaryData/Templates/jucer_NewCppFileTemplate.cpp
    Source/BinaryData/Templates/jucer_NewCppFileTemplate.h
    Source/BinaryData/Templates/jucer_NewInlineComponentTemplate.h
    Source/BinaryData/Templates/jucer_OpenGLComponentSimpleTemplate.h
    Source/BinaryData/Templates/jucer_OpenGLComponentTemplate.cpp
    Source/BinaryData/Templates/jucer_OpenGLComponentTemplate.h
    Source/BinaryData/Templates/jucer_PIPAudioProcessorTemplate.h
    Source/BinaryData/Templates/jucer_PIPTemplate.h
    Source/BinaryData/colourscheme_dark.xml
    Source/BinaryData/colourscheme_light.xml
    Source/BinaryData/gradle/LICENSE
    Source/BinaryData/gradle/gradle-wrapper.jar
    Source/BinaryData/gradle/gradlew
    Source/BinaryData/gradle/gradlew.bat
    Source/BinaryData/juce_SimpleBinaryBuilder.cpp

    ../Build/CMake/JuceLV2Defines.h.in
    ../Build/CMake/LaunchScreen.storyboard
    ../Build/CMake/PIPAudioProcessor.cpp.in
    ../Build/CMake/PIPAudioProcessorWithARA.cpp.in
    ../Build/CMake/PIPComponent.cpp.in
    ../Build/CMake/PIPConsole.cpp.in
    ../Build/CMake/RecentFilesMenuTemplate.nib
    ../Build/CMake/UnityPluginGUIScript.cs.in
    ../Build/CMake/juce_runtime_arch_detection.cpp
    ../Build/CMake/juce_LinuxSubprocessHelper.cpp)

target_link_libraries(Projucer PRIVATE
    ProjucerData
    juce::juce_build_tools
    juce::juce_cryptography
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
