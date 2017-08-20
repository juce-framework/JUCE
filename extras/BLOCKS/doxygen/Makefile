SHELL := /bin/bash

INCLUDED_MODULES := juce_audio_basics,juce_audio_devices,juce_blocks_basics,juce_core,juce_events

SOURCE_FILES := $(shell find ../../../modules -type f -name "juce_*.h" -or -name "juce_*.dox"| sed 's/ /\\ /g')
EXAMPLE_DIRS := ../standalone_sdk/examples ../../../examples/BLOCKS
EXAMPLE_SOURCE_FILES := $(foreach DIR,$(EXAMPLE_DIRS),$(shell find $(DIR) -type f -name "*.h" -or -name "*.cpp" | sed 's/ /\\ /g'))
DOCUMENTATION_FILES := $(shell find pages -type f -name "*.dox" | sed 's/ /\\ /g')
IMAGES := $(shell find images -type f | sed 's/ /\\ /g')

.PHONEY: clean

doc/index.html: build/Doxyfile DoxygenLayout.xml footer.html header.html stylesheet.css $(DOCUMENTATION_FILES) $(EXAMPLE_SOURCE_FILES) $(IMAGES)
	doxygen $<

build/Doxyfile: ../../../doxygen/Doxyfile Doxyfile build/juce_modules.dox
	cat ../../../doxygen/Doxyfile Doxyfile > $@

build/juce_modules.dox: ../../../doxygen/process_source_files.py $(SOURCE_FILES)
	rm -rf build
	python $< ../../../modules build --subdirs=$(INCLUDED_MODULES)

clean:
	rm -rf build doc
