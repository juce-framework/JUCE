# Automatically generated makefile, created by the Projucer
# Don't edit this file! Your changes will be overwritten when you re-save the Projucer project!

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_ARM_MODE := arm
endif

LOCAL_MODULE := juce_jni
LOCAL_SRC_FILES := \
  ../../../Source/Main.cpp\
  ../../../Source/MainWindow.cpp\
  ../../../Source/IntroScreen.cpp\
  ../../../Source/Demos/AnimationDemo.cpp\
  ../../../Source/Demos/AudioLatencyDemo.cpp\
  ../../../Source/Demos/AudioPlaybackDemo.cpp\
  ../../../Source/Demos/AudioRecordingDemo.cpp\
  ../../../Source/Demos/AudioSettingsDemo.cpp\
  ../../../Source/Demos/AudioSynthesiserDemo.cpp\
  ../../../Source/Demos/Box2DDemo.cpp\
  ../../../Source/Demos/CameraDemo.cpp\
  ../../../Source/Demos/ChildProcessDemo.cpp\
  ../../../Source/Demos/CodeEditorDemo.cpp\
  ../../../Source/Demos/ComponentTransformsDemo.cpp\
  ../../../Source/Demos/CryptographyDemo.cpp\
  ../../../Source/Demos/DialogsDemo.cpp\
  ../../../Source/Demos/FlexBoxDemo.cpp\
  ../../../Source/Demos/FontsDemo.cpp\
  ../../../Source/Demos/GraphicsDemo.cpp\
  ../../../Source/Demos/ImagesDemo.cpp\
  ../../../Source/Demos/JavaScript.cpp\
  ../../../Source/Demos/KeyMappingsDemo.cpp\
  ../../../Source/Demos/LiveConstantDemo.cpp\
  ../../../Source/Demos/LookAndFeelDemo.cpp\
  ../../../Source/Demos/MDIDemo.cpp\
  ../../../Source/Demos/MidiDemo.cpp\
  ../../../Source/Demos/MultithreadingDemo.cpp\
  ../../../Source/Demos/MultiTouch.cpp\
  ../../../Source/Demos/NetworkingDemo.cpp\
  ../../../Source/Demos/OpenGLDemo.cpp\
  ../../../Source/Demos/OpenGLDemo2D.cpp\
  ../../../Source/Demos/PropertiesDemo.cpp\
  ../../../Source/Demos/SystemInfoDemo.cpp\
  ../../../Source/Demos/TimersAndEventsDemo.cpp\
  ../../../Source/Demos/UnitTestsDemo.cpp\
  ../../../Source/Demos/ValueTreesDemo.cpp\
  ../../../Source/Demos/VideoDemo.cpp\
  ../../../Source/Demos/WebBrowserDemo.cpp\
  ../../../Source/Demos/WidgetsDemo.cpp\
  ../../../Source/Demos/WindowsDemo.cpp\
  ../../../Source/Demos/XMLandJSONDemo.cpp\
  ../../../JuceLibraryCode/BinaryData.cpp\
  ../../../JuceLibraryCode/juce_audio_basics.cpp\
  ../../../JuceLibraryCode/juce_audio_devices.cpp\
  ../../../JuceLibraryCode/juce_audio_formats.cpp\
  ../../../JuceLibraryCode/juce_audio_processors.cpp\
  ../../../JuceLibraryCode/juce_audio_utils.cpp\
  ../../../JuceLibraryCode/juce_box2d.cpp\
  ../../../JuceLibraryCode/juce_core.cpp\
  ../../../JuceLibraryCode/juce_cryptography.cpp\
  ../../../JuceLibraryCode/juce_data_structures.cpp\
  ../../../JuceLibraryCode/juce_events.cpp\
  ../../../JuceLibraryCode/juce_graphics.cpp\
  ../../../JuceLibraryCode/juce_gui_basics.cpp\
  ../../../JuceLibraryCode/juce_gui_extra.cpp\
  ../../../JuceLibraryCode/juce_opengl.cpp\
  ../../../JuceLibraryCode/juce_video.cpp\

ifeq ($(NDK_DEBUG),1)
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -std=gnu++11 -DJUCE_ANDROID=1 -DJUCE_ANDROID_API_VERSION=23 -DJUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo -DJUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -DDEBUG=1 -D_DEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_ANDROID_7F0E4A25=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000
  LOCAL_LDLIBS := -llog -lGLESv2 -landroid -lEGL
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -std=gnu++11 -DJUCE_ANDROID=1 -DJUCE_ANDROID_API_VERSION=23 -DJUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo -DJUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -DDEBUG=1 -D_DEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_ANDROID_7F0E4A25=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000
  LOCAL_LDLIBS := -llog -lGLESv2 -landroid -lEGL
else
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -O3 -std=gnu++11 -DJUCE_ANDROID=1 -DJUCE_ANDROID_API_VERSION=23 -DJUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo -DJUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -DNDEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_ANDROID_7F0E4A25=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000
  LOCAL_LDLIBS := -llog -lGLESv2 -landroid -lEGL
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -O3 -std=gnu++11 -DJUCE_ANDROID=1 -DJUCE_ANDROID_API_VERSION=23 -DJUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo -DJUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -DNDEBUG=1 -DJUCE_UNIT_TESTS=1 -DJUCER_ANDROID_7F0E4A25=1 -DJUCE_APP_VERSION=3.0.0 -DJUCE_APP_VERSION_HEX=0x30000
  LOCAL_LDLIBS := -llog -lGLESv2 -landroid -lEGL
endif

include $(BUILD_SHARED_LIBRARY)
