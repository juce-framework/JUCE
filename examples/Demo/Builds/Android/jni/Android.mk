# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

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
  ../../../../../modules/juce_audio_basics/juce_audio_basics.cpp\
  ../../../../../modules/juce_audio_devices/juce_audio_devices.cpp\
  ../../../../../modules/juce_audio_formats/juce_audio_formats.cpp\
  ../../../../../modules/juce_audio_processors/juce_audio_processors.cpp\
  ../../../../../modules/juce_audio_utils/juce_audio_utils.cpp\
  ../../../../../modules/juce_box2d/juce_box2d.cpp\
  ../../../../../modules/juce_core/juce_core.cpp\
  ../../../../../modules/juce_cryptography/juce_cryptography.cpp\
  ../../../../../modules/juce_data_structures/juce_data_structures.cpp\
  ../../../../../modules/juce_events/juce_events.cpp\
  ../../../../../modules/juce_graphics/juce_graphics.cpp\
  ../../../../../modules/juce_gui_basics/juce_gui_basics.cpp\
  ../../../../../modules/juce_gui_extra/juce_gui_extra.cpp\
  ../../../../../modules/juce_opengl/juce_opengl.cpp\
  ../../../../../modules/juce_video/juce_video.cpp\

ifeq ($(NDK_DEBUG),1)
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -std=c++11 -std=gnu++11 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=9" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1" -D "JUCE_APP_VERSION=3.0.0" -D "JUCE_APP_VERSION_HEX=0x30000"
  LOCAL_LDLIBS := -llog -lGLESv2
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -std=c++11 -std=gnu++11 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=9" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1" -D "JUCE_APP_VERSION=3.0.0" -D "JUCE_APP_VERSION_HEX=0x30000"
  LOCAL_LDLIBS := -llog -lGLESv2
else
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -O3 -std=c++11 -std=gnu++11 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=9" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1" -D "JUCE_APP_VERSION=3.0.0" -D "JUCE_APP_VERSION_HEX=0x30000"
  LOCAL_LDLIBS := -llog -lGLESv2
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -O3 -std=c++11 -std=gnu++11 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=9" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_jucedemo_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/jucedemo/JuceDemo\" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1" -D "JUCE_APP_VERSION=3.0.0" -D "JUCE_APP_VERSION_HEX=0x30000"
  LOCAL_LDLIBS := -llog -lGLESv2
endif

include $(BUILD_SHARED_LIBRARY)
