/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef __JUCER_PROJECTEXPORT_XCODE_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_XCODE_JUCEHEADER__

#include "jucer_ProjectExporter.h"


//==============================================================================
class XCodeProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameMac()                         { return "XCode (MacOSX)"; }
    static const char* getNameiOS()                         { return "XCode (iOS)"; }
    static const char* getValueTreeTypeName (bool iPhone)   { return iPhone ? "XCODE_IPHONE" : "XCODE_MAC"; }

    //==============================================================================
    XCodeProjectExporter (Project& project_, const ValueTree& settings_, const bool iPhone_)
        : ProjectExporter (project_, settings_),
          iPhone (iPhone_)
    {
        name = iPhone ? getNameiOS() : getNameMac();

        projectIDSalt = hashCode64 (project.getProjectUID());

        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = getDefaultBuildsRootFolder() + (iPhone ? "iOS" : "MacOSX");

        if (getVSTFolder().toString().isEmpty())
            getVSTFolder() = "~/SDKs/vstsdk2.4";

        if (getRTASFolder().toString().isEmpty())
            getRTASFolder() = "~/SDKs/PT_80_SDK";
    }

    ~XCodeProjectExporter()
    {
    }

    static XCodeProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (false)))
            return new XCodeProjectExporter (project, settings, false);
        else if (settings.hasType (getValueTreeTypeName (true)))
            return new XCodeProjectExporter (project, settings, true);

        return 0;
    }

    //==============================================================================
    bool isDefaultFormatForCurrentOS()
    {
      #if JUCE_MAC
        return ! iPhone;
      #else
        return false;
      #endif
    }

    bool isPossibleForCurrentProject()      { return project.isGUIApplication() || ! iPhone; }
    bool usesMMFiles() const                { return true; }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);

        props.add (new TextPropertyComponent (getSetting ("objCExtraSuffix"), "Objective-C class name suffix", 64, false));
        props.getLast()->setTooltip ("Because objective-C linkage is done by string-matching, you can get horrible linkage mix-ups when different modules containing the "
                                     "same class-names are loaded simultaneously. This setting lets you provide a unique string that will be used in naming the obj-C classes in your executable to avoid this.");

        if (project.isGUIApplication() && ! iPhone)
        {
            props.add (new TextPropertyComponent (getSetting ("documentExtensions"), "Document file extensions", 128, false));
            props.getLast()->setTooltip ("A comma-separated list of file extensions for documents that your app can open.");
        }
    }

    void launchProject()
    {
        getProjectBundle().startAsProcess();
    }

    //==============================================================================
    const String create()
    {
        infoPlistFile = getTargetFolder().getChildFile ("Info.plist");

        if (! createIconFile())
            return "Can't write the icon file";

        File projectBundle (getProjectBundle());
        if (! projectBundle.createDirectory())
            return "Can't write to the target directory";

        createObjects();

        File projectFile (projectBundle.getChildFile ("project.pbxproj"));

        {
            MemoryOutputStream mo;
            writeProjectFile (mo);

            if (! FileHelpers::overwriteFileWithNewDataIfDifferent (projectFile, mo))
                return "Can't write to file: " + projectFile.getFullPathName();
        }

        if (! writeInfoPlistFile())
            return "Can't write the Info.plist file";

        return String::empty;
    }

private:
    OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, groups, misc, projectConfigs, targetConfigs;
    StringArray buildPhaseIDs, resourceIDs, sourceIDs, frameworkIDs;
    StringArray frameworkFileIDs, rezFileIDs, resourceFileRefs;
    File infoPlistFile, iconFile;
    int64 projectIDSalt;
    const bool iPhone;

    static const String sanitisePath (const String& path)
    {
        if (path.startsWithChar ('~'))
            return "$(HOME)" + path.substring (1);

        return path;
    }

    const File getProjectBundle() const         { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (".xcodeproj"); }

    bool hasPList() const                               { return ! (project.isLibrary() || project.isCommandLineApp()); }
    const String getAudioPluginBundleExtension() const  { return "component"; }

    //==============================================================================
    void createObjects()
    {
        if (! project.isLibrary())
            addFrameworks();

        const String productName (project.getConfiguration (0).getTargetBinaryName().toString());
        if (project.isGUIApplication())
            addBuildProduct ("wrapper.application", productName + ".app");
        else if (project.isCommandLineApp())
            addBuildProduct ("compiled.mach-o.executable", productName);
        else if (project.isLibrary())
            addBuildProduct ("archive.ar", getLibbedFilename (productName));
        else if (project.isAudioPlugin())
            addBuildProduct ("wrapper.cfbundle", productName + "." + getAudioPluginBundleExtension());
        else if (project.isBrowserPlugin())
            addBuildProduct ("wrapper.cfbundle", productName + ".plugin");
        else
            jassert (productName.isEmpty());

        if (hasPList())
        {
            RelativePath plistPath (infoPlistFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (plistPath);
            resourceFileRefs.add (createID (plistPath));
        }

        if (iconFile.exists())
        {
            RelativePath iconPath (iconFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (iconPath);
            resourceIDs.add (addBuildFile (iconPath, false, false));
            resourceFileRefs.add (createID (iconPath));
        }

        addProjectItem (project.getMainGroup());

        for (int i = 0; i < project.getNumConfigurations(); ++i)
        {
            Project::BuildConfiguration config (project.getConfiguration (i));

            addProjectConfig (config.getName().getValue(), getProjectSettings (config));
            addTargetConfig (config.getName().getValue(), getTargetSettings (config));
        }

        addConfigList (projectConfigs, createID ("__projList"));
        addConfigList (targetConfigs, createID ("__configList"));

        if (! project.isLibrary())
            addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

        if (rezFileIDs.size() > 0)
            addBuildPhase ("PBXRezBuildPhase", rezFileIDs);

        addBuildPhase ("PBXSourcesBuildPhase", sourceIDs);

        if (! project.isLibrary())
            addBuildPhase ("PBXFrameworksBuildPhase", frameworkIDs);

        if (project.isAudioPlugin())
            addPluginShellScriptPhase();

        addTargetObject();
        addProjectObject();
    }

    static const Image fixMacIconImageSize (Image& image)
    {
        const int w = image.getWidth();
        const int h = image.getHeight();

        if (w != h || (w != 16 && w != 32 && w != 48 && w != 64))
        {
            const int newSize = w >= 128 ? 128 : (w >= 64 ? 64 : (w >= 32 ? 32 : 16));
            Image newIm (Image::ARGB, newSize, newSize, true);
            Graphics g (newIm);
            g.drawImageWithin (image, 0, 0, newSize, newSize,
                               RectanglePlacement::centred | RectanglePlacement::onlyReduceInSize, false);
            return newIm;
        }

        return image;
    }

    void writeIcnsFile (const Array<Image>& images, OutputStream& out)
    {
        MemoryOutputStream data;

        for (int i = 0; i < images.size(); ++i)
        {
            Image image (fixMacIconImageSize (images.getReference (i)));

            const int w = image.getWidth();
            const int h = image.getHeight();

            const char* type = 0;
            const char* maskType = 0;

            if (w == h)
            {
                if (w == 16)  { type = "is32"; maskType = "s8mk"; }
                if (w == 32)  { type = "il32"; maskType = "l8mk"; }
                if (w == 48)  { type = "ih32"; maskType = "h8mk"; }
                if (w == 128) { type = "it32"; maskType = "t8mk"; }
            }

            if (type != 0)
            {
                data.write (type, 4);
                data.writeIntBigEndian (8 + 4 * w * h);

                const Image::BitmapData bitmap (image, false);

                int y;
                for (y = 0; y < h; ++y)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        const Colour pixel (bitmap.getPixelColour (x, y));
                        data.writeByte ((char) pixel.getAlpha());
                        data.writeByte ((char) pixel.getRed());
                        data.writeByte ((char) pixel.getGreen());
                        data.writeByte ((char) pixel.getBlue());
                    }
                }

                data.write (maskType, 4);
                data.writeIntBigEndian (8 + w * h);

                for (y = 0; y < h; ++y)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        const Colour pixel (bitmap.getPixelColour (x, y));
                        data.writeByte ((char) pixel.getAlpha());
                    }
                }
            }
        }

        jassert (data.getDataSize() > 0); // no suitable sized images?

        out.write ("icns", 4);
        out.writeIntBigEndian (data.getDataSize() + 8);
        out << data;
    }

    bool createIconFile()
    {
        Array<Image> images;

        Image bigIcon (project.getBigIcon());
        if (bigIcon.isValid())
            images.add (bigIcon);

        Image smallIcon (project.getSmallIcon());
        if (smallIcon.isValid())
            images.add (smallIcon);

        if (images.size() == 0)
            return true;

        MemoryOutputStream mo;
        writeIcnsFile (images, mo);

        iconFile = getTargetFolder().getChildFile ("Icon.icns");
        return FileHelpers::overwriteFileWithNewDataIfDifferent (iconFile, mo);
    }

    bool writeInfoPlistFile()
    {
        if (! hasPList())
            return true;

        XmlElement plist ("plist");
        XmlElement* dict = plist.createNewChildElement ("dict");

        addPlistDictionaryKey (dict, "CFBundleExecutable",          "${EXECUTABLE_NAME}");
        addPlistDictionaryKey (dict, "CFBundleIconFile",            iconFile.exists() ? iconFile.getFileName() : String::empty);
        addPlistDictionaryKey (dict, "CFBundleIdentifier",          project.getBundleIdentifier().toString());
        addPlistDictionaryKey (dict, "CFBundleName",                project.getProjectName().toString());

        if (project.isAudioPlugin())
        {
            addPlistDictionaryKey (dict, "CFBundlePackageType",     "TDMw");
            addPlistDictionaryKey (dict, "CFBundleSignature",       "PTul");
        }
        else
        {
            addPlistDictionaryKey (dict, "CFBundlePackageType",     "APPL");
            addPlistDictionaryKey (dict, "CFBundleSignature",       "????");
        }

        addPlistDictionaryKey (dict, "CFBundleShortVersionString",  project.getVersion().toString());
        addPlistDictionaryKey (dict, "CFBundleVersion",             project.getVersion().toString());

        StringArray documentExtensions;
        documentExtensions.addTokens (replacePreprocessorDefs (project.getPreprocessorDefs(), getSetting ("documentExtensions").toString()),
                                      ",", String::empty);
        documentExtensions.trim();
        documentExtensions.removeEmptyStrings (true);

        if (documentExtensions.size() > 0)
        {
            dict->createNewChildElement ("key")->addTextElement ("CFBundleDocumentTypes");
            XmlElement* dict2 = dict->createNewChildElement ("array")->createNewChildElement ("dict");

            for (int i = 0; i < documentExtensions.size(); ++i)
            {
                String ex (documentExtensions[i]);
                if (ex.startsWithChar ('.'))
                    ex = ex.substring (1);

                dict2->createNewChildElement ("key")->addTextElement ("CFBundleTypeExtensions");
                dict2->createNewChildElement ("array")->createNewChildElement ("string")->addTextElement (ex);
                addPlistDictionaryKey (dict2, "CFBundleTypeName", ex);
                addPlistDictionaryKey (dict2, "CFBundleTypeRole", "Editor");
                addPlistDictionaryKey (dict2, "NSPersistentStoreTypeKey", "XML");
            }
        }

        MemoryOutputStream mo;
        plist.writeToStream (mo, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");

        return FileHelpers::overwriteFileWithNewDataIfDifferent (infoPlistFile, mo);
    }

    const StringArray getHeaderSearchPaths (const Project::BuildConfiguration& config)
    {
        StringArray searchPaths (config.getHeaderSearchPaths());

        if (project.shouldAddVSTFolderToPath() && getVSTFolder().toString().isNotEmpty())
            searchPaths.add (rebaseFromProjectFolderToBuildTarget (RelativePath (getVSTFolder().toString(), RelativePath::projectFolder)).toUnixStyle());

        if (project.isAudioPlugin())
        {
            if (isAU())
            {
                searchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/PublicUtility");
                searchPaths.add ("$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/Utility");
            }

            if (isRTAS())
            {
                searchPaths.add ("/Developer/Headers/FlatCarbon");

                static const char* rtasIncludePaths[] = { "AlturaPorts/TDMPlugIns/PlugInLibrary/Controls",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/CoreClasses",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/DSPClasses",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/EffectClasses",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/MacBuild",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/Meters",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/ProcessClasses/Interfaces",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/RTASP_Adapt",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/Utilities",
                                                          "AlturaPorts/TDMPlugIns/PlugInLibrary/ViewClasses",
                                                          "AlturaPorts/TDMPlugIns/DSPManager/**",
                                                          "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/Encryption",
                                                          "AlturaPorts/TDMPlugIns/SupplementalPlugInLib/GraphicsExtensions",
                                                          "AlturaPorts/TDMPlugIns/common",
                                                          "AlturaPorts/TDMPlugIns/common/PI_LibInterface",
                                                          "AlturaPorts/TDMPlugIns/PACEProtection/**",
                                                          "AlturaPorts/TDMPlugIns/SignalProcessing/**",
                                                          "AlturaPorts/OMS/Headers",
                                                          "AlturaPorts/Fic/Interfaces/**",
                                                          "AlturaPorts/Fic/Source/SignalNets",
                                                          "AlturaPorts/DSIPublicInterface/PublicHeaders",
                                                          "DAEWin/Include",
                                                          "AlturaPorts/DigiPublic/Interfaces",
                                                          "AlturaPorts/DigiPublic",
                                                          "AlturaPorts/NewFileLibs/DOA",
                                                          "AlturaPorts/NewFileLibs/Cmn",
                                                          "xplat/AVX/avx2/avx2sdk/inc",
                                                          "xplat/AVX/avx2/avx2sdk/utils" };

                RelativePath sdkFolder (getRTASFolder().toString(), RelativePath::projectFolder);

                for (int i = 0; i < numElementsInArray (rtasIncludePaths); ++i)
                    searchPaths.add (rebaseFromProjectFolderToBuildTarget (sdkFolder.getChildFile (rtasIncludePaths[i])).toUnixStyle());
            }
        }

        return searchPaths;
    }

    void getLinkerFlagsForStaticLibrary (const RelativePath& library, StringArray& flags, StringArray& librarySearchPaths)
    {
        jassert (library.getFileNameWithoutExtension().substring (0, 3) == "lib");

        flags.add ("-l" + library.getFileNameWithoutExtension().substring (3));

        String searchPath (library.toUnixStyle().upToLastOccurrenceOf ("/", false, false));
        if (! library.isAbsolute())
            searchPath = "$(SRCROOT)/" + searchPath;

        librarySearchPaths.add (sanitisePath (searchPath));
    }

    void getLinkerFlags (const Project::BuildConfiguration& config, StringArray& flags, StringArray& librarySearchPaths)
    {
        if (project.isAudioPlugin())
        {
            flags.add ("-bundle");

            if (isRTAS() && getRTASFolder().toString().isNotEmpty())
            {
                getLinkerFlagsForStaticLibrary (RelativePath (getRTASFolder().toString(), RelativePath::buildTargetFolder)
                                                    .getChildFile (config.isDebug().getValue() ? "MacBag/Libs/Debug/libPluginLibrary.a"
                                                                                               : "MacBag/Libs/Release/libPluginLibrary.a"),
                                                flags, librarySearchPaths);
            }
        }

        if (project.getJuceLinkageMode() == Project::useLinkedJuce)
        {
            RelativePath juceLib (getJucePathFromTargetFolder().getChildFile (config.isDebug().getValue() ? "bin/libjucedebug.a"
                                                                                                          : "bin/libjuce.a"));
            getLinkerFlagsForStaticLibrary (juceLib, flags, librarySearchPaths);
        }

        flags.add (replacePreprocessorTokens (config, getExtraLinkerFlags().toString()));
        flags.removeEmptyStrings (true);
    }

    const StringArray getProjectSettings (const Project::BuildConfiguration& config)
    {
        StringArray s;
        s.add ("ALWAYS_SEARCH_USER_PATHS = NO");
        s.add ("GCC_C_LANGUAGE_STANDARD = c99");
        s.add ("GCC_WARN_ABOUT_RETURN_TYPE = YES");
        s.add ("GCC_WARN_CHECK_SWITCH_STATEMENTS = YES");
        s.add ("GCC_WARN_UNUSED_VARIABLE = YES");
        s.add ("GCC_WARN_MISSING_PARENTHESES = YES");
        s.add ("GCC_WARN_NON_VIRTUAL_DESTRUCTOR = YES");
        s.add ("GCC_WARN_TYPECHECK_CALLS_TO_PRINTF = YES");
        s.add ("WARNING_CFLAGS = -Wreorder");
        s.add ("GCC_MODEL_TUNING = G5");

        if (project.isLibrary() || project.getJuceLinkageMode() == Project::useLinkedJuce)
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = NO");
        }
        else
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = YES");
        }

        s.add ("ZERO_LINK = NO");

        if (! isRTAS())   // (dwarf seems to be incompatible with the RTAS libs)
            s.add ("DEBUG_INFORMATION_FORMAT = \"dwarf\"");

        s.add ("PRODUCT_NAME = \"" + config.getTargetBinaryName().toString() + "\"");
        return s;
    }

    const StringArray getTargetSettings (const Project::BuildConfiguration& config)
    {
        StringArray s;
        s.add ("ARCHS = \"$(ARCHS_STANDARD_32_BIT)\"");
        s.add ("PREBINDING = NO");
        s.add ("HEADER_SEARCH_PATHS = \"" + replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (" ")) + " $(inherited)\"");
        s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());
        s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

        const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlags().toString()).trim());
        if (extraFlags.isNotEmpty())
            s.add ("OTHER_CPLUSPLUSFLAGS = " + extraFlags);

        if (project.isGUIApplication())
        {
            s.add ("INSTALL_PATH = \"$(HOME)/Applications\"");
        }
        else if (project.isAudioPlugin())
        {
            s.add ("LIBRARY_STYLE = Bundle");
            s.add ("INSTALL_PATH = \"$(HOME)/Library/Audio/Plug-Ins/Components/\"");
            s.add ("WRAPPER_EXTENSION = " + getAudioPluginBundleExtension());
            s.add ("GENERATE_PKGINFO_FILE = YES");
            s.add ("OTHER_REZFLAGS = \"-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                   " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                   " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"\"");
        }
        else if (project.isBrowserPlugin())
        {
            s.add ("LIBRARY_STYLE = Bundle");
            s.add ("INSTALL_PATH = \"/Library/Internet Plug-Ins/\"");
        }
        else if (project.isLibrary())
        {
            if (config.getTargetBinaryRelativePath().toString().isNotEmpty())
            {
                RelativePath binaryPath (config.getTargetBinaryRelativePath().toString(), RelativePath::projectFolder);
                binaryPath = binaryPath.rebased (project.getFile().getParentDirectory(), getTargetFolder(), RelativePath::buildTargetFolder);

                s.add ("DSTROOT = " + sanitisePath (binaryPath.toUnixStyle()));
                s.add ("SYMROOT = " + sanitisePath (binaryPath.toUnixStyle()));
            }

            s.add ("CONFIGURATION_BUILD_DIR = \"$(BUILD_DIR)\"");
            s.add ("DEPLOYMENT_LOCATION = YES");
        }
        else if (project.isCommandLineApp())
        {
        }
        else
        {
            jassertfalse;
        }

        if (iPhone)
        {
            s.add ("SDKROOT = iphonesimulator3.2");
        }
        else
        {
            const String sdk (config.getMacSDKVersion().toString());
            const String sdkCompat (config.getMacCompatibilityVersion().toString());

            if (sdk == Project::BuildConfiguration::osxVersion10_4)
            {
                s.add ("SDKROOT = macosx10.4");
                s.add ("GCC_VERSION = 4.0");
            }
            else if (sdk == Project::BuildConfiguration::osxVersion10_5)
            {
                s.add ("SDKROOT = macosx10.5");
            }
            else if (sdk == Project::BuildConfiguration::osxVersion10_6)
            {
                s.add ("SDKROOT = macosx10.6");
            }

            if (sdkCompat == Project::BuildConfiguration::osxVersion10_4)
                s.add ("MACOSX_DEPLOYMENT_TARGET = 10.4");
            else if (sdkCompat == Project::BuildConfiguration::osxVersion10_5)
                s.add ("MACOSX_DEPLOYMENT_TARGET = 10.5");
            else if (sdkCompat == Project::BuildConfiguration::osxVersion10_6)
                s.add ("MACOSX_DEPLOYMENT_TARGET = 10.6");

            s.add ("MACOSX_DEPLOYMENT_TARGET_ppc = 10.4");
        }

        {
            StringArray linkerFlags, librarySearchPaths;
            getLinkerFlags (config, linkerFlags, librarySearchPaths);

            if (linkerFlags.size() > 0)
                s.add ("OTHER_LDFLAGS = \"" + linkerFlags.joinIntoString (" ") + "\"");

            if (librarySearchPaths.size() > 0)
            {
                String libPaths ("LIBRARY_SEARCH_PATHS = (\"$(inherited)\"");

                for (int i = 0; i < librarySearchPaths.size(); ++i)
                    libPaths += ", \"\\\"" + librarySearchPaths[i] + "\\\"\"";

                s.add (libPaths + ")");
            }
        }

        StringPairArray defines;

        if (config.isDebug().getValue())
        {
            defines.set ("_DEBUG", "1");
            defines.set ("DEBUG", "1");
            s.add ("ONLY_ACTIVE_ARCH = YES");
            s.add ("COPY_PHASE_STRIP = NO");
            s.add ("GCC_DYNAMIC_NO_PIC = NO");
            s.add ("GCC_ENABLE_FIX_AND_CONTINUE = NO");
        }
        else
        {
            defines.set ("_NDEBUG", "1");
            defines.set ("NDEBUG", "1");
            s.add ("GCC_GENERATE_DEBUGGING_SYMBOLS = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = YES");
        }

        {
            const String objCSuffix (getSetting ("objCExtraSuffix").toString().trim());
            if (objCSuffix.isNotEmpty())
                defines.set ("JUCE_ObjCExtraSuffix", replacePreprocessorTokens (config, objCSuffix));
        }

        {
            defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

            StringArray defsList;

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines.getAllKeys()[i]);
                const String value (defines.getAllValues()[i]);
                if (value.isNotEmpty())
                    def << "=" << value;

                defsList.add (def.quoted());
            }

            s.add ("GCC_PREPROCESSOR_DEFINITIONS = (" + indentList (defsList, ",") + ")");
        }

        return s;
    }

    void addFrameworks()
    {
        StringArray s;

        if (iPhone)
        {
            s.addTokens ("UIKit Foundation CoreGraphics AudioToolbox QuartzCore OpenGLES", false);
        }
        else
        {
            s.addTokens ("Cocoa Carbon IOKit CoreAudio CoreMIDI WebKit DiscRecording OpenGL QuartzCore QTKit QuickTime", false);

            if (isAU())
                s.addTokens ("AudioUnit CoreAudioKit AudioToolbox", false);
            else if (project.getJuceConfigFlag ("JUCE_PLUGINHOST_AU").toString() == Project::configFlagEnabled)
                s.addTokens ("AudioUnit CoreAudioKit", false);
        }

        for (int i = 0; i < s.size(); ++i)
            addFramework (s[i]);
    }

    //==============================================================================
    void writeProjectFile (OutputStream& output)
    {
        output << "// !$*UTF8*$!\n{\n"
                  "\tarchiveVersion = 1;\n"
                  "\tclasses = {\n\t};\n"
                  "\tobjectVersion = 44;\n"
                  "\tobjects = {\n\n";

        Array <ValueTree*> objects;
        objects.addArray (pbxBuildFiles);
        objects.addArray (pbxFileReferences);
        objects.addArray (groups);
        objects.addArray (targetConfigs);
        objects.addArray (projectConfigs);
        objects.addArray (misc);

        for (int i = 0; i < objects.size(); ++i)
        {
            ValueTree& o = *objects.getUnchecked(i);
            output << "\t\t" << static_cast <const juce_wchar*> (o.getType()) << " = { ";

            for (int j = 0; j < o.getNumProperties(); ++j)
            {
                const Identifier propertyName (o.getPropertyName(j));
                String val (o.getProperty (propertyName).toString());

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,-\r\n")
                                        && ! (val.trimStart().startsWithChar ('(')
                                                || val.trimStart().startsWithChar ('{'))))
                    val = val.quoted();

                output << propertyName.toString() << " = " << val << "; ";
            }

            output << "};\n";
        }

        output << "\t};\n\trootObject = " << createID ("__root") << ";\n}\n";
    }

    static void addPlistDictionaryKey (XmlElement* xml, const String& key, const String& value)
    {
        xml->createNewChildElement ("key")->addTextElement (key);
        xml->createNewChildElement ("string")->addTextElement (value);
    }

    const String addBuildFile (const RelativePath& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings)
    {
        String fileID (createID (path.toUnixStyle() + "buildref"));

        if (addToSourceBuildPhase)
            sourceIDs.add (fileID);

        ValueTree* v = new ValueTree (fileID);
        v->setProperty ("isa", "PBXBuildFile", 0);
        v->setProperty ("fileRef", fileRefID, 0);

        if (inhibitWarnings)
            v->setProperty ("settings", "{COMPILER_FLAGS = \"-w\"; }", 0);

        pbxBuildFiles.add (v);
        return fileID;
    }

    const String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings)
    {
        return addBuildFile (path, createID (path), addToSourceBuildPhase, inhibitWarnings);
    }

    void addFileReference (const RelativePath& path, const String& sourceTree, const String& lastKnownFileType, const String& fileRefID)
    {
        ValueTree* v = new ValueTree (fileRefID);
        v->setProperty ("isa", "PBXFileReference", 0);
        v->setProperty ("lastKnownFileType", lastKnownFileType, 0);
        v->setProperty (Ids::name, path.getFileName(), 0);
        v->setProperty ("path", sanitisePath (path.toUnixStyle()), 0);
        v->setProperty ("sourceTree", sourceTree, 0);
        pbxFileReferences.add (v);
    }

    const String addFileReference (const RelativePath& path)
    {
        const String fileRefID (createID (path));

        jassert (path.isAbsolute() || path.getRoot() == RelativePath::buildTargetFolder);
        addFileReference (path, path.isAbsolute() ? "<absolute>" : "SOURCE_ROOT",
                          getFileType (path), fileRefID);

        return fileRefID;
    }

    static const String getFileType (const RelativePath& file)
    {
        if (file.hasFileExtension ("cpp;cc;cxx"))                return "sourcecode.cpp.cpp";
        else if (file.hasFileExtension (".mm"))                  return "sourcecode.cpp.objcpp";
        else if (file.hasFileExtension (".m"))                   return "sourcecode.c.objc";
        else if (file.hasFileExtension (headerFileExtensions))   return "sourcecode.c.h";
        else if (file.hasFileExtension (".framework"))           return "wrapper.framework";
        else if (file.hasFileExtension (".jpeg;.jpg"))           return "image.jpeg";
        else if (file.hasFileExtension ("png;gif"))              return "image" + file.getFileExtension();
        else if (file.hasFileExtension ("html;htm"))             return "text.html";
        else if (file.hasFileExtension ("txt;rtf"))              return "text" + file.getFileExtension();
        else if (file.hasFileExtension ("plist"))                return "text.plist.xml";
        else if (file.hasFileExtension ("app"))                  return "wrapper.application";
        else if (file.hasFileExtension ("component;vst;plugin")) return "wrapper.cfbundle";
        else if (file.hasFileExtension ("xcodeproj"))            return "wrapper.pb-project";
        else if (file.hasFileExtension ("a"))                    return "archive.ar";

        return "file" + file.getFileExtension();
    }

    const String addFile (const RelativePath& path, bool shouldBeCompiled, bool inhibitWarnings)
    {
        if (shouldBeCompiled)
            addBuildFile (path, true, inhibitWarnings);
        else if (path.hasFileExtension (".r"))
            rezFileIDs.add (addBuildFile (path, false, inhibitWarnings));

        return addFileReference (path);
    }

    const String addProjectItem (const Project::Item& projectItem)
    {
        if (projectItem.isGroup())
        {
            StringArray childIDs;
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
            {
                const String childID (addProjectItem (projectItem.getChild(i)));

                if (childID.isNotEmpty())
                    childIDs.add (childID);
            }

            return addGroup (projectItem, childIDs);
        }
        else
        {
            if (projectItem.shouldBeAddedToTargetProject())
            {
                const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);
                return addFile (path, projectItem.shouldBeCompiled(), false);
            }
        }

        return String::empty;
    }

    void addFramework (const String& frameworkName)
    {
        const RelativePath path ("System/Library/Frameworks/" + frameworkName + ".framework", RelativePath::unknown);
        const String fileRefID (createID (path));
        addFileReference (path, "SDKROOT", getFileType (path), fileRefID);
        frameworkIDs.add (addBuildFile (path, fileRefID, false, false));
        frameworkFileIDs.add (fileRefID);
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs)
    {
        ValueTree* v = new ValueTree (groupID);
        v->setProperty ("isa", "PBXGroup", 0);
        v->setProperty ("children", "(" + indentList (childIDs, ",") + " )", 0);
        v->setProperty (Ids::name, groupName, 0);
        v->setProperty ("sourceTree", "<group>", 0);
        groups.add (v);
    }

    const String createGroup (const Array<RelativePath>& files, const String& groupName, const String& groupIDName, bool inhibitWarnings)
    {
        StringArray fileIDs;

        for (int i = 0; i < files.size(); ++i)
        {
            addFile (files.getReference(i), shouldFileBeCompiledByDefault (files.getReference(i)), inhibitWarnings);
            fileIDs.add (createID (files.getReference(i)));
        }

        const String groupID (createID (groupIDName));
        addGroup (groupID, groupName, fileIDs);
        return groupID;
    }

    const String addGroup (const Project::Item& item, StringArray& childIDs)
    {
        String groupName (item.getName().toString());

        if (item.isMainGroup())
        {
            groupName = "Source";

            // Add 'Juce Library Code' group
            if (juceWrapperFiles.size() > 0)
                childIDs.add (createGroup (juceWrapperFiles, project.getJuceCodeGroupName(), "__jucelibfiles", false));

            if (isVST())
                childIDs.add (createGroup (getVSTFilesRequired(), "Juce VST Wrapper", "__jucevstfiles", false));

            if (isAU())
                childIDs.add (createAUWrappersGroup());

            if (isRTAS())
                childIDs.add (createGroup (getRTASFilesRequired(), "Juce RTAS Wrapper", "__jucertasfiles", true));

            { // Add 'resources' group
                String resourcesGroupID (createID ("__resources"));
                addGroup (resourcesGroupID, "Resources", resourceFileRefs);
                childIDs.add (resourcesGroupID);
            }

            { // Add 'frameworks' group
                String frameworksGroupID (createID ("__frameworks"));
                addGroup (frameworksGroupID, "Frameworks", frameworkFileIDs);
                childIDs.add (frameworksGroupID);
            }

            { // Add 'products' group
                String productsGroupID (createID ("__products"));
                StringArray products;
                products.add (createID ("__productFileID"));
                addGroup (productsGroupID, "Products", products);
                childIDs.add (productsGroupID);
            }
        }

        String groupID (getIDForGroup (item));
        addGroup (groupID, groupName, childIDs);
        return groupID;
    }

    void addBuildProduct (const String& fileType, const String& binaryName)
    {
        ValueTree* v = new ValueTree (createID ("__productFileID"));
        v->setProperty ("isa", "PBXFileReference", 0);
        v->setProperty ("explicitFileType", fileType, 0);
        v->setProperty ("includeInIndex", (int) 0, 0);
        v->setProperty ("path", sanitisePath (binaryName), 0);
        v->setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", 0);
        pbxFileReferences.add (v);
    }

    void addTargetConfig (const String& configName, const StringArray& buildSettings)
    {
        ValueTree* v = new ValueTree (createID ("targetconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", 0);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", 0);
        v->setProperty (Ids::name, configName, 0);
        targetConfigs.add (v);
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings)
    {
        ValueTree* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", 0);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", 0);
        v->setProperty (Ids::name, configName, 0);
        projectConfigs.add (v);
    }

    void addConfigList (const OwnedArray <ValueTree>& configsToUse, const String& listID)
    {
        StringArray configIDs;

        for (int i = 0; i < configsToUse.size(); ++i)
            configIDs.add (configsToUse[i]->getType().toString());

        ValueTree* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", 0);
        v->setProperty ("buildConfigurations", "(" + indentList (configIDs, ",") + " )", 0);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, 0);

        if (configsToUse[0] != 0)
            v->setProperty ("defaultConfigurationName", configsToUse[0]->getProperty (Ids::name), 0);

        misc.add (v);
    }

    ValueTree* addBuildPhase (const String& phaseType, const StringArray& fileIds)
    {
        String phaseId (createID (phaseType + "resbuildphase"));
        buildPhaseIDs.add (phaseId);

        ValueTree* v = new ValueTree (phaseId);
        v->setProperty ("isa", phaseType, 0);
        v->setProperty ("buildActionMask", "2147483647", 0);
        v->setProperty ("files", "(" + indentList (fileIds, ",") + " )", 0);
        v->setProperty ("runOnlyForDeploymentPostprocessing", (int) 0, 0);
        misc.add (v);
        return v;
    }

    void addTargetObject()
    {
        ValueTree* v = new ValueTree (createID ("__target"));
        v->setProperty ("isa", "PBXNativeTarget", 0);
        v->setProperty ("buildConfigurationList", createID ("__configList"), 0);
        v->setProperty ("buildPhases", "(" + indentList (buildPhaseIDs, ",") + " )", 0);
        v->setProperty ("buildRules", "( )", 0);
        v->setProperty ("dependencies", "( )", 0);
        v->setProperty (Ids::name, project.getDocumentTitle(), 0);
        v->setProperty ("productName", project.getDocumentTitle(), 0);
        v->setProperty ("productReference", createID ("__productFileID"), 0);

        if (project.isGUIApplication())
        {
            v->setProperty ("productInstallPath", "$(HOME)/Applications", 0);
            v->setProperty ("productType", "com.apple.product-type.application", 0);
        }
        else if (project.isCommandLineApp())
        {
            v->setProperty ("productInstallPath", "/usr/bin", 0);
            v->setProperty ("productType", "com.apple.product-type.tool", 0);
        }
        else if (project.isAudioPlugin() || project.isBrowserPlugin())
        {
            v->setProperty ("productInstallPath", "$(HOME)/Library/Audio/Plug-Ins/Components/", 0);
            v->setProperty ("productType", "com.apple.product-type.bundle", 0);
        }
        else if (project.isLibrary())
        {
            v->setProperty ("productType", "com.apple.product-type.library.static", 0);
        }
        else
            jassertfalse; //xxx

        misc.add (v);
    }

    void addProjectObject()
    {
        ValueTree* v = new ValueTree (createID ("__root"));
        v->setProperty ("isa", "PBXProject", 0);
        v->setProperty ("buildConfigurationList", createID ("__projList"), 0);
        v->setProperty ("compatibilityVersion", "Xcode 3.0", 0);
        v->setProperty ("hasScannedForEncodings", (int) 0, 0);
        v->setProperty ("mainGroup", getIDForGroup (project.getMainGroup()), 0);
        v->setProperty ("projectDirPath", "\"\"", 0);
        v->setProperty ("projectRoot", "\"\"", 0);
        v->setProperty ("targets", "( " + createID ("__target") + " )", 0);
        misc.add (v);
    }

    void addPluginShellScriptPhase()
    {
        ValueTree* v = addBuildPhase ("PBXShellScriptBuildPhase", StringArray());
        v->setProperty (Ids::name, "Copy to the different plugin folders", 0);
        v->setProperty ("shellPath", "/bin/sh", 0);
        v->setProperty ("shellScript", String::fromUTF8 (BinaryData::AudioPluginXCodeScript_txt, BinaryData::AudioPluginXCodeScript_txtSize)
                                            .replace ("\\", "\\\\")
                                            .replace ("\"", "\\\"")
                                            .replace ("\r\n", "\\n")
                                            .replace ("\n", "\\n"), 0);
    }

    //==============================================================================
    static const String indentList (const StringArray& list, const String& separator)
    {
        if (list.size() == 0)
            return " ";

        return "\n\t\t\t\t" + list.joinIntoString (separator + "\n\t\t\t\t")
                  + (separator == ";" ? separator : String::empty);
    }

    const String createID (const RelativePath& path) const
    {
        return createID (path.toUnixStyle());
    }

    const String createID (const String& rootString) const
    {
        static const char digits[] = "0123456789ABCDEF";
        char n[24];
        Random ran (projectIDSalt + hashCode64 (rootString));

        for (int i = 0; i < numElementsInArray (n); ++i)
            n[i] = digits [ran.nextInt (16)];

        return String (n, numElementsInArray (n));
    }

    const String getIDForGroup (const Project::Item& item) const
    {
        return createID (item.getID());
    }

    bool shouldFileBeCompiledByDefault (const RelativePath& file) const
    {
        return file.hasFileExtension (sourceFileExtensions);
    }

    //==============================================================================
    const Array<RelativePath> getRTASFilesRequired() const
    {
        Array<RelativePath> s;
        if (isRTAS())
        {
            const char* files[] = { "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode1.cpp",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode2.cpp",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode3.cpp",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_DigiCode_Header.h",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_MacResources.r",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_MacUtilities.mm",
                                    "extras/audio plugins/wrapper/RTAS/juce_RTAS_Wrapper.cpp" };

            for (int i = 0; i < numElementsInArray (files); ++i)
                s.add (getJucePathFromTargetFolder().getChildFile (files[i]));
        }

        return s;
    }

    const String createAUWrappersGroup()
    {
        Array<RelativePath> auWrappers;

        const char* files[] = { "extras/audio plugins/wrapper/AU/juce_AU_Resources.r",
                                "extras/audio plugins/wrapper/AU/juce_AU_Wrapper.mm" };
        int i;
        for (i = 0; i < numElementsInArray (files); ++i)
            auWrappers.add (getJucePathFromTargetFolder().getChildFile (files[i]));

        const char* appleAUFiles[] = {  "Extras/CoreAudio/PublicUtility/CADebugMacros.h",
                                        "Extras/CoreAudio/PublicUtility/CAAUParameter.cpp",
                                        "Extras/CoreAudio/PublicUtility/CAAUParameter.h",
                                        "Extras/CoreAudio/PublicUtility/CAAudioChannelLayout.cpp",
                                        "Extras/CoreAudio/PublicUtility/CAAudioChannelLayout.h",
                                        "Extras/CoreAudio/PublicUtility/CAMutex.cpp",
                                        "Extras/CoreAudio/PublicUtility/CAMutex.h",
                                        "Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.cpp",
                                        "Extras/CoreAudio/PublicUtility/CAStreamBasicDescription.h",
                                        "Extras/CoreAudio/PublicUtility/CAVectorUnitTypes.h",
                                        "Extras/CoreAudio/PublicUtility/CAVectorUnit.cpp",
                                        "Extras/CoreAudio/PublicUtility/CAVectorUnit.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUViewBase/AUViewLocalizedStringKeys.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewDispatch.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewControl.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewControl.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/CarbonEventHandler.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/CarbonEventHandler.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUCarbonViewBase/AUCarbonViewBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUDispatch.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUDispatch.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUInputElement.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUInputElement.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUOutputElement.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUOutputElement.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUResources.r",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUScopeElement.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/AUScopeElement.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/ComponentBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/AUBase/ComponentBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUMIDIEffectBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUOutputBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUOutputBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/MusicDeviceBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/MusicDeviceBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUEffectBase.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/OtherBases/AUEffectBase.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUBuffer.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUBuffer.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUDebugDispatcher.cpp",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUDebugDispatcher.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUInputFormatConverter.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUSilentTimeout.h",
                                        "Extras/CoreAudio/AudioUnits/AUPublic/Utility/AUTimestampGenerator.h" };

        StringArray fileIDs, appleFileIDs;

        for (i = 0; i < auWrappers.size(); ++i)
        {
            addFile (auWrappers.getReference(i), shouldFileBeCompiledByDefault (auWrappers.getReference(i)), false);
            fileIDs.add (createID (auWrappers.getReference(i)));
        }

        for (i = 0; i < numElementsInArray (appleAUFiles); ++i)
        {
            RelativePath file (appleAUFiles[i], RelativePath::unknown);
            const String fileRefID (createID (file));

            addFileReference (file, "DEVELOPER_DIR", getFileType (file), fileRefID);

            if (shouldFileBeCompiledByDefault (file))
                addBuildFile (file, fileRefID, true, true);

            appleFileIDs.add (fileRefID);
        }

        const String appleGroupID (createID ("__juceappleaufiles"));
        addGroup (appleGroupID, "Apple AU Files", appleFileIDs);
        fileIDs.add (appleGroupID);

        const String groupID (createID ("__juceaufiles"));
        addGroup (groupID, "Juce AU Wrapper", fileIDs);
        return groupID;
    }
};


#endif   // __JUCER_PROJECTEXPORT_XCODE_JUCEHEADER__
