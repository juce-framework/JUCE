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

#include "../Application/jucer_Headers.h"
#include "jucer_Application.h"
#include "jucer_AutoUpdater.h"

//==============================================================================
LatestVersionCheckerAndUpdater::LatestVersionCheckerAndUpdater()
    : Thread ("VersionChecker")
{
}

LatestVersionCheckerAndUpdater::~LatestVersionCheckerAndUpdater()
{
    stopThread (6000);
    clearSingletonInstance();
}

void LatestVersionCheckerAndUpdater::checkForNewVersion (bool background)
{
    if (! isThreadRunning())
    {
        backgroundCheck = background;
        startThread (3);
    }
}

//==============================================================================
void LatestVersionCheckerAndUpdater::run()
{
    auto info = VersionInfo::fetchLatestFromUpdateServer();

    if (info == nullptr)
    {
        if (! backgroundCheck)
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                              "Update Server Communication Error",
                                              "Failed to communicate with the JUCE update server.\n"
                                              "Please try again in a few minutes.\n\n"
                                              "If this problem persists you can download the latest version of JUCE from juce.com");

        return;
    }

    if (! info->isNewerVersionThanCurrent())
    {
        if (! backgroundCheck)
            AlertWindow::showMessageBoxAsync (MessageBoxIconType::InfoIcon,
                                              "No New Version Available",
                                              "Your JUCE version is up to date.");
        return;
    }

    auto osString = []
    {
       #if JUCE_MAC
        return "osx";
       #elif JUCE_WINDOWS
        return "windows";
       #elif JUCE_LINUX
        return "linux";
       #elif JUCE_BSD
        return "bsd";
       #else
        jassertfalse;
        return "Unknown";
       #endif
    }();

    String requiredFilename ("juce-" + info->versionString + "-" + osString + ".zip");

    for (auto& asset : info->assets)
    {
        if (asset.name == requiredFilename)
        {
            auto versionString = info->versionString;
            auto releaseNotes  = info->releaseNotes;

            MessageManager::callAsync ([this, versionString, releaseNotes, asset]
            {
                askUserAboutNewVersion (versionString, releaseNotes, asset);
            });

            return;
        }
    }

    if (! backgroundCheck)
        AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                          "Failed to find any new downloads",
                                          "Please try again in a few minutes.");
}

//==============================================================================
class UpdateDialog  : public Component
{
public:
    UpdateDialog (const String& newVersion, const String& releaseNotes)
    {
        titleLabel.setText ("JUCE version " + newVersion, dontSendNotification);
        titleLabel.setFont ({ 15.0f, Font::bold });
        titleLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (titleLabel);

        contentLabel.setText ("A new version of JUCE is available - would you like to download it?", dontSendNotification);
        contentLabel.setFont (15.0f);
        contentLabel.setJustificationType (Justification::topLeft);
        addAndMakeVisible (contentLabel);

        releaseNotesLabel.setText ("Release notes:", dontSendNotification);
        releaseNotesLabel.setFont (15.0f);
        releaseNotesLabel.setJustificationType (Justification::topLeft);
        addAndMakeVisible (releaseNotesLabel);

        releaseNotesEditor.setMultiLine (true);
        releaseNotesEditor.setReadOnly (true);
        releaseNotesEditor.setText (releaseNotes);
        addAndMakeVisible (releaseNotesEditor);

        addAndMakeVisible (chooseButton);
        chooseButton.onClick = [this] { exitModalStateWithResult (1); };

        addAndMakeVisible (cancelButton);
        cancelButton.onClick = [this]
        {
            ProjucerApplication::getApp().setAutomaticVersionCheckingEnabled (! dontAskAgainButton.getToggleState());
            exitModalStateWithResult (-1);
        };

        dontAskAgainButton.setToggleState (! ProjucerApplication::getApp().isAutomaticVersionCheckingEnabled(), dontSendNotification);
        addAndMakeVisible (dontAskAgainButton);

        juceIcon = Drawable::createFromImageData (BinaryData::juce_icon_png,
                                                  BinaryData::juce_icon_pngSize);
        lookAndFeelChanged();

        setSize (500, 280);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (10);

        auto topSlice = b.removeFromTop (juceIconBounds.getHeight())
                         .withTrimmedLeft (juceIconBounds.getWidth());

        titleLabel.setBounds (topSlice.removeFromTop (25));
        topSlice.removeFromTop (5);
        contentLabel.setBounds (topSlice.removeFromTop (25));

        auto buttonBounds = b.removeFromBottom (60);
        buttonBounds.removeFromBottom (25);
        chooseButton.setBounds (buttonBounds.removeFromLeft (buttonBounds.getWidth() / 2).reduced (20, 0));
        cancelButton.setBounds (buttonBounds.reduced (20, 0));
        dontAskAgainButton.setBounds (cancelButton.getBounds().withY (cancelButton.getBottom() + 5).withHeight (20));

        releaseNotesEditor.setBounds (b.reduced (0, 10));
    }

    void paint (Graphics& g) override
    {
        g.fillAll (findColour (backgroundColourId));

        if (juceIcon != nullptr)
            juceIcon->drawWithin (g, juceIconBounds.toFloat(),
                                  RectanglePlacement::stretchToFit, 1.0f);
    }

    static std::unique_ptr<DialogWindow> launchDialog (const String& newVersionString,
                                                       const String& releaseNotes)
    {
        DialogWindow::LaunchOptions options;

        options.dialogTitle = "Download JUCE version " + newVersionString + "?";
        options.resizable = false;

        auto* content = new UpdateDialog (newVersionString, releaseNotes);
        options.content.set (content, true);

        std::unique_ptr<DialogWindow> dialog (options.create());

        content->setParentWindow (dialog.get());
        dialog->enterModalState (true, nullptr, true);

        return dialog;
    }

private:
    void lookAndFeelChanged() override
    {
        cancelButton.setColour (TextButton::buttonColourId, findColour (secondaryButtonBackgroundColourId));
        releaseNotesEditor.applyFontToAllText (releaseNotesEditor.getFont());
    }

    void setParentWindow (DialogWindow* parent)
    {
        parentWindow = parent;
    }

    void exitModalStateWithResult (int result)
    {
        if (parentWindow != nullptr)
            parentWindow->exitModalState (result);
    }

    Label titleLabel, contentLabel, releaseNotesLabel;
    TextEditor releaseNotesEditor;
    TextButton chooseButton { "Choose Location..." }, cancelButton { "Cancel" };
    ToggleButton dontAskAgainButton { "Don't ask again" };
    std::unique_ptr<Drawable> juceIcon;
    Rectangle<int> juceIconBounds { 10, 10, 64, 64 };

    DialogWindow* parentWindow = nullptr;
};

void LatestVersionCheckerAndUpdater::askUserForLocationToDownload (const VersionInfo::Asset& asset)
{
    chooser = std::make_unique<FileChooser> ("Please select the location into which you would like to install the new version",
                                             File { getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get() });

    chooser->launchAsync (FileBrowserComponent::openMode | FileBrowserComponent::canSelectDirectories,
                          [weakThis = WeakReference<LatestVersionCheckerAndUpdater> { this }, asset] (const FileChooser& fc)
    {
        auto targetFolder = fc.getResult();

        if (targetFolder == File{})
            return;

        // By default we will install into 'targetFolder/JUCE', but we should install into
        // 'targetFolder' if that is an existing JUCE directory.
        bool willOverwriteJuceFolder = [&targetFolder]
        {
            if (isJUCEFolder (targetFolder))
                return true;

            targetFolder = targetFolder.getChildFile ("JUCE");

            return isJUCEFolder (targetFolder);
        }();

        auto targetFolderPath = targetFolder.getFullPathName();

        const auto onResult = [weakThis, asset, targetFolder] (int result)
        {
            if (weakThis == nullptr || result == 0)
                return;

            weakThis->downloadAndInstall (asset, targetFolder);
        };

        if (willOverwriteJuceFolder)
        {
            if (targetFolder.getChildFile (".git").isDirectory())
            {
                AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon, "Downloading New JUCE Version",
                                                  targetFolderPath + "\n\nis a GIT repository!\n\nYou should use a \"git pull\" to update it to the latest version.");

                return;
            }

            AlertWindow::showOkCancelBox (MessageBoxIconType::WarningIcon,
                                          "Overwrite Existing JUCE Folder?",
                                          "Do you want to replace the folder\n\n" + targetFolderPath + "\n\nwith the latest version from juce.com?\n\n"
                                              "This will move the existing folder to " + targetFolderPath + "_old.\n\n"
                                              "Replacing the folder that contains the currently running Projucer executable may not work on Windows.",
                                          {},
                                          {},
                                          nullptr,
                                          ModalCallbackFunction::create (onResult));
            return;
        }

        if (targetFolder.exists())
        {
            AlertWindow::showOkCancelBox (MessageBoxIconType::WarningIcon,
                                          "Existing File Or Directory",
                                          "Do you want to move\n\n" + targetFolderPath + "\n\nto\n\n" + targetFolderPath + "_old?",
                                          {},
                                          {},
                                          nullptr,
                                          ModalCallbackFunction::create (onResult));
            return;
        }

        if (weakThis != nullptr)
            weakThis->downloadAndInstall (asset, targetFolder);
    });
}

void LatestVersionCheckerAndUpdater::askUserAboutNewVersion (const String& newVersionString,
                                                             const String& releaseNotes,
                                                             const VersionInfo::Asset& asset)
{
    if (backgroundCheck)
        addNotificationToOpenProjects (asset);
    else
        showDialogWindow (newVersionString, releaseNotes, asset);
}

void LatestVersionCheckerAndUpdater::showDialogWindow (const String& newVersionString,
                                                       const String& releaseNotes,
                                                       const VersionInfo::Asset& asset)
{
    dialogWindow = UpdateDialog::launchDialog (newVersionString, releaseNotes);

    if (auto* mm = ModalComponentManager::getInstance())
    {
        mm->attachCallback (dialogWindow.get(),
                            ModalCallbackFunction::create ([this, asset] (int result)
                                                           {
                                                               if (result == 1)
                                                                    askUserForLocationToDownload (asset);

                                                                dialogWindow.reset();
                                                            }));
    }
}

void LatestVersionCheckerAndUpdater::addNotificationToOpenProjects (const VersionInfo::Asset& asset)
{
    for (auto* window : ProjucerApplication::getApp().mainWindowList.windows)
    {
        if (auto* project = window->getProject())
        {
            auto ignore = [safeWindow = Component::SafePointer<MainWindow> { window }]
            {
                if (safeWindow != nullptr)
                    safeWindow->getProject()->removeProjectMessage (ProjectMessages::Ids::newVersionAvailable);
            };

            auto dontAskAgain = [ignore]
            {
                ignore();
                ProjucerApplication::getApp().setAutomaticVersionCheckingEnabled (false);
            };

            project->addProjectMessage (ProjectMessages::Ids::newVersionAvailable,
                                        { { "Download", [this, asset] { askUserForLocationToDownload (asset); } },
                                          { "Ignore", std::move (ignore) },
                                          { "Don't ask again", std::move (dontAskAgain) } });
        }
    }
}

//==============================================================================
class DownloadAndInstallThread   : private ThreadWithProgressWindow
{
public:
    DownloadAndInstallThread  (const VersionInfo::Asset& a, const File& t, std::function<void()>&& cb)
        : ThreadWithProgressWindow ("Downloading New Version", true, true),
          asset (a), targetFolder (t), completionCallback (std::move (cb))
    {
        launchThread (3);
    }

private:
    void run() override
    {
        setProgress (-1.0);

        MemoryBlock zipData;
        auto result = download (zipData);

        if (result.wasOk() && ! threadShouldExit())
            result = install (zipData);

        if (result.failed())
            MessageManager::callAsync ([result] { AlertWindow::showMessageBoxAsync (MessageBoxIconType::WarningIcon,
                                                                                    "Installation Failed",
                                                                                    result.getErrorMessage()); });
        else
            MessageManager::callAsync (completionCallback);
    }

    Result download (MemoryBlock& dest)
    {
        setStatusMessage ("Downloading...");

        int statusCode = 0;
        auto inStream = VersionInfo::createInputStreamForAsset (asset, statusCode);

        if (inStream != nullptr && statusCode == 200)
        {
            int64 total = 0;
            MemoryOutputStream mo (dest, true);

            for (;;)
            {
                if (threadShouldExit())
                    return Result::fail ("Cancelled");

                auto written = mo.writeFromInputStream (*inStream, 8192);

                if (written == 0)
                    break;

                total += written;

                setStatusMessage ("Downloading... " + File::descriptionOfSizeInBytes (total));
            }

            return Result::ok();
        }

        return Result::fail ("Failed to download from: " + asset.url);
    }

    Result install (const MemoryBlock& data)
    {
        setStatusMessage ("Installing...");

        MemoryInputStream input (data, false);
        ZipFile zip (input);

        if (zip.getNumEntries() == 0)
            return Result::fail ("The downloaded file was not a valid JUCE file!");

        struct ScopedDownloadFolder
        {
            explicit ScopedDownloadFolder (const File& installTargetFolder)
            {
                folder = installTargetFolder.getSiblingFile (installTargetFolder.getFileNameWithoutExtension() + "_download").getNonexistentSibling();
                jassert (folder.createDirectory());
            }

            ~ScopedDownloadFolder()   { folder.deleteRecursively(); }

            File folder;
        };

        ScopedDownloadFolder unzipTarget (targetFolder);

        if (! unzipTarget.folder.isDirectory())
            return Result::fail ("Couldn't create a temporary folder to unzip the new version!");

        auto r = zip.uncompressTo (unzipTarget.folder);

        if (r.failed())
            return r;

        if (threadShouldExit())
            return Result::fail ("Cancelled");

       #if JUCE_LINUX || JUCE_BSD || JUCE_MAC
        r = setFilePermissions (unzipTarget.folder, zip);

        if (r.failed())
            return r;

        if (threadShouldExit())
            return Result::fail ("Cancelled");
       #endif

        if (targetFolder.exists())
        {
            auto oldFolder = targetFolder.getSiblingFile (targetFolder.getFileNameWithoutExtension() + "_old").getNonexistentSibling();

            if (! targetFolder.moveFileTo (oldFolder))
                return Result::fail ("Could not remove the existing folder!\n\n"
                                     "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
                                     "Please select a folder that is writable by the current user.");
        }

        if (! unzipTarget.folder.getChildFile ("JUCE").moveFileTo (targetFolder))
            return Result::fail ("Could not overwrite the existing folder!\n\n"
                                 "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
                                 "Please select a folder that is writable by the current user.");

        return Result::ok();
    }

    Result setFilePermissions (const File& root, const ZipFile& zip)
    {
        constexpr uint32 executableFlag = (1 << 22);

        for (int i = 0; i < zip.getNumEntries(); ++i)
        {
            auto* entry = zip.getEntry (i);

            if ((entry->externalFileAttributes & executableFlag) != 0 && entry->filename.getLastCharacter() != '/')
            {
                auto exeFile = root.getChildFile (entry->filename);

                if (! exeFile.exists())
                    return Result::fail ("Failed to find executable file when setting permissions " + exeFile.getFileName());

                if (! exeFile.setExecutePermission (true))
                    return Result::fail ("Failed to set executable file permission for " + exeFile.getFileName());
            }
        }

        return Result::ok();
    }

    VersionInfo::Asset asset;
    File targetFolder;
    std::function<void()> completionCallback;
};

static void restartProcess (const File& targetFolder)
{
   #if JUCE_MAC || JUCE_LINUX || JUCE_BSD
    #if JUCE_MAC
     auto newProcess = targetFolder.getChildFile ("Projucer.app").getChildFile ("Contents").getChildFile ("MacOS").getChildFile ("Projucer");
    #elif JUCE_LINUX || JUCE_BSD
     auto newProcess = targetFolder.getChildFile ("Projucer");
    #endif

    StringArray command ("/bin/sh", "-c", "while killall -0 Projucer; do sleep 5; done; " + newProcess.getFullPathName().quoted());
   #elif JUCE_WINDOWS
    auto newProcess = targetFolder.getChildFile ("Projucer.exe");

    auto command = "cmd.exe /c\"@echo off & for /l %a in (0) do ( tasklist | find \"Projucer\" >nul & ( if errorlevel 1 ( "
                    + targetFolder.getChildFile ("Projucer.exe").getFullPathName().quoted() + " & exit /b ) else ( timeout /t 10 >nul ) ) )\"";
   #endif

    if (newProcess.existsAsFile())
    {
        ChildProcess restartProcess;
        restartProcess.start (command, 0);

        ProjucerApplication::getApp().systemRequestedQuit();
    }
}

void LatestVersionCheckerAndUpdater::downloadAndInstall (const VersionInfo::Asset& asset, const File& targetFolder)
{
    installer.reset (new DownloadAndInstallThread (asset, targetFolder,
                                                   [this, targetFolder]
                                                   {
                                                       installer.reset();
                                                       restartProcess (targetFolder);
                                                   }));
}

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (LatestVersionCheckerAndUpdater)
