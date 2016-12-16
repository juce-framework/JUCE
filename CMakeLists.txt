# JUCE

list(APPEND sources
    "modules/juce_core/juce_core.cpp"
    "modules/juce_core/juce_core.h"
    "modules/juce_cryptography/juce_cryptography.cpp"
    "modules/juce_cryptography/juce_cryptography.h"
    "modules/juce_data_structures/juce_data_structures.cpp"
    "modules/juce_data_structures/juce_data_structures.h"
    "modules/juce_events/juce_events.cpp"
    "modules/juce_events/juce_events.h"
    "modules/juce_graphics/juce_graphics.cpp"
    "modules/juce_graphics/juce_graphics.h"
    "modules/juce_gui_basics/juce_gui_basics.cpp"
    "modules/juce_gui_basics/juce_gui_basics.h"
    "modules/juce_gui_extra/juce_gui_extra.cpp"
    "modules/juce_gui_extra/juce_gui_extra.h"
    "modules/juce_opengl/juce_opengl.cpp"
    "modules/juce_opengl/juce_opengl.h"
    "JuceHeader.h"
)

add_library(juce STATIC ${sources})

target_link_libraries(juce PUBLIC zlibstatic)

target_compile_definitions(juce PUBLIC
    _USE_MATH_DEFINES
    JUCE_GLOBAL_MODULE_SETTINGS_INCLUDED
    JUCE_WINDOWS
    JUCE_STANDALONE_APPLICATION
    JUCE_INCLUDE_ZLIB_CODE=0
)

target_include_directories(juce PUBLIC
    .
    modules
)
