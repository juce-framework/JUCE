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

#include "../jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "../Project Saving/jucer_ProjectExporter.h"
#include "projucer_MessageIDs.h"
#include "projucer_CppHelpers.h"
#include "projucer_SourceCodeRange.h"
#include "projucer_ClassDatabase.h"
#include "projucer_DiagnosticMessage.h"
#include "projucer_ProjectBuildInfo.h"
#include "projucer_ClientServerMessages.h"
#include "projucer_CompileEngineClient.h"
#include "../LiveBuildEngine/projucer_CompileEngineServer.h"

#ifndef RUN_CLANG_IN_CHILD_PROCESS
 #error
#endif


//==============================================================================
namespace ProjectProperties
{
    const Identifier liveSettingsType ("LIVE_SETTINGS");
   #if JUCE_MAC
    const Identifier liveSettingsSubtype ("OSX");
   #elif JUCE_WINDOWS
    const Identifier liveSettingsSubtype ("WINDOWS");
   #elif JUCE_LINUX
    const Identifier liveSettingsSubtype ("LINUX");
   #endif

    static ValueTree getLiveSettings (Project& project)
    {
        return project.getProjectRoot().getOrCreateChildWithName (liveSettingsType, nullptr)
                                       .getOrCreateChildWithName (liveSettingsSubtype, nullptr);
    }

    static const ValueTree getLiveSettingsConst (Project& project)
    {
        return project.getProjectRoot().getChildWithName (liveSettingsType)
                                       .getChildWithName (liveSettingsSubtype);
    }

    static Value getLiveSetting    (Project& p, const Identifier& i)  { return getLiveSettings (p).getPropertyAsValue (i, p.getUndoManagerFor (getLiveSettings (p))); }
    static var   getLiveSettingVar (Project& p, const Identifier& i)  { return getLiveSettingsConst (p) [i]; }

    static Value  getUserHeaderPathValue (Project& p)          { return getLiveSetting    (p, Ids::headerPath); }
    static String getUserHeaderPathString (Project& p)         { return getLiveSettingVar (p, Ids::headerPath); }
    static Value  getSystemHeaderPathValue (Project& p)        { return getLiveSetting    (p, Ids::systemHeaderPath); }
    static String getSystemHeaderPathString (Project& p)       { return getLiveSettingVar (p, Ids::systemHeaderPath); }
    static Value  getExtraDLLsValue (Project& p)               { return getLiveSetting    (p, Ids::extraDLLs); }
    static String getExtraDLLsString (Project& p)              { return getLiveSettingVar (p, Ids::extraDLLs); }
    static Value  getExtraCompilerFlagsValue (Project& p)      { return getLiveSetting    (p, Ids::extraCompilerFlags); }
    static String getExtraCompilerFlagsString (Project& p)     { return getLiveSettingVar (p, Ids::extraCompilerFlags); }
    static Value  getExtraPreprocessorDefsValue (Project& p)   { return getLiveSetting    (p, Ids::defines); }
    static String getExtraPreprocessorDefsString (Project& p)  { return getLiveSettingVar (p, Ids::defines); }

    static File getProjucerTempFolder()
    {
       #if JUCE_MAC
        return File ("~/Library/Caches/com.juce.projucer");
       #else
        return File::getSpecialLocation (File::tempDirectory).getChildFile ("com.juce.projucer");
       #endif
    }

    static File getCacheLocation (Project& project)
    {
        String cacheFolderName = project.getProjectFilenameRoot() + "_" + project.getProjectUID();

       #if JUCE_DEBUG
        cacheFolderName += "_debug";
       #endif

        return getProjucerTempFolder()
                .getChildFile ("Intermediate Files")
                .getChildFile (cacheFolderName);
    }
}

//==============================================================================
void LiveBuildProjectSettings::getLiveSettings (Project& project, PropertyListBuilder& props)
{
    using namespace ProjectProperties;

    props.addSearchPathProperty (getUserHeaderPathValue (project), "User header paths", "User header search paths.");
    props.addSearchPathProperty (getSystemHeaderPathValue (project), "System header paths", "System header search paths.");

    props.add (new TextPropertyComponent (getExtraPreprocessorDefsValue (project), "Preprocessor Definitions", 32768, true),
               "Extra preprocessor definitions. Use the form \"NAME1=value NAME2=value\", using whitespace or commas "
               "to separate the items - to include a space or comma in a definition, precede it with a backslash.");

    props.add (new TextPropertyComponent (getExtraCompilerFlagsValue (project), "Extra compiler flags", 2048, true),
               "Extra command-line flags to be passed to the compiler. This string can contain references to preprocessor"
               " definitions in the form ${NAME_OF_DEFINITION}, which will be replaced with their values.");

    props.add (new TextPropertyComponent (getExtraDLLsValue (project), "Extra dynamic libraries", 2048, true),
               "Extra dynamic libs that the running code may require. Use new-lines or commas to separate the items");
}

void LiveBuildProjectSettings::updateNewlyOpenedProject (Project&) { /* placeholder */ }

bool LiveBuildProjectSettings::isBuildDisabled (Project& p)
{
    const bool defaultBuildDisabled = true;
    return p.getStoredProperties().getBoolValue ("buildDisabled", defaultBuildDisabled);
}

void LiveBuildProjectSettings::setBuildDisabled (Project& p, bool b)    { p.getStoredProperties().setValue ("buildDisabled", b); }
bool LiveBuildProjectSettings::areWarningsDisabled (Project& p)         { return p.getStoredProperties().getBoolValue ("warningsDisabled"); }
void LiveBuildProjectSettings::setWarningsDisabled (Project& p, bool b) { p.getStoredProperties().setValue ("warningsDisabled", b); }

//==============================================================================
class ClientIPC  : public MessageHandler,
                   private InterprocessConnection,
                   private Timer
{
public:
    ClientIPC (CompileEngineChildProcess& cp)
       : InterprocessConnection (true), owner (cp)
    {
        launchServer();
    }

    ~ClientIPC()
    {
       #if RUN_CLANG_IN_CHILD_PROCESS
        if (childProcess.isRunning())
        {
           #if JUCE_DEBUG
            killServerPolitely();
           #else
            // in release builds we don't want to wait
            // for the server to clean up and shut down
            killServerWithoutMercy();
           #endif
        }
       #endif
    }

    void launchServer()
    {
        DBG ("Client: Launching Server...");
        const String pipeName ("ipc_" + String::toHexString (Random().nextInt64()));

        const String command (createCommandLineForLaunchingServer (pipeName,
                                                                   owner.project.getProjectUID(),
                                                                   ProjectProperties::getCacheLocation (owner.project)));

       #if RUN_CLANG_IN_CHILD_PROCESS
        if (! childProcess.start (command))
        {
            jassertfalse;
        }
       #else
        server = createClangServer (command);
       #endif

        bool ok = connectToPipe (pipeName, 10000);
        jassert (ok);

        if (ok)
            MessageTypes::sendPing (*this);

        startTimer (serverKeepAliveTimeout);
    }

    void killServerPolitely()
    {
        DBG ("Client: Killing Server...");
        MessageTypes::sendQuit (*this);

        disconnect();
        stopTimer();

       #if RUN_CLANG_IN_CHILD_PROCESS
        childProcess.waitForProcessToFinish (5000);
       #endif

        killServerWithoutMercy();
    }

    void killServerWithoutMercy()
    {
        disconnect();
        stopTimer();

       #if RUN_CLANG_IN_CHILD_PROCESS
        childProcess.kill();
       #else
        destroyClangServer (server);
        server = nullptr;
       #endif
    }

    void connectionMade()
    {
        DBG ("Client: connected");
        stopTimer();
    }

    void connectionLost()
    {
        DBG ("Client: disconnected");
        startTimer (100);
    }

    bool sendMessage (const ValueTree& m)
    {
        return InterprocessConnection::sendMessage (MessageHandler::convertMessage (m));
    }

    void messageReceived (const MemoryBlock& message)
    {
       #if RUN_CLANG_IN_CHILD_PROCESS
        startTimer (serverKeepAliveTimeout);
       #else
        stopTimer();
       #endif
        MessageTypes::dispatchToClient (owner, MessageHandler::convertMessage (message));
    }

    enum { serverKeepAliveTimeout = 10000 };

private:
    CompileEngineChildProcess& owner;

   #if RUN_CLANG_IN_CHILD_PROCESS
    ChildProcess childProcess;
   #else
    void* server;
   #endif

    void timerCallback()    { owner.handleCrash (String()); }
};

//==============================================================================
class CompileEngineChildProcess::ChildProcess    : private ValueTree::Listener,
                                                   private Timer
{
public:
    ChildProcess (CompileEngineChildProcess& proc, Project& p)
        : owner (proc), project (p)
    {
        projectRoot = project.getProjectRoot();

        restartServer();
        projectRoot.addListener (this);
        openedOk = true;
    }

    ~ChildProcess()
    {
        projectRoot.removeListener (this);

        if (isRunningApp && server != nullptr)
            server->killServerWithoutMercy();

        server = nullptr;
    }

    void restartServer()
    {
        server = nullptr;
        server = new ClientIPC (owner);
        sendRebuild();
    }

    void sendRebuild()
    {
        stopTimer();

        ProjectBuildInfo build;

        if (! doesProjectMatchSavedHeaderState (project))
        {
            MessageTypes::sendNewBuild (*server, build);

            owner.errorList.resetToError ("Project structure does not match the saved headers! "
                                          "Please re-save your project to enable compilation");
            return;
        }

        if (areAnyModulesMissing (project))
        {
            MessageTypes::sendNewBuild (*server, build);

            owner.errorList.resetToError ("Some of your JUCE modules can't be found! "
                                          "Please check that all the module paths are correct");
            return;
        }

        build.setSystemIncludes (getSystemIncludePaths());
        build.setUserIncludes (getUserIncludes());

        build.setGlobalDefs (getGlobalDefs (project));
        build.setCompileFlags (ProjectProperties::getExtraCompilerFlagsString (project).trim());
        build.setExtraDLLs (getExtraDLLs());
        build.setJuceModulesFolder (EnabledModuleList::findDefaultModulesFolder (project).getFullPathName());

        build.setUtilsCppInclude (project.getAppIncludeFile().getFullPathName());

        scanForProjectFiles (project, build);

        owner.updateAllEditors();

        MessageTypes::sendNewBuild (*server, build);
    }

    void cleanAll()
    {
        MessageTypes::sendCleanAll (*server);
        sendRebuild();
    }

    void reinstantiatePreviews()
    {
        MessageTypes::sendReinstantiate (*server);
    }

    bool launchApp()
    {
        MessageTypes::sendLaunchApp (*server);
        return true;
    }

    ScopedPointer<ClientIPC> server;

    bool openedOk = false;
    bool isRunningApp = false;

private:
    CompileEngineChildProcess& owner;
    Project& project;
    ValueTree projectRoot;

    void projectStructureChanged()
    {
        startTimer (100);
    }

    void timerCallback() override
    {
        sendRebuild();
    }

    void valueTreePropertyChanged (ValueTree&, const Identifier&) override       { projectStructureChanged(); }
    void valueTreeChildAdded (ValueTree&, ValueTree&) override                   { projectStructureChanged(); }
    void valueTreeChildRemoved (ValueTree&, ValueTree&, int) override            { projectStructureChanged(); }
    void valueTreeParentChanged (ValueTree&) override                            { projectStructureChanged(); }
    void valueTreeChildOrderChanged (ValueTree&, int, int) override              {}

    static String getGlobalDefs (Project& proj)
    {
        String defs (ProjectProperties::getExtraPreprocessorDefsString (proj));

        for (Project::ExporterIterator exporter (proj); exporter.next();)
            if (exporter->canLaunchProject())
                defs << " " << exporter->getExporterIdentifierMacro() << "=1";

        return defs;
    }

    static void scanProjectItem (const Project::Item& projectItem, Array<File>& compileUnits, Array<File>& userFiles)
    {
        if (projectItem.isGroup())
        {
            for (int i = 0; i < projectItem.getNumChildren(); ++i)
                scanProjectItem (projectItem.getChild(i), compileUnits, userFiles);

            return;
        }

        if (projectItem.shouldBeCompiled())
        {
            const File f (projectItem.getFile());

            if (f.exists())
                compileUnits.add (f);
        }

        if (projectItem.shouldBeAddedToTargetProject() && ! projectItem.shouldBeAddedToBinaryResources())
        {
            const File f (projectItem.getFile());

            if (f.exists())
                userFiles.add (f);
        }
    }

    void scanForProjectFiles (Project& proj, ProjectBuildInfo& build)
    {
        Array<File> compileUnits, userFiles;
        scanProjectItem (proj.getMainGroup(), compileUnits, userFiles);

        {
            OwnedArray<LibraryModule> modules;
            proj.getModules().createRequiredModules (modules);

            for (Project::ExporterIterator exporter (proj); exporter.next();)
            {
                if (exporter->canLaunchProject())
                {
                    for (const LibraryModule* m : modules)
                    {
                        const File localModuleFolder = proj.getModules().shouldCopyModuleFilesLocally (m->moduleInfo.getID()).getValue()
                                                          ? proj.getLocalModuleFolder (m->moduleInfo.getID())
                                                          : m->moduleInfo.getFolder();

                        m->findAndAddCompiledUnits (*exporter, nullptr, compileUnits);
                    }

                    break;
                }
            }
        }

        for (int i = 0; ; ++i)
        {
            const File binaryDataCpp (proj.getBinaryDataCppFile (i));
            if (! binaryDataCpp.exists())
                break;

            compileUnits.add (binaryDataCpp);
        }

        for (int i = compileUnits.size(); --i >= 0;)
            if (compileUnits.getReference(i).hasFileExtension (".r"))
                compileUnits.remove (i);

        build.setFiles (compileUnits, userFiles);
    }

    static bool doesProjectMatchSavedHeaderState (Project& project)
    {
        ValueTree liveModules (project.getProjectRoot().getChildWithName (Ids::MODULES));

        ScopedPointer<XmlElement> xml (XmlDocument::parse (project.getFile()));

        if (xml == nullptr || ! xml->hasTagName (Ids::JUCERPROJECT.toString()))
            return false;

        ValueTree diskModules (ValueTree::fromXml (*xml).getChildWithName (Ids::MODULES));

        return liveModules.isEquivalentTo (diskModules);
    }

    static bool areAnyModulesMissing (Project& project)
    {
        OwnedArray<LibraryModule> modules;
        project.getModules().createRequiredModules (modules);

        for (auto* module : modules)
            if (! module->getFolder().isDirectory())
                return true;

        return false;
    }

    StringArray getUserIncludes()
    {
        StringArray paths;
        paths.add (project.getGeneratedCodeFolder().getFullPathName());
        paths.addArray (getSearchPathsFromString (ProjectProperties::getUserHeaderPathString (project)));
        return convertSearchPathsToAbsolute (paths);
    }

    StringArray getSystemIncludePaths()
    {
        StringArray paths;
        paths.addArray (getSearchPathsFromString (ProjectProperties::getSystemHeaderPathString (project)));

        if (project.getProjectType().isAudioPlugin())
        {
            paths.add (getAppSettings().getGlobalPath (Ids::vst3Path, TargetOS::getThisOS()).toString());
        }

        OwnedArray<LibraryModule> modules;
        project.getModules().createRequiredModules (modules);

        for (auto* module : modules)
            paths.addIfNotAlreadyThere (module->getFolder().getParentDirectory().getFullPathName());

        return convertSearchPathsToAbsolute (paths);
    }

    StringArray convertSearchPathsToAbsolute (const StringArray& paths) const
    {
        StringArray s;
        const File root (project.getProjectFolder());

        for (String p : paths)
            s.add (root.getChildFile (p).getFullPathName());

        return s;
    }

    StringArray getExtraDLLs()
    {
        StringArray dlls;
        dlls.addTokens (ProjectProperties::getExtraDLLsString (project), "\n\r,", StringRef());
        dlls.trim();
        dlls.removeEmptyStrings();
        return dlls;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcess)
};

//==============================================================================
CompileEngineChildProcess::CompileEngineChildProcess (Project& p)
    : project (p),
      continuousRebuild (false)
{
    ProjucerApplication::getApp().openDocumentManager.addListener (this);

    createProcess();

    errorList.setWarningsEnabled (! LiveBuildProjectSettings::areWarningsDisabled (project));
}

CompileEngineChildProcess::~CompileEngineChildProcess()
{
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);

    process = nullptr;
    lastComponentList.clear();
}

void CompileEngineChildProcess::createProcess()
{
    jassert (process == nullptr);
    process = new ChildProcess (*this, project);

    if (! process->openedOk)
        process = nullptr;

    updateAllEditors();
}

void CompileEngineChildProcess::cleanAll()
{
    if (process != nullptr)
        process->cleanAll();
}

void CompileEngineChildProcess::openPreview (const ClassDatabase::Class& comp)
{
    if (process != nullptr)
    {
        MainWindow* projectWindow = nullptr;
        OwnedArray<MainWindow>& windows = ProjucerApplication::getApp().mainWindowList.windows;

        for (int i = 0; i < windows.size(); ++i)
        {
            if (MainWindow* w = windows[i])
            {
                if (w->getProject() == &project)
                {
                    projectWindow = w;
                    break;
                }
            }
        }

        Rectangle<int> mainWindowRect;

        if (projectWindow != nullptr)
            mainWindowRect = projectWindow->getBounds();

        MessageTypes::sendOpenPreview (*process->server, comp, mainWindowRect);
    }
}

void CompileEngineChildProcess::reinstantiatePreviews()
{
    if (process != nullptr)
        process->reinstantiatePreviews();
}

void CompileEngineChildProcess::processActivationChanged (bool isForeground)
{
    if (process != nullptr)
        MessageTypes::sendProcessActivationState (*process->server, isForeground);
}

//==============================================================================
bool CompileEngineChildProcess::canLaunchApp() const
{
    return process != nullptr
            && runningAppProcess == nullptr
            && activityList.getNumActivities() == 0
            && errorList.getNumErrors() == 0
            && project.getProjectType().isGUIApplication();
}

void CompileEngineChildProcess::launchApp()
{
    if (process != nullptr)
        process->launchApp();
}

bool CompileEngineChildProcess::canKillApp() const
{
    return runningAppProcess != nullptr;
}

void CompileEngineChildProcess::killApp()
{
    runningAppProcess = nullptr;
}

void CompileEngineChildProcess::handleAppLaunched()
{
    runningAppProcess = process;
    runningAppProcess->isRunningApp = true;
    createProcess();
}

void CompileEngineChildProcess::handleAppQuit()
{
    DBG ("handleAppQuit");
    runningAppProcess = nullptr;
}

//==============================================================================
struct CompileEngineChildProcess::Editor  : private CodeDocument::Listener,
                                            private Timer
{
    Editor (CompileEngineChildProcess& ccp, const File& f, CodeDocument& doc)
        : owner (ccp), file (f), document (doc), transactionTimer (doc)
    {
        sendFullUpdate();
        document.addListener (this);
    }

    ~Editor()
    {
        document.removeListener (this);
    }

    void codeDocumentTextInserted (const String& newText, int insertIndex) override
    {
        CodeChange (Range<int> (insertIndex, insertIndex), newText).addToList (pendingChanges);
        startEditorChangeTimer();
        transactionTimer.stopTimer();

        owner.lastComponentList.globalNamespace
           .nudgeAllCodeRanges (file.getFullPathName(), insertIndex, newText.length());
    }

    void codeDocumentTextDeleted (int start, int end) override
    {
        CodeChange (Range<int> (start, end), String()).addToList (pendingChanges);
        startEditorChangeTimer();
        transactionTimer.stopTimer();

        owner.lastComponentList.globalNamespace
           .nudgeAllCodeRanges (file.getFullPathName(), start, start - end);
    }

    void sendFullUpdate()
    {
        reset();

        if (owner.process != nullptr)
            MessageTypes::sendFileContentFullUpdate (*owner.process->server, file, document.getAllContent());
    }

    bool flushEditorChanges()
    {
        if (pendingChanges.size() > 0)
        {
            if (owner.process != nullptr && owner.process->server != nullptr)
                MessageTypes::sendFileChanges (*owner.process->server, pendingChanges, file);

            reset();
            return true;
        }

        stopTimer();
        return false;
    }

    void reset()
    {
        stopTimer();
        pendingChanges.clear();
    }

    void startTransactionTimer()
    {
        transactionTimer.startTimer (1000);
    }

    void startEditorChangeTimer()
    {
        startTimer (200);
    }

    CompileEngineChildProcess& owner;
    File file;
    CodeDocument& document;

private:
    Array<CodeChange> pendingChanges;

    void timerCallback() override
    {
        if (owner.continuousRebuild)
            flushEditorChanges();
        else
            stopTimer();
    }

    struct TransactionTimer   : public Timer
    {
        TransactionTimer (CodeDocument& doc) : document (doc) {}

        void timerCallback() override
        {
            stopTimer();
            document.newTransaction();
        }

        CodeDocument& document;
    };

    TransactionTimer transactionTimer;
};

void CompileEngineChildProcess::editorOpened (const File& file, CodeDocument& document)
{
    editors.add (new Editor (*this, file, document));
}

bool CompileEngineChildProcess::documentAboutToClose (OpenDocumentManager::Document* document)
{
    for (int i = editors.size(); --i >= 0;)
    {
        if (document->getFile() == editors.getUnchecked(i)->file)
        {
            const File f (editors.getUnchecked(i)->file);
            editors.remove (i);

            if (process != nullptr)
                MessageTypes::sendHandleFileReset (*process->server, f);
        }
    }

    return true;
}

void CompileEngineChildProcess::updateAllEditors()
{
    for (int i = editors.size(); --i >= 0;)
        editors.getUnchecked(i)->sendFullUpdate();
}

//==============================================================================
void CompileEngineChildProcess::handleCrash (const String& message)
{
    Logger::writeToLog ("*** Child process crashed: " + message);

    if (crashHandler != nullptr)
        crashHandler (message);
}

void CompileEngineChildProcess::handleNewDiagnosticList (const ValueTree& l)         { errorList.setList (l); }
void CompileEngineChildProcess::handleActivityListChanged (const StringArray& l)     { activityList.setList (l); }

void CompileEngineChildProcess::handleCloseIDE()
{
    if (JUCEApplication* app = JUCEApplication::getInstance())
        app->systemRequestedQuit();
}

void CompileEngineChildProcess::handleMissingSystemHeaders()
{
    if (ProjectContentComponent* p = findProjectContentComponent())
        p->handleMissingSystemHeaders();
}

void CompileEngineChildProcess::handleKeyPress (const String& className, const KeyPress& key)
{
    ApplicationCommandManager& commandManager = ProjucerApplication::getCommandManager();

    CommandID command = commandManager.getKeyMappings()->findCommandForKeyPress (key);

    if (command == StandardApplicationCommandIDs::undo)
    {
        handleUndoInEditor (className);
    }
    else if (command == StandardApplicationCommandIDs::redo)
    {
        handleRedoInEditor (className);
    }
    else if (ApplicationCommandTarget* const target = ApplicationCommandManager::findTargetForComponent (findProjectContentComponent()))
    {
        commandManager.setFirstCommandTarget (target);
        commandManager.getKeyMappings()->keyPressed (key, findProjectContentComponent());
        commandManager.setFirstCommandTarget (nullptr);
    }
}

void CompileEngineChildProcess::handleUndoInEditor (const String& /*className*/)
{
}

void CompileEngineChildProcess::handleRedoInEditor (const String& /*className*/)
{
}

void CompileEngineChildProcess::handleClassListChanged (const ValueTree& newList)
{
    lastComponentList = ClassDatabase::ClassList::fromValueTree (newList);
    activityList.sendClassListChangedMessage (lastComponentList);
}

void CompileEngineChildProcess::handleBuildFailed()
{
    if (errorList.getNumErrors() > 0)
        ProjucerApplication::getCommandManager().invokeDirectly (CommandIDs::showBuildTab, true);

    ProjucerApplication::getCommandManager().commandStatusChanged();
}

void CompileEngineChildProcess::handleChangeCode (const SourceCodeRange& location, const String& newText)
{
    if (Editor* ed = getOrOpenEditorFor (location.file))
    {
        if (ed->flushEditorChanges())
            return; // client-side editor changes were pending, so deal with them first, and discard
                    // the incoming change, whose position may now be wrong.

        ed->document.deleteSection (location.range.getStart(), location.range.getEnd());
        ed->document.insertText (location.range.getStart(), newText);

        // deliberately clear the messages that we just added, to avoid these changes being
        // sent to the server (which will already have processed the same ones locally)
        ed->reset();
        ed->startTransactionTimer();
    }
}

void CompileEngineChildProcess::handlePing()
{
}

//==============================================================================
void CompileEngineChildProcess::setContinuousRebuild (bool b)
{
    continuousRebuild = b;
}

void CompileEngineChildProcess::flushEditorChanges()
{
    for (Editor* ed : editors)
        ed->flushEditorChanges();
}

ProjectContentComponent* CompileEngineChildProcess::findProjectContentComponent() const
{
    for (MainWindow* mw : ProjucerApplication::getApp().mainWindowList.windows)
        if (mw->getProject() == &project)
            return mw->getProjectContentComponent();

    return nullptr;
}

CompileEngineChildProcess::Editor* CompileEngineChildProcess::getOrOpenEditorFor (const File& file)
{
    for (Editor* ed : editors)
        if (ed->file == file)
            return ed;

    if (ProjectContentComponent* pcc = findProjectContentComponent())
        if (pcc->showEditorForFile (file, false))
            return getOrOpenEditorFor (file);

    return nullptr;
}

void CompileEngineChildProcess::handleHighlightCode (const SourceCodeRange& location)
{
    ProjectContentComponent* pcc = findProjectContentComponent();

    if (pcc != nullptr && pcc->showEditorForFile (location.file, false))
    {
        SourceCodeEditor* sce = dynamic_cast <SourceCodeEditor*> (pcc->getEditorComponent());

        if (sce != nullptr && sce->editor != nullptr)
        {
            sce->highlight (location.range, true);

            Process::makeForegroundProcess();

            CodeEditorComponent& ed = *sce->editor;
            ed.getTopLevelComponent()->toFront (false);
            ed.grabKeyboardFocus();
        }
    }
}

void CompileEngineChildProcess::cleanAllCachedFilesForProject (Project& p)
{
    File cacheFolder (ProjectProperties::getCacheLocation (p));

    if (cacheFolder.isDirectory())
        cacheFolder.deleteRecursively();
}
