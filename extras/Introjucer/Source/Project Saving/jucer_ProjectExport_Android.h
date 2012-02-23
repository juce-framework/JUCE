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

#ifndef __JUCER_PROJECTEXPORT_ANDROID_JUCEHEADER__
#define __JUCER_PROJECTEXPORT_ANDROID_JUCEHEADER__

#include "jucer_ProjectExporter.h"


//==============================================================================
class AndroidProjectExporter  : public ProjectExporter
{
public:
    //==============================================================================
    static const char* getNameAndroid()         { return "Android Project"; }
    static const char* getValueTreeTypeName()   { return "ANDROID"; }

    static AndroidProjectExporter* createForSettings (Project& project, const ValueTree& settings)
    {
        if (settings.hasType (getValueTreeTypeName()))
            return new AndroidProjectExporter (project, settings);

        return nullptr;
    }

    //==============================================================================
    AndroidProjectExporter (Project& project_, const ValueTree& settings_)
        : ProjectExporter (project_, settings_)
    {
        name = getNameAndroid();

        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = getDefaultBuildsRootFolder() + "Android";

        if (getActivityClassPath().toString().isEmpty())
            getActivityClassPath() = createDefaultClassName();

        if (getSDKPath().toString().isEmpty())
            getSDKPath() = "${user.home}/SDKs/android-sdk-macosx";

        if (getNDKPath().toString().isEmpty())
            getNDKPath() = "${user.home}/SDKs/android-ndk-r7b";

        if (getMinimumSDKVersion().toString().isEmpty())
            getMinimumSDKVersion() = 8;

        if (getInternetNeeded().toString().isEmpty())
            getInternetNeeded() = true;
    }

    //==============================================================================
    int getLaunchPreferenceOrderForCurrentOS()
    {
       #if JUCE_ANDROID
        return 1;
       #else
        return 0;
       #endif
    }

    bool isPossibleForCurrentProject()          { return projectType.isGUIApplication(); }
    bool usesMMFiles() const                    { return false; }
    bool canCopeWithDuplicateFiles()            { return false; }

    void launchProject()
    {
    }

    void createPropertyEditors (PropertyListBuilder& props)
    {
        ProjectExporter::createPropertyEditors (props);

        props.add (new TextPropertyComponent (getActivityClassPath(), "Android Activity class name", 256, false),
                   "The full java class name to use for the app's Activity class.");

        props.add (new TextPropertyComponent (getSDKPath(), "Android SDK Path", 1024, false),
                   "The path to the Android SDK folder on the target build machine");

        props.add (new TextPropertyComponent (getNDKPath(), "Android NDK Path", 1024, false),
                   "The path to the Android NDK folder on the target build machine");

        props.add (new TextPropertyComponent (getMinimumSDKVersion(), "Minimum SDK version", 32, false),
                   "The number of the minimum version of the Android SDK that the app requires");

        props.add (new BooleanPropertyComponent (getInternetNeeded(), "Internet Access", "Specify internet access permission in the manifest"),
                   "If enabled, this will set the android.permission.INTERNET flag in the manifest.");

        props.add (new BooleanPropertyComponent (getAudioRecordNeeded(), "Audio Input Required", "Specify audio record permission in the manifest"),
                   "If enabled, this will set the android.permission.RECORD_AUDIO flag in the manifest.");

        props.add (new TextPropertyComponent (getOtherPermissions(), "Custom permissions", 2048, false),
                   "A space-separated list of other permission flags that should be added to the manifest.");
    }

    Value getActivityClassPath() const          { return getSetting (Ids::androidActivityClass); }
    Value getSDKPath() const                    { return getSetting (Ids::androidSDKPath); }
    Value getNDKPath() const                    { return getSetting (Ids::androidNDKPath); }
    Value getInternetNeeded() const             { return getSetting (Ids::androidInternetNeeded); }
    Value getAudioRecordNeeded() const          { return getSetting (Ids::androidMicNeeded); }
    Value getMinimumSDKVersion() const          { return getSetting (Ids::androidMinimumSDK); }
    Value getOtherPermissions() const           { return getSetting (Ids::androidOtherPermissions); }

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

    //==============================================================================
    void create (const OwnedArray<LibraryModule>& modules)
    {
        const File target (getTargetFolder());
        const File jniFolder (target.getChildFile ("jni"));

        copyActivityJavaFiles (modules);
        createDirectoryOrThrow (jniFolder);
        createDirectoryOrThrow (target.getChildFile ("res").getChildFile ("values"));
        createDirectoryOrThrow (target.getChildFile ("libs"));
        createDirectoryOrThrow (target.getChildFile ("bin"));

        {
            ScopedPointer<XmlElement> manifest (createManifestXML());
            writeXmlOrThrow (*manifest, target.getChildFile ("AndroidManifest.xml"), "utf-8", 100, true);
        }

        writeApplicationMk (jniFolder.getChildFile ("Application.mk"));
        writeAndroidMk (jniFolder.getChildFile ("Android.mk"));

        {
            ScopedPointer<XmlElement> antBuildXml (createAntBuildXML());
            writeXmlOrThrow (*antBuildXml, target.getChildFile ("build.xml"), "UTF-8", 100);
        }

        writeProjectPropertiesFile (target.getChildFile ("project.properties"));
        writeLocalPropertiesFile (target.getChildFile ("local.properties"));
        writeStringsFile (target.getChildFile ("res/values/strings.xml"));

        const Image bigIcon (getBigIcon());
        const Image smallIcon (getSmallIcon());

        if (bigIcon.isValid() && smallIcon.isValid())
        {
            const int step = jmax (bigIcon.getWidth(), bigIcon.getHeight()) / 8;
            writeIcon (target.getChildFile ("res/drawable-xhdpi/icon.png"), getBestIconForSize (step * 8, false));
            writeIcon (target.getChildFile ("res/drawable-hdpi/icon.png"),  getBestIconForSize (step * 6, false));
            writeIcon (target.getChildFile ("res/drawable-mdpi/icon.png"),  getBestIconForSize (step * 4, false));
            writeIcon (target.getChildFile ("res/drawable-ldpi/icon.png"),  getBestIconForSize (step * 3, false));
        }
        else
        {
            writeIcon (target.getChildFile ("res/drawable-mdpi/icon.png"), bigIcon.isValid() ? bigIcon : smallIcon);
        }
    }

protected:
    //==============================================================================
    class AndroidBuildConfiguration  : public BuildConfiguration
    {
    public:
        AndroidBuildConfiguration (Project& project, const ValueTree& settings)
            : BuildConfiguration (project, settings)
        {
            if (getArchitectures().toString().isEmpty())
                getArchitectures() = "armeabi armeabi-v7a";
        }

        Value getArchitectures() const   { return getValue (Ids::androidArchitectures); }

        void createPropertyEditors (PropertyListBuilder& props)
        {
            createBasicPropertyEditors (props);

            props.add (new TextPropertyComponent (getArchitectures(), "Architectures", 256, false),
                       "A list of the ARM architectures to build (for a fat binary).");
        }
    };

    BuildConfiguration::Ptr createBuildConfig (const ValueTree& settings) const
    {
        return new AndroidBuildConfiguration (project, settings);
    }

private:
    //==============================================================================
    XmlElement* createManifestXML()
    {
        XmlElement* manifest = new XmlElement ("manifest");

        manifest->setAttribute ("xmlns:android", "http://schemas.android.com/apk/res/android");
        manifest->setAttribute ("android:versionCode", "1");
        manifest->setAttribute ("android:versionName", "1.0");
        manifest->setAttribute ("package", getActivityClassPackage());

        XmlElement* screens = manifest->createNewChildElement ("supports-screens");
        screens->setAttribute ("android:smallScreens", "true");
        screens->setAttribute ("android:normalScreens", "true");
        screens->setAttribute ("android:largeScreens", "true");
        //screens->setAttribute ("android:xlargeScreens", "true");
        screens->setAttribute ("android:anyDensity", "true");

        manifest->createNewChildElement ("uses-sdk")->setAttribute ("android:minSdkVersion", getMinimumSDKVersion().toString());

        {
            const StringArray permissions (getPermissionsRequired());

            for (int i = permissions.size(); --i >= 0;)
                manifest->createNewChildElement ("uses-permission")->setAttribute ("android:name", permissions[i]);
        }

        if (project.isModuleEnabled ("juce_opengl"))
        {
            XmlElement* feature = manifest->createNewChildElement ("uses-feature");
            feature->setAttribute ("android:glEsVersion", "0x00020000");
            feature->setAttribute ("android:required", "true");
        }

        XmlElement* app = manifest->createNewChildElement ("application");
        app->setAttribute ("android:label", "@string/app_name");
        app->setAttribute ("android:icon", "@drawable/icon");

        XmlElement* act = app->createNewChildElement ("activity");
        act->setAttribute ("android:name", getActivityName());
        act->setAttribute ("android:label", "@string/app_name");

        XmlElement* intent = act->createNewChildElement ("intent-filter");
        intent->createNewChildElement ("action")->setAttribute ("android:name", "android.intent.action.MAIN");
        intent->createNewChildElement ("category")->setAttribute ("android:name", "android.intent.category.LAUNCHER");

        return manifest;
    }

    StringArray getPermissionsRequired() const
    {
        StringArray s;
        s.addTokens (getOtherPermissions().toString(), ", ", "");

        if (getInternetNeeded().getValue())         s.add ("android.permission.INTERNET");
        if (getAudioRecordNeeded().getValue())      s.add ("android.permission.RECORD_AUDIO");

        s.trim();
        s.removeDuplicates (false);
        return s;
    }

    //==============================================================================
    void findAllFilesToCompile (const Project::Item& projectItem, Array<RelativePath>& results)
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                findAllFilesToCompile (projectItem.getChild(i), results);
        }
        else
        {
            if (projectItem.shouldBeCompiled())
                results.add (RelativePath (projectItem.getFile(), getTargetFolder(), RelativePath::buildTargetFolder));
        }
    }

    //==============================================================================
    String getActivityName() const
    {
        return getActivityClassPath().toString().fromLastOccurrenceOf (".", false, false);
    }

    String getActivityClassPackage() const
    {
        return getActivityClassPath().toString().upToLastOccurrenceOf (".", false, false);
    }

    String getJNIActivityClassName() const
    {
        return getActivityClassPath().toString().replaceCharacter ('.', '/');
    }

    static LibraryModule* getCoreModule (const OwnedArray<LibraryModule>& modules)
    {
        for (int i = modules.size(); --i >= 0;)
            if (modules.getUnchecked(i)->getID() == "juce_core")
                return modules.getUnchecked(i);

        return nullptr;
    }

    void copyActivityJavaFiles (const OwnedArray<LibraryModule>& modules)
    {
        const String className (getActivityName());
        const String package (getActivityClassPackage());
        String path (package.replaceCharacter ('.', File::separator));

        if (path.isEmpty() || className.isEmpty())
            throw SaveError ("Invalid Android Activity class name: " + getActivityClassPath().toString());

        const File classFolder (getTargetFolder().getChildFile ("src")
                                                 .getChildFile (path));
        createDirectoryOrThrow (classFolder);

        LibraryModule* const coreModule = getCoreModule (modules);

        if (coreModule == nullptr)
            throw SaveError ("To build an Android app, the juce_core module must be included in your project!");

        File javaDestFile (classFolder.getChildFile (className + ".java"));

        File javaSourceFile (coreModule->getFolder().getChildFile ("native")
                                                    .getChildFile ("java")
                                                    .getChildFile ("JuceAppActivity.java"));

        MemoryOutputStream newFile;
        newFile << javaSourceFile.loadFileAsString()
                                 .replace ("JuceAppActivity", className)
                                 .replace ("package com.juce;", "package " + package + ";");

        overwriteFileIfDifferentOrThrow (javaDestFile, newFile);
    }

    void writeApplicationMk (const File& file)
    {
        MemoryOutputStream mo;

        mo << "# Automatically generated makefile, created by the Introjucer" << newLine
           << "# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!" << newLine
           << newLine
           << "APP_STL := gnustl_static" << newLine
           << "APP_CPPFLAGS += -fsigned-char -fexceptions -frtti" << newLine
           << "APP_PLATFORM := android-8" << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeAndroidMk (const File& file)
    {
        Array<RelativePath> files;

        for (int i = 0; i < groups.size(); ++i)
            findAllFilesToCompile (groups.getReference(i), files);

        MemoryOutputStream mo;
        writeAndroidMk (mo, files);

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeAndroidMk (OutputStream& out, const Array<RelativePath>& files)
    {
        out << "# Automatically generated makefile, created by the Introjucer" << newLine
            << "# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!" << newLine
            << newLine
            << "LOCAL_PATH := $(call my-dir)" << newLine
            << newLine
            << "include $(CLEAR_VARS)" << newLine
            << newLine
            << "LOCAL_MODULE := juce_jni" << newLine
            << "LOCAL_SRC_FILES := \\" << newLine;

        for (int i = 0; i < files.size(); ++i)
            out << "  ../" << escapeSpaces (files.getReference(i).toUnixStyle()) << "\\" << newLine;

        String debugSettings, releaseSettings;

        out << newLine
            << "ifeq ($(CONFIG),Debug)" << newLine;
        writeConfigSettings (out, true);
        out << "else" << newLine;
        writeConfigSettings (out, false);
        out << "endif" << newLine
            << newLine
            << "include $(BUILD_SHARED_LIBRARY)" << newLine;
    }

    void writeConfigSettings (OutputStream& out, bool forDebug)
    {
        for (ConfigIterator config (*this); config.next();)
        {
            if (config->isDebug() == forDebug)
            {
                const AndroidBuildConfiguration& androidConfig = dynamic_cast <const AndroidBuildConfiguration&> (*config);

                out << "  LOCAL_CPPFLAGS += " << createCPPFlags (*config) << newLine
                    << "  APP_ABI := " << androidConfig.getArchitectures().toString() << newLine
                    << getDynamicLibs (androidConfig);

                break;
            }
        }
    }

    String getDynamicLibs (const AndroidBuildConfiguration& config)
    {
        String flags ("  LOCAL_LDLIBS :=");

        flags << config.getGCCLibraryPathFlags();

        {
            StringArray libs;
            libs.add ("log");
            libs.add ("GLESv1_CM");
            libs.add ("GLESv2");

            for (int i = 0; i < libs.size(); ++i)
                flags << " -l" << libs[i];
        }

        return flags + newLine;
    }

    String createIncludePathFlags (const BuildConfiguration& config)
    {
        String flags;
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths.removeDuplicates (false);

        for (int i = 0; i < searchPaths.size(); ++i)
            flags << " -I " << FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i])).quoted();

        return flags;
    }

    String createCPPFlags (const BuildConfiguration& config)
    {
        StringPairArray defines;
        defines.set ("JUCE_ANDROID", "1");
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSNAME", getJNIActivityClassName().replaceCharacter ('/', '_'));
        defines.set ("JUCE_ANDROID_ACTIVITY_CLASSPATH", "\\\"" + getJNIActivityClassName() + "\\\"");

        String flags ("-fsigned-char -fexceptions -frtti");

        if (config.isDebug().getValue())
        {
            flags << " -g";
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        flags << createIncludePathFlags (config)
              << " -O" << config.getGCCOptimisationFlag();

        defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));
        return flags + createGCCPreprocessorFlags (defines);
    }

    //==============================================================================
    XmlElement* createAntBuildXML()
    {
        XmlElement* proj = new XmlElement ("project");
        proj->setAttribute ("name", projectName);
        proj->setAttribute ("default", "debug");

        proj->createNewChildElement ("loadproperties")->setAttribute ("srcFile", "local.properties");
        proj->createNewChildElement ("loadproperties")->setAttribute ("srcFile", "project.properties");

        XmlElement* path = proj->createNewChildElement ("path");
        path->setAttribute ("id", "android.antlibs");
        path->createNewChildElement ("pathelement")->setAttribute ("path", "${sdk.dir}/tools/lib/anttasks.jar");
        path->createNewChildElement ("pathelement")->setAttribute ("path", "${sdk.dir}/tools/lib/sdklib.jar");
        path->createNewChildElement ("pathelement")->setAttribute ("path", "${sdk.dir}/tools/lib/androidprefs.jar");

        XmlElement* taskdef = proj->createNewChildElement ("taskdef");
        taskdef->setAttribute ("name", "setup");
        taskdef->setAttribute ("classname", "com.android.ant.SetupTask");
        taskdef->setAttribute ("classpathref", "android.antlibs");

        addNDKBuildStep (proj, "clean", "clean");

        addNDKBuildStep (proj, "debug", "CONFIG=Debug");
        addNDKBuildStep (proj, "release", "CONFIG=Release");

        proj->createNewChildElement ("import")->setAttribute ("file", "${sdk.dir}/tools/ant/build.xml");

        return proj;
    }

    static void addNDKBuildStep (XmlElement* project, const String& type, const String& arg)
    {
        XmlElement* target = project->createNewChildElement ("target");
        target->setAttribute ("name", type);

        XmlElement* executable = target->createNewChildElement ("exec");
        executable->setAttribute ("executable", "${ndk.dir}/ndk-build");
        executable->setAttribute ("dir", "${basedir}");
        executable->setAttribute ("failonerror", "true");

        executable->createNewChildElement ("arg")->setAttribute ("value", "--jobs=2");
        executable->createNewChildElement ("arg")->setAttribute ("value", arg);
    }

    void writeProjectPropertiesFile (const File& file)
    {
        MemoryOutputStream mo;
        mo << "# This file is used to override default values used by the Ant build system." << newLine
           << "# It is automatically generated - DO NOT EDIT IT or your changes will be lost!." << newLine
           << newLine
           << "target=Google Inc.:Google APIs:8" << newLine
           << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeLocalPropertiesFile (const File& file)
    {
        MemoryOutputStream mo;
        mo << "# This file is used to override default values used by the Ant build system." << newLine
           << "# It is automatically generated by the Introjucer - DO NOT EDIT IT or your changes will be lost!." << newLine
           << newLine
           << "sdk.dir=" << escapeSpaces (replacePreprocessorDefs (getAllPreprocessorDefs(), getSDKPath().toString())) << newLine
           << "ndk.dir=" << escapeSpaces (replacePreprocessorDefs (getAllPreprocessorDefs(), getNDKPath().toString())) << newLine
           << newLine;

        overwriteFileIfDifferentOrThrow (file, mo);
    }

    void writeIcon (const File& file, const Image& im)
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

    void writeStringsFile (const File& file)
    {
        XmlElement strings ("resources");
        XmlElement* name = strings.createNewChildElement ("string");
        name->setAttribute ("name", "app_name");
        name->addTextElement (projectName);

        writeXmlOrThrow (strings, file, "utf-8", 100);
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE (AndroidProjectExporter);
};


#endif   // __JUCER_PROJECTEXPORT_ANDROID_JUCEHEADER__
