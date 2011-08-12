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
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -fPIC -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4" -I "../../JuceLibraryCode"
  TARGET := JuceDemoPlugin.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4" -I "../../JuceLibraryCode"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -fPIC -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -shared -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2" -I "~/SDKs/vstsdk2.4" -I "../../JuceLibraryCode"
  TARGET := JuceDemoPlugin.so
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/PluginProcessor_a059e380.o \
  $(OBJDIR)/PluginEditor_94d4fb09.o \
  $(OBJDIR)/juce_audio_basics_wrapper_e4aa31f5.o \
  $(OBJDIR)/juce_audio_devices_wrapper_845aeedf.o \
  $(OBJDIR)/juce_audio_formats_wrapper_994a82de.o \
  $(OBJDIR)/juce_audio_processors_wrapper_a34d7091.o \
  $(OBJDIR)/juce_audio_utils_wrapper_e9783ef3.o \
  $(OBJDIR)/juce_core_wrapper_b07ed786.o \
  $(OBJDIR)/juce_data_structures_wrapper_4f7fd480.o \
  $(OBJDIR)/juce_events_wrapper_a8dc8280.o \
  $(OBJDIR)/juce_graphics_wrapper_a0e77172.o \
  $(OBJDIR)/juce_gui_basics_wrapper_e00f2ff0.o \
  $(OBJDIR)/juce_RTAS_DigiCode1_wrapper_6c4cad39.o \
  $(OBJDIR)/juce_RTAS_DigiCode2_wrapper_5acfb3ba.o \
  $(OBJDIR)/juce_RTAS_DigiCode3_wrapper_4952ba3b.o \
  $(OBJDIR)/juce_RTAS_Wrapper_wrapper_1df0fd8f.o \
  $(OBJDIR)/juce_VST_Wrapper_wrapper_da6c54d6.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking JuceDemoPlugin
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning JuceDemoPlugin
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/PluginProcessor_a059e380.o: ../../Source/PluginProcessor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginProcessor.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/PluginEditor_94d4fb09.o: ../../Source/PluginEditor.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling PluginEditor.cpp"
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

$(OBJDIR)/juce_audio_utils_wrapper_e9783ef3.o: ../../JuceLibraryCode/juce_audio_utils_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_audio_utils_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_core_wrapper_b07ed786.o: ../../JuceLibraryCode/juce_core_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_core_wrapper.cpp"
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

$(OBJDIR)/juce_RTAS_DigiCode1_wrapper_6c4cad39.o: ../../JuceLibraryCode/juce_RTAS_DigiCode1_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RTAS_DigiCode1_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RTAS_DigiCode2_wrapper_5acfb3ba.o: ../../JuceLibraryCode/juce_RTAS_DigiCode2_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RTAS_DigiCode2_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RTAS_DigiCode3_wrapper_4952ba3b.o: ../../JuceLibraryCode/juce_RTAS_DigiCode3_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RTAS_DigiCode3_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_RTAS_Wrapper_wrapper_1df0fd8f.o: ../../JuceLibraryCode/juce_RTAS_Wrapper_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_RTAS_Wrapper_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/juce_VST_Wrapper_wrapper_da6c54d6.o: ../../JuceLibraryCode/juce_VST_Wrapper_wrapper.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling juce_VST_Wrapper_wrapper.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
