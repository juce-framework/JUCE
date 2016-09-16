# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

# (this disables dependency generation if multiple architectures are set)
DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifndef STRIP
  STRIP=strip
endif

ifndef AR
  AR=ar
endif

ifndef CONFIG
  CONFIG=Debug
endif

ifeq ($(CONFIG),Debug)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Debug
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DDEBUG=1 -D_DEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_LINUX_MAKE_7346DA2A=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000 $(shell pkg-config --cflags alsa freetype2 libcurl x11 xext xinerama) -pthread -I../../JuceLibraryCode -I../../../../modules
  JUCE_CFLAGS += $(CFLAGS) $(JUCE_CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  JUCE_CXXFLAGS += $(CXXFLAGS) $(JUCE_CFLAGS) -std=c++11
  JUCE_LDFLAGS += $(LDFLAGS) $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -L/usr/X11R6/lib/ $(shell pkg-config --libs alsa freetype2 libcurl x11 xext xinerama) -lGL -ldl -lpthread -lrt 

  TARGET := JuceDemo
  BLDCMD = $(CXX) -o $(JUCE_OUTDIR)/$(TARGET) $(OBJECTS) $(JUCE_LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

ifeq ($(CONFIG),Release)
  JUCE_BINDIR := build
  JUCE_LIBDIR := build
  JUCE_OBJDIR := build/intermediate/Release
  JUCE_OUTDIR := build

  ifeq ($(TARGET_ARCH),)
    TARGET_ARCH := -march=native
  endif

  JUCE_CPPFLAGS := $(DEPFLAGS) -DLINUX=1 -DNDEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_LINUX_MAKE_7346DA2A=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000 $(shell pkg-config --cflags alsa freetype2 libcurl x11 xext xinerama) -pthread -I../../JuceLibraryCode -I../../../../modules
  JUCE_CFLAGS += $(CFLAGS) $(JUCE_CPPFLAGS) $(TARGET_ARCH) -O3
  JUCE_CXXFLAGS += $(CXXFLAGS) $(JUCE_CFLAGS) -std=c++11
  JUCE_LDFLAGS += $(LDFLAGS) $(TARGET_ARCH) -L$(JUCE_BINDIR) -L$(JUCE_LIBDIR) -fvisibility=hidden -L/usr/X11R6/lib/ $(shell pkg-config --libs alsa freetype2 libcurl x11 xext xinerama) -lGL -ldl -lpthread -lrt 

  TARGET := JuceDemo
  BLDCMD = $(CXX) -o $(JUCE_OUTDIR)/$(TARGET) $(OBJECTS) $(JUCE_LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
  CLEANCMD = rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)
endif

OBJECTS := \
  $(JUCE_OBJDIR)/Main_90ebc5c2.o \
  $(JUCE_OBJDIR)/MainWindow_499ac812.o \
  $(JUCE_OBJDIR)/IntroScreen_73347059.o \
  $(JUCE_OBJDIR)/AnimationDemo_9fc7e4e9.o \
  $(JUCE_OBJDIR)/AudioLatencyDemo_78d49233.o \
  $(JUCE_OBJDIR)/AudioPlaybackDemo_ea12adf6.o \
  $(JUCE_OBJDIR)/AudioRecordingDemo_435914e6.o \
  $(JUCE_OBJDIR)/AudioSettingsDemo_6d710bfe.o \
  $(JUCE_OBJDIR)/AudioSynthesiserDemo_1b3666f6.o \
  $(JUCE_OBJDIR)/Box2DDemo_51053c42.o \
  $(JUCE_OBJDIR)/CameraDemo_a10f7e50.o \
  $(JUCE_OBJDIR)/ChildProcessDemo_2c2b949e.o \
  $(JUCE_OBJDIR)/CodeEditorDemo_d5c17a65.o \
  $(JUCE_OBJDIR)/ComponentTransformsDemo_377f7889.o \
  $(JUCE_OBJDIR)/CryptographyDemo_232d46d7.o \
  $(JUCE_OBJDIR)/DialogsDemo_19d585d0.o \
  $(JUCE_OBJDIR)/FlexBoxDemo_e4f3d9b7.o \
  $(JUCE_OBJDIR)/FontsDemo_6da1e5e9.o \
  $(JUCE_OBJDIR)/GraphicsDemo_85142ef6.o \
  $(JUCE_OBJDIR)/ImagesDemo_ecf13aa3.o \
  $(JUCE_OBJDIR)/JavaScript_d6e42eb5.o \
  $(JUCE_OBJDIR)/KeyMappingsDemo_cf05c709.o \
  $(JUCE_OBJDIR)/LiveConstantDemo_e3b18a9b.o \
  $(JUCE_OBJDIR)/LookAndFeelDemo_25c558a3.o \
  $(JUCE_OBJDIR)/MDIDemo_c476c2d7.o \
  $(JUCE_OBJDIR)/MidiDemo_75ebc30c.o \
  $(JUCE_OBJDIR)/MultithreadingDemo_ff566eaa.o \
  $(JUCE_OBJDIR)/MultiTouch_595f3a2e.o \
  $(JUCE_OBJDIR)/NetworkingDemo_fbd1b19f.o \
  $(JUCE_OBJDIR)/OpenGLDemo_fdac55da.o \
  $(JUCE_OBJDIR)/OpenGLDemo2D_48eeadec.o \
  $(JUCE_OBJDIR)/PropertiesDemo_8ff1a8de.o \
  $(JUCE_OBJDIR)/SystemInfoDemo_984acd28.o \
  $(JUCE_OBJDIR)/TimersAndEventsDemo_f2656547.o \
  $(JUCE_OBJDIR)/UnitTestsDemo_5c21762.o \
  $(JUCE_OBJDIR)/ValueTreesDemo_9657d84f.o \
  $(JUCE_OBJDIR)/VideoDemo_b55a1560.o \
  $(JUCE_OBJDIR)/WebBrowserDemo_3df2dbdf.o \
  $(JUCE_OBJDIR)/WidgetsDemo_fb0454.o \
  $(JUCE_OBJDIR)/WindowsDemo_e8cfa428.o \
  $(JUCE_OBJDIR)/XMLandJSONDemo_5bc88df3.o \
  $(JUCE_OBJDIR)/BinaryData_ce4232d4.o \
  $(JUCE_OBJDIR)/juce_audio_basics_6b797ca1.o \
  $(JUCE_OBJDIR)/juce_audio_devices_a742c38b.o \
  $(JUCE_OBJDIR)/juce_audio_formats_5a29c68a.o \
  $(JUCE_OBJDIR)/juce_audio_processors_dea3173d.o \
  $(JUCE_OBJDIR)/juce_audio_utils_c7eb679f.o \
  $(JUCE_OBJDIR)/juce_box2d_95719d14.o \
  $(JUCE_OBJDIR)/juce_core_75b14332.o \
  $(JUCE_OBJDIR)/juce_cryptography_6de2ebff.o \
  $(JUCE_OBJDIR)/juce_data_structures_72d3da2c.o \
  $(JUCE_OBJDIR)/juce_events_d2be882c.o \
  $(JUCE_OBJDIR)/juce_graphics_9c18891e.o \
  $(JUCE_OBJDIR)/juce_gui_basics_8a6da59c.o \
  $(JUCE_OBJDIR)/juce_gui_extra_4a026f23.o \
  $(JUCE_OBJDIR)/juce_opengl_cd70b4c2.o \
  $(JUCE_OBJDIR)/juce_video_f128c512.o \

.PHONY: clean

$(JUCE_OUTDIR)/$(TARGET): check-pkg-config $(OBJECTS) $(RESOURCES)
	@echo Linking JuceDemo
	-@mkdir -p $(JUCE_BINDIR)
	-@mkdir -p $(JUCE_LIBDIR)
	-@mkdir -p $(JUCE_OUTDIR)
	@$(BLDCMD)

check-pkg-config:
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors alsa freetype2 libcurl x11 xext xinerama

clean:
	@echo Cleaning JuceDemo
	@$(CLEANCMD)

strip:
	@echo Stripping JuceDemo
	-@$(STRIP) --strip-unneeded $(JUCE_OUTDIR)/$(TARGET)

$(JUCE_OBJDIR)/Main_90ebc5c2.o: ../../Source/Main.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling Main.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MainWindow_499ac812.o: ../../Source/MainWindow.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MainWindow.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/IntroScreen_73347059.o: ../../Source/IntroScreen.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling IntroScreen.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AnimationDemo_9fc7e4e9.o: ../../Source/Demos/AnimationDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AnimationDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AudioLatencyDemo_78d49233.o: ../../Source/Demos/AudioLatencyDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AudioLatencyDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AudioPlaybackDemo_ea12adf6.o: ../../Source/Demos/AudioPlaybackDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AudioPlaybackDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AudioRecordingDemo_435914e6.o: ../../Source/Demos/AudioRecordingDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AudioRecordingDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AudioSettingsDemo_6d710bfe.o: ../../Source/Demos/AudioSettingsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AudioSettingsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/AudioSynthesiserDemo_1b3666f6.o: ../../Source/Demos/AudioSynthesiserDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling AudioSynthesiserDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/Box2DDemo_51053c42.o: ../../Source/Demos/Box2DDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling Box2DDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/CameraDemo_a10f7e50.o: ../../Source/Demos/CameraDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling CameraDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ChildProcessDemo_2c2b949e.o: ../../Source/Demos/ChildProcessDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ChildProcessDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/CodeEditorDemo_d5c17a65.o: ../../Source/Demos/CodeEditorDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling CodeEditorDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ComponentTransformsDemo_377f7889.o: ../../Source/Demos/ComponentTransformsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ComponentTransformsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/CryptographyDemo_232d46d7.o: ../../Source/Demos/CryptographyDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling CryptographyDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/DialogsDemo_19d585d0.o: ../../Source/Demos/DialogsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling DialogsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FlexBoxDemo_e4f3d9b7.o: ../../Source/Demos/FlexBoxDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FlexBoxDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/FontsDemo_6da1e5e9.o: ../../Source/Demos/FontsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling FontsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/GraphicsDemo_85142ef6.o: ../../Source/Demos/GraphicsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling GraphicsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ImagesDemo_ecf13aa3.o: ../../Source/Demos/ImagesDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ImagesDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/JavaScript_d6e42eb5.o: ../../Source/Demos/JavaScript.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling JavaScript.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/KeyMappingsDemo_cf05c709.o: ../../Source/Demos/KeyMappingsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling KeyMappingsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/LiveConstantDemo_e3b18a9b.o: ../../Source/Demos/LiveConstantDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling LiveConstantDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/LookAndFeelDemo_25c558a3.o: ../../Source/Demos/LookAndFeelDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling LookAndFeelDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MDIDemo_c476c2d7.o: ../../Source/Demos/MDIDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MDIDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MidiDemo_75ebc30c.o: ../../Source/Demos/MidiDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MidiDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MultithreadingDemo_ff566eaa.o: ../../Source/Demos/MultithreadingDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MultithreadingDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/MultiTouch_595f3a2e.o: ../../Source/Demos/MultiTouch.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling MultiTouch.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/NetworkingDemo_fbd1b19f.o: ../../Source/Demos/NetworkingDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling NetworkingDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/OpenGLDemo_fdac55da.o: ../../Source/Demos/OpenGLDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling OpenGLDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/OpenGLDemo2D_48eeadec.o: ../../Source/Demos/OpenGLDemo2D.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling OpenGLDemo2D.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/PropertiesDemo_8ff1a8de.o: ../../Source/Demos/PropertiesDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling PropertiesDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/SystemInfoDemo_984acd28.o: ../../Source/Demos/SystemInfoDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling SystemInfoDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/TimersAndEventsDemo_f2656547.o: ../../Source/Demos/TimersAndEventsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling TimersAndEventsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/UnitTestsDemo_5c21762.o: ../../Source/Demos/UnitTestsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling UnitTestsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/ValueTreesDemo_9657d84f.o: ../../Source/Demos/ValueTreesDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling ValueTreesDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/VideoDemo_b55a1560.o: ../../Source/Demos/VideoDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling VideoDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/WebBrowserDemo_3df2dbdf.o: ../../Source/Demos/WebBrowserDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling WebBrowserDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/WidgetsDemo_fb0454.o: ../../Source/Demos/WidgetsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling WidgetsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/WindowsDemo_e8cfa428.o: ../../Source/Demos/WindowsDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling WindowsDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/XMLandJSONDemo_5bc88df3.o: ../../Source/Demos/XMLandJSONDemo.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling XMLandJSONDemo.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/BinaryData_ce4232d4.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling BinaryData.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_audio_basics_6b797ca1.o: ../../JuceLibraryCode/juce_audio_basics.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_audio_basics.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_audio_devices_a742c38b.o: ../../JuceLibraryCode/juce_audio_devices.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_audio_devices.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_audio_formats_5a29c68a.o: ../../JuceLibraryCode/juce_audio_formats.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_audio_formats.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_audio_processors_dea3173d.o: ../../JuceLibraryCode/juce_audio_processors.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_audio_processors.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_audio_utils_c7eb679f.o: ../../JuceLibraryCode/juce_audio_utils.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_audio_utils.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_box2d_95719d14.o: ../../JuceLibraryCode/juce_box2d.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_box2d.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_core_75b14332.o: ../../JuceLibraryCode/juce_core.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_core.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_cryptography_6de2ebff.o: ../../JuceLibraryCode/juce_cryptography.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_cryptography.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_data_structures_72d3da2c.o: ../../JuceLibraryCode/juce_data_structures.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_data_structures.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_events_d2be882c.o: ../../JuceLibraryCode/juce_events.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_events.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_graphics_9c18891e.o: ../../JuceLibraryCode/juce_graphics.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_graphics.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_gui_basics_8a6da59c.o: ../../JuceLibraryCode/juce_gui_basics.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_gui_basics.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_gui_extra_4a026f23.o: ../../JuceLibraryCode/juce_gui_extra.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_gui_extra.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_opengl_cd70b4c2.o: ../../JuceLibraryCode/juce_opengl.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_opengl.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

$(JUCE_OBJDIR)/juce_video_f128c512.o: ../../JuceLibraryCode/juce_video.cpp
	-@mkdir -p $(JUCE_OBJDIR)
	@echo "Compiling juce_video.cpp"
	@$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
