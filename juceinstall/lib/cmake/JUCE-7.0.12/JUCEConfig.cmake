# ==============================================================================
#
#  This file is part of the JUCE library.
#  Copyright (c) 2020 - Raw Material Software Limited
#
#  JUCE is an open source library subject to commercial or open-source
#  licensing.
#
#  The code included in this file is provided under the terms of the ISC license
#  http://www.isc.org/downloads/software-support-policy/isc-license. Permission
#  To use, copy, modify, and/or distribute this software for any purpose with or
#  without fee is hereby granted provided that the above copyright notice and
#  this permission notice appear in all copies.
#
#  JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
#  EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
#  DISCLAIMED.
#
# ==============================================================================


####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was JUCEConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

include("${CMAKE_CURRENT_LIST_DIR}/LV2_HELPER.cmake")

if(NOT TARGET juce::juceaide)
    add_executable(juce::juceaide IMPORTED)
    set_target_properties(juce::juceaide PROPERTIES
        IMPORTED_LOCATION "${PACKAGE_PREFIX_DIR}/bin/JUCE-7.0.12/juceaide")
endif()

check_required_components("JUCE")

set(JUCE_MODULES_DIR "${PACKAGE_PREFIX_DIR}/include/JUCE-7.0.12/modules" CACHE INTERNAL
    "The path to JUCE modules")

include("${PACKAGE_PREFIX_DIR}/lib/cmake/JUCE-7.0.12/JUCEModuleSupport.cmake")
include("${PACKAGE_PREFIX_DIR}/lib/cmake/JUCE-7.0.12/JUCEUtils.cmake")

set(_juce_modules
    juce_analytics
    juce_audio_basics
    juce_audio_devices
    juce_audio_formats
    juce_audio_plugin_client
    juce_audio_processors
    juce_audio_utils
    juce_box2d
    juce_core
    juce_cryptography
    juce_data_structures
    juce_dsp
    juce_events
    juce_graphics
    juce_gui_basics
    juce_gui_extra
    juce_opengl
    juce_osc
    juce_product_unlocking
    juce_video)

set(_targets_defined)
set(_targets_expected)

foreach(_juce_module IN LISTS _juce_modules)
    list(APPEND _targets_expected ${_juce_module} juce::${_juce_modules})
    if(TARGET ${_juce_module})
        list(APPEND _targets_defined ${_juce_module})
    endif()

    if(TARGET juce::${_juce_module})
        list(APPEND _targets_defined juce::${_juce_module})
    endif()
endforeach()

if("${_targets_defined}" STREQUAL "${_targets_expected}")
    unset(_targets_defined)
    unset(_targets_expected)
    return()
endif()

if(NOT "${_targets_defined}" STREQUAL "")
    message(FATAL_ERROR "Some targets in this export set were already defined.")
endif()

unset(_targets_defined)
unset(_targets_expected)

foreach(_juce_module IN LISTS _juce_modules)
    juce_add_module("${PACKAGE_PREFIX_DIR}/include/JUCE-7.0.12/modules/${_juce_module}" ALIAS_NAMESPACE juce)
endforeach()

unset(_juce_modules)
