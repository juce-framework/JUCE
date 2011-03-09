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

// (This file gets included by juce_linux_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE

//==============================================================================
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

    if (title.isNotEmpty())
        command << " --title=\"" << title << "\"";

    if (file != File::nonexistent)
        command << " --filename=\"" << file.getFullPathName () << "\"";

    if (isDirectory)
        command << " --directory";

    if (isSave)
        command << " --save";

    if (selectMultipleFiles)
        command << " --multiple --separator=\"" << separator << "\"";

    command << " 2>&1";

    MemoryOutputStream result;
    int status = -1;
    FILE* stream = popen (command.toUTF8(), "r");

    if (stream != 0)
    {
        for (;;)
        {
            char buffer [1024];
            const int bytesRead = fread (buffer, 1, sizeof (buffer), stream);

            if (bytesRead <= 0)
                break;

            result.write (buffer, bytesRead);
        }

        status = pclose (stream);
    }

    if (status == 0)
    {
        StringArray tokens;

        if (selectMultipleFiles)
            tokens.addTokens (result.toUTF8(), separator, String::empty);
        else
            tokens.add (result.toUTF8());

        for (int i = 0; i < tokens.size(); i++)
            results.add (File (tokens[i]));

        return;
    }

    //xxx ain't got one!
    jassertfalse;
}

#endif
