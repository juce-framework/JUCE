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
#include "jucer_Application.h"
#include "jucer_AutoUpdater.h"

LatestVersionChecker::JuceVersionTriple::JuceVersionTriple()
  :  major ((ProjectInfo::versionNumber & 0xff0000) >> 16),
     minor ((ProjectInfo::versionNumber & 0x00ff00) >> 8),
     build ((ProjectInfo::versionNumber & 0x0000ff) >> 0)
{}

LatestVersionChecker::JuceVersionTriple::JuceVersionTriple (int juceVersionNumber)
  :  major ((juceVersionNumber & 0xff0000) >> 16),
     minor ((juceVersionNumber & 0x00ff00) >> 8),
     build ((juceVersionNumber & 0x0000ff) >> 0)
{}

LatestVersionChecker::JuceVersionTriple::JuceVersionTriple (int majorInt, int minorInt, int buildNumber)
    : major (majorInt),
      minor (minorInt),
      build (buildNumber)
{}

bool LatestVersionChecker::JuceVersionTriple::fromString (const String& versionString,
                                                          LatestVersionChecker::JuceVersionTriple& result)
{
    StringArray tokenizedString = StringArray::fromTokens (versionString, ".", StringRef());

    if (tokenizedString.size() != 3)
        return false;

    result.major = tokenizedString [0].getIntValue();
    result.minor = tokenizedString [1].getIntValue();
    result.build = tokenizedString [2].getIntValue();

    return true;
}

String LatestVersionChecker::JuceVersionTriple::toString() const
{
    String retval;
    retval << major << '.' << minor << '.' << build;
    return retval;
}

bool LatestVersionChecker::JuceVersionTriple::operator> (const LatestVersionChecker::JuceVersionTriple& b) const noexcept
{
    if (major == b.major)
    {
        if (minor == b.minor)
            return build > b.build;

        return minor > b.minor;
    }

    return major > b.major;
}

//==============================================================================
struct RelaunchTimer  : private Timer
{
    RelaunchTimer (const File& f)  : parentFolder (f)
    {
        startTimer (1500);
    }

    void timerCallback() override
    {
        stopTimer();

        File app = parentFolder.getChildFile (
                      #if JUCE_MAC
                       "Projucer.app");
                      #elif JUCE_WINDOWS
                       "Projucer.exe");
                      #elif JUCE_LINUX
                       "Projucer");
                      #endif

        JUCEApplication::quit();

        if (app.exists())
        {
            app.setExecutePermission (true);

           #if JUCE_MAC
            app.getChildFile ("Contents")
               .getChildFile ("MacOS")
               .getChildFile ("Projucer").setExecutePermission (true);
           #endif

                app.startAsProcess();
        }

        delete this;
    }

    File parentFolder;
};

//==============================================================================
class DownloadNewVersionThread   : public ThreadWithProgressWindow
{
public:
    DownloadNewVersionThread (LatestVersionChecker& versionChecker,URL u,
                              const String& extraHeaders, File target)
        : ThreadWithProgressWindow ("Downloading New Version", true, true),
          owner (versionChecker),
          result (Result::ok()),
          url (u), headers (extraHeaders), targetFolder (target)
    {
    }

    static void performDownload (LatestVersionChecker& versionChecker, URL u,
                                 const String& extraHeaders, File targetFolder)
    {
        DownloadNewVersionThread d (versionChecker, u, extraHeaders, targetFolder);

        if (d.runThread())
        {
            if (d.result.failed())
            {
                AlertWindow::showMessageBoxAsync (AlertWindow::WarningIcon,
                                                  "Installation Failed",
                                                  d.result.getErrorMessage());
            }
            else
            {
                new RelaunchTimer (targetFolder);
            }
        }
    }

    void run() override
    {
        setProgress (-1.0);

        MemoryBlock zipData;
        result = download (zipData);

        if (result.wasOk() && ! threadShouldExit())
        {
            setStatusMessage ("Installing...");
            result = owner.performUpdate (zipData, targetFolder);
        }
    }

    Result download (MemoryBlock& dest)
    {
        setStatusMessage ("Downloading...");

        int statusCode = 302;
        const int maxRedirects = 5;

        // we need to do the redirecting manually due to inconsistencies on the way headers are handled on redirects
        ScopedPointer<InputStream> in;

        for (int redirect = 0; redirect < maxRedirects; ++redirect)
        {
            StringPairArray responseHeaders;

            in = url.createInputStream (false, nullptr, nullptr, headers, 10000, &responseHeaders, &statusCode, 0);
            if (in == nullptr || statusCode != 302)
                break;

            String redirectPath = responseHeaders ["Location"];
            if (redirectPath.isEmpty())
                break;

            url = owner.getLatestVersionURL (headers, redirectPath);
        }

        if (in != nullptr && statusCode == 200)
        {
            int64 total = 0;
            MemoryOutputStream mo (dest, true);

            for (;;)
            {
                if (threadShouldExit())
                    return Result::fail ("cancel");

                int64 written = mo.writeFromInputStream (*in, 8192);

                if (written == 0)
                    break;

                total += written;

                setStatusMessage (String (TRANS ("Downloading...  (123)"))
                                  .replace ("123", File::descriptionOfSizeInBytes (total)));
            }

            return Result::ok();
        }

        return Result::fail ("Failed to download from: " + url.toString (false));
    }

    LatestVersionChecker& owner;
    Result result;
    URL url;
    String headers;
    File targetFolder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownloadNewVersionThread)
};

//==============================================================================
class UpdateUserDialog   : public Component,
                           public ButtonListener
{
public:
    UpdateUserDialog (const LatestVersionChecker::JuceVersionTriple& version,
                      const String& productName,
                      const String& releaseNotes,
                      const char* overwriteFolderPath)
        : hasOverwriteButton (overwriteFolderPath != nullptr)
    {
        addAndMakeVisible (titleLabel = new Label ("Title Label",
                                                   TRANS ("Download \"123\" version 456?").replace ("123", productName)
                                                                                          .replace ("456", version.toString())));

        titleLabel->setFont (Font (15.00f, Font::bold));
        titleLabel->setJustificationType (Justification::centredLeft);
        titleLabel->setEditable (false, false, false);
        titleLabel->setColour (TextEditor::textColourId, Colours::black);
        titleLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (contentLabel = new Label ("Content Label",
                                                     TRANS ("A new version of \"123\" is available - would you like to download it?")
                                                        .replace ("123", productName)));
        contentLabel->setFont (Font (15.00f, Font::plain));
        contentLabel->setJustificationType (Justification::topLeft);
        contentLabel->setEditable (false, false, false);
        contentLabel->setColour (TextEditor::textColourId, Colours::black);
        contentLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (okButton = new TextButton ("OK Button"));
        okButton->setButtonText (TRANS(hasOverwriteButton ? "Choose Another Folder..." : "OK"));
        okButton->addListener (this);

        addAndMakeVisible (cancelButton = new TextButton ("Cancel Button"));
        cancelButton->setButtonText (TRANS("Cancel"));
        cancelButton->addListener (this);

        addAndMakeVisible (changeLogLabel = new Label ("Change Log Label",
                                                       TRANS("Release Notes:")));
        changeLogLabel->setFont (Font (15.00f, Font::plain));
        changeLogLabel->setJustificationType (Justification::topLeft);
        changeLogLabel->setEditable (false, false, false);
        changeLogLabel->setColour (TextEditor::textColourId, Colours::black);
        changeLogLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

        addAndMakeVisible (changeLog = new TextEditor ("Change Log"));
        changeLog->setMultiLine (true);
        changeLog->setReturnKeyStartsNewLine (true);
        changeLog->setReadOnly (true);
        changeLog->setScrollbarsShown (true);
        changeLog->setCaretVisible (false);
        changeLog->setPopupMenuEnabled (false);
        changeLog->setText (releaseNotes);

        if (hasOverwriteButton)
        {
            addAndMakeVisible (overwriteLabel = new Label ("Overwrite Label",
                                                           TRANS("Updating will overwrite everything in the following folder:")));
            overwriteLabel->setFont (Font (15.00f, Font::plain));
            overwriteLabel->setJustificationType (Justification::topLeft);
            overwriteLabel->setEditable (false, false, false);
            overwriteLabel->setColour (TextEditor::textColourId, Colours::black);
            overwriteLabel->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

            addAndMakeVisible (overwritePath = new Label ("Overwrite Path", overwriteFolderPath));
            overwritePath->setFont (Font (15.00f, Font::bold));
            overwritePath->setJustificationType (Justification::topLeft);
            overwritePath->setEditable (false, false, false);
            overwritePath->setColour (TextEditor::textColourId, Colours::black);
            overwritePath->setColour (TextEditor::backgroundColourId, Colour (0x00000000));

            addAndMakeVisible (overwriteButton = new TextButton ("Overwrite Button"));
            overwriteButton->setButtonText (TRANS("Overwrite"));
            overwriteButton->addListener (this);
        }

        juceIcon = Drawable::createFromImageData (BinaryData::juce_icon_png,
                                                  BinaryData::juce_icon_pngSize);

        setSize (518, overwritePath ? 345 : 269);
    }

    ~UpdateUserDialog()
    {
        titleLabel = nullptr;
        contentLabel = nullptr;
        okButton = nullptr;
        cancelButton = nullptr;
        changeLogLabel = nullptr;
        changeLog = nullptr;
        overwriteLabel = nullptr;
        overwritePath = nullptr;
        overwriteButton = nullptr;
        juceIcon = nullptr;
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);
        g.setColour (Colours::black);

        if (juceIcon != nullptr)
            juceIcon->drawWithin (g, Rectangle<float> (20, 17, 64, 64),
                                  RectanglePlacement::stretchToFit, 1.000f);
    }

    void resized() override
    {
        titleLabel->setBounds (88, 10, 397, 24);
        contentLabel->setBounds (88, 40, 397, 51);
        changeLogLabel->setBounds (22, 92, 341, 24);
        changeLog->setBounds (24, 112, 476, 102);

        if (hasOverwriteButton)
        {
            okButton->setBounds (getWidth() - 24 - 174, getHeight() - 37, 174, 28);
            overwriteButton->setBounds ((getWidth() - 24 - 174) + -14 - 86, getHeight() - 37, 86, 28);
            cancelButton->setBounds (24, getHeight() - 37, 70, 28);

            overwriteLabel->setBounds (24, 238, 472, 16);
            overwritePath->setBounds (24, 262, 472, 40);
        }
        else
        {
            okButton->setBounds (getWidth() - 24 - 47, getHeight() - 37, 47, 28);
            cancelButton->setBounds ((getWidth() - 24 - 47) + -14 - 70, getHeight() - 37, 70, 28);
        }
    }

    void buttonClicked (Button* clickedButton) override
    {
        if (DialogWindow* parentDialog = findParentComponentOfClass<DialogWindow>())
        {
            if      (clickedButton == overwriteButton) parentDialog->exitModalState (1);
            else if (clickedButton == okButton)        parentDialog->exitModalState (2);
            else if (clickedButton == cancelButton)    parentDialog->exitModalState (-1);
        }
        else
            jassertfalse;
    }

    static DialogWindow* launch (const LatestVersionChecker::JuceVersionTriple& version,
                                 const String& productName,
                                 const String& releaseNotes,
                                 const char* overwritePath = nullptr)
    {
        OptionalScopedPointer<Component> userDialog (new UpdateUserDialog (version, productName,
                                                                           releaseNotes, overwritePath), true);

        DialogWindow::LaunchOptions lo;
        lo.dialogTitle = TRANS ("Download \"123\" version 456?").replace ("456", version.toString())
                                                                .replace ("123", productName);
        lo.dialogBackgroundColour = Colours::lightgrey;
        lo.content = userDialog;
        lo.componentToCentreAround = nullptr;
        lo.escapeKeyTriggersCloseButton = true;
        lo.useNativeTitleBar = true;
        lo.resizable = false;
        lo.useBottomRightCornerResizer = false;

        return lo.launchAsync();
    }

private:
    bool hasOverwriteButton;
    ScopedPointer<Label> titleLabel, contentLabel, changeLogLabel, overwriteLabel, overwritePath;
    ScopedPointer<TextButton> okButton, cancelButton;
    ScopedPointer<TextEditor> changeLog;
    ScopedPointer<TextButton> overwriteButton;
    ScopedPointer<Drawable> juceIcon;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UpdateUserDialog)
};

//==============================================================================
class UpdaterDialogModalCallback : public ModalComponentManager::Callback
{
public:
    struct DelayedCallback  : private Timer
    {
        DelayedCallback (LatestVersionChecker& versionChecker,
                         URL& newVersionToDownload,
                         const String& extraHeaders,
                         const File& appParentFolder,
                         int returnValue)
            : parent (versionChecker), download (newVersionToDownload),
              headers (extraHeaders), folder (appParentFolder), result (returnValue)
        {
            startTimer (200);
        }

    private:
        void timerCallback() override
        {
            stopTimer();
            parent.modalStateFinished (result, download, headers, folder);

            delete this;
        }

        LatestVersionChecker& parent;
        URL download;
        String headers;
        File folder;
        int result;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DelayedCallback)
    };

    UpdaterDialogModalCallback (LatestVersionChecker& versionChecker,
                                URL& newVersionToDownload,
                                const String& extraHeaders,
                                const File& appParentFolder)
        : parent (versionChecker), download (newVersionToDownload),
          headers (extraHeaders), folder (appParentFolder)
    {}

    void modalStateFinished (int returnValue) override
    {
        // the dialog window is only closed after this function exits
        // so we need a deferred callback to the parent. Unfortunately
        // our instance is also deleted after this function is used
        // so we can't use our own instance for a timer callback
        // we must allocate a new one.
        new DelayedCallback (parent, download, headers, folder, returnValue);
    }

private:
    LatestVersionChecker& parent;
    URL download;
    String headers;
    File folder;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UpdaterDialogModalCallback)
};


//==============================================================================
LatestVersionChecker::LatestVersionChecker()  : Thread ("Updater"),
                                                statusCode (-1),
                                                hasAttemptedToReadWebsite (false)
{
    startTimer (2000);
}

LatestVersionChecker::~LatestVersionChecker()
{
    stopThread (20000);
}

String LatestVersionChecker::getOSString()
{
    SystemStats::OperatingSystemType osType = SystemStats::getOperatingSystemType();

    if      ((osType & SystemStats::MacOSX)  != 0) return "OSX";
    else if ((osType & SystemStats::Windows) != 0) return "Windows";
    else if ((osType & SystemStats::Linux)   != 0) return "Linux";
    else return SystemStats::getOperatingSystemName();
}

const LatestVersionChecker::JuceServerLocationsAndKeys& LatestVersionChecker::getJuceServerURLsAndKeys() const
{
    static LatestVersionChecker::JuceServerLocationsAndKeys urlsAndKeys =
    {
        "https://my.roli.com",
        "265441b-343403c-20f6932-76361d",
        1,
        "/software_versions/update_to/Projucer/"
    };

    return urlsAndKeys;
}

int LatestVersionChecker::getProductVersionNumber() const   { return ProjectInfo::versionNumber; }
const char* LatestVersionChecker::getProductName() const    { return ProjectInfo::projectName; }
bool LatestVersionChecker::allowCustomLocation() const      { return true; }

Result LatestVersionChecker::performUpdate (const MemoryBlock& data, File& targetFolder)
{
    File unzipTarget;
    bool isUsingTempFolder = false;

    {
        MemoryInputStream input (data, false);
        ZipFile zip (input);

        if (zip.getNumEntries() == 0)
            return Result::fail ("The downloaded file wasn't a valid JUCE file!");

        unzipTarget = targetFolder;

        if (unzipTarget.exists())
        {
            isUsingTempFolder = true;
            unzipTarget = targetFolder.getNonexistentSibling();

            if (! unzipTarget.createDirectory())
                return Result::fail ("Couldn't create a folder to unzip the new version!");
        }

        Result r (zip.uncompressTo (unzipTarget));

        if (r.failed())
        {
            if (isUsingTempFolder)
                unzipTarget.deleteRecursively();

            return r;
        }
    }

    if (isUsingTempFolder)
    {
        File oldFolder (targetFolder.getSiblingFile (targetFolder.getFileNameWithoutExtension() + "_old")
                        .getNonexistentSibling());

        if (! targetFolder.moveFileTo (oldFolder))
        {
            unzipTarget.deleteRecursively();
            return Result::fail ("Could not remove the existing folder!");
        }

        if (! unzipTarget.moveFileTo (targetFolder))
        {
            unzipTarget.deleteRecursively();
            return Result::fail ("Could not overwrite the existing folder!");
        }
    }

    return Result::ok();
}

URL LatestVersionChecker::getLatestVersionURL (String& headers, const String& path) const
{
    const LatestVersionChecker::JuceServerLocationsAndKeys& urlsAndKeys = getJuceServerURLsAndKeys();

    String updateURL;
    bool isAbsolute = (path.startsWith ("http://") || path.startsWith ("https://"));
    bool isRedirect = path.isNotEmpty();

    if (isAbsolute)
    {
        updateURL = path;
    }
    else
    {
        updateURL << urlsAndKeys.updateSeverHostname
                  << (isRedirect ? path : String (urlsAndKeys.updatePath));

        if (! isRedirect)
        {
            updateURL << JuceVersionTriple (getProductVersionNumber()).toString() << '/'
                      << getOSString() << "?language=" << SystemStats::getUserLanguage();
        }
    }

    headers.clear();

    if (! isAbsolute)
    {
        headers << "X-API-Key: " << urlsAndKeys.publicAPIKey;

        if (! isRedirect)
        {
            headers << "\nContent-Type: application/json\n"
                    << "Accept: application/json; version=" << urlsAndKeys.apiVersion;
        }
    }

    return URL (updateURL);
}

URL LatestVersionChecker::getLatestVersionURL (String& headers) const
{
    String emptyString;
    return getLatestVersionURL (headers, emptyString);
}

void LatestVersionChecker::checkForNewVersion()
{
    hasAttemptedToReadWebsite = true;

    {
        String extraHeaders;
        URL updateURL (getLatestVersionURL (extraHeaders));
        StringPairArray responseHeaders;

        const int numRedirects = 0;

        const ScopedPointer<InputStream> in (updateURL.createInputStream (false, nullptr, nullptr,
                                                                          extraHeaders, 0, &responseHeaders,
                                                                          &statusCode, numRedirects));

        if (threadShouldExit())
            return;  // can't connect: fail silently.

        if (in != nullptr && (statusCode == 303 || statusCode == 400))
        {
            // if this doesn't fail then there is a new version available.
            // By leaving the scope of this function we will abort the download
            // to give the user a chance to cancel an update
            if (statusCode == 303)
                newRelativeDownloadPath = responseHeaders ["Location"];

            jsonReply = JSON::parse (in->readEntireStreamAsString());
        }
    }

    if (! threadShouldExit())
        startTimer (100);
}

 bool LatestVersionChecker::processResult (var reply, const String& downloadPath)
 {
     if (statusCode == 303)
     {
         String versionString = reply.getProperty ("version", var()).toString();
         String releaseNotes = reply.getProperty ("notes", var()).toString();
         JuceVersionTriple version;

         if (versionString.isNotEmpty() && releaseNotes.isNotEmpty())
         {
             if (JuceVersionTriple::fromString (versionString, version))
             {
                 String extraHeaders;

                 URL newVersionToDownload = getLatestVersionURL (extraHeaders, downloadPath);
                 return askUserAboutNewVersion (version, releaseNotes, newVersionToDownload, extraHeaders);
             }
         }
     }
     else if (statusCode == 400)
     {
         // In the far-distant future, this may be contacting a defunct
         // URL, so hopefully the website will contain a helpful message
         // for the user..
         var errorObj = reply.getDynamicObject()->getProperty ("error");

         if (errorObj.isObject())
         {
             String message = errorObj.getProperty ("message", var()).toString();

             if (message.isNotEmpty())
             {
                 AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                              TRANS("JUCE Updater"),
                                              message);

                 return false;
             }
         }
     }

     // try again
     return true;
}

bool LatestVersionChecker::askUserAboutNewVersion (const LatestVersionChecker::JuceVersionTriple& version,
                                                   const String& releaseNotes,
                                                   URL& newVersionToDownload,
                                                   const String& extraHeaders)
{
    JuceVersionTriple currentVersion (getProductVersionNumber());

    if (version > currentVersion)
    {
        File appParentFolder (File::getSpecialLocation (File::currentApplicationFile).getParentDirectory());
        DialogWindow* modalDialog = nullptr;

        if (isZipFolder (appParentFolder) && allowCustomLocation())
        {
            modalDialog = UpdateUserDialog::launch (version, getProductName(), releaseNotes,
                                                    appParentFolder.getFullPathName().toRawUTF8());
        }
        else
        {
            modalDialog = UpdateUserDialog::launch (version, getProductName(), releaseNotes);
        }

        if (modalDialog != nullptr)
        {
            UpdaterDialogModalCallback* callback = new UpdaterDialogModalCallback (*this,
                                                                                   newVersionToDownload,
                                                                                   extraHeaders,
                                                                                   appParentFolder);

            // attachCallback will delete callback
            if (ModalComponentManager* mm = ModalComponentManager::getInstance())
                mm->attachCallback (modalDialog, callback);
        }

        return false;
    }

    return true;
}

void LatestVersionChecker::modalStateFinished (int result,
                                               URL& newVersionToDownload,
                                               const String& extraHeaders,
                                               File appParentFolder)
{

    if (result == 1 || result == 2)
    {
        if (result == 1 || ! allowCustomLocation())
            DownloadNewVersionThread::performDownload (*this, newVersionToDownload, extraHeaders, appParentFolder);
        else
            askUserForLocationToDownload (newVersionToDownload, extraHeaders);
    }
}

void LatestVersionChecker::askUserForLocationToDownload (URL& newVersionToDownload, const String& extraHeaders)
{
    File targetFolder (findDefaultModulesFolder());

    if (isJuceModulesFolder (targetFolder))
        targetFolder = targetFolder.getParentDirectory();

    FileChooser chooser (TRANS("Please select the location into which you'd like to install the new version"),
        targetFolder);

    if (chooser.browseForDirectory())
    {
        targetFolder = chooser.getResult();

        if (isJuceModulesFolder (targetFolder))
            targetFolder = targetFolder.getParentDirectory();

        if (targetFolder.getChildFile ("JUCE").isDirectory())
            targetFolder = targetFolder.getChildFile ("JUCE");

        if (targetFolder.getChildFile (".git").isDirectory())
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                         TRANS ("Downloading new JUCE version"),
                                         TRANS ("This folder is a GIT repository!\n\n"
                                                "You should use a \"git pull\" to update it to the latest version. "
                                                "Or to use the Projucer to get an update, you should select an empty "
                                                "folder into which you'd like to download the new code."));

            return;
        }

        if (isJuceFolder (targetFolder))
        {
            if (! AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                TRANS("Overwrite existing JUCE folder?"),
                                                TRANS("Do you want to overwrite the folder:\n\n"
                                                      "xfldrx\n\n"
                                                      " ..with the latest version from juce.com?\n\n"
                                                      "(Please note that this will overwrite everything in that folder!)")
                                                .replace ("xfldrx", targetFolder.getFullPathName())))
            {
                return;
            }
        }
        else
        {
            targetFolder = targetFolder.getChildFile ("JUCE").getNonexistentSibling();
        }

        DownloadNewVersionThread::performDownload (*this, newVersionToDownload, extraHeaders, targetFolder);
    }
}

bool LatestVersionChecker::isZipFolder (const File& f)
{
    return f.getChildFile ("modules").isDirectory()
        && f.getChildFile ("extras").isDirectory()
        && f.getChildFile ("examples").isDirectory()
        && ! f.getChildFile (".git").isDirectory();
}

void LatestVersionChecker::timerCallback()
{
    stopTimer();

    if (hasAttemptedToReadWebsite)
    {
        bool restartTimer = true;
        if (jsonReply.isObject())
            restartTimer = processResult (jsonReply, newRelativeDownloadPath);

        hasAttemptedToReadWebsite = false;

        if (restartTimer)
            startTimer (7200000);
    }
    else
    {
        startThread (3);
    }
}

void LatestVersionChecker::run()
{
    checkForNewVersion();
}
