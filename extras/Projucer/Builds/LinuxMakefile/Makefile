# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

# build with "V=1" for verbose builds
ifeq ($(V), 1)
V_AT =
else
V_AT = @
endif

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

JUCE_ARCH_LABEL := $(shell uname -m)

ifeq ($(CONFIG),Debug)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Debug
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DDEBUG=1 -D_DEBUG=1 -DJUCER_LINUX_MAKE_6D53C8B4=1 -DJUCE_APP_VERSION=5.4.7 -DJUCE_APP_VERSION_HEX=0x50407 $(shell pkg-config --cflags x11 xinerama xext freetype2 webkit2gtk-4.0 gtk+-x11-3.0) -pthread -I../../JuceLibraryCode -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP := -DJucePlugin_Build_VST=0 -DJucePlugin_Build_VST3=0 -DJucePlugin_Build_AU=0 -DJucePlugin_Build_AUv3=0 -DJucePlugin_Build_RTAS=0 -DJucePlugin_Build_AAX=0 -DJucePlugin_Build_Standalone=0 -DJucePlugin_Build_Unity=0
  JUCE_TARGET_APP := Projucer

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++11 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell pkg-config --libs x11 xinerama xext freetype2 webkit2gtk-4.0 gtk+-x11-3.0) -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DNDEBUG=1 -DJUCER_LINUX_MAKE_6D53C8B4=1 -DJUCE_APP_VERSION=5.4.7 -DJUCE_APP_VERSION_HEX=0x50407 $(shell pkg-config --cflags x11 xinerama xext freetype2 webkit2gtk-4.0 gtk+-x11-3.0) -pthread -I../../JuceLibraryCode -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP := -DJucePlugin_Build_VST=0 -DJucePlugin_Build_VST3=0 -DJucePlugin_Build_AU=0 -DJucePlugin_Build_AUv3=0 -DJucePlugin_Build_RTAS=0 -DJucePlugin_Build_AAX=0 -DJucePlugin_Build_Standalone=0 -DJucePlugin_Build_Unity=0
  JUCE_TARGET_APP := Projucer

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -O3 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++11 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell pkg-config --libs x11 xinerama xext freetype2 webkit2gtk-4.0 gtk+-x11-3.0) -fvisibility=hidden -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS_APP := \
  $(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o \
  $(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o \
  $(JUCE_OBJDIR)/jucer_Main_f8488f5b.o \
  $(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o \
  $(JUCE_OBJDIR)/jucer_ProjucerAnalytics_5740f336.o \
  $(JUCE_OBJDIR)/jucer_DocumentEditorComponent_bc853a2f.o \
  $(JUCE_OBJDIR)/jucer_OpenDocumentManager_ba866622.o \
  $(JUCE_OBJDIR)/jucer_SourceCodeEditor_55965985.o \
  $(JUCE_OBJDIR)/jucer_ComponentTypeHandler_d7e70242.o \
  $(JUCE_OBJDIR)/jucer_ButtonDocument_b2cabdec.o \
  $(JUCE_OBJDIR)/jucer_ComponentDocument_1d8c1063.o \
  $(JUCE_OBJDIR)/jucer_ColouredElement_acb45c39.o \
  $(JUCE_OBJDIR)/jucer_PaintElement_db652060.o \
  $(JUCE_OBJDIR)/jucer_PaintElementGroup_256346c9.o \
  $(JUCE_OBJDIR)/jucer_PaintElementImage_49702ba5.o \
  $(JUCE_OBJDIR)/jucer_PaintElementPath_d1b280a5.o \
  $(JUCE_OBJDIR)/jucer_ComponentLayoutEditor_4fc44b8d.o \
  $(JUCE_OBJDIR)/jucer_ComponentOverlayComponent_5e028963.o \
  $(JUCE_OBJDIR)/jucer_EditingPanelBase_28115dee.o \
  $(JUCE_OBJDIR)/jucer_JucerDocumentEditor_89371a26.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutineEditor_43fd8744.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutinePanel_83779cd7.o \
  $(JUCE_OBJDIR)/jucer_ResourceEditorPanel_237eee2.o \
  $(JUCE_OBJDIR)/jucer_TestComponent_1a4179c4.o \
  $(JUCE_OBJDIR)/jucer_BinaryResources_e0f99b46.o \
  $(JUCE_OBJDIR)/jucer_ComponentLayout_6ea00129.o \
  $(JUCE_OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o \
  $(JUCE_OBJDIR)/jucer_JucerDocument_ff8afcc2.o \
  $(JUCE_OBJDIR)/jucer_ObjectTypes_4406f01c.o \
  $(JUCE_OBJDIR)/jucer_PaintRoutine_e1e891ee.o \
  $(JUCE_OBJDIR)/jucer_LicenseController_9abff0cf.o \
  $(JUCE_OBJDIR)/jucer_CompileEngineClient_aee8c99c.o \
  $(JUCE_OBJDIR)/jucer_CompileEngineServer_5d8914.o \
  $(JUCE_OBJDIR)/jucer_DownloadCompileEngineThread_19bb4bb3.o \
  $(JUCE_OBJDIR)/jucer_HeaderComponent_1ebf72ba.o \
  $(JUCE_OBJDIR)/jucer_Module_3f7666a5.o \
  $(JUCE_OBJDIR)/jucer_Project_c131864a.o \
  $(JUCE_OBJDIR)/jucer_ProjectExporter_cf377b25.o \
  $(JUCE_OBJDIR)/jucer_ProjectSaver_4276639b.o \
  $(JUCE_OBJDIR)/jucer_ResourceFile_74b61849.o \
  $(JUCE_OBJDIR)/jucer_AppearanceSettings_fa5d736e.o \
  $(JUCE_OBJDIR)/jucer_StoredSettings_c7168d2d.o \
  $(JUCE_OBJDIR)/jucer_CodeHelpers_1e797672.o \
  $(JUCE_OBJDIR)/jucer_FileHelpers_54f12f83.o \
  $(JUCE_OBJDIR)/jucer_MiscUtilities_31fc8dd8.o \
  $(JUCE_OBJDIR)/jucer_VersionInfo_46f3ed40.o \
  $(JUCE_OBJDIR)/jucer_PIPGenerator_fd3402c7.o \
  $(JUCE_OBJDIR)/jucer_Icons_d02d18f1.o \
  $(JUCE_OBJDIR)/jucer_JucerTreeViewBase_9b9f2ff0.o \
  $(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_3b20291d.o \
  $(JUCE_OBJDIR)/jucer_SlidingPanelComponent_4e7dc07e.o \
  $(JUCE_OBJDIR)/jucer_NewFileWizard_fac97f47.o \
  $(JUCE_OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o \
  $(JUCE_OBJDIR)/BinaryData_ce4232d4.o \
  $(JUCE_OBJDIR)/include_juce_analytics_f8e9fa94.o \
  $(JUCE_OBJDIR)/include_juce_core_f26d17db.o \
  $(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o \
  $(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o \
  $(JUCE_OBJDIR)/include_juce_events_fd7d695.o \
  $(JUCE_OBJDIR)/include_juce_graphics_f817e147.o \
  $(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o \
  $(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o \

.PHONY: clean all strip

all : $(JUCE_OUTDIR)/$(JUCE_TARGET_APP)

$(JUCE_OUTDIR)/$(JUCE_TARGET_APP) : $(OBJECTS_APP) $(RESOURCES)
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors x11 xinerama xext freetype2 webkit2gtk-4.0 gtk+-x11-3.0
	@echo Linking "Projucer - App"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(OBJECTS_APP) $(JUCE_LDFLAGS) $(JUCE_LDFLAGS_APP) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o: ../../Source/Application/jucer_AutoUpdater.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_AutoUpdater.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o: ../../Source/Application/jucer_CommandLine.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CommandLine.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Main_f8488f5b.o: ../../Source/Application/jucer_Main.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o: ../../Source/Application/jucer_MainWindow.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjucerAnalytics_5740f336.o: ../../Source/Application/jucer_ProjucerAnalytics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjucerAnalytics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DocumentEditorComponent_bc853a2f.o: ../../Source/CodeEditor/jucer_DocumentEditorComponent.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_OpenDocumentManager_ba866622.o: ../../Source/CodeEditor/jucer_OpenDocumentManager.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SourceCodeEditor_55965985.o: ../../Source/CodeEditor/jucer_SourceCodeEditor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentTypeHandler_d7e70242.o: ../../Source/ComponentEditor/Components/jucer_ComponentTypeHandler.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentTypeHandler.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ButtonDocument_b2cabdec.o: ../../Source/ComponentEditor/Documents/jucer_ButtonDocument.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ButtonDocument.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentDocument_1d8c1063.o: ../../Source/ComponentEditor/Documents/jucer_ComponentDocument.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ColouredElement_acb45c39.o: ../../Source/ComponentEditor/PaintElements/jucer_ColouredElement.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ColouredElement.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElement_db652060.o: ../../Source/ComponentEditor/PaintElements/jucer_PaintElement.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElement.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElementGroup_256346c9.o: ../../Source/ComponentEditor/PaintElements/jucer_PaintElementGroup.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElementGroup.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElementImage_49702ba5.o: ../../Source/ComponentEditor/PaintElements/jucer_PaintElementImage.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElementImage.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintElementPath_d1b280a5.o: ../../Source/ComponentEditor/PaintElements/jucer_PaintElementPath.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintElementPath.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentLayoutEditor_4fc44b8d.o: ../../Source/ComponentEditor/UI/jucer_ComponentLayoutEditor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentLayoutEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentOverlayComponent_5e028963.o: ../../Source/ComponentEditor/UI/jucer_ComponentOverlayComponent.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentOverlayComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_EditingPanelBase_28115dee.o: ../../Source/ComponentEditor/UI/jucer_EditingPanelBase.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_EditingPanelBase.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerDocumentEditor_89371a26.o: ../../Source/ComponentEditor/UI/jucer_JucerDocumentEditor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerDocumentEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutineEditor_43fd8744.o: ../../Source/ComponentEditor/UI/jucer_PaintRoutineEditor.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutineEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutinePanel_83779cd7.o: ../../Source/ComponentEditor/UI/jucer_PaintRoutinePanel.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutinePanel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ResourceEditorPanel_237eee2.o: ../../Source/ComponentEditor/UI/jucer_ResourceEditorPanel.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ResourceEditorPanel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_TestComponent_1a4179c4.o: ../../Source/ComponentEditor/UI/jucer_TestComponent.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_TestComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_BinaryResources_e0f99b46.o: ../../Source/ComponentEditor/jucer_BinaryResources.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_BinaryResources.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ComponentLayout_6ea00129.o: ../../Source/ComponentEditor/jucer_ComponentLayout.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ComponentLayout.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o: ../../Source/ComponentEditor/jucer_GeneratedCode.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_GeneratedCode.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerDocument_ff8afcc2.o: ../../Source/ComponentEditor/jucer_JucerDocument.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerDocument.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ObjectTypes_4406f01c.o: ../../Source/ComponentEditor/jucer_ObjectTypes.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ObjectTypes.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PaintRoutine_e1e891ee.o: ../../Source/ComponentEditor/jucer_PaintRoutine.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PaintRoutine.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_LicenseController_9abff0cf.o: ../../Source/Licenses/jucer_LicenseController.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_LicenseController.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CompileEngineClient_aee8c99c.o: ../../Source/LiveBuildEngine/jucer_CompileEngineClient.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CompileEngineClient.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CompileEngineServer_5d8914.o: ../../Source/LiveBuildEngine/jucer_CompileEngineServer.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CompileEngineServer.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DownloadCompileEngineThread_19bb4bb3.o: ../../Source/LiveBuildEngine/jucer_DownloadCompileEngineThread.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_DownloadCompileEngineThread.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_HeaderComponent_1ebf72ba.o: ../../Source/Project/UI/jucer_HeaderComponent.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_HeaderComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Module_3f7666a5.o: ../../Source/Project/jucer_Module.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Module.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Project_c131864a.o: ../../Source/Project/jucer_Project.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectExporter_cf377b25.o: ../../Source/ProjectSaving/jucer_ProjectExporter.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectSaver_4276639b.o: ../../Source/ProjectSaving/jucer_ProjectSaver.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjectSaver.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ResourceFile_74b61849.o: ../../Source/ProjectSaving/jucer_ResourceFile.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_AppearanceSettings_fa5d736e.o: ../../Source/Settings/jucer_AppearanceSettings.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_AppearanceSettings.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_StoredSettings_c7168d2d.o: ../../Source/Settings/jucer_StoredSettings.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CodeHelpers_1e797672.o: ../../Source/Utility/Helpers/jucer_CodeHelpers.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_FileHelpers_54f12f83.o: ../../Source/Utility/Helpers/jucer_FileHelpers.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MiscUtilities_31fc8dd8.o: ../../Source/Utility/Helpers/jucer_MiscUtilities.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_VersionInfo_46f3ed40.o: ../../Source/Utility/Helpers/jucer_VersionInfo.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_VersionInfo.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PIPGenerator_fd3402c7.o: ../../Source/Utility/PIPs/jucer_PIPGenerator.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_PIPGenerator.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Icons_d02d18f1.o: ../../Source/Utility/UI/jucer_Icons.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_Icons.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerTreeViewBase_9b9f2ff0.o: ../../Source/Utility/UI/jucer_JucerTreeViewBase.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_3b20291d.o: ../../Source/Utility/UI/jucer_ProjucerLookAndFeel.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_ProjucerLookAndFeel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SlidingPanelComponent_4e7dc07e.o: ../../Source/Utility/UI/jucer_SlidingPanelComponent.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_SlidingPanelComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_NewFileWizard_fac97f47.o: ../../Source/Wizards/jucer_NewFileWizard.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o: ../../Source/Wizards/jucer_NewProjectWizardClasses.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling jucer_NewProjectWizardClasses.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling BinaryData.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_analytics_f8e9fa94.o: ../../JuceLibraryCode/include_juce_analytics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_analytics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_core_f26d17db.o: ../../JuceLibraryCode/include_juce_core.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_core.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o: ../../JuceLibraryCode/include_juce_cryptography.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_cryptography.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o: ../../JuceLibraryCode/include_juce_data_structures.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_data_structures.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_events_fd7d695.o: ../../JuceLibraryCode/include_juce_events.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_events.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_f817e147.o: ../../JuceLibraryCode/include_juce_graphics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_graphics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o: ../../JuceLibraryCode/include_juce_gui_basics.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o: ../../JuceLibraryCode/include_juce_gui_extra.cpp
	-$(V_AT)mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling include_juce_gui_extra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

clean:
	@echo Cleaning Projucer
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping Projucer
	-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)

-include $(OBJECTS_APP:%.o=%.d)
