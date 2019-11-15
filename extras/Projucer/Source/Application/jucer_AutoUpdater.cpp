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
#include "jucer_Application.h"
#include "jucer_AutoUpdater.h"

//==============================================================================
LatestVersionCheckerAndUpdater::LatestVersionCheckerAndUpdater()
    : Thread ("VersionChecker")
{
}

LatestVersionCheckerAndUpdater::~LatestVersionCheckerAndUpdater()
{
    stopThread (1000);
    clearSingletonInstance();
}

void LatestVersionCheckerAndUpdater::checkForNewVersion (bool showAlerts)
{
    if (! isThreadRunning())
    {
        showAlertWindows = showAlerts;
        startThread (3);
    }
}

//==============================================================================
void LatestVersionCheckerAndUpdater::run()
{
    queryUpdateServer();

    if (! threadShouldExit())
        MessageManager::callAsync ([this] { processResult(); });
}

//==============================================================================
String getOSString()
{
   #if JUCE_MAC
    return "OSX";
   #elif JUCE_WINDOWS
    return "Windows";
   #elif JUCE_LINUX
    return "Linux";
   #else
    jassertfalse;
    return "Unknown";
   #endif
}

namespace VersionHelpers
{
    String formatProductVersion (int versionNum)
    {
        int major = (versionNum & 0xff0000) >> 16;
        int minor = (versionNum & 0x00ff00) >> 8;
        int build = (versionNum & 0x0000ff) >> 0;

        return String (major) + '.' + String (minor) + '.' + String (build);
    }

    String getProductVersionString()
    {
        return formatProductVersion (ProjectInfo::versionNumber);
    }

    bool isNewVersion (const String& current, const String& other)
    {
        auto currentTokens = StringArray::fromTokens (current, ".", {});
        auto otherTokens   = StringArray::fromTokens (other, ".", {});

        jassert (currentTokens.size() == 3 && otherTokens.size() == 3);

        if (currentTokens[0].getIntValue() == otherTokens[0].getIntValue())
        {
            if (currentTokens[1].getIntValue() == otherTokens[1].getIntValue())
                return currentTokens[2].getIntValue() < otherTokens[2].getIntValue();

            return currentTokens[1].getIntValue() < otherTokens[1].getIntValue();
        }

        return currentTokens[0].getIntValue() < otherTokens[0].getIntValue();
    }
}

void LatestVersionCheckerAndUpdater::queryUpdateServer()
{
    StringPairArray responseHeaders;

    URL latestVersionURL ("https://my.roli.com/software_versions/update_to/Projucer/"
                          + VersionHelpers::getProductVersionString() + '/' + getOSString()
                          + "?language=" + SystemStats::getUserLanguage());

    std::unique_ptr<InputStream> inStream (latestVersionURL.createInputStream (false, nullptr, nullptr,
                                                                               "X-API-Key: 265441b-343403c-20f6932-76361d\nContent-Type: "
                                                                               "application/json\nAccept: application/json; version=1",
                                                                               0, &responseHeaders, &statusCode, 0));

    if (threadShouldExit())
        return;

    if (inStream.get() != nullptr && (statusCode == 303 || statusCode == 400))
    {
        if (statusCode == 303)
            relativeDownloadPath = responseHeaders["Location"];

        jassert (relativeDownloadPath.isNotEmpty());

        jsonReply = JSON::parse (inStream->readEntireStreamAsString());
    }
    else if (showAlertWindows)
    {
        if (statusCode == 204)
            AlertWindow::showMessageBoxAsync (AlertWindow::InfoIcon, "No New Version Available", "Your JUCE version is up to date.");
        else
            AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Network Error", "Could not connect to the web server.\n"
                                                                                          "Please check your internet connection and try again.");
    }
}

void LatestVersionCheckerAndUpdater::processResult()
{
    if (! jsonReply.isObject())
        return;

    if (statusCode == 400)
    {
        auto errorObject = jsonReply.getDynamicObject()->getProperty ("error");

        if (errorObject.isObject())
        {
            auto message = errorObject.getProperty ("message", {}).toString();

            if (message.isNotEmpty())
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "JUCE Updater", message);
        }
    }
    else if (statusCode == 303)
    {
        askUserAboutNewVersion (jsonReply.getProperty ("version", {}).toString(),
                                jsonReply.getProperty ("notes",   {}).toString());
    }
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
            if (dontAskAgainButton.getToggleState())
                getGlobalProperties().setValue (Ids::dontQueryForUpdate.toString(), 1);
            else
                getGlobalProperties().removeValue (Ids::dontQueryForUpdate);

            exitModalStateWithResult (-1);
        };

        dontAskAgainButton.setToggleState (getGlobalProperties().getValue (Ids::dontQueryForUpdate, {}).isNotEmpty(), dontSendNotification);
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

    static std::unique_ptr<DialogWindow> launchDialog (const String& newVersion, const String& releaseNotes)
    {
        DialogWindow::LaunchOptions options;

        options.dialogTitle = "Download JUCE version " + newVersion + "?";
        options.resizable = false;

        auto* content = new UpdateDialog (newVersion, releaseNotes);
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

void LatestVersionCheckerAndUpdater::askUserForLocationToDownload()
{
    FileChooser chooser ("Please select the location into which you'd like to install the new version",
                         { getAppSettings().getStoredPath (Ids::jucePath, TargetOS::getThisOS()).get() });

    if (chooser.browseForDirectory())
    {
        auto targetFolder = chooser.getResult();

        if (isJUCEFolder (targetFolder))
        {
            if (targetFolder.getChildFile (".git").isDirectory())
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Downloading New JUCE Version",
                                                  "This folder is a GIT repository!\n\nYou should use a \"git pull\" to update it to the latest version.");

                return;
            }

            if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon, "Overwrite Existing JUCE Folder?",
                                                String ("Do you want to overwrite the folder:\n\n" + targetFolder.getFullPathName() + "\n\n..with the latest version from juce.com?\n\n"
                                                        "This will move the existing folder to " + targetFolder.getFullPathName() + "_old.")))
            {
                return;
            }
        }
        else
        {
            targetFolder = targetFolder.getChildFile ("JUCE").getNonexistentSibling();
        }

        downloadAndInstall (targetFolder);
    }
}

void LatestVersionCheckerAndUpdater::askUserAboutNewVersion (const String& newVersion, const String& releaseNotes)
{
    if (newVersion.isNotEmpty() && releaseNotes.isNotEmpty()
        && VersionHelpers::isNewVersion (VersionHelpers::getProductVersionString(), newVersion))
    {
        dialogWindow = UpdateDialog::launchDialog (newVersion, releaseNotes);

        if (auto* mm = ModalComponentManager::getInstance())
            mm->attachCallback (dialogWindow.get(), ModalCallbackFunction::create ([this] (int result)
                                                                             {
                                                                                 if (result == 1)
                                                                                     askUserForLocationToDownload();

                                                                                 dialogWindow.reset();
                                                                             }));
    }
}

//==============================================================================
class DownloadAndInstallThread   : private ThreadWithProgressWindow
{
public:
    DownloadAndInstallThread  (const URL& u, const File& t, std::function<void()>&& cb)
        : ThreadWithProgressWindow ("Downloading New Version", true, true),
          downloadURL (u), targetFolder (t), completionCallback (std::move (cb))
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
            MessageManager::callAsync ([result] () { AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon, "Installation Failed", result.getErrorMessage()); });
        else
            MessageManager::callAsync (completionCallback);
    }

    Result download (MemoryBlock& dest)
    {
        setStatusMessage ("Downloading...");

        int statusCode = 0;
        StringPairArray responseHeaders;

        std::unique_ptr<InputStream> inStream (downloadURL.createInputStream (false, nullptr, nullptr, {}, 0,
                                                                              &responseHeaders, &statusCode, 0));

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

        return Result::fail ("Failed to download from: " + downloadURL.toString (false));
    }

    Result install (MemoryBlock& data)
    {
        setStatusMessage ("Installing...");

        auto result = unzipDownload (data);

        if (threadShouldExit())
            result = Result::fail ("Cancelled");

        if (result.failed())
            return result;

        return Result::ok();
    }

    Result unzipDownload (const MemoryBlock& data)
    {
        MemoryInputStream input (data, false);
        ZipFile zip (input);

        if (zip.getNumEntries() == 0)
            return Result::fail ("The downloaded file was not a valid JUCE file!");

        auto unzipTarget = File::createTempFile ({});

        if (! unzipTarget.createDirectory())
            return Result::fail ("Couldn't create a temporary folder to unzip the new version!");

        auto r = zip.uncompressTo (unzipTarget);

        if (r.failed())
        {
            unzipTarget.deleteRecursively();
            return r;
        }

       #if JUCE_LINUX || JUCE_MAC
        r = setFilePermissions (unzipTarget, zip);

        if (r.failed())
        {
            unzipTarget.deleteRecursively();
            return r;
        }
       #endif

        if (targetFolder.exists())
        {
            auto oldFolder = targetFolder.getSiblingFile (targetFolder.getFileNameWithoutExtension() + "_old").getNonexistentSibling();

            if (! targetFolder.moveFileTo (oldFolder))
            {
                unzipTarget.deleteRecursively();
                return Result::fail ("Could not remove the existing folder!\n\n"
                                     "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
                                     "Please select a folder that is writable by the current user.");
            }
        }

        if (! unzipTarget.moveFileTo (targetFolder))
        {
            unzipTarget.deleteRecursively();
            return Result::fail ("Could not overwrite the existing folder!\n\n"
                                 "This may happen if you are trying to download into a directory that requires administrator privileges to modify.\n"
                                 "Please select a folder that is writable by the current user.");
        }

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

    URL downloadURL;
    File targetFolder;
    std::function<void()> completionCallback;
};

void restartProcess (const File& targetFolder)
{
   #if JUCE_MAC || JUCE_LINUX
    #if JUCE_MAC
     auto newProcess = targetFolder.getChildFile ("Projucer.app").getChildFile ("Contents").getChildFile ("MacOS").getChildFile ("Projucer");
    #elif JUCE_LINUX
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

void LatestVersionCheckerAndUpdater::downloadAndInstall (const File& targetFolder)
{
    installer.reset (new DownloadAndInstallThread ({ relativeDownloadPath }, targetFolder,
                                                   [this, targetFolder]
                                                   {
                                                       installer.reset();
                                                       restartProcess (targetFolder);
                                                   }));
}

//==============================================================================
JUCE_IMPLEMENT_SINGLETON (LatestVersionCheckerAndUpdater)
