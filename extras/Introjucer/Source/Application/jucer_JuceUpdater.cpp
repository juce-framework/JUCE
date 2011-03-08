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

#include "../jucer_Headers.h"
#include "jucer_JuceUpdater.h"


//==============================================================================
JuceUpdater::JuceUpdater()
    : filenameComp ("Juce Folder", StoredSettings::getInstance()->getLastKnownJuceFolder(),
                    true, true, false, "*", String::empty, "Select your Juce folder"),
      checkNowButton ("Check Online for Available Updates...",
                      "Contacts the website to see if this version is up-to-date")
{
    addAndMakeVisible (&label);
    addAndMakeVisible (&filenameComp);
    addAndMakeVisible (&checkNowButton);
    addAndMakeVisible (&currentVersionLabel);
    checkNowButton.addListener (this);
    filenameComp.addListener (this);

    currentVersionLabel.setFont (Font (14.0f, Font::italic));
    label.setFont (Font (12.0f));
    label.setText ("Destination folder:", false);

    addAndMakeVisible (&availableVersionsList);
    availableVersionsList.setModel (this);

    setSize (600, 300);
}

JuceUpdater::~JuceUpdater()
{
    checkNowButton.removeListener (this);
    filenameComp.removeListener (this);
}

void JuceUpdater::show (Component* mainWindow)
{
    JuceUpdater updater;
    DialogWindow::showModalDialog ("Juce Update...", &updater, mainWindow,
                                   Colours::lightgrey,
                                   true, false, false);
}

void JuceUpdater::resized()
{
    filenameComp.setBounds (20, 40, getWidth() - 40, 22);
    label.setBounds (filenameComp.getX(), filenameComp.getY() - 18, filenameComp.getWidth(), 18);
    currentVersionLabel.setBounds (filenameComp.getX(), filenameComp.getBottom(), filenameComp.getWidth(), 25);
    checkNowButton.changeWidthToFitText (20);
    checkNowButton.setCentrePosition (getWidth() / 2, filenameComp.getBottom() + 40);
    availableVersionsList.setBounds (filenameComp.getX(), checkNowButton.getBottom() + 20, filenameComp.getWidth(), getHeight() - (checkNowButton.getBottom() + 20));
}

void JuceUpdater::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

const String findVersionNum (const String& file, const String& token)
{
    return file.fromFirstOccurrenceOf (token, false, false)
               .upToFirstOccurrenceOf ("\n", false, false)
               .trim();
}

const String JuceUpdater::getCurrentVersion()
{
    const String header (filenameComp.getCurrentFile()
                            .getChildFile ("src/core/juce_StandardHeader.h").loadFileAsString());

    const String v1 (findVersionNum (header, "JUCE_MAJOR_VERSION"));
    const String v2 (findVersionNum (header, "JUCE_MINOR_VERSION"));
    const String v3 (findVersionNum (header, "JUCE_BUILDNUMBER"));

    if ((v1 + v2 + v3).isEmpty())
        return String::empty;

    return v1 + "." + v2 + "." + v3;
}

XmlElement* JuceUpdater::downloadVersionList()
{
    return URL ("http://www.rawmaterialsoftware.com/juce/downloads/juce_versions.php").readEntireXmlStream();
}

void JuceUpdater::updateVersions (const XmlElement& xml)
{
    availableVersions.clear();

    forEachXmlChildElementWithTagName (xml, v, "VERSION")
    {
        VersionInfo* vi = new VersionInfo();
        vi->url = URL (v->getStringAttribute ("url"));
        vi->desc = v->getStringAttribute ("desc");
        vi->version = v->getStringAttribute ("version");
        vi->date = v->getStringAttribute ("date");
        availableVersions.add (vi);
    }

    availableVersionsList.updateContent();
}

void JuceUpdater::buttonClicked (Button*)
{
    ScopedPointer<XmlElement> xml (downloadVersionList());

    if (xml == 0 || xml->hasTagName ("html"))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Connection Problems...",
                                     "Couldn't connect to the Raw Material Software website!");

        return;
    }

    if (! xml->hasTagName ("JUCEVERSIONS"))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Update Problems...",
                                     "This version of the Jucer may be too old to receive automatic updates!\n\n"
                                     "Please visit www.rawmaterialsoftware.com and get the latest version manually!");
        return;
    }

    const String currentVersion (getCurrentVersion());

    OwnedArray<VersionInfo> versions;
    updateVersions (*xml);
}

//==============================================================================
class NewVersionDownloader  : public ThreadWithProgressWindow
{
public:
    NewVersionDownloader (const String& title, const URL& url_, const File& target_)
        : ThreadWithProgressWindow (title, true, true),
          url (url_), target (target_)
    {
    }

    void run()
    {
        setStatusMessage ("Contacting website...");

        ScopedPointer<InputStream> input (url.createInputStream (false));

        if (input == 0)
        {
            error = "Couldn't connect to the website...";
            return;
        }

        if (! target.deleteFile())
        {
            error = "Couldn't delete the destination file...";
            return;
        }

        ScopedPointer<OutputStream> output (target.createOutputStream (32768));

        if (output == 0)
        {
            error = "Couldn't write to the destination file...";
            return;
        }

        setStatusMessage ("Downloading...");

        int totalBytes = (int) input->getTotalLength();
        int bytesSoFar = 0;

        while (! (input->isExhausted() || threadShouldExit()))
        {
            HeapBlock<char> buffer (8192);
            const int num = input->read (buffer, 8192);

            if (num == 0)
                break;

            output->write (buffer, num);
            bytesSoFar += num;

            setProgress (totalBytes > 0 ? bytesSoFar / (double) totalBytes : -1.0);
        }
    }

    String error;

private:
    URL url;
    File target;
};

//==============================================================================
class Unzipper  : public ThreadWithProgressWindow
{
public:
    Unzipper (ZipFile& zipFile_, const File& targetDir_)
        : ThreadWithProgressWindow ("Unzipping...", true, true),
          worked (true), zipFile (zipFile_), targetDir (targetDir_)
    {
    }

    void run()
    {
        for (int i = 0; i < zipFile.getNumEntries(); ++i)
        {
            if (threadShouldExit())
                break;

            const ZipFile::ZipEntry* e = zipFile.getEntry (i);
            setStatusMessage ("Unzipping " + e->filename + "...");
            setProgress (i / (double) zipFile.getNumEntries());

            worked = zipFile.uncompressEntry (i, targetDir, true) && worked;
        }
    }

    bool worked;

private:
    ZipFile& zipFile;
    File targetDir;
};

//==============================================================================
void JuceUpdater::applyVersion (VersionInfo* version)
{
    File destDir (filenameComp.getCurrentFile());

    const bool destDirExisted = destDir.isDirectory();

    if (destDirExisted && destDir.getNumberOfChildFiles (File::findFilesAndDirectories, "*") > 0)
    {
        int r = AlertWindow::showYesNoCancelBox (AlertWindow::WarningIcon, "Folder already exists",
                                                 "The folder " + destDir.getFullPathName() + "\nalready contains some files...\n\n"
                                                 "Do you want to delete everything in the folder and replace it entirely, or just merge the new files into the existing folder?",
                                                 "Delete and replace entire folder",
                                                 "Add and overwrite existing files",
                                                 "Cancel");

        if (r == 0)
            return;

        if (r == 1)
        {
            if (! destDir.deleteRecursively())
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Problems...",
                                             "Couldn't delete the existing folder!");
                return;
            }
        }
    }

    if (! (destDir.isDirectory() || destDir.createDirectory()))
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Problems...",
                                     "Couldn't create that target folder..");
        return;
    }

    File zipFile (destDir.getNonexistentChildFile ("juce_download", ".tar.gz", false));

    bool worked = false;

    {
        NewVersionDownloader downloader ("Downloading Version " + version->version + "...",
                                         version->url, zipFile);
        worked = downloader.runThread();
    }

    if (worked)
    {
        ZipFile zip (zipFile);
        Unzipper unzipper (zip, destDir);
        worked = unzipper.runThread() && unzipper.worked;
    }

    zipFile.deleteFile();

    if ((! destDirExisted) && (destDir.getNumberOfChildFiles (File::findFilesAndDirectories, "*") == 0 || ! worked))
        destDir.deleteRecursively();

    filenameComponentChanged (&filenameComp);
}

void JuceUpdater::filenameComponentChanged (FilenameComponent*)
{
    const String version (getCurrentVersion());

    if (version.isEmpty())
        currentVersionLabel.setText ("(Not a Juce folder)", false);
    else
        currentVersionLabel.setText ("(Current version in this folder: " + version + ")", false);
}

//==============================================================================
int JuceUpdater::getNumRows()
{
    return availableVersions.size();
}

void JuceUpdater::paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected)
{
    if (rowIsSelected)
        g.fillAll (findColour (TextEditor::highlightColourId));
}

Component* JuceUpdater::refreshComponentForRow (int rowNumber, bool isRowSelected, Component* existingComponentToUpdate)
{
    class UpdateListComponent  : public Component,
                                 public ButtonListener
    {
    public:
        UpdateListComponent (JuceUpdater& updater_)
            : updater (updater_), version (0), applyButton ("Install this version...")
        {
            addAndMakeVisible (&applyButton);
            applyButton.addListener (this);
            setInterceptsMouseClicks (false, true);
        }

        ~UpdateListComponent()
        {
            applyButton.removeListener (this);
        }

        void setVersion (VersionInfo* v)
        {
            if (version != v)
            {
                version = v;
                repaint();
                resized();
            }
        }

        void paint (Graphics& g)
        {
            if (version != 0)
            {
                g.setColour (Colours::green.withAlpha (0.12f));

                g.fillRect (0, 1, getWidth(), getHeight() - 2);
                g.setColour (Colours::black);
                g.setFont (getHeight() * 0.7f);

                String s;
                s << "Version " << version->version << " - " << version->desc << " - " << version->date;

                g.drawText (s, 4, 0, applyButton.getX() - 4, getHeight(), Justification::centredLeft, true);
            }
        }

        void resized()
        {
            applyButton.changeWidthToFitText (getHeight() - 4);
            applyButton.setTopRightPosition (getWidth(), 2);
            applyButton.setVisible (version != 0);
        }

        void buttonClicked (Button*)
        {
            updater.applyVersion (version);
        }

    private:
        JuceUpdater& updater;
        VersionInfo* version;
        TextButton applyButton;
    };

    UpdateListComponent* c = dynamic_cast <UpdateListComponent*> (existingComponentToUpdate);
    if (c == 0)
        c = new UpdateListComponent (*this);

    c->setVersion (availableVersions [rowNumber]);
    return c;
}
