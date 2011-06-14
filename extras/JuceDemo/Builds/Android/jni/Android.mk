# Automatically generated makefile, created by the Jucer
# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_CPP_EXTENSION := cpp
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
  ../../../JuceLibraryCode/JuceLibraryCode1.cpp\
  ../../../JuceLibraryCode/JuceLibraryCode2.cpp\
  ../../../JuceLibraryCode/JuceLibraryCode3.cpp\
  ../../../JuceLibraryCode/JuceLibraryCode4.cpp\

ifeq ($(CONFIG),Debug)
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -g -O0 -D "JUCE_ANDROID=1" -D "DEBUG=1" -D "_DEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
else
  LOCAL_CPPFLAGS += -fsigned-char -fexceptions -frtti -Os -D "JUCE_ANDROID=1" -D "NDEBUG=1" -D "JUCE_UNIT_TESTS=1" -D "JUCER_ANDROID_7F0E4A25=1"
endif

include $(BUILD_SHARED_LIBRARY)
