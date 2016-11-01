SHELL := /bin/bash

SOURCE_FILES := $(shell find ../../../modules -type f -name "juce_*.h" | sed 's/ /\\ /g')
EXAMPLE_DIRS := ../standalone_sdk/examples ../../../examples/BLOCKS
EXAMPLE_SOURCE_FILES := $(foreach DIR,$(EXAMPLE_DIRS),$(shell find $(DIR) -type f -name "*.h" -or -name "*.cpp" | sed 's/ /\\ /g'))
DOCUMENTATION_FILES := $(shell find pages -type f -name "*.dox" | sed 's/ /\\ /g')
IMAGES := $(shell find images -type f | sed 's/ /\\ /g')

.PHONEY: clean

doc/index.html: build/juce_modules.dox Doxyfile DoxygenLayout.xml footer.html header.html stylesheet.css $(DOCUMENTATION_FILES) $(EXAMPLE_SOURCE_FILES) $(IMAGES)
	doxygen

build/juce_modules.dox: process_source_files.py $(SOURCE_FILES)
	python $<

clean:
	rm -rf build doc
