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

#include "../Application/jucer_Headers.h"
#include "jucer_Application.h"
#include "jucer_MainWindow.h"
#include "StartPage/jucer_StartPageComponent.h"
#include "../Utility/UI/jucer_JucerTreeViewBase.h"
#include "../ProjectSaving/jucer_ProjectSaver.h"
#include "../Project/UI/jucer_ProjectContentComponent.h"

//==============================================================================
class BlurOverlayWithComponent final : public Component,
                                       private ComponentMovementWatcher,
                                       private AsyncUpdater
{
public:
    BlurOverlayWithComponent (MainWindow& window, std::unique_ptr<Component> comp)
        : ComponentMovementWatcher (&window),
          mainWindow (window),
          componentToShow (std::move (comp))
    {
        kernel.createGaussianBlur (1.25f);

        addAndMakeVisible (*componentToShow);

        setAlwaysOnTop (true);
        setOpaque (true);
        setVisible (true);

        static_cast<Component&> (mainWindow).addChildComponent (this);
        handleComponentMovedOrResized();

        enterModalState();
    }

    void resized() override
    {
        setBounds (mainWindow.getLocalBounds());
        componentToShow->centreWithSize (componentToShow->getWidth(), componentToShow->getHeight());
        refreshBackgroundImage();
    }

    void paint (Graphics& g) override
    {
        g.drawImage (componentImage, getLocalBounds().toFloat());
    }

private:
    void componentPeerChanged() override                {}

    void componentVisibilityChanged() override          {}
    using ComponentMovementWatcher::componentVisibilityChanged;

    void handleComponentMovedOrResized()                { triggerAsyncUpdate(); }

    void componentMovedOrResized (bool, bool) override  { handleComponentMovedOrResized(); }
    using ComponentMovementWatcher::componentMovedOrResized;

    void handleAsyncUpdate() override                   { resized(); }

    void lookAndFeelChanged() override
    {
        refreshBackgroundImage();
        repaint();
    }

    void refreshBackgroundImage()
    {
        setAlwaysOnTop (false);
        toBack();

        auto parentBounds = mainWindow.getBounds();

        componentImage = mainWindow.createComponentSnapshot (mainWindow.getLocalBounds())
                                   .rescaled (roundToInt ((float) parentBounds.getWidth() / 1.75f),
                                              roundToInt ((float) parentBounds.getHeight() / 1.75f));

        kernel.applyToImage (componentImage, componentImage, getLocalBounds());

        setAlwaysOnTop (true);
        toFront (true);
    }

    //==============================================================================
    MainWindow& mainWindow;
    std::unique_ptr<Component> componentToShow;

    ImageConvolutionKernel kernel { 3 };
    Image componentImage;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlurOverlayWithComponent)
};

//==============================================================================
MainWindow::MainWindow()
    : DocumentWindow (ProjucerApplication::getApp().getApplicationName(),
                      ProjucerApplication::getApp().lookAndFeel.getCurrentColourScheme()
                                                   .getUIColour (LookAndFeel_V4::ColourScheme::UIColour::windowBackground),
                      DocumentWindow::allButtons,
                      false)
{
    setUsingNativeTitleBar (true);
    setResizable (true, false);
    setResizeLimits (600, 500, 32000, 32000);

   #if ! JUCE_MAC
    setMenuBar (ProjucerApplication::getApp().getMenuModel());
   #endif

    createProjectContentCompIfNeeded();

    auto& commandManager = ProjucerApplication::getCommandManager();

    auto registerAllAppCommands = [&]
    {
        commandManager.registerAllCommandsForTarget (this);
        commandManager.registerAllCommandsForTarget (getProjectContentComponent());
    };

    auto updateAppKeyMappings = [&]
    {
        commandManager.getKeyMappings()->resetToDefaultMappings();

        if (auto keys = getGlobalProperties().getXmlValue ("keyMappings"))
            commandManager.getKeyMappings()->restoreFromXml (*keys);

        addKeyListener (commandManager.getKeyMappings());
    };

    registerAllAppCommands();
    updateAppKeyMappings();

    setWantsKeyboardFocus (false);
    getLookAndFeel().setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);

    projectNameValue.addListener (this);

    centreWithSize (800, 600);
}

MainWindow::~MainWindow()
{
   #if ! JUCE_MAC
    setMenuBar (nullptr);
   #endif

    removeKeyListener (ProjucerApplication::getCommandManager().getKeyMappings());

    // save the current size and position to our settings file..
    getGlobalProperties().setValue ("lastMainWindowPos", getWindowStateAsString());

    clearContentComponent();
}

void MainWindow::createProjectContentCompIfNeeded()
{
    if (getProjectContentComponent() == nullptr)
    {
        clearContentComponent();
        setContentOwned (new ProjectContentComponent(), false);
    }
}

void MainWindow::updateTitleBarIcon()
{
    if (auto* peer = getPeer())
    {
        if (currentProject != nullptr)
        {
            peer->setRepresentedFile (currentProject->getFile());
            peer->setIcon (ImageCache::getFromMemory (BinaryData::juce_icon_png, BinaryData::juce_icon_pngSize));
        }
        else
        {
            peer->setRepresentedFile ({});
        }
    }
}

void MainWindow::makeVisible()
{
    setVisible (true);
    addToDesktop();
    restoreWindowPosition();
    updateTitleBarIcon();
    getContentComponent()->grabKeyboardFocus();
}

ProjectContentComponent* MainWindow::getProjectContentComponent() const
{
    return dynamic_cast<ProjectContentComponent*> (getContentComponent());
}

void MainWindow::closeButtonPressed()
{
    ProjucerApplication::getApp().mainWindowList.closeWindow (this);
}

void MainWindow::closeCurrentProject (OpenDocumentManager::SaveIfNeeded askUserToSave, std::function<void (bool)> callback)
{
    if (currentProject == nullptr)
    {
        NullCheckedInvocation::invoke (callback, true);

        return;
    }

    currentProject->getStoredProperties().setValue (getProjectWindowPosName(), getWindowStateAsString());

    if (auto* pcc = getProjectContentComponent())
    {
        pcc->saveOpenDocumentList();
        pcc->hideEditor();
    }

    ProjucerApplication::getApp().openDocumentManager
        .closeAllDocumentsUsingProjectAsync (*currentProject,
                                             askUserToSave,
                                             [parent = SafePointer<MainWindow> { this }, askUserToSave, callback] (bool closedSuccessfully)
    {
        if (parent == nullptr)
            return;

        if (! closedSuccessfully)
        {
            NullCheckedInvocation::invoke (callback, false);

            return;
        }

        auto setProjectAndCallback = [parent, callback]
        {
            parent->setProject (nullptr);

            NullCheckedInvocation::invoke (callback, true);
        };

        if (askUserToSave == OpenDocumentManager::SaveIfNeeded::no)
        {
            setProjectAndCallback();
            return;
        }

        parent->currentProject->saveIfNeededAndUserAgreesAsync ([parent, setProjectAndCallback, callback] (FileBasedDocument::SaveResult saveResult)
        {
            if (parent == nullptr)
                return;

            if (saveResult == FileBasedDocument::savedOk)
                setProjectAndCallback();
            else
                NullCheckedInvocation::invoke (callback, false);
        });
    });
}

void MainWindow::moveProject (File newProjectFileToOpen, OpenInIDE openInIDE)
{
    closeCurrentProject (OpenDocumentManager::SaveIfNeeded::no,
                         [parent = SafePointer<MainWindow> { this }, newProjectFileToOpen, openInIDE] (bool)
    {
        if (parent == nullptr)
            return;

        parent->openFile (newProjectFileToOpen, [parent, openInIDE] (bool openedSuccessfully)
        {
            if (! (openedSuccessfully && parent != nullptr && parent->currentProject != nullptr && openInIDE == OpenInIDE::yes))
                return;

            // The project component knows how to process the saveAndOpenInIDE command, but the
            // main application does not. In order to process the command successfully, we need
            // to ensure that the project content component has focus.
            auto& manager = ProjucerApplication::getApp().getCommandManager();
            manager.setFirstCommandTarget (parent->getProjectContentComponent());
            ProjucerApplication::getApp().getCommandManager().invokeDirectly (CommandIDs::saveAndOpenInIDE, false);
            manager.setFirstCommandTarget (nullptr);
        });
    });
}

void MainWindow::setProject (std::unique_ptr<Project> newProject)
{
    if (newProject == nullptr)
    {
        if (auto* content = getProjectContentComponent())
            content->setProject (nullptr);

        currentProject.reset();
    }
    else
    {
        currentProject = std::move (newProject);

        createProjectContentCompIfNeeded();
        getProjectContentComponent()->setProject (currentProject.get());
    }

    if (currentProject != nullptr)
        currentProject->addChangeListener (this);

    changeListenerCallback (currentProject.get());

    projectNameValue.referTo (currentProject != nullptr ? currentProject->getProjectValue (Ids::name) : Value());
    initialiseProjectWindow();

    ProjucerApplication::getCommandManager().commandStatusChanged();
}

void MainWindow::restoreWindowPosition()
{
    String windowState;

    if (currentProject != nullptr)
        windowState = currentProject->getStoredProperties().getValue (getProjectWindowPosName());

    if (windowState.isEmpty())
        windowState = getGlobalProperties().getValue ("lastMainWindowPos");

    restoreWindowStateFromString (windowState);
}

bool MainWindow::canOpenFile (const File& file) const
{
    return (! file.isDirectory())
             && (file.hasFileExtension (Project::projectFileExtension)
                  || ProjucerApplication::getApp().openDocumentManager.canOpenFile (file));
}

void MainWindow::openFile (const File& file, std::function<void (bool)> callback)
{
    if (file.hasFileExtension (Project::projectFileExtension))
    {
        auto newDoc = std::make_unique<Project> (file);
        auto result = newDoc->loadFrom (file, true);

        if (result.wasOk())
        {
            closeCurrentProject (OpenDocumentManager::SaveIfNeeded::yes,
                                 [parent = SafePointer<MainWindow> { this },
                                  sharedDoc = std::make_shared<std::unique_ptr<Project>> (std::move (newDoc)),
                                  callback] (bool saveResult)
            {
                if (parent == nullptr)
                    return;

                if (saveResult)
                {
                    parent->setProject (std::move (*sharedDoc.get()));
                    parent->currentProject->setChangedFlag (false);

                    parent->createProjectContentCompIfNeeded();
                    parent->getProjectContentComponent()->reloadLastOpenDocuments();
                }

                NullCheckedInvocation::invoke (callback, saveResult);
            });

            return;
        }

        NullCheckedInvocation::invoke (callback, false);

        return;
    }

    if (file.exists())
    {
        SafePointer<MainWindow> parent { this };
        auto createCompAndShowEditor = [parent, file, callback]
        {
            if (parent != nullptr)
            {
                parent->createProjectContentCompIfNeeded();
                NullCheckedInvocation::invoke (callback, parent->getProjectContentComponent()->showEditorForFile (file, true));
            }
        };

        if (isPIPFile (file))
        {
            openPIP (file, [parent, createCompAndShowEditor, callback] (bool openedSuccessfully)
            {
                if (parent == nullptr)
                    return;

                if (openedSuccessfully)
                {
                    NullCheckedInvocation::invoke (callback, true);
                    return;
                }

                createCompAndShowEditor();
            });

            return;
        }

        createCompAndShowEditor();
        return;
    }

    NullCheckedInvocation::invoke (callback, false);
}

void MainWindow::openPIP (const File& pipFile, std::function<void (bool)> callback)
{
    auto generator = std::make_shared<PIPGenerator> (pipFile);

    if (! generator->hasValidPIP())
    {
        NullCheckedInvocation::invoke (callback, false);
        return;
    }

    auto generatorResult = generator->createJucerFile();

    if (generatorResult != Result::ok())
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "PIP Error.",
                                                         generatorResult.getErrorMessage());
        messageBox = AlertWindow::showScopedAsync (options, nullptr);

        NullCheckedInvocation::invoke (callback, false);
        return;
    }

    if (! generator->createMainCpp())
    {
        auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                         "PIP Error.",
                                                         "Failed to create Main.cpp.");
        messageBox = AlertWindow::showScopedAsync (options, nullptr);

        NullCheckedInvocation::invoke (callback, false);
        return;
    }

    openFile (generator->getJucerFile(), [parent = SafePointer<MainWindow> { this }, generator, callback] (bool openedSuccessfully)
    {
        if (parent == nullptr)
            return;

        if (! openedSuccessfully)
        {
            auto options = MessageBoxOptions::makeOptionsOk (MessageBoxIconType::WarningIcon,
                                                             "PIP Error.",
                                                             "Failed to open .jucer file.");
            parent->messageBox = AlertWindow::showScopedAsync (options, nullptr);

            NullCheckedInvocation::invoke (callback, false);
            return;
        }

        parent->setupTemporaryPIPProject (*generator);

        NullCheckedInvocation::invoke (callback, true);
    });
}

void MainWindow::setupTemporaryPIPProject (PIPGenerator& generator)
{
    jassert (currentProject != nullptr);

    currentProject->setTemporaryDirectory (generator.getOutputDirectory());

    if (auto* pcc = getProjectContentComponent())
    {
        auto fileToDisplay = generator.getPIPFile();

        if (fileToDisplay != File())
        {
            pcc->showEditorForFile (fileToDisplay, true);

            if (auto* sourceCodeEditor = dynamic_cast <SourceCodeEditor*> (pcc->getEditorComponent()))
                sourceCodeEditor->editor->scrollToLine (findBestLineToScrollToForClass (StringArray::fromLines (fileToDisplay.loadFileAsString()),
                                                                                        generator.getMainClassName(), currentProject->getProjectType().isAudioPlugin()));
        }
    }
}

bool MainWindow::isInterestedInFileDrag (const StringArray& filenames)
{
    for (auto& filename : filenames)
        if (canOpenFile (File (filename)))
            return true;

    return false;
}

static void filesDroppedRecursive (Component::SafePointer<MainWindow> parent, StringArray filenames)
{
    if (filenames.isEmpty())
        return;

    auto f = filenames[0];
    filenames.remove (0);

    if (! parent->canOpenFile (f))
    {
        filesDroppedRecursive (parent, filenames);
        return;
    }

    parent->openFile (f, [parent, filenames] (bool openedSuccessfully)
    {
        if (parent == nullptr || ! openedSuccessfully)
            return;

        filesDroppedRecursive (parent, filenames);
    });
}

void MainWindow::filesDropped (const StringArray& filenames, int /*mouseX*/, int /*mouseY*/)
{
    filesDroppedRecursive (this, filenames);
}

bool MainWindow::shouldDropFilesWhenDraggedExternally (const DragAndDropTarget::SourceDetails& sourceDetails,
                                                       StringArray& files, bool& canMoveFiles)
{
    if (auto* tv = dynamic_cast<TreeView*> (sourceDetails.sourceComponent.get()))
    {
        Array<JucerTreeViewBase*> selected;

        for (int i = tv->getNumSelectedItems(); --i >= 0;)
            if (auto* b = dynamic_cast<JucerTreeViewBase*> (tv->getSelectedItem (i)))
                selected.add (b);

        if (! selected.isEmpty())
        {
            for (int i = selected.size(); --i >= 0;)
            {
                if (auto* jtvb = selected.getUnchecked (i))
                {
                    auto f = jtvb->getDraggableFile();

                    if (f.existsAsFile())
                        files.add (f.getFullPathName());
                }
            }

            canMoveFiles = false;
            return ! files.isEmpty();
        }
    }

    return false;
}

void MainWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();

    if (auto* pcc = getProjectContentComponent())
        pcc->updateMissingFileStatuses();

    ProjucerApplication::getApp().openDocumentManager.reloadModifiedFiles();
}

void MainWindow::initialiseProjectWindow()
{
    setResizable (true, false);
    updateTitleBarIcon();
}

void MainWindow::showStartPage()
{
    jassert (currentProject == nullptr);

    setContentOwned (new StartPageComponent ([this] (std::unique_ptr<Project>&& newProject) { setProject (std::move (newProject)); },
                                             [this] (const File& exampleFile) { openFile (exampleFile, nullptr); }),
                     true);

    setResizable (false, false);
    setName ("New Project");
    addToDesktop();
    centreWithSize (getContentComponent()->getWidth(), getContentComponent()->getHeight());

    setVisible (true);
    getContentComponent()->grabKeyboardFocus();
}

//==============================================================================
ApplicationCommandTarget* MainWindow::getNextCommandTarget()
{
    return nullptr;
}

void MainWindow::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] =
    {
        CommandIDs::closeWindow,
        CommandIDs::goToPreviousWindow,
        CommandIDs::goToNextWindow
    };

    commands.addArray (ids, numElementsInArray (ids));
}

void MainWindow::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::closeWindow:
            result.setInfo ("Close Window", "Closes the current window", CommandCategories::general, 0);
            result.defaultKeypresses.add (KeyPress ('w', ModifierKeys::commandModifier, 0));
            break;

        case CommandIDs::goToPreviousWindow:
            result.setInfo ("Previous Window", "Activates the previous window", CommandCategories::general, 0);
            result.setActive (ProjucerApplication::getApp().mainWindowList.windows.size() > 1);
            result.defaultKeypresses.add (KeyPress (KeyPress::tabKey, ModifierKeys::shiftModifier | ModifierKeys::ctrlModifier, 0));
            break;

        case CommandIDs::goToNextWindow:
            result.setInfo ("Next Window", "Activates the next window", CommandCategories::general, 0);
            result.setActive (ProjucerApplication::getApp().mainWindowList.windows.size() > 1);
            result.defaultKeypresses.add (KeyPress (KeyPress::tabKey, ModifierKeys::ctrlModifier, 0));
            break;

        default:
            break;
    }
}

bool MainWindow::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::closeWindow:
            closeButtonPressed();
            break;

        case CommandIDs::goToPreviousWindow:
            ProjucerApplication::getApp().mainWindowList.goToSiblingWindow (this, -1);
            break;

        case CommandIDs::goToNextWindow:
            ProjucerApplication::getApp().mainWindowList.goToSiblingWindow (this, 1);
            break;

        default:
            return false;
    }

    return true;
}

void MainWindow::valueChanged (Value& value)
{
    if (value == projectNameValue)
        setName (currentProject != nullptr ? currentProject->getProjectNameString() + " - Projucer"
                                           : "Projucer");
}

void MainWindow::changeListenerCallback (ChangeBroadcaster* source)
{
    auto* project = getProject();

    if (source == project)
        if (auto* peer = getPeer())
            peer->setHasChangedSinceSaved (project != nullptr ? project->hasChangedSinceSaved()
                                                              : false);
}

//==============================================================================
MainWindowList::MainWindowList()
{
}

void MainWindowList::forceCloseAllWindows()
{
    windows.clear();
}

static void askAllWindowsToCloseRecursive (WeakReference<MainWindowList> parent, std::function<void (bool)> callback)
{
    if (parent->windows.size() == 0)
    {
        NullCheckedInvocation::invoke (callback, true);
        return;
    }

    parent->windows[0]->closeCurrentProject (OpenDocumentManager::SaveIfNeeded::yes, [parent, callback] (bool closedSuccessfully)
    {
        if (parent == nullptr)
            return;

        if (! closedSuccessfully)
        {
            NullCheckedInvocation::invoke (callback, false);
            return;
        }

        parent->windows.remove (0);
        askAllWindowsToCloseRecursive (parent, std::move (callback));
    });
}

void MainWindowList::askAllWindowsToClose (std::function<void (bool)> callback)
{
    saveCurrentlyOpenProjectList();
    askAllWindowsToCloseRecursive (this, std::move (callback));
}

void MainWindowList::createWindowIfNoneAreOpen()
{
    if (windows.isEmpty())
        createNewMainWindow()->showStartPage();
}

void MainWindowList::closeWindow (MainWindow* w)
{
    jassert (windows.contains (w));

   #if ! JUCE_MAC
    if (windows.size() == 1 && ! isInReopenLastProjects)
    {
        JUCEApplicationBase::getInstance()->systemRequestedQuit();
    }
    else
   #endif
    {
        w->closeCurrentProject (OpenDocumentManager::SaveIfNeeded::yes,
                                [parent = WeakReference<MainWindowList> { this }, w] (bool closedSuccessfully)
                                {
                                    if (parent == nullptr)
                                        return;

                                    if (closedSuccessfully)
                                    {
                                        parent->windows.removeObject (w);
                                        parent->saveCurrentlyOpenProjectList();
                                    }
                                });
    }
}

void MainWindowList::goToSiblingWindow (MainWindow* w, int delta)
{
    auto index = windows.indexOf (w);

    if (index >= 0)
        if (auto* next = windows[(index + delta + windows.size()) % windows.size()])
            next->toFront (true);
}

void MainWindowList::openDocument (OpenDocumentManager::Document* doc, bool grabFocus)
{
    auto& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
    {
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent (i)))
        {
            if (auto* pcc = mw->getProjectContentComponent())
            {
                if (pcc->hasFileInRecentList (doc->getFile()))
                {
                    mw->toFront (true);
                    mw->getProjectContentComponent()->showDocument (doc, grabFocus);
                    return;
                }
            }
        }
    }

    getFrontmostWindow()->getProjectContentComponent()->showDocument (doc, grabFocus);
}

void MainWindowList::openFile (const File& file, std::function<void (bool)> callback, bool openInBackground)
{
    if (! file.exists())
    {
        NullCheckedInvocation::invoke (callback, false);
        return;
    }

    for (auto* w : windows)
    {
        if (w->getProject() != nullptr && w->getProject()->getFile() == file)
        {
            w->toFront (true);

            NullCheckedInvocation::invoke (callback, true);
            return;
        }
    }

    WeakReference<MainWindowList> parent { this };

    if (file.hasFileExtension (Project::projectFileExtension)
        || isPIPFile (file))
    {
        WeakReference<Component> previousFrontWindow (getFrontmostWindow());

        auto* w = getOrCreateEmptyWindow();
        jassert (w != nullptr);

        w->openFile (file, [parent, previousFrontWindow, w, openInBackground, callback] (bool openedSuccessfully)
        {
            if (parent == nullptr)
                return;

            if (openedSuccessfully)
            {
                w->makeVisible();
                w->setResizable (true, false);
                parent->checkWindowBounds (*w);

                if (openInBackground && previousFrontWindow != nullptr)
                    previousFrontWindow->toFront (true);
            }
            else
            {
                parent->closeWindow (w);
            }

            NullCheckedInvocation::invoke (callback, openedSuccessfully);
        });

        return;
    }

    getFrontmostWindow()->openFile (file, [parent, callback] (bool openedSuccessfully)
    {
        if (parent != nullptr)
            NullCheckedInvocation::invoke (callback, openedSuccessfully);
    });
}

MainWindow* MainWindowList::createNewMainWindow()
{
    windows.add (new MainWindow());
    return windows.getLast();
}

MainWindow* MainWindowList::getFrontmostWindow (bool createIfNotFound)
{
    if (windows.isEmpty())
    {
        if (createIfNotFound)
        {
            auto* w = createNewMainWindow();
            jassert (w != nullptr);

            w->makeVisible();
            checkWindowBounds (*w);

            return w;
        }

        return nullptr;
    }

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        auto* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));

        if (windows.contains (mw))
            return mw;
    }

    return windows.getLast();
}

MainWindow* MainWindowList::getOrCreateEmptyWindow()
{
    if (windows.size() == 0)
        return createNewMainWindow();

    for (int i = Desktop::getInstance().getNumComponents(); --i >= 0;)
    {
        auto* mw = dynamic_cast<MainWindow*> (Desktop::getInstance().getComponent (i));

        if (windows.contains (mw) && mw->getProject() == nullptr)
            return mw;
    }

    return createNewMainWindow();
}

MainWindow* MainWindowList::getMainWindowForFile (const File& file)
{
    if (windows.size() > 0)
    {
        for (auto* window : windows)
        {
            if (auto* project = window->getProject())
            {
                if (project->getFile() == file)
                    return window;
            }
        }
    }

    return nullptr;
}

void MainWindowList::checkWindowBounds (MainWindow& windowToCheck)
{
    auto avoidSuperimposedWindows = [&]
    {
        for (auto* otherWindow : windows)
        {
            if (otherWindow == nullptr || otherWindow == &windowToCheck)
                continue;

            auto boundsToCheck = windowToCheck.getScreenBounds();
            auto otherBounds = otherWindow->getScreenBounds();

            if (std::abs (boundsToCheck.getX() - otherBounds.getX()) < 3
                 && std::abs (boundsToCheck.getY() - otherBounds.getY()) < 3
                 && std::abs (boundsToCheck.getRight()  - otherBounds.getRight()) < 3
                 && std::abs (boundsToCheck.getBottom() - otherBounds.getBottom()) < 3)
            {
                int dx = 40, dy = 30;

                if (otherBounds.getCentreX() >= boundsToCheck.getCentreX())  dx = -dx;
                if (otherBounds.getCentreY() >= boundsToCheck.getCentreY())  dy = -dy;

                windowToCheck.setBounds (boundsToCheck.translated (dx, dy));
            }
        }
    };

    auto ensureWindowIsFullyOnscreen = [&]
    {
        auto windowBounds = windowToCheck.getScreenBounds();
        auto screenLimits = Desktop::getInstance().getDisplays().getDisplayForRect (windowBounds)->userArea;

        if (auto* peer = windowToCheck.getPeer())
            if (const auto frameSize = peer->getFrameSizeIfPresent())
                frameSize->subtractFrom (screenLimits);

        auto constrainedX = jlimit (screenLimits.getX(), jmax (screenLimits.getX(), screenLimits.getRight()  - windowBounds.getWidth()),  windowBounds.getX());
        auto constrainedY = jlimit (screenLimits.getY(), jmax (screenLimits.getY(), screenLimits.getBottom() - windowBounds.getHeight()), windowBounds.getY());

        Point<int> constrainedTopLeft (constrainedX, constrainedY);

        if (windowBounds.getPosition() != constrainedTopLeft)
            windowToCheck.setTopLeftPosition (constrainedTopLeft);
    };

    avoidSuperimposedWindows();
    ensureWindowIsFullyOnscreen();
}

void MainWindowList::saveCurrentlyOpenProjectList()
{
    Array<File> projects;
    auto& desktop = Desktop::getInstance();

    for (int i = 0; i < desktop.getNumComponents(); ++i)
    {
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent (i)))
            if (auto* p = mw->getProject())
                if (! p->isTemporaryProject())
                    projects.add (p->getFile());
    }

    getAppSettings().setLastProjects (projects);
}

void MainWindowList::reopenLastProjects()
{
    const ScopedValueSetter<bool> setter (isInReopenLastProjects, true);

    for (auto& p : getAppSettings().getLastProjects())
        if (p.existsAsFile())
            openFile (p, nullptr, true);
}

void MainWindowList::sendLookAndFeelChange()
{
    for (auto* w : windows)
        w->sendLookAndFeelChange();
}

Project* MainWindowList::getFrontmostProject()
{
    auto& desktop = Desktop::getInstance();

    for (int i = desktop.getNumComponents(); --i >= 0;)
        if (auto* mw = dynamic_cast<MainWindow*> (desktop.getComponent (i)))
            if (auto* p = mw->getProject())
                return p;

    return nullptr;
}
