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

    void downloadNewVersion (URL url)
    {
        const ScopedPointer<InputStream> in (getLatestVersionURL().createInputStream (false));

        if (in == nullptr || threadShouldExit())
            return;  // can't connect: fail silently.

            jsonReply = JSON::parse (in->readEntireStreamAsString());

    }

    void processResult (var reply)
    {
        DBG (JSON::toString (reply));

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

        String version;
        URL url;
    };

    void askUserAboutNewVersion (const VersionInfo& info)
    {
        if (info.version != SystemStats::getJUCEVersion()
             && info.version.containsChar ('.')
             && info.version.length() > 2)
        {
            DBG (info.version);
            DBG (info.url.toString (true));

            if (isRunningFromZipFolder())
            {
JUCE_COMPILER_WARNING("todo")

//                startDownload (info.url);
            }
            else
            {
JUCE_COMPILER_WARNING("todo")

//                startDownload (info.url);

            }
        }
    }

    void startDownload (URL url)
    {
        jassert (! isThreadRunning());
        newVersionToDownload = url;
        startThread (3);
    }

    bool isRunningFromZipFolder() const
    {
        File appParentFolder (File::getSpecialLocation (File::currentApplicationFile));

        return appParentFolder.getChildFile ("modules").isDirectory()
            && appParentFolder.getChildFile ("extras").isDirectory()
            && appParentFolder.getChildFile ("examples").isDirectory()
            && ! appParentFolder.getChildFile (".git").isDirectory();
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
        if (newVersionToDownload.isEmpty())
            checkForNewVersion();
        else
            downloadNewVersion (newVersionToDownload);
    }

    var jsonReply;
    bool hasAttemptedToReadWebsite;
    URL newVersionToDownload;

    //==============================================================================
    class DownloadNewVersionThread   : public ThreadWithProgressWindow
    {
    public:
        DownloadNewVersionThread (URL u)
            : ThreadWithProgressWindow ("Downloading New Version", true, true),
              result (Result::ok()),
              url (u)
        {
        }

        static void update (URL u)
        {
            DownloadNewVersionThread d (u);

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
            MemoryBlock zipData;
            result = download (zipData);

            if (result.wasOk() && ! threadShouldExit())
                result = unzip (zipData);
        }

        Result download (MemoryBlock& dest)
        {
            setStatusMessage ("Downloading...");

            const ScopedPointer<InputStream> in (url.createInputStream (false, nullptr, nullptr, String::empty, 10000));

            if (in != nullptr && in->readIntoMemoryBlock (dest))
                return Result::ok();

            return Result::fail ("Failed to download from: " + url.toString (false));
        }

        Result unzip (const MemoryBlock& data)
        {
            setStatusMessage ("Installing...");

            MemoryInputStream input (data, false);
            ZipFile zip (input);

            if (zip.getNumEntries() == 0)
                return Result::fail ("The downloaded file wasn't a valid JUCE file!");

//            if (! m.getFolder().deleteRecursively())
//                return Result::fail ("Couldn't delete the existing folder:\n" + m.getFolder().getFullPathName());
//
//            return zip.uncompressTo (m.getFolder().getParentDirectory(), true);
        }

        Result result;
        URL url;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownloadNewVersionThread)
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatestVersionChecker)
};


#endif  // JUCER_AUTOUPDATER_H_INCLUDED
