# ==============================================================================
#
#  This file is part of the JUCE framework.
#  Copyright (c) Raw Material Software Limited
#
#  JUCE is an open source framework subject to commercial or open source
#  licensing.
#
#  By downloading, installing, or using the JUCE framework, or combining the
#  JUCE framework with any other source code, object code, content or any other
#  copyrightable work, you agree to the terms of the JUCE End User Licence
#  Agreement, and all incorporated terms including the JUCE Privacy Policy and
#  the JUCE Website Terms of Service, as applicable, which will bind you. If you
#  do not agree to the terms of these agreements, we will not license the JUCE
#  framework to you, and you must discontinue the installation or download
#  process and cease use of the JUCE framework.
#
#  JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
#  JUCE Privacy Policy: https://juce.com/juce-privacy-policy
#  JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/
#
#  Or:
#
#  You may also use this code under the terms of the AGPLv3:
#  https://www.gnu.org/licenses/agpl-3.0.en.html
#
#  THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
#  WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
#  MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.
#
# ==============================================================================

find_program(JUCE_XCRUN xcrun)

if(NOT JUCE_XCRUN)
    message(WARNING "No xcrun was found. Can't generate Icon Composer assets. Please ensure that the"
                    " Xcode command-line tools are installed.")
    return()
endif()

function(get_actool_version output_var)
    execute_process(
        COMMAND xcrun actool --version
        OUTPUT_VARIABLE cmd_output
        ERROR_VARIABLE cmd_error
        RESULT_VARIABLE cmd_result
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    if(NOT cmd_result EQUAL 0)
        set(${output_var} "" PARENT_SCOPE)
        return()
    endif()

    string(REGEX REPLACE "[ \t\r\n]+" "" cmd_output "${cmd_output}")
    string(REGEX MATCH "<key>short-bundle-version</key>[^<]*<string>(.*)</string>" match_found "${cmd_output}")

    if(match_found)
        set(${output_var} "${CMAKE_MATCH_1}" PARENT_SCOPE)
    else()
        set(${output_var} "" PARENT_SCOPE)
    endif()
endfunction()

get_actool_version(actool_version)

if(NOT "${actool_version}" VERSION_GREATER_EQUAL "26.0")
    message(WARNING "Couldn't find actool with version 26.0 or greater."
                    " Icon Composer assets will not be generated.")
    return()
endif()

get_filename_component(icon_name "${icon_path}" NAME_WE)

execute_process(
    COMMAND "${JUCE_XCRUN}" actool
            --compile "${bundle_dir}/Contents/Resources"
            --platform macosx
            --minimum-deployment-target 10.11
            --output-partial-info-plist /dev/null
            --app-icon "${icon_name}"
            "${icon_path}"
    OUTPUT_VARIABLE result_stdout
    ERROR_VARIABLE result_stderr
    RESULT_VARIABLE result)

if(result)
    message(STATUS "${result_stdout}")
    message(WARNING "${result_stderr}")
    message(ERROR "Failed to generate Icon Composer asset. actool returned with exit code ${result}")
endif()
