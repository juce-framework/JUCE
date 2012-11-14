/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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
    return exeIsAvailable ("zenity") || exeIsAvailable ("kdialog");
}

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& file,
                                      const String& /* filters */,
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

        if (file.exists() || file.getParentDirectory().exists())
        {
            startPath = file.getFullPathName();
        }
        else
        {
            startPath = File::getSpecialLocation (File::userHomeDirectory).getFullPathName();

            if (isSave)
                startPath += "/" + file.getFileName();
        }

        args.add (startPath);
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

    ChildProcess child;
    if (child.start (args))
    {
        const String result (child.readAllProcessOutput().trim());

        if (result.isNotEmpty())
        {
            StringArray tokens;

            if (selectMultipleFiles)
                tokens.addTokens (result, separator, "\"");
            else
                tokens.add (result);

            for (int i = 0; i < tokens.size(); i++)
                results.add (File (tokens[i]));
        }

        child.waitForProcessToFinish (60 * 1000);
    }

    previousWorkingDirectory.setAsCurrentWorkingDirectory();
}
