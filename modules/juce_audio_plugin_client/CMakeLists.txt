# ==============================================================================
#
#  This file is part of the JUCE library.
#  Copyright (c) 2022 - Raw Material Software Limited
#
#  JUCE is an open source library subject to commercial or open-source
#  licensing.
#
#  By using JUCE, you agree to the terms of both the JUCE 7 End-User License
#  Agreement and JUCE Privacy Policy.
#
#  End User License Agreement: www.juce.com/juce-7-licence
#  Privacy Policy: www.juce.com/juce-privacy-policy
#
#  Or: You may also use this code under the terms of the GPL v3 (see
#  www.gnu.org/licenses).
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
