# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef STRIP
  STRIP=strip
endif

ifndef AR
  AR=ar
endif

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Debug
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DDEBUG=1 -D_DEBUG=1 -DJUCER_LINUX_MAKE_6D53C8B4=1 -DJUCE_APP_VERSION=4.3.0 -DJUCE_APP_VERSION_HEX=0x40300 $(shell pkg-config --cflags freetype2 libcurl x11 xext xinerama) -pthread -I../../JuceLibraryCode -I../../../../modules
  JUCE_CFLAGS += $(CFLAGS) $(JUCE_CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0 -std=c++11
  JUCE_CXXFLAGS += $(CXXFLAGS) $(JUCE_CFLAGS) -std=c++11
  JUCE_LDFLAGS += $(LDFLAGS) $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell pkg-config --libs freetype2 libcurl x11 xext xinerama) -ldl -lpthread -lrt 

  TARGET := Projucer
  BLDCMD = $(CXX) -o $(JUCE_OUTDIR)/$(TARGET) $(OBJECTS) $(JUCE_LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DNDEBUG=1 -DJUCER_LINUX_MAKE_6D53C8B4=1 -DJUCE_APP_VERSION=4.3.0 -DJUCE_APP_VERSION_HEX=0x40300 $(shell pkg-config --cflags freetype2 libcurl x11 xext xinerama) -pthread -I../../JuceLibraryCode -I../../../../modules
  JUCE_CFLAGS += $(CFLAGS) $(JUCE_CPPFLAGS) $(TARGET_ARCH) -O3 -std=c++11
  JUCE_CXXFLAGS += $(CXXFLAGS) $(JUCE_CFLAGS) -std=c++11
  JUCE_LDFLAGS += $(LDFLAGS) $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -fvisibility=hidden $(shell pkg-config --libs freetype2 libcurl x11 xext xinerama) -ldl -lpthread -lrt 

  TARGET := Projucer
  BLDCMD = $(CXX) -o $(JUCE_OUTDIR)/$(TARGET) $(OBJECTS) $(JUCE_LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS := \
  $(JUCE_OBJDIR)/jucer_AppearanceSettings_788d9889.o \
  $(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o \
  $(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o \
  $(JUCE_OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o \
  $(JUCE_OBJDIR)/jucer_DownloadCompileEngineThread_8a38703f.o \
  $(JUCE_OBJDIR)/jucer_GlobalPreferences_b0f1bd3d.o \
  $(JUCE_OBJDIR)/jucer_Main_f8488f5b.o \
  $(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o \
  $(JUCE_OBJDIR)/jucer_OpenDocumentManager_4c72d210.o \
  $(JUCE_OBJDIR)/projucer_CompileEngineClient_542e6b2d.o \
  $(JUCE_OBJDIR)/projucer_CompileEngineServer_a5a32aa5.o \
  $(JUCE_OBJDIR)/jucer_SourceCodeEditor_461f5487.o \
  $(JUCE_OBJDIR)/jucer_ComponentTypeHandler_6bec6262.o \
  $(JUCE_OBJDIR)/jucer_ButtonDocument_56c341cc.o \
  $(JUCE_OBJDIR)/jucer_ComponentDocument_92868083.o \
  $(JUCE_OBJDIR)/jucer_ColouredElement_70cbc839.o \
  $(JUCE_OBJDIR)/jucer_PaintElement_653460.o \
  $(JUCE_OBJDIR)/jucer_PaintElementPath_908894a5.o \
  $(JUCE_OBJDIR)/jucer_ComponentLayoutEditor_aeb44f8d.o \
  $(JUCE_OBJDIR)/jucer_ComponentOverlayComponent_fd508d63.o \
  $(JUCE_OBJDIR)/jucer_EditingPanelBase_6c2ed9ee.o \
  $(JUCE_OBJDIR)/jucer_JucerDocumentEditor_2c581e26.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutineEditor_f6ac0344.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutinePanel_c309a0d7.o \
  $(JUCE_OBJDIR)/jucer_ResourceEditorPanel_a558f2e2.o \
  $(JUCE_OBJDIR)/jucer_TestComponent_db757dc4.o \
  $(JUCE_OBJDIR)/jucer_BinaryResources_e0f99b46.o \
  $(JUCE_OBJDIR)/jucer_ComponentLayout_6ea00129.o \
  $(JUCE_OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o \
  $(JUCE_OBJDIR)/jucer_JucerDocument_ff8afcc2.o \
  $(JUCE_OBJDIR)/jucer_ObjectTypes_4406f01c.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutine_e1e891ee.o \
  $(JUCE_OBJDIR)/jucer_DependencyPathPropertyComponent_4d137229.o \
  $(JUCE_OBJDIR)/jucer_Module_3f7666a5.o \
  $(JUCE_OBJDIR)/jucer_Project_c131864a.o \
  $(JUCE_OBJDIR)/jucer_ProjectExporter_eefe2e5b.o \
  $(JUCE_OBJDIR)/jucer_ProjectSaver_38b16c25.o \
  $(JUCE_OBJDIR)/jucer_ResourceFile_6af120d3.o \
  $(JUCE_OBJDIR)/jucer_CodeHelpers_c317179c.o \
  $(JUCE_OBJDIR)/jucer_FileHelpers_f98ed0ad.o \
  $(JUCE_OBJDIR)/jucer_Icons_9db2f51e.o \
  $(JUCE_OBJDIR)/jucer_JucerTreeViewBase_d043309d.o \
  $(JUCE_OBJDIR)/jucer_MiscUtilities_25b68c82.o \
  $(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_d6c6b28a.o \
  $(JUCE_OBJDIR)/jucer_SlidingPanelComponent_9aa7a2ab.o \
  $(JUCE_OBJDIR)/jucer_StoredSettings_26078d2c.o \
  $(JUCE_OBJDIR)/jucer_NewFileWizard_fac97f47.o \
  $(JUCE_OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o \
  $(JUCE_OBJDIR)/BinaryData_ce4232d4.o \
  $(JUCE_OBJDIR)/juce_core_75b14332.o \
  $(JUCE_OBJDIR)/juce_cryptography_6de2ebff.o \
  $(JUCE_OBJDIR)/juce_data_structures_72d3da2c.o \
  $(JUCE_OBJDIR)/juce_events_d2be882c.o \
  $(JUCE_OBJDIR)/juce_graphics_9c18891e.o \
  $(JUCE_OBJDIR)/juce_gui_basics_8a6da59c.o \
  $(JUCE_OBJDIR)/juce_gui_extra_4a026f23.o \

.PHONY: clean

$(JUCE_OUTDIR)/$(TARGET): check-pkg-config $(OBJECTS) $(RESOURCES)
	@echo Linking Projucer
	-@mkdir -p $(JUCE_BINDIR)
	-@mkdir -p $(JUCE_LIBDIR)
	-@mkdir -p $(JUCE_OUTDIR)
	@$(BLDCMD)

check-pkg-config:
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors freetype2 libcurl x11 xext xinerama

clean:
	@echo Cleaning Projucer
	@$(CLEANCMD)

strip:
	@echo Stripping Projucer
	-@$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)

$(JUCE_OBJDIR)/jucer_AppearanceSettings_788d9889.o: ../../Source/Application/jucer_AppearanceSettings.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_AppearanceSettings.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o: ../../Source/Application/jucer_AutoUpdater.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_AutoUpdater.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o: ../../Source/Application/jucer_CommandLine.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CommandLine.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o: ../../Source/Application/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DownloadCompileEngineThread_8a38703f.o: ../../Source/Application/jucer_DownloadCompileEngineThread.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_DownloadCompileEngineThread.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_GlobalPreferences_b0f1bd3d.o: ../../Source/Application/jucer_GlobalPreferences.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_GlobalPreferences.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Main_f8488f5b.o: ../../Source/Application/jucer_Main.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o: ../../Source/Application/jucer_MainWindow.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_OpenDocumentManager_4c72d210.o: ../../Source/Application/jucer_OpenDocumentManager.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/projucer_CompileEngineClient_542e6b2d.o: ../../Source/LiveBuildEngine/projucer_CompileEngineClient.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling projucer_CompileEngineClient.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/projucer_CompileEngineServer_a5a32aa5.o: ../../Source/LiveBuildEngine/projucer_CompileEngineServer.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling projucer_CompileEngineServer.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SourceCodeEditor_461f5487.o: ../../Source/Code\ Editor/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentTypeHandler_6bec6262.o: ../../Source/ComponentEditor/components/jucer_ComponentTypeHandler.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentTypeHandler.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ButtonDocument_56c341cc.o: ../../Source/ComponentEditor/documents/jucer_ButtonDocument.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ButtonDocument.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentDocument_92868083.o: ../../Source/ComponentEditor/documents/jucer_ComponentDocument.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ColouredElement_70cbc839.o: ../../Source/ComponentEditor/paintelements/jucer_ColouredElement.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ColouredElement.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElement_653460.o: ../../Source/ComponentEditor/paintelements/jucer_PaintElement.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElement.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElementPath_908894a5.o: ../../Source/ComponentEditor/paintelements/jucer_PaintElementPath.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElementPath.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentLayoutEditor_aeb44f8d.o: ../../Source/ComponentEditor/ui/jucer_ComponentLayoutEditor.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentLayoutEditor.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentOverlayComponent_fd508d63.o: ../../Source/ComponentEditor/ui/jucer_ComponentOverlayComponent.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentOverlayComponent.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_EditingPanelBase_6c2ed9ee.o: ../../Source/ComponentEditor/ui/jucer_EditingPanelBase.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_EditingPanelBase.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerDocumentEditor_2c581e26.o: ../../Source/ComponentEditor/ui/jucer_JucerDocumentEditor.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerDocumentEditor.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutineEditor_f6ac0344.o: ../../Source/ComponentEditor/ui/jucer_PaintRoutineEditor.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutineEditor.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutinePanel_c309a0d7.o: ../../Source/ComponentEditor/ui/jucer_PaintRoutinePanel.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutinePanel.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ResourceEditorPanel_a558f2e2.o: ../../Source/ComponentEditor/ui/jucer_ResourceEditorPanel.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ResourceEditorPanel.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_TestComponent_db757dc4.o: ../../Source/ComponentEditor/ui/jucer_TestComponent.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_TestComponent.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_BinaryResources_e0f99b46.o: ../../Source/ComponentEditor/jucer_BinaryResources.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_BinaryResources.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentLayout_6ea00129.o: ../../Source/ComponentEditor/jucer_ComponentLayout.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentLayout.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o: ../../Source/ComponentEditor/jucer_GeneratedCode.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_GeneratedCode.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerDocument_ff8afcc2.o: ../../Source/ComponentEditor/jucer_JucerDocument.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerDocument.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ObjectTypes_4406f01c.o: ../../Source/ComponentEditor/jucer_ObjectTypes.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ObjectTypes.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutine_e1e891ee.o: ../../Source/ComponentEditor/jucer_PaintRoutine.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutine.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DependencyPathPropertyComponent_4d137229.o: ../../Source/Project/jucer_DependencyPathPropertyComponent.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_DependencyPathPropertyComponent.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Module_3f7666a5.o: ../../Source/Project/jucer_Module.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Module.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Project_c131864a.o: ../../Source/Project/jucer_Project.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectExporter_eefe2e5b.o: ../../Source/Project\ Saving/jucer_ProjectExporter.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectSaver_38b16c25.o: ../../Source/Project\ Saving/jucer_ProjectSaver.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjectSaver.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ResourceFile_6af120d3.o: ../../Source/Project\ Saving/jucer_ResourceFile.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CodeHelpers_c317179c.o: ../../Source/Utility/jucer_CodeHelpers.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_FileHelpers_f98ed0ad.o: ../../Source/Utility/jucer_FileHelpers.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Icons_9db2f51e.o: ../../Source/Utility/jucer_Icons.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Icons.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerTreeViewBase_d043309d.o: ../../Source/Utility/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MiscUtilities_25b68c82.o: ../../Source/Utility/jucer_MiscUtilities.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_d6c6b28a.o: ../../Source/Utility/jucer_ProjucerLookAndFeel.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjucerLookAndFeel.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SlidingPanelComponent_9aa7a2ab.o: ../../Source/Utility/jucer_SlidingPanelComponent.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_SlidingPanelComponent.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_StoredSettings_26078d2c.o: ../../Source/Utility/jucer_StoredSettings.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_NewFileWizard_fac97f47.o: ../../Source/Wizards/jucer_NewFileWizard.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o: ../../Source/Wizards/jucer_NewProjectWizardClasses.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_NewProjectWizardClasses.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_core_75b14332.o: ../../JuceLibraryCode/juce_core.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_cryptography_6de2ebff.o: ../../JuceLibraryCode/juce_cryptography.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_cryptography.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_data_structures_72d3da2c.o: ../../JuceLibraryCode/juce_data_structures.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_events_d2be882c.o: ../../JuceLibraryCode/juce_events.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_graphics_9c18891e.o: ../../JuceLibraryCode/juce_graphics.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_gui_basics_8a6da59c.o: ../../JuceLibraryCode/juce_gui_basics.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_gui_extra_4a026f23.o: ../../JuceLibraryCode/juce_gui_extra.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
