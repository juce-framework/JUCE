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

static bool exeIsAvailable (const char* const executable)
{
     ChildProcess child;
     const bool ok = child.start ("which " + String (executable))
                       && child.readAllProcessOutput().trim().isNotEmpty();

     child.waitForProcessToFinish (60 * 1000);
     return ok;
}

bool FileChooser::isPlatformDialogAvailable()
{
   #if JUCE_DISABLE_NATIVE_FILECHOOSERS
    return false;
   #else
    static bool canUseNativeBox = exeIsAvailable ("zenity") || exeIsAvailable ("kdialog");
    return canUseNativeBox;
   #endif
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

static void addKDialogArgs (StringArray& args, String& separator,
                            const String& title, const File& file, const String& filters,
                            bool isDirectory, bool isSave, bool selectMultipleFiles)
{
    args.add ("kdialog");

    if (title.isNotEmpty())
        args.add ("--title=" + title);

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

    if (file.exists())
    {
        startPath = file;
    }
    else if (file.getParentDirectory().exists())
    {
        startPath = file.getParentDirectory();
    }
    else
    {
        startPath = File::getSpecialLocation (File::userHomeDirectory);

        if (isSave)
            startPath = startPath.getChildFile (file.getFileName());
    }

    args.add (startPath.getFullPathName());
    args.add (filters.replaceCharacter (';', ' '));
}

static void addZenityArgs (StringArray& args, String& separator,
                           const String& title, const File& file, const String& filters,
                           bool isDirectory, bool isSave, bool selectMultipleFiles)
{
    args.add ("zenity");
    args.add ("--file-selection");

    if (title.isNotEmpty())
        args.add ("--title=" + title);

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

    if (filters.isNotEmpty() && filters != "*" && filters != "*.*")
    {
        args.add ("--file-filter");
        args.add (filters.replaceCharacter (';', ' '));

        args.add ("--file-filter");
        args.add ("All files | *");
    }

    if (file.isDirectory())
        file.setAsCurrentWorkingDirectory();
    else if (file.getParentDirectory().exists())
        file.getParentDirectory().setAsCurrentWorkingDirectory();
    else
        File::getSpecialLocation (File::userHomeDirectory).setAsCurrentWorkingDirectory();

    if (! file.getFileName().isEmpty())
        args.add ("--filename=" + file.getFileName());

    // supplying the window ID of the topmost window makes sure that Zenity pops up..
    if (uint64 topWindowID = getTopWindowID())
        setenv ("WINDOWID", String (topWindowID).toRawUTF8(), true);
}

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title, const File& file, const String& filters,
                                      bool isDirectory, bool /* selectsFiles */,
                                      bool isSave, bool /* warnAboutOverwritingExistingFiles */,
                                      bool selectMultipleFiles, FilePreviewComponent*)
{
    const File previousWorkingDirectory (File::getCurrentWorkingDirectory());

    StringArray args;
    String separator;

    // use kdialog for KDE sessions or if zenity is missing
    if (exeIsAvailable ("kdialog") && (isKdeFullSession() || ! exeIsAvailable ("zenity")))
        addKDialogArgs (args, separator, title, file, filters, isDirectory, isSave, selectMultipleFiles);
    else
        addZenityArgs (args, separator, title, file, filters, isDirectory, isSave, selectMultipleFiles);

    args.add ("2>/dev/null"); // (to avoid logging info ending up in the results)

    ChildProcess child;

    if (child.start (args, ChildProcess::wantStdOut))
    {
        const String result (child.readAllProcessOutput().trim());

        if (result.isNotEmpty())
        {
            StringArray tokens;

            if (selectMultipleFiles)
                tokens.addTokens (result, separator, "\"");
            else
                tokens.add (result);

            for (int i = 0; i < tokens.size(); ++i)
                results.add (File::getCurrentWorkingDirectory().getChildFile (tokens[i]));
        }

        child.waitForProcessToFinish (60 * 1000);
    }

    previousWorkingDirectory.setAsCurrentWorkingDirectory();
}
