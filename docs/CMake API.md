# The JUCE CMake API

## System Requirements

- All project types require CMake 3.15 or higher.
- Android targets are not currently supported.
- WebView2 on Windows via JUCE_USE_WIN_WEBVIEW2 flag in juce_gui_extra is not currently supported.

Most system package managers have packages for CMake, but we recommend using the most recent release
from https://cmake.org/download. You should always use a CMake that's newer than your build
toolchain, so that CMake can identify your build tools and understand how to invoke them.

In addition to CMake you'll need a build toolchain for your platform, such as Xcode or MSVC.

## Getting Started

### Using `add_subdirectory`

The simplest way to include JUCE in your project is to add JUCE as a
subdirectory of your project, and to include the line `add_subdirectory(JUCE)`
in your project CMakeLists.txt. This will make the JUCE targets and helper
functions available for use by your custom targets.

### Using `find_package`

To install JUCE globally on your system, you'll need to tell CMake where to
place the installed files.

    # Go to JUCE directory
    cd /path/to/clone/JUCE
    # Configure build with library components only
    cmake -B cmake-build-install -DCMAKE_INSTALL_PREFIX=/path/to/JUCE/install
    # Run the installation
    cmake --build cmake-build-install --target install

In your project which consumes JUCE, make sure the project CMakeLists.txt contains the line
`find_package(JUCE CONFIG REQUIRED)`. This will make the JUCE modules and CMake helper functions
available for use in the rest of your build. Then, run the build like so:

    # Go to project directory
    cd /path/to/my/project
    # Configure build, passing the JUCE install path you used earlier
    cmake -B cmake-build -DCMAKE_PREFIX_PATH=/path/to/JUCE/install
    # Build the project
    cmake --build cmake-build

### Example projects

In the JUCE/examples/CMake directory, you'll find example projects for a GUI app, a console app,
and an audio plugin. You can simply copy one of these subdirectories out of the JUCE repo, add JUCE
as a submodule, and uncomment the call to `add_subdirectory` where indicated in the CMakeLists.txt.
Alternatively, if you've installed JUCE using a package manager or the CMake install target, you can
uncomment the call to `find_package`.

Once your project is set up, you can generate a build tree for it in the normal way. To get started,
you might invoke CMake like this, from the new directory you created.

    cmake -Bbuild (-GgeneratorName) (-DJUCE_BUILD_EXTRAS=ON) (-DJUCE_BUILD_EXAMPLES=ON)

This will create a build tree in a directory named 'build', using the CMakeLists in the current
working directory, using the default generator (makefiles on mac/linux, and the most recent Visual
Studio on Windows). You can choose a specific generator to use with the `-G` flag (call `cmake -G`
to see a full list of generators on your platform). If you included JUCE as a subdirectory, you can
enable the Extras and Examples targets by including the last two arguments (they're off by default).
There's quite a lot of example projects, and generating project files might take a bit longer when
these options are on, so you probably won't want to include them most of the time.

Then, to build the project:

    cmake --build build (--target targetNameFromCMakeLists) (--config Release/Debug/...)

This tells cmake to build the target named `targetNameFromCMakeLists`, in the specified
configuration, using the appropriate tool. Of course, if you generated makefiles or ninja files, you
could call `make` or `ninja` in the build directory. If you generated an IDE project, like an Xcode
or Visual Studio project, then you could open the generated project in your IDE.

### Building for iOS

To build for iOS, you'll need CMake 3.14 or higher. Using the Xcode generator is highly recommended,
as other generators may not automatically find the correct SDK for the iPhone simulator, and may
fail to run certain parts of the build, such as compiling icons and processing the app's plist. By
default, CMake will build for the same system that originally configured the project, so to enable
cross-compilation for iOS, a few extra flags must be passed to the initial CMake invocation:

    cmake -Bbuild-ios -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=9.3

Here we create a build tree in the directory named 'build-ios', using the Xcode generator. The
`-DCMAKE_SYSTEM_NAME=iOS` option tells CMake to enable cross-compiling for iOS. The
`-DCMAKE_OSX_DEPLOYMENT_TARGET=9.3` option sets the minimum deployment target (it applies to iOS
despite the 'OSX' in the variable name!).

Once the project has generated, we can open it as normal in Xcode (look for the project file in the
build directory). Alternatively, to build from the command-line, we could run this command:

    cmake --build build-ios --target <targetName> -- -sdk iphonesimulator

Here, we're building the target named `<targetName>` from the build tree in the directory
`build-ios`. All the arguments after `--` are ignored by CMake, and are passed through to the
underlying build tool. In this case, the build tool will be `xcodebuild` because we used the Xcode
generator above. We tell xcodebuild that we're building the app for the iOS simulator, which doesn't
require special code signing.

If we wanted to build for a real device, we would need to pass some extra signing details to the
initial CMake configuration command:

    cmake -Bbuild-ios -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_OSX_DEPLOYMENT_TARGET=9.3 \
        -DCMAKE_XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY="iPhone Developer"
        -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=<10 character id>

The `CODE_SIGN_IDENTITY` is the kind of certificate you want to use (iPhone Developer is appropriate
for development) and `DEVELOPMENT_TEAM` is the 10-character ID that can be found by opening the
Keychain Access app, finding your development certificate, and checking its 'Organizational Unit'
info field.

When building the target, you may also need to tell Xcode that it can automatically update
provisioning profiles, which is achieved by passing the `-allowProvisioningUpdates` flag:

    cmake --build build-ios --target <targetName> -- -allowProvisioningUpdates

#### Archiving for iOS

CMake's out-of-the-box archiving behaviour doesn't always work as expected, especially for targets
that depend on custom static libraries. Xcode may generate these libraries into a 'DerivedData'
directory, but then omit this directory from the library search paths later in the build.

If the "Product -> Archive" action isn't working due to missing staticlibs, try setting the
`ARCHIVE_OUTPUT_DIRECTORY` property explicitly:

    set_target_properties(my_static_lib_target PROPERTIES ARCHIVE_OUTPUT_DIRECTORY "./")

Note that the static library produced by `juce_add_binary_data` automatically sets this property.

### Building universal binaries for macOS

Building universal binaries that will run on both arm64 and x86_64 can be achieved by
configuring the CMake project with `"-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"`.

### Building with Clang on Windows

Clang-cl (Clang with MSVC-like command-line) should work by default. If you are generating a Visual
Studio project, and have installed the LLVM package which is distributed with Visual Studio, then
you can configure a Clang-cl build by passing "-T ClangCL" on your configuration commandline.

If you wish to use Clang with GNU-like command-line instead, you can pass
`-DCMAKE_CXX_COMPILER=clang++` and `-DCMAKE_C_COMPILER=clang` on your configuration commandline.
clang++ and clang must be on your `PATH` for this to work. Only more recent versions of CMake
support Clang's GNU-like command-line on Windows. CMake 3.12 is not supported, CMake 3.15 has
support, CMake 3.20 or higher is recommended.  Note that CMake doesn't seem to automatically link a
runtime library when building in this configuration, but this can be remedied by setting the
`MSVC_RUNTIME_LIBRARY` property. See the [official
documentation](https://cmake.org/cmake/help/v3.15/prop_tgt/MSVC_RUNTIME_LIBRARY.html) of this
property for usage recommendations.

### A note about compile definitions

Module options and plugin options that would previously have been set in the Projucer can be set on
a target-by-target basis in CMake, via `target_compile_definitions`. To find the options exposed by
a particular module, check its module header for sections with the following structure:

    /** Config: NAME_OF_KEY
        Docs go here...
    */
    #ifndef NAME_OF_KEY
     #define NAME_OF_KEY ...
    #endif

To override the default config option, use the following CMake code, replacing `<value>` as
appropriate:

    target_compile_definitions(my_target PUBLIC NAME_OF_KEY=<value>)

The `JucePlugin_PreferredChannelConfig` preprocessor definition for plugins is difficult to specify
in a portable way due to its use of curly braces, which may be misinterpreted in Linux/Mac builds
using the Ninja/Makefile generators. It is recommended to avoid this option altogether, and to use
the newer buses API to specify the desired plugin inputs and outputs.

## API Reference

### Options

These flags can be enabled or disabled to change the behaviour of parts of the JUCE build.

These options would normally be configured by either:
- Supplying an option in the form `-DNAME_OF_OPTION=ON/OFF` to the initial CMake configuration call,
  or
- Calling `set(NAME_OF_OPTION ON/OFF)` before including JUCE in your project via `add_subdirectory`
  or `find_package`.

#### `JUCE_BUILD_EXTRAS`

This controls whether targets are added for the projects in the 'extras' folder, such as the
Projucer and AudioPluginHost. This is off by default, because you probably won't need these targets
if you've included JUCE in your own project.

#### `JUCE_BUILD_EXAMPLES`

This controls whether targets are added for the projects in the 'examples' folder, such as the
DemoRunner and PIPs. This is off by default, because you probably won't need these targets if you've
included JUCE in your own project.

#### `JUCE_ENABLE_MODULE_SOURCE_GROUPS`

This option will make module source files browsable in IDE projects. It has no effect in non-IDE
projects. This option is off by default, as it will increase the size of generated IDE projects and
might slow down configuration a bit. If you enable this, you should probably also add
`set_property(GLOBAL PROPERTY USE_FOLDERS YES)` to your top level CMakeLists as this is required for
source grouping to work.

Source groupings are a little sensitive to the project layout. As such, you should always ensure
that the call to `juce_add_module` which adds a specific module happens *before* calling
`juce_add_*` to add any dependent targets.

The modules will be placed in a group named "JUCE Modules" within the group for each target,
alongside the "Source Files" and "Header Files" groups.

Note: Source groups will only work when all JUCE-dependent targets are created using the
`juce_add_*` functions. The standard `add_executable` and `add_library` commands are likely to
result in broken builds when source groups are enabled!

#### `JUCE_COPY_PLUGIN_AFTER_BUILD`

Controls whether plugin targets should be installed to the system after building. Note that the
plugin folders may be protected, so the build may require elevated permissions in order for the
installation to work correctly, or you may need to adjust the permissions of the destination
folders.

### Functions

#### `juce_add_<target>`

    juce_add_gui_app(<target> [KEY value]...)
    juce_add_console_app(<target> [KEY value]...)
    juce_add_plugin(<target> [KEY value]...)

`juce_add_gui_app` and `juce_add_console_app` add an executable target with name `<target>`.
`juce_add_plugin` adds a 'shared code' static library target with name `<target>`, along with extra
targets for each of the specified plugin formats. Each of these functions also takes a number of
optional arguments in the form of a `KEY` followed by one or more `value`s which can be used to set
additional attributes of the target. If these optional arguments aren't specified, their values will
fall back to sensible defaults.

Each of these arguments adds a property to the resulting target in the form `JUCE_paramName`, where
`paramName` is one of the parameter keys below. For example, after a call to
`juce_add_gui_app(my_target PRODUCT_NAME "Target")`, the target `my_target` will have a property
named `JUCE_PRODUCT_NAME` with the value `"Target"`. After creating a target with one of these
commands, properties beginning with `JUCE_` can be _queried_, but changing their values might not
have any effect (or might even break things in unexpected ways!), so always pass JUCE target
attributes directly to these creation functions, rather than adding them later.

`PRODUCT_NAME`
- The name of the output built by this target, similar to CMake's `OUTPUT_NAME` property. If not
  specified, this will default to the target name.

`VERSION`
- A version number string in the format "major.minor.bugfix". If not specified, the `VERSION` of
  the project containing the target will be used instead. On Apple platforms, this is the
  user-facing version string. This option corresponds to the `CFBundleShortVersionString` field in
  the target's plist.

`BUILD_VERSION`
- A version number string in the format "major.minor.bugfix". If not specified, this will match
  the `VERSION` of the target. On Apple platforms, this is the private version string used to
  distinguish between App Store builds. This option corresponds to the `CFBundleVersion` field in
  the target's plist.

`BUNDLE_ID`
- An identifier string in the form "com.yourcompany.productname" which should uniquely identify
  this target. Mainly used for macOS builds. If not specified, a default will be generated using
  the target's `COMPANY_NAME` and `PRODUCT_NAME`.

`MICROPHONE_PERMISSION_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an app's Info.plist.

`MICROPHONE_PERMISSION_TEXT`
- The text your app will display when it requests microphone permissions.

`CAMERA_PERMISSION_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an app's Info.plist.

`CAMERA_PERMISSION_TEXT`
- The text your app will display when it requests camera permissions.

`BLUETOOTH_PERMISSION_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an app's Info.plist.

`BLUETOOTH_PERMISSION_TEXT`
- The text your app will display when it requests bluetooth permissions.

`SEND_APPLE_EVENTS_PERMISSION_ENABLED`
- May be either TRUE or FALSE. Enable this to allow your app to send Apple events.

`SEND_APPLE_EVENTS_PERMISSION_TEXT`
- The text your app will display when it requests permission to send Apple events.

`FILE_SHARING_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

`DOCUMENT_BROWSER_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

`STATUS_BAR_HIDDEN`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

 `REQUIRES_FULL_SCREEN`
 - May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

`BACKGROUND_AUDIO_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

`BACKGROUND_BLE_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's Info.plist.

`APP_GROUPS_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's entitlements.

`APP_GROUP_IDS`
- The app groups to which your iOS app belongs. These will be added to your app's entitlements.

`ICLOUD_PERMISSIONS_ENABLED`
- May be either TRUE or FALSE. Adds the appropriate entries to an iOS app's entitlements.

`IPHONE_SCREEN_ORIENTATIONS`
- May be one or more of `UIInterfaceOrientationUnknown`, `UIInterfaceOrientationPortrait`,
  `UIInterfaceOrientationPortraitUpsideDown`, `UIInterfaceOrientationLandscapeLeft`, or
  `UIInterfaceOrientationLandscapeRight`. Adds appropriate entries to an iOS app's plist.

`IPAD_SCREEN_ORIENTATIONS`
- May be one or more of `UIInterfaceOrientationUnknown`, `UIInterfaceOrientationPortrait`,
  `UIInterfaceOrientationPortraitUpsideDown`, `UIInterfaceOrientationLandscapeLeft`, or
  `UIInterfaceOrientationLandscapeRight`. Adds appropriate entries to an iOS app's plist.

`LAUNCH_STORYBOARD_FILE`
- A custom launch storyboard file to use on iOS. If not supplied, a default storyboard will be
  used.

`CUSTOM_XCASSETS_FOLDER`
- A path to an xcassets directory, containing icons and/or launch images for this target. If this
  is specified, the ICON_BIG and ICON_SMALL arguments will not have an effect on iOS, and a launch
  storyboard will not be used.

`TARGETED_DEVICE_FAMILY`
- Specifies the device families on which the product must be capable of running. Allowed values
  are "1", "2", and "1,2"; these correspond to "iPhone/iPod touch", "iPad", and "iPhone/iPod and
  iPad" respectively. This will default to "1,2", meaning that the target will target iPhone,
  iPod, and iPad.

`ICON_BIG`, `ICON_SMALL`
- Paths to image files that will be used to generate app icons. If only one of these parameters
  is specified, then that image will be used for all icon resolutions. If both arguments are
  specified, then the appropriate image will be picked for each icon resolution.

`COMPANY_COPYRIGHT`
- Copyright text which will be added to the app/plugin's Info.plist. The value of this argument
  will be inherited from the `JUCE_COMPANY_COPYRIGHT` property, so if you want to use the same
  `COMPANY_COPYRIGHT` for several targets in a build tree, you can call
  `set_directory_properties(PROPERTIES JUCE_COMPANY_COPYRIGHT ...)` after including JUCE but
  before adding the targets, and then omit the `COMPANY_COPYRIGHT` argument when creating the
  individual targets.

`COMPANY_NAME`
- The name of this target's author. Will be added to the app/plugin's Info.plist, and may be used
  to generate part of the `BUNDLE_ID` if no ID was given explicitly. The value of this argument
  will be inherited from the `JUCE_COMPANY_NAME` property, so if you want to use the same
  `COMPANY_NAME` for several targets in a build tree, you can call
  `set_directory_properties(PROPERTIES JUCE_COMPANY_NAME ...)` after including JUCE but before
  adding the targets, and then omit the `COMPANY_NAME` argument when creating the individual
  targets.

`COMPANY_WEBSITE`
- The address of a website related to this target in some way. The value of this argument will be
  inherited from the `JUCE_COMPANY_WEBSITE` property, so if you want to use the same
  `COMPANY_WEBSITE` for several targets in a build tree, you can call
  `set_directory_properties(PROPERTIES JUCE_COMPANY_WEBSITE ...)` after including JUCE but before
  adding the targets, and then omit the `COMPANY_WEBSITE` argument when creating the individual
  targets.

`COMPANY_EMAIL`
- An email address for this target's author. The value of this argument will be inherited from the
  `JUCE_COMPANY_EMAIL` property, so if you want to use the same `COMPANY_EMAIL` for several
  targets in a build tree, you can call `set_directory_properties(PROPERTIES JUCE_COMPANY_EMAIL
  ...)` after including JUCE but before adding the targets, and then omit the `COMPANY_EMAIL`
  argument when creating the individual targets.

`DOCUMENT_EXTENSIONS`
- File extensions that should be associated with this target. For example, the Projucer passes
  the string `jucer` because it wants to open `.jucer` files. If your target has several different
  document types, you can pass them as multiple arguments, e.g. `DOCUMENT_EXTENSIONS wav mp3 aif`.

`NEEDS_CURL`
- On Linux, JUCE may or may not need to link to Curl depending on the compile definitions that are
  set on a JUCE target. By default, we don't link Curl because you might not need it, but if you
  get linker or include errors that reference Curl, just set this argument to `TRUE`.

`NEEDS_WEB_BROWSER`
- On Linux, JUCE may or may not need to link to Webkit depending on the compile definitions that
  are set on a JUCE target. By default, we don't link Webkit because you might not need it, but
  if you get linker or include errors that reference Webkit, just set this argument to `TRUE`.

`NEEDS_STORE_KIT`
- On macOS, JUCE may or may not need to link to StoreKit depending on the compile definitions that
  are set on a JUCE target. By default, we don't link StoreKit because you might not need it, but
  if you get linker or include errors that reference StoreKit, just set this argument to `TRUE`.

`PUSH_NOTIFICATIONS_ENABLED`
- Sets app entitlements to allow push notifications. False by default.

`NETWORK_MULTICAST_ENABLED`
- Sets app entitlements to allow IP multicast or broadcast on macOS/iOS. False by default.

`HARDENED_RUNTIME_ENABLED`
- Enables macOS' hardened runtime for this target. Required for notarisation. False by default.

`HARDENED_RUNTIME_OPTIONS`
- A set of space-separated entitlement keys that will be added to this target's entitlements
  plist if `HARDENED_RUNTIME_ENABLED` is `TRUE`. Each key should be in the form
  `com.apple.security.*` where `*` is a specific entitlement.

`APP_SANDBOX_ENABLED`
- Enables macOS' app sandbox for this target. False by default.

`APP_SANDBOX_INHERIT`
- Allows child processes to inherit the static entitlements of their parent process. If this
  is set to `TRUE`, no other app sandbox entitlements will be set on this target.

`APP_SANDBOX_OPTIONS`
- A set of space-separated entitlement keys that will be added to this target's entitlements
  plist if `APP_SANDBOX_ENABLED` is `TRUE`. Each key should be in the form `com.apple.security.*`
  where `*` is a specific entitlement.

`APP_SANDBOX_FILE_ACCESS_HOME_RO`
- A set of space-separated paths that will be added to this target's entitlements plist for
  accessing read-only paths relative to the home directory if `APP_SANDBOX_ENABLED` is `TRUE`.

`APP_SANDBOX_FILE_ACCESS_HOME_RW`
- A set of space-separated paths that will be added to this target's entitlements plist for
  accessing read/write paths relative to the home directory if `APP_SANDBOX_ENABLED` is `TRUE`.

`APP_SANDBOX_FILE_ACCESS_ABS_RO`
- A set of space-separated paths that will be added to this target's entitlements plist for
  accessing read-only absolute paths if `APP_SANDBOX_ENABLED` is `TRUE`.

`APP_SANDBOX_FILE_ACCESS_ABS_RW`
- A set of space-separated paths that will be added to this target's entitlements plist for
  accessing read/write absolute paths if `APP_SANDBOX_ENABLED` is `TRUE`.

`PLIST_TO_MERGE`
- A string to insert into an app/plugin's Info.plist.

`FORMATS`
- For plugin targets, specifies the plugin targets to build. Should be provided as a
  space-separated list. Valid values are `Standalone Unity VST3 AU AUv3 AAX VST`. `AU` and `AUv3`
  plugins will only be enabled when building on macOS. It is an error to pass `AAX` or `VST`
  without first calling `juce_set_aax_sdk_path` or `juce_set_vst2_sdk_path` respectively.

`PLUGIN_NAME`
- The name of the plugin. In a DAW environment, this is the name that will be displayed to the
  user when they go to load a plugin. This name may differ from the name of the physical plugin
  file (to set the name of the plugin file, use the `PRODUCT_NAME` option). If not specified,
  the `PLUGIN_NAME` will default to match the `PRODUCT_NAME`.

`PLUGIN_MANUFACTURER_CODE`
- A four-character unique ID for your company. For AU compatibility, this must contain at least
  one upper-case letter. GarageBand 10.3 requires the first letter to be upper-case, and the
  remaining letters to be lower-case.

`PLUGIN_CODE`
- A four-character unique ID for your plugin. For AU compatibility, this must contain exactly one
  upper-case letter. GarageBand 10.3 requires the first letter to be upper-case, and the remaining
  letters to be lower-case.

`DESCRIPTION`
- A short description of your plugin.

`IS_SYNTH`
- Whether the plugin is a synth. Will be used to set sensible plugin category values if they
  are not provided explicitly.

`NEEDS_MIDI_INPUT`
- Whether the plugin should provide a midi input.

`NEEDS_MIDI_OUTPUT`
- Whether the plugin should provide a midi output.

`IS_MIDI_EFFECT`
- Whether the plugin is a MIDI effect (some hosts provide a special channel-strip location for
  MIDI effect plugins).

`EDITOR_WANTS_KEYBOARD_FOCUS`
- Whether the plugin requires keyboard focus, or should defer all keyboard handling to the host.

`DISABLE_AAX_BYPASS`
- Whether the AAX bypass function should be disabled.

`DISABLE_AAX_MULTI_MONO`
- Whether the AAX multi mono bus layout should be disabled.

`AAX_IDENTIFIER`
- The bundle ID for the AAX plugin target. Matches the `BUNDLE_ID` by default.

`VST_NUM_MIDI_INS`
- For VST2 and VST3 plugins that accept midi, this allows you to configure the number of inputs.

`VST_NUM_MIDI_OUTS`
- For VST2 and VST3 plugins that produce midi, this allows you to configure the number of outputs.

`VST2_CATEGORY`
- Should be one of: `kPlugCategUnknown`, `kPlugCategEffect`, `kPlugCategSynth`,
  `kPlugCategAnalysis`, `kPlugCategMatering`, `kPlugCategSpacializer`, `kPlugCategRoomFx`,
  `kPlugSurroundFx`, `kPlugCategRestoration`, `kPlugCategOfflineProcess`, `kPlugCategShell`,
  `kPlugCategGenerator`.

`VST3_CATEGORIES`
- Should be one or more, separated by spaces, of the following: `Fx`, `Instrument`, `Analyzer`,
  `Delay`, `Distortion`, `Drum`, `Dynamics`, `EQ`, `External`, `Filter`, `Generator`, `Mastering`,
  `Modulation`, `Mono`, `Network`, `NoOfflineProcess`, `OnlyOfflineProcess`, `OnlyRT`,
  `Pitch Shift`, `Restoration`, `Reverb`, `Sampler`, `Spatial`, `Stereo`, `Surround`, `Synth`,
  `Tools`, `Up-Downmix`

`AU_MAIN_TYPE`
- Should be one of: `kAudioUnitType_Effect`, `kAudioUnitType_FormatConverter`,
  `kAudioUnitType_Generator`, `kAudioUnitType_MIDIProcessor`, `kAudioUnitType_Mixer`,
  `kAudioUnitType_MusicDevice`, `kAudioUnitType_MusicEffect`, `kAudioUnitType_OfflineEffect`,
  `kAudioUnitType_Output`, `kAudioUnitType_Panner`

`AU_EXPORT_PREFIX`
- A prefix for the names of entry-point functions that your component exposes. Typically this
  will be a version of your plugin's name that can be used as part of a C++ token. Defaults
  to your plugin's name with the suffix 'AU'.

`AU_SANDBOX_SAFE`
- May be either TRUE or FALSE. Adds the appropriate entries to an AU plugin's Info.plist.

`SUPPRESS_AU_PLIST_RESOURCE_USAGE`
- May be either TRUE or FALSE. Defaults to FALSE. Set this to TRUE to disable the `resourceUsage`
  key in the target's plist. This is useful for AU plugins that must access resources which cannot
  be declared in the resourceUsage block, such as UNIX domain sockets. In particular,
  PACE-protected AU plugins may require this option to be enabled in order for the plugin to load
  in GarageBand.

`AAX_CATEGORY`
- Should be one or more of: `AAX_ePlugInCategory_None`, `AAX_ePlugInCategory_EQ`,
  `AAX_ePlugInCategory_Dynamics`, `AAX_ePlugInCategory_PitchShift`, `AAX_ePlugInCategory_Reverb`,
  `AAX_ePlugInCategory_Delay`, `AAX_ePlugInCategory_Modulation`, `AAX_ePlugInCategory_Harmonic`,
  `AAX_ePlugInCategory_NoiseReduction`, `AAX_ePlugInCategory_Dither`,
  `AAX_ePlugInCategory_SoundField`, `AAX_ePlugInCategory_HWGenerators`,
  `AAX_ePlugInCategory_SWGenerators`, `AAX_ePlugInCategory_WrappedPlugin`,
  `AAX_ePlugInCategory_Effect`

`PLUGINHOST_AU`
- May be either TRUE or FALSE (defaults to FALSE). If TRUE, will add the preprocessor definition
  `JUCE_PLUGINHOST_AU=1` to the new target, and will link the macOS frameworks necessary for
  hosting plugins. Using this parameter should be preferred over using
  `target_compile_definitions` to manually set the `JUCE_PLUGINHOST_AU` preprocessor definition.

`USE_LEGACY_COMPATIBILITY_PLUGIN_CODE`
- May be either TRUE or FALSE (defaults to FALSE). If TRUE, will override the value of the
  preprocessor definition "JucePlugin_ManufacturerCode" with the hex equivalent of "proj". This
  option exists to maintain compatibility with a previous, buggy version of JUCE's CMake support
  which mishandled the manufacturer code property. Most projects should leave this option set to
  its default value.

`COPY_PLUGIN_AFTER_BUILD`
- Whether or not to install the plugin to the current system after building. False by default.
  If you want all of the plugins in a subdirectory to be installed automatically after building,
  you can set the property `JUCE_COPY_PLUGIN_AFTER_BUILD` on the directory before adding the
  plugins, rather than setting this argument on each individual target. Note that on Windows,
  the default install locations may not be writable by normal user accounts.

`VST_COPY_DIR`
- The location to which VST2 (legacy) plugins will be copied after building if
  `COPY_PLUGIN_AFTER_BUILD` is set on this target. If you want to install all of the VST2 plugins
  in a subdirectory to a non-default location, you can set the `JUCE_VST_COPY_DIR` property on
  the directory before adding the plugin targets, rather than setting this argument on each
  individual target.

`VST3_COPY_DIR`
- The location to which VST3 plugins will be copied after building if `COPY_PLUGIN_AFTER_BUILD`
  is set on this target. If you want to install all of the VST3 plugins in a subdirectory to a
  non-default location, you can set the `JUCE_VST3_COPY_DIR` property on the directory before
  adding the plugin targets, rather than setting this argument on each individual target.

`AAX_COPY_DIR`
- The location to which AAX plugins will be copied after building if `COPY_PLUGIN_AFTER_BUILD`
  is set on this target. If you want to install all of the AAX plugins in a subdirectory to a
  non-default location, you can set the `JUCE_AAX_COPY_DIR` property on the directory before
  adding the plugin targets, rather than setting this argument on each individual target.

`AU_COPY_DIR`
- The location to which AU plugins will be copied after building if `COPY_PLUGIN_AFTER_BUILD`
  is set on this target. If you want to install all of the AU plugins in a subdirectory to a
  non-default location, you can set the `JUCE_AU_COPY_DIR` property on the directory before
  adding the plugin targets, rather than setting this argument on each individual target.

`UNITY_COPY_DIR`
- The location to which Unity plugins will be copied after building if `COPY_PLUGIN_AFTER_BUILD`
  is set on this target. If you want to install all of the Unity plugins in a subdirectory to a
  non-default location, you can set the `JUCE_UNITY_COPY_DIR` property on the directory before
  adding the plugin targets, rather than setting this argument on each individual target.
  Unlike the other `COPY_DIR` arguments, this argument does not have a default value so be sure
  to set it if you have enabled `COPY_PLUGIN_AFTER_BUILD` and the `Unity` format.

#### `juce_add_binary_data`

    juce_add_binary_data(<name>
        [HEADER_NAME ...]
        [NAMESPACE ...]
        SOURCES ...)

Create a static library that embeds the contents of the files passed as arguments to this function.
Adds a library target called `<name>` which can be linked into other targets using
`target_link_libraries`.

The `HEADER_NAME` argument is optional. If provided, the generated header will be given the
requested name, otherwise the generated header will be named "BinaryData.h". In completely new
projects, you should provide a unique name here, so that projects containing more than one binary
data target are able to include the binary data headers without ambiguity.

The `NAMESPACE` argument is also optional. If not provided, the generated files will use the default
namespace `BinaryData`. Each of the files located at the paths following `SOURCES` will be encoded
and embedded in the resulting static library. This library can be linked as normal using
`target_link_libraries(<otherTarget> PRIVATE <name>)`, and the header can be included using
`#include <BinaryData.h>`.

#### `juce_add_bundle_resources_directory`

    juce_add_bundle_resources_directory(<target> <folder>)

Copy the entire directory at the location `<folder>` into an Apple bundle's resource directory, i.e.
the `Resources` directory for a macOS bundle, and the top-level directory of an iOS bundle.

#### `juce_generate_juce_header`

    juce_generate_juce_header(<target>)

Introspects the JUCE modules that have been linked to `<target>` and generates a `JuceHeader.h`
which contains `#include` statements for each of the module headers. This header also contains an
optional `using namespace juce` statement, and an optional `ProjectInfo` block, each of which can be
disabled by setting the compile definitions `DONT_SET_USING_JUCE_NAMESPACE` and
`JUCE_DONT_DECLARE_PROJECTINFO` respectively. The resulting header can be included with `#include
<JuceHeader.h>`. In plain CMake projects which don't require Projucer compatibility, the use of
JuceHeader.h is optional. Instead, module headers can be included directly in source files that
require them.

#### `juce_enable_copy_plugin_step`

    juce_enable_copy_plugin_step(<target>)

As an alternative to the JUCE_COPY_PLUGIN_AFTER_BUILD property, you may call this function to
manually enable post-build copy on a plugin. The argument to this function should be a target
previously created with `juce_add_plugin`.

JUCE_COPY_PLUGIN_AFTER_BUILD will cause plugins to be installed immediately after building. This is
not always appropriate, if extra build steps (such as signing or modifying the plugin bundle) must
be executed before the install. In such cases, you should leave JUCE_COPY_PLUGIN_AFTER_BUILD
disabled, use `add_custom_command(TARGET POST_BUILD)` to add your own post-build steps, and then
finally call `juce_enable_copy_plugin_step`.

If your custom build steps need to use the location of the plugin artefact, you can extract this
by querying the property `JUCE_PLUGIN_ARTEFACT_FILE` on a plugin target (*not* the shared code
target!).

#### `juce_set_<kind>_sdk_path`

    juce_set_aax_sdk_path(<absolute path>)
    juce_set_vst2_sdk_path(<absolute path>)
    juce_set_vst3_sdk_path(<absolute path>)

Call these functions from your CMakeLists to set up your local AAX, VST2, and VST3 SDKs. These
functions should be called *before* adding any targets that may depend on the AAX/VST2/VST3 SDKs
(plugin hosts, AAX/VST2/VST3 plugins etc.).

#### `juce_add_module`

    juce_add_module(<path to module>)
    juce_add_modules(<names of module>...)

`juce_add_module` adds a library target for the JUCE module located at the provided path. `<path>`
must be the path to a module directory (e.g. /Users/me/JUCE/modules/juce_core). This will add an
interface library with a name matching the directory name of the module. The resulting library can
be linked to other targets as normal, using `target_link_libraries`.

Due to the way that `INTERFACE` libraries work in CMake, linking to a module added in this way
*must* be done using `PRIVATE` visibility. Using `PUBLIC` will cause the module sources to be added
both to the target's `SOURCES` and `INTERFACE_SOURCES`, which may result in many copies of the
module being built into a single target, which would cause build failures in the best case and
silent ODR violations in the worst case. Scary stuff!

This command has a few optional arguments: `INSTALL_PATH` is a path, relative to the install prefix,
to which the module sources will be copied during installation of the module. ALIAS_NAMESPACE will
add an alias for the module target(s) with the provided namespace. For example, the following
invocation will add a module target named `my_module`, along with an alias named
`company::my_module`. ``` juce_add_module(my_module ALIAS_NAMESPACE company)` ```

`juce_add_modules` is a convenience function that can be used to add multiple JUCE modules at once.
This version accepts many module paths, rather than just one. For an example of usage, see the
CMakeLists in the `modules` directory.

#### `juce_add_pip`

    juce_add_pip(<header>)

This function parses the PIP metadata block in the provided header, and adds appropriate build
targets for a console app, GUI app, or audio plugin. For audio plugin targets, it builds as many
plugin formats as possible. To build AAX or VST2 targets, call `juce_set_aax_sdk_path` and/or
`juce_set_vst2_sdk_path` *before* calling `juce_add_pip`.

This is mainly provided to build the built-in example projects in the JUCE repo, and for building
quick proof-of-concept demo apps with minimal set-up. For any use-case more complex than a
proof-of-concept, you should prefer the `juce_add_gui_app`, `juce_add_plugin`, or
`juce_add_console_app` functions, which provide more fine-grained control over the properties of
your target.

#### `juce_disable_default_flags`

    juce_disable_default_flags()

This function sets the `CMAKE_<LANG>_FLAGS_<MODE>` to empty in the current directory and below,
allowing alternative optimisation/debug flags to be supplied without conflicting with the
CMake-supplied defaults.

### Targets

#### `juce::juce_recommended_warning_flags`

    target_link_libraries(myTarget PUBLIC juce::juce_recommended_warning_flags)

This is a target which can be linked to other targets using `target_link_libraries`, in order to
enable the recommended JUCE warnings when building them.

This target just sets compiler and linker flags, and doesn't have any associated libraries or
include directories. When building plugins, it's probably desirable to link this to the shared code
target with `PUBLIC` visibility, so that all the plugin wrappers inherit the same compile/link
flags.

#### `juce::juce_recommended_config_flags`

    target_link_libraries(myTarget PUBLIC juce::juce_recommended_config_flags)

This is a target which can be linked to other targets using `target_link_libraries`, in order to
enable the recommended JUCE optimisation and debug flags.

This target just sets compiler and linker flags, and doesn't have any associated libraries or
include directories. When building plugins, it's probably desirable to link this to the shared code
target with `PUBLIC` visibility, so that all the plugin wrappers inherit the same compile/link
flags.

#### `juce::juce_recommended_lto_flags`

    target_link_libraries(myTarget PUBLIC juce::juce_recommended_lto_flags)

This is a target which can be linked to other targets using `target_link_libraries`, in order to
enable the recommended JUCE link time optimisation settings.

This target just sets compiler and linker flags, and doesn't have any associated libraries or
include directories. When building plugins, it's probably desirable to link this to the shared code
target with `PUBLIC` visibility, so that all the plugin wrappers inherit the same compile/link
flags.

