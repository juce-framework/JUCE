# Automatically generated makefile, created by the Jucer
# Don't edit this file! Your changes will be overwritten when you re-save the Jucer project!

ifndef CONFIG
  CONFIG=Debug
endif

DEPFLAGS := $(if $(word 2, $(TARGET_ARCH)), , -MMD)

ifeq ($(CONFIG),Debug)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Debug
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -g -ggdb -O0
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -mwindows -L"/usr/X11R6/lib/" -L"../../../../bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "DEBUG=1" -D "_DEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := HelloWorld
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

ifeq ($(CONFIG),Release)
  BINDIR := build
  LIBDIR := build
  OBJDIR := build/intermediate/Release
  OUTDIR := build
  CPPFLAGS := $(DEPFLAGS) -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  CFLAGS += $(CPPFLAGS) $(TARGET_ARCH) -Os
  CXXFLAGS += $(CFLAGS)
  LDFLAGS += -L$(BINDIR) -L$(LIBDIR) -mwindows -L"/usr/X11R6/lib/" -L"../../../../bin" -lfreetype -lpthread -lrt -lX11 -lGL -lGLU -lXinerama -lasound
  LDDEPS :=
  RESFLAGS :=  -D "LINUX=1" -D "NDEBUG=1" -I "/usr/include" -I "/usr/include/freetype2"
  TARGET := HelloWorld
  BLDCMD = $(CXX) -o $(OUTDIR)/$(TARGET) $(OBJECTS) $(LDFLAGS) $(RESOURCES) $(TARGET_ARCH)
endif

OBJECTS := \
  $(OBJDIR)/Main.o \
  $(OBJDIR)/MainComponent.o \
  $(OBJDIR)/BinaryData.o \
  $(OBJDIR)/JuceLibraryCode.o \

.PHONY: clean

$(OUTDIR)/$(TARGET): $(OBJECTS) $(LDDEPS) $(RESOURCES)
	@echo Linking HelloWorld
	-@mkdir -p $(BINDIR)
	-@mkdir -p $(LIBDIR)
	-@mkdir -p $(OUTDIR)
	@$(BLDCMD)

clean:
	@echo Cleaning HelloWorld
	-@rm -f $(OUTDIR)/$(TARGET)
	-@rm -rf $(OBJDIR)/*
	-@rm -rf $(OBJDIR)

$(OBJDIR)/Main.o: ../../Source/Main.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/MainComponent.o: ../../Source/MainComponent.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/BinaryData.o: ../../JuceLibraryCode/BinaryData.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

$(OBJDIR)/JuceLibraryCode.o: ../../JuceLibraryCode/JuceLibraryCode.cpp
	-@mkdir -p $(OBJDIR)
	@echo $(notdir $<)
	@$(CXX) $(CXXFLAGS) -o "$@" -c "$<"

-include $(OBJECTS:%.o=%.d)
