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

bool FileChooser::isPlatformDialogAvailable()
{
    ChildProcess child;
    const bool ok = child.start ("which zenity")
                     && child.readAllProcessOutput().trim().isNotEmpty();

    child.waitForProcessToFinish (60 * 1000);
    return ok;
}

void FileChooser::showPlatformDialog (Array<File>& results,
                                      const String& title,
                                      const File& file,
                                      const String& filters,
                                      bool isDirectory,
                                      bool selectsFiles,
                                      bool isSave,
                                      bool warnAboutOverwritingExistingFiles,
                                      bool selectMultipleFiles,
                                      FilePreviewComponent* previewComponent)
{
    const String separator (":");
    String command ("zenity --file-selection");

    if (title.isNotEmpty())         command << " --title=\"" << title << "\"";
    if (file != File::nonexistent)  command << " --filename=\"" << file.getFullPathName () << "\"";
    if (isDirectory)                command << " --directory";
    if (isSave)                     command << " --save";
    if (selectMultipleFiles)        command << " --multiple --separator=" << separator;

    command << " 2>&1";

    ChildProcess child;
    if (child.start (command))
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
}
