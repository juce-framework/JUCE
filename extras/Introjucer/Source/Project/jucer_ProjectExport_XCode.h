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

        if (getSettings() ["objCExtraSuffix"].isVoid())
            getObjCSuffix() = createAlphaNumericUID();
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
    Value getObjCSuffix()       { return getSetting ("objCExtraSuffix"); }

    int getLaunchPreferenceOrderForCurrentOS()
    {
       #if JUCE_MAC
        return iPhone ? 1 : 2;
       #else
        return 0;
       #endif
    }

    bool isAvailableOnCurrentOS()
    {
       #if JUCE_MAC
        return true;
       #else
        return false;
       #endif
    }

    bool isPossibleForCurrentProject()      { return project.getProjectType().isGUIApplication() || ! iPhone; }
    bool usesMMFiles() const                { return true; }
    bool isXcode() const                    { return true; }

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);

        props.add (new TextPropertyComponent (getObjCSuffix(), "Objective-C class name suffix", 64, false));
        props.getLast()->setTooltip ("Because objective-C linkage is done by string-matching, you can get horrible linkage mix-ups when different modules containing the "
                                     "same class-names are loaded simultaneously. This setting lets you provide a unique string that will be used in naming the obj-C classes in your executable to avoid this.");

        if (project.getProjectType().isGUIApplication() && ! iPhone)
        {
            props.add (new TextPropertyComponent (getSetting ("documentExtensions"), "Document file extensions", 128, false));
            props.getLast()->setTooltip ("A comma-separated list of file extensions for documents that your app can open.");
        }
        else if (iPhone)
        {
            props.add (new BooleanPropertyComponent (getSetting ("UIFileSharingEnabled"), "File Sharing Enabled", "Enabled"));
            props.getLast()->setTooltip ("Enable this to expose your app's files to iTunes.");

            props.add (new BooleanPropertyComponent (getSetting ("UIStatusBarHidden"), "Status Bar Hidden", "Enabled"));
            props.getLast()->setTooltip ("Enable this to disable the status bar in your app.");
        }
    }

    void launchProject()
    {
        getProjectBundle().startAsProcess();
    }

    //==============================================================================
    void create()
    {
        infoPlistFile = getTargetFolder().getChildFile ("Info.plist");

        createIconFile();

        File projectBundle (getProjectBundle());
        createDirectoryOrThrow (projectBundle);

        createObjects();

        File projectFile (projectBundle.getChildFile ("project.pbxproj"));

        {
            MemoryOutputStream mo;
            writeProjectFile (mo);
            overwriteFileIfDifferentOrThrow (projectFile, mo);
        }

        writeInfoPlistFile();
    }

private:
    OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, groups, misc, projectConfigs, targetConfigs;
    StringArray buildPhaseIDs, resourceIDs, sourceIDs, frameworkIDs;
    StringArray frameworkFileIDs, rezFileIDs, resourceFileRefs;
    File infoPlistFile, iconFile;
    int64 projectIDSalt;
    const bool iPhone;

    static String sanitisePath (const String& path)
    {
        if (path.startsWithChar ('~'))
            return "$(HOME)" + path.substring (1);

        return path;
    }

    File getProjectBundle() const                 { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (".xcodeproj"); }

    bool hasPList() const                         { return ! (project.getProjectType().isLibrary() || project.getProjectType().isCommandLineApp()); }
    String getAudioPluginBundleExtension() const  { return "component"; }

    //==============================================================================
    void createObjects()
    {
        if (! project.getProjectType().isLibrary())
            addFrameworks();

        const String productName (project.getConfiguration (0).getTargetBinaryName().toString());

        if (project.getProjectType().isGUIApplication())         addBuildProduct ("wrapper.application", productName + ".app");
        else if (project.getProjectType().isCommandLineApp())    addBuildProduct ("compiled.mach-o.executable", productName);
        else if (project.getProjectType().isLibrary())           addBuildProduct ("archive.ar", getLibbedFilename (productName));
        else if (project.getProjectType().isAudioPlugin())       addBuildProduct ("wrapper.cfbundle", productName + "." + getAudioPluginBundleExtension());
        else if (project.getProjectType().isBrowserPlugin())     addBuildProduct ("wrapper.cfbundle", productName + ".plugin");
        else jassert (productName.isEmpty());

        if (hasPList())
        {
            RelativePath plistPath (infoPlistFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (plistPath.toUnixStyle());
            resourceFileRefs.add (createID (plistPath));
        }

        if (iconFile.exists())
        {
            RelativePath iconPath (iconFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (iconPath.toUnixStyle());
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

        if (! project.getProjectType().isLibrary())
            addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

        if (rezFileIDs.size() > 0)
            addBuildPhase ("PBXRezBuildPhase", rezFileIDs);

        addBuildPhase ("PBXSourcesBuildPhase", sourceIDs);

        if (! project.getProjectType().isLibrary())
            addBuildPhase ("PBXFrameworksBuildPhase", frameworkIDs);

        if (project.getProjectType().isAudioPlugin())
            addPluginShellScriptPhase();

        addTargetObject();
        addProjectObject();
    }

    static Image fixMacIconImageSize (Image& image)
    {
        const int w = image.getWidth();
        const int h = image.getHeight();

        if (w != h || (w != 16 && w != 32 && w != 48 && w != 64))
        {
            const int newSize = w >= 128 ? 128 : (w >= 64 ? 64 : (w >= 32 ? 32 : 16));
            Image newIm (Image::ARGB, newSize, newSize, true, Image::SoftwareImage);
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

            const char* type = nullptr;
            const char* maskType = nullptr;

            if (w == h)
            {
                if (w == 16)  { type = "is32"; maskType = "s8mk"; }
                if (w == 32)  { type = "il32"; maskType = "l8mk"; }
                if (w == 48)  { type = "ih32"; maskType = "h8mk"; }
                if (w == 128) { type = "it32"; maskType = "t8mk"; }
            }

            if (type != nullptr)
            {
                data.write (type, 4);
                data.writeIntBigEndian (8 + 4 * w * h);

                const Image::BitmapData bitmap (image, Image::BitmapData::readOnly);

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

    void createIconFile()
    {
        Array<Image> images;

        Image bigIcon (project.getBigIcon());
        if (bigIcon.isValid())
            images.add (bigIcon);

        Image smallIcon (project.getSmallIcon());
        if (smallIcon.isValid())
            images.add (smallIcon);

        if (images.size() > 0)
        {
            MemoryOutputStream mo;
            writeIcnsFile (images, mo);

            iconFile = getTargetFolder().getChildFile ("Icon.icns");
            overwriteFileIfDifferentOrThrow (iconFile, mo);
        }
    }

    void writeInfoPlistFile()
    {
        if (! hasPList())
            return;

        XmlElement plist ("plist");
        XmlElement* dict = plist.createNewChildElement ("dict");

        if (iPhone)
            addPlistDictionaryKeyBool (dict, "LSRequiresIPhoneOS", true);

        addPlistDictionaryKey (dict, "CFBundleExecutable",          "${EXECUTABLE_NAME}");
        addPlistDictionaryKey (dict, "CFBundleIconFile",            iconFile.exists() ? iconFile.getFileName() : String::empty);
        addPlistDictionaryKey (dict, "CFBundleIdentifier",          project.getBundleIdentifier().toString());
        addPlistDictionaryKey (dict, "CFBundleName",                project.getProjectName().toString());

        if (project.getProjectType().isAudioPlugin())
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
        documentExtensions.addTokens (replacePreprocessorDefs (getAllPreprocessorDefs(), getSetting ("documentExtensions").toString()),
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

        if (getSetting ("UIFileSharingEnabled").getValue())
            addPlistDictionaryKeyBool (dict, "UIFileSharingEnabled", true);

        if (getSetting ("UIStatusBarHidden").getValue())
            addPlistDictionaryKeyBool (dict, "UIStatusBarHidden", true);

        MemoryOutputStream mo;
        plist.writeToStream (mo, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");

        overwriteFileIfDifferentOrThrow (infoPlistFile, mo);
    }

    StringArray getHeaderSearchPaths (const Project::BuildConfiguration& config)
    {
        StringArray searchPaths (config.getHeaderSearchPaths());

        for (int i = 0; i < libraryModules.size(); ++i)
            libraryModules.getUnchecked(i)->addExtraSearchPaths (*this, searchPaths);

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
        if (project.getProjectType().isAudioPlugin())
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

    StringArray getProjectSettings (const Project::BuildConfiguration& config)
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

        if (project.getProjectType().isLibrary() || project.getJuceLinkageMode() == Project::useLinkedJuce)
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = NO");
        }
        else
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = YES");
        }

        if (iPhone)
        {
            s.add ("\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\"");
            s.add ("SDKROOT = iphoneos");
            s.add ("TARGETED_DEVICE_FAMILY = \"1,2\"");
        }

        s.add ("ZERO_LINK = NO");

        if (! isRTAS())   // (dwarf seems to be incompatible with the RTAS libs)
            s.add ("DEBUG_INFORMATION_FORMAT = \"dwarf\"");

        s.add ("PRODUCT_NAME = \"" + config.getTargetBinaryName().toString() + "\"");
        return s;
    }

    StringArray getTargetSettings (const Project::BuildConfiguration& config)
    {
        StringArray s;

        const String arch (config.getMacArchitecture().toString());
        if (arch == Project::BuildConfiguration::osxArch_Native)                s.add ("ARCHS = \"$(ARCHS_NATIVE)\"");
        else if (arch == Project::BuildConfiguration::osxArch_32BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_BIT)\"");
        else if (arch == Project::BuildConfiguration::osxArch_64BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\"");
        else if (arch == Project::BuildConfiguration::osxArch_64Bit)            s.add ("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\"");

        s.add ("PREBINDING = NO");
        s.add ("HEADER_SEARCH_PATHS = \"" + replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (" ")) + " $(inherited)\"");
        s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());
        s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

        const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlags().toString()).trim());
        if (extraFlags.isNotEmpty())
            s.add ("OTHER_CPLUSPLUSFLAGS = " + extraFlags);

        if (project.getProjectType().isGUIApplication())
        {
            s.add ("INSTALL_PATH = \"$(HOME)/Applications\"");
        }
        else if (project.getProjectType().isAudioPlugin())
        {
            s.add ("LIBRARY_STYLE = Bundle");
            s.add ("INSTALL_PATH = \"$(HOME)/Library/Audio/Plug-Ins/Components/\"");
            s.add ("WRAPPER_EXTENSION = " + getAudioPluginBundleExtension());
            s.add ("GENERATE_PKGINFO_FILE = YES");
            s.add ("OTHER_REZFLAGS = \"-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                   " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                   " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"\"");
        }
        else if (project.getProjectType().isBrowserPlugin())
        {
            s.add ("LIBRARY_STYLE = Bundle");
            s.add ("INSTALL_PATH = \"/Library/Internet Plug-Ins/\"");
        }
        else if (project.getProjectType().isLibrary())
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
        else if (project.getProjectType().isCommandLineApp())
        {
        }
        else
        {
            jassertfalse;
        }

        if (! iPhone)
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

            if (sdkCompat == Project::BuildConfiguration::osxVersion10_4)       s.add ("MACOSX_DEPLOYMENT_TARGET = 10.4");
            else if (sdkCompat == Project::BuildConfiguration::osxVersion10_5)  s.add ("MACOSX_DEPLOYMENT_TARGET = 10.5");
            else if (sdkCompat == Project::BuildConfiguration::osxVersion10_6)  s.add ("MACOSX_DEPLOYMENT_TARGET = 10.6");

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
            const String objCSuffix (getObjCSuffix().toString().trim());
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
            s.addTokens ("UIKit Foundation CoreGraphics CoreText AudioToolbox QuartzCore OpenGLES", false);
        }
        else
        {
            s.addTokens ("Cocoa Carbon IOKit CoreAudio CoreMIDI WebKit DiscRecording OpenGL QuartzCore QTKit QuickTime AudioToolbox", false);

            if (isAU())
                s.addTokens ("AudioUnit CoreAudioKit AudioToolbox", false);
            else if (project.getConfigFlag ("JUCE_PLUGINHOST_AU").toString() == Project::configFlagEnabled)
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
                  "\tobjectVersion = 45;\n"
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
            output << "\t\t" << o.getType().toString() << " = { ";

            for (int j = 0; j < o.getNumProperties(); ++j)
            {
                const Identifier propertyName (o.getPropertyName(j));
                String val (o.getProperty (propertyName).toString());

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-_\r\n")
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

    static void addPlistDictionaryKeyBool (XmlElement* xml, const String& key, const bool value)
    {
        xml->createNewChildElement ("key")->addTextElement (key);
        xml->createNewChildElement (value ? "true" : "false");
    }

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings)
    {
        String fileID (createID (path + "buildref"));

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

    String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings)
    {
        return addBuildFile (path.toUnixStyle(), createID (path), addToSourceBuildPhase, inhibitWarnings);
    }

    String addFileReference (String pathString)
    {
        String sourceTree ("SOURCE_ROOT");
        RelativePath path (pathString, RelativePath::unknown);

        if (pathString.startsWith ("${"))
        {
            sourceTree = pathString.substring (2).upToFirstOccurrenceOf ("}", false, false);
            pathString = pathString.fromFirstOccurrenceOf ("}/", false, false);
        }
        else if (path.isAbsolute())
        {
            sourceTree = "<absolute>";
        }

        const String fileRefID (createID (pathString));

        ValueTree* v = new ValueTree (fileRefID);
        v->setProperty ("isa", "PBXFileReference", 0);
        v->setProperty ("lastKnownFileType", getFileType (path), 0);
        v->setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), 0);
        v->setProperty ("path", sanitisePath (pathString), 0);
        v->setProperty ("sourceTree", sourceTree, 0);
        pbxFileReferences.add (v);

        return fileRefID;
    }

    static String getFileType (const RelativePath& file)
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

    String addFile (const RelativePath& path, bool shouldBeCompiled, bool inhibitWarnings)
    {
        if (shouldBeCompiled)
        {
            if (path.hasFileExtension (".r"))
                rezFileIDs.add (addBuildFile (path, false, inhibitWarnings));
            else
                addBuildFile (path, true, inhibitWarnings);
        }

        return addFileReference (path.toUnixStyle());
    }

    String addProjectItem (const Project::Item& projectItem)
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
                String itemPath (projectItem.getFilePath());
                bool inhibitWarnings = projectItem.getShouldInhibitWarningsValue().getValue();

                if (itemPath.startsWith ("${"))
                {
                    const RelativePath path (itemPath, RelativePath::unknown);
                    return addFile (path, projectItem.shouldBeCompiled(), inhibitWarnings);
                }
                else
                {
                    const RelativePath path (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);
                    return addFile (path, projectItem.shouldBeCompiled(), inhibitWarnings);
                }
            }
        }

        return String::empty;
    }

    void addFramework (const String& frameworkName)
    {
        const String path ("System/Library/Frameworks/" + frameworkName + ".framework");
        const String fileRefID (createID (path));
        addFileReference ("${SDKROOT}/" + path);
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

    String addGroup (const Project::Item& item, StringArray& childIDs)
    {
        String groupName (item.getName().toString());

        if (item.isMainGroup())
        {
            groupName = "Source";

            for (int i = 0; i < generatedGroups.size(); ++i)
                if (generatedGroups.getReference(i).getNumChildren() > 0)
                    childIDs.add (addProjectItem (generatedGroups.getReference(i)));

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

        const String groupID (getIDForGroup (item));
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

        if (configsToUse[0] != nullptr)
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

        if (project.getProjectType().isGUIApplication())
        {
            v->setProperty ("productInstallPath", "$(HOME)/Applications", 0);
            v->setProperty ("productType", "com.apple.product-type.application", 0);
        }
        else if (project.getProjectType().isCommandLineApp())
        {
            v->setProperty ("productInstallPath", "/usr/bin", 0);
            v->setProperty ("productType", "com.apple.product-type.tool", 0);
        }
        else if (project.getProjectType().isAudioPlugin() || project.getProjectType().isBrowserPlugin())
        {
            v->setProperty ("productInstallPath", "$(HOME)/Library/Audio/Plug-Ins/Components/", 0);
            v->setProperty ("productType", "com.apple.product-type.bundle", 0);
        }
        else if (project.getProjectType().isLibrary())
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
        v->setProperty ("compatibilityVersion", "Xcode 3.1", 0);
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
    static String indentList (const StringArray& list, const String& separator)
    {
        if (list.size() == 0)
            return " ";

        return "\n\t\t\t\t" + list.joinIntoString (separator + "\n\t\t\t\t")
                  + (separator == ";" ? separator : String::empty);
    }

    String createID (const RelativePath& path) const
    {
        return createID (path.toUnixStyle());
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        static const char digits[] = "0123456789ABCDEF";
        char n[24];
        Random ran (projectIDSalt + hashCode64 (rootString));

        for (int i = 0; i < numElementsInArray (n); ++i)
            n[i] = digits [ran.nextInt() & 15];

        return String (n, numElementsInArray (n));
    }

    String getIDForGroup (const Project::Item& item) const
    {
        return createID (item.getID());
    }

    bool shouldFileBeCompiledByDefault (const RelativePath& file) const
    {
        return file.hasFileExtension (sourceFileExtensions);
    }
};


#endif   // __JUCER_PROJECTEXPORT_XCODE_JUCEHEADER__
