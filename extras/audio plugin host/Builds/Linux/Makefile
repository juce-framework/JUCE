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
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4"
  TARGET := Plugin\ Host
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4"
  TARGET := Plugin\ Host
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/FilterGraph_62e9c017.o \
  $(OBJDIR)/GraphEditorPanel_3dbd4872.o \
  $(OBJDIR)/HostStartup_5ce96f96.o \
  $(OBJDIR)/InternalFilters_beb54bdf.o \
  $(OBJDIR)/MainHostWindow_e920295a.o \
  $(OBJDIR)/juce_audio_basics_wrapper_e4aa31f5.o \
  $(OBJDIR)/juce_audio_devices_wrapper_845aeedf.o \
  $(OBJDIR)/juce_audio_formats_wrapper_994a82de.o \
  $(OBJDIR)/juce_audio_processors_wrapper_a34d7091.o \
  $(OBJDIR)/juce_core_wrapper_b07ed786.o \
  $(OBJDIR)/juce_cryptography_wrapper_8b556353.o \
  $(OBJDIR)/juce_data_structures_wrapper_4f7fd480.o \
  $(OBJDIR)/juce_events_wrapper_a8dc8280.o \
  $(OBJDIR)/juce_graphics_wrapper_a0e77172.o \
  $(OBJDIR)/juce_gui_basics_wrapper_e00f2ff0.o \
  $(OBJDIR)/juce_gui_extra_wrapper_b6e18277.o \
  $(OBJDIR)/juce_opengl_wrapper_74bbb916.o \
  $(OBJDIR)/juce_video_wrapper_fcc67966.o \

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

$(OBJDIR)/FilterGraph_62e9c017.o: ../../Source/FilterGraph.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FilterGraph.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/GraphEditorPanel_3dbd4872.o: ../../Source/GraphEditorPanel.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling GraphEditorPanel.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/HostStartup_5ce96f96.o: ../../Source/HostStartup.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling HostStartup.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InternalFilters_beb54bdf.o: ../../Source/InternalFilters.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling InternalFilters.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainHostWindow_e920295a.o: ../../Source/MainHostWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MainHostWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_basics_wrapper_e4aa31f5.o: ../../JuceLibraryCode/juce_audio_basics_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_basics_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_devices_wrapper_845aeedf.o: ../../JuceLibraryCode/juce_audio_devices_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_devices_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_formats_wrapper_994a82de.o: ../../JuceLibraryCode/juce_audio_formats_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_formats_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_audio_processors_wrapper_a34d7091.o: ../../JuceLibraryCode/juce_audio_processors_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_processors_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_wrapper_b07ed786.o: ../../JuceLibraryCode/juce_core_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_cryptography_wrapper_8b556353.o: ../../JuceLibraryCode/juce_cryptography_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_cryptography_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_data_structures_wrapper_4f7fd480.o: ../../JuceLibraryCode/juce_data_structures_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_data_structures_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_events_wrapper_a8dc8280.o: ../../JuceLibraryCode/juce_events_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_events_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_graphics_wrapper_a0e77172.o: ../../JuceLibraryCode/juce_graphics_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_graphics_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_basics_wrapper_e00f2ff0.o: ../../JuceLibraryCode/juce_gui_basics_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_basics_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_gui_extra_wrapper_b6e18277.o: ../../JuceLibraryCode/juce_gui_extra_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_gui_extra_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_opengl_wrapper_74bbb916.o: ../../JuceLibraryCode/juce_opengl_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_opengl_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_video_wrapper_fcc67966.o: ../../JuceLibraryCode/juce_video_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_video_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
