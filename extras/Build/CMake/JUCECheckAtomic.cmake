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

function(_juce_create_atomic_target target_name)
    add_library("${target_name}" INTERFACE)
    add_library("juce::${target_name}" ALIAS "${target_name}")

    if(NOT ((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD")))
        return()
    endif()

    set(test_atomic_with_is_lock_free_file_contents
            [[
            #include <atomic>

            int main (int argc, char** argv)
            {
                std::atomic<long long> ll { static_cast<long long> (argc) };
                ll ^= static_cast<long long> (ll.is_lock_free());
                return static_cast<int> (ll);
            }
        ]])

    set(test_simple_atomic_file_contents
        [[
            #include <atomic>

            int main (int argc, char** argv)
            {
                std::atomic<long long> ll { static_cast<long long> (argc) };
                ll ^= 1;
                return static_cast<int> (ll);
            }
        ]])

    string(RANDOM LENGTH 16 random_file_string)
    set(test_file_name "${CMAKE_CURRENT_BINARY_DIR}/check_atomic_${random_file_string}.cpp")

    string(RANDOM LENGTH 16 random_dir_string)
    set(test_bindir "${CMAKE_CURRENT_BINARY_DIR}/check_atomic_dir_${random_dir_string}")

    file(WRITE "${test_file_name}" "${test_atomic_with_is_lock_free_file_contents}")

    try_compile(compile_result "${test_bindir}" "${test_file_name}"
        OUTPUT_VARIABLE test_build_output_0
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS FALSE)

    if(NOT compile_result)
        try_compile(compile_result "${test_bindir}" "${test_file_name}"
            OUTPUT_VARIABLE test_build_output_1
            LINK_LIBRARIES atomic
            CXX_STANDARD 17
            CXX_STANDARD_REQUIRED TRUE
            CXX_EXTENSIONS FALSE)

        if (NOT compile_result)
            file(WRITE "${test_file_name}" "${test_simple_atomic_file_contents}")

            try_compile(compile_result "${test_bindir}" "${test_file_name}"
                    OUTPUT_VARIABLE test_build_output_2
                    LINK_LIBRARIES atomic
                    CXX_STANDARD 17
                    CXX_STANDARD_REQUIRED TRUE
                    CXX_EXTENSIONS FALSE)

            if (NOT compile_result)
                message(FATAL_ERROR
                    "First build output:\n"
                    "${test_build_output_0}"
                    "\n\nSecond build output:\n"
                    "${test_build_output_1}"
                    "\n\nThird build output:\n"
                    "${test_build_output_2}"
                    "\n\nJUCE requires support for std::atomic, but this system cannot "
                    "successfully compile a program which uses std::atomic. "
                    "You may need to install a dedicated libatomic package using your "
                    "system's package manager.")
            else()
                message(WARNING
                    "First build output:\n"
                    "${test_build_output_0}"
                    "\n\nSecond build output:\n"
                    "${test_build_output_1}"
                    "\n\nIf you are seeing this warning it means that the libatomic library"
                    "on this system doesn't support is_lock_free."
                    "Please let the JUCE team know.")
            endif()
        endif()

        target_link_libraries("${target_name}" INTERFACE atomic)
    endif()

    file(REMOVE "${test_file_name}")
    file(REMOVE_RECURSE "${test_bindir}")
endfunction()

_juce_create_atomic_target(juce_atomic_wrapper)

