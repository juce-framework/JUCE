/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../../Application/jucer_Headers.h"
#include "jucer_ProjectContentComponent.h"
#include "../../LiveBuildEngine/jucer_DownloadCompileEngineThread.h"
#include "../../LiveBuildEngine/jucer_CompileEngineSettings.h"

#include "Sidebar/jucer_TabComponents.h"
#include "Sidebar/jucer_ProjectTab.h"
#include "Sidebar/jucer_LiveBuildTab.h"

NewFileWizard::Type* createGUIComponentWizard();

//==============================================================================
ProjectContentComponent::LogoComponent::LogoComponent()
{
    if (auto svg = parseXML (BinaryData::background_logo_svg))
        logo = Drawable::createFromSVG (*svg);
}

void ProjectContentComponent::LogoComponent::paint (Graphics& g)
{
    g.setColour (findColour (defaultTextColourId));

    auto r = getLocalBounds();

    g.setFont (15.0f);
    g.drawFittedText (getVersionInfo(), r.removeFromBottom (50), Justification::centredBottom, 3);

    logo->drawWithin (g, r.withTrimmedBottom (r.getHeight() / 4).toFloat(),
                      RectanglePlacement (RectanglePlacement::centred), 1.0f);
}

String ProjectContentComponent::LogoComponent::getVersionInfo()
{
    return SystemStats::getJUCEVersion()
            + newLine
            + ProjucerApplication::getApp().getVersionDescription();
}

//==============================================================================
ProjectContentComponent::ContentViewport::ContentViewport (Component* content)
{
    addAndMakeVisible (viewport);
    viewport.setViewedComponent (content, true);
}

void ProjectContentComponent::ContentViewport::resized()
{
    viewport.setBounds (getLocalBounds());
}

//==============================================================================
ProjectContentComponent::ProjectContentComponent()
{
    setOpaque (true);
    setWantsKeyboardFocus (true);

    addAndMakeVisible (logoComponent);
    addAndMakeVisible (headerComponent);
    addAndMakeVisible (projectMessagesComponent);

    addAndMakeVisible (fileNameLabel);
    fileNameLabel.setJustificationType (Justification::centred);

    sidebarSizeConstrainer.setMinimumWidth (200);
    sidebarSizeConstrainer.setMaximumWidth (500);

    sidebarTabs.setOutline (0);
    sidebarTabs.getTabbedButtonBar().setMinimumTabScaleFactor (0.5);

    ProjucerApplication::getApp().openDocumentManager.addListener (this);

    isLiveBuildEnabled = getGlobalProperties().getBoolValue (Ids::liveBuildEnabled);
    getGlobalProperties().addChangeListener (this);
    liveBuildEnablementChanged (isLiveBuildEnabled);

    Desktop::getInstance().addFocusChangeListener (this);
}

ProjectContentComponent::~ProjectContentComponent()
{
    Desktop::getInstance().removeFocusChangeListener (this);
    killChildProcess();

    getGlobalProperties().removeChangeListener (this);
    ProjucerApplication::getApp().openDocumentManager.removeListener (this);

    setProject (nullptr);
    removeChildComponent (&bubbleMessage);
}

void ProjectContentComponent::paint (Graphics& g)
{
    g.fillAll (findColour (backgroundColourId));
}

void ProjectContentComponent::resized()
{
    auto r = getLocalBounds();

    r.removeFromRight (10);
    r.removeFromLeft (15);
    r.removeFromTop (5);

    projectMessagesComponent.setBounds (r.removeFromBottom (40).withWidth (100).reduced (0, 5));
    headerComponent.setBounds (r.removeFromTop (40));

    r.removeFromTop (10);

    auto sidebarArea = r.removeFromLeft (sidebarTabs.getWidth() != 0 ? sidebarTabs.getWidth()
                                                                     : r.getWidth() / 4);

    if (sidebarTabs.isVisible())
        sidebarTabs.setBounds (sidebarArea);

    if (resizerBar != nullptr)
        resizerBar->setBounds (r.withWidth (4));

    headerComponent.sidebarTabsWidthChanged (sidebarTabs.getWidth());

    if (contentView != nullptr)
    {
        fileNameLabel.setBounds (r.removeFromTop (15));
        contentView->setBounds (r);
    }

    logoComponent.setBounds (r.reduced (r.getWidth() / 6, r.getHeight() / 6));
}

void ProjectContentComponent::lookAndFeelChanged()
{
    repaint();

    if (translationTool != nullptr)
        translationTool->repaint();
}

void ProjectContentComponent::childBoundsChanged (Component* child)
{
    if (child == &sidebarTabs)
        resized();
}

void ProjectContentComponent::setProject (Project* newProject)
{
    if (project != newProject)
    {
        lastCrashMessage = {};
        killChildProcess();

        if (project != nullptr)
            project->removeChangeListener (this);

        contentView.reset();
        resizerBar.reset();

        deleteProjectTabs();
        project = newProject;

        rebuildProjectUI();
    }
}

//==============================================================================
static LiveBuildTab* findBuildTab (const TabbedComponent& tabs)
{
    return dynamic_cast<LiveBuildTab*> (tabs.getTabContentComponent (1));
}

bool ProjectContentComponent::isBuildTabEnabled() const
{
    auto bt = findBuildTab (sidebarTabs);

    return bt != nullptr && bt->isEnabled;
}

void ProjectContentComponent::createProjectTabs()
{
    jassert (project != nullptr);

    auto tabColour = Colours::transparentBlack;

    sidebarTabs.addTab (getProjectTabName(), tabColour, new ProjectTab (project), true);

    if (isLiveBuildEnabled)
    {
        CompileEngineChildProcess::Ptr childProc (getChildProcess());
        sidebarTabs.addTab (getBuildTabName(), tabColour, new LiveBuildTab (childProc, lastCrashMessage), true);

        if (childProc != nullptr)
        {
            childProc->crashHandler = [this] (const String& m) { this->handleCrash (m); };

            sidebarTabs.getTabbedButtonBar().getTabButton (1)->setExtraComponent (new BuildStatusTabComp (childProc->errorList,
                                                                                                          childProc->activityList),
                                                                                                          TabBarButton::afterText);
        }
    }
}

void ProjectContentComponent::deleteProjectTabs()
{
    if (project != nullptr && sidebarTabs.getNumTabs() > 0)
    {
        auto& settings = project->getStoredProperties();

        if (sidebarTabs.getWidth() > 0)
            settings.setValue ("projectPanelWidth", sidebarTabs.getWidth());

        settings.setValue ("lastViewedTabIndex", sidebarTabs.getCurrentTabIndex());

        for (int i = 0; i < 3; ++i)
            settings.setValue ("projectTabPanelHeight" + String (i),
                               getProjectTab()->getPanelHeightProportion (i));
    }

    sidebarTabs.clearTabs();
}

void ProjectContentComponent::rebuildProjectUI()
{
    deleteProjectTabs();

    if (project != nullptr)
    {
        addAndMakeVisible (sidebarTabs);
        createProjectTabs();

        //==============================================================================
        auto& settings = project->getStoredProperties();

        auto lastTreeWidth = settings.getValue ("projectPanelWidth").getIntValue();
        if (lastTreeWidth < 150)
            lastTreeWidth = 240;

        sidebarTabs.setBounds (0, 0, lastTreeWidth, getHeight());

        auto lastTabIndex = settings.getValue ("lastViewedTabIndex", "0").getIntValue();

        if (lastTabIndex >= sidebarTabs.getNumTabs())
            lastTabIndex = 0;

        sidebarTabs.setCurrentTabIndex (lastTabIndex);

        auto* projectTab = getProjectTab();
        for (int i = 2; i >= 0; --i)
            projectTab->setPanelHeightProportion (i, settings.getValue ("projectTabPanelHeight" + String (i), "1")
                                                             .getFloatValue());

        //==============================================================================
        resizerBar.reset (new ResizableEdgeComponent (&sidebarTabs, &sidebarSizeConstrainer,
                                                      ResizableEdgeComponent::rightEdge));
        addAndMakeVisible (resizerBar.get());
        resizerBar->setAlwaysOnTop (true);

        project->addChangeListener (this);

        updateMissingFileStatuses();

        headerComponent.setVisible (true);
        headerComponent.setCurrentProject (project);

        projectMessagesComponent.setVisible (true);
    }
    else
    {
        sidebarTabs.setVisible (false);
        headerComponent.setVisible (false);
        projectMessagesComponent.setVisible (false);
    }

    projectMessagesComponent.setProject (project);

    resized();
}

void ProjectContentComponent::saveTreeViewState()
{
}

void ProjectContentComponent::saveOpenDocumentList()
{
    if (project != nullptr)
    {
        std::unique_ptr<XmlElement> xml (recentDocumentList.createXML());

        if (xml != nullptr)
            project->getStoredProperties().setValue ("lastDocs", xml.get());
    }
}

void ProjectContentComponent::reloadLastOpenDocuments()
{
    if (project != nullptr)
    {
        if (auto xml = project->getStoredProperties().getXmlValue ("lastDocs"))
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

void ProjectContentComponent::changeListenerCallback (ChangeBroadcaster* broadcaster)
{
    if (broadcaster == project)
    {
        updateMissingFileStatuses();
    }
    else if (broadcaster == &getGlobalProperties())
    {
        auto isEnabled = ProjucerApplication::getApp().isLiveBuildEnabled();

        if (isLiveBuildEnabled != isEnabled)
            liveBuildEnablementChanged (isEnabled);
    }
}

void ProjectContentComponent::refreshProjectTreeFileStatuses()
{
    if (auto* projectTab = getProjectTab())
        if (auto* fileTree = projectTab->getFileTreePanel())
            fileTree->repaint();
}

void ProjectContentComponent::updateMissingFileStatuses()
{
    if (auto* pTab = getProjectTab())
        if (auto* tree = pTab->getFileTreePanel())
            tree->updateMissingFileStatuses();
}

bool ProjectContentComponent::showEditorForFile (const File& f, bool grabFocus)
{
    if (getCurrentFile() == f
            || showDocument (ProjucerApplication::getApp().openDocumentManager.openFile (project, f), grabFocus))
    {
        fileNameLabel.setText (f.getFileName(), dontSendNotification);
        return true;
    }

    return false;
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

    auto opened = setEditorComponent (doc->createEditor(), doc);

    if (opened && grabFocus && isShowing())
        contentView->grabKeyboardFocus();

    return opened;
}

void ProjectContentComponent::hideEditor()
{
    currentDocument = nullptr;
    contentView.reset();

    fileNameLabel.setVisible (false);

    ProjucerApplication::getCommandManager().commandStatusChanged();
    resized();
}

void ProjectContentComponent::hideDocument (OpenDocumentManager::Document* doc)
{
    if (doc == currentDocument)
    {
        if (auto* replacement = recentDocumentList.getClosestPreviousDocOtherThan (doc))
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
        contentView.reset();

        if (doc == nullptr)
        {
            auto* viewport = new ContentViewport (editor);

            contentView.reset (viewport);
            currentDocument = nullptr;
            fileNameLabel.setVisible (false);

            addAndMakeVisible (viewport);
        }
        else
        {
            contentView.reset (editor);
            currentDocument = doc;
            fileNameLabel.setText (doc->getFile().getFileName(), dontSendNotification);
            fileNameLabel.setVisible (true);

            addAndMakeVisible (editor);
        }

        resized();

        ProjucerApplication::getCommandManager().commandStatusChanged();
        return true;
    }

    return false;
}

Component* ProjectContentComponent::getEditorComponentContent() const
{
    if (contentView != nullptr)
        if (auto* vp = dynamic_cast<ContentViewport*> (contentView.get()))
            return vp->viewport.getViewedComponent();

    return nullptr;
}

void ProjectContentComponent::closeDocument()
{
    if (currentDocument != nullptr)
        ProjucerApplication::getApp().openDocumentManager
                                     .closeDocument (currentDocument, OpenDocumentManager::SaveIfNeeded::yes);
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

        refreshProjectTreeFileStatuses();
    }
    else
    {
        saveProject();
    }
}

void ProjectContentComponent::saveAs()
{
    if (currentDocument != nullptr)
    {
        if (! currentDocument->saveAs())
            showSaveWarning (currentDocument);

        refreshProjectTreeFileStatuses();
    }
}

bool ProjectContentComponent::goToPreviousFile()
{
    auto* doc = recentDocumentList.getCurrentDocument();

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
        auto file = currentDocument->getCounterpartFile();

        if (file.exists())
            return showEditorForFile (file, true);
    }

    return false;
}

bool ProjectContentComponent::saveProject()
{
    if (project != nullptr)
        return (project->save (true, true) == FileBasedDocument::savedOk);

    return false;
}

void ProjectContentComponent::closeProject()
{
    if (auto* mw = findParentComponentOfClass<MainWindow>())
        mw->closeCurrentProject (OpenDocumentManager::SaveIfNeeded::yes);
}

void ProjectContentComponent::showProjectSettings()
{
    setEditorComponent (new ProjectSettingsComponent (*project), nullptr);
}

void ProjectContentComponent::showCurrentExporterSettings()
{
    if (auto selected = headerComponent.getSelectedExporter())
        showExporterSettings (selected->getUniqueName());
}

void ProjectContentComponent::showExporterSettings (const String& exporterName)
{
    if (exporterName.isEmpty())
        return;

    showExportersPanel();

    if (auto* exportersPanel = getProjectTab()->getExportersTreePanel())
    {
        if (auto* exporters = dynamic_cast<TreeItemTypes::ExportersTreeRoot*> (exportersPanel->rootItem.get()))
        {
            for (auto i = exporters->getNumSubItems(); i >= 0; --i)
            {
                if (auto* e = dynamic_cast<TreeItemTypes::ExporterItem*> (exporters->getSubItem (i)))
                {
                    if (e->getDisplayName() == exporterName)
                    {
                        if (e->isSelected())
                            e->setSelected (false, true);

                        e->setSelected (true, true);
                    }
                }
            }
        }
    }
}

void ProjectContentComponent::showModule (const String& moduleID)
{
    showModulesPanel();

    if (auto* modsPanel = getProjectTab()->getModuleTreePanel())
    {
        if (auto* mods = dynamic_cast<TreeItemTypes::EnabledModulesItem*> (modsPanel->rootItem.get()))
        {
            for (auto i = mods->getNumSubItems(); --i >= 0;)
            {
                if (auto* m = dynamic_cast<TreeItemTypes::ModuleItem*> (mods->getSubItem (i)))
                {
                    if (m->moduleID == moduleID)
                    {
                        if (m->isSelected())
                            m->setSelected (false, true);

                        m->setSelected (true, true);
                    }
                }
            }
        }
    }
}

void ProjectContentComponent::showLiveBuildSettings()
{
    setEditorComponent (new LiveBuildSettingsComponent (*project), nullptr);
}

StringArray ProjectContentComponent::getExportersWhichCanLaunch() const
{
    StringArray s;

    if (project != nullptr)
        for (Project::ExporterIterator exporter (*project); exporter.next();)
            if (exporter->canLaunchProject())
                s.add (exporter->getUniqueName());

    return s;
}

void ProjectContentComponent::openInSelectedIDE (bool saveFirst)
{
    if (project != nullptr)
        if (auto selectedExporter = headerComponent.getSelectedExporter())
            project->openProjectInIDE (*selectedExporter, saveFirst);
}

void ProjectContentComponent::showNewExporterMenu()
{
    if (project != nullptr)
    {
        PopupMenu menu;

        menu.addSectionHeader ("Create a new export target:");

        SafePointer<ProjectContentComponent> safeThis (this);

        for (auto& exporterInfo : ProjectExporter::getExporterTypeInfos())
        {
            PopupMenu::Item item;

            item.itemID = -1;
            item.text = exporterInfo.displayName;

            item.image = [exporterInfo]
            {
                auto drawableImage = std::make_unique<DrawableImage>();
                drawableImage->setImage (exporterInfo.icon);

                return drawableImage;
            }();

            item.action = [safeThis, exporterInfo]
            {
                if (safeThis != nullptr)
                    if (auto* p = safeThis->getProject())
                        p->addNewExporter (exporterInfo.identifier);
            };

            menu.addItem (item);
        }

        menu.showMenuAsync ({});
    }
}

void ProjectContentComponent::deleteSelectedTreeItems()
{
    if (auto*  tree = getProjectTab()->getTreeWithSelectedItems())
        tree->deleteSelectedItems();
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
                                translationTool, true,
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

static bool reinvokeCommandAfterCancellingModalComps (const ApplicationCommandTarget::InvocationInfo& info)
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
    commands.addArray ({ CommandIDs::saveProject,
                         CommandIDs::closeProject,
                         CommandIDs::saveDocument,
                         CommandIDs::saveDocumentAs,
                         CommandIDs::closeDocument,
                         CommandIDs::goToPreviousDoc,
                         CommandIDs::goToNextDoc,
                         CommandIDs::goToCounterpart,
                         CommandIDs::showProjectSettings,
                         CommandIDs::showProjectTab,
                         CommandIDs::showBuildTab,
                         CommandIDs::showFileExplorerPanel,
                         CommandIDs::showModulesPanel,
                         CommandIDs::showExportersPanel,
                         CommandIDs::showExporterSettings,
                         CommandIDs::openInIDE,
                         CommandIDs::saveAndOpenInIDE,
                         CommandIDs::createNewExporter,
                         CommandIDs::deleteSelectedItem,
                         CommandIDs::showTranslationTool,
                         CommandIDs::cleanAll,
                         CommandIDs::toggleBuildEnabled,
                         CommandIDs::buildNow,
                         CommandIDs::toggleContinuousBuild,
                         CommandIDs::launchApp,
                         CommandIDs::killApp,
                         CommandIDs::reinstantiateComp,
                         CommandIDs::showWarnings,
                         CommandIDs::nextError,
                         CommandIDs::prevError,
                         CommandIDs::addNewGUIFile });
}

void ProjectContentComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    String documentName;
    if (currentDocument != nullptr)
        documentName = " '" + currentDocument->getName().substring (0, 32) + "'";

   #if JUCE_MAC
    auto cmdCtrl = (ModifierKeys::ctrlModifier | ModifierKeys::commandModifier);
   #else
    auto cmdCtrl = (ModifierKeys::ctrlModifier | ModifierKeys::altModifier);
   #endif

    switch (commandID)
    {
    case CommandIDs::saveProject:
        result.setInfo ("Save Project",
                        "Saves the current project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr && ! project->isSaveAndExportDisabled() && ! project->isCurrentlySaving());
        result.defaultKeypresses.add ({ 'p', ModifierKeys::commandModifier, 0 });
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
        result.setActive (currentDocument != nullptr || (project != nullptr && ! project->isCurrentlySaving()));
        result.defaultKeypresses.add ({ 's', ModifierKeys::commandModifier, 0 });
        break;

    case CommandIDs::saveDocumentAs:
        result.setInfo ("Save As...",
                        "Saves the current document to a new location",
                        CommandCategories::general, 0);
        result.setActive (currentDocument != nullptr);
        result.defaultKeypresses.add ({ 's', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        break;

    case CommandIDs::closeDocument:
        result.setInfo ("Close" + documentName,
                        "Closes the current document",
                        CommandCategories::general, 0);
        result.setActive (contentView != nullptr);
        result.defaultKeypresses.add ({ 'w', cmdCtrl, 0 });
        break;

    case CommandIDs::goToPreviousDoc:
        result.setInfo ("Previous Document",
                        "Go to previous document",
                        CommandCategories::general, 0);
        result.setActive (recentDocumentList.canGoToPrevious());
        result.defaultKeypresses.add ({ KeyPress::leftKey, cmdCtrl, 0 });
        break;

    case CommandIDs::goToNextDoc:
        result.setInfo ("Next Document",
                        "Go to next document",
                        CommandCategories::general, 0);
        result.setActive (recentDocumentList.canGoToNext());
        result.defaultKeypresses.add ({ KeyPress::rightKey, cmdCtrl, 0 });
        break;

    case CommandIDs::goToCounterpart:
        result.setInfo ("Open Counterpart File",
                        "Open corresponding header or cpp file",
                        CommandCategories::general, 0);
        result.setActive (canGoToCounterpart());
        result.defaultKeypresses.add ({ KeyPress::upKey, cmdCtrl, 0 });
        break;

    case CommandIDs::showProjectSettings:
        result.setInfo ("Show Project Settings",
                        "Shows the main project options page",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'x', cmdCtrl, 0 });
        break;

    case CommandIDs::showProjectTab:
        result.setInfo ("Show Project Tab",
                        "Shows the tab containing the project information",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'p', cmdCtrl, 0 });
        break;

    case CommandIDs::showBuildTab:
        result.setInfo ("Show Build Tab",
                        "Shows the tab containing the build panel",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr && isLiveBuildEnabled);
        result.defaultKeypresses.add ({ 'b', cmdCtrl, 0 });
        break;

    case CommandIDs::showFileExplorerPanel:
        result.setInfo ("Show File Explorer Panel",
                        "Shows the panel containing the tree of files for this project",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'f', cmdCtrl, 0 });
        break;

    case CommandIDs::showModulesPanel:
        result.setInfo ("Show Modules Panel",
                        "Shows the panel containing the project's list of modules",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'm', cmdCtrl, 0 });
        break;

    case CommandIDs::showExportersPanel:
        result.setInfo ("Show Exporters Panel",
                        "Shows the panel containing the project's list of exporters",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'e', cmdCtrl, 0 });
        break;

    case CommandIDs::showExporterSettings:
        result.setInfo ("Show Exporter Settings",
                        "Shows the settings page for the currently selected exporter",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.defaultKeypresses.add ({ 'e', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        break;

    case CommandIDs::openInIDE:
        result.setInfo ("Open in IDE...",
                        "Launches the project in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (ProjectExporter::canProjectBeLaunched (project) && ! project->isSaveAndExportDisabled());
        break;

    case CommandIDs::saveAndOpenInIDE:
        result.setInfo ("Save Project and Open in IDE...",
                        "Saves the project and launches it in an external IDE",
                        CommandCategories::general, 0);
        result.setActive (ProjectExporter::canProjectBeLaunched (project) && ! project->isSaveAndExportDisabled() && ! project->isCurrentlySaving());
        result.defaultKeypresses.add ({ 'l', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        break;

    case CommandIDs::createNewExporter:
        result.setInfo ("Create New Exporter...",
                        "Creates a new exporter for a compiler type",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        break;

    case CommandIDs::deleteSelectedItem:
        result.setInfo ("Delete Selected File",
                        String(),
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ KeyPress::deleteKey, 0, 0 });
        result.defaultKeypresses.add ({ KeyPress::backspaceKey, 0, 0 });
        result.setActive (sidebarTabs.getCurrentTabIndex() == 0);
        break;

    case CommandIDs::showTranslationTool:
        result.setInfo ("Translation File Builder",
                        "Shows the translation file helper tool",
                        CommandCategories::general, 0);
        break;

    case CommandIDs::cleanAll:
        result.setInfo ("Clean All",
                        "Cleans all intermediate files",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ 'k', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        result.setActive (project != nullptr);
        break;

    case CommandIDs::toggleBuildEnabled:
        result.setInfo ("Enable Compilation",
                        "Enables/disables the compiler",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ 'b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        result.setActive (project != nullptr);
        result.setTicked (childProcess != nullptr);
        break;

    case CommandIDs::buildNow:
        result.setInfo ("Build Now",
                        "Recompiles any out-of-date files and updates the JIT engine",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ 'b', ModifierKeys::commandModifier, 0 });
        result.setActive (childProcess != nullptr);
        break;

    case CommandIDs::toggleContinuousBuild:
        result.setInfo ("Enable Continuous Recompiling",
                        "Continuously recompiles any changes made in code editors",
                        CommandCategories::general, 0);
        result.setActive (childProcess != nullptr);
        result.setTicked (isContinuousRebuildEnabled());
        break;

    case CommandIDs::launchApp:
        result.setInfo ("Launch Application",
                        "Invokes the app's main() function",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ 'r', ModifierKeys::commandModifier, 0 });
        result.setActive (childProcess != nullptr && childProcess->canLaunchApp());
        break;

    case CommandIDs::killApp:
        result.setInfo ("Stop Application",
                        "Kills the app if it's running",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ '.', ModifierKeys::commandModifier, 0 });
        result.setActive (childProcess != nullptr && childProcess->canKillApp());
        break;

    case CommandIDs::reinstantiateComp:
        result.setInfo ("Re-instantiate Components",
                        "Re-loads any component editors that are open",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ 'r', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0 });
        result.setActive (childProcess != nullptr);
        break;

    case CommandIDs::showWarnings:
        result.setInfo ("Show Warnings",
                        "Shows or hides compilation warnings",
                        CommandCategories::general, 0);
        result.setActive (project != nullptr);
        result.setTicked (areWarningsEnabled());
        break;

    case CommandIDs::nextError:
        result.setInfo ("Highlight next error",
                        "Jumps to the next error or warning",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ '\'', ModifierKeys::commandModifier, 0 });
        result.setActive (childProcess != nullptr && ! childProcess->errorList.isEmpty());
        break;

    case CommandIDs::prevError:
        result.setInfo ("Highlight previous error",
                        "Jumps to the last error or warning",
                        CommandCategories::general, 0);
        result.defaultKeypresses.add ({ '\"', ModifierKeys::commandModifier, 0 });
        result.setActive (childProcess != nullptr && ! childProcess->errorList.isEmpty());
        break;

    case CommandIDs::addNewGUIFile:
        result.setInfo ("Add new GUI Component...",
                        "Adds a new GUI Component file to the project",
                        CommandCategories::general,
                        (! ProjucerApplication::getApp().isGUIEditorEnabled() ? ApplicationCommandInfo::isDisabled : 0));
        break;

    default:
        break;
    }
}

bool ProjectContentComponent::perform (const InvocationInfo& info)
{
    // don't allow the project to be saved again if it's currently saving
    if (isSaveCommand (info.commandID) && (project != nullptr && project->isCurrentlySaving()))
        return false;

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
        case CommandIDs::saveProject:               saveProject();      break;
        case CommandIDs::closeProject:              closeProject();     break;
        case CommandIDs::saveDocument:              saveDocument();     break;
        case CommandIDs::saveDocumentAs:            saveAs();           break;
        case CommandIDs::closeDocument:             closeDocument();    break;
        case CommandIDs::goToPreviousDoc:           goToPreviousFile(); break;
        case CommandIDs::goToNextDoc:               goToNextFile();     break;
        case CommandIDs::goToCounterpart:           goToCounterpart();  break;

        case CommandIDs::showProjectSettings:       showProjectSettings();         break;
        case CommandIDs::showProjectTab:            showProjectTab();              break;
        case CommandIDs::showBuildTab:              showBuildTab();                break;
        case CommandIDs::showFileExplorerPanel:     showFilesPanel();              break;
        case CommandIDs::showModulesPanel:          showModulesPanel();            break;
        case CommandIDs::showExportersPanel:        showExportersPanel();          break;
        case CommandIDs::showExporterSettings:      showCurrentExporterSettings(); break;

        case CommandIDs::openInIDE:                 openInSelectedIDE (false); break;
        case CommandIDs::saveAndOpenInIDE:          openInSelectedIDE (true);  break;

        case CommandIDs::createNewExporter:         showNewExporterMenu(); break;

        case CommandIDs::deleteSelectedItem:        deleteSelectedTreeItems(); break;

        case CommandIDs::showTranslationTool:       showTranslationTool(); break;

        case CommandIDs::cleanAll:                  cleanAll();                                                   break;
        case CommandIDs::toggleBuildEnabled:        setBuildEnabled (! isBuildEnabled());                         break;
        case CommandIDs::buildNow:                  rebuildNow();                                                 break;
        case CommandIDs::toggleContinuousBuild:     setContinuousRebuildEnabled (! isContinuousRebuildEnabled()); break;
        case CommandIDs::launchApp:                 launchApp();                                                  break;
        case CommandIDs::killApp:                   killApp();                                                    break;
        case CommandIDs::reinstantiateComp:         reinstantiateLivePreviewWindows();                            break;
        case CommandIDs::showWarnings:              toggleWarnings();                                             break;
        case CommandIDs::nextError:                 showNextError();                                              break;
        case CommandIDs::prevError:                 showPreviousError();                                          break;

        case CommandIDs::addNewGUIFile:             addNewGUIFile();                                              break;

        default:
            return false;
    }

    return true;
}

bool ProjectContentComponent::isSaveCommand (const CommandID id)
{
    return (id == CommandIDs::saveProject || id == CommandIDs::saveDocument || id == CommandIDs::saveAndOpenInIDE);
}

void ProjectContentComponent::getSelectedProjectItemsBeingDragged (const DragAndDropTarget::SourceDetails& dragSourceDetails,
                                                                   OwnedArray<Project::Item>& selectedNodes)
{
    TreeItemTypes::FileTreeItemBase::getSelectedProjectItemsBeingDragged (dragSourceDetails, selectedNodes);
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

void ProjectContentComponent::setBuildEnabled (bool isEnabled, bool displayError)
{
    if (project != nullptr && isEnabled != isBuildEnabled())
    {
        if (! displayError)
            lastCrashMessage = {};

        project->getCompileEngineSettings().setBuildEnabled (isEnabled);
        killChildProcess();
        refreshTabsIfBuildStatusChanged();
    }
}

void ProjectContentComponent::cleanAll()
{
    lastCrashMessage = {};

    if (childProcess != nullptr)
        childProcess->cleanAll();
    else if (auto* p = getProject())
        CompileEngineChildProcess::cleanAllCachedFilesForProject (*p);
}

void ProjectContentComponent::handleCrash (const String& message)
{
    lastCrashMessage = message.isEmpty() ? TRANS("JIT process stopped responding!")
                                         : (TRANS("JIT process crashed!") + ":\n\n" + message);

    if (project != nullptr)
    {
        setBuildEnabled (false, true);
        showBuildTab();
    }
}

bool ProjectContentComponent::isBuildEnabled() const
{
    return isLiveBuildEnabled
          && project != nullptr
          && project->getCompileEngineSettings().isBuildEnabled()
          && CompileEngineDLL::getInstance()->isLoaded();
}

void ProjectContentComponent::refreshTabsIfBuildStatusChanged()
{
    if (project != nullptr
        && isLiveBuildEnabled
        && (sidebarTabs.getNumTabs() < 2 || isBuildEnabled() != isBuildTabEnabled()))
    {
        rebuildProjectUI();
    }
}

bool ProjectContentComponent::areWarningsEnabled() const
{
    return project != nullptr && project->getCompileEngineSettings().areWarningsEnabled();
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
        project->getCompileEngineSettings().setWarningsEnabled (! areWarningsEnabled());
        updateWarningState();
    }
}

static ProjucerAppClasses::ErrorListComp* findErrorListComp (const TabbedComponent& tabs)
{
    if (auto* bt = findBuildTab (tabs))
        return bt->errorListComp;

    return nullptr;
}

void ProjectContentComponent::showNextError()
{
    if (auto* el = findErrorListComp (sidebarTabs))
    {
        showBuildTab();
        el->showNext();
    }
}

void ProjectContentComponent::showPreviousError()
{
    if (auto* el = findErrorListComp (sidebarTabs))
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

void ProjectContentComponent::addNewGUIFile()
{
    if (project != nullptr)
    {
        std::unique_ptr<NewFileWizard::Type> wizard (createGUIComponentWizard());
        wizard->createNewFile (*project, project->getMainGroup());
    }
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
    auto nowForeground = (Process::isForegroundProcess()
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

void ProjectContentComponent::liveBuildEnablementChanged (bool isEnabled)
{
    isLiveBuildEnabled = isEnabled;

    if (isLiveBuildEnabled)
    {
        startTimer (1600);
    }
    else
    {
        stopTimer();
        killChildProcess();
    }

    rebuildProjectUI();
    headerComponent.liveBuildEnablementChanged (isLiveBuildEnabled);
}

bool ProjectContentComponent::isContinuousRebuildEnabled()
{
    return project != nullptr && project->getCompileEngineSettings().isContinuousRebuildEnabled();
}

void ProjectContentComponent::setContinuousRebuildEnabled (bool b)
{
    if (project != nullptr && childProcess != nullptr)
    {
        project->getCompileEngineSettings().setContinuousRebuildEnabled (b);
        ProjucerApplication::getCommandManager().commandStatusChanged();
    }
}

ReferenceCountedObjectPtr<CompileEngineChildProcess> ProjectContentComponent::getChildProcess()
{
    if (childProcess == nullptr && isBuildEnabled())
        childProcess = ProjucerApplication::getApp().childProcessCache->getOrCreate (*project);

    return childProcess;
}

void ProjectContentComponent::handleMissingSystemHeaders()
{
   #if JUCE_MAC
    String tabMessage ("Compiler not available due to missing system headers\nPlease install a recent version of Xcode");
    String alertWindowMessage ("Missing system headers\nPlease install a recent version of Xcode");
   #elif JUCE_WINDOWS
    String tabMessage ("Compiler not available due to missing system headers\nPlease install a recent version of Visual Studio and the Windows Desktop SDK");
    String alertWindowMessage ("Missing system headers\nPlease install a recent version of Visual Studio and the Windows Desktop SDK");
   #elif JUCE_LINUX || JUCE_BSD
    String tabMessage ("Compiler not available due to missing system headers\nPlease install using your package manager");
    String alertWindowMessage ("Missing system headers\nPlease install using your package manager");
   #endif

    setBuildEnabled (false, true);

    deleteProjectTabs();
    createProjectTabs();

    if (auto* bt = getLiveBuildTab())
    {
        bt->isEnabled = false;
        bt->errorMessage = tabMessage;
    }

    showBuildTab();

    AlertWindow::showMessageBox (AlertWindow::AlertIconType::WarningIcon,
                                 "Missing system headers", alertWindowMessage);
}

//==============================================================================
void ProjectContentComponent::showProjectPanel (const int index)
{
    showProjectTab();

    if (auto* pTab = getProjectTab())
        pTab->showPanel (index);
}

ProjectTab* ProjectContentComponent::getProjectTab()
{
    return dynamic_cast<ProjectTab*> (sidebarTabs.getTabContentComponent (0));
}

LiveBuildTab* ProjectContentComponent::getLiveBuildTab()
{
    return dynamic_cast<LiveBuildTab*> (sidebarTabs.getTabContentComponent (1));
}
