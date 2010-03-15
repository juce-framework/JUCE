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
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -mwindows -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -O3
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -mwindows -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := Jucer
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/jucer_ComponentDocument.o \
  $(OBJDIR)/jucer_DrawableDocument.o \
  $(OBJDIR)/jucer_NewFileWizard.o \
  $(OBJDIR)/jucer_Project.o \
  $(OBJDIR)/jucer_ProjectExporter.o \
  $(OBJDIR)/jucer_ProjectWizard.o \
  $(OBJDIR)/jucer_ResourceFile.o \
  $(OBJDIR)/jucer_DrawableEditor.o \
  $(OBJDIR)/jucer_DocumentEditorComponent.o \
  $(OBJDIR)/jucer_GroupInformationComponent.o \
  $(OBJDIR)/jucer_ItemPreviewComponent.o \
  $(OBJDIR)/jucer_JucerTreeViewBase.o \
  $(OBJDIR)/jucer_MainWindow.o \
  $(OBJDIR)/jucer_OpenDocumentManager.o \
  $(OBJDIR)/jucer_ProjectContentComponent.o \
  $(OBJDIR)/jucer_ProjectInformationComponent.o \
  $(OBJDIR)/jucer_ProjectTreeViewBase.o \
  $(OBJDIR)/jucer_SourceCodeEditor.o \
  $(OBJDIR)/jucer_TreeViewTypes.o \
  $(OBJDIR)/jucer_StoredSettings.o \
  $(OBJDIR)/jucer_UtilityFunctions.o \
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

$(OBJDIR)/jucer_ComponentDocument.o: ../../Source/model/jucer_ComponentDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableDocument.o: ../../Source/model/jucer_DrawableDocument.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_NewFileWizard.o: ../../Source/model/jucer_NewFileWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Project.o: ../../Source/model/jucer_Project.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectExporter.o: ../../Source/model/jucer_ProjectExporter.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectWizard.o: ../../Source/model/jucer_ProjectWizard.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ResourceFile.o: ../../Source/model/jucer_ResourceFile.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DrawableEditor.o: ../../Source/ui/Drawable\ Editor/jucer_DrawableEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_DocumentEditorComponent.o: ../../Source/ui/jucer_DocumentEditorComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_GroupInformationComponent.o: ../../Source/ui/jucer_GroupInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ItemPreviewComponent.o: ../../Source/ui/jucer_ItemPreviewComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_JucerTreeViewBase.o: ../../Source/ui/jucer_JucerTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_MainWindow.o: ../../Source/ui/jucer_MainWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_OpenDocumentManager.o: ../../Source/ui/jucer_OpenDocumentManager.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectContentComponent.o: ../../Source/ui/jucer_ProjectContentComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectInformationComponent.o: ../../Source/ui/jucer_ProjectInformationComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_ProjectTreeViewBase.o: ../../Source/ui/jucer_ProjectTreeViewBase.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_SourceCodeEditor.o: ../../Source/ui/jucer_SourceCodeEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_TreeViewTypes.o: ../../Source/ui/jucer_TreeViewTypes.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_StoredSettings.o: ../../Source/utility/jucer_StoredSettings.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_UtilityFunctions.o: ../../Source/utility/jucer_UtilityFunctions.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/jucer_Main.o: ../../Source/jucer_Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode1.o: ../../JuceLibraryCode/JuceLibraryCode1.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode2.o: ../../JuceLibraryCode/JuceLibraryCode2.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode3.o: ../../JuceLibraryCode/JuceLibraryCode3.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode4.o: ../../JuceLibraryCode/JuceLibraryCode4.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
