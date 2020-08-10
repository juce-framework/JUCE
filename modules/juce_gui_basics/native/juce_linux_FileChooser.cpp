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

namespace juce
{

static bool exeIsAvailable (const char* const executable)
{
    ChildProcess child;
    const bool ok = child.start ("which " + String (executable))
                      && child.readAllProcessOutput().trim().isNotEmpty();

    child.waitForProcessToFinish (60 * 1000);
    return ok;
}


class FileChooser::Native    : public FileChooser::Pimpl,
                               private Timer
{
public:
    Native (FileChooser& fileChooser, int flags)
        : owner (fileChooser),
          isDirectory         ((flags & FileBrowserComponent::canSelectDirectories)   != 0),
          isSave              ((flags & FileBrowserComponent::saveMode)               != 0),
          selectMultipleFiles ((flags & FileBrowserComponent::canSelectMultipleItems) != 0),
          warnAboutOverwrite  ((flags & FileBrowserComponent::warnAboutOverwriting)   != 0)
    {
        const File previousWorkingDirectory (File::getCurrentWorkingDirectory());

        // use kdialog for KDE sessions or if zenity is missing
        if (exeIsAvailable ("kdialog") && (isKdeFullSession() || ! exeIsAvailable ("zenity")))
            addKDialogArgs();
        else
            addZenityArgs();
    }

    ~Native() override
    {
        finish (true);
    }

    void runModally() override
    {
        child.start (args, ChildProcess::wantStdOut);

        while (child.isRunning())
            if (! MessageManager::getInstance()->runDispatchLoopUntil(20))
                break;

        finish (false);
    }

    void launch() override
    {
        child.start (args, ChildProcess::wantStdOut);
        startTimer (100);
    }

private:
    FileChooser& owner;
    bool isDirectory, isSave, selectMultipleFiles, warnAboutOverwrite;

    ChildProcess child;
    StringArray args;
    String separator;

    void timerCallback() override
    {
        if (! child.isRunning())
        {
            stopTimer();
            finish (false);
        }
    }

    void finish (bool shouldKill)
    {
        String result;
        Array<URL> selection;

        if (shouldKill)
            child.kill();
        else
            result = child.readAllProcessOutput().trim();

        if (result.isNotEmpty())
        {
            StringArray tokens;

            if (selectMultipleFiles)
                tokens.addTokens (result, separator, "\"");
            else
                tokens.add (result);

            for (auto& token : tokens)
                selection.add (URL (File::getCurrentWorkingDirectory().getChildFile (token)));
        }

        if (! shouldKill)
        {
            child.waitForProcessToFinish (60 * 1000);
            owner.finished (selection);
        }
    }

    static uint64 getTopWindowID() noexcept
    {
        if (TopLevelWindow* top = TopLevelWindow::getActiveTopLevelWindow())
            return (uint64) (pointer_sized_uint) top->getWindowHandle();

        return 0;
    }

    static bool isKdeFullSession()
    {
        return SystemStats::getEnvironmentVariable ("KDE_FULL_SESSION", String())
                     .equalsIgnoreCase ("true");
    }

    void addKDialogArgs()
    {
        args.add ("kdialog");

        if (owner.title.isNotEmpty())
            args.add ("--title=" + owner.title);

        if (uint64 topWindowID = getTopWindowID())
        {
            args.add ("--attach");
            args.add (String (topWindowID));
        }

        if (selectMultipleFiles)
        {
            separator = "\n";
            args.add ("--multiple");
            args.add ("--separate-output");
            args.add ("--getopenfilename");
        }
        else
        {
            if (isSave)             args.add ("--getsavefilename");
            else if (isDirectory)   args.add ("--getexistingdirectory");
            else                    args.add ("--getopenfilename");
        }

        File startPath;

        if (owner.startingFile.exists())
        {
            startPath = owner.startingFile;
        }
        else if (owner.startingFile.getParentDirectory().exists())
        {
            startPath = owner.startingFile.getParentDirectory();
        }
        else
        {
            startPath = File::getSpecialLocation (File::userHomeDirectory);

            if (isSave)
                startPath = startPath.getChildFile (owner.startingFile.getFileName());
        }

        args.add (startPath.getFullPathName());
        args.add (owner.filters.replaceCharacter (';', ' '));
    }

    void addZenityArgs()
    {
        args.add ("zenity");
        args.add ("--file-selection");

        if (warnAboutOverwrite)
            args.add("--confirm-overwrite");

        if (owner.title.isNotEmpty())
            args.add ("--title=" + owner.title);

        if (selectMultipleFiles)
        {
            separator = ":";
            args.add ("--multiple");
            args.add ("--separator=" + separator);
        }
        else
        {
            if (isDirectory)  args.add ("--directory");
            if (isSave)       args.add ("--save");
        }

        if (owner.filters.isNotEmpty() && owner.filters != "*" && owner.filters != "*.*")
        {
            StringArray tokens;
            tokens.addTokens (owner.filters, ";,|", "\"");

            for (int i = 0; i < tokens.size(); ++i)
                args.add ("--file-filter=" + tokens[i]);
        }

        if (owner.startingFile.isDirectory())
            owner.startingFile.setAsCurrentWorkingDirectory();
        else if (owner.startingFile.getParentDirectory().exists())
            owner.startingFile.getParentDirectory().setAsCurrentWorkingDirectory();
        else
            File::getSpecialLocation (File::userHomeDirectory).setAsCurrentWorkingDirectory();

        auto filename = owner.startingFile.getFileName();

        if (! filename.isEmpty())
            args.add ("--filename=" + filename);

        // supplying the window ID of the topmost window makes sure that Zenity pops up..
        if (uint64 topWindowID = getTopWindowID())
            setenv ("WINDOWID", String (topWindowID).toRawUTF8(), true);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Native)
};

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    static bool canUseNativeBox = exeIsAvailable ("zenity") || exeIsAvailable ("kdialog");
    return canUseNativeBox;
   #endif
}

FileChooser::Pimpl* FileChooser::showPlatformDialog (FileChooser& owner, int flags, FilePreviewComponent*)
{
    return new Native (owner, flags);
}

} // namespace juce
