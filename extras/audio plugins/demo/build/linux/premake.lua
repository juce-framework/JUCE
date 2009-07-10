
project.name = "JucePluginDemo"
project.bindir = "build"
project.libdir = "build"

project.configs = { "Debug", "Release" }

package = newpackage()
package.name = "JucePluginDemo"
package.kind = "dll"
package.language = "c++"

package.objdir = "build/intermediate"
package.config["Debug"].objdir   = "build/intermediate/Debug"
package.config["Release"].objdir = "build/intermediate/Release"

package.config["Debug"].defines         = { "LINUX=1", "DEBUG=1", "_DEBUG=1" }
package.config["Debug"].buildoptions    = { "-O0 -ggdb -g3 -Wall" }

package.config["Release"].defines       = { "LINUX=1", "NDEBUG=1" }
package.config["Release"].buildoptions  = { "-O2 -s" }

package.includepaths = {
    "/usr/include",
    "../../../../../",
    "/usr/include/vst/source/common",
    "/usr/include/vst",
    "/usr/include/freetype2",
    "../../src"
}

package.libpaths = {
    "../../../../../bin",
    "/usr/X11R6/lib/",
    "/usr/lib"
}

package.config["Debug"].links = {
    "freetype", "pthread", "rt", "X11", "GL", "GLU", "Xinerama", "asound", "juce_debug"
}

package.config["Release"].links = {
    "freetype", "pthread", "rt", "X11", "GL", "GLU", "Xinerama", "asound", "juce"
}

package.files = {
    matchfiles (
        "../../../wrapper/*.h",
        "../../../wrapper/*.cpp",
        "../../../wrapper/VST/*.cpp"
    ),
    matchrecursive (
        "../../src/*.h",
        "../../src/*.cpp"
    )
}
