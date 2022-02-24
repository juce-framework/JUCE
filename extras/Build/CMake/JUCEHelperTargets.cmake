add_library(juce_recommended_warning_flags INTERFACE)
add_library(juce::juce_recommended_warning_flags ALIAS juce_recommended_warning_flags)

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    target_compile_options(juce_recommended_warning_flags INTERFACE "/W4")
elseif((CMAKE_CXX_COMPILER_ID STREQUAL "Clang") OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang"))
    target_compile_options(juce_recommended_warning_flags INTERFACE
        -Wall -Wshadow-all -Wshorten-64-to-32 -Wstrict-aliasing
        -Wuninitialized -Wunused-parameter -Wconversion -Wsign-compare
        -Wint-conversion -Wconditional-uninitialized -Wconstant-conversion
        -Wsign-conversion -Wbool-conversion -Wextra-semi -Wunreachable-code
        -Wcast-align -Wshift-sign-overflow -Wno-missing-field-initializers
        -Wnullable-to-nonnull-conversion -Wno-ignored-qualifiers -Wswitch-enum
        -Wpedantic -Wdeprecated
        $<$<OR:$<COMPILE_LANGUAGE:CXX>,$<COMPILE_LANGUAGE:OBJCXX>>:
            -Wzero-as-null-pointer-constant -Wunused-private-field
            -Woverloaded-virtual -Wreorder
            -Winconsistent-missing-destructor-override>)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(juce_recommended_warning_flags INTERFACE
        -Wall -Wextra -Wpedantic -Wstrict-aliasing -Wuninitialized
        -Wunused-parameter -Wsign-compare -Wsign-conversion -Wunreachable-code
        -Wcast-align -Wno-implicit-fallthrough -Wno-maybe-uninitialized
        -Wno-missing-field-initializers -Wno-ignored-qualifiers -Wswitch-enum
        -Wredundant-decls -Wno-strict-overflow -Wshadow
        $<$<COMPILE_LANGUAGE:CXX>:
            -Woverloaded-virtual -Wreorder -Wzero-as-null-pointer-constant>)
endif()

# ==================================================================================================

add_library(juce_recommended_config_flags INTERFACE)
add_library(juce::juce_recommended_config_flags ALIAS juce_recommended_config_flags)

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    target_compile_options(juce_recommended_config_flags INTERFACE
        $<IF:$<CONFIG:Debug>,/Od /Zi,/Ox> $<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">:/MP> /EHsc)
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

if((CMAKE_CXX_COMPILER_ID STREQUAL "MSVC") OR (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC"))
    target_compile_options(juce_recommended_lto_flags INTERFACE
        $<$<CONFIG:Release>:$<IF:$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">,-GL,-flto>>)
    target_link_libraries(juce_recommended_lto_flags INTERFACE
        $<$<CONFIG:Release>:$<$<STREQUAL:"${CMAKE_CXX_COMPILER_ID}","MSVC">:-LTCG>>)
elseif((NOT MINGW) AND ((CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
                     OR (CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
                     OR (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")))
    target_compile_options(juce_recommended_lto_flags INTERFACE $<$<CONFIG:Release>:-flto>)
    target_link_libraries(juce_recommended_lto_flags INTERFACE $<$<CONFIG:Release>:-flto>)
endif()
