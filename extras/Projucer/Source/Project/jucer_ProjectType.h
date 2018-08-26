/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

class Project;
class ProjectExporter;

//==============================================================================
class ProjectType
{
public:
    //==============================================================================
    virtual ~ProjectType();

    const String& getType() const noexcept          { return type; }
    const String& getDescription() const noexcept   { return desc; }

    //==============================================================================
    static Array<ProjectType*> getAllTypes();
    static const ProjectType* findType (const String& typeCode);

    //==============================================================================
    virtual bool isStaticLibrary() const        { return false; }
    virtual bool isDynamicLibrary() const       { return false; }
    virtual bool isGUIApplication() const       { return false; }
    virtual bool isCommandLineApp() const       { return false; }
    virtual bool isAudioPlugin() const          { return false; }

    //==============================================================================
    struct Target
    {
        enum Type
        {
            GUIApp            = 0,
            ConsoleApp        = 1,
            StaticLibrary     = 2,
            DynamicLibrary    = 3,

            VSTPlugIn         = 10,
            VST3PlugIn        = 11,
            AAXPlugIn         = 12,
            RTASPlugIn        = 13,
            AudioUnitPlugIn   = 14,
            AudioUnitv3PlugIn = 15,
            StandalonePlugIn  = 16,
            UnityPlugIn       = 17,

            SharedCodeTarget  = 20, // internal
            AggregateTarget   = 21,

            unspecified       = 30
        };

        enum TargetFileType
        {
            executable            = 0,
            staticLibrary         = 1,
            sharedLibraryOrDLL    = 2,
            pluginBundle          = 3,
            macOSAppex            = 4,
            unknown               = 5
        };

        //==============================================================================
        Target (Type targetType) : type (targetType) {}

        const char* getName() const noexcept;
        TargetFileType getTargetFileType() const noexcept;

        const Type type;

    private:
        //==============================================================================
        Target& operator= (const Target&) = delete;
    };

    virtual bool supportsTargetType (Target::Type /*targetType*/) const     { return false; }

protected:
    ProjectType (const String& type, const String& desc);

private:
    const String type, desc;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectType)
};

//==============================================================================
inline ProjectType::ProjectType (const String& t, const String& d)
    : type (t), desc (d)
{
}

inline ProjectType::~ProjectType()
{
    getAllTypes().removeFirstMatchingValue (this);
}

inline const ProjectType* ProjectType::findType (const String& typeCode)
{
    const auto& types = getAllTypes();

    for (auto i = types.size(); --i >= 0;)
        if (types.getUnchecked(i)->getType() == typeCode)
            return types.getUnchecked(i);

    jassertfalse;
    return nullptr;
}

//==============================================================================
struct ProjectType_GUIApp  : public ProjectType
{
    ProjectType_GUIApp()  : ProjectType (getTypeName(), "GUI Application") {}

    static const char* getTypeName() noexcept                          { return "guiapp"; }
    bool isGUIApplication() const  override                            { return true; }
    bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::GUIApp); }
};

struct ProjectType_ConsoleApp  : public ProjectType
{
    ProjectType_ConsoleApp()  : ProjectType (getTypeName(), "Console Application") {}

    static const char* getTypeName() noexcept                          { return "consoleapp"; }
    bool isCommandLineApp() const  override                            { return true; }
    bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::ConsoleApp); }
};

struct ProjectType_StaticLibrary  : public ProjectType
{
    ProjectType_StaticLibrary()  : ProjectType (getTypeName(), "Static Library") {}

    static const char* getTypeName() noexcept                          { return "library"; }
    bool isStaticLibrary() const  override                             { return true; }
    bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::StaticLibrary); }
};

struct ProjectType_DLL  : public ProjectType
{
    ProjectType_DLL()  : ProjectType (getTypeName(), "Dynamic Library") {}

    static const char* getTypeName() noexcept                          { return "dll"; }
    bool isDynamicLibrary() const override                             { return true; }
    bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::DynamicLibrary); }
};

struct ProjectType_AudioPlugin  : public ProjectType
{
    ProjectType_AudioPlugin()  : ProjectType (getTypeName(), "Audio Plug-in") {}

    static const char* getTypeName() noexcept   { return "audioplug"; }
    bool isAudioPlugin() const override         { return true; }

    bool supportsTargetType (Target::Type targetType) const override
    {
        switch (targetType)
        {
            case Target::VSTPlugIn:
            case Target::VST3PlugIn:
            case Target::AAXPlugIn:
            case Target::RTASPlugIn:
            case Target::AudioUnitPlugIn:
            case Target::AudioUnitv3PlugIn:
            case Target::StandalonePlugIn:
            case Target::UnityPlugIn:
            case Target::SharedCodeTarget:
            case Target::AggregateTarget:
                return true;
            default:
                break;
        }

        return false;
    }
};

//==============================================================================
inline Array<ProjectType*> ProjectType::getAllTypes()
{
    static ProjectType_GUIApp guiApp;
    static ProjectType_ConsoleApp consoleApp;
    static ProjectType_StaticLibrary staticLib;
    static ProjectType_DLL dll;
    static ProjectType_AudioPlugin plugin;

    return Array<ProjectType*>(&guiApp, &consoleApp, &staticLib, &dll, &plugin);
}
