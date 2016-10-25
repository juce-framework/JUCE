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

class AndroidProjectExporterBase  : public ProjectExporter
{
public:
    //==============================================================================
    AndroidProjectExporterBase (Project& p, const ValueTree& t)
        : ProjectExporter (p, t),
          androidScreenOrientation (settings, Ids::androidScreenOrientation, nullptr, "unspecified"),
          androidActivityClass (settings, Ids::androidActivityClass, nullptr, createDefaultClassName()),
          androidActivitySubClassName (settings, Ids::androidActivitySubClassName, nullptr),
          androidVersionCode (settings, Ids::androidVersionCode, nullptr, "1"),
          androidMinimumSDK (settings, Ids::androidMinimumSDK, nullptr, "23"),
          androidTheme (settings, Ids::androidTheme, nullptr),
          androidInternetNeeded (settings, Ids::androidInternetNeeded, nullptr, true),
          androidMicNeeded (settings, Ids::microphonePermissionNeeded, nullptr, false),
          androidBluetoothNeeded (settings, Ids::androidBluetoothNeeded, nullptr, true),
          androidOtherPermissions (settings, Ids::androidOtherPermissions, nullptr),
          androidKeyStore (settings, Ids::androidKeyStore, nullptr, "${user.home}/.android/debug.keystore"),
          androidKeyStorePass (settings, Ids::androidKeyStorePass, nullptr, "android"),
          androidKeyAlias (settings, Ids::androidKeyAlias, nullptr, "androiddebugkey"),
          androidKeyAliasPass (settings, Ids::androidKeyAliasPass, nullptr, "android")
    {
        initialiseDependencyPathValues();
    }

    //==============================================================================
    bool isXcode() const override                { return false; }
    bool isVisualStudio() const override         { return false; }
    bool isCodeBlocks() const override           { return false; }
    bool isMakefile() const override             { return false; }

    bool isAndroid() const override              { return true; }
    bool isWindows() const override              { return false; }
    bool isLinux() const override                { return false; }
    bool isOSX() const override                  { return false; }
    bool isiOS() const override                  { return false; }

    bool supportsVST() const override            { return false; }
    bool supportsVST3() const override           { return false; }
    bool supportsAAX() const override            { return false; }
    bool supportsRTAS() const override           { return false; }
    bool supportsAU()   const override           { return false; }
    bool supportsAUv3() const override           { return false; }
    bool supportsStandalone() const override     { return false;  }

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        const String package (getActivityClassPackage());
        const String path (package.replaceCharacter ('.', File::separator));
        const File target (getTargetFolder().getChildFile ("src").getChildFile (path));

        copyActivityJavaFiles (modules, target, package);
    }

    //==============================================================================
    void addPlatformSpecificSettingsForProjectType (const ProjectType&) override
    {
        // no-op.
    }

    //==============================================================================
    void createExporterProperties (PropertyListBuilder& props) override
    {
        createBaseExporterProperties (props);
        createToolchainExporterProperties (props);
        createManifestExporterProperties (props);
        createLibraryModuleExporterProperties (props);
        createCodeSigningExporterProperties (props);
        createOtherExporterProperties (props);
    }

    //==============================================================================
    enum ScreenOrientation
    {
        unspecified = 1,
        portrait    = 2,
        landscape   = 3
    };

    //==============================================================================
    CachedValue<String> androidScreenOrientation, androidActivityClass, androidActivitySubClassName,
                        androidVersionCode, androidMinimumSDK, androidTheme;

    CachedValue<bool>   androidInternetNeeded, androidMicNeeded, androidBluetoothNeeded;
    CachedValue<String> androidOtherPermissions;

    CachedValue<String> androidKeyStore, androidKeyStorePass, androidKeyAlias, androidKeyAliasPass;

    //==============================================================================
    void createBaseExporterProperties (PropertyListBuilder& props)
    {
        static const char* orientations[] = { "Portrait and Landscape", "Portrait", "Landscape", nullptr };
        static const char* orientationValues[] = { "unspecified", "portrait", "landscape", nullptr };

        props.add (new ChoicePropertyComponent (androidScreenOrientation.getPropertyAsValue(), "Screen orientation", StringArray (orientations), Array<var> (orientationValues)),
                   "The screen orientations that this app should support");

        props.add (new TextWithDefaultPropertyComponent<String> (androidActivityClass, "Android Activity class name", 256),
                   "The full java class name to use for the app's Activity class.");

        props.add (new TextPropertyComponent (androidActivitySubClassName.getPropertyAsValue(), "Android Activity sub-class name", 256, false),
                   "If not empty, specifies the Android Activity class name stored in the app's manifest. "
                   "Use this if you would like to use your own Android Activity sub-class.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidVersionCode, "Android Version Code", 32),
                   "An integer value that represents the version of the application code, relative to other versions.");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), sdkPath, "Android SDK Path"),
                   "The path to the Android SDK folder on the target build machine");

        props.add (new DependencyPathPropertyComponent (project.getFile().getParentDirectory(), ndkPath, "Android NDK Path"),
                   "The path to the Android NDK folder on the target build machine");

        props.add (new TextWithDefaultPropertyComponent<String> (androidMinimumSDK, "Minimum SDK version", 32),
                   "The number of the minimum version of the Android SDK that the app requires");
    }

    //==============================================================================
    virtual void createToolchainExporterProperties (PropertyListBuilder& props) = 0;  // different for ant and Android Studio

    //==============================================================================
    void createManifestExporterProperties (PropertyListBuilder& props)
    {
        props.add (new BooleanPropertyComponent (androidInternetNeeded.getPropertyAsValue(), "Internet Access", "Specify internet access permission in the manifest"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidMicNeeded.getPropertyAsValue(), "Audio Input Required", "Specify audio record permission in the manifest"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new BooleanPropertyComponent (androidBluetoothNeeded.getPropertyAsValue(), "Bluetooth permissions Required", "Specify bluetooth permission (required for Bluetooth MIDI)"),
                   "If enabled, this will set the android.permission.BLUETOOTH and  android.permission.BLUETOOTH_ADMIN flag in the manifest. This is required for Bluetooth MIDI on Android.");

        props.add (new TextPropertyComponent (androidOtherPermissions.getPropertyAsValue(), "Custom permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");
    }

    //==============================================================================
    virtual void createLibraryModuleExporterProperties (PropertyListBuilder& props) = 0;   // different for ant and Android Studio

    //==============================================================================
    void createCodeSigningExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyStore, "Key Signing: key.store", 2048),
                   "The key.store value, used when signing the package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyStorePass, "Key Signing: key.store.password", 2048),
                   "The key.store password, used when signing the package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyAlias, "Key Signing: key.alias", 2048),
                   "The key.alias value, used when signing the package.");

        props.add (new TextWithDefaultPropertyComponent<String> (androidKeyAliasPass, "Key Signing: key.alias.password", 2048),
                   "The key.alias password, used when signing the package.");
    }

    //==============================================================================
    void createOtherExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (androidTheme.getPropertyAsValue(), "Android Theme", 256, false),
                   "E.g. @android:style/Theme.NoTitleBar or leave blank for default");
    }

    //==============================================================================
    String createDefaultClassName() const
    {
        String s (project.getBundleIdentifier().toString().toLowerCase());

        if (s.length() > 5
            && s.containsChar ('.')
            && s.containsOnly ("abcdefghijklmnopqrstuvwxyz_.")
            && ! s.startsWithChar ('.'))
        {
            if (! s.endsWithChar ('.'))
                s << ".";
        }
        else
        {
            s = "com.yourcompany.";
        }

        return s + CodeHelpers::makeValidIdentifier (project.getProjectFilenameRoot(), false, true, false);
    }

    void initialiseDependencyPathValues()
    {
        sdkPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::androidSDKPath),
                                                               Ids::androidSDKPath, TargetOS::getThisOS())));

        ndkPath.referTo (Value (new DependencyPathValueSource (getSetting (Ids::androidNDKPath),
                                                               Ids::androidNDKPath, TargetOS::getThisOS())));
    }

    void copyActivityJavaFiles (const OwnedArray<LibraryModule>& modules, const File& targetFolder, const String& package) const
    {
        const String className (getActivityName());

        if (className.isEmpty())
            throw SaveError ("Invalid Android Activity class name: " + androidActivityClass.get());

        createDirectoryOrThrow (targetFolder);

        LibraryModule* const coreModule = getCoreModule (modules);

        if (coreModule != nullptr)
        {
            File javaDestFile (targetFolder.getChildFile (className + ".java"));

            File javaSourceFolder (coreModule->getFolder().getChildFile ("native")
                                                          .getChildFile ("java"));

            String juceMidiCode, juceMidiImports, juceRuntimePermissionsCode;

            juceMidiImports << newLine;

            if (androidMinimumSDK.get().getIntValue() >= 23)
            {
                File javaAndroidMidi (javaSourceFolder.getChildFile ("AndroidMidi.java"));
                File javaRuntimePermissions (javaSourceFolder.getChildFile ("AndroidRuntimePermissions.java"));

                juceMidiImports << "import android.media.midi.*;" << newLine
                                << "import android.bluetooth.*;" << newLine
                                << "import android.bluetooth.le.*;" << newLine;

                juceMidiCode = javaAndroidMidi.loadFileAsString().replace ("JuceAppActivity", className);

                juceRuntimePermissionsCode = javaRuntimePermissions.loadFileAsString().replace ("JuceAppActivity", className);
            }
            else
            {
                juceMidiCode = javaSourceFolder.getChildFile ("AndroidMidiFallback.java")
                                   .loadFileAsString()
                                   .replace ("JuceAppActivity", className);
            }

            File javaSourceFile (javaSourceFolder.getChildFile ("JuceAppActivity.java"));
            StringArray javaSourceLines (StringArray::fromLines (javaSourceFile.loadFileAsString()));

            {
                MemoryOutputStream newFile;

                for (int i = 0; i < javaSourceLines.size(); ++i)
                {
                    const String& line = javaSourceLines[i];

                    if (line.contains ("$$JuceAndroidMidiImports$$"))
                        newFile << juceMidiImports;
                    else if (line.contains ("$$JuceAndroidMidiCode$$"))
                        newFile << juceMidiCode;
                    else if (line.contains ("$$JuceAndroidRuntimePermissionsCode$$"))
                        newFile << juceRuntimePermissionsCode;
                    else
                        newFile << line.replace ("JuceAppActivity", className)
                                       .replace ("package com.juce;", "package " + package + ";") << newLine;
                }

                javaSourceLines = StringArray::fromLines (newFile.toString());
            }

            while (javaSourceLines.size() > 2
                    && javaSourceLines[javaSourceLines.size() - 1].trim().isEmpty()
                    && javaSourceLines[javaSourceLines.size() - 2].trim().isEmpty())
                javaSourceLines.remove (javaSourceLines.size() - 1);

            overwriteFileIfDifferentOrThrow (javaDestFile, javaSourceLines.joinIntoString (newLine));
        }
    }

    String getActivityName() const
    {
        return androidActivityClass.get().fromLastOccurrenceOf (".", false, false);
    }

    String getActivitySubClassName() const
    {
        String activityPath = androidActivitySubClassName.get();

        return (activityPath.isEmpty()) ? getActivityName() : activityPath.fromLastOccurrenceOf (".", false, false);
    }

    String getActivityClassPackage() const
    {
        return androidActivityClass.get().upToLastOccurrenceOf (".", false, false);
    }

    String getJNIActivityClassName() const
    {
        return androidActivityClass.get().replaceCharacter ('.', '/');
    }

    static LibraryModule* getCoreModule (const OwnedArray<LibraryModule>& modules)
    {
        for (int i = modules.size(); --i >= 0;)
            if (modules.getUnchecked(i)->getID() == "juce_core")
                return modules.getUnchecked(i);

        return nullptr;
    }

    StringArray getPermissionsRequired() const
    {
        StringArray s;
        s.addTokens (androidOtherPermissions.get(), ", ", "");

        if (androidInternetNeeded.get())
            s.add ("android.permission.INTERNET");

        if (androidMicNeeded.get())
            s.add ("android.permission.RECORD_AUDIO");

        if (androidBluetoothNeeded.get())
        {
            s.add ("android.permission.BLUETOOTH");
            s.add ("android.permission.BLUETOOTH_ADMIN");
            s.add ("android.permission.ACCESS_COARSE_LOCATION");
        }

        return getCleanedStringArray (s);
    }

    template <typename PredicateT>
    void findAllProjectItemsWithPredicate (const Project::Item& projectItem, Array<RelativePath>& results, const PredicateT& predicate) const
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                findAllProjectItemsWithPredicate (projectItem.getChild(i), results, predicate);
        }
        else
        {
            if (predicate (projectItem))
                results.add (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder));
        }
    }

    void writeIcon (const File& file, const Image& im) const
    {
        if (im.isValid())
        {
            createDirectoryOrThrow (file.getParentDirectory());

            PNGImageFormat png;
            MemoryOutputStream mo;

            if (! png.writeImageToStream (im, mo))
                throw SaveError ("Can't generate Android icon file");

            overwriteFileIfDifferentOrThrow (file, mo);
        }
    }

    void writeIcons (const File& folder) const
    {
        ScopedPointer<Drawable> bigIcon (getBigIcon());
        ScopedPointer<Drawable> smallIcon (getSmallIcon());

        if (bigIcon != nullptr && smallIcon != nullptr)
        {
            const int step = jmax (bigIcon->getWidth(), bigIcon->getHeight()) / 8;
            writeIcon (folder.getChildFile ("drawable-xhdpi/icon.png"), getBestIconForSize (step * 8, false));
            writeIcon (folder.getChildFile ("drawable-hdpi/icon.png"),  getBestIconForSize (step * 6, false));
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"),  getBestIconForSize (step * 4, false));
            writeIcon (folder.getChildFile ("drawable-ldpi/icon.png"),  getBestIconForSize (step * 3, false));
        }
        else if (Drawable* icon = bigIcon != nullptr ? bigIcon : smallIcon)
        {
            writeIcon (folder.getChildFile ("drawable-mdpi/icon.png"), rescaleImageForIcon (*icon, icon->getWidth()));
        }
    }

    template <typename BuildConfigType>
    String getABIs (bool forDebug) const
    {
        for (ConstConfigIterator config (*this); config.next();)
        {
            const BuildConfigType& androidConfig = dynamic_cast<const BuildConfigType&> (*config);

            if (config->isDebug() == forDebug)
                return androidConfig.getArchitectures();
        }

        return String();
    }

    //==============================================================================
    XmlElement* createManifestXML() const
    {
        XmlElement* manifest = new XmlElement ("manifest");

        manifest->setAttribute ("xmlns:android", "http://schemas.android.com/apk/res/android");
        manifest->setAttribute ("android:versionCode", androidVersionCode.get());
        manifest->setAttribute ("android:versionName",  project.getVersionString());
        manifest->setAttribute ("package", getActivityClassPackage());

        XmlElement* screens = manifest->createNewChildElement ("supports-screens");
        screens->setAttribute ("android:smallScreens", "true");
        screens->setAttribute ("android:normalScreens", "true");
        screens->setAttribute ("android:largeScreens", "true");
        //screens->setAttribute ("android:xlargeScreens", "true");
        screens->setAttribute ("android:anyDensity", "true");

        XmlElement* sdk = manifest->createNewChildElement ("uses-sdk");
        sdk->setAttribute ("android:minSdkVersion", androidMinimumSDK.get());
        sdk->setAttribute ("android:targetSdkVersion", androidMinimumSDK.get());

        {
            const StringArray permissions (getPermissionsRequired());

            for (int i = permissions.size(); --i >= 0;)
                manifest->createNewChildElement ("uses-permission")->setAttribute ("android:name", permissions[i]);
        }

        if (project.getModules().isModuleEnabled ("juce_opengl"))
        {
            XmlElement* feature = manifest->createNewChildElement ("uses-feature");
            feature->setAttribute ("android:glEsVersion", "0x00020000");
            feature->setAttribute ("android:required", "true");
        }

        XmlElement* app = manifest->createNewChildElement ("application");
        app->setAttribute ("android:label", "@string/app_name");

        if (androidTheme.get().isNotEmpty())
            app->setAttribute ("android:theme", androidTheme.get());

        {
            ScopedPointer<Drawable> bigIcon (getBigIcon()), smallIcon (getSmallIcon());

            if (bigIcon != nullptr || smallIcon != nullptr)
                app->setAttribute ("android:icon", "@drawable/icon");
        }

        if (androidMinimumSDK.get().getIntValue() >= 11)
            app->setAttribute ("android:hardwareAccelerated", "false"); // (using the 2D acceleration slows down openGL)

        XmlElement* act = app->createNewChildElement ("activity");
        act->setAttribute ("android:name", getActivitySubClassName());
        act->setAttribute ("android:label", "@string/app_name");
        act->setAttribute ("android:configChanges", "keyboardHidden|orientation|screenSize");
        act->setAttribute ("android:screenOrientation", androidScreenOrientation.get());

        XmlElement* intent = act->createNewChildElement ("intent-filter");
        intent->createNewChildElement ("action")->setAttribute ("android:name", "android.intent.action.MAIN");
        intent->createNewChildElement ("category")->setAttribute ("android:name", "android.intent.category.LAUNCHER");

        return manifest;
    }

    //==============================================================================
    Value sdkPath, ndkPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidProjectExporterBase)
};
