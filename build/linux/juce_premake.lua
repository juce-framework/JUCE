
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
package.config["Release"].buildoptions  = { "-Wall" }


package.includepaths = { 
    "../../", 
    "/usr/include",
    "/usr/include/freetype2"
}

package.linkflags = { "static-runtime" }


package.files = { matchfiles (
    "../../src/*.h",
    "../../src/juce_core/basics/*.cpp",
    "../../src/juce_core/basics/*.h",
    "../../src/juce_core/cryptography/*.cpp",
    "../../src/juce_core/cryptography/*.h",
    "../../src/juce_core/containers/*.cpp",
    "../../src/juce_core/containers/*.h",
    "../../src/juce_core/io/*.cpp",
    "../../src/juce_core/io/*.h",
    "../../src/juce_core/io/files/*.cpp",
    "../../src/juce_core/io/files/*.h",
    "../../src/juce_core/io/network/*.cpp",
    "../../src/juce_core/io/network/*.h",
    "../../src/juce_core/io/streams/*.cpp",
    "../../src/juce_core/io/streams/*.h",
    "../../src/juce_core/io/streams/zlib/*.cpp",
    "../../src/juce_core/io/streams/zlib/*.c",
    "../../src/juce_core/io/streams/zlib/*.h",
    "../../src/juce_core/misc/*.cpp",
    "../../src/juce_core/misc/*.h",
    "../../src/juce_core/text/*.cpp",
    "../../src/juce_core/text/*.h",
    "../../src/juce_core/threads/*.cpp",
    "../../src/juce_core/threads/*.h",
    "../../src/juce_appframework/application/*.cpp",
    "../../src/juce_appframework/application/*.h",
    "../../src/juce_appframework/audio/*.cpp",
    "../../src/juce_appframework/audio/*.h",
    "../../src/juce_appframework/audio/dsp/*.cpp",
    "../../src/juce_appframework/audio/dsp/*.h",
    "../../src/juce_appframework/audio/midi/*.cpp",
    "../../src/juce_appframework/audio/midi/*.h",
    "../../src/juce_appframework/audio/audio_file_formats/*.cpp",
    "../../src/juce_appframework/audio/audio_file_formats/*.h",
    "../../src/juce_appframework/audio/audio_file_formats/flac/libFLAC/*.c",
    "../../src/juce_appframework/audio/audio_file_formats/flac/libFLAC/*.h",
    "../../src/juce_appframework/audio/audio_file_formats/oggvorbis/*.c",
    "../../src/juce_appframework/audio/audio_file_formats/oggvorbis/*.h",
    "../../src/juce_appframework/audio/audio_file_formats/oggvorbis/libvorbis-1.1.2/lib/*.c",
    "../../src/juce_appframework/audio/audio_file_formats/oggvorbis/libvorbis-1.1.2/lib/*.h",
    "../../src/juce_appframework/audio/audio_sources/*.cpp",
    "../../src/juce_appframework/audio/audio_sources/*.h",
    "../../src/juce_appframework/audio/devices/*.cpp",
    "../../src/juce_appframework/audio/devices/*.h",
    "../../src/juce_appframework/audio/synthesisers/*.cpp",
    "../../src/juce_appframework/audio/synthesisers/*.h",
    "../../src/juce_appframework/documents/*.cpp",
    "../../src/juce_appframework/documents/*.h",
    "../../src/juce_appframework/events/*.cpp",
    "../../src/juce_appframework/events/*.h",
    "../../src/juce_appframework/gui/graphics/brushes/*.cpp",
    "../../src/juce_appframework/gui/graphics/brushes/*.h",
    "../../src/juce_appframework/gui/graphics/colour/*.cpp",
    "../../src/juce_appframework/gui/graphics/colour/*.h",
    "../../src/juce_appframework/gui/graphics/contexts/*.cpp",
    "../../src/juce_appframework/gui/graphics/contexts/*.h",
    "../../src/juce_appframework/gui/graphics/drawables/*.cpp",
    "../../src/juce_appframework/gui/graphics/drawables/*.h",
    "../../src/juce_appframework/gui/graphics/effects/*.cpp",
    "../../src/juce_appframework/gui/graphics/effects/*.h",
    "../../src/juce_appframework/gui/graphics/fonts/*.cpp",
    "../../src/juce_appframework/gui/graphics/fonts/*.h",
    "../../src/juce_appframework/gui/graphics/geometry/*.cpp",
    "../../src/juce_appframework/gui/graphics/geometry/*.h",
    "../../src/juce_appframework/gui/graphics/imaging/*.cpp",
    "../../src/juce_appframework/gui/graphics/imaging/*.h",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/*.cpp",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/*.h",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/jpglib/*.cpp",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/jpglib/*.c",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/jpglib/*.h",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/pnglib/*.cpp",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/pnglib/*.c",
    "../../src/juce_appframework/gui/graphics/imaging/image_file_formats/pnglib/*.h",
    "../../src/juce_appframework/gui/components/*.cpp",
    "../../src/juce_appframework/gui/components/*.h",
    "../../src/juce_appframework/gui/components/buttons/*.cpp",
    "../../src/juce_appframework/gui/components/buttons/*.h",
    "../../src/juce_appframework/gui/components/controls/*.cpp",
    "../../src/juce_appframework/gui/components/controls/*.h",
    "../../src/juce_appframework/gui/components/filebrowser/*.cpp",
    "../../src/juce_appframework/gui/components/filebrowser/*.h",
    "../../src/juce_appframework/gui/components/keyboard/*.cpp",
    "../../src/juce_appframework/gui/components/keyboard/*.h",
    "../../src/juce_appframework/gui/components/layout/*.cpp",
    "../../src/juce_appframework/gui/components/layout/*.h",
    "../../src/juce_appframework/gui/components/lookandfeel/*.cpp",
    "../../src/juce_appframework/gui/components/lookandfeel/*.h",
    "../../src/juce_appframework/gui/components/menus/*.cpp",
    "../../src/juce_appframework/gui/components/menus/*.h",
    "../../src/juce_appframework/gui/components/mouse/*.cpp",
    "../../src/juce_appframework/gui/components/mouse/*.h",
    "../../src/juce_appframework/gui/components/properties/*.cpp",
    "../../src/juce_appframework/gui/components/properties/*.h",
    "../../src/juce_appframework/gui/components/special/*.cpp",
    "../../src/juce_appframework/gui/components/special/*.h",
    "../../src/juce_appframework/gui/components/windows/*.cpp",
    "../../src/juce_appframework/gui/components/windows/*.h",
    "platform_specific_code/*.h", 
    "platform_specific_code/*.cpp"
    )
}
