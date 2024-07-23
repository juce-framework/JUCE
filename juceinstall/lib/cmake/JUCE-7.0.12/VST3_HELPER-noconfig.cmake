#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "juce::juce_vst3_helper" for configuration ""
set_property(TARGET juce::juce_vst3_helper APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(juce::juce_vst3_helper PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/bin/JUCE-7.0.12/juce_vst3_helper"
  )

list(APPEND _cmake_import_check_targets juce::juce_vst3_helper )
list(APPEND _cmake_import_check_files_for_juce::juce_vst3_helper "${_IMPORT_PREFIX}/bin/JUCE-7.0.12/juce_vst3_helper" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
