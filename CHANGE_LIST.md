# Major JUCE features and updates

This file lists the more notable headline features. For more detailed info
about changes and bugfixes please see the git log and BREAKING_CHANGES.md.

## Version 8.0.9

  - Added support for configurable font features (ligatures, kerning, ...)
  - Multiple improvements to Android windowing
  - Multiple improvements to text shaping
  - Fixed clearing audio buffers on WASAPI device start/stop
  - Fixed AU hosting of plug-ins with poorly implemented parameters
  - Added macOS/iOS 26 support
  - Added support for iOS UIScene lifecycle on iOS 13+
  - Added support for 32-bit int WAV files
  - Multiple Linux WebView improvements
  - Fixed an issue with MIDI FX AAX plug-ins on any audio channel layouts
  - Accessibility navigation is now enabled by default for disabled components

## Version 8.0.8

  - Improved the TextEditor layout behaviour
  - Added new options to control text line spacing
  - Added more Direct2D bug fixes and performance improvements
  - Fixed a iOS simulator buffer size issue
  - Moved the MIDI CapabilityInquiry Demo into the DemoRunner
  - Updated the default Android toolchain version

## Version 8.0.7

  - Improved unicode handling and performance in TextEditor
  - Fixed iOS external device sample rate handling in iOS 18
  - Added many Direct2D bug fixes and performance improvements
  - Added a new MessageManager::callSync counterpart to callAsync
  - Fixed a crash in Ableton when closing a plug-in window
  - Improved sscache compatibility
  - Fixed some PopupMenu bugs
  - Updated Zlib

## Version 8.0.6

  - Changed a Visual Studio toolchain error into a warning 

## Version 8.0.5

  - Added Windows Arm support
  - Added support for local notifications
  - Added passthrough compiler options to juceaide
  - Added support for VST3 parameter migrations
  - Fixed some Windows mouse events and window dragging behaviour
  - Added Ranges functionality
  - Added VST2 and VST3 MIDI note names

## Version 8.0.4

  - Simplified singleton creation
  - Fixed some Javascript and C++ interoperability issues
  - Added exact passthrough of MIDI CC timestamps
  - Switched to obtaining MIDI plug-in properties at runtime
  - Improved Windows Arm CMake support
  - Improved ShapedText
  - Fixed some issues with Windows DLL builds
  - Add system-provided timestamps to VBlankAttachment and animations
  - Fixed some iOS deprecation warnings
  - Updated embedded CHOC version
  - Updated embedded Oboe version
  - Moved the JavaScript implementation into a separate module

## Version 8.0.3

  - Updated the AAX SDK to 2.8.0
  - Fixed multiple Direct2D drawing issues
  - Fixed buffer size and sample rate selection on iOS 18

## Version 8.0.2

  - Fixed some issues handling large images in Direct2D
  - Enabled rounded window corners in Windows 11
  - Fixed some compiler warnings in Xcode 16
  - Improved macOS and Android GU rendering performance
  - Added support for C++20 and C++23
  - Fixed a Windows mouse response issue
  - Updated the VST3 SDK to 3.7.12

## Version 8.0.1

  - Fixed some issues with text layout
  - Removed source code for unsupported platforms
  - Fixed some Direct2D issues
  - Update the embedded version of harfbuzz
  - Added more surround formats

## Version 8.0.0

  - Added a new Direct2D renderer
  - Added support for WebView based UIs
  - Added consistent unicode support across platforms
  - Added a new animation module
  - Bundled the AAX SDK

## Version 7.0.12

  - Fixed an issue with timers in Pro Tools
  - Fixed an issue with Projucer Xcode code signing

## Version 7.0.11

  - Fixed an issue with paths containing a tilde in Xcode
  - Multiple fixes for plug-in deployment and code signing in Xcode
  - Fixed an issue painting an empty RectangleList
  - Improved the performance of TreeView rendering

## Version 7.0.10

  - Fixed multiple issues selecting devices in AudioDeviceSelector
  - Updated the bundled Oboe version
  - Fixed multiple issues with Timer
  - Updated the bundled version of FLAC
  - Added configuration options for sockets
  - Added new JSON::Formatter
  - Added support for Xcode 15.1
  - Update OpenGL compatibility headers
  - Added ChildProcessManager
  - Fixed multiple MIDI-CI issues

## Version 7.0.9

  - Added MIDI-CI support
  - Added enumerate utility function
  - Fixed a macOS/iOS CMake signing issue

## Version 7.0.8

  - Added macOS/iOS AudioWorkgroup support
  - Added Xcode 15, macOS Sonoma and LLVM 17 compatibility
  - Added serialisation tools
  - Fixed some VST3 manifest generation issues
  - Fixed a MessageManager locking bug
  - Fixed GCC 7 VST3 support
  - Fixed some SVG scaling issues

## Version 7.0.7

  - Fixed some macOS 14.0 deprecations
  - Fixed some issues with VST3 manifest generation
  - Fixed a Metal layer rendering issue
  - Fixed an issue setting realtime thread priorities
  - Fixed a crash in VirtualDesktopWatcher
  - Fixed an AUv3 bundling problem

## Version 7.0.6

  - Added support for VST3 bundles and moduleinfo.json
  - Improved message box dismissal
  - Improved WebView support
  - Updated to the latest VST3 and AAX SDKs
  - Fixed some Metal layer rendering issues
  - Improved ambisonic support
  - Improved machine ID support
  - Improved the HighResolutionTimer implementation

## Version 7.0.5

  - Fixed Windows 7 compatibility
  - Fixed dark mode notifications on macOS
  - Improved the performance of AudioProcessorGraph

## Version 7.0.4

  - Improved Metal device handling
  - Adopted more C++17 features
  - Improved input handling on macOS and iOS
  - Fixed a GUI display issue on Linux
  - Fixed some compiler warnings

## Version 7.0.3

  - Added a unique machine ID
  - Added new threading classes
  - Improved the performance of multiple OpenGL contexts
  - Refactored AudioProcessorGraph
  - Improved AudioDeviceManager sample rate handling
  - Fixed Studio One drawing performance
  - Updated the FLAC library

## Version 7.0.2

  - Fixed accessibility table navigation
  - Fixed Android file access on older APIs
  - Improved Linux VST3 threading
  - Improved ARA integration

## Version 7.0.1

  - Fixed some Xcode and MSVC compiler warnings
  - Improved VST3 bus configuration and channel handling
  - Fixed some Metal layer rendering bugs

## Version 7.0.0

  - Added Audio Random Access (ARA) SDK support
  - Added support for authoring and hosting LV2 plug-ins
  - Added a default renderer for macOS and iOS
  - Added new macOS and iOS rendering options
  - Added hardware synchronised drawing on Windows, macOS and iOS
  - Updated the Android billing and file access APIs
  - Revamped AudioPlayHead functionality
  - Improved accessibility support

## Version 6.1.6

  - Improved the handling of AU multichannel layouts
  - Added JUCE_NODISCARD to builder-patten functions
  - Added recursion options to DirectoryIterator
  - Unified the loading of OpenGL 3.2 core profiles
  - Improved macOS full-screen behaviour with non-native titlebars

## Version 6.1.5

  - Improved the accessibility framework
  - Added handling of non-Latin virtual key codes on macOS
  - Improved X11 compatibility
  - Updated the iOS in-app purchases workflow
  - Improved macOS windowing behaviour
  - Improved MinGW-w64 compatibility
  - Added an MPEKeyboardComponent class

## Version 6.1.4

  - Restored Projucer project saving behavior
  - Fixed a CGImage memory access violation on Monterey
  - Improved macOS thread priority management

## Version 6.1.3

  - Added support for Visual Studio 2022 to the Projucer
  - Added support for creating OpenGL 3.2 contexts on Windows
  - Added support for plugin hosts to easily retrieve stable parameter IDs
  - Added high-resolution image support to DragAndDropContainer
  - Added support for a wider range of frame-rates in plugins and hosts
  - Made Font and TypefaceCache threadsafe, to allow font rendering on background threads
  - Improved FlexBox compatibility with the CSS FlexBox specification
  - Improved macOS 12 compatibility, including OpenGL and FileChooser fixes
  - Improved accessibility support

## Version 6.1.2

  - Fixed an OpenGL display refresh rate issue on macOS
  - Improved the scaling behaviour of hosted VST3 plug-ins
  - Improved accessibility support

## Version 6.1.1

  - Fixed a CMake installation issue
  - Improved parameter value loading after plug-in restarts
  - Fixed some problems with multi-line text layouts
  - Added a fallback for modal native message boxes on Windows
  - Fixed an issue setting OpenGL repaint events
  - Improved accessibility support

## Version 6.1.0

  - Added accessibility support
  - Enabled use of VST3 plug-in extensions
  - Improved OpenGL function loading
  - Updated to C++14
  - Added support for macOS Monterey and iOS 15
  - Added async versions of all modal functions
  - Fixed some VST3 threading issues
  - Added cross-platform-compatible VST3 UID hash
  - Improved MinGW compatibility
  - Fixed some issues with BufferingAudioReader
  - Improved TextEditor repainting
  - Added support for larger ASIO buffers
  - Updated Android Oboe to 1.6.1
  - Improved modal dismissing
  - Improved assertion handling on macOS ARM

## Version 6.0.8

  - Fixed a macOS graphics invalidation region issue
  - Improved the handling of modal dialog dismissal
  - Fixed audio glitching in CoreAudio before microphone permission is granted
  - Improved AUv3 resizing and initialisation
  - Fixed some string to double conversions
  - Improved iOS split view behaviour
  - Added Display::safeAreaInserts
  - Improved assertion behaviour on macOS ARM
  - Multiple resizing and display scaling fixes
  - Added more information to audioProcessorChanged callbacks
  - Fixed some DSP convolution issues
  - Added host detection on macOS ARM

## Version 6.0.7

  - Fixed a macOS drawing issue
  - Updated the DemoRunner bundle ID

## Version 6.0.6

  - Moved to the new CoreMIDI API on supported platforms
  - Added support for the "New Build System" in Xcode
  - Made the audio format readers more robust
  - Improved the HiResTimer implementation
  - Fixed a VST3 program parameter issue
  - Updated to Oboe 1.5 on Android

## Version 6.0.5

  - Added more support for styling PopupMenus
  - Fixed some race conditions in the IPC and name named pipe classes
  - Implemented multiple FileChooser improvements
  - Added compatibility with the latest Android SDK
  - Prevented CoreAudio glitches before accepting audio access permissions
  - Made reading MIDI and audio files more robust

## Version 6.0.4

  - Improved the Projucer update mechanism
  - Fixed an AUv3 parameter normalisation issue
  - Fixed WASAPI exclusive mode sample rate selection bug
  - Fixed a Linux build issue when omitting ALSA

## Version 6.0.3

  - Fixed version numbers in project files

## Version 6.0.2

  - Added support for macOS 11 and arm64
  - Added Windows IAudioClient3 support for low latency audio drivers
  - Added Windows and macOS precompiled header support in the Projucer
  - Improved accessibility support in the macOS menu bar
  - Fixed VST3 hosting for plug-ins requiring persistent DLL loads
  - Updated macOS camera capture API
  - Improved resave diffs in Projucer project files
  - Fixed some Linux JACK issues

## Version 6.0.1

  - Fixed a bug in the Projucer GUI editor causing existing code to be overwritten
  - Updated Android Oboe to 1.4.2
  - Bumped default Android Studio gradle and plugin versions to the latest
  - Fixed some Android Oboe and OpenSL issues
  - Fixed some Doxygen parsing issues
  - Fixed MIDI input/output bus enablement in VST3 plug-ins
  - Improved Windows Clang compatibility
  - Fixed GCC 4.8 and 5.0 compatibility
  - Fixed some VST3 build errors and warnings on Linux
  - Fixed dynamically loaded X11 library names on Linux
  - Fixed Projucer CLion exporter generated CMakeLists.txt
  - Fixed drag and drop for non-DPI aware plug-ins on Windows

## Version 6.0.0

  - Added support for building JUCE projects with CMake
  - Revamped the DSP module
  - Added VST3 support on Linux
  - Added support for the latest webview components on macOS/iOS and Windows
  - Removed the sign-in requirement, app reporting and analytics from the Projucer
  - Added support for running headlessly on Linux
  - Bundled Oboe source in JUCE and made it the default audio device on Android
  - Various Oboe stability improvements
  - Various Projucer UI improvements
  - Added HWNDComponent for embedding native HWNDs on Windows
  - Added support for all camera names on macOS
  - Added support for building with Clang on Windows
  - Modified MidiMessageCollector to preallocate storage
  - Modified AudioProcessorGraph to allow extracting nodes
  - Refactored the APVTS parameter attachment classes and added a new ParameterAttachment class
  - Added IPP FFT implementation
  - Added all example plugins as internal nodes in AudioPluginHost project
  - Removed JuceHeader requirement from Projucer projects
  - Added support for legacy CC output events
  - Added MidiBuffer::Iterator class compatible with C++11 range-for
  - Added RangedDirectoryIterator class compatible with C++11 range-for
  - Provided range-for comaptibility for String class
  - Windows and Linux hiDPI scaling improvements
  - Various bug-fixes, improvements and documentation updates

## Version 5.4.7

  - Fixed a macOS focus bug causing Components to not receive mouse events
  - Fixed a potential NullPointerException in the Android IAP code
  - Fixed an entitlements file generation bug in the Projucer
  - Send VST2 audioMasterUpdateDisplay opcode on the message thread to fix some hosts not updating
  - Fixed some build errors and warnings when using Clang on Windows
  - Changed the default architecture specified in Linux Makefiles generated by the Projucer

## Version 5.4.6

  - Fixed compatibility with macOS versions below 10.11
  - Multiple thread safety improvements
  - Added dynamic parameter and parameter group names
  - Updated to the latest Android In-App Purchases API
  - Improvements to the Windows message queue under high load
  - Replaced WaitableEvent internals with std::condition_variable
  - Fixed some macOS text alignment issues

## Version 5.4.5

  - Improved message queue performance on Linux
  - Added missing lifecycle callbacks on Android Q
  - Refactored the AudioBlock class
  - Fixed APVTS parameter update recursion
  - Updated Bela code to support latest release
  - Fixed issues drawing italicised text on macOS
  - Fixed broken back button behaviour on Android
  - Added Bluetooth permissions settings needed for iOS 13.0+ to the Projucer
  - Replaced select() calls with poll()
  - Various bug-fixes, improvements and documentation updates

## Version 5.4.4

  - Improvements to floating point number printing
  - Faster plug-in parameter indexing
  - Added support for persisting attachements to MIDI devices
  - Refactored Linux event loop handling
  - Multiple C++ modernisation improvements to the API
  - Added support for macOS 10.15 and iOS 13
  - Added support for Visual Studio 2019
  - Removed support for Visual Studio 2013

## Version 5.4.3

  - Added a Visual Studio 2019 exporter to the Projucer
  - Added options to configure macOS Hardened Runtime in the Projucer
  - Fixed a potential memory corruption when drawing on macOS/iOS
  - Fixed file drag and drop for Windows 8
  - Multiple DSP module enhancements
  - Various bug-fixes, improvements and documentation updates

## Version 5.4.2

  - Restructured the low-level Android native code
  - Added an ADSR envelope class
  - AudioProcessorValueTreeState performance improvements
  - Improved Xcode 10 support
  - Improved VST3 hosting
  - Windows hiDPI scaling enhancements

## Version 5.4.1

  - Fixed a VST2 compilation error in VS2013
  - Fixed some live-build compilation errors in the Projucer
  - Fixed a bug in the Oversampling class
  - Made MPESynthesiserVoice::noteOnTime public
  - Fixed some bugs in the Unity plug-in wrapper
  - Fixed some VS2015 compiler errors

## Version 5.4.0

  - macOS Mojave and iOS 12 support
  - Windows hiDPI support
  - Unity native plug-in support
  - Microsoft BLE MIDI support
  - Plug-in parameter groups
  - Support for production-ready Android OBOE
  - Video playback support on Android and iOS
  - AudioProcessorValueTreeState improvements
  - Support for Android Studio 3.2
  - Various bug-fixes, improvements and documentation updates

## Version 5.3.2

  - Removed the OSX 10.5 and 10.6 deployment target options from the Projucer and enabled more C++11 features across all platforms
  - Replaced all usage of ScopedPointer with std::unique_ptr
  - Added camera support for iOS and Android
  - Fixed some issues using an UndoManager with an AudioProcessorValueTreeState
  - Added MIDI input to IAA plug-ins
  - Made multiple calls to MidiInput::openDevice share the same underlying win32 MIDI handle
  - Added a config flag to juce_audio_processors for enabling LADSPA plugin hosting and enabled it in the AudioPluginHost
  - Added a "plug-in can do" callback to the VSTCallbackHandler interface
  - Fixed various undefined behavior in SIMDRegister
  - Added the methods AudioBlock::copyTo/AudioBlock::copyFrom which facilitate copying to/from an AudioBuffer
  - Added a lambda callback to OpenGLGraphicsContextCustomShader to allow custom set-up when the shader is activated
  - Fixed a bug causing an unintentional menu item highlight disco party when using a popup menu in a plug-in's UI
  - Marked as deprecated: String::empty, var::null, File::nonexistent, ValueTree::invalid and other problematic statically-initialised null values

## Version 5.3.1

  - Add Android and iOS support to AudioPluginHost
  - Added support for Bela in the form of an AudioIODeviceType
  - Add bypass support to both hosting and plug-in client code
  - Added an isBoolean flag to APVTS parameters
  - Re-worked plug-in wrappers to all use new parameter system via LegacyAudioParameter wrapper class
  - Fixed an issue where opening the same midi device twice would cause a crash on Windows
  - Deprecated MouseInputSource::hasMouseMovedSignificantlySincePressed() and replaced with more descriptive methods
  - Added support for relative or special path symbolic links when compressing/uncompressing zip archives and creating/reading files
  - Ensured that File::replaceInternal does not fail with ACL errors on Windows
  - Merged-in some Ogg-Vorbis security fixes
  - Fixed a bug which would prevent a SystemTrayIconComponent from creating a native popup window on macOS
  - Various Android and iOS fixes
  - Added a "PIP Creator" utility tool to the Projucer
  - Added options for setting plugin categories and characteristics with MultiChoicePropertyComponent in the Projucer
  - Fixed a Projucer bug where the OSX base SDK version was not being set
  - Added a command-line option to use LF as linefeeds rather than CRLF in the Projucer cleanup tools
  - Multiple documentation updates

## Version 5.3.0

  - Added support for Android OBOE (developer preview)
  - Updated JUCE's MPE classes to comply with the new MMA-adopted specification
  - Multiple documentation updates
  - Restructured the examples and extras directories and updated all JUCE examples
  - Multiple hosted parameter improvements
  - Overhauled the GenericAudioProcessorEditor
  - Added support for a subset of the Cockos VST extensions
  - Added support for loading VST3 preset files
  - Added boolean AudioProcessorParameters
  - Added thread safe methods for getting and setting the AudioProcessorValueTreeState state
  - Added customisable MacOS icons

## Version 5.2.1

  - Added native content sharing support for iOS and Android
  - Added iOS and Android native file chooser support
  - Implemented WebBrowserComponent on Android
  - Added SystemStats::getDeviceManufacturer()
  - Ensured that JUCE will always use the high-performance audio path on Android if the device supports it
  - Added memory warning callbacks on iOS
  - Refactored iOSAudioDevice to support multi-channel audio devices and improve the handling of sample rate changes from other apps
  - Added SidePanel and BurgerMenu component classes
  - Added PushNotifications support on OSX
  - Added support for VST3 SDK 3.6.8
  - Added support for loading VST3 preset files
  - Added higher-order ambisonics support
  - Added thread safe methods for getting and setting the AudioProcessorValueTreeState state
  - Cleanup and refactoring work on the AudioProcessorGraph and the audio plugin host demo
  - Changed the default language standard for new projects from C++11 to C++14 and set all JUCE projects to use C++14
  - Made the ScopedPointer interface more compatible with std::unique_ptr
  - Changed Windows projects to use dynamic runtime linking by default
  - Added lambda callbacks to ListenerList, Slider, Button, Label, ComboBox and TextEditor
  - Fixed the live-build engine on Windows
  - Multiple DSP module fixes and features
  - Multiple threading and undefined behaviour fixes and improvements
  - Various graphics optimisations
  - Multiple Projucer UI and UX improvements
  - Various documentation tweaks and fixes

## Version 5.2.0

  - Added a CMake exporter to the Projucer
  - JUCE analytics module
  - Added support for push notifications on iOS and Android
  - Added in-app purchase support for macOS
  - Added a plugin binary copy step to the Visual Studio exporter
  - Added an option to set the debug information format in the Visual Studio exporter
  - Added a link-time optimisation option to all exporters
  - Added support for adding asm files to Android projects
  - Improved the reliability of the Projucer's live-build engine
  - Added support for AUv2 Midi Effect plug-in hosting
  - Added support for Atmos 7.0.2 and 7.1.2 Surround formats
  - Added support for the OGG sub-format inside a WAV file
  - Added support for querying the audio hardware on how many overruns/underruns occurred
  - Implement Process::hide on mobile platforms
  - Added support for multi-touch drag and drop
  - Improved the performance of 3D rendering when multiple OpenGL contexts are used at the same time
  - Tweaked the rate at which EdgeTable grows its internal storage, to improve performance rendering large and complex paths

## Version 5.1.2

  - Fixed multiple plugin-resizing bugs
  - Added support for AUv3 MIDI and screen size negotiation
  - Added support for Xcode 9 and iOS 11
  - Added an In-App Purchases module
  - Added backwards compatible constexpr support
  - Standalone plug-in improvements
  - Better .jucer file change monitoring in the Projucer
  - Increased the speed of AU parameter lookup
  - Improved the Android thread management when dealing with web requests
  - Better denormal support
  - Plug-in parameters can be explicitly marked as continuous or discrete
  - Multiple documentation updates

## Version 5.1.1

  - Fixed Windows live build engine on Visual Studio 2017
  - Fixed a compiler error in juce_MathFunctions.h in Visual Studio 2013
  - Fixed a potential crash when using the ProcessorDuplicator
  - Fixed a compiler-error in Filter::IIR
  - Fixed an issue where the WavFileFormatWriter could not create files with discrete channels
  - Fixed an issue where a window which is beneath a hidden window would not receive any clicks on Linux
  - Altered the format of BREAKING-CHANGES.txt to display better on GitHub
  - Projucer: Fixed an issue in exporter tilde expansion
  - Fixed compiler errors when building the DSP module with a static version of FFTW
  - Fixed an audio glitch when bypassing the convolution engine
  - Fixed an issue where a JUCE VST2 would not correctly report that it supports resizing of it’s plugin editor
  - Various documentation tweaks and fixes

## Version 5.1.0

  - Release of the JUCE DSP module
  - Multichannel audio readers and writers
  - Plugin editor Hi-DPI scaling support
  - Major improvements to Projucer module search paths
  - Added Projucer support for iOS app groups
  - Added support for AVFoundation and deprecated the use of Quicktime
  - Added a new real-time audio thread priority for Android
  - Various Projucer UI fixes
  - Various documentation fixes
  - Various minor improvements and bug fixes

## Version 5.0.2

  - Improved project save speed in the Projucer
  - Added option to save individual exporters in the Projucer
  - Added the ability to create custom colour schemes for the Projucer’s code editor
  - Minor fixes to JUCE’s SVG parser
  - Various bug fixes in the way JUCE handles Hi-DPI monitors
  - Improved code browsing in Visual Studio Exports
  - Improved the handling of audio device buffer size changes on iOS
  - Fixed bug in the Win32 FileChooser dialog when selecting a nonexistent root drive
  - Fixed a Projucer crash when saving projects with no targets
  - Fixed a bug where Projucer generated Makefiles would not trigger a recompilation when header files had changed
  - The standalone plugin target is now compatible with effect plug-ins
  - Fixed an issue where it was not possible to use the live build engine on plugin projects
  - Improved the way the Projucer’s live-build engine searches for platform headers on Windows
  - Fixed an issue where the Projucer would complain about not having internet even if the user had a license
  - Fixed a use-after-free in the AUv3 wrapper
  - Fixed an issue where the channel layout would not be reported correctly in the AUv3 wrapper
  - Fixed a potential memory overrun issue when hosting VST2 plugins with more than eight channels
  - Fixed a problem with the Mac main menu bar showing menus in the wrong position
  - Various Projucer UI fixes
  - Various documentation fixes
  - Various minor improvements and bug fixes

## Version 5.0.1

  - Fixed Windows live build engine on Visual Studio 2017
  - Fixed memory-leak in Projucer live build engine
  - Fixed an issue where you could not paste your redeem serial number with Cmd+V on macOS
  - Fixed an issue where the Projucer would crash on linux due to missing symbols in WebKit
  - Minor Projucer UI improvements
  - Various minor improvements and bug fixes

## Version 5.0.0

  - New licensing model
  - Projucer UI/UX overhaul
  - New look and feel (version 4)
  - New standalone plug-in format
  - Added support for Visual Studio 2017
  - Added support for VST3 SDK 3.6.7
  - Added support for Apple Inter-App Audio on iOS
  - Various Android stability and performance improvements
  - Added support for non-experimental gradle plug-in versions >= 2.2 and Android Studio 2.3
  - Added support for closed-source third-party modules
  - Added support for Windows 10 Bluetooth LE MIDI devices
  - Modernised JUCE codebase to use C++11/14 features
  - Added support for Linux embedded platforms
  - Added support for WebBrowserComponent on Linux
  - Added support for IPv6
  - Various minor improvements and bug fixes
  - Various documentation improvements

## Version 4.3.1

  - Added support for iOS download tasks
  - Added support for AAX plug-in meters
  - Added support for dynamically disabling/enabling sidechains in ProTools
  - Re-introduced support for VST3 plug-ins reporting which VST2 plug-in they can replace
  - Added withRightX and withBottomY methods to Rectangle
  - Added support for windows 10 on screen keyboard
  - Added move semantics to AudioBuffer
  - Added colour coding scheme to module icons in the Projucer to indicate which type of license a module uses
  - Removed all deprecation warnings for macOS Sierra
  - Fixed multiple touch, pen and mouse input related bugs on Windows
  - Added submenu support to ComboBoxes and simplified the usage of ComboBoxes
  - Various minor improvements and bug fixes
  - Various documentation improvements

## Version 4.3.0

  - Added API and examples for ROLI Blocks
  - Multiple Projucer live-build UI and diagnostics improvements
  - JUCE now supports hosting multi-bus plug-ins
  - BufferingAudioSource now supports pre-buffering (useful for offline processing)
  - Added microphone permissions switch to Projucer for iOS targets
  - Standalone wrappers now correctly save and restore midi settings
  - Various performance improvements to BigInteger
  - Fixed various FlexBox bugs
  - Added a workaround for the broken “Open Recent…” menu on os x
  - Various minor improvements and bug fixes
  - Various documentation improvements

## Version 4.2.4

  - Pre-release of live build engine on Windows
  - Added FlexBox layout engine
  - Removed dependency on external Steinberg SDK when building and/or hosting VST2 plug-ins
  - Added support for MIDI network sessions in the iOS simulator
  - Added support for symmetric skew to Slider, NormalisableRange and SliderPropertyComponent
  - Projucer now asks the user what to do when it detects that the .jucer file was modified outside of the Projucer
  - Improved support for Windows 10 touch devices
  - Added begin/end iterator methods for ValueTree, for handy range-based-for loops over its children
  - Added support for recent mingw-w64 compilers
  - Added useful proportional Rectangle utility methods
  - Significantly improved the performance of BigInteger
  - Added support for expiring licenses to juce_tracktion_marketplace
  - Added support for retina mouse cursors on OS X
  - Added a new low-quality mode for the CameraDevice
  - Added pkg-config support for Linux
  - Projucer will now wrap your AAX plug-in in the bundle format expected Pro Tools on Windows
  - Multiple bug-fixes for AudioUnit parameter ids
  - Fixed a bug where AlertWindows weren’t always on top
  - Multiple fixes for web InputStreams
  - Various improvements to the live build engine
  - Various minor improvements and bug fixes
  - Various documentation improvements

## Version 4.2.3

  - Various VST3 improvements: resizing VST3 windows, plug-in compatibility issues
  - Use NSURLSession on newer OS X versions
  - Add compatibility for VST 3 SDK update 3.6.6
  - Miscellaneous fixes and improvements

## Version 4.2.1

  - New class CachedValue, for providing easy and efficient access to ValueTree properties
  - Reduced audio plug-in binary sizes on OS X and added symbol-stripping option
  - Miscellaneous fixes and improvements

## Version 4.2

  - Added support for AudioUnit v3 on OS X and iOS
  - Simplified the JUCE module format. Removed the json module definition files, and made
    it easier to manually add modules to projects. The format is fully described in the
    document juce/modules/JUCE Module Format.txt
  - iOS project support: added custom resource folders, custom xcassets, app capabilities,
    and screen orientation settings.
  - Deleted the Introjucer.. But don't panic! All of its functionality is now supplied by a
    more open-source version of the Projucer. By refactoring the closed-source LLVM compilation
    code into a DLL, we've been able to unify the Introjucer and Projucer into a single
    open-source project. This will allow everyone to compile the Projucer's IDE themselves, and
    having just one app instead of two will make things a lot less confusing!

## Version  4.1

  - Added multi-bus support for audio plug-in clients
  - Added support for MIDI effect plug-ins (AU and AAX).
  - Added new example: Network Graphics Demo

## Version 4.0.3

  - Added MPE (Multidimensional Polyphonic Expression) classes
  - Added full support for generating and parsing Midi RPN/NRPN messages
  - Made the LinearSmoothedValue class public
  - Miscellaneous fixes and minor improvements

## Version 4.0.2

  - Miscellaneous fixes and house-keeping

## Version 4.0.1

  - Initial release of the Projucer!
  - Full OSC support!
  - Android Studio exporting from the Introjucer
  - Android-M pro-audio low-latency i/o support
  - Bluetooth MIDI device support on iOS and Android
  - AudioSampleBuffer refactored into a templated class AudioBuffer, to allow
    32 or 64 bit float support
  - Audio plugin and hosting now supports 64-bit data
  - Support for force-touch and pen pressure on iOS and Windows
  - Added easy sound-file playing methods to AudioDeviceManager
  - Many updates to Introjucer
  - Many new tutorials and examples

## Version 3.3.0

  - New functions for Base64 conversion
  - New command-line options in the introjucer for trimming whitespace and
    replacing tabs in source files

## Version 3.2.0

  - Major OpenGL performance/stability improvements
  - Performance improvements to FloatVectorOperations math functions
  - New FloatVectorOperations: abs, min, max, addWithMultiply, clip
  - Midi channel pressure support
  - New example projects ComponentTutorialExample, SimpleFFTExample,
    PluckedStringsDemo
  - New class ValueTreeSynchroniser, for remote-syncing multiple
    ValueTrees
  - HTTPS/SSL support on Linux
  - Added methods for degrees to radians conversions
  - Added Neon instruction set support for Android targets
  - JUCE ValueTree performance improvements
  - Linux and Android multi-monitor HiDPI support
  - Support the “display=none” attribute in SVG files
  - Support for text elements in SVG files
  - Added Whirlpool hash class to the cryptography module
  - Various improvements for parameter automation in VST, VST-3,
    AudioUnits and AAX
  - Various improvements to JUCE Synthesiser
  - Linux Code::Blocks project support
  - Multicast support
  - Add support to generate project version numbers from project git tags
  - Various updates to example projects
  - Stability improvements to re-order and resize code of plug-in windows
  - Support for external third-party native libraries on Android
  - Introjucer’s auto-update now displays release notes
  - Various Introjucer usability improvements
  - Support for in-memory fonts on Android
  - New FFT class
  - WASAPI exclusive mode support
  - More C++11 feature support macros
  - Performance improvements to XML parsing
  - Add compatibility for AAX SDK 2.2.0
  - Added parameters to the ValueTree::Listener::valueTreeChildRemoved()
    and valueTreeChildOrderChanged() methods to include more info about
    exactly what changed
  - Over 400 minor changes, bug-fixes, documentation improvements, etc.
