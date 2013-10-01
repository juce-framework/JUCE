# Automatically generated makefile, created by the Introjucer
# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm
LOCAL_MODULE := juce_jni
LOCAL_SRC_FILES := \
  ../../../Source/ApplicationStartup.cpp\
  ../../../Source/MainDemoWindow.cpp\
  ../../../Source/demos/AudioDemoLatencyPage.cpp\
  ../../../Source/demos/AudioDemoPlaybackPage.cpp\
  ../../../Source/demos/AudioDemoRecordPage.cpp\
  ../../../Source/demos/AudioDemoSetupPage.cpp\
  ../../../Source/demos/AudioDemoSynthPage.cpp\
  ../../../Source/demos/AudioDemoTabComponent.cpp\
  ../../../Source/demos/CameraDemo.cpp\
  ../../../Source/demos/CodeEditorDemo.cpp\
  ../../../Source/demos/DirectShowDemo.cpp\
  ../../../Source/demos/DragAndDropDemo.cpp\
  ../../../Source/demos/FontsAndTextDemo.cpp\
  ../../../Source/demos/InterprocessCommsDemo.cpp\
  ../../../Source/demos/OpenGLDemo.cpp\
  ../../../Source/demos/QuickTimeDemo.cpp\
  ../../../Source/demos/RenderingTestComponent.cpp\
  ../../../Source/demos/TableDemo.cpp\
  ../../../Source/demos/ThreadingDemo.cpp\
  ../../../Source/demos/TreeViewDemo.cpp\
  ../../../Source/demos/WebBrowserDemo.cpp\
  ../../../Source/demos/WidgetsDemo.cpp\
  ../../../JuceLibraryCode/BinaryData.cpp\
  ../../../../../modules/juce_audio_basics/juce_audio_basics.cpp\
  ../../../../../modules/juce_audio_devices/juce_audio_devices.cpp\
  ../../../../../modules/juce_audio_formats/juce_audio_formats.cpp\
  ../../../../../modules/juce_audio_processors/juce_audio_processors.cpp\
  ../../../../../modules/juce_audio_utils/juce_audio_utils.cpp\
  ../../../../../modules/juce_core/juce_core.cpp\
  ../../../../../modules/juce_cryptography/juce_cryptography.cpp\
  ../../../../../modules/juce_data_structures/juce_data_structures.cpp\
  ../../../../../modules/juce_events/juce_events.cpp\
  ../../../../../modules/juce_graphics/juce_graphics.cpp\
  ../../../../../modules/juce_gui_basics/juce_gui_basics.cpp\
  ../../../../../modules/juce_gui_extra/juce_gui_extra.cpp\
  ../../../../../modules/juce_opengl/juce_opengl.cpp\
  ../../../../../modules/juce_video/juce_video.cpp\

ifeq ($(CONFIG),Debug)
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=8" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/JuceDemo\" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
  LOCAL_LDLIBS := -llog -lGLESv2
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -g -I "../../JuceLibraryCode" -I "../../../../modules" -O0 -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=8" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/JuceDemo\" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
  LOCAL_LDLIBS := -llog -lGLESv2
else
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -Os -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=8" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/JuceDemo\" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
  LOCAL_LDLIBS := -llog -lGLESv2
  LOCAL_CFLAGS += -fsigned-char -fexceptions -frtti -I "../../JuceLibraryCode" -I "../../../../modules" -Os -D "JUCE_ANDROID=1" -D "JUCE_ANDROID_API_VERSION=8" -D "JUCE_ANDROID_ACTIVITY_CLASSNAME=com_juce_JuceDemo" -D JUCE_ANDROID_ACTIVITY_CLASSPATH=\"com/juce/JuceDemo\" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
  LOCAL_LDLIBS := -llog -lGLESv2
endif

include $(BUILD_SHARED_LIBRARY)
