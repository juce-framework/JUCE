# JUCE 6 Technical Preview

__This branch is a technical preview of JUCE 6. It provides the opportunity to
try out the new features and provide any feedback on how they can best work for
you before the official release. Please use [this
topic](https://forum.juce.com/t/juce6-technical-preview-branch/38699) on the
JUCE forum to discuss the new features.__

## Getting Started with CMake

For comprehensive documentation on JUCE's CMake API, see
`examples/CMake/readme.md` in this repo.

### Building Examples

To build the examples and extras bundled with JUCE, simply clone JUCE and then
run:

```
# Go to JUCE directory
cd /path/to/cloned/JUCE
# Configure build with all examples and extras enabled
cmake -B cmake-build -DJUCE_BUILD_EXAMPLES=ON -DJUCE_BUILD_EXTRAS=ON
# Build a specific target (building everything will take a long time!)
cmake --build cmake-build --target DemoRunner
```

### Using `add_subdirectory`

The simplest way to include JUCE in your project is to add JUCE as a
subdirectory of your project, and to include the line `add_subdirectory(JUCE)`
in your project CMakeLists.txt. This will make the JUCE targets and helper
functions available for use by your custom targets.

### Using `find_package`

To install JUCE globally on your system, you'll need to tell CMake where to
place the installed files. As this is a preview branch, we recommend that you
*avoid* installing to the default install location, and instead choose a path
that you can easily delete and recreate when installing newer preview versions.

```
# Go to JUCE directory
cd /path/to/clone/JUCE
# Configure build with library components only
cmake -B cmake-build-install -DCMAKE_INSTALL_PREFIX=/path/to/JUCE/install
# Run the installation
cmake --build cmake-build-install --target install
```

Make sure the dependent project CMakeLists.txt contains the line
`find_package(JUCE CONFIG REQUIRED)`. This will make the JUCE modules and CMake
helper functions available for use in the rest of your build. Then, run the
build like so:

```
# Go to project directory
cd /path/to/my/project
# Configure build, passing the JUCE install path you used earlier
cmake -B cmake-build -DCMAKE_PREFIX_PATH=/path/to/JUCE/install
# Build the project
cmake --build cmake-build
```
