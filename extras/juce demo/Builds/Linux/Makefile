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
  TARGET := JuceDemo
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
  TARGET := JuceDemo
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/ApplicationStartup.o \
  $(OBJDIR)/MainDemoWindow.o \
  $(OBJDIR)/AudioDemoLatencyPage.o \
  $(OBJDIR)/AudioDemoPlaybackPage.o \
  $(OBJDIR)/AudioDemoRecordPage.o \
  $(OBJDIR)/AudioDemoSetupPage.o \
  $(OBJDIR)/AudioDemoSynthPage.o \
  $(OBJDIR)/AudioDemoTabComponent.o \
  $(OBJDIR)/CameraDemo.o \
  $(OBJDIR)/CodeEditorDemo.o \
  $(OBJDIR)/DragAndDropDemo.o \
  $(OBJDIR)/FontsAndTextDemo.o \
  $(OBJDIR)/InterprocessCommsDemo.o \
  $(OBJDIR)/OpenGLDemo.o \
  $(OBJDIR)/QuickTimeDemo.o \
  $(OBJDIR)/RenderingTestComponent.o \
  $(OBJDIR)/TableDemo.o \
  $(OBJDIR)/ThreadingDemo.o \
  $(OBJDIR)/TreeViewDemo.o \
  $(OBJDIR)/WebBrowserDemo.o \
  $(OBJDIR)/WidgetsDemo.o \
  $(OBJDIR)/BinaryData.o \
  $(OBJDIR)/JuceLibraryCode1.o \
  $(OBJDIR)/JuceLibraryCode2.o \
  $(OBJDIR)/JuceLibraryCode3.o \
  $(OBJDIR)/JuceLibraryCode4.o \

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

$(OBJDIR)/ApplicationStartup.o: ../../Source/ApplicationStartup.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ApplicationStartup.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainDemoWindow.o: ../../Source/MainDemoWindow.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling MainDemoWindow.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoLatencyPage.o: ../../Source/demos/AudioDemoLatencyPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoLatencyPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoPlaybackPage.o: ../../Source/demos/AudioDemoPlaybackPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoPlaybackPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoRecordPage.o: ../../Source/demos/AudioDemoRecordPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoRecordPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoSetupPage.o: ../../Source/demos/AudioDemoSetupPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoSetupPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoSynthPage.o: ../../Source/demos/AudioDemoSynthPage.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoSynthPage.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/AudioDemoTabComponent.o: ../../Source/demos/AudioDemoTabComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling AudioDemoTabComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CameraDemo.o: ../../Source/demos/CameraDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CameraDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/CodeEditorDemo.o: ../../Source/demos/CodeEditorDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling CodeEditorDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/DragAndDropDemo.o: ../../Source/demos/DragAndDropDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling DragAndDropDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/FontsAndTextDemo.o: ../../Source/demos/FontsAndTextDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling FontsAndTextDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/InterprocessCommsDemo.o: ../../Source/demos/InterprocessCommsDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling InterprocessCommsDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/OpenGLDemo.o: ../../Source/demos/OpenGLDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling OpenGLDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/QuickTimeDemo.o: ../../Source/demos/QuickTimeDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling QuickTimeDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/RenderingTestComponent.o: ../../Source/demos/RenderingTestComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling RenderingTestComponent.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TableDemo.o: ../../Source/demos/TableDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TableDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/ThreadingDemo.o: ../../Source/demos/ThreadingDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling ThreadingDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/TreeViewDemo.o: ../../Source/demos/TreeViewDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling TreeViewDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WebBrowserDemo.o: ../../Source/demos/WebBrowserDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WebBrowserDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/WidgetsDemo.o: ../../Source/demos/WidgetsDemo.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling WidgetsDemo.cpp"
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo "Compiling BinaryData.cpp"
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
