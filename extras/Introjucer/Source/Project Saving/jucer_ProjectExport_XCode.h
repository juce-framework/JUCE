/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

namespace
{
    const char* const osxVersionDefault         = "default";
    const char* const osxVersion10_4            = "10.4 SDK";
    const char* const osxVersion10_5            = "10.5 SDK";
    const char* const osxVersion10_6            = "10.6 SDK";
    const char* const osxVersion10_7            = "10.7 SDK";

    const char* const osxArch_Default           = "default";
    const char* const osxArch_Native            = "Native";
    const char* const osxArch_32BitUniversal    = "32BitUniversal";
    const char* const osxArch_64BitUniversal    = "64BitUniversal";
    const char* const osxArch_64Bit             = "64BitIntel";
}

//==============================================================================
class XCodeProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameMac()                         { return "XCode (MacOSX)"; }
    static const char* getNameiOS()                         { return "XCode (iOS)"; }
    static const char* getValueTreeTypeName (bool iOS)      { return iOS ? "XCODE_IPHONE" : "XCODE_MAC"; }

    //==============================================================================
    XCodeProjectExporter (Project& project_, const ValueTree& settings_, const bool iOS_)
        : ProjectExporter (project_, settings_),
          iOS (iOS_)
    {
        name = iOS ? getNameiOS() : getNameMac();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + (iOS ? "iOS" : "MacOSX");
    }

    static XCodeProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (false)))
            return new XCodeProjectExporter (project, settings, false);
        else if (settings.hasType (getValueTreeTypeName (true)))
            return new XCodeProjectExporter (project, settings, true);

        return nullptr;
    }

    //==============================================================================
    Value getPListToMergeValue()            { return getSetting ("customPList"); }
    String getPListToMergeString() const    { return settings   ["customPList"]; }

    Value getExtraFrameworksValue()         { return getSetting (Ids::extraFrameworks); }
    String getExtraFrameworksString() const { return settings   [Ids::extraFrameworks]; }

    Value  getPostBuildScriptValue()        { return getSetting (Ids::postbuildCommand); }
    String getPostBuildScript() const       { return settings   [Ids::postbuildCommand]; }

    int getLaunchPreferenceOrderForCurrentOS()
    {
       #if JUCE_MAC
        return iOS ? 1 : 2;
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

    bool isPossibleForCurrentProject()      { return projectType.isGUIApplication() || ! iOS; }
    bool usesMMFiles() const                { return true; }
    bool isXcode() const                    { return true; }
    bool isOSX() const                      { return ! iOS; }
    bool canCopeWithDuplicateFiles()        { return true; }

    void createPropertyEditors (PropertyListBuilder& props)
    {
        ProjectExporter::createPropertyEditors (props);

        if (projectType.isGUIApplication() && ! iOS)
        {
            props.add (new TextPropertyComponent (getSetting ("documentExtensions"), "Document file extensions", 128, false),
                       "A comma-separated list of file extensions for documents that your app can open.");
        }
        else if (iOS)
        {
            props.add (new BooleanPropertyComponent (getSetting ("UIFileSharingEnabled"), "File Sharing Enabled", "Enabled"),
                       "Enable this to expose your app's files to iTunes.");

            props.add (new BooleanPropertyComponent (getSetting ("UIStatusBarHidden"), "Status Bar Hidden", "Enabled"),
                       "Enable this to disable the status bar in your app.");
        }

        props.add (new TextPropertyComponent (getPListToMergeValue(), "Custom PList", 8192, true),
                   "You can paste the contents of an XML PList file in here, and the settings that it contains will override any "
                   "settings that the Introjucer creates. BEWARE! When doing this, be careful to remove from the XML any "
                   "values that you DO want the introjucer to change!");

        props.add (new TextPropertyComponent (getExtraFrameworksValue(), "Extra Frameworks", 2048, false),
                   "A comma-separated list of extra frameworks that should be added to the build. "
                   "(Don't include the .framework extension in the name)");

        if (projectType.isLibrary())
        {
            const char* const libTypes[] = { "Static Library (.a)", "Dynamic Library (.dylib)", 0 };
            const int libTypeValues[] = { 1, 2, 0 };
            props.add (new ChoicePropertyComponent (getLibraryType(), "Library Type",
                                                    StringArray (libTypes), Array<var> (libTypeValues)));
        }

        props.add (new TextPropertyComponent (getPostBuildScriptValue(), "Post-build shell script", 32768, true),
                   "Some shell-script that will be run after a build completes.");
    }

    void launchProject()
    {
        getProjectBundle().startAsProcess();
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const
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

protected:
    Value getLibraryType()          { return getSetting (Ids::libraryType); }
    bool isStaticLibrary() const    { return projectType.isLibrary() && (int) settings [Ids::libraryType] == 1; }


    //==============================================================================
    class XcodeBuildConfiguration  : public BuildConfiguration
    {
    public:
        XcodeBuildConfiguration (Project& project, const ValueTree& settings, const bool iOS_)
            : BuildConfiguration (project, settings), iOS (iOS_)
        {
        }

        Value  getMacSDKVersionValue()                 { return getValue (Ids::osxSDK); }
        String getMacSDKVersion() const                { return config   [Ids::osxSDK]; }
        Value  getMacCompatibilityVersionValue()       { return getValue (Ids::osxCompatibility); }
        String getMacCompatibilityVersion() const      { return config   [Ids::osxCompatibility]; }
        Value  getiOSCompatibilityVersionValue()       { return getValue (Ids::iosCompatibility); }
        String getiOSCompatibilityVersion() const      { return config   [Ids::iosCompatibility]; }
        Value  getMacArchitectureValue()               { return getValue (Ids::osxArchitecture); }
        String getMacArchitecture() const              { return config   [Ids::osxArchitecture]; }
        Value  getCustomXcodeFlagsValue()              { return getValue (Ids::customXcodeFlags); }
        String getCustomXcodeFlags() const             { return config   [Ids::customXcodeFlags]; }
        Value  getCppLibTypeValue()                    { return getValue (Ids::cppLibType); }
        String getCppLibType() const                   { return config   [Ids::cppLibType]; }

        void createPropertyEditors (PropertyListBuilder& props)
        {
            createBasicPropertyEditors (props);

            if (iOS)
            {
                if (getiOSCompatibilityVersion().isEmpty())
                    getiOSCompatibilityVersionValue() = osxVersionDefault;

                const char* iosVersions[]      = { "Use Default",     "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", 0 };
                const char* iosVersionValues[] = { osxVersionDefault, "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", 0 };

                props.add (new ChoicePropertyComponent (getiOSCompatibilityVersionValue(), "iOS Deployment Target",
                                                        StringArray (iosVersions), Array<var> (iosVersionValues)),
                           "The minimum version of iOS that the target binary will run on.");
            }
            else
            {
                if (getMacSDKVersion().isEmpty())
                    getMacSDKVersionValue() = osxVersionDefault;

                if (getMacCompatibilityVersion().isEmpty())
                    getMacCompatibilityVersionValue() = osxVersionDefault;

                const char* osxVersions[]      = { "Use Default",     osxVersion10_5, osxVersion10_6, osxVersion10_7, 0 };
                const char* osxVersionValues[] = { osxVersionDefault, osxVersion10_5, osxVersion10_6, osxVersion10_7, 0 };

                props.add (new ChoicePropertyComponent (getMacSDKVersionValue(), "OSX Base SDK Version",
                                                        StringArray (osxVersions), Array<var> (osxVersionValues)),
                           "The version of OSX to link against in the XCode build.");

                props.add (new ChoicePropertyComponent (getMacCompatibilityVersionValue(), "OSX Compatibility Version",
                                                        StringArray (osxVersions), Array<var> (osxVersionValues)),
                           "The minimum version of OSX that the target binary will be compatible with.");

                if (getMacArchitecture().isEmpty())
                    getMacArchitectureValue() = osxArch_Default;

                const char* osxArch[] = { "Use Default", "Native architecture of build machine",
                                          "Universal Binary (32-bit)", "Universal Binary (64-bit)", "64-bit Intel", 0 };
                const char* osxArchValues[] = { osxArch_Default, osxArch_Native, osxArch_32BitUniversal,
                                                osxArch_64BitUniversal, osxArch_64Bit, 0 };

                props.add (new ChoicePropertyComponent (getMacArchitectureValue(), "OSX Architecture",
                                                        StringArray (osxArch), Array<var> (osxArchValues)),
                           "The type of OSX binary that will be produced.");
            }

            props.add (new TextPropertyComponent (getCustomXcodeFlagsValue(), "Custom Xcode flags", 8192, false),
                       "A comma-separated list of custom Xcode setting flags which will be appended to the list of generated flags, "
                       "e.g. MACOSX_DEPLOYMENT_TARGET_i386 = 10.5, VALID_ARCHS = \"ppc i386 x86_64\"");

            const char* cppLibNames[] = { "Use Default", "Use LLVM libc++", 0 };
            Array<var> cppLibValues;
            cppLibValues.add (var::null);
            cppLibValues.add ("libc++");

            props.add (new ChoicePropertyComponent (getCppLibTypeValue(), "C++ Library", StringArray (cppLibNames), cppLibValues),
                       "The type of C++ std lib that will be linked.");
        }

        bool iOS;
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& settings) const
    {
        return new XcodeBuildConfiguration (project, settings, iOS);
    }

private:
    mutable OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, pbxGroups, misc, projectConfigs, targetConfigs;
    mutable StringArray buildPhaseIDs, resourceIDs, sourceIDs, frameworkIDs;
    mutable StringArray frameworkFileIDs, rezFileIDs, resourceFileRefs;
    mutable File infoPlistFile, iconFile;
    const bool iOS;

    static String sanitisePath (const String& path)
    {
        if (path.startsWithChar ('~'))
            return "$(HOME)" + path.substring (1);

        return path;
    }

    File getProjectBundle() const                 { return getTargetFolder().getChildFile (project.getProjectFilenameRoot()).withFileExtension (".xcodeproj"); }

    //==============================================================================
    void createObjects() const
    {
        addFrameworks();
        addMainBuildProduct();

        if (xcodeCreatePList)
        {
            RelativePath plistPath (infoPlistFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (plistPath.toUnixStyle());
            resourceFileRefs.add (createFileRefID (plistPath));
        }

        if (iconFile.exists())
        {
            RelativePath iconPath (iconFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (iconPath.toUnixStyle());
            resourceIDs.add (addBuildFile (iconPath, false, false));
            resourceFileRefs.add (createFileRefID (iconPath));
        }

        {
            StringArray topLevelGroupIDs;

            for (int i = 0; i < groups.size(); ++i)
                if (groups.getReference(i).getNumChildren() > 0)
                    topLevelGroupIDs.add (addProjectItem (groups.getReference(i)));

            { // Add 'resources' group
                String resourcesGroupID (createID ("__resources"));
                addGroup (resourcesGroupID, "Resources", resourceFileRefs);
                topLevelGroupIDs.add (resourcesGroupID);
            }

            { // Add 'frameworks' group
                String frameworksGroupID (createID ("__frameworks"));
                addGroup (frameworksGroupID, "Frameworks", frameworkFileIDs);
                topLevelGroupIDs.add (frameworksGroupID);
            }

            { // Add 'products' group
                String productsGroupID (createID ("__products"));
                StringArray products;
                products.add (createID ("__productFileID"));
                addGroup (productsGroupID, "Products", products);
                topLevelGroupIDs.add (productsGroupID);
            }

            addGroup (createID ("__mainsourcegroup"), "Source", topLevelGroupIDs);
        }

        for (ConstConfigIterator config (*this); config.next();)
        {
            const XcodeBuildConfiguration& xcodeConfig = dynamic_cast <const XcodeBuildConfiguration&> (*config);
            addProjectConfig (config->getName(), getProjectSettings (xcodeConfig));
            addTargetConfig  (config->getName(), getTargetSettings (xcodeConfig));
        }

        addConfigList (projectConfigs, createID ("__projList"));
        addConfigList (targetConfigs, createID ("__configList"));

        if (! isStaticLibrary())
            addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

        if (rezFileIDs.size() > 0)
            addBuildPhase ("PBXRezBuildPhase", rezFileIDs);

        addBuildPhase ("PBXSourcesBuildPhase", sourceIDs);

        if (! isStaticLibrary())
            addBuildPhase ("PBXFrameworksBuildPhase", frameworkIDs);

        addShellScriptPhase();

        addTargetObject();
        addProjectObject();
    }

    static Image fixMacIconImageSize (Image& image)
    {
        const int validSizes[] = { 16, 32, 48, 128, 256, 512, 1024 };

        const int w = image.getWidth();
        const int h = image.getHeight();

        int bestSize = 16;

        for (int i = 0; i < numElementsInArray (validSizes); ++i)
        {
            if (w == h && w == validSizes[i])
                return image;

            if (jmax (w, h) > validSizes[i])
                bestSize = validSizes[i];
        }

        return rescaleImageForIcon (image, bestSize);
    }

    static void writeOldIconFormat (MemoryOutputStream& out, const Image& image, const char* type, const char* maskType)
    {
        const int w = image.getWidth();
        const int h = image.getHeight();

        out.write (type, 4);
        out.writeIntBigEndian (8 + 4 * w * h);

        const Image::BitmapData bitmap (image, Image::BitmapData::readOnly);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));
                out.writeByte ((char) pixel.getAlpha());
                out.writeByte ((char) pixel.getRed());
                out.writeByte ((char) pixel.getGreen());
                out.writeByte ((char) pixel.getBlue());
            }
        }

        out.write (maskType, 4);
        out.writeIntBigEndian (8 + w * h);

        for (int y = 0; y < h; ++y)
        {
            for (int x = 0; x < w; ++x)
            {
                const Colour pixel (bitmap.getPixelColour (x, y));
                out.writeByte ((char) pixel.getAlpha());
            }
        }
    }

    static void writeNewIconFormat (MemoryOutputStream& out, const Image& image, const char* type)
    {
        MemoryOutputStream pngData;
        PNGImageFormat pngFormat;
        pngFormat.writeImageToStream (image, pngData);

        out.write (type, 4);
        out.writeIntBigEndian (8 + pngData.getDataSize());
        out << pngData;
    }

    void writeIcnsFile (const Array<Image>& images, OutputStream& out) const
    {
        MemoryOutputStream data;

        for (int i = 0; i < images.size(); ++i)
        {
            const Image image (fixMacIconImageSize (images.getReference (i)));
            jassert (image.getWidth() == image.getHeight());

            switch (image.getWidth())
            {
                case 16:   writeOldIconFormat (data, image, "is32", "s8mk"); break;
                case 32:   writeOldIconFormat (data, image, "il32", "l8mk"); break;
                case 48:   writeOldIconFormat (data, image, "ih32", "h8mk"); break;
                case 128:  writeOldIconFormat (data, image, "it32", "t8mk"); break;
                case 256:  writeNewIconFormat (data, image, "ic08"); break;
                case 512:  writeNewIconFormat (data, image, "ic09"); break;
                case 1024: writeNewIconFormat (data, image, "ic10"); break;
                default:   break;
            }
        }

        jassert (data.getDataSize() > 0); // no suitable sized images?

        out.write ("icns", 4);
        out.writeIntBigEndian (data.getDataSize() + 8);
        out << data;
    }

    void createIconFile() const
    {
        Array<Image> images;

        Image bigIcon (getBigIcon());
        if (bigIcon.isValid())
            images.add (bigIcon);

        Image smallIcon (getSmallIcon());
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

    void writeInfoPlistFile() const
    {
        if (! xcodeCreatePList)
            return;

        ScopedPointer<XmlElement> plist (XmlDocument::parse (getPListToMergeString()));

        if (plist == nullptr || ! plist->hasTagName ("plist"))
            plist = new XmlElement ("plist");

        XmlElement* dict = plist->getChildByName ("dict");

        if (dict == nullptr)
            dict = plist->createNewChildElement ("dict");

        if (iOS)
            addPlistDictionaryKeyBool (dict, "LSRequiresIPhoneOS", true);

        addPlistDictionaryKey (dict, "CFBundleExecutable",          "${EXECUTABLE_NAME}");
        addPlistDictionaryKey (dict, "CFBundleIconFile",            iconFile.exists() ? iconFile.getFileName() : String::empty);
        addPlistDictionaryKey (dict, "CFBundleIdentifier",          project.getBundleIdentifier().toString());
        addPlistDictionaryKey (dict, "CFBundleName",                projectName);
        addPlistDictionaryKey (dict, "CFBundlePackageType",         xcodePackageType);
        addPlistDictionaryKey (dict, "CFBundleSignature",           xcodeBundleSignature);
        addPlistDictionaryKey (dict, "CFBundleShortVersionString",  project.getVersionString());
        addPlistDictionaryKey (dict, "CFBundleVersion",             project.getVersionString());
        addPlistDictionaryKey (dict, "NSHumanReadableCopyright",    project.getCompanyName().toString());
        addPlistDictionaryKeyBool (dict, "NSHighResolutionCapable", true);

        StringArray documentExtensions;
        documentExtensions.addTokens (replacePreprocessorDefs (getAllPreprocessorDefs(), settings ["documentExtensions"]),
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

        if (settings ["UIFileSharingEnabled"])
            addPlistDictionaryKeyBool (dict, "UIFileSharingEnabled", true);

        if (settings ["UIStatusBarHidden"])
            addPlistDictionaryKeyBool (dict, "UIStatusBarHidden", true);

        for (int i = 0; i < xcodeExtraPListEntries.size(); ++i)
            dict->addChildElement (new XmlElement (xcodeExtraPListEntries.getReference(i)));

        MemoryOutputStream mo;
        plist->writeToStream (mo, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");

        overwriteFileIfDifferentOrThrow (infoPlistFile, mo);
    }

    StringArray getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths.removeDuplicates (false);
        return searchPaths;
    }

    void getLinkerFlagsForStaticLibrary (const RelativePath& library, StringArray& flags, StringArray& librarySearchPaths) const
    {
        jassert (library.getFileNameWithoutExtension().substring (0, 3) == "lib");

        flags.add ("-l" + library.getFileNameWithoutExtension().substring (3));

        String searchPath (library.toUnixStyle().upToLastOccurrenceOf ("/", false, false));

        if (! library.isAbsolute())
        {
            String srcRoot (rebaseFromProjectFolderToBuildTarget (RelativePath (".", RelativePath::projectFolder)).toUnixStyle());

            if (srcRoot.endsWith ("/."))      srcRoot = srcRoot.dropLastCharacters (2);
            if (! srcRoot.endsWithChar ('/')) srcRoot << '/';

            searchPath = srcRoot + searchPath;
        }

        librarySearchPaths.add (sanitisePath (searchPath));
    }

    void getLinkerFlags (const BuildConfiguration& config, StringArray& flags, StringArray& librarySearchPaths) const
    {
        if (xcodeIsBundle)
            flags.add ("-bundle");

        const Array<RelativePath>& extraLibs = config.isDebug() ? xcodeExtraLibrariesDebug
                                                                : xcodeExtraLibrariesRelease;

        for (int i = 0; i < extraLibs.size(); ++i)
            getLinkerFlagsForStaticLibrary (extraLibs.getReference(i), flags, librarySearchPaths);

        flags.add (replacePreprocessorTokens (config, getExtraLinkerFlagsString()));
        flags.removeEmptyStrings (true);
    }

    StringArray getProjectSettings (const XcodeBuildConfiguration& config) const
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

        if (projectType.isLibrary())
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = NO");
        }
        else
        {
            s.add ("GCC_INLINES_ARE_PRIVATE_EXTERN = YES");
        }

        if (iOS)
        {
            s.add ("\"CODE_SIGN_IDENTITY[sdk=iphoneos*]\" = \"iPhone Developer\"");
            s.add ("SDKROOT = iphoneos");
            s.add ("TARGETED_DEVICE_FAMILY = \"1,2\"");

            const String iosVersion (config.getiOSCompatibilityVersion());
            if (iosVersion.isNotEmpty() && iosVersion != osxVersionDefault)
                s.add ("IPHONEOS_DEPLOYMENT_TARGET = " + iosVersion);
        }

        s.add ("ZERO_LINK = NO");

        if (xcodeCanUseDwarf)
            s.add ("DEBUG_INFORMATION_FORMAT = \"dwarf\"");

        s.add ("PRODUCT_NAME = \"" + config.getTargetBinaryNameString() + "\"");
        return s;
    }

    StringArray getTargetSettings (const XcodeBuildConfiguration& config) const
    {
        StringArray s;

        const String arch (config.getMacArchitecture());
        if (arch == osxArch_Native)                s.add ("ARCHS = \"$(ARCHS_NATIVE)\"");
        else if (arch == osxArch_32BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_BIT)\"");
        else if (arch == osxArch_64BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\"");
        else if (arch == osxArch_64Bit)            s.add ("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\"");

        s.add ("HEADER_SEARCH_PATHS = \"" + replacePreprocessorTokens (config, getHeaderSearchPaths (config).joinIntoString (" ")) + " $(inherited)\"");
        s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());
        s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

        const String extraFlags (replacePreprocessorTokens (config, getExtraCompilerFlagsString()).trim());
        if (extraFlags.isNotEmpty())
            s.add ("OTHER_CPLUSPLUSFLAGS = \"" + extraFlags + "\"");

        if (xcodeProductInstallPath.isNotEmpty())
            s.add ("INSTALL_PATH = \"" + xcodeProductInstallPath + "\"");

        if (xcodeIsBundle)
        {
            s.add ("LIBRARY_STYLE = Bundle");
            s.add ("WRAPPER_EXTENSION = " + xcodeBundleExtension.substring (1));
            s.add ("GENERATE_PKGINFO_FILE = YES");
        }

        if (xcodeOtherRezFlags.isNotEmpty())
            s.add ("OTHER_REZFLAGS = \"" + xcodeOtherRezFlags + "\"");

        if (projectType.isLibrary())
        {
            if (config.getTargetBinaryRelativePathString().isNotEmpty())
            {
                RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
                binaryPath = binaryPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder);

                s.add ("DSTROOT = " + sanitisePath (binaryPath.toUnixStyle()));
                s.add ("SYMROOT = " + sanitisePath (binaryPath.toUnixStyle()));
            }

            s.add ("CONFIGURATION_BUILD_DIR = \"$(BUILD_DIR)\"");
            s.add ("DEPLOYMENT_LOCATION = YES");
        }

        String gccVersion ("com.apple.compilers.llvm.clang.1_0");

        if (! iOS)
        {
            const String sdk (config.getMacSDKVersion());
            const String sdkCompat (config.getMacCompatibilityVersion());

            if (sdk == osxVersion10_5)          s.add ("SDKROOT = macosx10.5");
            else if (sdk == osxVersion10_6)     s.add ("SDKROOT = macosx10.6");
            else if (sdk == osxVersion10_7)     s.add ("SDKROOT = macosx10.7");

            if (sdkCompat == osxVersion10_4)       s.add ("MACOSX_DEPLOYMENT_TARGET = 10.4");
            else if (sdkCompat == osxVersion10_5)  s.add ("MACOSX_DEPLOYMENT_TARGET = 10.5");
            else if (sdkCompat == osxVersion10_6)  s.add ("MACOSX_DEPLOYMENT_TARGET = 10.6");
            else if (sdkCompat == osxVersion10_7)  s.add ("MACOSX_DEPLOYMENT_TARGET = 10.7");

            s.add ("MACOSX_DEPLOYMENT_TARGET_ppc = 10.4");
            s.add ("SDKROOT_ppc = macosx10.5");

            if (xcodeExcludedFiles64Bit.isNotEmpty())
            {
                s.add ("EXCLUDED_SOURCE_FILE_NAMES = \"$(EXCLUDED_SOURCE_FILE_NAMES_$(CURRENT_ARCH))\"");
                s.add ("EXCLUDED_SOURCE_FILE_NAMES_x86_64 = " + xcodeExcludedFiles64Bit);
            }
        }

        s.add ("GCC_VERSION = " + gccVersion);
        s.add ("CLANG_CXX_LANGUAGE_STANDARD = \"c++0x\"");

        if (config.getCppLibType().isNotEmpty())
            s.add ("CLANG_CXX_LIBRARY = " + config.getCppLibType().quoted());

        {
            StringArray linkerFlags, librarySearchPaths;
            getLinkerFlags (config, linkerFlags, librarySearchPaths);

            if (linkerFlags.size() > 0)
                s.add ("OTHER_LDFLAGS = \"" + linkerFlags.joinIntoString (" ") + "\"");

            librarySearchPaths.addArray (config.getLibrarySearchPaths());
            librarySearchPaths.removeDuplicates (false);

            if (librarySearchPaths.size() > 0)
            {
                String libPaths ("LIBRARY_SEARCH_PATHS = (\"$(inherited)\"");

                for (int i = 0; i < librarySearchPaths.size(); ++i)
                    libPaths += ", \"\\\"" + librarySearchPaths[i] + "\\\"\"";

                s.add (libPaths + ")");
            }
        }

        StringPairArray defines;

        if (config.isDebug())
        {
            defines.set ("_DEBUG", "1");
            defines.set ("DEBUG", "1");
            s.add ("ONLY_ACTIVE_ARCH = YES");
            s.add ("COPY_PHASE_STRIP = NO");
            s.add ("GCC_DYNAMIC_NO_PIC = NO");
        }
        else
        {
            defines.set ("_NDEBUG", "1");
            defines.set ("NDEBUG", "1");
            s.add ("GCC_GENERATE_DEBUGGING_SYMBOLS = NO");
            s.add ("GCC_SYMBOLS_PRIVATE_EXTERN = YES");
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

        s.addTokens (config.getCustomXcodeFlags(), ",", "\"'");
        s.trim();
        s.removeEmptyStrings();
        s.removeDuplicates (false);

        return s;
    }

    void addFrameworks() const
    {
        if (! isStaticLibrary())
        {
            StringArray s (xcodeFrameworks);
            s.addTokens (getExtraFrameworksString(), ",;", "\"'");

            s.trim();
            s.removeDuplicates (true);
            s.sort (true);

            for (int i = 0; i < s.size(); ++i)
                addFramework (s[i]);
        }
    }

    //==============================================================================
    void writeProjectFile (OutputStream& output) const
    {
        output << "// !$*UTF8*$!\n{\n"
                  "\tarchiveVersion = 1;\n"
                  "\tclasses = {\n\t};\n"
                  "\tobjectVersion = 46;\n"
                  "\tobjects = {\n\n";

        Array <ValueTree*> objects;
        objects.addArray (pbxBuildFiles);
        objects.addArray (pbxFileReferences);
        objects.addArray (pbxGroups);
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
        forEachXmlChildElementWithTagName (*xml, e, "key")
        {
            if (e->getAllSubText().trim().equalsIgnoreCase (key))
            {
                if (e->getNextElement() != nullptr && e->getNextElement()->hasTagName ("key"))
                {
                    // try to fix broken plist format..
                    xml->removeChildElement (e, true);
                    break;
                }
                else
                {
                    return; // (value already exists)
                }
            }
        }

        xml->createNewChildElement ("key")   ->addTextElement (key);
        xml->createNewChildElement ("string")->addTextElement (value);
    }

    static void addPlistDictionaryKeyBool (XmlElement* xml, const String& key, const bool value)
    {
        xml->createNewChildElement ("key")->addTextElement (key);
        xml->createNewChildElement (value ? "true" : "false");
    }

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings) const
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

    String addBuildFile (const RelativePath& path, bool addToSourceBuildPhase, bool inhibitWarnings) const
    {
        return addBuildFile (path.toUnixStyle(), createFileRefID (path), addToSourceBuildPhase, inhibitWarnings);
    }

    String addFileReference (String pathString) const
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

        const String fileRefID (createFileRefID (pathString));

        ScopedPointer<ValueTree> v (new ValueTree (fileRefID));
        v->setProperty ("isa", "PBXFileReference", 0);
        v->setProperty ("lastKnownFileType", getFileType (path), 0);
        v->setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), 0);
        v->setProperty ("path", sanitisePath (pathString), 0);
        v->setProperty ("sourceTree", sourceTree, 0);

        const int existing = pbxFileReferences.indexOfSorted (*this, v);

        if (existing >= 0)
        {
            // If this fails, there's either a string hash collision, or the same file is being added twice (incorrectly)
            jassert (pbxFileReferences.getUnchecked (existing)->isEquivalentTo (*v));
        }
        else
        {
            pbxFileReferences.addSorted (*this, v.release());
        }

        return fileRefID;
    }

public:
    static int compareElements (const ValueTree* first, const ValueTree* second)
    {
        return first->getType().getCharPointer().compare (second->getType().getCharPointer());
    }

private:
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
        else if (file.hasFileExtension ("xml;zip;wav"))          return "file" + file.getFileExtension();
        else if (file.hasFileExtension ("txt;rtf"))              return "text" + file.getFileExtension();
        else if (file.hasFileExtension ("plist"))                return "text.plist.xml";
        else if (file.hasFileExtension ("app"))                  return "wrapper.application";
        else if (file.hasFileExtension ("component;vst;plugin")) return "wrapper.cfbundle";
        else if (file.hasFileExtension ("xcodeproj"))            return "wrapper.pb-project";
        else if (file.hasFileExtension ("a"))                    return "archive.ar";

        return "file" + file.getFileExtension();
    }

    String addFile (const RelativePath& path, bool shouldBeCompiled, bool inhibitWarnings) const
    {
        const String pathAsString (path.toUnixStyle());
        const String refID (addFileReference (path.toUnixStyle()));

        if (shouldBeCompiled)
        {
            if (path.hasFileExtension (".r"))
                rezFileIDs.add (addBuildFile (pathAsString, refID, false, inhibitWarnings));
            else
                addBuildFile (pathAsString, refID, true, inhibitWarnings);
        }

        return refID;
    }

    String addProjectItem (const Project::Item& projectItem) const
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
                bool inhibitWarnings = projectItem.shouldInhibitWarnings();

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

    void addFramework (const String& frameworkName) const
    {
        const String path ("System/Library/Frameworks/" + frameworkName + ".framework");
        const String fileRefID (createFileRefID (path));
        addFileReference ("${SDKROOT}/" + path);
        frameworkIDs.add (addBuildFile (path, fileRefID, false, false));
        frameworkFileIDs.add (fileRefID);
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs) const
    {
        ValueTree* v = new ValueTree (groupID);
        v->setProperty ("isa", "PBXGroup", 0);
        v->setProperty ("children", "(" + indentList (childIDs, ",") + " )", 0);
        v->setProperty (Ids::name, groupName, 0);
        v->setProperty ("sourceTree", "<group>", 0);
        pbxGroups.add (v);
    }

    String addGroup (const Project::Item& item, StringArray& childIDs) const
    {
        const String groupName (item.getName());
        const String groupID (getIDForGroup (item));
        addGroup (groupID, groupName, childIDs);
        return groupID;
    }

    void addMainBuildProduct() const
    {
        jassert (xcodeFileType.isNotEmpty());
        jassert (xcodeBundleExtension.isEmpty() || xcodeBundleExtension.startsWithChar('.'));
        String productName (getConfiguration(0)->getTargetBinaryName().toString());

        if (xcodeFileType == "archive.ar")
            productName = getLibbedFilename (productName);
        else
            productName += xcodeBundleExtension;

        addBuildProduct (xcodeFileType, productName);
    }

    void addBuildProduct (const String& fileType, const String& binaryName) const
    {
        ValueTree* v = new ValueTree (createID ("__productFileID"));
        v->setProperty ("isa", "PBXFileReference", 0);
        v->setProperty ("explicitFileType", fileType, 0);
        v->setProperty ("includeInIndex", (int) 0, 0);
        v->setProperty ("path", sanitisePath (binaryName), 0);
        v->setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", 0);
        pbxFileReferences.add (v);
    }

    void addTargetConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("targetconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", 0);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", 0);
        v->setProperty (Ids::name, configName, 0);
        targetConfigs.add (v);
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", 0);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", 0);
        v->setProperty (Ids::name, configName, 0);
        projectConfigs.add (v);
    }

    void addConfigList (const OwnedArray <ValueTree>& configsToUse, const String& listID) const
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

    ValueTree* addBuildPhase (const String& phaseType, const StringArray& fileIds) const
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

    void addTargetObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__target"));
        v->setProperty ("isa", "PBXNativeTarget", 0);
        v->setProperty ("buildConfigurationList", createID ("__configList"), 0);
        v->setProperty ("buildPhases", "(" + indentList (buildPhaseIDs, ",") + " )", 0);
        v->setProperty ("buildRules", "( )", 0);
        v->setProperty ("dependencies", "( )", 0);
        v->setProperty (Ids::name, projectName, 0);
        v->setProperty ("productName", projectName, 0);
        v->setProperty ("productReference", createID ("__productFileID"), 0);

        if (xcodeProductInstallPath.isNotEmpty())
            v->setProperty ("productInstallPath", xcodeProductInstallPath, 0);

        jassert (xcodeProductType.isNotEmpty());
        v->setProperty ("productType", xcodeProductType, 0);

        misc.add (v);
    }

    void addProjectObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__root"));
        v->setProperty ("isa", "PBXProject", 0);
        v->setProperty ("buildConfigurationList", createID ("__projList"), 0);
        v->setProperty ("compatibilityVersion", "Xcode 3.2", 0);
        v->setProperty ("hasScannedForEncodings", (int) 0, 0);
        v->setProperty ("mainGroup", createID ("__mainsourcegroup"), 0);
        v->setProperty ("projectDirPath", "\"\"", 0);
        v->setProperty ("projectRoot", "\"\"", 0);
        v->setProperty ("targets", "( " + createID ("__target") + " )", 0);
        misc.add (v);
    }

    void addShellScriptPhase() const
    {
        if (getPostBuildScript().isNotEmpty())
        {
            ValueTree* const v = addBuildPhase ("PBXShellScriptBuildPhase", StringArray());
            v->setProperty (Ids::name, "Post-build script", 0);
            v->setProperty ("shellPath", "/bin/sh", 0);
            v->setProperty ("shellScript", getPostBuildScript().replace ("\\", "\\\\")
                                                               .replace ("\"", "\\\"")
                                                               .replace ("\r\n", "\\n")
                                                               .replace ("\n", "\\n"), 0);
        }
    }

    //==============================================================================
    static String indentList (const StringArray& list, const String& separator)
    {
        if (list.size() == 0)
            return " ";

        return "\n\t\t\t\t" + list.joinIntoString (separator + "\n\t\t\t\t")
                  + (separator == ";" ? separator : String::empty);
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        rootString += project.getProjectUID();

        return MD5 (rootString.toUTF8()).toHexString().substring (0, 24).toUpperCase();
    }

    String createFileRefID (const RelativePath& path) const
    {
        return createFileRefID (path.toUnixStyle());
    }

    String createFileRefID (const String& path) const
    {
        return createID ("__fileref_" + path);
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
