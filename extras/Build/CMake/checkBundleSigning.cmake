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

find_program(JUCE_XCRUN xcrun)

if(NOT JUCE_XCRUN)
    return()
endif()

execute_process(
    COMMAND "${JUCE_XCRUN}" codesign --verify "${src}"
    RESULT_VARIABLE result)

if(result)
    message(STATUS "Replacing invalid signature with ad-hoc signature")
    execute_process(COMMAND "${JUCE_XCRUN}" codesign -s - "${src}")
endif()
