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
  $(OBJDIR)/jucer_CodeGenerator.o \
  $(OBJDIR)/jucer_ComponentDocument.o \
  $(OBJDIR)/jucer_ComponentTypeManager.o \
  $(OBJDIR)/jucer_DrawableDocument.o \
  $(OBJDIR)/jucer_NewFileWizard.o \
  $(OBJDIR)/jucer_Project.o \
  $(OBJDIR)/jucer_ProjectExporter.o \
  $(OBJDIR)/jucer_ProjectWizard.o \
  $(OBJDIR)/jucer_ResourceFile.o \
  $(OBJDIR)/jucer_SourceCodeEditor.o \
  $(OBJDIR)/jucer_ComponentEditor.o \
  $(OBJDIR)/jucer_ComponentViewer.o \
  $(OBJDIR)/jucer_DrawableEditor.o \
  $(OBJDIR)/jucer_EditorCanvas.o \
  $(OBJDIR)/jucer_DocumentEditorComponent.o \
  $(OBJDIR)/jucer_JucerTreeViewBase.o \
  $(OBJDIR)/jucer_MainWindow.o \
  $(OBJDIR)/jucer_OpenDocumentManager.o \
  $(OBJDIR)/jucer_GroupInformationComponent.o \
  $(OBJDIR)/jucer_ItemPreviewComponent.o \
  $(OBJDIR)/jucer_ProjectContentComponent.o \
  $(OBJDIR)/jucer_ProjectInformationComponent.o \
  $(OBJDIR)/jucer_ProjectTreeViewBase.o \
  $(OBJDIR)/jucer_TreeViewTypes.o \
  $(OBJDIR)/jucer_CodeHelpers.o \
  $(OBJDIR)/jucer_FileHelpers.o \
  $(OBJDIR)/jucer_StoredSettings.o \
  $(OBJDIR)/jucer_MiscUtilities.o \
  $(OBJDIR)/jucer_Main.o \
  $(OBJDIR)/BinaryData.o \
  $(OBJDIR)/JuceLibraryCode1.o \
  $(OBJDIR)/JuceLibraryCode2.o \
  $(OBJDIR)/JuceLibraryCode3.o \
  $(OBJDIR)/JuceLibraryCode4.o \

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

$(OBJDIR)/jucer_CodeGenerator.o: ../../Source/model/Component/jucer_CodeGenerator.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeGenerator.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentDocument.o: ../../Source/model/Component/jucer_ComponentDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentTypeManager.o: ../../Source/model/Component/Types/jucer_ComponentTypeManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentTypeManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableDocument.o: ../../Source/model/Drawable/jucer_DrawableDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DrawableDocument.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewFileWizard.o: ../../Source/model/Project/jucer_NewFileWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_NewFileWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Project.o: ../../Source/model/Project/jucer_Project.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Project.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectExporter.o: ../../Source/model/Project/jucer_ProjectExporter.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectExporter.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectWizard.o: ../../Source/model/Project/jucer_ProjectWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectWizard.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceFile.o: ../../Source/model/Project/jucer_ResourceFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ResourceFile.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SourceCodeEditor.o: ../../Source/ui/Code\ Editor/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_SourceCodeEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentEditor.o: ../../Source/ui/Component\ Editor/jucer_ComponentEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ComponentViewer.o: ../../Source/ui/Component\ Editor/jucer_ComponentViewer.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ComponentViewer.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableEditor.o: ../../Source/ui/Drawable\ Editor/jucer_DrawableEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DrawableEditor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_EditorCanvas.o: ../../Source/ui/Editor\ Base/jucer_EditorCanvas.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_EditorCanvas.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DocumentEditorComponent.o: ../../Source/ui/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_DocumentEditorComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerTreeViewBase.o: ../../Source/ui/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_JucerTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MainWindow.o: ../../Source/ui/jucer_MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MainWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_OpenDocumentManager.o: ../../Source/ui/jucer_OpenDocumentManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_OpenDocumentManager.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_GroupInformationComponent.o: ../../Source/ui/Project\ Editor/jucer_GroupInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_GroupInformationComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ItemPreviewComponent.o: ../../Source/ui/Project\ Editor/jucer_ItemPreviewComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ItemPreviewComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectContentComponent.o: ../../Source/ui/Project\ Editor/jucer_ProjectContentComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectContentComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectInformationComponent.o: ../../Source/ui/Project\ Editor/jucer_ProjectInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectInformationComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectTreeViewBase.o: ../../Source/ui/Project\ Editor/jucer_ProjectTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_ProjectTreeViewBase.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TreeViewTypes.o: ../../Source/ui/Project\ Editor/jucer_TreeViewTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_TreeViewTypes.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_CodeHelpers.o: ../../Source/utility/jucer_CodeHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_CodeHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_FileHelpers.o: ../../Source/utility/jucer_FileHelpers.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_FileHelpers.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings.o: ../../Source/utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_StoredSettings.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MiscUtilities.o: ../../Source/utility/jucer_MiscUtilities.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_MiscUtilities.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Main.o: ../../Source/jucer_Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling jucer_Main.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode1.o: ../../JuceLibraryCode/JuceLibraryCode1.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode1.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode2.o: ../../JuceLibraryCode/JuceLibraryCode2.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode2.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode3.o: ../../JuceLibraryCode/JuceLibraryCode3.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode3.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode4.o: ../../JuceLibraryCode/JuceLibraryCode4.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling JuceLibraryCode4.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
