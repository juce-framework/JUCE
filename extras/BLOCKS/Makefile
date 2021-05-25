# Execute this Makefile in an empty directory to create the contents of the
# standalone SDK repository.

ROOT_DIR := $(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))

JUCE_MODULES := juce_audio_basics juce_audio_devices juce_blocks_basics juce_core juce_events
SDK_JUCE_MODULES := $(addprefix SDK/,$(JUCE_MODULES))

FILES := $(wildcard $(ROOT_DIR)/standalone_sdk/*)
SDK_FILES := $(notdir $(FILES))

all: $(SDK_FILES) $(SDK_JUCE_MODULES)

# Create $(SDK_JUCE_MODULES)
SDK/%: $(ROOT_DIR)/../../modules/%
	rm -rf $@
	cp -r $< $@

$(SDK_JUCE_MODULES): $(SDK_FILES)

# Create $(SDK_FILES)
%: $(ROOT_DIR)/standalone_sdk/%
	rm -rf $@
	cp -r $< $@
