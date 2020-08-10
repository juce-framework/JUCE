add_library(juce_recommended_warning_flags INTERFACE)
add_library(juce::juce_recommended_warning_flags ALIAS juce_recommended_warning_flags)

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
    target_compile_options(juce_recommended_warning_flags INTERFACE "/W4")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    target_compile_options(juce_recommended_warning_flags INTERFACE
        -Wall -Wshadow-all -Wshorten-64-to-32 -Wstrict-aliasing -Wuninitialized
        -Wunused-parameter -Wconversion -Wsign-compare -Wint-conversion
        -Wconditional-uninitialized -Woverloaded-virtual -Wreorder
        -Wconstant-conversion -Wsign-conversion -Wunused-private-field
        -Wbool-conversion -Wextra-semi -Wunreachable-code
        -Wzero-as-null-pointer-constant -Wcast-align
        -Winconsistent-missing-destructor-override -Wshift-sign-overflow
        -Wnullable-to-nonnull-conversion -Wno-missing-field-initializers
        -Wno-ignored-qualifiers -Wswitch-enum -Wpedantic)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(juce_recommended_warning_flags INTERFACE
        -Wall -Wextra -Wstrict-aliasing -Wuninitialized -Wunused-parameter
        -Wsign-compare -Woverloaded-virtual -Wreorder -Wsign-conversion
        -Wunreachable-code -Wzero-as-null-pointer-constant -Wcast-align
        -Wno-implicit-fallthrough -Wno-maybe-uninitialized
        -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wswitch-enum
        -Wredundant-decls -Wpedantic)

    if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER "7.0.0")
        target_compile_options(juce_recommended_warning_flags INTERFACE "-Wno-strict-overflow")
    endif()
endif()

# ==================================================================================================

add_library(juce_recommended_config_flags INTERFACE)
add_library(juce::juce_recommended_config_flags ALIAS juce_recommended_config_flags)

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
    target_compile_options(juce_recommended_config_flags INTERFACE
        $<IF:$<CONFIG:Debug>,/Od,/Ox> $<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">:/MP> /EHsc)
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
       OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
       OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    target_compile_options(juce_recommended_config_flags INTERFACE
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3>)
endif()

# ==================================================================================================

add_library(juce_recommended_lto_flags INTERFACE)
add_library(juce::juce_recommended_lto_flags ALIAS juce_recommended_lto_flags)

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC"))
    target_compile_options(juce_recommended_lto_flags INTERFACE
        $<$<CONFIG:Release>:$<IF:$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">,-GL,-flto>>)
    target_link_libraries(juce_recommended_lto_flags INTERFACE
        $<$<CONFIG:Release>:$<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">:-LTCG>>)
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
       OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
       OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU"))
    target_compile_options(juce_recommended_lto_flags INTERFACE $<$<CONFIG:Release>:-flto>)
    target_link_libraries(juce_recommended_lto_flags INTERFACE $<$<CONFIG:Release>:-flto>)
endif()
