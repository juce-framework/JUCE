/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#ifndef __JUCER_PROJECT_JUCEHEADER__
#define __JUCER_PROJECT_JUCEHEADER__

#include "../../jucer_Headers.h"
class ProjectExporter;

//==============================================================================
class Project  : public FileBasedDocument,
                 public ValueTree::Listener
{
public:
    //==============================================================================
    Project (const File& file);
    ~Project();

    //==============================================================================
    // FileBasedDocument stuff..
    const String getDocumentTitle();
    const String loadDocument (const File& file);
    const String saveDocument (const File& file);
    const File getLastDocumentOpened();
    void setLastDocumentOpened (const File& file);

    void setTitle (const String& newTitle);

    //==============================================================================
    ValueTree getProjectRoot() const                    { return projectRoot; }
    Value getProjectName()                              { return getMainGroup().getName(); }
    const String getProjectFilenameRoot()               { return File::createLegalFileName (getDocumentTitle()); }
    const String getProjectUID() const                  { return projectRoot ["id"]; }

    //==============================================================================
    bool shouldBeAddedToBinaryResourcesByDefault (const File& file);
    const File resolveFilename (const String& filename) const;
    const String getRelativePathForFile (const File& file) const;

    //==============================================================================
    // Creates editors for the project settings
    void createPropertyEditors (Array <PropertyComponent*>& properties);

    //==============================================================================
    // project type info
    enum ProjectType
    {
        application     = 1, // must all be sequntial, starting from 1, so that combo boxes can update correctly
        commandLineApp  = 2,
        audioPlugin     = 3,
        library         = 4,
        browserPlugin   = 5
    };

    const StringArray getProjectTypes() const;
    Value getProjectType() const;

    bool isLibrary() const                              { return (int) getProjectType().getValue() == (int) library; }
    bool isGUIApplication() const                       { return (int) getProjectType().getValue() == (int) application; }
    bool isCommandLineApp() const                       { return (int) getProjectType().getValue() == (int) commandLineApp; }
    bool isAudioPlugin() const                          { return (int) getProjectType().getValue() == (int) audioPlugin; }
    bool isBrowserPlugin() const                        { return (int) getProjectType().getValue() == (int) browserPlugin; }

    Value getVersion() const                            { return getProjectValue ("version"); }
    Value getBundleIdentifier() const                   { return getProjectValue ("bundleIdentifier"); }
    void setBundleIdentifierToDefault()                 { getBundleIdentifier() = "com.yourcompany." + CodeFormatting::makeValidIdentifier (getProjectName().toString(), false, true, false); }

    //==============================================================================
    enum JuceLinkage
    {
        notLinkedToJuce = 1,   // must all be sequntial, starting from 1, so that combo boxes can update correctly
        useLinkedJuce = 2,
        useAmalgamatedJuce = 3,
        useAmalgamatedJuceViaSingleTemplate = 4,
        useAmalgamatedJuceViaMultipleTemplates = 5,
    };

    const StringArray getJuceLinkageModes() const;
    Value getJuceLinkageModeValue() const;
    JuceLinkage getJuceLinkageMode() const              { return (JuceLinkage) (int) getJuceLinkageModeValue().getValue(); }

    bool isUsingWrapperFiles() const                    { return isUsingFullyAmalgamatedFile() || isUsingSingleTemplateFile() || isUsingMultipleTemplateFiles(); }
    bool isUsingFullyAmalgamatedFile() const            { return getJuceLinkageMode() == useAmalgamatedJuce; }
    bool isUsingSingleTemplateFile() const              { return getJuceLinkageMode() == useAmalgamatedJuceViaSingleTemplate; }
    bool isUsingMultipleTemplateFiles() const           { return getJuceLinkageMode() == useAmalgamatedJuceViaMultipleTemplates; }

    //==============================================================================
    Value getProjectValue (const var::identifier& name) const       { return projectRoot.getPropertyAsValue (name, getUndoManagerFor (projectRoot)); }

    Value shouldBuildVST() const                        { return getProjectValue ("buildVST"); }
    Value shouldBuildRTAS() const                       { return getProjectValue ("buildRTAS"); }
    Value shouldBuildAU() const                         { return getProjectValue ("buildAU"); }
    bool shouldAddVSTFolderToPath()                     { return (isAudioPlugin() && (bool) shouldBuildVST().getValue()) || (int) getJuceConfigFlag ("JUCE_PLUGINHOST_VST").getValue() == 1; }

    Value getPluginName() const                         { return getProjectValue ("pluginName"); }
    Value getPluginDesc() const                         { return getProjectValue ("pluginDesc"); }
    Value getPluginManufacturer() const                 { return getProjectValue ("pluginManufacturer"); }
    Value getPluginManufacturerCode() const             { return getProjectValue ("pluginManufacturerCode"); }
    Value getPluginCode() const                         { return getProjectValue ("pluginCode"); }
    Value getPluginChannelConfigs() const               { return getProjectValue ("pluginChannelConfigs"); }
    Value getPluginIsSynth() const                      { return getProjectValue ("pluginIsSynth"); }
    Value getPluginWantsMidiInput() const               { return getProjectValue ("pluginWantsMidiIn"); }
    Value getPluginProducesMidiOut() const              { return getProjectValue ("pluginProducesMidiOut"); }
    Value getPluginSilenceInProducesSilenceOut() const  { return getProjectValue ("pluginSilenceInIsSilenceOut"); }
    Value getPluginTailLengthSeconds() const            { return getProjectValue ("pluginTailLength"); }
    Value getPluginEditorNeedsKeyFocus() const          { return getProjectValue ("pluginEditorRequiresKeys"); }
    Value getPluginAUExportPrefix() const               { return getProjectValue ("pluginAUExportPrefix"); }
    Value getPluginAUCocoaViewClassName() const         { return getProjectValue ("pluginAUViewClass"); }
    Value getPluginRTASCategory() const                 { return getProjectValue ("pluginRTASCategory"); }

    //==============================================================================
    const File getAppIncludeFile() const                { return getWrapperFolder().getChildFile (getJuceSourceHFilename()); }
    const File getWrapperFolder() const                 { return getFile().getSiblingFile ("JuceLibraryCode"); }
    const File getPluginCharacteristicsFile() const     { return getWrapperFolder().getChildFile (getPluginCharacteristicsFilename()); }

    //==============================================================================
    const String getAmalgamatedHeaderFileName() const       { return "juce_amalgamated.h"; }
    const String getAmalgamatedMMFileName() const           { return "juce_amalgamated.mm"; }
    const String getAmalgamatedCppFileName() const          { return "juce_amalgamated.cpp"; }

    const String getAppConfigFilename() const               { return "AppConfig.h"; }
    const String getJuceSourceFilenameRoot() const          { return "JuceLibraryCode"; }
    int getNumSeparateAmalgamatedFiles() const              { return 4; }
    const String getJuceSourceHFilename() const             { return "JuceHeader.h"; }
    const String getJuceCodeGroupName() const               { return "Juce Library Code"; }
    const String getPluginCharacteristicsFilename() const   { return "JucePluginCharacteristics.h"; }

    //==============================================================================
    class Item
    {
    public:
        //==============================================================================
        Item (Project& project, const ValueTree& itemNode);
        Item (const Item& other);
        ~Item();

        void initialiseNodeValues();

        //==============================================================================
        bool isValid() const                            { return node.isValid(); }
        const ValueTree& getNode() const throw()        { return node; }
        ValueTree& getNode() throw()                    { return node; }
        Project& getProject() const throw()             { return project; }
        bool operator== (const Item& other) const       { return node == other.node && &project == &other.project; }
        bool operator!= (const Item& other) const       { return ! operator== (other); }

        //==============================================================================
        bool isFile() const;
        bool isGroup() const;
        bool isMainGroup() const;

        const String getID() const;

        //==============================================================================
        Value getName() const;
        const File getFile() const;
        void setFile (const File& file);
        const File determineGroupFolder() const;
        bool renameFile (const File& newFile);

        bool shouldBeAddedToTargetProject() const;
        bool shouldBeCompiled() const;
        Value getShouldCompileValue() const;
        bool shouldBeAddedToBinaryResources() const;
        Value getShouldAddToResourceValue() const;

        //==============================================================================
        bool canContain (const Item& child) const;
        int getNumChildren() const                      { return node.getNumChildren(); }
        Item getChild (int index) const                 { return Item (project, node.getChild (index)); }
        void addChild (const Item& newChild, int insertIndex);
        bool addFile (const File& file, int insertIndex);
        void removeItemFromProject();
        void sortAlphabetically();
        Item findItemForFile (const File& file) const;

        Item getParent() const;

        Image* getIcon() const;

    private:
        //==============================================================================
        Project& project;
        ValueTree node;

        UndoManager* getUndoManager() const              { return project.getUndoManagerFor (node); }
        Item& operator= (const Item&);
    };

    Item getMainGroup();
    Item createNewGroup();
    Item createNewItem (const File& file);

    //==============================================================================
    class BuildConfiguration
    {
    public:
        BuildConfiguration (const BuildConfiguration&);
        const BuildConfiguration& operator= (const BuildConfiguration&);
        ~BuildConfiguration();

        //==============================================================================
        Project& getProject() const                         { return *project; }

        void createPropertyEditors (Array <PropertyComponent*>& properties);

        //==============================================================================
        Value getName() const                               { return config.getPropertyAsValue ("name", getUndoManager()); }
        Value isDebug() const                               { return config.getPropertyAsValue ("isDebug", getUndoManager()); }
        Value getTargetBinaryName() const                   { return config.getPropertyAsValue ("targetName", getUndoManager()); }
        // the path relative to the build folder in which the binary should go
        Value getTargetBinaryRelativePath() const           { return config.getPropertyAsValue ("binaryPath", getUndoManager()); }
        Value getOptimisationLevel() const                  { return config.getPropertyAsValue ("optimisation", getUndoManager()); }
        const String getGCCOptimisationFlag() const;
        Value getPreprocessorDefs() const                   { return config.getPropertyAsValue ("defines", getUndoManager()); }
        const StringArray parsePreprocessorDefs() const;
        Value getHeaderSearchPath() const                   { return config.getPropertyAsValue ("headerPath", getUndoManager()); }
        const StringArray getHeaderSearchPaths() const;
        Value getMacSDKVersion() const;
        Value getMacCompatibilityVersion() const;

        //==============================================================================
    private:
        friend class Project;
        Project* project;
        ValueTree config;

        UndoManager* getUndoManager() const                 { return project->getUndoManagerFor (config); }

        BuildConfiguration (Project* project, const ValueTree& configNode);
    };

    int getNumConfigurations() const;
    BuildConfiguration getConfiguration (int index);
    void addNewConfiguration (BuildConfiguration* configToCopy);
    void deleteConfiguration (int index);
    bool hasConfigurationNamed (const String& name) const;
    const String getUniqueConfigName (String name) const;

    //==============================================================================
    ValueTree getExporters();
    int getNumExporters();
    ProjectExporter* createExporter (int index);
    void addNewExporter (int exporterIndex);
    void deleteExporter (int index);
    void createDefaultExporters();

    //==============================================================================
    struct JuceConfigFlag
    {
        String symbol, description;
        Value value;   // 1 = true, 2 = false, anything else = use default
    };

    void getJuceConfigFlags (OwnedArray <JuceConfigFlag>& flags);
    Value getJuceConfigFlag (const String& name);

    //==============================================================================
    const String getFileTemplate (const String& templateName);

    //==============================================================================
    void valueTreePropertyChanged (ValueTree& tree, const var::identifier& property);
    void valueTreeChildrenChanged (ValueTree& tree);
    void valueTreeParentChanged (ValueTree& tree);

    //==============================================================================
    UndoManager* getUndoManagerFor (const ValueTree& node) const             { return 0; }

    //==============================================================================
    static const char* projectFileExtension;

private:
    ValueTree projectRoot;
    static File lastDocumentOpened;

    const File getLocalJuceFolder();
    void updateProjectSettings();
    void setMissingDefaultValues();
    ValueTree getConfigurations() const;
    void createDefaultConfigs();
    ValueTree getJuceConfigNode();

    Project (const Project&);
    const Project& operator= (const Project&);
};


#endif   // __JUCER_PROJECT_JUCEHEADER__
