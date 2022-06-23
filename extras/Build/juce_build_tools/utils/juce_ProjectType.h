/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace build_tools
{
    //==============================================================================
    class ProjectType
    {
    public:
        //==============================================================================
        virtual ~ProjectType() { getAllTypes().removeFirstMatchingValue (this); }

        const String& getType() const noexcept          { return type; }
        const String& getDescription() const noexcept   { return desc; }

        //==============================================================================
        static Array<ProjectType*> getAllTypes();
        static const ProjectType* findType (const String& typeCode)
        {
            const auto& types = getAllTypes();

            for (auto i = types.size(); --i >= 0;)
                if (types.getUnchecked(i)->getType() == typeCode)
                    return types.getUnchecked(i);

            jassertfalse;
            return nullptr;
        }

        //==============================================================================
        virtual bool isStaticLibrary() const        { return false; }
        virtual bool isDynamicLibrary() const       { return false; }
        virtual bool isGUIApplication() const       { return false; }
        virtual bool isCommandLineApp() const       { return false; }
        virtual bool isAudioPlugin() const          { return false; }
        virtual bool isARAAudioPlugin() const       { return false; }

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

                AudioUnitPlugIn   = 14,
                AudioUnitv3PlugIn = 15,
                StandalonePlugIn  = 16,
                UnityPlugIn       = 17,
                LV2PlugIn         = 18,

                SharedCodeTarget  = 20, // internal
                AggregateTarget   = 21,

                LV2TurtleProgram  = 25, // internal

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
            explicit Target (Type targetType) : type (targetType) {}

            const char* getName() const noexcept
            {
                switch (type)
                {
                    case GUIApp:            return "App";
                    case ConsoleApp:        return "ConsoleApp";
                    case StaticLibrary:     return "Static Library";
                    case DynamicLibrary:    return "Dynamic Library";
                    case VSTPlugIn:         return "VST";
                    case VST3PlugIn:        return "VST3";
                    case AudioUnitPlugIn:   return "AU";
                    case StandalonePlugIn:  return "Standalone Plugin";
                    case AudioUnitv3PlugIn: return "AUv3 AppExtension";
                    case AAXPlugIn:         return "AAX";
                    case UnityPlugIn:       return "Unity Plugin";
                    case LV2PlugIn:         return "LV2 Plugin";
                    case SharedCodeTarget:  return "Shared Code";
                    case AggregateTarget:   return "All";
                    case LV2TurtleProgram:  return "LV2 Manifest Helper";
                    case unspecified:       break;
                }

                return "undefined";
            }

            static Type typeFromName (const String& name)
            {
                if (name == "App")                  return Type::GUIApp;
                if (name == "ConsoleApp")           return Type::ConsoleApp;
                if (name == "Static Library")       return Type::StaticLibrary;
                if (name == "Dynamic Library")      return Type::DynamicLibrary;
                if (name == "VST")                  return Type::VSTPlugIn;
                if (name == "VST3")                 return Type::VST3PlugIn;
                if (name == "AU")                   return Type::AudioUnitPlugIn;
                if (name == "Standalone Plugin")    return Type::StandalonePlugIn;
                if (name == "AUv3 AppExtension")    return Type::AudioUnitv3PlugIn;
                if (name == "AAX")                  return Type::AAXPlugIn;
                if (name == "Unity Plugin")         return Type::UnityPlugIn;
                if (name == "LV2 Plugin")           return Type::LV2PlugIn;
                if (name == "Shared Code")          return Type::SharedCodeTarget;
                if (name == "All")                  return Type::AggregateTarget;
                if (name == "LV2 Manifest Helper")  return Type::LV2TurtleProgram;

                jassertfalse;
                return Type::ConsoleApp;
            }

            TargetFileType getTargetFileType() const noexcept
            {
                switch (type)
                {
                    case GUIApp:            return executable;
                    case ConsoleApp:        return executable;
                    case StaticLibrary:     return staticLibrary;
                    case DynamicLibrary:    return sharedLibraryOrDLL;
                    case VSTPlugIn:         return pluginBundle;
                    case VST3PlugIn:        return pluginBundle;
                    case AudioUnitPlugIn:   return pluginBundle;
                    case StandalonePlugIn:  return executable;
                    case AudioUnitv3PlugIn: return macOSAppex;
                    case AAXPlugIn:         return pluginBundle;
                    case UnityPlugIn:       return pluginBundle;
                    case LV2PlugIn:         return pluginBundle;
                    case SharedCodeTarget:  return staticLibrary;
                    case LV2TurtleProgram:  return executable;
                    case AggregateTarget:
                    case unspecified:
                        break;
                }

                return unknown;
            }

            const Type type;

        private:
            //==============================================================================
            Target& operator= (const Target&) = delete;
        };

        virtual bool supportsTargetType (Target::Type /*targetType*/) const     { return false; }

    protected:
        ProjectType (const String& t, const String& d)
            : type (t), desc (d)
        {}

    private:
        const String type, desc;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectType)
    };

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
                case Target::AudioUnitPlugIn:
                case Target::AudioUnitv3PlugIn:
                case Target::StandalonePlugIn:
                case Target::UnityPlugIn:
                case Target::LV2PlugIn:
                case Target::LV2TurtleProgram:
                case Target::SharedCodeTarget:
                case Target::AggregateTarget:
                    return true;

                case Target::GUIApp:
                case Target::ConsoleApp:
                case Target::StaticLibrary:
                case Target::DynamicLibrary:
                case Target::unspecified:
                    break;
            }

            return false;
        }
    };

    struct ProjectType_ARAAudioPlugin : public ProjectType
    {
        ProjectType_ARAAudioPlugin() : ProjectType (getTypeName(), "ARA Audio Plug-in") {}

        static const char* getTypeName() noexcept { return "araaudioplug"; }
        bool isAudioPlugin() const override { return true; }
        bool isARAAudioPlugin() const override { return true; }

        bool supportsTargetType (Target::Type targetType) const override
        {
            switch (targetType)
            {
                case Target::VSTPlugIn:
                case Target::VST3PlugIn:
                case Target::AAXPlugIn:
                case Target::AudioUnitPlugIn:
                case Target::AudioUnitv3PlugIn:
                case Target::StandalonePlugIn:
                case Target::UnityPlugIn:
                case Target::SharedCodeTarget:
                case Target::AggregateTarget:
                    return true;

                case Target::GUIApp:
                case Target::ConsoleApp:
                case Target::StaticLibrary:
                case Target::DynamicLibrary:
                case Target::unspecified:
                case Target::LV2PlugIn:
                case Target::LV2TurtleProgram:
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
        static ProjectType_ARAAudioPlugin araplugin;

        return Array<ProjectType*>(&guiApp, &consoleApp, &staticLib, &dll, &plugin, &araplugin);
    }
}
}
