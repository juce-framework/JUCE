# ==============================================================================
#
#  This file is part of the JUCE library.
#  Copyright (c) 2020 - Raw Material Software Limited
#
#  JUCE is an open source library subject to commercial or open-source
#  licensing.
#
#  By using JUCE, you agree to the terms of both the JUCE 6 End-User License
#  Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).
#
#  End User License Agreement: www.juce.com/juce-6-licence
#  Privacy Policy: www.juce.com/juce-privacy-policy
#
#  Or: You may also use this code under the terms of the GPL v3 (see
#  www.gnu.org/licenses).
#
#  JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
#  EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
#  DISCLAIMED.
#
# ==============================================================================

juce_add_gui_app(Projucer
    BUNDLE_ID                           com.juce.theprojucer
    ICON_BIG                            ${CMAKE_CURRENT_SOURCE_DIR}/Source/BinaryData/Icons/juce_icon.png
    DOCUMENT_EXTENSIONS                 jucer
    NEEDS_CURL                          TRUE
    NEEDS_WEB_BROWSER                   TRUE)

juce_generate_juce_header(Projucer)

# This is to work around a bug with how cmake computes language standard flags with
# target_compile_features
set_target_properties(Projucer PROPERTIES CXX_STANDARD 11)

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
    Source/ComponentEditor/Components/jucer_ComponentTypeHandler.cpp
    Source/ComponentEditor/Documents/jucer_ButtonDocument.cpp
    Source/ComponentEditor/Documents/jucer_ComponentDocument.cpp
    Source/ComponentEditor/PaintElements/jucer_ColouredElement.cpp
    Source/ComponentEditor/PaintElements/jucer_PaintElement.cpp
    Source/ComponentEditor/PaintElements/jucer_PaintElementGroup.cpp
    Source/ComponentEditor/PaintElements/jucer_PaintElementImage.cpp
    Source/ComponentEditor/PaintElements/jucer_PaintElementPath.cpp
    Source/ComponentEditor/UI/jucer_ComponentLayoutEditor.cpp
    Source/ComponentEditor/UI/jucer_ComponentOverlayComponent.cpp
    Source/ComponentEditor/UI/jucer_EditingPanelBase.cpp
    Source/ComponentEditor/UI/jucer_JucerDocumentEditor.cpp
    Source/ComponentEditor/UI/jucer_PaintRoutineEditor.cpp
    Source/ComponentEditor/UI/jucer_PaintRoutinePanel.cpp
    Source/ComponentEditor/UI/jucer_ResourceEditorPanel.cpp
    Source/ComponentEditor/UI/jucer_TestComponent.cpp
    Source/ComponentEditor/jucer_BinaryResources.cpp
    Source/ComponentEditor/jucer_ComponentLayout.cpp
    Source/ComponentEditor/jucer_GeneratedCode.cpp
    Source/ComponentEditor/jucer_JucerDocument.cpp
    Source/ComponentEditor/jucer_ObjectTypes.cpp
    Source/ComponentEditor/jucer_PaintRoutine.cpp
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
    JUCE_WEB_BROWSER=0)

juce_add_binary_data(ProjucerData SOURCES
    Source/BinaryData/Icons/background_logo.svg
    Source/BinaryData/Icons/export_android.svg
    Source/BinaryData/Icons/export_clion.svg
    Source/BinaryData/Icons/export_codeBlocks.svg
    Source/BinaryData/Icons/export_linux.svg
    Source/BinaryData/Icons/export_visualStudio.svg
    Source/BinaryData/Icons/export_xcode.svg
    Source/BinaryData/Icons/gpl_logo.svg
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
    Source/BinaryData/Templates/jucer_AudioPluginEditorTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginEditorTemplate.h
    Source/BinaryData/Templates/jucer_AudioPluginFilterTemplate.cpp
    Source/BinaryData/Templates/jucer_AudioPluginFilterTemplate.h
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

    ../Build/CMake/LaunchScreen.storyboard
    ../Build/CMake/PIPAudioProcessor.cpp.in
    ../Build/CMake/PIPComponent.cpp.in
    ../Build/CMake/PIPConsole.cpp.in
    ../Build/CMake/RecentFilesMenuTemplate.nib
    ../Build/CMake/UnityPluginGUIScript.cs.in
    ../Build/CMake/juce_runtime_arch_detection.cpp)

target_link_libraries(Projucer PRIVATE
    ProjucerData
    juce::juce_build_tools
    juce::juce_cryptography
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags)
