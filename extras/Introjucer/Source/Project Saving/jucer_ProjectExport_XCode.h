/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
    const int currentSDKVersion = 10;

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
    static const char* getNameMac()                         { return "Xcode (MacOSX)"; }
    static const char* getNameiOS()                         { return "Xcode (iOS)"; }
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

        // Deleting the .rsrc files can be needed to force Xcode to update the version number.
        deleteRsrcFiles();
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
        Value  getCppLanguageStandardValue()           { return getValue (Ids::cppLanguageStandard); }
        String getCppLanguageStandard() const          { return config   [Ids::cppLanguageStandard]; }
        Value  getCppLibTypeValue()                    { return getValue (Ids::cppLibType); }
        String getCppLibType() const                   { return config   [Ids::cppLibType]; }
        Value  getCodeSignIdentityValue()              { return getValue (Ids::codeSigningIdentity); }
        String getCodeSignIdentity() const             { return config   [Ids::codeSigningIdentity]; }
        Value  getFastMathValue()                      { return getValue (Ids::fastMath); }
        bool   isFastMathEnabled() const               { return config   [Ids::fastMath]; }
        Value  getLinkTimeOptimisationValue()          { return getValue (Ids::linkTimeOptimisation); }
        bool   isLinkTimeOptimisationEnabled() const   { return config   [Ids::linkTimeOptimisation]; }

        var getDefaultOptimisationLevel() const override    { return var ((int) (isDebug() ? gccO0 : gccO3)); }

        void createConfigProperties (PropertyListBuilder& props)
        {
            addGCCOptimisationProperty (props);

            if (iOS)
            {
                const char* iosVersions[]      = { "Use Default",     "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", "6.0", "6.1", "7.0", "7.1", 0 };
                const char* iosVersionValues[] = { osxVersionDefault, "3.2", "4.0", "4.1", "4.2", "4.3", "5.0", "5.1", "6.0", "6.1", "7.0", "7.1", 0 };

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

            const char* cppLanguageStandardNames[] = { "Use Default", "C++98", "GNU++98", "C++11", "GNU++11", "C++14", "GNU++14", nullptr };
            Array<var> cppLanguageStandardValues;
            cppLanguageStandardValues.add (var::null);
            cppLanguageStandardValues.add ("c++98");
            cppLanguageStandardValues.add ("gnu++98");
            cppLanguageStandardValues.add ("c++11");
            cppLanguageStandardValues.add ("gnu++11");
            cppLanguageStandardValues.add ("c++14");
            cppLanguageStandardValues.add ("gnu++14");

            props.add (new ChoicePropertyComponent (getCppLanguageStandardValue(), "C++ Language Standard", StringArray (cppLanguageStandardNames), cppLanguageStandardValues),
                       "The standard of the C++ language that will be used for compilation.");

            const char* cppLibNames[] = { "Use Default", "LLVM libc++", "GNU libstdc++", nullptr };
            Array<var> cppLibValues;
            cppLibValues.add (var::null);
            cppLibValues.add ("libc++");
            cppLibValues.add ("libstdc++");

            props.add (new ChoicePropertyComponent (getCppLibTypeValue(), "C++ Library", StringArray (cppLibNames), cppLibValues),
                       "The type of C++ std lib that will be linked.");

            props.add (new TextPropertyComponent (getCodeSignIdentityValue(), "Code-signing Identity", 8192, false),
                       "The name of a code-signing identity for Xcode to apply.");

            props.add (new BooleanPropertyComponent (getFastMathValue(), "Relax IEEE compliance", "Enabled"),
                       "Enable this to use FAST_MATH non-IEEE mode. (Warning: this can have unexpected results!)");

            props.add (new BooleanPropertyComponent (getLinkTimeOptimisationValue(), "Link-Time Optimisation", "Enabled"),
                       "Enable this to perform link-time code generation. This is recommended for release builds.");
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

    static String addQuotesIfContainsSpace (const String& s)
    {
        return s.containsChar (' ') ? s.quoted() : s;
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

        if (iOS)
        {
            if (! projectType.isStaticLibrary())
                createiOSAssetsFolder();
        }
        else
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

    static Image fixMacIconImageSize (Drawable& image)
    {
        const int validSizes[] = { 16, 32, 48, 128, 256, 512, 1024 };

        const int w = image.getWidth();
        const int h = image.getHeight();

        int bestSize = 16;

        for (int i = 0; i < numElementsInArray (validSizes); ++i)
        {
            if (w == h && w == validSizes[i])
            {
                bestSize = w;
                break;
            }

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

    void writeIcnsFile (const OwnedArray<Drawable>& images, OutputStream& out) const
    {
        MemoryOutputStream data;
        int smallest = 0x7fffffff;
        Drawable* smallestImage = nullptr;

        for (int i = 0; i < images.size(); ++i)
        {
            const Image image (fixMacIconImageSize (*images.getUnchecked(i)));
            jassert (image.getWidth() == image.getHeight());

            if (image.getWidth() < smallest)
            {
                smallest = image.getWidth();
                smallestImage = images.getUnchecked(i);
            }

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

        // If you only supply a 1024 image, the file doesn't work on 10.8, so we need
        // to force a smaller one in there too..
        if (smallest > 512 && smallestImage != nullptr)
            writeNewIconFormat (data, rescaleImageForIcon (*smallestImage, 512), "ic09");

        out.write ("icns", 4);
        out.writeIntBigEndian ((int) data.getDataSize() + 8);
        out << data;
    }

    void getIconImages (OwnedArray<Drawable>& images) const
    {
        ScopedPointer<Drawable> bigIcon (getBigIcon());
        if (bigIcon != nullptr)
            images.add (bigIcon.release());

        ScopedPointer<Drawable> smallIcon (getSmallIcon());
        if (smallIcon != nullptr)
            images.add (smallIcon.release());
    }

    void createiOSIconFiles (File appIconSet) const
    {
        const Array<AppIconType> types (getiOSAppIconTypes());

        OwnedArray<Drawable> images;
        getIconImages (images);

        if (images.size() > 0)
        {
            for (int i = 0; i < types.size(); ++i)
            {
                const AppIconType type = types.getUnchecked(i);
                const Image image (rescaleImageForIcon (*images.getFirst(), type.size));

                MemoryOutputStream pngData;
                PNGImageFormat pngFormat;
                pngFormat.writeImageToStream (image, pngData);

                overwriteFileIfDifferentOrThrow (appIconSet.getChildFile (type.filename), pngData);
            }
        }
    }

    void createIconFile() const
    {
        OwnedArray<Drawable> images;
        getIconImages (images);

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
        {
            addPlistDictionaryKeyBool (dict, "LSRequiresIPhoneOS", true);
            addPlistDictionaryKeyBool (dict, "UIViewControllerBasedStatusBarAppearance", false);
        }

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

        if (iOS)
        {
            static const char* kDefaultiOSOrientationStrings[] =
            {
                "UIInterfaceOrientationPortrait",
                "UIInterfaceOrientationPortraitUpsideDown",
                "UIInterfaceOrientationLandscapeLeft",
                "UIInterfaceOrientationLandscapeRight",
                nullptr
            };

            StringArray iOSOrientations (kDefaultiOSOrientationStrings);

            dict->createNewChildElement ("key")->addTextElement ("UISupportedInterfaceOrientations");
            XmlElement* plistStringArray = dict->createNewChildElement ("array");

            for (int i = 0; i < iOSOrientations.size(); ++i)
                plistStringArray->createNewChildElement ("string")->addTextElement (iOSOrientations[i]);
        }

        for (int i = 0; i < xcodeExtraPListEntries.size(); ++i)
            dict->addChildElement (new XmlElement (xcodeExtraPListEntries.getReference(i)));

        MemoryOutputStream mo;
        plist->writeToStream (mo, "<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">");

        overwriteFileIfDifferentOrThrow (infoPlistFile, mo);
    }

    void deleteRsrcFiles() const
    {
        for (DirectoryIterator di (getTargetFolder().getChildFile ("build"), true, "*.rsrc", File::findFiles); di.next();)
            di.getFile().deleteFile();
    }

    String getHeaderSearchPaths (const BuildConfiguration& config) const
    {
        StringArray paths (extraSearchPaths);
        paths.addArray (config.getHeaderSearchPaths());
        paths.add ("$(inherited)");

        paths = getCleanedStringArray (paths);

        for (int i = 0; i < paths.size(); ++i)
        {
            String& s = paths.getReference(i);

            s = replacePreprocessorTokens (config, s);

            if (s.containsChar (' '))
                s = "\"\\\"" + s + "\\\"\""; // crazy double quotes required when there are spaces..
            else
                s = "\"" + s + "\"";
        }

        return "(" + paths.joinIntoString (", ") + ")";
    }

    static String getLinkerFlagForLib (String library)
    {
        if (library.substring (0, 3) == "lib")
            library = library.substring (3);

        return "-l" + library.upToLastOccurrenceOf (".", false, false);
    }

    void getLinkerFlagsForStaticLibrary (const RelativePath& library, StringArray& flags, StringArray& librarySearchPaths) const
    {
        flags.add (getLinkerFlagForLib (library.getFileNameWithoutExtension()));

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

        for (int i = 0; i < xcodeLibs.size(); ++i)
            flags.add (getLinkerFlagForLib (xcodeLibs[i]));

        flags = getCleanedStringArray (flags);
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

        if (config.isDebug())
             if (config.getMacArchitecture() == osxArch_Default || config.getMacArchitecture().isEmpty())
                 s.add ("ONLY_ACTIVE_ARCH = YES");

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

        s.add ("PRODUCT_NAME = \"" + replacePreprocessorTokens (config, config.getTargetBinaryNameString()) + "\"");
        return s;
    }

    StringArray getTargetSettings (const XcodeBuildConfiguration& config) const
    {
        StringArray s;

        const String arch (config.getMacArchitecture());
        if (arch == osxArch_Native)                s.add ("ARCHS = \"$(NATIVE_ARCH_ACTUAL)\"");
        else if (arch == osxArch_32BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_BIT)\"");
        else if (arch == osxArch_64BitUniversal)   s.add ("ARCHS = \"$(ARCHS_STANDARD_32_64_BIT)\"");
        else if (arch == osxArch_64Bit)            s.add ("ARCHS = \"$(ARCHS_STANDARD_64_BIT)\"");

        s.add ("HEADER_SEARCH_PATHS = " + getHeaderSearchPaths (config));
        s.add ("GCC_OPTIMIZATION_LEVEL = " + config.getGCCOptimisationFlag());
        s.add ("INFOPLIST_FILE = " + infoPlistFile.getFileName());

        if (config.isLinkTimeOptimisationEnabled())
            s.add ("LLVM_LTO = YES");

        if (config.isFastMathEnabled())
            s.add ("GCC_FAST_MATH = YES");

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

            s.add ("DSTROOT = " + addQuotesIfContainsSpace (sanitisePath (binaryPath.toUnixStyle())));
            s.add ("SYMROOT = " + addQuotesIfContainsSpace (sanitisePath (binaryPath.toUnixStyle())));
        }
        else
        {
            s.add ("CONFIGURATION_BUILD_DIR = \"$(PROJECT_DIR)/build/$(CONFIGURATION)\"");
        }

        String gccVersion ("com.apple.compilers.llvm.clang.1_0");

        if (iOS)
        {
            s.add ("ASSETCATALOG_COMPILER_APPICON_NAME = AppIcon");
            s.add ("ASSETCATALOG_COMPILER_LAUNCHIMAGE_NAME = LaunchImage");
        }
        else
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

        if (config.getCodeSignIdentity().isNotEmpty())
            s.add ("CODE_SIGN_IDENTITY = " + config.getCodeSignIdentity().quoted());

        if (config.getCppLanguageStandard().isNotEmpty())
            s.add ("CLANG_CXX_LANGUAGE_STANDARD = " + config.getCppLanguageStandard().quoted());

        if (config.getCppLibType().isNotEmpty())
            s.add ("CLANG_CXX_LIBRARY = " + config.getCppLibType().quoted());

        s.add ("COMBINE_HIDPI_IMAGES = YES");

        {
            StringArray linkerFlags, librarySearchPaths;
            getLinkerFlags (config, linkerFlags, librarySearchPaths);

            if (linkerFlags.size() > 0)
                s.add ("OTHER_LDFLAGS = \"" + linkerFlags.joinIntoString (" ") + "\"");

            librarySearchPaths.addArray (config.getLibrarySearchPaths());
            librarySearchPaths = getCleanedStringArray (librarySearchPaths);

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

            s.add ("GCC_PREPROCESSOR_DEFINITIONS = " + indentParenthesisedList (defsList));
        }

        s.addTokens (config.getCustomXcodeFlags(), ",", "\"'");

        return getCleanedStringArray (s);
    }

    void addFrameworks() const
    {
        if (! projectType.isStaticLibrary())
        {
            StringArray s (xcodeFrameworks);
            s.addTokens (getExtraFrameworksString(), ",;", "\"'");

            if (project.getConfigFlag ("JUCE_QUICKTIME") == Project::configFlagDisabled)
                s.removeString ("QuickTime");

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
            output << "\t\t" << o.getType().toString() << " = {";

            for (int j = 0; j < o.getNumProperties(); ++j)
            {
                const Identifier propertyName (o.getPropertyName(j));
                String val (o.getProperty (propertyName).toString());

                if (val.isEmpty() || (val.containsAnyOf (" \t;<>()=,&+-_@~\r\n\\#%^`*")
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
        if (file.hasFileExtension (cppFileExtensions))      return "sourcecode.cpp.cpp";
        if (file.hasFileExtension (".mm"))                  return "sourcecode.cpp.objcpp";
        if (file.hasFileExtension (".m"))                   return "sourcecode.c.objc";
        if (file.hasFileExtension (".c"))                   return "sourcecode.c.c";
        if (file.hasFileExtension (headerFileExtensions))   return "sourcecode.c.h";
        if (file.hasFileExtension (asmFileExtensions))      return "sourcecode.c.asm";
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
        if (file.hasFileExtension ("xcassets"))             return "folder.assetcatalog";

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
        String path (frameworkName);
        if (! File::isAbsolutePath (path))
            path = "System/Library/Frameworks/" + path;

        if (! path.endsWithIgnoreCase (".framework"))
            path << ".framework";

        const String fileRefID (createFileRefID (path));

        addFileReference ((File::isAbsolutePath (frameworkName) ? "" : "${SDKROOT}/") + path);
        frameworkIDs.add (addBuildFile (path, fileRefID, false, false));
        frameworkFileIDs.add (fileRefID);
    }

    void addGroup (const String& groupID, const String& groupName, const StringArray& childIDs) const
    {
        ValueTree* v = new ValueTree (groupID);
        v->setProperty ("isa", "PBXGroup", nullptr);
        v->setProperty ("children", indentParenthesisedList (childIDs), nullptr);
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
        ProjectExporter::BuildConfiguration::Ptr config = getConfiguration(0);
        jassert (config != nullptr);
        String productName (replacePreprocessorTokens (*config, config->getTargetBinaryNameString()));

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
        v->setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
        v->setProperty (Ids::name, configName, nullptr);
        targetConfigs.add (v);
    }

    void addProjectConfig (const String& configName, const StringArray& buildSettings) const
    {
        ValueTree* v = new ValueTree (createID ("projectconfigid_" + configName));
        v->setProperty ("isa", "XCBuildConfiguration", nullptr);
        v->setProperty ("buildSettings", indentBracedList (buildSettings), nullptr);
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
        v->setProperty ("buildConfigurations", indentParenthesisedList (configIDs), nullptr);
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
        v->setProperty ("files", indentParenthesisedList (fileIds), nullptr);
        v->setProperty ("runOnlyForDeploymentPostprocessing", (int) 0, nullptr);
        misc.add (v);
        return *v;
    }

    void addTargetObject() const
    {
        ValueTree* const v = new ValueTree (createID ("__target"));
        v->setProperty ("isa", "PBXNativeTarget", nullptr);
        v->setProperty ("buildConfigurationList", createID ("__configList"), nullptr);
        v->setProperty ("buildPhases", indentParenthesisedList (buildPhaseIDs), nullptr);
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

    String getiOSAssetContents (var images) const
    {
        DynamicObject::Ptr v (new DynamicObject());

        var info (new DynamicObject());
        info.getDynamicObject()->setProperty ("version", 1);
        info.getDynamicObject()->setProperty ("author", "xcode");

        v->setProperty ("images", images);
        v->setProperty ("info", info);

        return JSON::toString (var (v));
    }

    struct AppIconType
    {
        const char* idiom;
        const char* sizeString;
        const char* filename;
        const char* scale;
        int size;
    };

    static Array<AppIconType> getiOSAppIconTypes()
    {
        AppIconType types[] =
        {
            { "iphone", "29x29",   "Icon-Small.png",             "1x", 29  },
            { "iphone", "29x29",   "Icon-Small@2x.png",          "2x", 58  },
            { "iphone", "40x40",   "Icon-Spotlight-40@2x.png",   "2x", 80  },
            { "iphone", "57x57",   "Icon.png",                   "1x", 57  },
            { "iphone", "57x57",   "Icon@2x.png",                "2x", 114 },
            { "iphone", "60x60",   "Icon-60@2x.png",             "2x", 120 },
            { "iphone", "60x60",   "Icon-@3x.png",               "3x", 180 },
            { "ipad",   "29x29",   "Icon-Small-1.png",           "1x", 29  },
            { "ipad",   "29x29",   "Icon-Small@2x-1.png",        "2x", 58  },
            { "ipad",   "40x40",   "Icon-Spotlight-40.png",      "1x", 40  },
            { "ipad",   "40x40",   "Icon-Spotlight-40@2x-1.png", "2x", 80  },
            { "ipad",   "50x50",   "Icon-Small-50.png",          "1x", 50  },
            { "ipad",   "50x50",   "Icon-Small-50@2x.png",       "2x", 100 },
            { "ipad",   "72x72",   "Icon-72.png",                "1x", 72  },
            { "ipad",   "72x72",   "Icon-72@2x.png",             "2x", 144 },
            { "ipad",   "76x76",   "Icon-76.png",                "1x", 76  },
            { "ipad",   "76x76",   "Icon-76@2x.png",             "2x", 152 }
        };

        return Array<AppIconType> (types, numElementsInArray (types));
    }

    String getiOSAppIconContents() const
    {
        const Array<AppIconType> types = getiOSAppIconTypes();

        var images;

        for (int i = 0; i < types.size(); ++i)
        {
            AppIconType type = types.getUnchecked(i);

            DynamicObject::Ptr d = new DynamicObject();
            d->setProperty ("idiom",    type.idiom);
            d->setProperty ("size",     type.sizeString);
            d->setProperty ("filename", type.filename);
            d->setProperty ("scale",    type.scale);
            images.append (var (d));
        }

        return getiOSAssetContents (images);
    }

    String getiOSLaunchImageContents() const
    {
        struct ImageType
        {
            const char* orientation;
            const char* idiom;
            const char* extent;
            const char* scale;
        };

        const ImageType types[] = { { "portrait",  "iphone", "full-screen", "2x" },
                                    { "landscape", "iphone", "full-screen", "2x" },
                                    { "portrait",  "ipad",   "full-screen", "1x" },
                                    { "landscape", "ipad",   "full-screen", "1x" },
                                    { "portrait",  "ipad",   "full-screen", "2x" },
                                    { "landscape", "ipad",   "full-screen", "2x" } };
        var images;

        for (size_t i = 0; i < sizeof (types) / sizeof (types[0]); ++i)
        {
            DynamicObject::Ptr d = new DynamicObject();
            d->setProperty ("orientation", types[i].orientation);
            d->setProperty ("idiom", types[i].idiom);
            d->setProperty ("extent",  types[i].extent);
            d->setProperty ("minimum-system-version", "7.0");
            d->setProperty ("scale", types[i].scale);
            images.append (var (d));
        }

        return getiOSAssetContents (images);
    }

    void createiOSAssetsFolder() const
    {
        File assets (getTargetFolder().getChildFile (project.getProjectFilenameRoot()).getChildFile ("Images.xcassets"));

        overwriteFileIfDifferentOrThrow (assets.getChildFile ("AppIcon.appiconset").getChildFile ("Contents.json"), getiOSAppIconContents());
        createiOSIconFiles (assets.getChildFile ("AppIcon.appiconset"));

        overwriteFileIfDifferentOrThrow (assets.getChildFile ("LaunchImage.launchimage").getChildFile ("Contents.json"), getiOSLaunchImageContents());

        RelativePath assetsPath (assets, getTargetFolder(), RelativePath::buildTargetFolder);
        addFileReference (assetsPath.toUnixStyle());
        resourceIDs.add (addBuildFile (assetsPath, false, false));
        resourceFileRefs.add (createFileRefID (assetsPath));
    }

    //==============================================================================
    static String indentBracedList (const StringArray& list)        { return "{" + indentList (list, ";", 0, true) + " }"; }
    static String indentParenthesisedList (const StringArray& list) { return "(" + indentList (list, ",", 1, false) + " )"; }

    static String indentList (const StringArray& list, const String& separator, int extraTabs, bool shouldSort)
    {
        if (list.size() == 0)
            return " ";

        const String tabs ("\n" + String::repeatedString ("\t", extraTabs + 4));

        if (shouldSort)
        {
            StringArray sorted (list);
            sorted.sort (true);

            return tabs + sorted.joinIntoString (separator + tabs) + separator;
        }

        return tabs + list.joinIntoString (separator + tabs) + separator;
    }

    String createID (String rootString) const
    {
        if (rootString.startsWith ("${"))
            rootString = rootString.fromFirstOccurrenceOf ("}/", false, false);

        rootString += project.getProjectUID();

        return MD5 (rootString.toUTF8()).toHexString().substring (0, 24).toUpperCase();
    }

    String createFileRefID (const RelativePath& path) const     { return createFileRefID (path.toUnixStyle()); }
    String createFileRefID (const String& path) const           { return createID ("__fileref_" + path); }
    String getIDForGroup (const Project::Item& item) const      { return createID (item.getID()); }

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
