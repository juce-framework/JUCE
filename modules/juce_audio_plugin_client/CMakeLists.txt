# ==============================================================================
#
#  This file is part of the JUCE 7 technical preview.
#  Copyright (c) 2022 - Raw Material Software Limited
#
#  You may use this code under the terms of the GPL v3
#  (see www.gnu.org/licenses).
#
#  For the technical preview this file cannot be licensed commercially.
#
#  JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
#  EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
#  DISCLAIMED.
#
# ==============================================================================

add_executable(juce_lv2_helper LV2/juce_LV2TurtleDumpProgram.cpp)
add_executable(juce::juce_lv2_helper ALIAS juce_lv2_helper)
target_compile_features(juce_lv2_helper PRIVATE cxx_std_14)
set_target_properties(juce_lv2_helper PROPERTIES BUILD_WITH_INSTALL_RPATH ON)
target_link_libraries(juce_lv2_helper PRIVATE ${CMAKE_DL_LIBS})
install(TARGETS juce_lv2_helper EXPORT LV2_HELPER DESTINATION "bin/JUCE-${JUCE_VERSION}")
