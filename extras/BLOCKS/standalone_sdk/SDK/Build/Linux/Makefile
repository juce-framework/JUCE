ifndef AR
    AR := ar
endif

ifndef CONFIG
    CONFIG := Debug
endif

TARGET := libBLOCKS-SDK.a

JUCE_OUTDIR := $(CONFIG)
JUCE_OBJDIR := build/$(CONFIG)

JUCE_INCLUDES := -I../..
JUCE_SDKDEFINES := -DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_STANDALONE_APPLICATION=1
JUCE_CXXFLAGS = -std=c++11 $(DEPFLAGS) -march=native -DLINUX=1 $(JUCE_SDKDEFINES) $(shell pkg-config --cflags alsa freetype2 libcurl x11 xext xinerama) -pthread $(JUCE_INCLUDES)

ifeq ($(CONFIG),Debug)
  JUCE_CXXFLAGS += -DDEBUG=1 -D_DEBUG=1 -g -ggdb -O0
endif

ifeq ($(CONFIG),Release)
  JUCE_CXXFLAGS += -DNDEBUG=1 -Os
endif

JUCE_MODULES := juce_audio_basics juce_audio_devices juce_blocks_basics juce_core juce_events
JUCE_SOURCE := $(foreach MODULE_NAME,$(JUCE_MODULES),../../$(MODULE_NAME)/$(MODULE_NAME).cpp)
JUCE_OBJECTS := $(foreach MODULE_NAME,$(JUCE_MODULES),$(JUCE_OBJDIR)/$(MODULE_NAME)/$(MODULE_NAME).o)

.PHONY: clean

$(JUCE_OUTDIR)/$(TARGET): check-pkg-config $(JUCE_OBJECTS)
	@mkdir -p $(JUCE_OUTDIR)
	$(AR) -cr $@ $(JUCE_OBJECTS)

check-pkg-config:
	@command -v pkg-config >/dev/null 2>&1 || { echo >&2 "pkg-config not installed. Please, install it."; exit 1; }
	@pkg-config --print-errors alsa freetype2 libcurl x11 xext xinerama

clean:
	rm -rf $(JUCE_OUTDIR)/$(TARGET) $(JUCE_OBJDIR)

$(JUCE_OBJDIR)/%.o: ../../%.cpp
	-@mkdir -p $(dir $@)
	$(CXX) $(JUCE_CXXFLAGS) -o "$@" -c "$<"

