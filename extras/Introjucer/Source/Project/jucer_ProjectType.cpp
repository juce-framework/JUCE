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

#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "jucer_AudioPluginModule.h"


//==============================================================================
ProjectType::ProjectType (const String& type_, const String& desc_)
    : type (type_), desc (desc_)
{
    getAllTypes().add (this);
}

ProjectType::~ProjectType()
{
    getAllTypes().removeValue (this);
}

Array<ProjectType*>& ProjectType::getAllTypes()
{
    static Array<ProjectType*> types;
    return types;
}

const ProjectType* ProjectType::findType (const String& typeCode)
{
    const Array<ProjectType*>& types = getAllTypes();

    for (int i = types.size(); --i >= 0;)
        if (types.getUnchecked(i)->getType() == typeCode)
            return types.getUnchecked(i);

    jassertfalse;
    return nullptr;
}

//==============================================================================
class ProjectType_GUIApp  : public ProjectType
{
public:
    ProjectType_GUIApp()  : ProjectType (getTypeName(), "Application (GUI)") {}

    static const char* getTypeName() noexcept   { return "guiapp"; }
    bool isGUIApplication() const               { return true; }

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodePackageType = "APPL";
        exporter.xcodeBundleSignature = "????";
        exporter.xcodeCreatePList = true;
        exporter.xcodeFileType = "wrapper.application";
        exporter.xcodeBundleExtension = ".app";
        exporter.xcodeProductType = "com.apple.product-type.application";
        exporter.xcodeProductInstallPath = "$(HOME)/Applications";

        exporter.msvcIsWindowsSubsystem = true;
        exporter.msvcTargetSuffix = ".exe";
    }
};

//==============================================================================
class ProjectType_ConsoleApp  : public ProjectType
{
public:
    ProjectType_ConsoleApp()  : ProjectType (getTypeName(), "Application (Non-GUI)") {}

    static const char* getTypeName() noexcept   { return "consoleapp"; }
    bool isCommandLineApp() const               { return true; }

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "compiled.mach-o.executable";
        exporter.xcodeBundleExtension = String::empty;
        exporter.xcodeProductType = "com.apple.product-type.tool";
        exporter.xcodeProductInstallPath = "/usr/bin";

        exporter.msvcIsWindowsSubsystem = false;
        exporter.msvcTargetSuffix = ".exe";
        exporter.msvcExtraPreprocessorDefs.set ("_CONSOLE", "");
    }
};

//==============================================================================
class ProjectType_Library  : public ProjectType
{
public:
    ProjectType_Library()  : ProjectType (getTypeName(), "Static Library") {}

    static const char* getTypeName() noexcept   { return "library"; }
    bool isLibrary() const                      { return true; }

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "archive.ar";
        exporter.xcodeProductType = "com.apple.product-type.library.static";
        exporter.xcodeProductInstallPath = String::empty;

        exporter.makefileTargetSuffix = ".so";

        exporter.msvcTargetSuffix = exporter.getSetting (Ids::libraryType) == 2 ? ".dll" : ".lib";

        exporter.msvcExtraPreprocessorDefs.set ("_LIB", "");
    }
};

//==============================================================================
class ProjectType_AudioPlugin  : public ProjectType
{
public:
    ProjectType_AudioPlugin()  : ProjectType (getTypeName(), "Audio Plug-in") {}

    static const char* getTypeName() noexcept   { return "audioplug"; }
    bool isAudioPlugin() const                  { return true; }

    void setMissingProjectProperties (Project& project) const
    {
        if (! project.getProjectRoot().hasProperty (Ids::buildVST))
        {
            const String sanitisedProjectName (CodeHelpers::makeValidIdentifier (project.getProjectName().toString(), false, true, false));

            shouldBuildVST (project) = true;
            shouldBuildRTAS (project) = false;
            shouldBuildAU (project) = true;

            getPluginName (project) = project.getProjectName().toString();
            getPluginDesc (project) = project.getProjectName().toString();
            getPluginManufacturer (project) = "yourcompany";
            getPluginManufacturerCode (project) = "Manu";
            getPluginCode (project) = "Plug";
            getPluginChannelConfigs (project) = "{1, 1}, {2, 2}";
            getPluginIsSynth (project) = false;
            getPluginWantsMidiInput (project) = false;
            getPluginProducesMidiOut (project) = false;
            getPluginSilenceInProducesSilenceOut (project) = false;
            getPluginTailLengthSeconds (project) = 0;
            getPluginEditorNeedsKeyFocus (project) = false;
            getPluginAUExportPrefix (project) = sanitisedProjectName + "AU";
            getPluginAUCocoaViewClassName (project) = sanitisedProjectName + "AU_V1";
            getPluginRTASCategory (project) = String::empty;
        }
    }

    void createPropertyEditors (const Project& project, Array <PropertyComponent*>& props) const
    {
        props.add (new BooleanPropertyComponent (shouldBuildVST (project), "Build VST", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce a VST plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildAU (project), "Build AudioUnit", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an AudioUnit plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildRTAS (project), "Build RTAS", "Enabled"));
        props.getLast()->setTooltip ("Whether the project should produce an RTAS plugin.");

        props.add (new TextPropertyComponent (getPluginName (project), "Plugin Name", 128, false));
        props.getLast()->setTooltip ("The name of your plugin (keep it short!)");
        props.add (new TextPropertyComponent (getPluginDesc (project), "Plugin Description", 256, false));
        props.getLast()->setTooltip ("A short description of your plugin.");

        props.add (new TextPropertyComponent (getPluginManufacturer (project), "Plugin Manufacturer", 256, false));
        props.getLast()->setTooltip ("The name of your company (cannot be blank).");
        props.add (new TextPropertyComponent (getPluginManufacturerCode (project), "Plugin Manufacturer Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your company. Note that for AU compatibility, this must contain at least one upper-case letter!");
        props.add (new TextPropertyComponent (getPluginCode (project), "Plugin Code", 4, false));
        props.getLast()->setTooltip ("A four-character unique ID for your plugin. Note that for AU compatibility, this must contain at least one upper-case letter!");

        props.add (new TextPropertyComponent (getPluginChannelConfigs (project), "Plugin Channel Configurations", 256, false));
        props.getLast()->setTooltip ("This is the set of input/output channel configurations that your plugin can handle.  The list is a comma-separated set of pairs of values in the form { numInputs, numOutputs }, and each "
                                     "pair indicates a valid configuration that the plugin can handle. So for example, {1, 1}, {2, 2} means that the plugin can be used in just two configurations: either with 1 input "
                                     "and 1 output, or with 2 inputs and 2 outputs.");

        props.add (new BooleanPropertyComponent (getPluginIsSynth (project), "Plugin is a Synth", "Is a Synth"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to be treated as a synth or generator. It doesn't make much difference to the plugin itself, but some hosts treat synths differently to other plugins.");

        props.add (new BooleanPropertyComponent (getPluginWantsMidiInput (project), "Plugin Midi Input", "Plugin wants midi input"));
        props.getLast()->setTooltip ("Enable this if you want your plugin to accept midi messages.");

        props.add (new BooleanPropertyComponent (getPluginProducesMidiOut (project), "Plugin Midi Output", "Plugin produces midi output"));
        props.getLast()->setTooltip ("Enable this if your plugin is going to produce midi messages.");

        props.add (new BooleanPropertyComponent (getPluginSilenceInProducesSilenceOut (project), "Silence", "Silence in produces silence out"));
        props.getLast()->setTooltip ("Enable this if your plugin has no tail - i.e. if passing a silent buffer to it will always result in a silent buffer being produced.");

        props.add (new TextPropertyComponent (getPluginTailLengthSeconds (project), "Tail Length (in seconds)", 12, false));
        props.getLast()->setTooltip ("This indicates the length, in seconds, of the plugin's tail. This information may or may not be used by the host.");

        props.add (new BooleanPropertyComponent (getPluginEditorNeedsKeyFocus (project), "Key Focus", "Plugin editor requires keyboard focus"));
        props.getLast()->setTooltip ("Enable this if your plugin needs keyboard input - some hosts can be a bit funny about keyboard focus..");

        props.add (new TextPropertyComponent (getPluginAUExportPrefix (project), "Plugin AU Export Prefix", 64, false));
        props.getLast()->setTooltip ("A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");

        props.add (new TextPropertyComponent (getPluginAUCocoaViewClassName (project), "Plugin AU Cocoa View Name", 64, false));
        props.getLast()->setTooltip ("In an AU, this is the name of Cocoa class that creates the UI. Some hosts bizarrely display the class-name, so you might want to make it reflect your plugin. But the name must be "
                                     "UNIQUE to this exact version of your plugin, to avoid objective-C linkage mix-ups that happen when different plugins containing the same class-name are loaded simultaneously.");

        props.add (new TextPropertyComponent (getPluginRTASCategory (project), "Plugin RTAS Category", 64, false));
        props.getLast()->setTooltip ("(Leave this blank if your plugin is a synth). This is one of the RTAS categories from FicPluginEnums.h, such as: ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics, "
                                     "ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay, "
                                     "ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction, "
                                     "ePlugInCategory_Dither, ePlugInCategory_SoundField");
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeIsBundle = true;
        exporter.xcodeCreatePList = true;
        exporter.xcodePackageType = "TDMw";
        exporter.xcodeBundleSignature = "PTul";
        exporter.xcodeFileType = "wrapper.cfbundle";
        exporter.xcodeBundleExtension = ".component";
        exporter.xcodeProductType = "com.apple.product-type.bundle";
        exporter.xcodeProductInstallPath = "$(HOME)/Library/Audio/Plug-Ins/Components/";

        exporter.xcodeShellScriptTitle = "Copy to the different plugin folders";
        exporter.xcodeShellScript = String::fromUTF8 (BinaryData::AudioPluginXCodeScript_txt, BinaryData::AudioPluginXCodeScript_txtSize);

        exporter.xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                                      " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                      " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"";

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }
};

//==============================================================================
class ProjectType_BrowserPlugin  : public ProjectType
{
public:
    ProjectType_BrowserPlugin()  : ProjectType (getTypeName(), "Browser Plug-in") {}

    static const char* getTypeName() noexcept   { return "browserplug"; }
    bool isBrowserPlugin() const                { return true; }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeIsBundle = true;
        exporter.xcodeCreatePList = true;
        exporter.xcodeFileType = "wrapper.cfbundle";
        exporter.xcodeBundleExtension = ".plugin";
        exporter.xcodeProductType = "com.apple.product-type.bundle";
        exporter.xcodeProductInstallPath = "$(HOME)/Library/Internet Plug-Ins//";

        {
            XmlElement mimeTypesKey ("key");
            mimeTypesKey.setText ("WebPluginMIMETypes");

            XmlElement mimeTypesEntry ("dict");
            const String exeName (exporter.getProject().getProjectFilenameRoot().toLowerCase());
            mimeTypesEntry.createNewChildElement ("key")->setText ("application/" + exeName + "-plugin");
            XmlElement* d = mimeTypesEntry.createNewChildElement ("dict");
            d->createNewChildElement ("key")->setText ("WebPluginExtensions");
            d->createNewChildElement ("array")
               ->createNewChildElement ("string")->setText (exeName);
            d->createNewChildElement ("key")->setText ("WebPluginTypeDescription");
            d->createNewChildElement ("string")->setText (exporter.getProject().getProjectName().toString());

            exporter.xcodeExtraPListEntries.add (mimeTypesKey);
            exporter.xcodeExtraPListEntries.add (mimeTypesEntry);
        }

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }
};

//==============================================================================
static ProjectType_GUIApp       guiType;
static ProjectType_ConsoleApp   consoleType;
static ProjectType_Library      libraryType;
static ProjectType_AudioPlugin  audioPluginType;

//==============================================================================
const char* ProjectType::getGUIAppTypeName()        { return ProjectType_GUIApp::getTypeName(); }
const char* ProjectType::getConsoleAppTypeName()    { return ProjectType_ConsoleApp::getTypeName(); }
const char* ProjectType::getAudioPluginTypeName()   { return ProjectType_AudioPlugin::getTypeName(); }
