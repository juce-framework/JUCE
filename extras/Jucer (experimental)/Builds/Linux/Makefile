# Automatically generated makefile, created by the Jucer
# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!

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
  CPPFLAGS := $(DEPFLAGS) -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/jucer_CodeGenerator_294a5b95.o \
  $(OBJDIR)/jucer_ComponentDocument_d2c1f367.o \
  $(OBJDIR)/jucer_ComponentTypeManager_21664b47.o \
  $(OBJDIR)/jucer_DrawableDocument_d699b7df.o \
  $(OBJDIR)/jucer_DrawableTypeHandler_5b2f1d76.o \
  $(OBJDIR)/jucer_NewFileWizard_6b466712.o \
  $(OBJDIR)/jucer_Project_60ace9e4.o \
  $(OBJDIR)/jucer_ProjectExporter_8b6a09e5.o \
  $(OBJDIR)/jucer_ProjectWizard_2f72feaf.o \
  $(OBJDIR)/jucer_ResourceFile_4a6c0189.o \
  $(OBJDIR)/jucer_SourceCodeEditor_39e7ba7a.o \
  $(OBJDIR)/jucer_ComponentEditor_bb50044d.o \
  $(OBJDIR)/jucer_ComponentViewer_c7a43a72.o \
  $(OBJDIR)/jucer_DrawableEditor_fdd9b221.o \
  $(OBJDIR)/jucer_EditorCanvas_26232846.o \
  $(OBJDIR)/jucer_DocumentEditorComponent_18f9dc69.o \
  $(OBJDIR)/jucer_JucerTreeViewBase_15bcb6cd.o \
  $(OBJDIR)/jucer_MainWindow_a480b21f.o \
  $(OBJDIR)/jucer_OpenDocumentManager_1fc9355c.o \
  $(OBJDIR)/jucer_GroupInformationComponent_e9638b77.o \
  $(OBJDIR)/jucer_ItemPreviewComponent_3f46a04b.o \
  $(OBJDIR)/jucer_ProjectContentComponent_2f7570e4.o \
  $(OBJDIR)/jucer_ProjectInformationComponent_56d05d91.o \
  $(OBJDIR)/jucer_ProjectTreeViewBase_e3158ab4.o \
  $(OBJDIR)/jucer_TreeViewTypes_3f4b8ffd.o \
  $(OBJDIR)/jucer_CodeHelpers_ed13a7bc.o \
  $(OBJDIR)/jucer_FileHelpers_238b60cd.o \
  $(OBJDIR)/jucer_MiscUtilities_c2cf94a2.o \
  $(OBJDIR)/jucer_StoredSettings_2c0f890c.o \
  $(OBJDIR)/jucer_Main_3602e9c.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/JuceLibraryCode1_682c927f.o \
  $(OBJDIR)/JuceLibraryCode2_683aaa00.o \
  $(OBJDIR)/JuceLibraryCode3_6848c181.o \
  $(OBJDIR)/JuceLibraryCode4_6856d902.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking The Jucer
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning The Jucer
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/jucer_CodeGenerator_294a5b95.o: ../../Source/model/Component/jucer_CodeGenerator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeGenerator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentDocument_d2c1f367.o: ../../Source/model/Component/jucer_ComponentDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentTypeManager_21664b47.o: ../../Source/model/Component/Types/jucer_ComponentTypeManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentTypeManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableDocument_d699b7df.o: ../../Source/model/Drawable/jucer_DrawableDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DrawableDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableTypeHandler_5b2f1d76.o: ../../Source/model/Drawable/jucer_DrawableTypeHandler.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DrawableTypeHandler.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewFileWizard_6b466712.o: ../../Source/model/Project/jucer_NewFileWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Project_60ace9e4.o: ../../Source/model/Project/jucer_Project.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectExporter_8b6a09e5.o: ../../Source/model/Project/jucer_ProjectExporter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectWizard_2f72feaf.o: ../../Source/model/Project/jucer_ProjectWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceFile_4a6c0189.o: ../../Source/model/Project/jucer_ResourceFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SourceCodeEditor_39e7ba7a.o: ../../Source/ui/Code\ Editor/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentEditor_bb50044d.o: ../../Source/ui/Component\ Editor/jucer_ComponentEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentViewer_c7a43a72.o: ../../Source/ui/Component\ Editor/jucer_ComponentViewer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentViewer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableEditor_fdd9b221.o: ../../Source/ui/Drawable\ Editor/jucer_DrawableEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DrawableEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_EditorCanvas_26232846.o: ../../Source/ui/Editor\ Base/jucer_EditorCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_EditorCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DocumentEditorComponent_18f9dc69.o: ../../Source/ui/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerTreeViewBase_15bcb6cd.o: ../../Source/ui/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MainWindow_a480b21f.o: ../../Source/ui/jucer_MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_OpenDocumentManager_1fc9355c.o: ../../Source/ui/jucer_OpenDocumentManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_GroupInformationComponent_e9638b77.o: ../../Source/ui/Project\ Editor/jucer_GroupInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_GroupInformationComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ItemPreviewComponent_3f46a04b.o: ../../Source/ui/Project\ Editor/jucer_ItemPreviewComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ItemPreviewComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectContentComponent_2f7570e4.o: ../../Source/ui/Project\ Editor/jucer_ProjectContentComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectContentComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectInformationComponent_56d05d91.o: ../../Source/ui/Project\ Editor/jucer_ProjectInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectInformationComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectTreeViewBase_e3158ab4.o: ../../Source/ui/Project\ Editor/jucer_ProjectTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TreeViewTypes_3f4b8ffd.o: ../../Source/ui/Project\ Editor/jucer_TreeViewTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_TreeViewTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CodeHelpers_ed13a7bc.o: ../../Source/utility/jucer_CodeHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FileHelpers_238b60cd.o: ../../Source/utility/jucer_FileHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MiscUtilities_c2cf94a2.o: ../../Source/utility/jucer_MiscUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings_2c0f890c.o: ../../Source/utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Main_3602e9c.o: ../../Source/jucer_Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode1_682c927f.o: ../../JuceLibraryCode/JuceLibraryCode1.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode1.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode2_683aaa00.o: ../../JuceLibraryCode/JuceLibraryCode2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode2.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode3_6848c181.o: ../../JuceLibraryCode/JuceLibraryCode3.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode3.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode4_6856d902.o: ../../JuceLibraryCode/JuceLibraryCode4.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode4.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
