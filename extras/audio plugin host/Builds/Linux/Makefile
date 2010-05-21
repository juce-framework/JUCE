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
  TARGET := Plugin\ Host
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "JUCER_LINUX_MAKE_7346DA2A=1" -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := Plugin\ Host
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/FilterGraph.o \
  $(OBJDIR)/GraphEditorPanel.o \
  $(OBJDIR)/HostStartup.o \
  $(OBJDIR)/InternalFilters.o \
  $(OBJDIR)/MainHostWindow.o \
  $(OBJDIR)/JuceLibraryCode1.o \
  $(OBJDIR)/JuceLibraryCode2.o \
  $(OBJDIR)/JuceLibraryCode3.o \
  $(OBJDIR)/JuceLibraryCode4.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking Plugin Host
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning Plugin Host
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/FilterGraph.o: ../../Source/FilterGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterGraph.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GraphEditorPanel.o: ../../Source/GraphEditorPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GraphEditorPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HostStartup.o: ../../Source/HostStartup.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling HostStartup.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InternalFilters.o: ../../Source/InternalFilters.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling InternalFilters.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainHostWindow.o: ../../Source/MainHostWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MainHostWindow.cpp"
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
