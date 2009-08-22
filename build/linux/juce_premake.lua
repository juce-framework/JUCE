
project.name = "JUCE"
project.bindir = "../../bin"
project.libdir = "../../bin"

project.configs = { "Debug", "Release" }

package = newpackage()
package.name = "JUCE"
package.kind = "lib"
package.language = "c++"

package.objdir = "../../bin/intermediate_linux/"
package.config["Debug"].objdir   = "../../bin/intermediate_linux/Debug"
package.config["Release"].objdir = "../../bin/intermediate_linux/Release"

package.config["Debug"].target   = "juce_debug"
package.config["Release"].target = "juce"

package.config["Debug"].defines         = { "LINUX=1", "DEBUG=1", "_DEBUG=1" };
package.config["Debug"].buildoptions    = { "-D_DEBUG -ggdb -Wall" }

package.config["Release"].defines       = { "LINUX=1", "NDEBUG=1" };
package.config["Release"].buildoptions  = { "-O2 -Wall -fvisibility=hidden" }


package.includepaths = { 
    "../../", 
    "/usr/include",
    "/usr/include/freetype2"
}

package.linkflags = { "static-runtime" }


package.files = { matchfiles (
    "../../src/*.h",
    "../../src/core/*.cpp",
    "../../src/core/*.h",
    "../../src/cryptography/*.cpp",
    "../../src/cryptography/*.h",
    "../../src/containers/*.cpp",
    "../../src/containers/*.h",
    "../../src/io/*.cpp",
    "../../src/io/*.h",
    "../../src/io/files/*.cpp",
    "../../src/io/files/*.h",
    "../../src/io/network/*.cpp",
    "../../src/io/network/*.h",
    "../../src/io/streams/*.cpp",
    "../../src/io/streams/*.h",
    "../../src/text/*.cpp",
    "../../src/text/*.h",
    "../../src/threads/*.cpp",
    "../../src/threads/*.h",
    "../../src/application/*.cpp",
    "../../src/application/*.h",
    "../../src/audio/*.cpp",
    "../../src/audio/*.h",
    "../../src/audio/dsp/*.cpp",
    "../../src/audio/dsp/*.h",
    "../../src/audio/midi/*.cpp",
    "../../src/audio/midi/*.h",
    "../../src/audio/processors/*.cpp",
    "../../src/audio/processors/*.h",
    "../../src/audio/plugins/*.cpp",
    "../../src/audio/plugins/*.h",
    "../../src/audio/plugins/formats/*.cpp",
    "../../src/audio/plugins/formats/*.h",
    "../../src/audio/audio_file_formats/*.cpp",
    "../../src/audio/audio_file_formats/*.h",
    "../../src/audio/audio_sources/*.cpp",
    "../../src/audio/audio_sources/*.h",
    "../../src/audio/devices/*.cpp",
    "../../src/audio/devices/*.h",
    "../../src/audio/synthesisers/*.cpp",
    "../../src/audio/synthesisers/*.h",
    "../../src/events/*.cpp",
    "../../src/events/*.h",
    "../../src/utilities/*.cpp",
    "../../src/utilities/*.h",
    "../../src/gui/graphics/brushes/*.cpp",
    "../../src/gui/graphics/brushes/*.h",
    "../../src/gui/graphics/colour/*.cpp",
    "../../src/gui/graphics/colour/*.h",
    "../../src/gui/graphics/contexts/*.cpp",
    "../../src/gui/graphics/contexts/*.h",
    "../../src/gui/graphics/drawables/*.cpp",
    "../../src/gui/graphics/drawables/*.h",
    "../../src/gui/graphics/effects/*.cpp",
    "../../src/gui/graphics/effects/*.h",
    "../../src/gui/graphics/fonts/*.cpp",
    "../../src/gui/graphics/fonts/*.h",
    "../../src/gui/graphics/geometry/*.cpp",
    "../../src/gui/graphics/geometry/*.h",
    "../../src/gui/graphics/imaging/*.cpp",
    "../../src/gui/graphics/imaging/*.h",
    "../../src/gui/graphics/imaging/image_file_formats/*.cpp",
    "../../src/gui/graphics/imaging/image_file_formats/*.h",
    "../../src/gui/components/*.cpp",
    "../../src/gui/components/*.h",
    "../../src/gui/components/buttons/*.cpp",
    "../../src/gui/components/buttons/*.h",
    "../../src/gui/components/controls/*.cpp",
    "../../src/gui/components/controls/*.h",
    "../../src/gui/components/filebrowser/*.cpp",
    "../../src/gui/components/filebrowser/*.h",
    "../../src/gui/components/keyboard/*.cpp",
    "../../src/gui/components/keyboard/*.h",
    "../../src/gui/components/layout/*.cpp",
    "../../src/gui/components/layout/*.h",
    "../../src/gui/components/lookandfeel/*.cpp",
    "../../src/gui/components/lookandfeel/*.h",
    "../../src/gui/components/menus/*.cpp",
    "../../src/gui/components/menus/*.h",
    "../../src/gui/components/mouse/*.cpp",
    "../../src/gui/components/mouse/*.h",
    "../../src/gui/components/properties/*.cpp",
    "../../src/gui/components/properties/*.h",
    "../../src/gui/components/special/*.cpp",
    "../../src/gui/components/special/*.h",
    "../../src/gui/components/windows/*.cpp",
    "../../src/gui/components/windows/*.h",
    "../../src/native/linux/*.h", 
    "../../src/native/linux/*.cpp", 
    "../../src/native/juce_linux_NativeCode.cpp"
    )
}
