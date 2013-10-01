/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace
{
    const char* const osxVersionDefault         = "default";
    const int oldestSDKVersion = 4;
    const int currentSDKVersion = 9;

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
    XCodeProjectExporter (Project& p, const ValueTree& t, const bool isIOS)
        : ProjectExporter (p, t),
          iOS (isIOS)
    {
        name = iOS ? getNameiOS() : getNameMac();

        if (getTargetLocationString().isEmpty())
            getTargetLocationValue() = getDefaultBuildsRootFolder() + (iOS ? "iOS" : "MacOSX");
    }

    static XCodeProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName (false)))  return new XCodeProjectExporter (project, settings, false);
        if (settings.hasType (getValueTreeTypeName (true)))   return new XCodeProjectExporter (project, settings, true);

        return nullptr;
    }

    //==============================================================================
    Value getPListToMergeValue()            { return getSetting ("customPList"); }
    String getPListToMergeString() const    { return settings   ["customPList"]; }

    Value getExtraFrameworksValue()         { return getSetting (Ids::extraFrameworks); }
    String getExtraFrameworksString() const { return settings   [Ids::extraFrameworks]; }

    Value  getPostBuildScriptValue()        { return getSetting (Ids::postbuildCommand); }
    String getPostBuildScript() const       { return settings   [Ids::postbuildCommand]; }

    Value  getPreBuildScriptValue()         { return getSetting (Ids::prebuildCommand); }
    String getPreBuildScript() const        { return settings   [Ids::prebuildCommand]; }

    bool usesMMFiles() const override                { return true; }
    bool isXcode() const override                    { return true; }
    bool isOSX() const override                      { return ! iOS; }
    bool canCopeWithDuplicateFiles() override        { return true; }

    void createExporterProperties (PropertyListBuilder& props) override
    {
        if (projectType.isGUIApplication() && ! iOS)
        {
            props.add (new TextPropertyComponent (getSetting ("documentExtensions"), "Document file extensions", 128, false),
                       "A comma-separated list of file extensions for documents that your app can open. "
                       "Using a leading '.' is optional, and the extensions are not case-sensitive.");
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

        props.add (new TextPropertyComponent (getPreBuildScriptValue(), "Pre-build shell script", 32768, true),
                   "Some shell-script that will be run before a build starts.");

        props.add (new TextPropertyComponent (getPostBuildScriptValue(), "Post-build shell script", 32768, true),
                   "Some shell-script that will be run after a build completes.");
    }

    bool launchProject() override
    {
       #if JUCE_MAC
        return getProjectBundle().startAsProcess();
       #else
        return false;
       #endif
    }

    bool canLaunchProject() override
    {
       #if JUCE_MAC
        return true;
       #else
        return false;
       #endif
    }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>&) const override
    {
        infoPlistFile = getTargetFolder().getChildFile ("Info.plist");
        menuNibFile = getTargetFolder().getChildFile ("RecentFilesMenuTemplate.nib");

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
    //==============================================================================
    class XcodeBuildConfiguration  : public BuildConfiguration
    {
    public:
        XcodeBuildConfiguration (Project& p, const ValueTree& t, const bool isIOS)
            : BuildConfiguration (p, t), iOS (isIOS)
        {
            if (iOS)
            {
                if (getiOSCompatibilityVersion().isEmpty())
                    getiOSCompatibilityVersionValue() = osxVersionDefault;
            }
            else
            {
                if (getMacSDKVersion().isEmpty())
                    getMacSDKVersionValue() = osxVersionDefault;

                if (getMacCompatibilityVersion().isEmpty())
                    getMacCompatibilityVersionValue() = osxVersionDefault;

                if (getMacArchitecture().isEmpty())
                    getMacArchitectureValue() = osxArch_Default;
            }
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

        void createConfigProperties (PropertyListBuilder& props)
        {
            if (iOS)
            {
                const char* iosVersions[]      = { "Use Default",     "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", 0 };
                const char* iosVersionValues[] = { osxVersionDefault, "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", 0 };

                props.add (new ChoicePropertyComponent (getiOSCompatibilityVersionValue(), "iOS Deployment Target",
                                                        StringArray (iosVersions), Array<var> (iosVersionValues)),
                           "The minimum version of iOS that the target binary will run on.");
            }
            else
            {
                StringArray versionNames;
                Array<var> versionValues;

                versionNames.add ("Use Default");
                versionValues.add (osxVersionDefault);

                for (int ver = oldestSDKVersion; ver <= currentSDKVersion; ++ver)
                {
                    versionNames.add (getSDKName (ver));
                    versionValues.add (getSDKName (ver));
                }

                props.add (new ChoicePropertyComponent (getMacSDKVersionValue(), "OSX Base SDK Version", versionNames, versionValues),
                           "The version of OSX to link against in the XCode build.");

                props.add (new ChoicePropertyComponent (getMacCompatibilityVersionValue(), "OSX Compatibility Version", versionNames, versionValues),
                           "The minimum version of OSX that the target binary will be compatible with.");

                const char* osxArch[] = { "Use Default", "Native architecture of build machine",
                                          "Universal Binary (32-bit)", "Universal Binary (32/64-bit)", "64-bit Intel", 0 };
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

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& v) const override
    {
        return new XcodeBuildConfiguration (project, v, iOS);
    }

private:
    mutable OwnedArray<ValueTree> pbxBuildFiles, pbxFileReferences, pbxGroups, misc, projectConfigs, targetConfigs;
    mutable StringArray buildPhaseIDs, resourceIDs, sourceIDs, frameworkIDs;
    mutable StringArray frameworkFileIDs, rezFileIDs, resourceFileRefs;
    mutable File infoPlistFile, menuNibFile, iconFile;
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

        if (! iOS)
        {
            MemoryOutputStream nib;
            nib.write (BinaryData::RecentFilesMenuTemplate_nib, BinaryData::RecentFilesMenuTemplate_nibSize);
            overwriteFileIfDifferentOrThrow (menuNibFile, nib);

            RelativePath menuNibPath (menuNibFile, getTargetFolder(), RelativePath::buildTargetFolder);
            addFileReference (menuNibPath.toUnixStyle());
            resourceIDs.add (addBuildFile (menuNibPath, false, false));
            resourceFileRefs.add (createFileRefID (menuNibPath));
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

            for (int i = 0; i < getAllGroups().size(); ++i)
            {
                const Project::Item& group = getAllGroups().getReference(i);

                if (group.getNumChildren() > 0)
                    topLevelGroupIDs.add (addProjectItem (group));
            }

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

        addShellScriptBuildPhase ("Pre-build script", getPreBuildScript());

        if (! projectType.isStaticLibrary())
            addBuildPhase ("PBXResourcesBuildPhase", resourceIDs);

        if (rezFileIDs.size() > 0)
            addBuildPhase ("PBXRezBuildPhase", rezFileIDs);

        addBuildPhase ("PBXSourcesBuildPhase", sourceIDs);

        if (! projectType.isStaticLibrary())
            addBuildPhase ("PBXFrameworksBuildPhase", frameworkIDs);

        addShellScriptBuildPhase ("Post-build script", getPostBuildScript());

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
        out.writeIntBigEndian (8 + (int) pngData.getDataSize());
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
        out.writeIntBigEndian ((int) data.getDataSize() + 8);
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
            XmlElement* arrayTag = nullptr;

            for (int i = 0; i < documentExtensions.size(); ++i)
            {
                String ex (documentExtensions[i]);
                if (ex.startsWithChar ('.'))
                    ex = ex.substring (1);

                if (arrayTag == nullptr)
                {
                    dict2->createNewChildElement ("key")->addTextElement ("CFBundleTypeExtensions");
                    arrayTag = dict2->createNewChildElement ("array");

                    addPlistDictionaryKey (dict2, "CFBundleTypeName", ex);
                    addPlistDictionaryKey (dict2, "CFBundleTypeRole", "Editor");
                    addPlistDictionaryKey (dict2, "NSPersistentStoreTypeKey", "XML");
                }

                arrayTag->createNewChildElement ("string")->addTextElement (ex);
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
        flags.add (getExternalLibraryFlags (config));

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

        if (projectType.isStaticLibrary())
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

        if (config.getTargetBinaryRelativePathString().isNotEmpty())
        {
            RelativePath binaryPath (config.getTargetBinaryRelativePathString(), RelativePath::projectFolder);
            binaryPath = binaryPath.rebased (projectFolder, getTargetFolder(), RelativePath::buildTargetFolder);

            s.add ("DSTROOT = " + sanitisePath (binaryPath.toUnixStyle()));
            s.add ("SYMROOT = " + sanitisePath (binaryPath.toUnixStyle()));
        }
        else
        {
            s.add ("CONFIGURATION_BUILD_DIR = \"$(PROJECT_DIR)/build/$(CONFIGURATION)\"");
        }

        String gccVersion ("com.apple.compilers.llvm.clang.1_0");

        if (! iOS)
        {
            const String sdk (config.getMacSDKVersion());
            const String sdkCompat (config.getMacCompatibilityVersion());

            for (int ver = oldestSDKVersion; ver <= currentSDKVersion; ++ver)
            {
                if (sdk == getSDKName (ver))         s.add ("SDKROOT = macosx10." + String (ver));
                if (sdkCompat == getSDKName (ver))   s.add ("MACOSX_DEPLOYMENT_TARGET = 10." + String (ver));
            }

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
        s.add ("CLANG_LINK_OBJC_RUNTIME = NO");

        if (config.getCppLibType().isNotEmpty())
            s.add ("CLANG_CXX_LIBRARY = " + config.getCppLibType().quoted());

        s.add ("COMBINE_HIDPI_IMAGES = YES");

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

            if (config.getMacArchitecture() == osxArch_Default
                 || config.getMacArchitecture().isEmpty())
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
            s.add ("DEAD_CODE_STRIPPING = YES");
        }

        {
            defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));

            StringArray defsList;

            for (int i = 0; i < defines.size(); ++i)
            {
                String def (defines.getAllKeys()[i]);
                const String value (defines.getAllValues()[i]);
                if (value.isNotEmpty())
                    def << "=" << value.replace ("\"", "\\\"");

                defsList.add ("\"" + def + "\"");
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
        if (! projectType.isStaticLibrary())
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

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-_@~\r\n")
                                        && ! (val.trimStart().startsWithChar ('(')
                                                || val.trimStart().startsWithChar ('{'))))
                    val = "\"" + val + "\"";

                output << propertyName.toString() << " = " << val << "; ";
            }

            output << "};\n";
        }

        output << "\t};\n\trootObject = " << createID ("__root") << ";\n}\n";
    }

    String addBuildFile (const String& path, const String& fileRefID, bool addToSourceBuildPhase, bool inhibitWarnings) const
    {
        String fileID (createID (path + "buildref"));

        if (addToSourceBuildPhase)
            sourceIDs.add (fileID);

        ValueTree* v = new ValueTree (fileID);
        v->setProperty ("isa", "PBXBuildFile", nullptr);
        v->setProperty ("fileRef", fileRefID, nullptr);

        if (inhibitWarnings)
            v->setProperty ("settings", "{COMPILER_FLAGS = \"-w\"; }", nullptr);

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
        v->setProperty ("isa", "PBXFileReference", nullptr);
        v->setProperty ("lastKnownFileType", getFileType (path), nullptr);
        v->setProperty (Ids::name, pathString.fromLastOccurrenceOf ("/", false, false), nullptr);
        v->setProperty ("path", sanitisePath (pathString), nullptr);
        v->setProperty ("sourceTree", sourceTree, nullptr);

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
        if (file.hasFileExtension ("cpp;cc;cxx"))           return "sourcecode.cpp.cpp";
        if (file.hasFileExtension (".mm"))                  return "sourcecode.cpp.objcpp";
        if (file.hasFileExtension (".m"))                   return "sourcecode.c.objc";
        if (file.hasFileExtension (".c"))                   return "sourcecode.c.c";
        if (file.hasFileExtension (headerFileExtensions))   return "sourcecode.c.h";
        if (file.hasFileExtension (".framework"))           return "wrapper.framework";
        if (file.hasFileExtension (".jpeg;.jpg"))           return "image.jpeg";
        if (file.hasFileExtension ("png;gif"))              return "image" + file.getFileExtension();
        if (file.hasFileExtension ("html;htm"))             return "text.html";
        if (file.hasFileExtension ("xml;zip;wav"))          return "file" + file.getFileExtension();
        if (file.hasFileExtension ("txt;rtf"))              return "text" + file.getFileExtension();
        if (file.hasFileExtension ("plist"))                return "text.plist.xml";
        if (file.hasFileExtension ("app"))                  return "wrapper.application";
        if (file.hasFileExtension ("component;vst;plugin")) return "wrapper.cfbundle";
        if (file.hasFileExtension ("xcodeproj"))            return "wrapper.pb-project";
        if (file.hasFileExtension ("a"))                    return "archive.ar";

        return "file" + file.getFileExtension();
    }

    String addFile (const RelativePath& path, bool shouldBeCompiled, bool shouldBeAddedToBinaryResources, bool inhibitWarnings) const
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
        else if (! shouldBeAddedToBinaryResources)
        {
            const String fileType (getFileType (path));

            if (fileType.startsWith ("image.") || fileType.startsWith ("text.") || fileType.startsWith ("file."))
            {
                resourceIDs.add (addBuildFile (pathAsString, refID, false, false));
                resourceFileRefs.add (refID);
            }
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

        if (projectItem.shouldBeAddedToTargetProject())
        {
            const String itemPath (projectItem.getFilePath());
            RelativePath path;

            if (itemPath.startsWith ("${"))
                path = RelativePath (itemPath, RelativePath::unknown);
            else
                path = RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder);

            return addFile (path, projectItem.shouldBeCompiled(),
                            projectItem.shouldBeAddedToBinaryResources(),
                            projectItem.shouldInhibitWarnings());
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
        v->setProperty ("isa", "PBXGroup", nullptr);
        v->setProperty ("children", "(" + indentList (childIDs, ",") + " )", nullptr);
        v->setProperty (Ids::name, groupName, nullptr);
        v->setProperty ("sourceTree", "<group>", nullptr);
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
        v->setProperty ("isa", "PBXFileReference", nullptr);
        v->setProperty ("explicitFileType", fileType, nullptr);
        v->setProperty ("includeInIndex", (int) 0, nullptr);
        v->setProperty ("path", sanitisePath (binaryName), nullptr);
        v->setProperty ("sourceTree", "BUILT_PRODUCTS_DIR", nullptr);
        pbxFileReferences.add (v);
    }

    void addTargetConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("targetconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", nullptr);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", nullptr);
        v->setProperty (Ids::name, configName, nullptr);
        targetConfigs.add (v);
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", nullptr);
        v->setProperty ("buildSettings", "{" + indentList (buildSettings, ";") + " }", nullptr);
        v->setProperty (Ids::name, configName, nullptr);
        projectConfigs.add (v);
    }

    void addConfigList (const OwnedArray <ValueTree>& configsToUse, const String& listID) const
    {
        StringArray configIDs;

        for (int i = 0; i < configsToUse.size(); ++i)
            configIDs.add (configsToUse[i]->getType().toString());

        ValueTree* v = new ValueTree (listID);
        v->setProperty ("isa", "XCConfigurationList", nullptr);
        v->setProperty ("buildConfigurations", "(" + indentList (configIDs, ",") + " )", nullptr);
        v->setProperty ("defaultConfigurationIsVisible", (int) 0, nullptr);

        if (configsToUse[0] != nullptr)
            v->setProperty ("defaultConfigurationName", configsToUse[0]->getProperty (Ids::name), nullptr);

        misc.add (v);
    }

    ValueTree& addBuildPhase (const String& phaseType, const StringArray& fileIds) const
    {
        String phaseId (createID (phaseType + "resbuildphase"));

        int n = 0;
        while (buildPhaseIDs.contains (phaseId))
            phaseId = createID (phaseType + "resbuildphase" + String (++n));

        buildPhaseIDs.add (phaseId);

        ValueTree* v = new ValueTree (phaseId);
        v->setProperty ("isa", phaseType, nullptr);
        v->setProperty ("buildActionMask", "2147483647", nullptr);
        v->setProperty ("files", "(" + indentList (fileIds, ",") + " )", nullptr);
        v->setProperty ("runOnlyForDeploymentPostprocessing", (int) 0, nullptr);
        misc.add (v);
        return *v;
    }

    void addTargetObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__target"));
        v->setProperty ("isa", "PBXNativeTarget", nullptr);
        v->setProperty ("buildConfigurationList", createID ("__configList"), nullptr);
        v->setProperty ("buildPhases", "(" + indentList (buildPhaseIDs, ",") + " )", nullptr);
        v->setProperty ("buildRules", "( )", nullptr);
        v->setProperty ("dependencies", "( )", nullptr);
        v->setProperty (Ids::name, projectName, nullptr);
        v->setProperty ("productName", projectName, nullptr);
        v->setProperty ("productReference", createID ("__productFileID"), nullptr);

        if (xcodeProductInstallPath.isNotEmpty())
            v->setProperty ("productInstallPath", xcodeProductInstallPath, nullptr);

        jassert (xcodeProductType.isNotEmpty());
        v->setProperty ("productType", xcodeProductType, nullptr);

        misc.add (v);
    }

    void addProjectObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__root"));
        v->setProperty ("isa", "PBXProject", nullptr);
        v->setProperty ("buildConfigurationList", createID ("__projList"), nullptr);
        v->setProperty ("attributes", "{ LastUpgradeCheck = 0440; }", nullptr);
        v->setProperty ("compatibilityVersion", "Xcode 3.2", nullptr);
        v->setProperty ("hasScannedForEncodings", (int) 0, nullptr);
        v->setProperty ("mainGroup", createID ("__mainsourcegroup"), nullptr);
        v->setProperty ("projectDirPath", "\"\"", nullptr);
        v->setProperty ("projectRoot", "\"\"", nullptr);
        v->setProperty ("targets", "( " + createID ("__target") + " )", nullptr);
        misc.add (v);
    }

    void addShellScriptBuildPhase (const String& phaseName, const String& script) const
    {
        if (script.trim().isNotEmpty())
        {
            ValueTree& v = addBuildPhase ("PBXShellScriptBuildPhase", StringArray());
            v.setProperty (Ids::name, phaseName, nullptr);
            v.setProperty ("shellPath", "/bin/sh", nullptr);
            v.setProperty ("shellScript", script.replace ("\\", "\\\\")
                                                .replace ("\"", "\\\"")
                                                .replace ("\r\n", "\\n")
                                                .replace ("\n", "\\n"), nullptr);
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

    static String getSDKName (int version)
    {
        jassert (version >= 4);
        return "10." + String (version) + " SDK";
    }
};
