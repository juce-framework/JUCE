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
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := JuceDemo
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS) 
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -L"/usr/X11R6/lib/" -L"../../../../../juce/bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound 
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_LINUX_MAKE_7346DA2A=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := JuceDemo
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/ApplicationStartup_3bf86e16.o \
  $(OBJDIR)/MainDemoWindow_f9e963d5.o \
  $(OBJDIR)/AudioDemoLatencyPage_c12cc5dc.o \
  $(OBJDIR)/AudioDemoPlaybackPage_a7b17745.o \
  $(OBJDIR)/AudioDemoRecordPage_77bd50fb.o \
  $(OBJDIR)/AudioDemoSetupPage_e6febadb.o \
  $(OBJDIR)/AudioDemoSynthPage_6d168c9a.o \
  $(OBJDIR)/AudioDemoTabComponent_81185e3.o \
  $(OBJDIR)/CameraDemo_954a4a30.o \
  $(OBJDIR)/CodeEditorDemo_55f35645.o \
  $(OBJDIR)/DirectShowDemo_2e0bc311.o \
  $(OBJDIR)/DragAndDropDemo_f8c9e5d7.o \
  $(OBJDIR)/FontsAndTextDemo_2cefcecb.o \
  $(OBJDIR)/InterprocessCommsDemo_aa1c1119.o \
  $(OBJDIR)/OpenGLDemo_f1e721ba.o \
  $(OBJDIR)/QuickTimeDemo_84dae9df.o \
  $(OBJDIR)/RenderingTestComponent_dde8a0e7.o \
  $(OBJDIR)/TableDemo_99f71d3.o \
  $(OBJDIR)/ThreadingDemo_35b0f31d.o \
  $(OBJDIR)/TreeViewDemo_c130a3ae.o \
  $(OBJDIR)/WebBrowserDemo_be24b7bf.o \
  $(OBJDIR)/WidgetsDemo_9419b474.o \
  $(OBJDIR)/BinaryData_ce4232d4.o \
  $(OBJDIR)/juce_audio_basics_wrapper_e4aa31f5.o \
  $(OBJDIR)/juce_audio_devices_wrapper_845aeedf.o \
  $(OBJDIR)/juce_audio_formats_wrapper_994a82de.o \
  $(OBJDIR)/juce_audio_processors_wrapper_a34d7091.o \
  $(OBJDIR)/juce_audio_utils_wrapper_e9783ef3.o \
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
	@echo Linking Juce Demo
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning Juce Demo
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/ApplicationStartup_3bf86e16.o: ../../Source/ApplicationStartup.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ApplicationStartup.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainDemoWindow_f9e963d5.o: ../../Source/MainDemoWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MainDemoWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoLatencyPage_c12cc5dc.o: ../../Source/demos/AudioDemoLatencyPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoLatencyPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoPlaybackPage_a7b17745.o: ../../Source/demos/AudioDemoPlaybackPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoPlaybackPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoRecordPage_77bd50fb.o: ../../Source/demos/AudioDemoRecordPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoRecordPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoSetupPage_e6febadb.o: ../../Source/demos/AudioDemoSetupPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoSetupPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoSynthPage_6d168c9a.o: ../../Source/demos/AudioDemoSynthPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoSynthPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoTabComponent_81185e3.o: ../../Source/demos/AudioDemoTabComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoTabComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CameraDemo_954a4a30.o: ../../Source/demos/CameraDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CameraDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CodeEditorDemo_55f35645.o: ../../Source/demos/CodeEditorDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CodeEditorDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DirectShowDemo_2e0bc311.o: ../../Source/demos/DirectShowDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DirectShowDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DragAndDropDemo_f8c9e5d7.o: ../../Source/demos/DragAndDropDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DragAndDropDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FontsAndTextDemo_2cefcecb.o: ../../Source/demos/FontsAndTextDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FontsAndTextDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InterprocessCommsDemo_aa1c1119.o: ../../Source/demos/InterprocessCommsDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling InterprocessCommsDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OpenGLDemo_f1e721ba.o: ../../Source/demos/OpenGLDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OpenGLDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/QuickTimeDemo_84dae9df.o: ../../Source/demos/QuickTimeDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling QuickTimeDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RenderingTestComponent_dde8a0e7.o: ../../Source/demos/RenderingTestComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RenderingTestComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TableDemo_99f71d3.o: ../../Source/demos/TableDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TableDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ThreadingDemo_35b0f31d.o: ../../Source/demos/ThreadingDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ThreadingDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TreeViewDemo_c130a3ae.o: ../../Source/demos/TreeViewDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TreeViewDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WebBrowserDemo_be24b7bf.o: ../../Source/demos/WebBrowserDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WebBrowserDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WidgetsDemo_9419b474.o: ../../Source/demos/WidgetsDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WidgetsDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
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
