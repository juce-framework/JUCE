
project.name = "Jucer"
project.bindir = "build"
project.libdir = "build"

project.configs = { "Debug", "Release" }

package = newpackage()
package.name = "Jucer"
package.kind = "winexe"
package.language = "c++"

package.objdir = "build/intermediate"
package.config["Debug"].objdir   = "build/intermediate/Debug"
package.config["Release"].objdir = "build/intermediate/Release"

package.config["Debug"].defines     = { "LINUX=1", "DEBUG=1", "_DEBUG=1" };
package.config["Debug"].buildoptions = { "-D_DEBUG -ggdb" }

package.config["Release"].defines   = { "LINUX=1", "NDEBUG=1" };

package.target = "jucer"

package.includepaths = { 
    "/usr/include",
    "/usr/include/freetype2"
}

package.libpaths = { 
    "/usr/X11R6/lib/",
    "../../../../bin"
}

package.config["Debug"].links = { 
    "freetype", "pthread", "X11", "GL", "GLU", "Xinerama", "asound"
}

package.config["Release"].links = { 
    "freetype", "pthread", "X11", "GL", "GLU", "Xinerama", "asound"
}

package.linkflags = { "static-runtime" }

package.files = { matchfiles (
    "../../src/*.h",
    "../../src/*.cpp",
    "../../src/ui/*.h",
    "../../src/ui/*.cpp",
    "../../src/utility/*.h",
    "../../src/utility/*.cpp",
    "../../src/properties/*.h",
    "../../src/properties/*.cpp",
    "../../src/model/*.h",
    "../../src/model/*.cpp",
    "../../src/model/components/*.h",
    "../../src/model/components/*.cpp",
    "../../src/model/documents/*.h",
    "../../src/model/documents/*.cpp",
    "../../src/model/paintelements/*.h",
    "../../src/model/paintelements/*.cpp"
    )
}
