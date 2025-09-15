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

ifndef PKG_CONFIG
  PKG_CONFIG=pkg-config
endif

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

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DDEBUG=1" "-D_DEBUG=1" "-DJUCE_PROJUCER_VERSION=0x8000a" "-DJUCE_MODULE_AVAILABLE_juce_build_tools=1" "-DJUCE_MODULE_AVAILABLE_juce_core=1" "-DJUCE_MODULE_AVAILABLE_juce_cryptography=1" "-DJUCE_MODULE_AVAILABLE_juce_data_structures=1" "-DJUCE_MODULE_AVAILABLE_juce_events=1" "-DJUCE_MODULE_AVAILABLE_juce_graphics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_extra=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_LOG_ASSERTIONS=1" "-DJUCE_USE_CURL=1" "-DJUCE_LOAD_CURL_SYMBOLS_LAZILY=1" "-DJUCE_ALLOW_STATIC_NULL_VARIABLES=0" "-DJUCE_STRICT_REFCOUNTEDPOINTER=1" "-DJUCE_WEB_BROWSER=0" "-DJUCE_STANDALONE_APPLICATION=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=8.0.10" "-DJUCE_APP_VERSION_HEX=0x8000a" $(shell $(PKG_CONFIG) --cflags freetype2 fontconfig) -pthread -I../../JuceLibraryCode -I../../../Build -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0" "-DJucePlugin_Build_LV2=0"
  JUCE_TARGET_APP := Projucer

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell $(PKG_CONFIG) --libs freetype2 fontconfig) -fvisibility=hidden -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) "-DLINUX=1" "-DNDEBUG=1" "-DJUCE_PROJUCER_VERSION=0x8000a" "-DJUCE_MODULE_AVAILABLE_juce_build_tools=1" "-DJUCE_MODULE_AVAILABLE_juce_core=1" "-DJUCE_MODULE_AVAILABLE_juce_cryptography=1" "-DJUCE_MODULE_AVAILABLE_juce_data_structures=1" "-DJUCE_MODULE_AVAILABLE_juce_events=1" "-DJUCE_MODULE_AVAILABLE_juce_graphics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_basics=1" "-DJUCE_MODULE_AVAILABLE_juce_gui_extra=1" "-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1" "-DJUCE_LOG_ASSERTIONS=1" "-DJUCE_USE_CURL=1" "-DJUCE_LOAD_CURL_SYMBOLS_LAZILY=1" "-DJUCE_ALLOW_STATIC_NULL_VARIABLES=0" "-DJUCE_STRICT_REFCOUNTEDPOINTER=1" "-DJUCE_WEB_BROWSER=0" "-DJUCE_STANDALONE_APPLICATION=1" "-DJUCER_LINUX_MAKE_6D53C8B4=1" "-DJUCE_APP_VERSION=8.0.10" "-DJUCE_APP_VERSION_HEX=0x8000a" $(shell $(PKG_CONFIG) --cflags freetype2 fontconfig) -pthread -I../../JuceLibraryCode -I../../../Build -I../../../../modules $(CPPFLAGS)
  JUCE_CPPFLAGS_APP :=  "-DJucePlugin_Build_VST=0" "-DJucePlugin_Build_VST3=0" "-DJucePlugin_Build_AU=0" "-DJucePlugin_Build_AUv3=0" "-DJucePlugin_Build_AAX=0" "-DJucePlugin_Build_Standalone=0" "-DJucePlugin_Build_Unity=0" "-DJucePlugin_Build_LV2=0"
  JUCE_TARGET_APP := Projucer

  JUCE_CFLAGS += $(JUCE_CPPFLAGS) $(TARGET_ARCH) -O3 $(CFLAGS)
  JUCE_CXXFLAGS += $(JUCE_CFLAGS) -std=c++17 $(CXXFLAGS)
  JUCE_LDFLAGS += $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) $(shell $(PKG_CONFIG) --libs freetype2 fontconfig) -fvisibility=hidden -lrt -ldl -lpthread $(LDFLAGS)

  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(JUCE_OBJDIR)
endif

OBJECTS_APP := \
  $(JUCE_OBJDIR)/jucer_NewProjectWizard_7817a7e4.o \
  $(JUCE_OBJDIR)/jucer_StartPageComponent_de6b22ec.o \
  $(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o \
  $(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o \
  $(JUCE_OBJDIR)/jucer_Main_f8488f5b.o \
  $(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o \
  $(JUCE_OBJDIR)/jucer_DocumentEditorComponent_bc853a2f.o \
  $(JUCE_OBJDIR)/jucer_OpenDocumentManager_ba866622.o \
  $(JUCE_OBJDIR)/jucer_SourceCodeEditor_55965985.o \
  $(JUCE_OBJDIR)/jucer_Modules_e20cbd10.o \
  $(JUCE_OBJDIR)/jucer_HeaderComponent_1ebf72ba.o \
  $(JUCE_OBJDIR)/jucer_Project_c131864a.o \
  $(JUCE_OBJDIR)/jucer_ProjectExporter_cf377b25.o \
  $(JUCE_OBJDIR)/jucer_ProjectSaver_4276639b.o \
  $(JUCE_OBJDIR)/jucer_ResourceFile_74b61849.o \
  $(JUCE_OBJDIR)/jucer_AppearanceSettings_fa5d736e.o \
  $(JUCE_OBJDIR)/jucer_StoredSettings_c7168d2d.o \
  $(JUCE_OBJDIR)/jucer_CodeHelpers_1e797672.o \
  $(JUCE_OBJDIR)/jucer_FileHelpers_54f12f83.o \
  $(JUCE_OBJDIR)/jucer_MiscUtilities_31fc8dd8.o \
  $(JUCE_OBJDIR)/jucer_NewFileWizard_b233dda1.o \
  $(JUCE_OBJDIR)/jucer_VersionInfo_46f3ed40.o \
  $(JUCE_OBJDIR)/jucer_PIPGenerator_fd3402c7.o \
  $(JUCE_OBJDIR)/jucer_Icons_d02d18f1.o \
  $(JUCE_OBJDIR)/jucer_JucerTreeViewBase_9b9f2ff0.o \
  $(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_3b20291d.o \
  $(JUCE_OBJDIR)/jucer_SlidingPanelComponent_4e7dc07e.o \
  $(JUCE_OBJDIR)/BinaryData_ce4232d4.o \
  $(JUCE_OBJDIR)/include_juce_build_tools_f5069398.o \
  $(JUCE_OBJDIR)/include_juce_core_f26d17db.o \
  $(JUCE_OBJDIR)/include_juce_core_CompilationTime_9257742c.o \
  $(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o \
  $(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o \
  $(JUCE_OBJDIR)/include_juce_events_fd7d695.o \
  $(JUCE_OBJDIR)/include_juce_graphics_f817e147.o \
  $(JUCE_OBJDIR)/include_juce_graphics_Harfbuzz_60c52ba2.o \
  $(JUCE_OBJDIR)/include_juce_graphics_Sheenbidi_c310974d.o \
  $(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o \
  $(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o \

.PHONY: clean all strip

all : $(JUCE_OUTDIR)/$(JUCE_TARGET_APP)

$(JUCE_OUTDIR)/$(JUCE_TARGET_APP) : $(OBJECTS_APP) $(JUCE_OBJDIR)/execinfo.cmd $(RESOURCES)
	@command -v $(PKG_CONFIG) >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@$(PKG_CONFIG) --print-errors freetype2 fontconfig
	@echo Linking "Projucer - App"
	-$(V_AT)mkdir -p $(JUCE_BINDIR)
	-$(V_AT)mkdir -p $(JUCE_LIBDIR)
	-$(V_AT)mkdir -p $(JUCE_OUTDIR)
	$(V_AT)$(CXX) -o $(JUCE_OUTDIR)/$(JUCE_TARGET_APP) $(OBJECTS_APP) $(JUCE_LDFLAGS) $(shell cat $(JUCE_OBJDIR)/execinfo.cmd) $(JUCE_LDFLAGS_APP) $(RESOURCES) $(TARGET_ARCH)

$(JUCE_OBJDIR)/jucer_NewProjectWizard_7817a7e4.o: ../../Source/Application/StartPage/jucer_NewProjectWizard.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_NewProjectWizard.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_StartPageComponent_de6b22ec.o: ../../Source/Application/StartPage/jucer_StartPageComponent.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_StartPageComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_AutoUpdater_ca658dc2.o: ../../Source/Application/jucer_AutoUpdater.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_AutoUpdater.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CommandLine_f35de107.o: ../../Source/Application/jucer_CommandLine.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_CommandLine.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Main_f8488f5b.o: ../../Source/Application/jucer_Main.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_Main.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MainWindow_1e163aeb.o: ../../Source/Application/jucer_MainWindow.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_MainWindow.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_DocumentEditorComponent_bc853a2f.o: ../../Source/CodeEditor/jucer_DocumentEditorComponent.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_OpenDocumentManager_ba866622.o: ../../Source/CodeEditor/jucer_OpenDocumentManager.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SourceCodeEditor_55965985.o: ../../Source/CodeEditor/jucer_SourceCodeEditor.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Modules_e20cbd10.o: ../../Source/Project/Modules/jucer_Modules.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_Modules.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_HeaderComponent_1ebf72ba.o: ../../Source/Project/UI/jucer_HeaderComponent.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_HeaderComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Project_c131864a.o: ../../Source/Project/jucer_Project.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_Project.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectExporter_cf377b25.o: ../../Source/ProjectSaving/jucer_ProjectExporter.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_ProjectExporter.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjectSaver_4276639b.o: ../../Source/ProjectSaving/jucer_ProjectSaver.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_ProjectSaver.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ResourceFile_74b61849.o: ../../Source/ProjectSaving/jucer_ResourceFile.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_ResourceFile.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_AppearanceSettings_fa5d736e.o: ../../Source/Settings/jucer_AppearanceSettings.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_AppearanceSettings.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_StoredSettings_c7168d2d.o: ../../Source/Settings/jucer_StoredSettings.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_StoredSettings.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_CodeHelpers_1e797672.o: ../../Source/Utility/Helpers/jucer_CodeHelpers.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_CodeHelpers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_FileHelpers_54f12f83.o: ../../Source/Utility/Helpers/jucer_FileHelpers.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_FileHelpers.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_MiscUtilities_31fc8dd8.o: ../../Source/Utility/Helpers/jucer_MiscUtilities.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_MiscUtilities.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_NewFileWizard_b233dda1.o: ../../Source/Utility/Helpers/jucer_NewFileWizard.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_NewFileWizard.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_VersionInfo_46f3ed40.o: ../../Source/Utility/Helpers/jucer_VersionInfo.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_VersionInfo.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_PIPGenerator_fd3402c7.o: ../../Source/Utility/PIPs/jucer_PIPGenerator.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_PIPGenerator.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_Icons_d02d18f1.o: ../../Source/Utility/UI/jucer_Icons.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_Icons.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_JucerTreeViewBase_9b9f2ff0.o: ../../Source/Utility/UI/jucer_JucerTreeViewBase.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_ProjucerLookAndFeel_3b20291d.o: ../../Source/Utility/UI/jucer_ProjucerLookAndFeel.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_ProjucerLookAndFeel.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/jucer_SlidingPanelComponent_4e7dc07e.o: ../../Source/Utility/UI/jucer_SlidingPanelComponent.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling jucer_SlidingPanelComponent.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling BinaryData.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_build_tools_f5069398.o: ../../JuceLibraryCode/include_juce_build_tools.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_build_tools.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_core_f26d17db.o: ../../JuceLibraryCode/include_juce_core.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_core.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_core_CompilationTime_9257742c.o: ../../JuceLibraryCode/include_juce_core_CompilationTime.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_core_CompilationTime.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_cryptography_8cb807a8.o: ../../JuceLibraryCode/include_juce_cryptography.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_cryptography.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_data_structures_7471b1e3.o: ../../JuceLibraryCode/include_juce_data_structures.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_data_structures.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_events_fd7d695.o: ../../JuceLibraryCode/include_juce_events.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_events.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_f817e147.o: ../../JuceLibraryCode/include_juce_graphics.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_graphics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_Harfbuzz_60c52ba2.o: ../../JuceLibraryCode/include_juce_graphics_Harfbuzz.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_graphics_Harfbuzz.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_graphics_Sheenbidi_c310974d.o: ../../JuceLibraryCode/include_juce_graphics_Sheenbidi.c
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_graphics_Sheenbidi.c"
	$(V_AT)$(CC) $(JUCE_CFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_basics_e3f79785.o: ../../JuceLibraryCode/include_juce_gui_basics.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_gui_basics.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/include_juce_gui_extra_6dee1c1a.o: ../../JuceLibraryCode/include_juce_gui_extra.cpp
	-$(V_AT)mkdir -p $(@D)
	@echo "Compiling include_juce_gui_extra.cpp"
	$(V_AT)$(CXX) $(JUCE_CXXFLAGS) $(JUCE_CPPFLAGS_APP) $(JUCE_CFLAGS_APP) -o "$@" -c "$<"

$(JUCE_OBJDIR)/execinfo.cmd:
	-$(V_AT)mkdir -p $(@D)
	-@if [ -z "$(V_AT)" ]; then echo "Checking if we need to link libexecinfo"; fi
	$(V_AT)printf "int main() { return 0; }" | $(CXX) -x c++ -o $(@D)/execinfo.x -lexecinfo - >/dev/null 2>&1 && printf -- "-lexecinfo" > "$@" || touch "$@"

$(JUCE_OBJDIR)/cxxfs.cmd:
	-$(V_AT)mkdir -p $(@D)
	-@if [ -z "$(V_AT)" ]; then echo "Checking if we need to link stdc++fs"; fi
	$(V_AT)printf "int main() { return 0; }" | $(CXX) -x c++ -o $(@D)/cxxfs.x -lstdc++fs - >/dev/null 2>&1 && printf -- "-lstdc++fs" > "$@" || touch "$@"

clean:
	@echo Cleaning Projucer
	$(V_AT)$(CLEANCMD)

strip:
	@echo Stripping Projucer
	-$(V_AT)$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(JUCE_TARGET_APP)

-include $(OBJECTS_APP:%.o=%.d)
