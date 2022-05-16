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

function(_juce_create_atomic_target target_name)
    add_library("${target_name}" INTERFACE)
    add_library("juce::${target_name}" ALIAS "${target_name}")

    if(NOT ((CMAKE_SYSTEM_NAME STREQUAL "Linux") OR (CMAKE_SYSTEM_NAME MATCHES ".*BSD")))
        return()
    endif()

    set(test_file_contents
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

    file(WRITE "${test_file_name}" "${test_file_contents}")

    try_compile(compile_result "${test_bindir}" "${test_file_name}"
        OUTPUT_VARIABLE test_build_output_0
        CXX_STANDARD 14
        CXX_STANDARD_REQUIRED TRUE
        CXX_EXTENSIONS FALSE)

    if(NOT compile_result)
        try_compile(compile_result "${test_bindir}" "${test_file_name}"
            OUTPUT_VARIABLE test_build_output_1
            LINK_LIBRARIES atomic
            CXX_STANDARD 14
            CXX_STANDARD_REQUIRED TRUE
            CXX_EXTENSIONS FALSE)

        if (NOT compile_result)
            message(FATAL_ERROR
                "First build output:\n"
                "${test_build_output_0}"
                "\n\nSecond build output:\n"
                "${test_build_output_1}"
                "\n\nJUCE requires support for std::atomic, but this system cannot "
                "successfully compile a program which uses std::atomic. "
                "You may need to install a dedicated libatomic package using your "
                "system's package manager.")
        endif()

        target_link_libraries("${target_name}" INTERFACE atomic)
    endif()

    file(REMOVE "${test_file_name}")
    file(REMOVE_RECURSE "${test_bindir}")
endfunction()

_juce_create_atomic_target(juce_atomic_wrapper)

