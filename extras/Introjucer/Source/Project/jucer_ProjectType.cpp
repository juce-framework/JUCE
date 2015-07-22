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

#include "jucer_ProjectType.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "../Project Saving/jucer_ProjectSaver.h"
#include "jucer_AudioPluginModule.h"


//==============================================================================
ProjectType::ProjectType (const String& t, const String& d)
    : type (t), desc (d)
{
    getAllTypes().add (this);
}

ProjectType::~ProjectType()
{
    getAllTypes().removeFirstMatchingValue (this);
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

    void createPropertyEditors (Project&, PropertyListBuilder&) const
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

    void createPropertyEditors (Project&, PropertyListBuilder&) const
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
class ProjectType_StaticLibrary  : public ProjectType
{
public:
    ProjectType_StaticLibrary()  : ProjectType (getTypeName(), "Static Library") {}

    static const char* getTypeName() noexcept   { return "library"; }
    bool isStaticLibrary() const                { return true; }

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (Project&, PropertyListBuilder&) const
    {
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "archive.ar";
        exporter.xcodeProductType = "com.apple.product-type.library.static";
        exporter.xcodeProductInstallPath = String::empty;
        exporter.makefileTargetSuffix = ".a";
        exporter.msvcTargetSuffix = ".lib";
        exporter.msvcExtraPreprocessorDefs.set ("_LIB", "");
    }
};

//==============================================================================
class ProjectType_DLL  : public ProjectType
{
public:
    ProjectType_DLL()  : ProjectType (getTypeName(), "Dynamic Library") {}

    static const char* getTypeName() noexcept   { return "dll"; }
    bool isDynamicLibrary() const               { return true; }

    void setMissingProjectProperties (Project&) const
    {
    }

    void createPropertyEditors (Project&, PropertyListBuilder&) const
    {
    }

    void prepareExporter (ProjectExporter& exporter) const
    {
        exporter.xcodeCreatePList = false;
        exporter.xcodeFileType = "compiled.mach-o.dylib";
        exporter.xcodeProductType = "com.apple.product-type.library.dynamic";
        exporter.xcodeBundleExtension = ".dylib";
        exporter.xcodeProductInstallPath = String::empty;
        exporter.makefileTargetSuffix = ".so";
        exporter.msvcTargetSuffix = ".dll";
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
        const String sanitisedProjectName (CodeHelpers::makeValidIdentifier (project.getTitle(), false, true, false));

        setValueIfVoid (shouldBuildVST (project), true);
        setValueIfVoid (shouldBuildVST3 (project), false);
        setValueIfVoid (shouldBuildAU (project),  true);
        setValueIfVoid (shouldBuildRTAS (project), false);
        setValueIfVoid (shouldBuildAAX (project), false);

        setValueIfVoid (getPluginName (project),                   project.getTitle());
        setValueIfVoid (getPluginDesc (project),                   project.getTitle());
        setValueIfVoid (getPluginManufacturer (project),           "yourcompany");
        setValueIfVoid (getPluginManufacturerCode (project),       "Manu");
        setValueIfVoid (getPluginCode (project),                   makeValid4CC (project.getProjectUID() + project.getProjectUID()));
        setValueIfVoid (getPluginChannelConfigs (project),         "{1, 1}, {2, 2}");
        setValueIfVoid (getPluginIsSynth (project),                false);
        setValueIfVoid (getPluginWantsMidiInput (project),         false);
        setValueIfVoid (getPluginProducesMidiOut (project),        false);
        setValueIfVoid (getPluginSilenceInProducesSilenceOut (project), false);
        setValueIfVoid (getPluginEditorNeedsKeyFocus (project),    false);
        setValueIfVoid (getPluginAUExportPrefix (project),         sanitisedProjectName + "AU");
        setValueIfVoid (getPluginRTASCategory (project),           String::empty);
        setValueIfVoid (project.getBundleIdentifier(),             project.getDefaultBundleIdentifier());
        setValueIfVoid (project.getAAXIdentifier(),                project.getDefaultAAXIdentifier());
        setValueIfVoid (getPluginAAXCategory (project),            "AAX_ePlugInCategory_Dynamics");
    }

    void createPropertyEditors (Project& project, PropertyListBuilder& props) const
    {
        props.add (new BooleanPropertyComponent (shouldBuildVST (project), "Build VST", "Enabled"),
                   "Whether the project should produce a VST plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildVST3 (project), "Build VST3", "Enabled"),
                   "Whether the project should produce a VST3 plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildAU (project), "Build AudioUnit", "Enabled"),
                   "Whether the project should produce an AudioUnit plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildRTAS (project), "Build RTAS", "Enabled"),
                   "Whether the project should produce an RTAS plugin.");
        props.add (new BooleanPropertyComponent (shouldBuildAAX (project), "Build AAX", "Enabled"),
                   "Whether the project should produce an AAX plugin.");

        props.add (new TextPropertyComponent (getPluginName (project), "Plugin Name", 128, false),
                   "The name of your plugin (keep it short!)");
        props.add (new TextPropertyComponent (getPluginDesc (project), "Plugin Description", 256, false),
                   "A short description of your plugin.");

        props.add (new TextPropertyComponent (getPluginManufacturer (project), "Plugin Manufacturer", 256, false),
                   "The name of your company (cannot be blank).");
        props.add (new TextPropertyComponent (getPluginManufacturerCode (project), "Plugin Manufacturer Code", 4, false),
                   "A four-character unique ID for your company. Note that for AU compatibility, this must contain at least one upper-case letter!");
        props.add (new TextPropertyComponent (getPluginCode (project), "Plugin Code", 4, false),
                   "A four-character unique ID for your plugin. Note that for AU compatibility, this must contain at least one upper-case letter!");

        props.add (new TextPropertyComponent (getPluginChannelConfigs (project), "Plugin Channel Configurations", 1024, false),
                   "This is the set of input/output channel configurations that your plugin can handle.  The list is a comma-separated set of pairs of values in the form { numInputs, numOutputs }, and each "
                   "pair indicates a valid configuration that the plugin can handle. So for example, {1, 1}, {2, 2} means that the plugin can be used in just two configurations: either with 1 input "
                   "and 1 output, or with 2 inputs and 2 outputs.");

        props.add (new BooleanPropertyComponent (getPluginIsSynth (project), "Plugin is a Synth", "Is a Synth"),
                   "Enable this if you want your plugin to be treated as a synth or generator. It doesn't make much difference to the plugin itself, but some hosts treat synths differently to other plugins.");

        props.add (new BooleanPropertyComponent (getPluginWantsMidiInput (project), "Plugin Midi Input", "Plugin wants midi input"),
                   "Enable this if you want your plugin to accept midi messages.");

        props.add (new BooleanPropertyComponent (getPluginProducesMidiOut (project), "Plugin Midi Output", "Plugin produces midi output"),
                   "Enable this if your plugin is going to produce midi messages.");

        props.add (new BooleanPropertyComponent (getPluginSilenceInProducesSilenceOut (project), "Silence", "Silence in produces silence out"),
                   "Enable this if your plugin has no tail - i.e. if passing a silent buffer to it will always result in a silent buffer being produced.");

        props.add (new BooleanPropertyComponent (getPluginEditorNeedsKeyFocus (project), "Key Focus", "Plugin editor requires keyboard focus"),
                   "Enable this if your plugin needs keyboard input - some hosts can be a bit funny about keyboard focus..");

        props.add (new TextPropertyComponent (getPluginAUSDKLocation (project), "Plugin AU SDK Path", 512, false),
                   "An optional path to the Apple AudioUnit SDK's 'CoreAudio' folder. Leave this blank to use the default location.");

        props.add (new TextPropertyComponent (getPluginAUExportPrefix (project), "Plugin AU Export Prefix", 64, false),
                   "A prefix for the names of exported entry-point functions that the component exposes - typically this will be a version of your plugin's name that can be used as part of a C++ token.");

        props.add (new TextPropertyComponent (getPluginAUMainType (project), "Plugin AU Main Type", 128, false),
                   "In an AU, this is the value that is set as JucePlugin_AUMainType. Leave it blank unless you want to use a custom value.");

        props.add (new TextPropertyComponent (getPluginVSTCategory (project), "VST Category", 64, false),
                   "In a VST, this is the value that is set as JucePlugin_VSTCategory. Leave it blank unless you want to use a custom value.");

        props.add (new TextPropertyComponent (getPluginRTASCategory (project), "Plugin RTAS Category", 64, false),
                   "(Leave this blank if your plugin is a synth). This is one of the RTAS categories from FicPluginEnums.h, such as: ePlugInCategory_None, ePlugInCategory_EQ, ePlugInCategory_Dynamics, "
                   "ePlugInCategory_PitchShift, ePlugInCategory_Reverb, ePlugInCategory_Delay, "
                   "ePlugInCategory_Modulation, ePlugInCategory_Harmonic, ePlugInCategory_NoiseReduction, "
                   "ePlugInCategory_Dither, ePlugInCategory_SoundField");

        props.add (new TextPropertyComponent (getPluginAAXCategory (project), "Plugin AAX Category", 64, false),
                   "This is one of the categories from the AAX_EPlugInCategory enum");

        props.add (new TextPropertyComponent (project.getAAXIdentifier(), "Plugin AAX Identifier", 256, false),
                   "The value to use for the JucePlugin_AAXIdentifier setting");
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

        exporter.xcodeOtherRezFlags = "-d ppc_$ppc -d i386_$i386 -d ppc64_$ppc64 -d x86_64_$x86_64"
                                      " -I /System/Library/Frameworks/CoreServices.framework/Frameworks/CarbonCore.framework/Versions/A/Headers"
                                      " -I \\\"$(DEVELOPER_DIR)/Extras/CoreAudio/AudioUnits/AUPublic/AUBase\\\"";

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }

    static String makeValid4CC (const String& seed)
    {
        String s (CodeHelpers::makeValidIdentifier (seed, false, true, false) + "xxxx");

        return s.substring (0, 1).toUpperCase()
             + s.substring (1, 4).toLowerCase();
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
            d->createNewChildElement ("string")->setText (exporter.getProject().getTitle());

            exporter.xcodeExtraPListEntries.add (mimeTypesKey);
            exporter.xcodeExtraPListEntries.add (mimeTypesEntry);
        }

        exporter.msvcTargetSuffix = ".dll";
        exporter.msvcIsDLL = true;

        exporter.makefileIsDLL = true;
    }
};

//==============================================================================
static ProjectType_GUIApp        guiType;
static ProjectType_ConsoleApp    consoleType;
static ProjectType_StaticLibrary libraryType;
static ProjectType_DLL           dllType;
static ProjectType_AudioPlugin   audioPluginType;

//==============================================================================
const char* ProjectType::getGUIAppTypeName()        { return ProjectType_GUIApp::getTypeName(); }
const char* ProjectType::getConsoleAppTypeName()    { return ProjectType_ConsoleApp::getTypeName(); }
const char* ProjectType::getStaticLibTypeName()     { return ProjectType_StaticLibrary::getTypeName(); }
const char* ProjectType::getDynamicLibTypeName()    { return ProjectType_DLL::getTypeName(); }
const char* ProjectType::getAudioPluginTypeName()   { return ProjectType_AudioPlugin::getTypeName(); }
