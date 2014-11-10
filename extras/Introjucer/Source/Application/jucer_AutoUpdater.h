/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

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

#ifndef JUCER_AUTOUPDATER_H_INCLUDED
#define JUCER_AUTOUPDATER_H_INCLUDED



//==============================================================================
class LatestVersionChecker  : private Thread,
                              private Timer
{
public:
    LatestVersionChecker()  : Thread ("Updater"),
                              hasAttemptedToReadWebsite (false)
    {
        startTimer (2000);
    }

    ~LatestVersionChecker()
    {
        stopThread (20000);
    }

    static URL getLatestVersionURL()
    {
        return URL ("http://www.juce.com/juce/updates/updatelist.php");
    }

    void checkForNewVersion()
    {
        hasAttemptedToReadWebsite = true;

        {
            const ScopedPointer<InputStream> in (getLatestVersionURL().createInputStream (false));

            if (in == nullptr || threadShouldExit())
                return;  // can't connect: fail silently.

            jsonReply = JSON::parse (in->readEntireStreamAsString());
        }

        if (threadShouldExit())
            return;

        if (jsonReply.isArray() || jsonReply.isObject())
            startTimer (100);
    }

    void processResult (var reply)
    {
        if (reply.isArray())
        {
            askUserAboutNewVersion (VersionInfo (reply[0]));
        }
        else if (reply.isObject())
        {
            // In the far-distant future, this may be contacting a defunct
            // URL, so hopefully the website will contain a helpful message
            // for the user..
            String message = reply.getProperty ("message", var()).toString();

            if (message.isNotEmpty())
            {
                AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                             TRANS("JUCE Updater"),
                                             message);
            }
        }
    }

    struct VersionInfo
    {
        VersionInfo (var v)
        {
            version = v.getProperty ("version", var()).toString().trim();

            url = v.getProperty (
                                #if JUCE_MAC
                                 "url_osx",
                                #elif JUCE_WINDOWS
                                 "url_win",
                                 #elif JUCE_LINUX
                                 "url_linux",
                                #endif
                                 var()).toString();
        }

        bool isDifferentVersionToCurrent() const
        {
JUCE_COMPILER_WARNING("testing")
return true;
            return version != JUCE_STRINGIFY(JUCE_MAJOR_VERSION)
                               "." JUCE_STRINGIFY(JUCE_MINOR_VERSION)
                               "." JUCE_STRINGIFY(JUCE_BUILDNUMBER)
                    && version.containsChar ('.')
                    && version.length() > 2;
        }

        String version;
        URL url;
    };

    void askUserAboutNewVersion (const VersionInfo& info)
    {
        if (info.isDifferentVersionToCurrent())
        {
            if (isRunningFromZipFolder())
            {
                if (AlertWindow::showOkCancelBox (AlertWindow::WarningIcon,
                                                  TRANS("Download JUCE version 123?").replace ("123", info.version),
                                                  TRANS("A new version of JUCE is available - would you like to overwrite the folder:\n\n"
                                                        "xfldrx\n\n"
                                                        " ..with the latest version from juce.com?\n\n"
                                                        "(Please note that this will overwrite everything in that folder!)")
                                                    .replace ("xfldrx", getZipFolder().getFullPathName())))
                {
                    DownloadNewVersionThread::performDownload (info.url, getZipFolder());
                }
            }
            else
            {
JUCE_COMPILER_WARNING("todo")

                File targetFolder;

//                FileChooser f;

                    DownloadNewVersionThread::performDownload (info.url, targetFolder);

            }
        }
    }

    static bool isZipFolder (const File& f)
    {
JUCE_COMPILER_WARNING("testing")
return true;

        return f.getChildFile ("modules").isDirectory()
            && f.getChildFile ("extras").isDirectory()
            && f.getChildFile ("examples").isDirectory()
            && ! f.getChildFile (".git").isDirectory();
    }

    static File getZipFolder()
    {
        File appParentFolder (File::getSpecialLocation (File::currentApplicationFile).getParentDirectory());
        return isZipFolder (appParentFolder) ? appParentFolder : File::nonexistent;
    }

    static bool isRunningFromZipFolder()
    {
        return getZipFolder() != File::nonexistent;
    }

private:
    void timerCallback() override
    {
        stopTimer();

        if (hasAttemptedToReadWebsite)
            processResult (jsonReply);
        else
            startThread (3);
    }

    void run() override
    {
        checkForNewVersion();
    }

    var jsonReply;
    bool hasAttemptedToReadWebsite;
    URL newVersionToDownload;

    //==============================================================================
    class DownloadNewVersionThread   : public ThreadWithProgressWindow
    {
    public:
        DownloadNewVersionThread (URL u, File target)
            : ThreadWithProgressWindow ("Downloading New Version", true, true),
              result (Result::ok()),
              url (u), targetFolder (target)
        {
        }

        static void performDownload (URL u, File targetFolder)
        {
            DownloadNewVersionThread d (u, targetFolder);

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
JUCE_COMPILER_WARNING("todo")
                }
            }
        }

        void run() override
        {
            setProgress (-1.0);

            MemoryBlock zipData;
            result = download (zipData);

            if (result.wasOk() && ! threadShouldExit())
                result = unzip (zipData);
        }

        Result download (MemoryBlock& dest)
        {
            setStatusMessage ("Downloading...");

            const ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, String::empty, 10000));

            if (in != nullptr)
            {
                int64 total = 0;
                MemoryOutputStream mo (dest, true);

                for (;;)
                {
                    if (threadShouldExit())
                        return Result::fail ("cancel");

                    size_t written = mo.writeFromInputStream (*in, 8192);

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

        Result unzip (const MemoryBlock& data)
        {
            setStatusMessage ("Installing...");

            File tempUnzipped;

            {
                MemoryInputStream input (data, false);
                ZipFile zip (input);

                if (zip.getNumEntries() == 0)
                    return Result::fail ("The downloaded file wasn't a valid JUCE file!");

                tempUnzipped = targetFolder.getNonexistentSibling();

                if (! tempUnzipped.createDirectory())
                    return Result::fail ("Couldn't create a folder to unzip the new version!");

                Result r (zip.uncompressTo (tempUnzipped));

                if (r.failed())
                {
                    tempUnzipped.deleteRecursively();
                    return r;
                }
            }

            File oldFolder (targetFolder.getSiblingFile (targetFolder.getFileNameWithoutExtension() + "_old").getNonexistentSibling());

            if (! targetFolder.moveFileTo (targetFolder.getNonexistentSibling()))
            {
                tempUnzipped.deleteRecursively();
                return Result::fail ("Could not remove the existing folder!");
            }

            if (! tempUnzipped.moveFileTo (targetFolder))
            {
                tempUnzipped.deleteRecursively();
                return Result::fail ("Could not overwrite the existing folder!");
            }

            return Result::ok();
        }

        Result result;
        URL url;
        File targetFolder;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownloadNewVersionThread)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatestVersionChecker)
};


#endif  // JUCER_AUTOUPDATER_H_INCLUDED
