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

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& file,
                                      const String& filters,
                                      bool isDirectory,
                                      bool /* selectsFiles */,
                                      bool isSave,
                                      bool /* warnAboutOverwritingExistingFiles */,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* /* previewComponent */)
{
    String separator;
    StringArray args;

    const File previousWorkingDirectory (File::getCurrentWorkingDirectory());
    const bool isKdeFullSession = SystemStats::getEnvironmentVariable ("KDE_FULL_SESSION", String::empty)
                                    .equalsIgnoreCase ("true");

    if (exeIsAvailable ("kdialog") && (isKdeFullSession || ! exeIsAvailable ("zenity")))
    {
        // use kdialog for KDE sessions or if zenity is missing
        args.add ("kdialog");

        if (title.isNotEmpty())
            args.add ("--title=" + title);

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

        String startPath;

        if (file.exists())
        {
            startPath = file.getFullPathName();
        }
        else if (file.getParentDirectory().exists())
        {
            startPath = file.getParentDirectory().getFullPathName();
        }
        else
        {
            startPath = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

            if (isSave)
                startPath += "/" + file.getFileName();
        }

        args.add (startPath);
        args.add (filters.replaceCharacter (';', ' '));
    }
    else
    {
        // zenity
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

        if (file.isDirectory())
            file.setAsCurrentWorkingDirectory();
        else if (file.getParentDirectory().exists())
            file.getParentDirectory().setAsCurrentWorkingDirectory();
        else
            File::getSpecialLocation (File::userHomeDirectory).setAsCurrentWorkingDirectory();

        if (! file.getFileName().isEmpty())
            args.add ("--filename=" + file.getFileName());
    }

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
