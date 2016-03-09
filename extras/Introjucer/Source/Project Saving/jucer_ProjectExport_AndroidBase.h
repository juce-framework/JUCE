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
    //==========================================================================
    AndroidProjectExporterBase (Project& p, const ValueTree& t)
        : ProjectExporter (p, t)
    {
        setEmptyPropertiesToDefaultValues();
    }

    //==========================================================================
    virtual bool isAndroidStudio() = 0;
    virtual bool isAndroidAnt() = 0;

    //==========================================================================
    void setEmptyPropertiesToDefaultValues()
    {
        if (getVersionCodeString().isEmpty())
            getVersionCodeValue() = 1;

        if (getActivityClassPath().isEmpty())
            getActivityClassPathValue() = createDefaultClassName();

        if (getMinimumSDKVersionString().isEmpty())
            getMinimumSDKVersionValue() = 23;

        if (getInternetNeededValue().toString().isEmpty())
            getInternetNeededValue() = true;

        if (getBluetoothPermissionsValue().toString().isEmpty())
            getBluetoothPermissionsValue() = true;

        if (getKeyStoreValue().getValue().isVoid())         getKeyStoreValue()      = "${user.home}/.android/debug.keystore";
        if (getKeyStorePassValue().getValue().isVoid())     getKeyStorePassValue()  = "android";
        if (getKeyAliasValue().getValue().isVoid())         getKeyAliasValue()      = "androiddebugkey";
        if (getKeyAliasPassValue().getValue().isVoid())     getKeyAliasPassValue()  = "android";

        initialiseDependencyPathValues();

        if (getScreenOrientationValue().toString().isEmpty())
            getScreenOrientationValue() = "unspecified";
    }

    //==========================================================================
    void create (const OwnedArray<LibraryModule>& modules) const override
    {
        const String package (getActivityClassPackage());
        const String path (package.replaceCharacter ('.', File::separator));
        const File target (getTargetFolder().getChildFile ("src").getChildFile (path));

        copyActivityJavaFiles (modules, target, package);
    }

    //==========================================================================
    // base properties

    Value  getScreenOrientationValue()              { return getSetting (Ids::androidScreenOrientation); }
    String getScreenOrientationString() const       { return settings [Ids::androidScreenOrientation]; }
    Value  getActivityClassPathValue()              { return getSetting (Ids::androidActivityClass); }
    String getActivityClassPath() const             { return settings [Ids::androidActivityClass]; }
    Value  getActivitySubClassPathValue()           { return getSetting (Ids::androidActivitySubClassName); }
    String getActivitySubClassPath() const          { return settings [Ids::androidActivitySubClassName]; }
    Value  getVersionCodeValue()                    { return getSetting (Ids::androidVersionCode); }
    String getVersionCodeString() const             { return settings [Ids::androidVersionCode]; }
    Value  getSDKPathValue()                        { return sdkPath; }
    String getSDKPathString() const                 { return sdkPath.toString(); }
    Value  getNDKPathValue()                        { return ndkPath; }
    String getNDKPathString() const                 { return ndkPath.toString(); }
    Value  getMinimumSDKVersionValue()              { return getSetting (Ids::androidMinimumSDK); }
    String getMinimumSDKVersionString() const       { return settings [Ids::androidMinimumSDK]; }

    // manifest properties

    Value  getInternetNeededValue()                 { return getSetting (Ids::androidInternetNeeded); }
    bool   getInternetNeeded() const                { return settings [Ids::androidInternetNeeded]; }
    Value  getAudioRecordNeededValue()              { return getSetting (Ids::androidMicNeeded); }
    bool   getAudioRecordNeeded() const             { return settings [Ids::androidMicNeeded]; }
    Value  getBluetoothPermissionsValue()           { return getSetting(Ids::androidBluetoothNeeded); }
    bool   getBluetoothPermissions() const          { return settings[Ids::androidBluetoothNeeded]; }
    Value  getOtherPermissionsValue()               { return getSetting (Ids::androidOtherPermissions); }
    String getOtherPermissions() const              { return settings [Ids::androidOtherPermissions]; }

    // code signing properties

    Value  getKeyStoreValue()                       { return getSetting (Ids::androidKeyStore); }
    String getKeyStoreString() const                { return settings [Ids::androidKeyStore]; }
    Value  getKeyStorePassValue()                   { return getSetting (Ids::androidKeyStorePass); }
    String getKeyStorePassString() const            { return settings [Ids::androidKeyStorePass]; }
    Value  getKeyAliasValue()                       { return getSetting (Ids::androidKeyAlias); }
    String getKeyAliasString() const                { return settings [Ids::androidKeyAlias]; }
    Value  getKeyAliasPassValue()                   { return getSetting (Ids::androidKeyAliasPass); }
    String getKeyAliasPassString() const            { return settings [Ids::androidKeyAliasPass]; }

    // other properties

    Value  getThemeValue()                          { return getSetting (Ids::androidTheme); }
    String getThemeString() const                   { return settings [Ids::androidTheme]; }

    //==========================================================================
    void createExporterProperties (PropertyListBuilder& props) override
    {
        createBaseExporterProperties (props);
        createToolchainExporterProperties (props);
        createManifestExporterProperties (props);
        createLibraryModuleExporterProperties (props);
        createCodeSigningExporterProperties (props);
        createOtherExporterProperties (props);
    }

    //==========================================================================
    enum ScreenOrientation
    {
        unspecified = 1,
        portrait    = 2,
        landscape   = 3
    };

    //==============================================================================
    void createBaseExporterProperties (PropertyListBuilder& props)
    {
        static const char* orientations[] = { "Portrait and Landscape", "Portrait", "Landscape", nullptr };
        static const char* orientationValues[] = { "unspecified", "portrait", "landscape", nullptr };

        props.add (new ChoicePropertyComponent (getScreenOrientationValue(), "Screen orientation", StringArray (orientations), Array<var> (orientationValues)),
                   "The screen orientations that this app should support");

        props.add (new TextPropertyComponent (getActivityClassPathValue(), "Android Activity class name", 256, false),
                   "The full java class name to use for the app's Activity class.");

        props.add (new TextPropertyComponent (getActivitySubClassPathValue(), "Android Activity sub-class name", 256, false),
                   "If not empty, specifies the Android Activity class name stored in the app's manifest. "
                   "Use this if you would like to use your own Android Activity sub-class.");

        props.add (new TextPropertyComponent (getVersionCodeValue(), "Android Version Code", 32, false),
                   "An integer value that represents the version of the application code, relative to other versions.");

        props.add (new DependencyPathPropertyComponent (getSDKPathValue(), "Android SDK Path"),
                   "The path to the Android SDK folder on the target build machine");

        props.add (new DependencyPathPropertyComponent (getNDKPathValue(), "Android NDK Path"),
                   "The path to the Android NDK folder on the target build machine");

        props.add (new TextPropertyComponent (getMinimumSDKVersionValue(), "Minimum SDK version", 32, false),
                   "The number of the minimum version of the Android SDK that the app requires");
    }

    //==========================================================================
    virtual void createToolchainExporterProperties (PropertyListBuilder& props) = 0;  // different for ant and Android Studio

    //==========================================================================
    void createManifestExporterProperties (PropertyListBuilder& props)
    {
        props.add (new BooleanPropertyComponent (getInternetNeededValue(), "Internet Access", "Specify internet access permission in the manifest"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new BooleanPropertyComponent (getAudioRecordNeededValue(), "Audio Input Required", "Specify audio record permission in the manifest"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new BooleanPropertyComponent (getBluetoothPermissionsValue(), "Bluetooth permissions Required", "Specify bluetooth permission (required for Bluetooth MIDI)"),
                   "If enabled, this will set the android.permission.BLUETOOTH and  android.permission.BLUETOOTH_ADMIN flag in the manifest. This is required for Bluetooth MIDI on Android.");

        props.add (new TextPropertyComponent (getOtherPermissionsValue(), "Custom permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");
    }

    //==========================================================================
    virtual void createLibraryModuleExporterProperties (PropertyListBuilder& props) = 0;   // different for ant and Android Studio

    //==========================================================================
    void createCodeSigningExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (getKeyStoreValue(), "Key Signing: key.store", 2048, false),
                   "The key.store value, used when signing the package.");

        props.add (new TextPropertyComponent (getKeyStorePassValue(), "Key Signing: key.store.password", 2048, false),
                   "The key.store password, used when signing the package.");

        props.add (new TextPropertyComponent (getKeyAliasValue(), "Key Signing: key.alias", 2048, false),
                   "The key.alias value, used when signing the package.");

        props.add (new TextPropertyComponent (getKeyAliasPassValue(), "Key Signing: key.alias.password", 2048, false),
                   "The key.alias password, used when signing the package.");
    }

    //==========================================================================
    void createOtherExporterProperties (PropertyListBuilder& props)
    {
        props.add (new TextPropertyComponent (getThemeValue(), "Android Theme", 256, false),
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
            throw SaveError ("Invalid Android Activity class name: " + getActivityClassPath());

        createDirectoryOrThrow (targetFolder);

        LibraryModule* const coreModule = getCoreModule (modules);

        if (coreModule != nullptr)
        {
            File javaDestFile (targetFolder.getChildFile (className + ".java"));

            File javaSourceFolder (coreModule->getFolder().getChildFile ("native")
                                                          .getChildFile ("java"));

            String juceMidiCode, juceMidiImports, juceRuntimePermissionsCode;

            juceMidiImports << newLine;

            if (getMinimumSDKVersionString().getIntValue() >= 23)
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
        return getActivityClassPath().fromLastOccurrenceOf (".", false, false);
    }

    String getActivitySubClassName() const
    {
        String activityPath = getActivitySubClassPath();

        return (activityPath.isEmpty()) ? getActivityName() : activityPath.fromLastOccurrenceOf (".", false, false);
    }

    String getActivityClassPackage() const
    {
        return getActivityClassPath().upToLastOccurrenceOf (".", false, false);
    }

    String getJNIActivityClassName() const
    {
        return getActivityClassPath().replaceCharacter ('.', '/');
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
        s.addTokens (getOtherPermissions(), ", ", "");

        if (getInternetNeeded())
            s.add ("android.permission.INTERNET");

        if (getAudioRecordNeeded())
            s.add ("android.permission.RECORD_AUDIO");

        if (getBluetoothPermissions())
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
        manifest->setAttribute ("android:versionCode", getVersionCodeString());
        manifest->setAttribute ("android:versionName",  project.getVersionString());
        manifest->setAttribute ("package", getActivityClassPackage());

        XmlElement* screens = manifest->createNewChildElement ("supports-screens");
        screens->setAttribute ("android:smallScreens", "true");
        screens->setAttribute ("android:normalScreens", "true");
        screens->setAttribute ("android:largeScreens", "true");
        //screens->setAttribute ("android:xlargeScreens", "true");
        screens->setAttribute ("android:anyDensity", "true");

        XmlElement* sdk = manifest->createNewChildElement ("uses-sdk");
        sdk->setAttribute ("android:minSdkVersion", getMinimumSDKVersionString());
        sdk->setAttribute ("android:targetSdkVersion", "11");

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

        String androidThemeString (getThemeString());
        if (androidThemeString.isNotEmpty())
            app->setAttribute ("android:theme", androidThemeString);

        {
            ScopedPointer<Drawable> bigIcon (getBigIcon()), smallIcon (getSmallIcon());

            if (bigIcon != nullptr || smallIcon != nullptr)
                app->setAttribute ("android:icon", "@drawable/icon");
        }

        if (getMinimumSDKVersionString().getIntValue() >= 11)
            app->setAttribute ("android:hardwareAccelerated", "false"); // (using the 2D acceleration slows down openGL)

        XmlElement* act = app->createNewChildElement ("activity");
        act->setAttribute ("android:name", getActivitySubClassName());
        act->setAttribute ("android:label", "@string/app_name");
        act->setAttribute ("android:configChanges", "keyboardHidden|orientation|screenSize");
        act->setAttribute ("android:screenOrientation", getScreenOrientationString());

        XmlElement* intent = act->createNewChildElement ("intent-filter");
        intent->createNewChildElement ("action")->setAttribute ("android:name", "android.intent.action.MAIN");
        intent->createNewChildElement ("category")->setAttribute ("android:name", "android.intent.category.LAUNCHER");

        return manifest;
    }

    //==============================================================================
    Value sdkPath, ndkPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidProjectExporterBase)
};
