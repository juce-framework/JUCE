# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(TARGET_ARCH),)
  TARGET_ARCH := -march=native
endif

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  TARGET := Introjucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  TARGET := Introjucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/jucer_AppearanceSettings_788d9889.o \
  $(OBJDIR)/jucer_CommandLine_f35de107.o \
  $(OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o \
  $(OBJDIR)/jucer_JuceUpdater_cf7865c4.o \
  $(OBJDIR)/jucer_Main_f8488f5b.o \
  $(OBJDIR)/jucer_MainWindow_1e163aeb.o \
  $(OBJDIR)/jucer_OpenDocumentManager_4c72d210.o \
  $(OBJDIR)/jucer_ConfigPage_99b07caa.o \
  $(OBJDIR)/jucer_Module_3f7666a5.o \
  $(OBJDIR)/jucer_NewFileWizard_b8a19ef8.o \
  $(OBJDIR)/jucer_NewProjectWizard_7a15bf5d.o \
  $(OBJDIR)/jucer_Project_c131864a.o \
  $(OBJDIR)/jucer_ProjectContentComponent_60de0eee.o \
  $(OBJDIR)/jucer_ProjectTreeViewBase_63c2bdbe.o \
  $(OBJDIR)/jucer_ProjectType_1bc27a24.o \
  $(OBJDIR)/jucer_TreeViewTypes_4a765287.o \
  $(OBJDIR)/jucer_ProjectExporter_eefe2e5b.o \
  $(OBJDIR)/jucer_ResourceFile_6af120d3.o \
  $(OBJDIR)/jucer_CodeHelpers_c317179c.o \
  $(OBJDIR)/jucer_FileHelpers_f98ed0ad.o \
  $(OBJDIR)/jucer_JucerTreeViewBase_d043309d.o \
  $(OBJDIR)/jucer_MiscUtilities_25b68c82.o \
  $(OBJDIR)/jucer_StoredSettings_26078d2c.o \
  $(OBJDIR)/jucer_SourceCodeEditor_461f5487.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_core_1ee54a40.o \
  $(OBJDIR)/juce_cryptography_3b1f489a.o \
  $(OBJDIR)/juce_data_structures_84790dfc.o \
  $(OBJDIR)/juce_events_584896b4.o \
  $(OBJDIR)/juce_graphics_f9afc18.o \
  $(OBJDIR)/juce_gui_basics_90929794.o \
  $(OBJDIR)/juce_gui_extra_b81d9e1c.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking The Introjucer
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning The Introjucer
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/jucer_AppearanceSettings_788d9889.o: ../../Source/Application/jucer_AppearanceSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_AppearanceSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CommandLine_f35de107.o: ../../Source/Application/jucer_CommandLine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CommandLine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o: ../../Source/Application/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JuceUpdater_cf7865c4.o: ../../Source/Application/jucer_JuceUpdater.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JuceUpdater.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Main_f8488f5b.o: ../../Source/Application/jucer_Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MainWindow_1e163aeb.o: ../../Source/Application/jucer_MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_OpenDocumentManager_4c72d210.o: ../../Source/Application/jucer_OpenDocumentManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ConfigPage_99b07caa.o: ../../Source/Project/jucer_ConfigPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ConfigPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Module_3f7666a5.o: ../../Source/Project/jucer_Module.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Module.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewFileWizard_b8a19ef8.o: ../../Source/Project/jucer_NewFileWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewProjectWizard_7a15bf5d.o: ../../Source/Project/jucer_NewProjectWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewProjectWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Project_c131864a.o: ../../Source/Project/jucer_Project.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectContentComponent_60de0eee.o: ../../Source/Project/jucer_ProjectContentComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectContentComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectTreeViewBase_63c2bdbe.o: ../../Source/Project/jucer_ProjectTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectType_1bc27a24.o: ../../Source/Project/jucer_ProjectType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TreeViewTypes_4a765287.o: ../../Source/Project/jucer_TreeViewTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_TreeViewTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectExporter_eefe2e5b.o: ../../Source/Project\ Saving/jucer_ProjectExporter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceFile_6af120d3.o: ../../Source/Project\ Saving/jucer_ResourceFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CodeHelpers_c317179c.o: ../../Source/Utility/jucer_CodeHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FileHelpers_f98ed0ad.o: ../../Source/Utility/jucer_FileHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerTreeViewBase_d043309d.o: ../../Source/Utility/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MiscUtilities_25b68c82.o: ../../Source/Utility/jucer_MiscUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings_26078d2c.o: ../../Source/Utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SourceCodeEditor_461f5487.o: ../../Source/Code\ Editor/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_1ee54a40.o: ../../../../modules/juce_core/juce_core.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_cryptography_3b1f489a.o: ../../../../modules/juce_cryptography/juce_cryptography.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_cryptography.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_data_structures_84790dfc.o: ../../../../modules/juce_data_structures/juce_data_structures.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_events_584896b4.o: ../../../../modules/juce_events/juce_events.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_graphics_f9afc18.o: ../../../../modules/juce_graphics/juce_graphics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_basics_90929794.o: ../../../../modules/juce_gui_basics/juce_gui_basics.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_extra_b81d9e1c.o: ../../../../modules/juce_gui_extra/juce_gui_extra.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
