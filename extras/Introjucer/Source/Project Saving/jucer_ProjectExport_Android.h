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

        return 0;
    }

    //==============================================================================
    AndroidProjectExporter (Project& project_, const ValueTree& settings_)
        : ProjectExporter (project_, settings_)
    {
        name = getNameAndroid();

        if (getTargetLocation().toString().isEmpty())
            getTargetLocation() = getDefaultBuildsRootFolder() + "Android";

        if (getSDKPath().toString().isEmpty())
            getSDKPath() = "${user.home}/SDKs/android-sdk-macosx";

        if (getNDKPath().toString().isEmpty())
            getNDKPath() = "${user.home}/SDKs/android-ndk-r7";

        if (getInternetNeeded().toString().isEmpty())
            getInternetNeeded() = true;

        androidDynamicLibs.add ("GLESv1_CM");
        androidDynamicLibs.add ("GLESv2");
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

    void createPropertyEditors (Array <PropertyComponent*>& props)
    {
        ProjectExporter::createPropertyEditors (props);

        props.add (new TextPropertyComponent (getSDKPath(), "Android SDK Path", 1024, false));
        props.getLast()->setTooltip ("The path to the Android SDK folder on the target build machine");

        props.add (new TextPropertyComponent (getNDKPath(), "Android NDK Path", 1024, false));
        props.getLast()->setTooltip ("The path to the Android NDK folder on the target build machine");

        props.add (new BooleanPropertyComponent (getInternetNeeded(), "Internet Access", "Specify internet access permission in the manifest"));
        props.getLast()->setTooltip ("If enabled, this will set the android.permission.INTERNET flag in the manifest.");
    }

    Value getSDKPath() const                    { return getSetting (Ids::androidSDKPath); }
    Value getNDKPath() const                    { return getSetting (Ids::androidNDKPath); }
    Value getInternetNeeded() const             { return getSetting (Ids::androidInternetNeeded); }

    //==============================================================================
    void create()
    {
        const File target (getTargetFolder());
        const File jniFolder (target.getChildFile ("jni"));

        createDirectoryOrThrow (target.getChildFile ("src/com"));
        createDirectoryOrThrow (jniFolder);
        createDirectoryOrThrow (target.getChildFile ("res/drawable-hdpi"));
        createDirectoryOrThrow (target.getChildFile ("res/drawable-mdpi"));
        createDirectoryOrThrow (target.getChildFile ("res/drawable-ldpi"));
        createDirectoryOrThrow (target.getChildFile ("res/values"));
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

        writeIcon (target.getChildFile ("res/drawable-hdpi/icon.png"), 72);
        writeIcon (target.getChildFile ("res/drawable-mdpi/icon.png"), 48);
        writeIcon (target.getChildFile ("res/drawable-ldpi/icon.png"), 36);

        writeStringsFile (target.getChildFile ("res/values/strings.xml"));
    }

private:
    //==============================================================================
    XmlElement* createManifestXML()
    {
        XmlElement* manifest = new XmlElement ("manifest");

        manifest->setAttribute ("xmlns:android", "http://schemas.android.com/apk/res/android");
        manifest->setAttribute ("android:versionCode", "1");
        manifest->setAttribute ("android:versionName", "1.0");
        manifest->setAttribute ("package", "com.juce");

        XmlElement* screens = manifest->createNewChildElement ("supports-screens");
        screens->setAttribute ("android:smallScreens", "true");
        screens->setAttribute ("android:normalScreens", "true");
        screens->setAttribute ("android:largeScreens", "true");
        //screens->setAttribute ("android:xlargeScreens", "true");
        screens->setAttribute ("android:anyDensity", "true");

        if (getInternetNeeded().getValue())
        {
            XmlElement* permission = manifest->createNewChildElement ("uses-permission");
            permission->setAttribute ("android:name", "android.permission.INTERNET");
        }

        XmlElement* app = manifest->createNewChildElement ("application");
        app->setAttribute ("android:label", "@string/app_name");
        app->setAttribute ("android:icon", "@drawable/icon");

        XmlElement* act = app->createNewChildElement ("activity");
        act->setAttribute ("android:name", "JuceAppActivity");
        act->setAttribute ("android:label", "@string/app_name");

        XmlElement* intent = act->createNewChildElement ("intent-filter");
        intent->createNewChildElement ("action")->setAttribute ("android:name", "android.intent.action.MAIN");
        intent->createNewChildElement ("category")->setAttribute ("android:name", "android.intent.category.LAUNCHER");

        return manifest;
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

    void writeApplicationMk (const File& file)
    {
        MemoryOutputStream mo;

        mo << "# Automatically generated makefile, created by the Introjucer" << newLine
           << "# Don't edit this file! Your changes will be overwritten when you re-save the Introjucer project!" << newLine
           << newLine
           << "APP_STL := gnustl_static" << newLine
           << "APP_CPPFLAGS += -fsigned-char -fexceptions -frtti" << newLine
           << "APP_PLATFORM := android-7" << newLine;

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

        out << newLine
            << "ifeq ($(CONFIG),Debug)" << newLine
            << "  LOCAL_CPPFLAGS += " << createCPPFlags (true) << newLine
            << "else" << newLine
            << "  LOCAL_CPPFLAGS += " << createCPPFlags (false) << newLine
            << "endif" << newLine
            << newLine
            << getDynamicLibs()
            << newLine
            << "include $(BUILD_SHARED_LIBRARY)" << newLine;
    }

    String getDynamicLibs()
    {
        if (androidDynamicLibs.size() == 0)
            return String::empty;

        String flags ("LOCAL_LDLIBS :=");
        for (int i = 0; i < androidDynamicLibs.size(); ++i)
            flags << " -l" << androidDynamicLibs[i];

        return flags + newLine;
    }

    String createIncludePathFlags (const Project::BuildConfiguration& config)
    {
        String flags;
        StringArray searchPaths (extraSearchPaths);
        searchPaths.addArray (config.getHeaderSearchPaths());
        searchPaths.removeDuplicates (false);

        for (int i = 0; i < searchPaths.size(); ++i)
            flags << " -I " << FileHelpers::unixStylePath (replacePreprocessorTokens (config, searchPaths[i])).quoted();

        return flags;
    }

    String createCPPFlags (bool forDebug)
    {
        String flags ("-fsigned-char -fexceptions -frtti");

        if (forDebug)
            flags << " -g";

        for (int i = 0; i < configs.size(); ++i)
        {
            if (configs.getReference(i).isDebug() == forDebug)
            {
                flags << createIncludePathFlags (configs.getReference(i));
                break;
            }
        }

        StringPairArray defines;
        defines.set ("JUCE_ANDROID", "1");

        if (forDebug)
        {
            defines.set ("DEBUG", "1");
            defines.set ("_DEBUG", "1");
        }
        else
        {
            defines.set ("NDEBUG", "1");
        }

        for (int i = 0; i < configs.size(); ++i)
        {
            const Project::BuildConfiguration& config = configs.getReference(i);

            if (config.isDebug() == forDebug)
            {
                flags << " -O" << config.getGCCOptimisationFlag();

                defines = mergePreprocessorDefs (defines, getAllPreprocessorDefs (config));
                break;
            }
        }

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

        //addLinkStep (proj, "${basedir}/" + rebaseFromProjectFolderToBuildTarget (RelativePath()).toUnixStyle() + "/", "jni/app");
        addLinkStep (proj, "${basedir}/" + getJucePathFromTargetFolder().toUnixStyle() + "/modules/juce_core/native/java/", "src/com/juce");

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

    static void addLinkStep (XmlElement* project, const String& from, const String& to)
    {
        XmlElement* executable = project->createNewChildElement ("exec");
        executable->setAttribute ("executable", "ln");
        executable->setAttribute ("dir", "${basedir}");
        executable->setAttribute ("failonerror", "false");

        executable->createNewChildElement ("arg")->setAttribute ("value", "-s");
        executable->createNewChildElement ("arg")->setAttribute ("value", from);
        executable->createNewChildElement ("arg")->setAttribute ("value", to);
    }

    void writeProjectPropertiesFile (const File& file)
    {
        MemoryOutputStream mo;
        mo << "# This file is used to override default values used by the Ant build system." << newLine
           << "# It is automatically generated - DO NOT EDIT IT or your changes will be lost!." << newLine
           << newLine
           << "target=Google Inc.:Google APIs:7" << newLine
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

    void writeIcon (const File& file, int size)
    {
        Image im (getBestIconForSize (size, false));

        if (im.isValid())
        {
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
