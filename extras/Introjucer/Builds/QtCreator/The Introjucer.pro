# Automatically generated qmake file, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

TEMPLATE = app
CONFIG  -= qt
CONFIG  += warn_off

CONFIG(release, debug|release){
    DESTDIR     = build/release/
    OBJECTS_DIR = build/release/intermediate/
    TARGET = Introjucer
}
CONFIG(debug, debug|release){
    DESTDIR     = build/debug/
    OBJECTS_DIR = build/debug/intermediate/
    TARGET = Introjucer
}

# Compiler flags
QMAKE_CXXFLAGS = -std=c++11 -Wall
unix:  QMAKE_CXXFLAGS += -I/usr/include/freetype2 -I/usr/include -D "LINUX=1"
win32: QMAKE_CXXFLAGS += -mstackrealign -D__MINGW__=1 -D__MINGW_EXTENSION=

QMAKE_CXXFLAGS_DEBUG   = -g -ggdb  -O0
CONFIG(debug, debug|release){
    INCLUDEPATH = \
        ../../JuceLibraryCode \
        ../../../../modules \

    DEFINES += \
        "DEBUG=1" \
        "_DEBUG=1" \
        "JUCER_QT_CREATOR_D5F46ABF=1" \
        "JUCE_APP_VERSION=3.1.0" \
        "JUCE_APP_VERSION_HEX=0x30100" \

}

QMAKE_CXXFLAGS_RELEASE =  -O3
CONFIG(release, debug|release){
    INCLUDEPATH = \
        ../../JuceLibraryCode \
        ../../../../modules \

    DEFINES += \
        "NDEBUG=1" \
        "JUCER_QT_CREATOR_D5F46ABF=1" \
        "JUCE_APP_VERSION=3.1.0" \
        "JUCE_APP_VERSION_HEX=0x30100" \

}


QMAKE_CFLAGS         = $$QMAKE_CXXFLAGS
QMAKE_CFLAGS_RELEASE = $$QMAKE_CXXFLAGS_RELEASE
QMAKE_CFLAGS_DEBUG   = $$QMAKE_CXXFLAGS_DEBUG


# Linker flags
LIBS = -L$$DESTDIR 
unix:  LIBS += -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt
win32: LIBS += -lgdi32 -luser32 -lkernel32 -lcomctl32 -lpthread -lcomdlg32 -limm32 -lole32 -loleaut32 -lrpcrt4 -lshlwapi -luuid -lversion -lwininet -lwinmm -lws2_32 -lwsock32
win32: QMAKE_LFLAGS += -static-libstdc++ -static-libgcc
QMAKE_LFLAGS += 
QMAKE_LFLAGS_DEBUG += -fvisibility=hidden

# Source and header files
SOURCES = \
	"../../Source/Application/jucer_AppearanceSettings.cpp" \
	"../../Source/Application/jucer_CommandLine.cpp" \
	"../../Source/Application/jucer_DocumentEditorComponent.cpp" \
	"../../Source/Application/jucer_Main.cpp" \
	"../../Source/Application/jucer_MainWindow.cpp" \
	"../../Source/Application/jucer_OpenDocumentManager.cpp" \
	"../../Source/Code Editor/jucer_SourceCodeEditor.cpp" \
	"../../Source/ComponentEditor/components/jucer_ComponentTypeHandler.cpp" \
	"../../Source/ComponentEditor/documents/jucer_ButtonDocument.cpp" \
	"../../Source/ComponentEditor/documents/jucer_ComponentDocument.cpp" \
	"../../Source/ComponentEditor/paintelements/jucer_ColouredElement.cpp" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElement.cpp" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementPath.cpp" \
	"../../Source/ComponentEditor/ui/jucer_ComponentLayoutEditor.cpp" \
	"../../Source/ComponentEditor/ui/jucer_ComponentOverlayComponent.cpp" \
	"../../Source/ComponentEditor/ui/jucer_EditingPanelBase.cpp" \
	"../../Source/ComponentEditor/ui/jucer_JucerDocumentEditor.cpp" \
	"../../Source/ComponentEditor/ui/jucer_PaintRoutineEditor.cpp" \
	"../../Source/ComponentEditor/ui/jucer_PaintRoutinePanel.cpp" \
	"../../Source/ComponentEditor/ui/jucer_ResourceEditorPanel.cpp" \
	"../../Source/ComponentEditor/ui/jucer_TestComponent.cpp" \
	"../../Source/ComponentEditor/jucer_BinaryResources.cpp" \
	"../../Source/ComponentEditor/jucer_ComponentLayout.cpp" \
	"../../Source/ComponentEditor/jucer_GeneratedCode.cpp" \
	"../../Source/ComponentEditor/jucer_JucerDocument.cpp" \
	"../../Source/ComponentEditor/jucer_ObjectTypes.cpp" \
	"../../Source/ComponentEditor/jucer_PaintRoutine.cpp" \
	"../../Source/Project Saving/jucer_ProjectExporter.cpp" \
	"../../Source/Project Saving/jucer_ResourceFile.cpp" \
	"../../Source/Project/jucer_Module.cpp" \
	"../../Source/Project/jucer_Project.cpp" \
	"../../Source/Project/jucer_ProjectContentComponent.cpp" \
	"../../Source/Project/jucer_ProjectType.cpp" \
	"../../Source/Utility/jucer_CodeHelpers.cpp" \
	"../../Source/Utility/jucer_FileHelpers.cpp" \
	"../../Source/Utility/jucer_Icons.cpp" \
	"../../Source/Utility/jucer_JucerTreeViewBase.cpp" \
	"../../Source/Utility/jucer_MiscUtilities.cpp" \
	"../../Source/Utility/jucer_SlidingPanelComponent.cpp" \
	"../../Source/Utility/jucer_StoredSettings.cpp" \
	"../../Source/Wizards/jucer_NewFileWizard.cpp" \
	"../../Source/Wizards/jucer_NewProjectWizardClasses.cpp" \
	"../../JuceLibraryCode/BinaryData.cpp" \
	"../../../../modules/juce_core/juce_core.cpp" \
	"../../../../modules/juce_cryptography/juce_cryptography.cpp" \
	"../../../../modules/juce_data_structures/juce_data_structures.cpp" \
	"../../../../modules/juce_events/juce_events.cpp" \
	"../../../../modules/juce_graphics/juce_graphics.cpp" \
	"../../../../modules/juce_gui_basics/juce_gui_basics.cpp" \
	"../../../../modules/juce_gui_extra/juce_gui_extra.cpp" \


HEADERS = \
	"../../Source/Application/jucer_AppearanceSettings.h" \
	"../../Source/Application/jucer_Application.h" \
	"../../Source/Application/jucer_AutoUpdater.h" \
	"../../Source/Application/jucer_CommandIDs.h" \
	"../../Source/Application/jucer_CommandLine.h" \
	"../../Source/Application/jucer_CommonHeaders.h" \
	"../../Source/Application/jucer_DocumentEditorComponent.h" \
	"../../Source/Application/jucer_FilePreviewComponent.h" \
	"../../Source/jucer_Headers.h" \
	"../../Source/Application/jucer_MainWindow.h" \
	"../../Source/Application/jucer_OpenDocumentManager.h" \
	"../../Source/Code Editor/jucer_SourceCodeEditor.h" \
	"../../Source/ComponentEditor/components/jucer_ButtonHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ComboBoxHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ComponentNameProperty.h" \
	"../../Source/ComponentEditor/components/jucer_ComponentTypeHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ComponentUndoableAction.h" \
	"../../Source/ComponentEditor/components/jucer_GenericComponentHandler.h" \
	"../../Source/ComponentEditor/components/jucer_GroupComponentHandler.h" \
	"../../Source/ComponentEditor/components/jucer_HyperlinkButtonHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ImageButtonHandler.h" \
	"../../Source/ComponentEditor/components/jucer_JucerComponentHandler.h" \
	"../../Source/ComponentEditor/components/jucer_LabelHandler.h" \
	"../../Source/ComponentEditor/components/jucer_SliderHandler.h" \
	"../../Source/ComponentEditor/components/jucer_TabbedComponentHandler.h" \
	"../../Source/ComponentEditor/components/jucer_TextButtonHandler.h" \
	"../../Source/ComponentEditor/components/jucer_TextEditorHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ToggleButtonHandler.h" \
	"../../Source/ComponentEditor/components/jucer_TreeViewHandler.h" \
	"../../Source/ComponentEditor/components/jucer_ViewportHandler.h" \
	"../../Source/ComponentEditor/documents/jucer_ButtonDocument.h" \
	"../../Source/ComponentEditor/documents/jucer_ComponentDocument.h" \
	"../../Source/ComponentEditor/paintelements/jucer_ColouredElement.h" \
	"../../Source/ComponentEditor/paintelements/jucer_ElementSiblingComponent.h" \
	"../../Source/ComponentEditor/paintelements/jucer_FillType.h" \
	"../../Source/ComponentEditor/paintelements/jucer_GradientPointComponent.h" \
	"../../Source/ComponentEditor/paintelements/jucer_ImageResourceProperty.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElement.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementEllipse.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementGroup.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementImage.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementPath.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementRectangle.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementRoundedRectangle.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementText.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PaintElementUndoableAction.h" \
	"../../Source/ComponentEditor/paintelements/jucer_PointComponent.h" \
	"../../Source/ComponentEditor/paintelements/jucer_StrokeType.h" \
	"../../Source/ComponentEditor/properties/jucer_ColourPropertyComponent.h" \
	"../../Source/ComponentEditor/properties/jucer_ComponentBooleanProperty.h" \
	"../../Source/ComponentEditor/properties/jucer_ComponentChoiceProperty.h" \
	"../../Source/ComponentEditor/properties/jucer_ComponentColourProperty.h" \
	"../../Source/ComponentEditor/properties/jucer_ComponentTextProperty.h" \
	"../../Source/ComponentEditor/properties/jucer_FilePropertyComponent.h" \
	"../../Source/ComponentEditor/properties/jucer_FontPropertyComponent.h" \
	"../../Source/ComponentEditor/properties/jucer_JustificationProperty.h" \
	"../../Source/ComponentEditor/properties/jucer_PositionPropertyBase.h" \
	"../../Source/ComponentEditor/ui/jucer_ComponentLayoutEditor.h" \
	"../../Source/ComponentEditor/ui/jucer_ComponentLayoutPanel.h" \
	"../../Source/ComponentEditor/ui/jucer_ComponentOverlayComponent.h" \
	"../../Source/ComponentEditor/ui/jucer_EditingPanelBase.h" \
	"../../Source/ComponentEditor/ui/jucer_JucerCommandIDs.h" \
	"../../Source/ComponentEditor/ui/jucer_JucerDocumentEditor.h" \
	"../../Source/ComponentEditor/ui/jucer_PaintRoutineEditor.h" \
	"../../Source/ComponentEditor/ui/jucer_PaintRoutinePanel.h" \
	"../../Source/ComponentEditor/ui/jucer_RelativePositionedRectangle.h" \
	"../../Source/ComponentEditor/ui/jucer_ResourceEditorPanel.h" \
	"../../Source/ComponentEditor/ui/jucer_SnapGridPainter.h" \
	"../../Source/ComponentEditor/ui/jucer_TestComponent.h" \
	"../../Source/ComponentEditor/jucer_BinaryResources.h" \
	"../../Source/ComponentEditor/jucer_ComponentLayout.h" \
	"../../Source/ComponentEditor/jucer_GeneratedCode.h" \
	"../../Source/ComponentEditor/jucer_JucerDocument.h" \
	"../../Source/ComponentEditor/jucer_ObjectTypes.h" \
	"../../Source/ComponentEditor/jucer_PaintRoutine.h" \
	"../../Source/ComponentEditor/jucer_UtilityFunctions.h" \
	"../../Source/Project Saving/jucer_ProjectExport_Android.h" \
	"../../Source/Project Saving/jucer_ProjectExport_CodeBlocks.h" \
	"../../Source/Project Saving/jucer_ProjectExport_Make.h" \
	"../../Source/Project Saving/jucer_ProjectExport_MSVC.h" \
	"../../Source/Project Saving/jucer_ProjectExport_XCode.h" \
	"../../Source/Project Saving/jucer_ProjectExporter.h" \
	"../../Source/Project Saving/jucer_ProjectSaver.h" \
	"../../Source/Project Saving/jucer_ResourceFile.h" \
	"../../Source/Project/jucer_AudioPluginModule.h" \
	"../../Source/Project/jucer_ConfigTree_Base.h" \
	"../../Source/Project/jucer_ConfigTree_Exporter.h" \
	"../../Source/Project/jucer_ConfigTree_Modules.h" \
	"../../Source/Project/jucer_GroupInformationComponent.h" \
	"../../Source/Project/jucer_Module.h" \
	"../../Source/Project/jucer_ModulesPanel.h" \
	"../../Source/Project/jucer_Project.h" \
	"../../Source/Project/jucer_ProjectContentComponent.h" \
	"../../Source/Project/jucer_ProjectTree_Base.h" \
	"../../Source/Project/jucer_ProjectTree_File.h" \
	"../../Source/Project/jucer_ProjectTree_Group.h" \
	"../../Source/Project/jucer_ProjectType.h" \
	"../../Source/Utility/jucer_CodeHelpers.h" \
	"../../Source/Utility/jucer_Colours.h" \
	"../../Source/Utility/jucer_FileHelpers.h" \
	"../../Source/Utility/jucer_Icons.h" \
	"../../Source/Utility/jucer_JucerTreeViewBase.h" \
	"../../Source/Utility/jucer_MiscUtilities.h" \
	"../../Source/Utility/jucer_PresetIDs.h" \
	"../../Source/Utility/jucer_RelativePath.h" \
	"../../Source/Utility/jucer_SlidingPanelComponent.h" \
	"../../Source/Utility/jucer_StoredSettings.h" \
	"../../Source/Utility/jucer_TranslationTool.h" \
	"../../Source/Utility/jucer_ValueSourceHelpers.h" \
	"../../Source/Wizards/jucer_NewFileWizard.h" \
	"../../Source/Wizards/jucer_NewProjectWizard.h" \
	"../../Source/Wizards/jucer_NewProjectWizardClasses.h" \
	"../../Source/Wizards/jucer_NewProjectWizardComponent.h" \
	"../../Source/Wizards/jucer_ProjectWizard_Animated.h" \
	"../../Source/Wizards/jucer_ProjectWizard_AudioApp.h" \
	"../../Source/Wizards/jucer_ProjectWizard_AudioPlugin.h" \
	"../../Source/Wizards/jucer_ProjectWizard_Blank.h" \
	"../../Source/Wizards/jucer_ProjectWizard_Console.h" \
	"../../Source/Wizards/jucer_ProjectWizard_DLL.h" \
	"../../Source/Wizards/jucer_ProjectWizard_GUIApp.h" \
	"../../Source/Wizards/jucer_ProjectWizard_openGL.h" \
	"../../Source/Wizards/jucer_ProjectWizard_StaticLibrary.h" \
	"../../Source/Wizards/jucer_StartPageComponent.h" \
	"../../Source/Wizards/jucer_TemplateThumbnailsComponent.h" \
	"../../Source/BinaryData/jucer_AudioPluginEditorTemplate.h" \
	"../../Source/BinaryData/jucer_AudioPluginFilterTemplate.h" \
	"../../Source/BinaryData/jucer_ComponentTemplate.h" \
	"../../Source/BinaryData/jucer_ContentCompTemplate.h" \
	"../../Source/BinaryData/jucer_InlineComponentTemplate.h" \
	"../../Source/BinaryData/jucer_NewComponentTemplate.h" \
	"../../Source/BinaryData/jucer_NewCppFileTemplate.h" \
	"../../Source/BinaryData/jucer_NewInlineComponentTemplate.h" \
	"../../../../modules/juce_core/text/juce_CharacterFunctions.h" \
	"../../../../modules/juce_core/text/juce_CharPointer_ASCII.h" \
	"../../../../modules/juce_core/text/juce_CharPointer_UTF8.h" \
	"../../../../modules/juce_core/text/juce_CharPointer_UTF16.h" \
	"../../../../modules/juce_core/text/juce_CharPointer_UTF32.h" \
	"../../../../modules/juce_core/text/juce_Identifier.h" \
	"../../../../modules/juce_core/text/juce_LocalisedStrings.h" \
	"../../../../modules/juce_core/text/juce_NewLine.h" \
	"../../../../modules/juce_core/text/juce_String.h" \
	"../../../../modules/juce_core/text/juce_StringArray.h" \
	"../../../../modules/juce_core/text/juce_StringPairArray.h" \
	"../../../../modules/juce_core/text/juce_StringPool.h" \
	"../../../../modules/juce_core/text/juce_StringRef.h" \
	"../../../../modules/juce_core/text/juce_TextDiff.h" \
	"../../../../modules/juce_core/maths/juce_BigInteger.h" \
	"../../../../modules/juce_core/maths/juce_Expression.h" \
	"../../../../modules/juce_core/maths/juce_MathsFunctions.h" \
	"../../../../modules/juce_core/maths/juce_NormalisableRange.h" \
	"../../../../modules/juce_core/maths/juce_Random.h" \
	"../../../../modules/juce_core/maths/juce_Range.h" \
	"../../../../modules/juce_core/memory/juce_Atomic.h" \
	"../../../../modules/juce_core/memory/juce_ByteOrder.h" \
	"../../../../modules/juce_core/memory/juce_ContainerDeletePolicy.h" \
	"../../../../modules/juce_core/memory/juce_HeapBlock.h" \
	"../../../../modules/juce_core/memory/juce_LeakedObjectDetector.h" \
	"../../../../modules/juce_core/memory/juce_Memory.h" \
	"../../../../modules/juce_core/memory/juce_MemoryBlock.h" \
	"../../../../modules/juce_core/memory/juce_OptionalScopedPointer.h" \
	"../../../../modules/juce_core/memory/juce_ReferenceCountedObject.h" \
	"../../../../modules/juce_core/memory/juce_ScopedPointer.h" \
	"../../../../modules/juce_core/memory/juce_SharedResourcePointer.h" \
	"../../../../modules/juce_core/memory/juce_Singleton.h" \
	"../../../../modules/juce_core/memory/juce_WeakReference.h" \
	"../../../../modules/juce_core/containers/juce_AbstractFifo.h" \
	"../../../../modules/juce_core/containers/juce_Array.h" \
	"../../../../modules/juce_core/containers/juce_ArrayAllocationBase.h" \
	"../../../../modules/juce_core/containers/juce_DynamicObject.h" \
	"../../../../modules/juce_core/containers/juce_ElementComparator.h" \
	"../../../../modules/juce_core/containers/juce_HashMap.h" \
	"../../../../modules/juce_core/containers/juce_LinkedListPointer.h" \
	"../../../../modules/juce_core/containers/juce_NamedValueSet.h" \
	"../../../../modules/juce_core/containers/juce_OwnedArray.h" \
	"../../../../modules/juce_core/containers/juce_PropertySet.h" \
	"../../../../modules/juce_core/containers/juce_ReferenceCountedArray.h" \
	"../../../../modules/juce_core/containers/juce_ScopedValueSetter.h" \
	"../../../../modules/juce_core/containers/juce_SortedSet.h" \
	"../../../../modules/juce_core/containers/juce_SparseSet.h" \
	"../../../../modules/juce_core/containers/juce_Variant.h" \
	"../../../../modules/juce_core/threads/juce_ChildProcess.h" \
	"../../../../modules/juce_core/threads/juce_CriticalSection.h" \
	"../../../../modules/juce_core/threads/juce_DynamicLibrary.h" \
	"../../../../modules/juce_core/threads/juce_HighResolutionTimer.h" \
	"../../../../modules/juce_core/threads/juce_InterProcessLock.h" \
	"../../../../modules/juce_core/threads/juce_Process.h" \
	"../../../../modules/juce_core/threads/juce_ReadWriteLock.h" \
	"../../../../modules/juce_core/threads/juce_ScopedLock.h" \
	"../../../../modules/juce_core/threads/juce_ScopedReadLock.h" \
	"../../../../modules/juce_core/threads/juce_ScopedWriteLock.h" \
	"../../../../modules/juce_core/threads/juce_SpinLock.h" \
	"../../../../modules/juce_core/threads/juce_Thread.h" \
	"../../../../modules/juce_core/threads/juce_ThreadLocalValue.h" \
	"../../../../modules/juce_core/threads/juce_ThreadPool.h" \
	"../../../../modules/juce_core/threads/juce_TimeSliceThread.h" \
	"../../../../modules/juce_core/threads/juce_WaitableEvent.h" \
	"../../../../modules/juce_core/time/juce_PerformanceCounter.h" \
	"../../../../modules/juce_core/time/juce_RelativeTime.h" \
	"../../../../modules/juce_core/time/juce_Time.h" \
	"../../../../modules/juce_core/files/juce_DirectoryIterator.h" \
	"../../../../modules/juce_core/files/juce_File.h" \
	"../../../../modules/juce_core/files/juce_FileFilter.h" \
	"../../../../modules/juce_core/files/juce_FileInputStream.h" \
	"../../../../modules/juce_core/files/juce_FileOutputStream.h" \
	"../../../../modules/juce_core/files/juce_FileSearchPath.h" \
	"../../../../modules/juce_core/files/juce_MemoryMappedFile.h" \
	"../../../../modules/juce_core/files/juce_TemporaryFile.h" \
	"../../../../modules/juce_core/files/juce_WildcardFileFilter.h" \
	"../../../../modules/juce_core/network/juce_IPAddress.h" \
	"../../../../modules/juce_core/network/juce_MACAddress.h" \
	"../../../../modules/juce_core/network/juce_NamedPipe.h" \
	"../../../../modules/juce_core/network/juce_Socket.h" \
	"../../../../modules/juce_core/network/juce_URL.h" \
	"../../../../modules/juce_core/streams/juce_BufferedInputStream.h" \
	"../../../../modules/juce_core/streams/juce_FileInputSource.h" \
	"../../../../modules/juce_core/streams/juce_InputSource.h" \
	"../../../../modules/juce_core/streams/juce_InputStream.h" \
	"../../../../modules/juce_core/streams/juce_MemoryInputStream.h" \
	"../../../../modules/juce_core/streams/juce_MemoryOutputStream.h" \
	"../../../../modules/juce_core/streams/juce_OutputStream.h" \
	"../../../../modules/juce_core/streams/juce_SubregionStream.h" \
	"../../../../modules/juce_core/logging/juce_FileLogger.h" \
	"../../../../modules/juce_core/logging/juce_Logger.h" \
	"../../../../modules/juce_core/system/juce_CompilerSupport.h" \
	"../../../../modules/juce_core/system/juce_PlatformDefs.h" \
	"../../../../modules/juce_core/system/juce_StandardHeader.h" \
	"../../../../modules/juce_core/system/juce_SystemStats.h" \
	"../../../../modules/juce_core/system/juce_TargetPlatform.h" \
	"../../../../modules/juce_core/xml/juce_XmlDocument.h" \
	"../../../../modules/juce_core/xml/juce_XmlElement.h" \
	"../../../../modules/juce_core/javascript/juce_Javascript.h" \
	"../../../../modules/juce_core/javascript/juce_JSON.h" \
	"../../../../modules/juce_core/zip/juce_GZIPCompressorOutputStream.h" \
	"../../../../modules/juce_core/zip/juce_GZIPDecompressorInputStream.h" \
	"../../../../modules/juce_core/zip/juce_ZipFile.h" \
	"../../../../modules/juce_core/unit_tests/juce_UnitTest.h" \
	"../../../../modules/juce_core/misc/juce_Result.h" \
	"../../../../modules/juce_core/misc/juce_Uuid.h" \
	"../../../../modules/juce_core/misc/juce_WindowsRegistry.h" \
	"../../../../modules/juce_core/native/juce_android_JNIHelpers.h" \
	"../../../../modules/juce_core/native/juce_BasicNativeHeaders.h" \
	"../../../../modules/juce_core/native/juce_osx_ObjCHelpers.h" \
	"../../../../modules/juce_core/native/juce_posix_SharedCode.h" \
	"../../../../modules/juce_core/native/juce_win32_ComSmartPtr.h" \
	"../../../../modules/juce_core/juce_core.h" \
	"../../../../modules/juce_cryptography/encryption/juce_BlowFish.h" \
	"../../../../modules/juce_cryptography/encryption/juce_Primes.h" \
	"../../../../modules/juce_cryptography/encryption/juce_RSAKey.h" \
	"../../../../modules/juce_cryptography/hashing/juce_MD5.h" \
	"../../../../modules/juce_cryptography/hashing/juce_SHA256.h" \
	"../../../../modules/juce_cryptography/juce_cryptography.h" \
	"../../../../modules/juce_data_structures/values/juce_Value.h" \
	"../../../../modules/juce_data_structures/values/juce_ValueTree.h" \
	"../../../../modules/juce_data_structures/undomanager/juce_UndoableAction.h" \
	"../../../../modules/juce_data_structures/undomanager/juce_UndoManager.h" \
	"../../../../modules/juce_data_structures/app_properties/juce_ApplicationProperties.h" \
	"../../../../modules/juce_data_structures/app_properties/juce_PropertiesFile.h" \
	"../../../../modules/juce_data_structures/juce_data_structures.h" \
	"../../../../modules/juce_events/messages/juce_ApplicationBase.h" \
	"../../../../modules/juce_events/messages/juce_CallbackMessage.h" \
	"../../../../modules/juce_events/messages/juce_DeletedAtShutdown.h" \
	"../../../../modules/juce_events/messages/juce_Initialisation.h" \
	"../../../../modules/juce_events/messages/juce_Message.h" \
	"../../../../modules/juce_events/messages/juce_MessageListener.h" \
	"../../../../modules/juce_events/messages/juce_MessageManager.h" \
	"../../../../modules/juce_events/messages/juce_MountedVolumeListChangeDetector.h" \
	"../../../../modules/juce_events/messages/juce_NotificationType.h" \
	"../../../../modules/juce_events/timers/juce_MultiTimer.h" \
	"../../../../modules/juce_events/timers/juce_Timer.h" \
	"../../../../modules/juce_events/broadcasters/juce_ActionBroadcaster.h" \
	"../../../../modules/juce_events/broadcasters/juce_ActionListener.h" \
	"../../../../modules/juce_events/broadcasters/juce_AsyncUpdater.h" \
	"../../../../modules/juce_events/broadcasters/juce_ChangeBroadcaster.h" \
	"../../../../modules/juce_events/broadcasters/juce_ChangeListener.h" \
	"../../../../modules/juce_events/broadcasters/juce_ListenerList.h" \
	"../../../../modules/juce_events/interprocess/juce_ConnectedChildProcess.h" \
	"../../../../modules/juce_events/interprocess/juce_InterprocessConnection.h" \
	"../../../../modules/juce_events/interprocess/juce_InterprocessConnectionServer.h" \
	"../../../../modules/juce_events/native/juce_osx_MessageQueue.h" \
	"../../../../modules/juce_events/native/juce_ScopedXLock.h" \
	"../../../../modules/juce_events/native/juce_win32_HiddenMessageWindow.h" \
	"../../../../modules/juce_events/juce_events.h" \
	"../../../../modules/juce_graphics/colour/juce_Colour.h" \
	"../../../../modules/juce_graphics/colour/juce_ColourGradient.h" \
	"../../../../modules/juce_graphics/colour/juce_Colours.h" \
	"../../../../modules/juce_graphics/colour/juce_FillType.h" \
	"../../../../modules/juce_graphics/colour/juce_PixelFormats.h" \
	"../../../../modules/juce_graphics/contexts/juce_GraphicsContext.h" \
	"../../../../modules/juce_graphics/contexts/juce_LowLevelGraphicsContext.h" \
	"../../../../modules/juce_graphics/contexts/juce_LowLevelGraphicsPostScriptRenderer.h" \
	"../../../../modules/juce_graphics/contexts/juce_LowLevelGraphicsSoftwareRenderer.h" \
	"../../../../modules/juce_graphics/images/juce_Image.h" \
	"../../../../modules/juce_graphics/images/juce_ImageCache.h" \
	"../../../../modules/juce_graphics/images/juce_ImageConvolutionKernel.h" \
	"../../../../modules/juce_graphics/images/juce_ImageFileFormat.h" \
	"../../../../modules/juce_graphics/geometry/juce_AffineTransform.h" \
	"../../../../modules/juce_graphics/geometry/juce_BorderSize.h" \
	"../../../../modules/juce_graphics/geometry/juce_EdgeTable.h" \
	"../../../../modules/juce_graphics/geometry/juce_Line.h" \
	"../../../../modules/juce_graphics/geometry/juce_Path.h" \
	"../../../../modules/juce_graphics/geometry/juce_PathIterator.h" \
	"../../../../modules/juce_graphics/geometry/juce_PathStrokeType.h" \
	"../../../../modules/juce_graphics/geometry/juce_Point.h" \
	"../../../../modules/juce_graphics/geometry/juce_Rectangle.h" \
	"../../../../modules/juce_graphics/geometry/juce_RectangleList.h" \
	"../../../../modules/juce_graphics/placement/juce_Justification.h" \
	"../../../../modules/juce_graphics/placement/juce_RectanglePlacement.h" \
	"../../../../modules/juce_graphics/fonts/juce_AttributedString.h" \
	"../../../../modules/juce_graphics/fonts/juce_CustomTypeface.h" \
	"../../../../modules/juce_graphics/fonts/juce_Font.h" \
	"../../../../modules/juce_graphics/fonts/juce_GlyphArrangement.h" \
	"../../../../modules/juce_graphics/fonts/juce_TextLayout.h" \
	"../../../../modules/juce_graphics/fonts/juce_Typeface.h" \
	"../../../../modules/juce_graphics/effects/juce_DropShadowEffect.h" \
	"../../../../modules/juce_graphics/effects/juce_GlowEffect.h" \
	"../../../../modules/juce_graphics/effects/juce_ImageEffectFilter.h" \
	"../../../../modules/juce_graphics/native/juce_mac_CoreGraphicsContext.h" \
	"../../../../modules/juce_graphics/native/juce_mac_CoreGraphicsHelpers.h" \
	"../../../../modules/juce_graphics/native/juce_RenderingHelpers.h" \
	"../../../../modules/juce_graphics/juce_graphics.h" \
	"../../../../modules/juce_gui_basics/components/juce_CachedComponentImage.h" \
	"../../../../modules/juce_gui_basics/components/juce_Component.h" \
	"../../../../modules/juce_gui_basics/components/juce_ComponentListener.h" \
	"../../../../modules/juce_gui_basics/components/juce_Desktop.h" \
	"../../../../modules/juce_gui_basics/components/juce_ModalComponentManager.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_ComponentDragger.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_DragAndDropContainer.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_DragAndDropTarget.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_FileDragAndDropTarget.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_LassoComponent.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_MouseCursor.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_MouseEvent.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_MouseInactivityDetector.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_MouseInputSource.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_MouseListener.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_SelectedItemSet.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_TextDragAndDropTarget.h" \
	"../../../../modules/juce_gui_basics/mouse/juce_TooltipClient.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_CaretComponent.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_KeyboardFocusTraverser.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_KeyListener.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_KeyPress.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_ModifierKeys.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_SystemClipboard.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_TextEditorKeyMapper.h" \
	"../../../../modules/juce_gui_basics/keyboard/juce_TextInputTarget.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ComboBox.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ImageComponent.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_Label.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ListBox.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ProgressBar.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_Slider.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_TableHeaderComponent.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_TableListBox.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_TextEditor.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_Toolbar.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ToolbarItemComponent.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ToolbarItemFactory.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_ToolbarItemPalette.h" \
	"../../../../modules/juce_gui_basics/widgets/juce_TreeView.h" \
	"../../../../modules/juce_gui_basics/windows/juce_AlertWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_CallOutBox.h" \
	"../../../../modules/juce_gui_basics/windows/juce_ComponentPeer.h" \
	"../../../../modules/juce_gui_basics/windows/juce_DialogWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_DocumentWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_NativeMessageBox.h" \
	"../../../../modules/juce_gui_basics/windows/juce_ResizableWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_ThreadWithProgressWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_TooltipWindow.h" \
	"../../../../modules/juce_gui_basics/windows/juce_TopLevelWindow.h" \
	"../../../../modules/juce_gui_basics/menus/juce_MenuBarComponent.h" \
	"../../../../modules/juce_gui_basics/menus/juce_MenuBarModel.h" \
	"../../../../modules/juce_gui_basics/menus/juce_PopupMenu.h" \
	"../../../../modules/juce_gui_basics/layout/juce_AnimatedPosition.h" \
	"../../../../modules/juce_gui_basics/layout/juce_AnimatedPositionBehaviours.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ComponentAnimator.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ComponentBoundsConstrainer.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ComponentBuilder.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ComponentMovementWatcher.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ConcertinaPanel.h" \
	"../../../../modules/juce_gui_basics/layout/juce_GroupComponent.h" \
	"../../../../modules/juce_gui_basics/layout/juce_MultiDocumentPanel.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ResizableBorderComponent.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ResizableCornerComponent.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ResizableEdgeComponent.h" \
	"../../../../modules/juce_gui_basics/layout/juce_ScrollBar.h" \
	"../../../../modules/juce_gui_basics/layout/juce_StretchableLayoutManager.h" \
	"../../../../modules/juce_gui_basics/layout/juce_StretchableLayoutResizerBar.h" \
	"../../../../modules/juce_gui_basics/layout/juce_StretchableObjectResizer.h" \
	"../../../../modules/juce_gui_basics/layout/juce_TabbedButtonBar.h" \
	"../../../../modules/juce_gui_basics/layout/juce_TabbedComponent.h" \
	"../../../../modules/juce_gui_basics/layout/juce_Viewport.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_ArrowButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_Button.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_DrawableButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_HyperlinkButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_ImageButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_ShapeButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_TextButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_ToggleButton.h" \
	"../../../../modules/juce_gui_basics/buttons/juce_ToolbarButton.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_MarkerList.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativeCoordinate.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativeCoordinatePositioner.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativeParallelogram.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativePoint.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativePointPath.h" \
	"../../../../modules/juce_gui_basics/positioning/juce_RelativeRectangle.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_Drawable.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawableComposite.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawableImage.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawablePath.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawableRectangle.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawableShape.h" \
	"../../../../modules/juce_gui_basics/drawables/juce_DrawableText.h" \
	"../../../../modules/juce_gui_basics/properties/juce_BooleanPropertyComponent.h" \
	"../../../../modules/juce_gui_basics/properties/juce_ButtonPropertyComponent.h" \
	"../../../../modules/juce_gui_basics/properties/juce_ChoicePropertyComponent.h" \
	"../../../../modules/juce_gui_basics/properties/juce_PropertyComponent.h" \
	"../../../../modules/juce_gui_basics/properties/juce_PropertyPanel.h" \
	"../../../../modules/juce_gui_basics/properties/juce_SliderPropertyComponent.h" \
	"../../../../modules/juce_gui_basics/properties/juce_TextPropertyComponent.h" \
	"../../../../modules/juce_gui_basics/lookandfeel/juce_LookAndFeel.h" \
	"../../../../modules/juce_gui_basics/lookandfeel/juce_LookAndFeel_V1.h" \
	"../../../../modules/juce_gui_basics/lookandfeel/juce_LookAndFeel_V2.h" \
	"../../../../modules/juce_gui_basics/lookandfeel/juce_LookAndFeel_V3.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_DirectoryContentsDisplayComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_DirectoryContentsList.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileBrowserComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileBrowserListener.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileChooser.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileChooserDialogBox.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileListComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FilenameComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FilePreviewComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileSearchPathListComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_FileTreeComponent.h" \
	"../../../../modules/juce_gui_basics/filebrowser/juce_ImagePreviewComponent.h" \
	"../../../../modules/juce_gui_basics/commands/juce_ApplicationCommandID.h" \
	"../../../../modules/juce_gui_basics/commands/juce_ApplicationCommandInfo.h" \
	"../../../../modules/juce_gui_basics/commands/juce_ApplicationCommandManager.h" \
	"../../../../modules/juce_gui_basics/commands/juce_ApplicationCommandTarget.h" \
	"../../../../modules/juce_gui_basics/commands/juce_KeyPressMappingSet.h" \
	"../../../../modules/juce_gui_basics/misc/juce_BubbleComponent.h" \
	"../../../../modules/juce_gui_basics/misc/juce_DropShadower.h" \
	"../../../../modules/juce_gui_basics/application/juce_Application.h" \
	"../../../../modules/juce_gui_basics/native/juce_MultiTouchMapper.h" \
	"../../../../modules/juce_gui_basics/juce_gui_basics.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_CodeDocument.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_CodeEditorComponent.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_CodeTokeniser.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_CPlusPlusCodeTokeniser.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_CPlusPlusCodeTokeniserFunctions.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_LuaCodeTokeniser.h" \
	"../../../../modules/juce_gui_extra/code_editor/juce_XMLCodeTokeniser.h" \
	"../../../../modules/juce_gui_extra/documents/juce_FileBasedDocument.h" \
	"../../../../modules/juce_gui_extra/embedding/juce_ActiveXControlComponent.h" \
	"../../../../modules/juce_gui_extra/embedding/juce_NSViewComponent.h" \
	"../../../../modules/juce_gui_extra/embedding/juce_UIViewComponent.h" \
	"../../../../modules/juce_gui_extra/misc/juce_AnimatedAppComponent.h" \
	"../../../../modules/juce_gui_extra/misc/juce_AppleRemote.h" \
	"../../../../modules/juce_gui_extra/misc/juce_BubbleMessageComponent.h" \
	"../../../../modules/juce_gui_extra/misc/juce_ColourSelector.h" \
	"../../../../modules/juce_gui_extra/misc/juce_KeyMappingEditorComponent.h" \
	"../../../../modules/juce_gui_extra/misc/juce_LiveConstantEditor.h" \
	"../../../../modules/juce_gui_extra/misc/juce_PreferencesPanel.h" \
	"../../../../modules/juce_gui_extra/misc/juce_RecentlyOpenedFilesList.h" \
	"../../../../modules/juce_gui_extra/misc/juce_SplashScreen.h" \
	"../../../../modules/juce_gui_extra/misc/juce_SystemTrayIconComponent.h" \
	"../../../../modules/juce_gui_extra/misc/juce_WebBrowserComponent.h" \
	"../../../../modules/juce_gui_extra/native/juce_mac_CarbonViewWrapperComponent.h" \
	"../../../../modules/juce_gui_extra/juce_gui_extra.h" \
	"../../JuceLibraryCode/AppConfig.h" \
	"../../JuceLibraryCode/BinaryData.h" \
	"../../JuceLibraryCode/JuceHeader.h" \

