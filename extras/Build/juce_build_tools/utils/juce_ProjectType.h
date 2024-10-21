/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce::build_tools
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
                if (types.getUnchecked (i)->getType() == typeCode)
                    return types.getUnchecked (i);

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

                LV2Helper         = 25, // internal
                VST3Helper        = 26, // internal

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
                    case LV2Helper:         return "LV2 Manifest Helper";
                    case VST3Helper:        return "VST3 Manifest Helper";
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
                if (name == "LV2 Manifest Helper")  return Type::LV2Helper;
                if (name == "VST3 Manifest Helper") return Type::VST3Helper;

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
                    case LV2Helper:         return executable;
                    case VST3Helper:        return executable;
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
    struct ProjectType_GUIApp final : public ProjectType
    {
        ProjectType_GUIApp()  : ProjectType (getTypeName(), "GUI Application") {}

        static const char* getTypeName() noexcept                          { return "guiapp"; }
        bool isGUIApplication() const  override                            { return true; }
        bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::GUIApp); }
    };

    struct ProjectType_ConsoleApp final : public ProjectType
    {
        ProjectType_ConsoleApp()  : ProjectType (getTypeName(), "Console Application") {}

        static const char* getTypeName() noexcept                          { return "consoleapp"; }
        bool isCommandLineApp() const  override                            { return true; }
        bool supportsTargetType (Target::Type targetType) const override   { return (targetType == Target::ConsoleApp); }
    };

    struct ProjectType_StaticLibrary final : public ProjectType
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

    struct ProjectType_AudioPlugin final : public ProjectType
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
                case Target::LV2Helper:
                case Target::VST3Helper:
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

    struct ProjectType_ARAAudioPlugin final : public ProjectType
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
                case Target::VST3Helper:
                    return true;

                case Target::GUIApp:
                case Target::ConsoleApp:
                case Target::StaticLibrary:
                case Target::DynamicLibrary:
                case Target::unspecified:
                case Target::LV2PlugIn:
                case Target::LV2Helper:
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

        return Array<ProjectType*> (&guiApp, &consoleApp, &staticLib, &dll, &plugin, &araplugin);
    }

} // namespace juce::build_tools
