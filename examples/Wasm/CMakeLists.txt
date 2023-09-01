     set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
     set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

     SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -sAUDIO_WORKLET -sWASM_WORKERS")
juce_add_console_app(WASMDemo
        BUNDLE_ID                       com.juce.wasmdemo
        ICON_BIG                        ${CMAKE_CURRENT_SOURCE_DIR}/Source/JUCEAppIcon.png
        NEEDS_CURL                      TRUE
        NEEDS_WEB_BROWSER               TRUE
        MICROPHONE_PERMISSION_ENABLED   TRUE
        CAMERA_PERMISSION_ENABLED       TRUE
        BLUETOOTH_PERMISSION_ENABLED    TRUE
        FILE_SHARING_ENABLED            TRUE
        DOCUMENT_BROWSER_ENABLED        TRUE
        REQUIRES_FULL_SCREEN            FALSE
        IPAD_SCREEN_ORIENTATIONS        UIInterfaceOrientationPortrait
        UIInterfaceOrientationPortraitUpsideDown
        UIInterfaceOrientationLandscapeLeft
        UIInterfaceOrientationLandscapeRight
        IPHONE_SCREEN_ORIENTATIONS      UIInterfaceOrientationPortrait
        UIInterfaceOrientationPortraitUpsideDown
        UIInterfaceOrientationLandscapeLeft
        UIInterfaceOrientationLandscapeRight)

juce_generate_juce_header(WASMDemo)

target_sources(WASMDemo PRIVATE
        Source/Main.cpp
)

target_compile_definitions(WASMDemo PRIVATE
        PIP_JUCE_EXAMPLES_DIRECTORY_STRING="${JUCE_SOURCE_DIR}/examples"
        JUCE_ALLOW_STATIC_NULL_VARIABLES=0
        JUCE_CONTENT_SHARING=0
        JUCE_DEMO_RUNNER=0
        JUCE_PLUGINHOST_LV2=0
        JUCE_PLUGINHOST_VST3=0
        JUCE_STRICT_REFCOUNTEDPOINTER=1
        JUCE_UNIT_TESTS=0
        JUCE_USE_CAMERA=0
        JUCE_USE_CURL=0
        JUCE_USE_MP3AUDIOFORMAT=1
        JUCE_WEB_BROWSER=0)

find_package(OpenAL REQUIRED)
target_link_libraries(WASMDemo PRIVATE
        juce::juce_core
        juce::juce_audio_devices
        juce::juce_audio_basics
        juce::juce_dsp
      )