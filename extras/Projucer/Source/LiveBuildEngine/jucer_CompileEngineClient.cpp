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

#include "../Application/jucer_Headers.h"
#include "../Application/jucer_Application.h"
#include "../ProjectSaving/jucer_ProjectExporter.h"
#include "jucer_MessageIDs.h"
#include "jucer_CppHelpers.h"
#include "jucer_SourceCodeRange.h"
#include "jucer_ClassDatabase.h"
#include "jucer_DiagnosticMessage.h"
#include "jucer_ProjectBuildInfo.h"
#include "jucer_ClientServerMessages.h"
#include "jucer_CompileEngineClient.h"
#include "jucer_CompileEngineServer.h"
#include "jucer_CompileEngineSettings.h"

#ifndef RUN_CLANG_IN_CHILD_PROCESS
 #error
#endif

//==============================================================================
static File getProjucerTempFolder() noexcept
{
   #if JUCE_MAC
    return { "~/Library/Caches/com.juce.projucer" };
   #else
    return File::getSpecialLocation (File::tempDirectory).getChildFile ("com.juce.projucer");
   #endif
}

static File getCacheLocationForProject (Project& project) noexcept
{
    auto cacheFolderName = project.getProjectFilenameRootString() + "_" + project.getProjectUIDString();

   #if JUCE_DEBUG
    cacheFolderName += "_debug";
   #endif

    return getProjucerTempFolder().getChildFile ("Intermediate Files").getChildFile (cacheFolderName);
}

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

        auto pipeName = "ipc_" + String::toHexString (Random().nextInt64());
        auto command = createCommandLineForLaunchingServer (pipeName, owner.project.getProjectUIDString(),
                                                            getCacheLocationForProject (owner.project));

       #if RUN_CLANG_IN_CHILD_PROCESS
        if (! childProcess.start (command))
            jassertfalse;
       #else
        server = createClangServer (command);
       #endif

        if (connectToPipe (pipeName, 10000))
            MessageTypes::sendPing (*this);
        else
            jassertfalse;

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

    void timerCallback()
    {
        stopTimer();
        owner.handleCrash (String());
    }
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

        server.reset();
    }

    void restartServer()
    {
        server.reset (new ClientIPC (owner));
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

        build.setGlobalDefs (getGlobalDefs());
        build.setCompileFlags (project.getCompileEngineSettings().getExtraCompilerFlagsString());
        build.setExtraDLLs (getExtraDLLs());
        build.setJuceModulesFolder (EnabledModuleList::findDefaultModulesFolder (project).getFullPathName());

        build.setUtilsCppInclude (project.getAppIncludeFile().getFullPathName());

        build.setWindowsTargetPlatformVersion (project.getCompileEngineSettings().getWindowsTargetPlatformVersionString());

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

    std::unique_ptr<ClientIPC> server;

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

    String getGlobalDefs()
    {
        StringArray defs;

        defs.add (project.getCompileEngineSettings().getExtraPreprocessorDefsString());

        {
            auto projectDefines = project.getPreprocessorDefs();

            for (int i = 0; i < projectDefines.size(); ++i)
            {
                auto def = projectDefines.getAllKeys()[i];
                auto value = projectDefines.getAllValues()[i];

                if (value.isNotEmpty())
                    def << "=" << value;

                 defs.add (def);
            }
        }

        for (Project::ExporterIterator exporter (project); exporter.next();)
            if (exporter->canLaunchProject())
                defs.add (exporter->getExporterIdentifierMacro() + "=1");

        // Use the JUCE implementation of std::function until the live build
        // engine can compile the one from the standard library
        defs.add (" _LIBCPP_FUNCTIONAL=1");
        defs.removeEmptyStrings();

        return defs.joinIntoString (" ");
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
            auto f = projectItem.getFile();

            if (f.exists())
                compileUnits.add (f);
        }

        if (projectItem.shouldBeAddedToTargetProject() && ! projectItem.shouldBeAddedToBinaryResources())
        {
            auto f = projectItem.getFile();

            if (f.exists())
                userFiles.add (f);
        }
    }

    void scanForProjectFiles (Project& proj, ProjectBuildInfo& build)
    {
        Array<File> compileUnits, userFiles;
        scanProjectItem (proj.getMainGroup(), compileUnits, userFiles);

        {
            auto isVST3Host = project.getModules().isModuleEnabled ("juce_audio_processors")
                           && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_VST3");

            auto isPluginProject = proj.getProjectType().isAudioPlugin();

            OwnedArray<LibraryModule> modules;
            proj.getModules().createRequiredModules (modules);

            for (Project::ExporterIterator exporter (proj); exporter.next();)
            {
                if (exporter->canLaunchProject())
                {
                    for (auto* m : modules)
                    {
                        auto localModuleFolder = proj.getModules().shouldCopyModuleFilesLocally (m->moduleInfo.getID()).getValue()
                                                        ? proj.getLocalModuleFolder (m->moduleInfo.getID())
                                                        : m->moduleInfo.getFolder();


                        m->findAndAddCompiledUnits (*exporter, nullptr, compileUnits,
                                                    isPluginProject || isVST3Host ? ProjectType::Target::SharedCodeTarget
                                                                                  : ProjectType::Target::unspecified);

                        if (isPluginProject || isVST3Host)
                            m->findAndAddCompiledUnits (*exporter, nullptr, compileUnits, ProjectType::Target::StandalonePlugIn);
                    }

                    break;
                }
            }
        }

        for (int i = 0; ; ++i)
        {
            auto binaryDataCpp = proj.getBinaryDataCppFile (i);
            if (! binaryDataCpp.exists())
                break;

            compileUnits.add (binaryDataCpp);
        }

        for (auto i = compileUnits.size(); --i >= 0;)
            if (compileUnits.getReference(i).hasFileExtension (".r"))
                compileUnits.remove (i);

        build.setFiles (compileUnits, userFiles);
    }

    static bool doesProjectMatchSavedHeaderState (Project& project)
    {
        auto liveModules = project.getProjectRoot().getChildWithName (Ids::MODULES);

        std::unique_ptr<XmlElement> xml (XmlDocument::parse (project.getFile()));

        if (xml == nullptr || ! xml->hasTagName (Ids::JUCERPROJECT.toString()))
            return false;

        auto diskModules = ValueTree::fromXml (*xml).getChildWithName (Ids::MODULES);

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
        paths.addArray (getSearchPathsFromString (project.getCompileEngineSettings().getUserHeaderPathString()));

        return convertSearchPathsToAbsolute (paths);
    }

    StringArray getSystemIncludePaths()
    {
        StringArray paths;
        paths.addArray (getSearchPathsFromString (project.getCompileEngineSettings().getSystemHeaderPathString()));

        auto isVST3Host = project.getModules().isModuleEnabled ("juce_audio_processors")
                       && project.isConfigFlagEnabled ("JUCE_PLUGINHOST_VST3");

        if (project.getProjectType().isAudioPlugin() || isVST3Host)
            paths.add (getAppSettings().getStoredPath (Ids::vst3Path).toString());

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
        auto dlls = StringArray::fromTokens (project.getCompileEngineSettings().getExtraDLLsString(), "\n\r,", {});
        dlls.trim();
        dlls.removeEmptyStrings();

        return dlls;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ChildProcess)
};

//==============================================================================
CompileEngineChildProcess::CompileEngineChildProcess (Project& p)
    : project (p)
{
    ProjucerApplication::getApp().openDocumentManager.addListener (this);
    createProcess();
    errorList.setWarningsEnabled (project.getCompileEngineSettings().areWarningsEnabled());
}

CompileEngineChildProcess::~CompileEngineChildProcess()
{
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);

    process.reset();
    lastComponentList.clear();
}

void CompileEngineChildProcess::createProcess()
{
    jassert (process == nullptr);
    process.reset (new ChildProcess (*this, project));

    if (! process->openedOk)
        process.reset();

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
    runningAppProcess.reset();
}

void CompileEngineChildProcess::handleAppLaunched()
{
    runningAppProcess.reset (process.release());
    runningAppProcess->isRunningApp = true;
    createProcess();
}

void CompileEngineChildProcess::handleAppQuit()
{
    DBG ("handleAppQuit");
    runningAppProcess.reset();
}

bool CompileEngineChildProcess::isAppRunning() const noexcept
{
    return runningAppProcess != nullptr && runningAppProcess->isRunningApp;
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
        if (owner.project.getCompileEngineSettings().isContinuousRebuildEnabled())
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
    File cacheFolder (getCacheLocationForProject (p));

    if (cacheFolder.isDirectory())
        cacheFolder.deleteRecursively();
}
