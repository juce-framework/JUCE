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
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L/usr/X11R6/lib/ -lX11 -lXext -lXinerama -ldl -lfreetype -lpthread -lrt 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I /usr/include -I /usr/include/freetype2 -I ../../JuceLibraryCode
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/jucer_ComponentTypeHandler_354f298a.o \
  $(OBJDIR)/jucer_ButtonDocument_2640b7a4.o \
  $(OBJDIR)/jucer_ComponentDocument_6b7b1bab.o \
  $(OBJDIR)/jucer_BinaryResources_58dfb26e.o \
  $(OBJDIR)/jucer_ComponentLayout_e6861851.o \
  $(OBJDIR)/jucer_GeneratedCode_21f6b0a6.o \
  $(OBJDIR)/jucer_JucerDocument_84dcbdea.o \
  $(OBJDIR)/jucer_ObjectTypes_b1a6db44.o \
  $(OBJDIR)/jucer_PaintRoutine_28460bc6.o \
  $(OBJDIR)/jucer_ColouredElement_df063961.o \
  $(OBJDIR)/jucer_FillType_95631dd7.o \
  $(OBJDIR)/jucer_PaintElement_e65c9438.o \
  $(OBJDIR)/jucer_PaintElementPath_e99c487d.o \
  $(OBJDIR)/jucer_StrokeType_1099c4c.o \
  $(OBJDIR)/jucer_FilePropertyComponent_1d8c8e56.o \
  $(OBJDIR)/jucer_FontPropertyComponent_7bcf0963.o \
  $(OBJDIR)/jucer_ComponentLayoutEditor_281590ff.o \
  $(OBJDIR)/jucer_ComponentLayoutPanel_e5a9efbc.o \
  $(OBJDIR)/jucer_ComponentOverlayComponent_786fc5d5.o \
  $(OBJDIR)/jucer_EditingPanelBase_c6a2f93c.o \
  $(OBJDIR)/jucer_JucerDocumentHolder_6f9b88d7.o \
  $(OBJDIR)/jucer_MainWindow_eaa39988.o \
  $(OBJDIR)/jucer_PaintRoutineEditor_84958712.o \
  $(OBJDIR)/jucer_PaintRoutinePanel_b7196b49.o \
  $(OBJDIR)/jucer_PrefsPanel_97415233.o \
  $(OBJDIR)/jucer_ResourceEditorPanel_d49fe8d4.o \
  $(OBJDIR)/jucer_SnapGridPainter_5a2aa466.o \
  $(OBJDIR)/jucer_TestComponent_f0e05136.o \
  $(OBJDIR)/jucer_StoredSettings_235fe7c3.o \
  $(OBJDIR)/jucer_UtilityFunctions_9848bfec.o \
  $(OBJDIR)/jucer_Main_f97c9d13.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_core_1ee54a40.o \
  $(OBJDIR)/juce_data_structures_84790dfc.o \
  $(OBJDIR)/juce_events_584896b4.o \
  $(OBJDIR)/juce_graphics_f9afc18.o \
  $(OBJDIR)/juce_gui_basics_90929794.o \
  $(OBJDIR)/juce_gui_extra_b81d9e1c.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking Jucer
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning Jucer
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/jucer_ComponentTypeHandler_354f298a.o: ../../src/model/components/jucer_ComponentTypeHandler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentTypeHandler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ButtonDocument_2640b7a4.o: ../../src/model/documents/jucer_ButtonDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ButtonDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentDocument_6b7b1bab.o: ../../src/model/documents/jucer_ComponentDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_BinaryResources_58dfb26e.o: ../../src/model/jucer_BinaryResources.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_BinaryResources.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentLayout_e6861851.o: ../../src/model/jucer_ComponentLayout.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentLayout.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_GeneratedCode_21f6b0a6.o: ../../src/model/jucer_GeneratedCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_GeneratedCode.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerDocument_84dcbdea.o: ../../src/model/jucer_JucerDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ObjectTypes_b1a6db44.o: ../../src/model/jucer_ObjectTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ObjectTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutine_28460bc6.o: ../../src/model/jucer_PaintRoutine.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutine.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ColouredElement_df063961.o: ../../src/model/paintelements/jucer_ColouredElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ColouredElement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FillType_95631dd7.o: ../../src/model/paintelements/jucer_FillType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FillType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintElement_e65c9438.o: ../../src/model/paintelements/jucer_PaintElement.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintElement.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintElementPath_e99c487d.o: ../../src/model/paintelements/jucer_PaintElementPath.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintElementPath.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StrokeType_1099c4c.o: ../../src/model/paintelements/jucer_StrokeType.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StrokeType.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FilePropertyComponent_1d8c8e56.o: ../../src/properties/jucer_FilePropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FilePropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FontPropertyComponent_7bcf0963.o: ../../src/properties/jucer_FontPropertyComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FontPropertyComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentLayoutEditor_281590ff.o: ../../src/ui/jucer_ComponentLayoutEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentLayoutEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentLayoutPanel_e5a9efbc.o: ../../src/ui/jucer_ComponentLayoutPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentLayoutPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentOverlayComponent_786fc5d5.o: ../../src/ui/jucer_ComponentOverlayComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentOverlayComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_EditingPanelBase_c6a2f93c.o: ../../src/ui/jucer_EditingPanelBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_EditingPanelBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerDocumentHolder_6f9b88d7.o: ../../src/ui/jucer_JucerDocumentHolder.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerDocumentHolder.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MainWindow_eaa39988.o: ../../src/ui/jucer_MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutineEditor_84958712.o: ../../src/ui/jucer_PaintRoutineEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutineEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PaintRoutinePanel_b7196b49.o: ../../src/ui/jucer_PaintRoutinePanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PaintRoutinePanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_PrefsPanel_97415233.o: ../../src/ui/jucer_PrefsPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_PrefsPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceEditorPanel_d49fe8d4.o: ../../src/ui/jucer_ResourceEditorPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceEditorPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SnapGridPainter_5a2aa466.o: ../../src/ui/jucer_SnapGridPainter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SnapGridPainter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TestComponent_f0e05136.o: ../../src/ui/jucer_TestComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_TestComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings_235fe7c3.o: ../../src/utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_UtilityFunctions_9848bfec.o: ../../src/utility/jucer_UtilityFunctions.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_UtilityFunctions.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Main_f97c9d13.o: ../../src/jucer_Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_1ee54a40.o: ../../../../modules/juce_core/juce_core.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core.cpp"
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
