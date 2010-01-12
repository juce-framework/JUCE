
project.name = "Amalgamator"
project.bindir = "build"
project.libdir = "build"

project.configs = { "Debug", "Release" }

package = newpackage()
package.name = "Amalgamator"
package.target = "amalgamator"
package.kind = "exe"
package.language = "c++"

package.objdir = "build/intermediate"
package.config["Debug"].objdir   = "build/intermediate/Debug"
package.config["Release"].objdir = "build/intermediate/Release"

package.config["Debug"].defines     = { "LINUX=1", "DEBUG=1", "_DEBUG=1" };
package.config["Debug"].buildoptions = { "-D_DEBUG -ggdb" }

package.config["Release"].defines   = { "LINUX=1", "NDEBUG=1" };

package.includepaths = { 
    "/usr/include",
    "/usr/include/freetype2"
}

package.libpaths = { 
    "/usr/X11R6/lib/"
}

package.config["Debug"].links = { 
    "pthread", "rt", "dl"
}

package.config["Release"].links = { 
    "pthread", "rt", "dl"
}

package.linkflags = { "static-runtime" }

package.files = { matchfiles (
    "../*.h",
    "../*.cpp"
    )
}
