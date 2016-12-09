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
#include "jucer_ProjectContentComponent.h"
#include "jucer_Module.h"
#include "../Application/jucer_MainWindow.h"
#include "../Application/jucer_Application.h"
#include "../Application/jucer_DownloadCompileEngineThread.h"
#include "../Code Editor/jucer_SourceCodeEditor.h"
#include "../Utility/jucer_FilePathPropertyComponent.h"
#include "jucer_TreeItemTypes.h"


//==============================================================================
class FileTreePanel   : public TreePanelBase
{
public:
    FileTreePanel (Project& p)
        : TreePanelBase (&p, "fileTreeState")
    {
        tree.setMultiSelectEnabled (true);
        setRoot (new FileTreeItemTypes::GroupItem (p.getMainGroup()));
    }

    void updateMissingFileStatuses()
    {
        if (FileTreeItemTypes::ProjectTreeItemBase* p = dynamic_cast<FileTreeItemTypes::ProjectTreeItemBase*> (rootItem.get()))
            p->checkFileStatus();
    }
};

//==============================================================================
class ConfigTreePanel   : public TreePanelBase
{
public:
    ConfigTreePanel (Project& p)
        : TreePanelBase (&p, "settingsTreeState")
    {
        tree.setMultiSelectEnabled (false);
        setRoot (new ConfigTreeItemTypes::RootItem (p));

        if (tree.getNumSelectedItems() == 0)
            tree.getRootItem()->setSelected (true, true);

       #if JUCE_MAC || JUCE_WINDOWS
        ApplicationCommandManager& commandManager = ProjucerApplication::getCommandManager();

        addAndMakeVisible (createExporterButton);
        createExporterButton.setCommandToTrigger (&commandManager, CommandIDs::createNewExporter, true);
        createExporterButton.setButtonText (commandManager.getNameOfCommand (CommandIDs::createNewExporter));
        createExporterButton.setColour (TextButton::buttonColourId, Colours::white.withAlpha (0.5f));

        addAndMakeVisible (openProjectButton);
        openProjectButton.setCommandToTrigger (&commandManager, CommandIDs::openInIDE, true);
        openProjectButton.setButtonText (commandManager.getNameOfCommand (CommandIDs::openInIDE));
        openProjectButton.setColour (TextButton::buttonColourId, Colours::white.withAlpha (0.5f));

        addAndMakeVisible (saveAndOpenButton);
        saveAndOpenButton.setCommandToTrigger (&commandManager, CommandIDs::saveAndOpenInIDE, true);
        saveAndOpenButton.setButtonText (commandManager.getNameOfCommand (CommandIDs::saveAndOpenInIDE));
        saveAndOpenButton.setColour (TextButton::buttonColourId, Colours::white.withAlpha (0.5f));
       #endif
    }

    void resized() override
    {
        Rectangle<int> r (getAvailableBounds());
        r.removeFromBottom (6);

        if (saveAndOpenButton.isVisible())
            saveAndOpenButton.setBounds (r.removeFromBottom (30).reduced (16, 4));

        if (openProjectButton.isVisible())
            openProjectButton.setBounds (r.removeFromBottom (30).reduced (16, 4));

        if (createExporterButton.isVisible())
        {
            r.removeFromBottom (10);
            createExporterButton.setBounds (r.removeFromBottom (30).reduced (16, 4));
        }

        tree.setBounds (r);
    }

    static void reselect (TreeViewItem& item)
    {
        item.setSelected (false, true);
        item.setSelected (true, true);
    }

    void showProjectSettings()
    {
        if (ConfigTreeItemTypes::ConfigTreeItemBase* root = dynamic_cast<ConfigTreeItemTypes::ConfigTreeItemBase*> (rootItem.get()))
            if (root->isProjectSettings())
                reselect (*root);
    }

    void showModules()
    {
        if (ConfigTreeItemTypes::ConfigTreeItemBase* mods = getModulesItem())
            reselect (*mods);
    }

    void showModule (const String& moduleID)
    {
        if (ConfigTreeItemTypes::ConfigTreeItemBase* mods = getModulesItem())
        {
            mods->setOpen (true);

            for (int i = mods->getNumSubItems(); --i >= 0;)
                if (ConfigTreeItemTypes::ModuleItem* m = dynamic_cast<ConfigTreeItemTypes::ModuleItem*> (mods->getSubItem (i)))
                    if (m->moduleID == moduleID)
                        reselect (*m);
        }
    }

    TextButton createExporterButton, openProjectButton, saveAndOpenButton;

private:
    ConfigTreeItemTypes::ConfigTreeItemBase* getModulesItem()
    {
        if (ConfigTreeItemTypes::ConfigTreeItemBase* root = dynamic_cast<ConfigTreeItemTypes::ConfigTreeItemBase*> (rootItem.get()))
            if (root->isProjectSettings())
                if (ConfigTreeItemTypes::ConfigTreeItemBase* mods = dynamic_cast<ConfigTreeItemTypes::ConfigTreeItemBase*> (root->getSubItem (0)))
                    if (mods->isModulesList())
                        return mods;

        return nullptr;
    }
};

//==============================================================================
struct LogoComponent  : public Component
{
    LogoComponent()
    {
        ScopedPointer<XmlElement> svg (XmlDocument::parse (BinaryData::background_logo_svg));
        logo = Drawable::createFromSVG (*svg);
    }

    void paint (Graphics& g) override
    {
        g.setColour (findColour (mainBackgroundColourId).contrasting (0.3f));

        Rectangle<int> r (getLocalBounds());

        g.setFont (15.0f);
        g.drawFittedText (getVersionInfo(), r.removeFromBottom (50), Justification::centredBottom, 3);

        logo->drawWithin (g, r.withTrimmedBottom (r.getHeight() / 4).toFloat(),
                          RectanglePlacement (RectanglePlacement::centred), 1.0f);
    }

    static String getVersionInfo()
    {
        return SystemStats::getJUCEVersion()
                + newLine
                + ProjucerApplication::getApp().getVersionDescription();
    }

    ScopedPointer<Drawable> logo;
};

//==============================================================================
ProjectContentComponent::ProjectContentComponent()
    : project (nullptr),
      currentDocument (nullptr),
      treeViewTabs (TabbedButtonBar::TabsAtTop)
{
    setOpaque (true);
    setWantsKeyboardFocus (true);

    addAndMakeVisible (logo = new LogoComponent());

    treeSizeConstrainer.setMinimumWidth (200);
    treeSizeConstrainer.setMaximumWidth (500);

    treeViewTabs.setOutline (0);
    treeViewTabs.getTabbedButtonBar().setMinimumTabScaleFactor (0.3);

    ProjucerApplication::getApp().openDocumentManager.addListener (this);

    Desktop::getInstance().addFocusChangeListener (this);
    startTimer (1600);
}

ProjectContentComponent::~ProjectContentComponent()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    killChildProcess();

    ProjucerApplication::getApp().openDocumentManager.removeListener (this);

    logo = nullptr;
    setProject (nullptr);
    contentView = nullptr;
    removeChildComponent (&bubbleMessage);
    jassert (getNumChildComponents() <= 1);
}

void ProjectContentComponent::paint (Graphics& g)
{
    ProjucerLookAndFeel::fillWithBackgroundTexture (*this, g);
}

void ProjectContentComponent::paintOverChildren (Graphics& g)
{
    if (resizerBar != nullptr)
    {
        const int shadowSize = 15;
        const int x = resizerBar->getX();

        ColourGradient cg (Colours::black.withAlpha (0.25f), (float) x, 0,
                           Colours::transparentBlack,        (float) (x - shadowSize), 0, false);
        cg.addColour (0.4, Colours::black.withAlpha (0.07f));
        cg.addColour (0.6, Colours::black.withAlpha (0.02f));

        g.setGradientFill (cg);
        g.fillRect (x - shadowSize, 0, shadowSize, getHeight());
    }
}

void ProjectContentComponent::resized()
{
    Rectangle<int> r (getLocalBounds());

    if (treeViewTabs.isVisible())
        treeViewTabs.setBounds (r.removeFromLeft (treeViewTabs.getWidth()));

    if (resizerBar != nullptr)
        resizerBar->setBounds (r.withWidth (4));

    if (contentView != nullptr)
        contentView->setBounds (r);

    if (logo != nullptr)
        logo->setBounds (r.reduced (r.getWidth() / 4, r.getHeight() / 4));
}

void ProjectContentComponent::lookAndFeelChanged()
{
    repaint();
}

void ProjectContentComponent::childBoundsChanged (Component* child)
{
    if (child == &treeViewTabs)
        resized();
}

void ProjectContentComponent::setProject (Project* newProject)
{
    if (project != newProject)
    {
        lastCrashMessage = String();
        killChildProcess();

        if (project != nullptr)
            project->removeChangeListener (this);

        contentView = nullptr;
        resizerBar = nullptr;

        deleteProjectTabs();
        project = newProject;
        rebuildProjectTabs();
    }
}

void ProjectContentComponent::rebuildProjectTabs()
{
    deleteProjectTabs();

    if (project != nullptr)
    {
        addAndMakeVisible (treeViewTabs);

        createProjectTabs();

        PropertiesFile& settings = project->getStoredProperties();

        const String lastTabName (settings.getValue ("lastTab"));
        int lastTabIndex = treeViewTabs.getTabNames().indexOf (lastTabName);

        if (lastTabIndex < 0 || lastTabIndex > treeViewTabs.getNumTabs())
            lastTabIndex = 1;

        treeViewTabs.setCurrentTabIndex (lastTabIndex);

        int lastTreeWidth = settings.getValue ("projectPanelWidth").getIntValue();
        if (lastTreeWidth < 150)
            lastTreeWidth = 240;

        treeViewTabs.setBounds (0, 0, lastTreeWidth, getHeight());

        addAndMakeVisible (resizerBar = new ResizableEdgeComponent (&treeViewTabs, &treeSizeConstrainer,
                                                                    ResizableEdgeComponent::rightEdge));
        resizerBar->setAlwaysOnTop (true);

        project->addChangeListener (this);

        updateMissingFileStatuses();
    }
    else
    {
        treeViewTabs.setVisible (false);
    }

    resized();
}


//==============================================================================
struct BuildTabComponent  : public ConcertinaPanel
{
    BuildTabComponent (CompileEngineChildProcess* child, ProjucerAppClasses::ErrorListComp* errorList)
        : errorListComp (errorList)
    {
        CurrentActivitiesComp* activities = new CurrentActivitiesComp (child->activityList);
        ComponentListComp* comps = new ComponentListComp (*child);

        addPanel (-1, errorList, true);
        addPanel (-1, comps, true);
        addPanel (-1, activities, true);

        setMaximumPanelSize (activities, CurrentActivitiesComp::getMaxPanelHeight());
        setPanelSize (errorList, 200, false);
        setPanelSize (comps, 300, false);
    }

    Component::SafePointer<ProjucerAppClasses::ErrorListComp> errorListComp;
};

struct ProjucerDisabledComp   : public Component,
                                private Button::Listener
{
    ProjucerDisabledComp (String message, bool loggedIn, bool showSubscribeButton,
                          bool showSignInButton, bool showSwitchAccountButton,
                          bool showDownloadButton)
              : isLoggedIn (loggedIn)
    {
        infoLabel.setColour (Label::textColourId, findColour (mainBackgroundColourId).contrasting (0.7f));
        infoLabel.setJustificationType (Justification::centred);
        infoLabel.setText (message, dontSendNotification);
        addAndMakeVisible (infoLabel);

        if (showSubscribeButton)
        {
            subscribeButton = new TextButton (String ( "Subscribe..."));
            addAndMakeVisible (*subscribeButton);
            subscribeButton->addListener (this);
        }

        if (showSignInButton)
        {
            signInButton = new TextButton (String ( "Sign in..."));
            addAndMakeVisible (*signInButton);
            signInButton->addListener (this);
        }

        if (showSwitchAccountButton)
        {
            switchAccountButton = new TextButton (String ("Switch account..."));
            addAndMakeVisible (*switchAccountButton);
            switchAccountButton->addListener (this);
        }

        if (showDownloadButton)
        {
            downloadButton = new TextButton (String ("Download live-build engine"));
            addAndMakeVisible (*downloadButton);
            downloadButton->addListener (this);
        }
    }

    void resized() override
    {
        int infoWidth = proportionOfWidth (0.9f);
        int infoHeight = 100;

        infoLabel.centreWithSize (infoWidth, infoHeight);

        int buttonWidth = jmin (getWidth() - 10, 150);
        int buttonHeight = 22;
        int itemDistance = 10;

        int buttonCenterX = infoLabel.getBounds().getCentreX();
        int buttonCenterY = infoLabel.getBottom() + itemDistance + buttonHeight / 2;

        if (subscribeButton.get() != nullptr)
        {
            subscribeButton->setSize (buttonWidth, buttonHeight);
            subscribeButton->setCentrePosition (buttonCenterX, buttonCenterY);
            buttonCenterY += itemDistance + buttonHeight;
        }

        if (signInButton.get() != nullptr)
        {
            signInButton->setSize (buttonWidth, buttonHeight);
            signInButton->setCentrePosition (buttonCenterX, buttonCenterY);
            buttonCenterY += itemDistance + buttonHeight;
        }

        if (switchAccountButton.get() != nullptr)
        {
            switchAccountButton->setSize (buttonWidth, buttonHeight);
            switchAccountButton->setCentrePosition (buttonCenterX, buttonCenterY);
            buttonCenterY += itemDistance + buttonHeight;
        }

        if (downloadButton.get() != nullptr)
        {
            downloadButton->setSize (buttonWidth, buttonHeight);
            downloadButton->setCentrePosition (buttonCenterX, buttonCenterY);
        }
    }

    void buttonClicked (Button* btn) override
    {
        if (btn == subscribeButton.get())
        {
            URL ("http://www.juce.com/get-juce#indie").launchInDefaultBrowser();
        }
        else if (btn == signInButton.get())
        {
            ProjucerApplication::getApp().showLoginForm();
        }
        else if (btn == switchAccountButton.get())
        {
            ProjucerApplication::getApp().showLoginForm();
        }
        else if (btn == downloadButton.get())
        {
            if (DownloadCompileEngineThread::downloadAndInstall())
            {
                if (! ProjucerLicenses::getInstance()->retryLoadDll())
                {
                    AlertWindow::showMessageBox(AlertWindow::WarningIcon,
                                                "Download and install",
                                                "Loading the live-build engine failed");
                    return;
                }

                // async!
                ProjucerApplication::getApp().showLoginForm();

                // if sign in successful project tabs update, otherwise they were not
                auto parent = findParentComponentOfClass<ProjectContentComponent>();
                parent->rebuildProjectTabs();
            }
        }
    }

    bool isLoggedIn;

private:
    Label infoLabel { "info", String() };
    ScopedPointer<TextButton> subscribeButton;
    ScopedPointer<TextButton> signInButton;
    ScopedPointer<TextButton> switchAccountButton;
    ScopedPointer<TextButton> downloadButton;
};

struct EnableBuildComp   : public Component
{
    EnableBuildComp()
    {
        addAndMakeVisible (&enableButton);
        enableButton.setCommandToTrigger (&ProjucerApplication::getCommandManager(), CommandIDs::enableBuild, true);
    }

    void resized() override
    {
        enableButton.centreWithSize (jmin (getWidth() - 10, 150), 22);
    }

    void paint (Graphics& g) override
    {
        if (ProjectContentComponent* ppc = findParentComponentOfClass<ProjectContentComponent>())
        {
            g.setColour (findColour (mainBackgroundColourId).contrasting (0.7f));
            g.setFont (13.0f);
            g.drawFittedText (ppc->lastCrashMessage,
                              getLocalBounds().reduced (8).withBottom (enableButton.getY() - 20),
                              Justification::centredBottom, 10);
        }
    }

    TextButton enableButton { "Restart Compiler" };
};

//==============================================================================
Component* ProjectContentComponent::createBuildTab (CompileEngineChildProcess* child)
{
    if (child != nullptr)
    {
        child->crashHandler = [this] (const String& m) { this->handleCrash (m); };

        return new BuildTabComponent (child, new ProjucerAppClasses::ErrorListComp (child->errorList));
    }

    jassert (project != nullptr);

    const auto osType = SystemStats::getOperatingSystemType();
    const bool isMac = (osType & SystemStats::MacOSX) != 0;
    const bool isWin = (osType & SystemStats::Windows) != 0;
    const bool isLinux = (osType & SystemStats::Linux) != 0;

    if (! isMac && ! isWin && ! isLinux)
        return createDisabledBuildTabInfoOnly (
            "Live-build features are not supported on your system.\n\n"
            "Please check supported platforms at www.juce.com!");

    if (isLinux)
        return createDisabledBuildTabInfoOnly (
            "Live-build features for Linux are under development.\n\n"
            "Please check for updates at www.juce.com!");

    if (isMac)
        if (osType < SystemStats::MacOSX_10_9)
            return createDisabledBuildTabInfoOnly (
                "Live-build features are available only on MacOSX 10.9 or higher.");

    if (isWin)
        if (! SystemStats::isOperatingSystem64Bit() || osType < SystemStats::Windows8_0)
            return createDisabledBuildTabInfoOnly (
                "Live-build features are available only on 64-Bit Windows 8 or higher.");

    const auto& unlockStatus = *ProjucerLicenses::getInstance();

    if (! unlockStatus.isLoggedIn())
        return createDisabledBuildTabSubscribe ("Sign in with your ROLI account",
                                                false, unlockStatus.isDLLPresent());

    if (! unlockStatus.hasLiveCodingLicence())
        return createDisabledBuildTabSubscribe ("Subscribe to JUCE Pro or Indie",
                                                true, unlockStatus.isDLLPresent());

    jassert (unlockStatus.isLoggedIn());
    jassert (unlockStatus.isDLLPresent());
    return new EnableBuildComp();
}

Component* ProjectContentComponent::createDisabledBuildTabSubscribe (String textPrefix,
                                                                     bool loggedIn, bool dllPresent)
{
    bool showSubscribeButton = true;
    bool showSignInButton = dllPresent && ! loggedIn;
    bool showSwitchAccountButton = dllPresent && loggedIn;
    bool showDownloadButton = ! dllPresent;

    return new ProjucerDisabledComp (
        textPrefix + " to use the Projucer's live-build features:",
        loggedIn, showSubscribeButton, showSignInButton, showSwitchAccountButton, showDownloadButton);
}

Component* ProjectContentComponent::createDisabledBuildTabInfoOnly(const char* message)
{
    return new ProjucerDisabledComp (message, false, false, false, false, false);
}

//==============================================================================
BuildTabComponent* findBuildTab (const TabbedComponent& tabs)
{
    return dynamic_cast<BuildTabComponent*> (tabs.getTabContentComponent (2));
}

bool ProjectContentComponent::isBuildTabEnabled() const
{
    return findBuildTab (treeViewTabs) != nullptr;
}

bool ProjectContentComponent::isBuildTabSuitableForLoggedInUser() const
{
    return isBuildTabEnabled()
             || isBuildTabLoggedInWithoutLicense()
             || dynamic_cast<EnableBuildComp*> (treeViewTabs.getTabContentComponent (2)) != nullptr;
}

bool ProjectContentComponent::isBuildTabLoggedInWithoutLicense() const
{
    if (auto* c = dynamic_cast<ProjucerDisabledComp*> (treeViewTabs.getTabContentComponent (2)))
        return c->isLoggedIn;

    return false;
}

void ProjectContentComponent::createProjectTabs()
{
    jassert (project != nullptr);
    const Colour tabColour (Colours::transparentBlack);

    treeViewTabs.addTab ("Files",  tabColour, new FileTreePanel (*project), true);
    treeViewTabs.addTab ("Config", tabColour, new ConfigTreePanel (*project), true);

    const CompileEngineChildProcess::Ptr childProc (getChildProcess());

    treeViewTabs.addTab ("Build", Colours::transparentBlack, createBuildTab (childProc), true);

    if (childProc != nullptr)
        treeViewTabs.getTabbedButtonBar().getTabButton (2)
            ->setExtraComponent (new BuildStatusTabComp (childProc->errorList,
                                                         childProc->activityList),
                                 TabBarButton::afterText);
}

void ProjectContentComponent::deleteProjectTabs()
{
    if (project != nullptr && treeViewTabs.isShowing())
    {
        PropertiesFile& settings = project->getStoredProperties();

        if (treeViewTabs.getWidth() > 0)
            settings.setValue ("projectPanelWidth", treeViewTabs.getWidth());

        if (treeViewTabs.getNumTabs() > 0)
            settings.setValue ("lastTab", treeViewTabs.getCurrentTabName());
    }

    treeViewTabs.clearTabs();
}

void ProjectContentComponent::saveTreeViewState()
{
    for (int i = treeViewTabs.getNumTabs(); --i >= 0;)
        if (TreePanelBase* t = dynamic_cast<TreePanelBase*> (treeViewTabs.getTabContentComponent (i)))
            t->saveOpenness();
}

void ProjectContentComponent::saveOpenDocumentList()
{
    if (project != nullptr)
    {
        ScopedPointer<XmlElement> xml (recentDocumentList.createXML());

        if (xml != nullptr)
            project->getStoredProperties().setValue ("lastDocs", xml);
    }
}

void ProjectContentComponent::reloadLastOpenDocuments()
{
    if (project != nullptr)
    {
        ScopedPointer<XmlElement> xml (project->getStoredProperties().getXmlValue ("lastDocs"));

        if (xml != nullptr)
        {
            recentDocumentList.restoreFromXML (*project, *xml);
            showDocument (recentDocumentList.getCurrentDocument(), true);
        }
    }
}

bool ProjectContentComponent::documentAboutToClose (OpenDocumentManager::Document* document)
{
    hideDocument (document);
    return true;
}

void ProjectContentComponent::changeListenerCallback (ChangeBroadcaster*)
{
    updateMissingFileStatuses();
}

void ProjectContentComponent::updateMissingFileStatuses()
{
    if (FileTreePanel* tree = dynamic_cast<FileTreePanel*> (treeViewTabs.getTabContentComponent (0)))
        tree->updateMissingFileStatuses();
}

bool ProjectContentComponent::showEditorForFile (const File& f, bool grabFocus)
{
    return getCurrentFile() == f
            || showDocument (ProjucerApplication::getApp().openDocumentManager.openFile (project, f), grabFocus);
}

bool ProjectContentComponent::hasFileInRecentList (const File& f) const
{
    return recentDocumentList.contains (f);
}

File ProjectContentComponent::getCurrentFile() const
{
    return currentDocument != nullptr ? currentDocument->getFile()
                                      : File();
}

bool ProjectContentComponent::showDocument (OpenDocumentManager::Document* doc, bool grabFocus)
{
    if (doc == nullptr)
        return false;

    if (doc->hasFileBeenModifiedExternally())
        doc->reloadFromFile();

    if (doc == getCurrentDocument() && contentView != nullptr)
    {
        if (grabFocus)
            contentView->grabKeyboardFocus();

        return true;
    }

    recentDocumentList.newDocumentOpened (doc);

    bool opened = setEditorComponent (doc->createEditor(), doc);

    if (opened && grabFocus)
        contentView->grabKeyboardFocus();

    return opened;
}

void ProjectContentComponent::hideEditor()
{
    currentDocument = nullptr;
    contentView = nullptr;
    updateMainWindowTitle();
    ProjucerApplication::getCommandManager().commandStatusChanged();
    resized();
}

void ProjectContentComponent::hideDocument (OpenDocumentManager::Document* doc)
{
    if (doc == currentDocument)
    {
        if (OpenDocumentManager::Document* replacement = recentDocumentList.getClosestPreviousDocOtherThan (doc))
            showDocument (replacement, true);
        else
            hideEditor();
    }
}

bool ProjectContentComponent::setEditorComponent (Component* editor,
                                                  OpenDocumentManager::Document* doc)
{
    if (editor != nullptr)
    {
        contentView = nullptr;
        contentView = editor;
        currentDocument = doc;
        addAndMakeVisible (editor);
        resized();

        updateMainWindowTitle();
        ProjucerApplication::getCommandManager().commandStatusChanged();
        return true;
    }

    updateMainWindowTitle();
    return false;
}

void ProjectContentComponent::closeDocument()
{
    if (currentDocument != nullptr)
        ProjucerApplication::getApp().openDocumentManager.closeDocument (currentDocument, true);
    else if (contentView != nullptr)
        if (! goToPreviousFile())
            hideEditor();
}

static void showSaveWarning (OpenDocumentManager::Document* currentDocument)
{
    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                 TRANS("Save failed!"),
                                 TRANS("Couldn't save the file:")
                                   + "\n" + currentDocument->getFile().getFullPathName());
}

void ProjectContentComponent::saveDocument()
{
    if (currentDocument != nullptr)
    {
        if (! currentDocument->save())
            showSaveWarning (currentDocument);
    }
    else
        saveProject();

    updateMainWindowTitle();
}

void ProjectContentComponent::saveAs()
{
    if (currentDocument != nullptr && ! currentDocument->saveAs())
        showSaveWarning (currentDocument);
}

bool ProjectContentComponent::goToPreviousFile()
{
    OpenDocumentManager::Document* doc = recentDocumentList.getCurrentDocument();

    if (doc == nullptr || doc == getCurrentDocument())
        doc = recentDocumentList.getPrevious();

    return showDocument (doc, true);
}

bool ProjectContentComponent::goToNextFile()
{
    return showDocument (recentDocumentList.getNext(), true);
}

bool ProjectContentComponent::canGoToCounterpart() const
{
    return currentDocument != nullptr
            && currentDocument->getCounterpartFile().exists();
}

bool ProjectContentComponent::goToCounterpart()
{
    if (currentDocument != nullptr)
    {
        const File file (currentDocument->getCounterpartFile());

        if (file.exists())
            return showEditorForFile (file, true);
    }

    return false;
}

bool ProjectContentComponent::saveProject()
{
    return project != nullptr
            && project->save (true, true) == FileBasedDocument::savedOk;
}

void ProjectContentComponent::closeProject()
{
    if (MainWindow* const mw = findParentComponentOfClass<MainWindow>())
        mw->closeCurrentProject();
}

void ProjectContentComponent::showFilesTab()
{
    treeViewTabs.setCurrentTabIndex (0);
}

void ProjectContentComponent::showConfigTab()
{
    treeViewTabs.setCurrentTabIndex (1);
}

void ProjectContentComponent::showProjectSettings()
{
    showConfigTab();

    if (ConfigTreePanel* const tree = dynamic_cast<ConfigTreePanel*> (treeViewTabs.getCurrentContentComponent()))
        tree->showProjectSettings();
}

void ProjectContentComponent::showModules()
{
    showConfigTab();

    if (ConfigTreePanel* const tree = dynamic_cast<ConfigTreePanel*> (treeViewTabs.getCurrentContentComponent()))
        tree->showModules();
}

void ProjectContentComponent::showModule (const String& moduleID)
{
    showConfigTab();

    if (ConfigTreePanel* const tree = dynamic_cast<ConfigTreePanel*> (treeViewTabs.getCurrentContentComponent()))
        tree->showModule (moduleID);
}

StringArray ProjectContentComponent::getExportersWhichCanLaunch() const
{
    StringArray s;

    if (project != nullptr)
        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (exporter->canLaunchProject())
                s.add (exporter->getName());

    return s;
}

void ProjectContentComponent::openInIDE (int exporterIndex, bool saveFirst)
{
    if (saveFirst)
        saveProject();

    int i = 0;

    if (project != nullptr)
        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (exporter->canLaunchProject())
                if (i++ == exporterIndex && exporter->launchProject())
                    break;
}

static void openIDEMenuCallback (int result, ProjectContentComponent* comp, bool saveFirst)
{
    if (comp != nullptr && result > 0)
        comp->openInIDE (result - 1, saveFirst);
}

void ProjectContentComponent::openInIDE (bool saveFirst)
{
    if (project != nullptr)
    {
        StringArray possibleExporters = getExportersWhichCanLaunch();

        if (possibleExporters.size() > 1)
        {
            PopupMenu menu;

            for (int i = 0; i < possibleExporters.size(); ++i)
                menu.addItem (i + 1, possibleExporters[i]);

            menu.showMenuAsync (PopupMenu::Options(),
                                ModalCallbackFunction::forComponent (openIDEMenuCallback, this, saveFirst));
        }
        else
        {
            openInIDE (0, saveFirst);
        }
    }
}

static void newExporterMenuCallback (int result, ProjectContentComponent* comp)
{
    if (comp != nullptr && result > 0)
    {
        if (Project* p = comp->getProject())
        {
            String exporterName (ProjectExporter::getExporterNames() [result - 1]);

            if (exporterName.isNotEmpty())
                p->addNewExporter (exporterName);
        }
    }
}

void ProjectContentComponent::showNewExporterMenu()
{
    if (project != nullptr)
    {
        PopupMenu menu;

        menu.addSectionHeader ("Create a new export target:");

        Array<ProjectExporter::ExporterTypeInfo> exporters (ProjectExporter::getExporterTypes());

        for (int i = 0; i < exporters.size(); ++i)
        {
            const ProjectExporter::ExporterTypeInfo& type = exporters.getReference(i);

            menu.addItem (i + 1, type.name, true, false, type.getIcon());
        }

        menu.showMenuAsync (PopupMenu::Options(),
                            ModalCallbackFunction::forComponent (newExporterMenuCallback, this));
    }
}

void ProjectContentComponent::deleteSelectedTreeItems()
{
    if (TreePanelBase* const tree = dynamic_cast<TreePanelBase*> (treeViewTabs.getCurrentContentComponent()))
        tree->deleteSelectedItems();
}

void ProjectContentComponent::updateMainWindowTitle()
{
    if (MainWindow* mw = findParentComponentOfClass<MainWindow>())
    {
        String title;
        File file;
        bool edited = false;

        if (currentDocument != nullptr)
        {
            title = currentDocument->getName();
            edited = currentDocument->needsSaving();
            file = currentDocument->getFile();
        }

        if (ComponentPeer* peer = mw->getPeer())
        {
            if (! peer->setDocumentEditedStatus (edited))
                if (edited)
                    title << "*";

            peer->setRepresentedFile (file);
        }

        mw->updateTitle (title);
    }
}

void ProjectContentComponent::showBubbleMessage (Rectangle<int> pos, const String& text)
{
    addChildComponent (bubbleMessage);
    bubbleMessage.setColour (BubbleComponent::backgroundColourId, Colours::white.withAlpha (0.7f));
    bubbleMessage.setColour (BubbleComponent::outlineColourId, Colours::black.withAlpha (0.8f));
    bubbleMessage.setAlwaysOnTop (true);

    bubbleMessage.showAt (pos, AttributedString (text), 3000, true, false);
}

//==============================================================================
void ProjectContentComponent::showTranslationTool()
{
    if (translationTool != nullptr)
    {
        translationTool->toFront (true);
    }
    else if (project != nullptr)
    {
        new FloatingToolWindow ("Translation File Builder",
                                "transToolWindowPos",
                                new TranslationToolComponent(),
                                translationTool,
                                600, 700,
                                600, 400, 10000, 10000);
    }
}

//==============================================================================
struct AsyncCommandRetrier  : public Timer
{
    AsyncCommandRetrier (const ApplicationCommandTarget::InvocationInfo& i)  : info (i)
    {
        info.originatingComponent = nullptr;
        startTimer (500);
    }

    void timerCallback() override
    {
        stopTimer();
        ProjucerApplication::getCommandManager().invoke (info, true);
        delete this;
    }

    ApplicationCommandTarget::InvocationInfo info;

    JUCE_DECLARE_NON_COPYABLE (AsyncCommandRetrier)
};

bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo& info)
{
    if (ModalComponentManager::getInstance()->cancelAllModalComponents())
    {
        new AsyncCommandRetrier (info);
        return true;
    }

    return false;
}

//==============================================================================
ApplicationCommandTarget* ProjectContentComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void ProjectContentComponent::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = { CommandIDs::saveDocument,
                              CommandIDs::saveDocumentAs,
                              CommandIDs::closeDocument,
                              CommandIDs::saveProject,
                              CommandIDs::closeProject,
                              CommandIDs::openInIDE,
                              CommandIDs::saveAndOpenInIDE,
                              CommandIDs::createNewExporter,
                              CommandIDs::showFilePanel,
                              CommandIDs::showConfigPanel,
                              CommandIDs::showProjectSettings,
                              CommandIDs::showProjectModules,
                              CommandIDs::goToPreviousDoc,
                              CommandIDs::goToNextDoc,
                              CommandIDs::goToCounterpart,
                              CommandIDs::deleteSelectedItem,
                              CommandIDs::showTranslationTool,
                              CommandIDs::showBuildTab,
                              CommandIDs::cleanAll,
                              CommandIDs::enableBuild,
                              CommandIDs::buildNow,
                              CommandIDs::toggleContinuousBuild,
                              CommandIDs::showWarnings,
                              CommandIDs::reinstantiateComp,
                              CommandIDs::launchApp,
                              CommandIDs::killApp,
                              CommandIDs::nextError,
                              CommandIDs::prevError };

    commands.addArray (ids, numElementsInArray (ids));
}

void ProjectContentComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    String documentName;
    if (currentDocument != nullptr)
        documentName = " '" + currentDocument->getName().substring (0, 32) + "'";

   #if JUCE_MAC
    const ModifierKeys cmdCtrl (ModifierKeys::ctrlModifier | ModifierKeys::commandModifier);
   #else
    const ModifierKeys cmdCtrl (ModifierKeys::ctrlModifier | ModifierKeys::altModifier);
   #endif

    switch (commandID)
    {
    case CommandIDs::saveProject:
        result.setInfo ("Save Project",
                        "Saves the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::closeProject:
        result.setInfo ("Close Project",
                        "Closes the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::saveDocument:
        result.setInfo ("Save" + documentName,
                        "Saves the current document",
                        CommandCategories::general, 0);
        result.setActive (currentDocument != nullptr || project != nullptr);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::saveDocumentAs:
        result.setInfo ("Save As...",
                        "Saves the current document to a new location",
                        CommandCategories::general, 0);
        result.setActive (currentDocument != nullptr);
        result.defaultKeypresses.add (KeyPress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::closeDocument:
        result.setInfo ("Close" + documentName,
                        "Closes the current document",
                        CommandCategories::general, 0);
        result.setActive (contentView != nullptr);
        result.defaultKeypresses.add (KeyPress ('w', cmdCtrl, 0));
        break;

    case CommandIDs::goToPreviousDoc:
        result.setInfo ("Previous Document", "Go to previous document", CommandCategories::general, 0);
        result.setActive (recentDocumentList.canGoToPrevious());
        result.defaultKeypresses.add (KeyPress (KeyPress::leftKey, cmdCtrl, 0));
        break;

    case CommandIDs::goToNextDoc:
        result.setInfo ("Next Document", "Go to next document", CommandCategories::general, 0);
        result.setActive (recentDocumentList.canGoToNext());
        result.defaultKeypresses.add (KeyPress (KeyPress::rightKey, cmdCtrl, 0));
        break;

    case CommandIDs::goToCounterpart:
        result.setInfo ("Open corresponding header or cpp file", "Open counterpart file", CommandCategories::general, 0);
        result.setActive (canGoToCounterpart());
        result.defaultKeypresses.add (KeyPress (KeyPress::upKey, cmdCtrl, 0));
        break;

    case CommandIDs::openInIDE:
        result.setInfo ("Open in IDE...",

                        "Launches the project in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (ProjectExporter::canProjectBeLaunched (project));
        break;

    case CommandIDs::saveAndOpenInIDE:
        result.setInfo ("Save Project and Open in IDE...",

                        "Saves the project and launches it in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (ProjectExporter::canProjectBeLaunched (project));
        result.defaultKeypresses.add (KeyPress ('l', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::createNewExporter:
        result.setInfo ("Create New Exporter...",
                        "Creates a new exporter for a compiler type",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::showFilePanel:
        result.setInfo ("Show File Panel",
                        "Shows the tree of files for this project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('p', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showConfigPanel:
        result.setInfo ("Show Config Panel",
                        "Shows the build options for the project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('i', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::showProjectSettings:
        result.setInfo ("Show Project Settings",
                        "Shows the main project options page",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('i', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::showProjectModules:
        result.setInfo ("Show Project Modules",
                        "Shows the project's list of modules",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add (KeyPress ('m', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        break;

    case CommandIDs::deleteSelectedItem:
        result.setInfo ("Delete Selected File", String(), CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress (KeyPress::deleteKey, 0, 0));
        result.defaultKeypresses.add (KeyPress (KeyPress::backspaceKey, 0, 0));
        result.setActive (dynamic_cast<TreePanelBase*> (treeViewTabs.getCurrentContentComponent()) != nullptr);
        break;

    case CommandIDs::showTranslationTool:
        result.setInfo ("Translation File Builder", "Shows the translation file helper tool", CommandCategories::general, 0);
        break;

    case CommandIDs::showBuildTab:
        result.setInfo ("Show Build Panel", "Shows the build panel", CommandCategories::general, 0);
        //result.defaultKeypresses.add (KeyPress ('b', ModifierKeys::commandModifier, 0));
        break;

    case CommandIDs::cleanAll:
        result.setInfo ("Clean All", "Cleans all intermediate files", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('k', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        result.setActive (project != nullptr);
        break;

    case CommandIDs::enableBuild:
        result.setInfo ("Enable Compilation", "Enables/disables the compiler", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        result.setActive (project != nullptr);
        result.setTicked (childProcess != nullptr);
        break;

    case CommandIDs::buildNow:
        result.setInfo ("Build Now", "Recompiles any out-of-date files and updates the JIT engine", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('b', ModifierKeys::commandModifier, 0));
        result.setActive (childProcess != nullptr);
        break;

    case CommandIDs::toggleContinuousBuild:
        result.setInfo ("Enable Continuous Recompiling", "Continuously recompiles any changes made in code editors", CommandCategories::general, 0);
        result.setActive (childProcess != nullptr);
        result.setTicked (isContinuousRebuildEnabled());
        break;

    case CommandIDs::showWarnings:
        result.setInfo ("Show Warnings", "Shows or hides compilation warnings", CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.setTicked (areWarningsEnabled());
        break;

    case CommandIDs::launchApp:
        result.setInfo ("Launch Application", "Invokes the app's main() function", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::commandModifier, 0));
        result.setActive (childProcess != nullptr && childProcess->canLaunchApp());
        break;

    case CommandIDs::killApp:
        result.setInfo ("Stop Application", "Kills the app if it's running", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('.', ModifierKeys::commandModifier, 0));
        result.setActive (childProcess != nullptr && childProcess->canKillApp());
        break;

    case CommandIDs::reinstantiateComp:
        result.setInfo ("Re-instantiate Components", "Re-loads any component editors that are open", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        result.setActive (childProcess != nullptr);
        break;

    case CommandIDs::nextError:
        result.setInfo ("Highlight next error", "Jumps to the next error or warning", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('e', ModifierKeys::commandModifier, 0));
        result.setActive (childProcess != nullptr && ! childProcess->errorList.isEmpty());
        break;

    case CommandIDs::prevError:
        result.setInfo ("Highlight previous error", "Jumps to the last error or warning", CommandCategories::general, 0);
        result.defaultKeypresses.add (KeyPress ('e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
        result.setActive (childProcess != nullptr && ! childProcess->errorList.isEmpty());
        break;

    default:
        break;
    }
}

bool ProjectContentComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::saveProject:
        case CommandIDs::closeProject:
        case CommandIDs::saveDocument:
        case CommandIDs::saveDocumentAs:
        case CommandIDs::closeDocument:
        case CommandIDs::goToPreviousDoc:
        case CommandIDs::goToNextDoc:
        case CommandIDs::goToCounterpart:
        case CommandIDs::saveAndOpenInIDE:
            if (reinvokeCommandAfterCancellingModalComps (info))
            {
                grabKeyboardFocus(); // to force any open labels to close their text editors
                return true;
            }

            break;

        default:
            break;
    }

    if (isCurrentlyBlockedByAnotherModalComponent())
        return false;

    switch (info.commandID)
    {
        case CommandIDs::saveProject:               saveProject(); break;
        case CommandIDs::closeProject:              closeProject(); break;
        case CommandIDs::saveDocument:              saveDocument(); break;
        case CommandIDs::saveDocumentAs:            saveAs(); break;

        case CommandIDs::closeDocument:             closeDocument(); break;
        case CommandIDs::goToPreviousDoc:           goToPreviousFile(); break;
        case CommandIDs::goToNextDoc:               goToNextFile(); break;
        case CommandIDs::goToCounterpart:           goToCounterpart(); break;

        case CommandIDs::showFilePanel:             showFilesTab(); break;
        case CommandIDs::showConfigPanel:           showConfigTab(); break;
        case CommandIDs::showProjectSettings:       showProjectSettings(); break;
        case CommandIDs::showProjectModules:        showModules(); break;

        case CommandIDs::openInIDE:                 openInIDE (false); break;
        case CommandIDs::saveAndOpenInIDE:          openInIDE (true); break;

        case CommandIDs::createNewExporter:         showNewExporterMenu(); break;

        case CommandIDs::deleteSelectedItem:        deleteSelectedTreeItems(); break;

        case CommandIDs::showTranslationTool:       showTranslationTool(); break;

        case CommandIDs::showBuildTab:              showBuildTab(); break;
        case CommandIDs::cleanAll:                  cleanAll(); break;
        case CommandIDs::enableBuild:               setBuildEnabled (! isBuildEnabled()); break;
        case CommandIDs::buildNow:                  rebuildNow(); break;
        case CommandIDs::toggleContinuousBuild:     setContinuousRebuildEnabled (! isContinuousRebuildEnabled()); break;
        case CommandIDs::launchApp:                 launchApp(); break;
        case CommandIDs::killApp:                   killApp(); break;
        case CommandIDs::reinstantiateComp:         reinstantiateLivePreviewWindows(); break;
        case CommandIDs::showWarnings:              toggleWarnings(); break;
        case CommandIDs::nextError:                 showNextError(); break;
        case CommandIDs::prevError:                 showPreviousError(); break;

        default:
            return false;
    }

    return true;
}

void ProjectContentComponent::getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails& dragSourceDetails,
                                                                   OwnedArray<Project::Item>& selectedNodes)
{
    FileTreeItemTypes::ProjectTreeItemBase::getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);
}

//==============================================================================
void ProjectContentComponent::killChildProcess()
{
    if (childProcess != nullptr)
    {
        deleteProjectTabs();
        childProcess = nullptr;
        ProjucerApplication::getApp().childProcessCache->removeOrphans();
    }
}

void ProjectContentComponent::setBuildEnabled (bool b)
{
    if (project != nullptr && b != isBuildEnabled())
    {
        LiveBuildProjectSettings::setBuildDisabled (*project, ! b);
        killChildProcess();
        refreshTabsIfBuildStatusChanged();
    }
}

void ProjectContentComponent::showBuildTab()
{
    WeakReference<Component> currentFocus (Component::getCurrentlyFocusedComponent());
    treeViewTabs.setCurrentTabIndex (2);

    if (currentFocus != nullptr)
        currentFocus->grabKeyboardFocus();
}

void ProjectContentComponent::cleanAll()
{
    lastCrashMessage = String();

    if (childProcess != nullptr)
        childProcess->cleanAll();
    else if (Project* p = getProject())
        CompileEngineChildProcess::cleanAllCachedFilesForProject (*p);
}

void ProjectContentComponent::handleCrash (const String& message)
{
    lastCrashMessage = message.isEmpty() ? TRANS("JIT process stopped responding!")
                                         : (TRANS("JIT process crashed!") + ":\n\n" + message);

    if (project != nullptr)
    {
        setBuildEnabled (false);
        showBuildTab();
    }
}

bool ProjectContentComponent::isBuildEnabled() const
{
    return project != nullptr
            && ! LiveBuildProjectSettings::isBuildDisabled (*project)
            && ProjucerLicenses::getInstance()->hasLiveCodingLicence()
            && ProjucerLicenses::getInstance()->isLoggedIn();
}

void ProjectContentComponent::refreshTabsIfBuildStatusChanged()
{
    if (project != nullptr
         && (treeViewTabs.getNumTabs() < 3
              || isBuildEnabled() != isBuildTabEnabled()
              || ProjucerLicenses::getInstance()->isLoggedIn() != isBuildTabSuitableForLoggedInUser()))
        rebuildProjectTabs();
}

bool ProjectContentComponent::areWarningsEnabled() const
{
    return project != nullptr && ! LiveBuildProjectSettings::areWarningsDisabled (*project);
}

void ProjectContentComponent::updateWarningState()
{
    if (childProcess != nullptr)
        childProcess->errorList.setWarningsEnabled (areWarningsEnabled());
}

void ProjectContentComponent::toggleWarnings()
{
    if (project != nullptr)
    {
        LiveBuildProjectSettings::setWarningsDisabled (*project, areWarningsEnabled());
        updateWarningState();
    }
}

static ProjucerAppClasses::ErrorListComp* findErrorListComp (const TabbedComponent& tabs)
{
    if (BuildTabComponent* bt = findBuildTab (tabs))
        return bt->errorListComp;

    return nullptr;
}

void ProjectContentComponent::showNextError()
{
    if (ProjucerAppClasses::ErrorListComp* el = findErrorListComp (treeViewTabs))
    {
        showBuildTab();
        el->showNext();
    }
}

void ProjectContentComponent::showPreviousError()
{
    if (ProjucerAppClasses::ErrorListComp* el = findErrorListComp (treeViewTabs))
    {
        showBuildTab();
        el->showPrevious();
    }
}

void ProjectContentComponent::reinstantiateLivePreviewWindows()
{
    if (childProcess != nullptr)
        childProcess->reinstantiatePreviews();
}

void ProjectContentComponent::launchApp()
{
    if (childProcess != nullptr)
        childProcess->launchApp();
}

void ProjectContentComponent::killApp()
{
    if (childProcess != nullptr)
        childProcess->killApp();
}

void ProjectContentComponent::rebuildNow()
{
    if (childProcess != nullptr)
        childProcess->flushEditorChanges();
}

void ProjectContentComponent::globalFocusChanged (Component* focusedComponent)
{
    const bool nowForeground = (Process::isForegroundProcess()
                                  && (focusedComponent == this || isParentOf (focusedComponent)));

    if (nowForeground != isForeground)
    {
        isForeground = nowForeground;

        if (childProcess != nullptr)
            childProcess->processActivationChanged (isForeground);
    }
}

void ProjectContentComponent::timerCallback()
{
    if (! isBuildEnabled())
        killChildProcess();

    refreshTabsIfBuildStatusChanged();
}

ReferenceCountedObjectPtr<CompileEngineChildProcess> ProjectContentComponent::getChildProcess()
{
    if (childProcess == nullptr && isBuildEnabled())
    {
        childProcess = ProjucerApplication::getApp().childProcessCache->getOrCreate (*project);

        if (childProcess != nullptr)
            childProcess->setContinuousRebuild (isContinuousRebuildEnabled());
    }

    return childProcess;
}

void ProjectContentComponent::handleMissingSystemHeaders()
{
   #if JUCE_MAC
    const String tabMessage = "Compiler not available due to missing system headers\nPlease install a recent version of Xcode";
    const String alertWindowMessage = "Missing system headers\nPlease install a recent version of Xcode";
   #elif JUCE_WINDOWS
    const String tabMessage = "Compiler not available due to missing system headers\nPlease install a recent version of Visual Studio and the Windows Desktop SDK";
    const String alertWindowMessage = "Missing system headers\nPlease install a recent version of Visual Studio and the Windows Desktop SDK";
   #elif JUCE_LINUX
    const String tabMessage = "Compiler not available due to missing system headers\nPlease do a sudo apt-get install ...";
    const String alertWindowMessage = "Missing system headers\nPlease do sudo apt-get install ...";
   #endif

    setBuildEnabled (false);

    deleteProjectTabs();
    createProjectTabs();

    bool isLoggedIn = ProjucerLicenses::getInstance()->isLoggedIn();
    ProjucerDisabledComp* buildTab = new ProjucerDisabledComp (tabMessage, isLoggedIn, false, false, false, false);

    treeViewTabs.addTab ("Build", Colours::transparentBlack, buildTab, true);
    showBuildTab();

    AlertWindow::showMessageBox (AlertWindow::AlertIconType::WarningIcon,
                                 "Missing system headers", alertWindowMessage);
}
