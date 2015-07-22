# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=3.2.0" -D "JUCE_APP_VERSION_HEX=0x30200" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../../../modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0 -std=gnu++0x -Wreorder -Wuninitialized -Wunused-parameter -Wstrict-aliasing -Wshadow -Wsign-compare
  CXXFLAGS += $(CFLAGS) -std=c++11
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt -lcurl 

  TARGET := Introjucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := 
  endif

  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "JUCE_APP_VERSION=3.2.0" -D "JUCE_APP_VERSION_HEX=0x30200" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode -I ../../../../modules
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3 -std=gnu++0x -Wreorder -Wuninitialized -Wunused-parameter -Wstrict-aliasing -Wshadow -Wsign-compare
  CXXFLAGS += $(CFLAGS) -std=c++11
  LDFLAGS += $(TARGET_ARCH) -L$(BINDIR) -L$(LIBDIR) -fvisibility=hidden -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt -lcurl 

  TARGET := Introjucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(OUTDIR)/$(TARGET) $(OBJDIR)
endif

OBJECTS := \
  $(OBJDIR)/jucer_AppearanceSettings_788d9889.o \
  $(OBJDIR)/jucer_AutoUpdater_ca658dc2.o \
  $(OBJDIR)/jucer_CommandLine_f35de107.o \
  $(OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o \
  $(OBJDIR)/jucer_Main_f8488f5b.o \
  $(OBJDIR)/jucer_MainWindow_1e163aeb.o \
  $(OBJDIR)/jucer_OpenDocumentManager_4c72d210.o \
  $(OBJDIR)/jucer_SourceCodeEditor_461f5487.o \
  $(OBJDIR)/jucer_ComponentTypeHandler_6bec6262.o \
  $(OBJDIR)/jucer_ButtonDocument_56c341cc.o \
  $(OBJDIR)/jucer_ComponentDocument_92868083.o \
  $(OBJDIR)/jucer_ColouredElement_70cbc839.o \
  $(OBJDIR)/jucer_PaintElement_653460.o \
  $(OBJDIR)/jucer_PaintElementPath_908894a5.o \
  $(OBJDIR)/jucer_ComponentLayoutEditor_aeb44f8d.o \
  $(OBJDIR)/jucer_ComponentOverlayComponent_fd508d63.o \
  $(OBJDIR)/jucer_EditingPanelBase_6c2ed9ee.o \
  $(OBJDIR)/jucer_JucerDocumentEditor_2c581e26.o \
  $(OBJDIR)/jucer_PaintRoutineEditor_f6ac0344.o \
  $(OBJDIR)/jucer_PaintRoutinePanel_c309a0d7.o \
  $(OBJDIR)/jucer_ResourceEditorPanel_a558f2e2.o \
  $(OBJDIR)/jucer_TestComponent_db757dc4.o \
  $(OBJDIR)/jucer_BinaryResources_e0f99b46.o \
  $(OBJDIR)/jucer_ComponentLayout_6ea00129.o \
  $(OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o \
  $(OBJDIR)/jucer_JucerDocument_ff8afcc2.o \
  $(OBJDIR)/jucer_ObjectTypes_4406f01c.o \
  $(OBJDIR)/jucer_PaintRoutine_e1e891ee.o \
  $(OBJDIR)/jucer_ProjectExporter_eefe2e5b.o \
  $(OBJDIR)/jucer_ResourceFile_6af120d3.o \
  $(OBJDIR)/jucer_Module_3f7666a5.o \
  $(OBJDIR)/jucer_Project_c131864a.o \
  $(OBJDIR)/jucer_ProjectContentComponent_60de0eee.o \
  $(OBJDIR)/jucer_ProjectType_1bc27a24.o \
  $(OBJDIR)/jucer_CodeHelpers_c317179c.o \
  $(OBJDIR)/jucer_FileHelpers_f98ed0ad.o \
  $(OBJDIR)/jucer_Icons_9db2f51e.o \
  $(OBJDIR)/jucer_JucerTreeViewBase_d043309d.o \
  $(OBJDIR)/jucer_MiscUtilities_25b68c82.o \
  $(OBJDIR)/jucer_SlidingPanelComponent_9aa7a2ab.o \
  $(OBJDIR)/jucer_StoredSettings_26078d2c.o \
  $(OBJDIR)/jucer_NewFileWizard_fac97f47.o \
  $(OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_core_1ee54a40.o \
  $(OBJDIR)/juce_cryptography_3b1f489a.o \
  $(OBJDIR)/juce_data_structures_84790dfc.o \
  $(OBJDIR)/juce_events_584896b4.o \
  $(OBJDIR)/juce_graphics_f9afc18.o \
  $(OBJDIR)/juce_gui_basics_90929794.o \
  $(OBJDIR)/juce_gui_extra_b81d9e1c.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(RESOURCES)
	@echo Linking The Introjucer
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning The Introjucer
	@$(CLEANCMD)

strip:
	@echo Stripping The Introjucer
	-@strip --strip-unneeded $(OUTDIR)/$(TARGET)

$(OBJDIR)/jucer_AppearanceSettings_788d9889.o: ../../Source/Application/jucer_AppearanceSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_AppearanceSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_AutoUpdater_ca658dc2.o: ../../Source/Application/jucer_AutoUpdater.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_AutoUpdater.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CommandLine_f35de107.o: ../../Source/Application/jucer_CommandLine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CommandLine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DocumentEditorComponent_695dff1d.o: ../../Source/Application/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
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

$(OBJDIR)/jucer_SourceCodeEditor_461f5487.o: ../../Source/Code\ Editor/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentTypeHandler_6bec6262.o: ../../Source/ComponentEditor/components/jucer_ComponentTypeHandler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentTypeHandler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ButtonDocument_56c341cc.o: ../../Source/ComponentEditor/documents/jucer_ButtonDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ButtonDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentDocument_92868083.o: ../../Source/ComponentEditor/documents/jucer_ComponentDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ColouredElement_70cbc839.o: ../../Source/ComponentEditor/paintelements/jucer_ColouredElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ColouredElement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintElement_653460.o: ../../Source/ComponentEditor/paintelements/jucer_PaintElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintElement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintElementPath_908894a5.o: ../../Source/ComponentEditor/paintelements/jucer_PaintElementPath.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintElementPath.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentLayoutEditor_aeb44f8d.o: ../../Source/ComponentEditor/ui/jucer_ComponentLayoutEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentLayoutEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentOverlayComponent_fd508d63.o: ../../Source/ComponentEditor/ui/jucer_ComponentOverlayComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentOverlayComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_EditingPanelBase_6c2ed9ee.o: ../../Source/ComponentEditor/ui/jucer_EditingPanelBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_EditingPanelBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerDocumentEditor_2c581e26.o: ../../Source/ComponentEditor/ui/jucer_JucerDocumentEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerDocumentEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutineEditor_f6ac0344.o: ../../Source/ComponentEditor/ui/jucer_PaintRoutineEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutineEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutinePanel_c309a0d7.o: ../../Source/ComponentEditor/ui/jucer_PaintRoutinePanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutinePanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceEditorPanel_a558f2e2.o: ../../Source/ComponentEditor/ui/jucer_ResourceEditorPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceEditorPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TestComponent_db757dc4.o: ../../Source/ComponentEditor/ui/jucer_TestComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_TestComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_BinaryResources_e0f99b46.o: ../../Source/ComponentEditor/jucer_BinaryResources.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_BinaryResources.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentLayout_6ea00129.o: ../../Source/ComponentEditor/jucer_ComponentLayout.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentLayout.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_GeneratedCode_9ca4ef7e.o: ../../Source/ComponentEditor/jucer_GeneratedCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_GeneratedCode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerDocument_ff8afcc2.o: ../../Source/ComponentEditor/jucer_JucerDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ObjectTypes_4406f01c.o: ../../Source/ComponentEditor/jucer_ObjectTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ObjectTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutine_e1e891ee.o: ../../Source/ComponentEditor/jucer_PaintRoutine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectExporter_eefe2e5b.o: ../../Source/Project\ Saving/jucer_ProjectExporter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceFile_6af120d3.o: ../../Source/Project\ Saving/jucer_ResourceFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Module_3f7666a5.o: ../../Source/Project/jucer_Module.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Module.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Project_c131864a.o: ../../Source/Project/jucer_Project.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectContentComponent_60de0eee.o: ../../Source/Project/jucer_ProjectContentComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectContentComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectType_1bc27a24.o: ../../Source/Project/jucer_ProjectType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CodeHelpers_c317179c.o: ../../Source/Utility/jucer_CodeHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FileHelpers_f98ed0ad.o: ../../Source/Utility/jucer_FileHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Icons_9db2f51e.o: ../../Source/Utility/jucer_Icons.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Icons.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerTreeViewBase_d043309d.o: ../../Source/Utility/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MiscUtilities_25b68c82.o: ../../Source/Utility/jucer_MiscUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SlidingPanelComponent_9aa7a2ab.o: ../../Source/Utility/jucer_SlidingPanelComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SlidingPanelComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings_26078d2c.o: ../../Source/Utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewFileWizard_fac97f47.o: ../../Source/Wizards/jucer_NewFileWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewProjectWizardClasses_891f6fa2.o: ../../Source/Wizards/jucer_NewProjectWizardClasses.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewProjectWizardClasses.cpp"
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
