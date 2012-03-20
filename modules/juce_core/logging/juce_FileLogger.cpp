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

FileLogger::FileLogger (const File& logFile_,
                        const String& welcomeMessage,
                        const int maxInitialFileSizeBytes)
    : logFile (logFile_)
{
    if (maxInitialFileSizeBytes >= 0)
        trimFileSize (maxInitialFileSizeBytes);

    if (! logFile_.exists())
    {
        // do this so that the parent directories get created..
        logFile_.create();
    }

    String welcome;
    welcome << newLine
            << "**********************************************************" << newLine
            << welcomeMessage << newLine
            << "Log started: " << Time::getCurrentTime().toString (true, true) << newLine;

    FileLogger::logMessage (welcome);
}

FileLogger::~FileLogger()
{
}

//==============================================================================
void FileLogger::logMessage (const String& message)
{
    DBG (message);

    const ScopedLock sl (logLock);

    FileOutputStream out (logFile, 256);
    out << message << newLine;
}


void FileLogger::trimFileSize (int maxFileSizeBytes) const
{
    if (maxFileSizeBytes <= 0)
    {
        logFile.deleteFile();
    }
    else
    {
        const int64 fileSize = logFile.getSize();

        if (fileSize > maxFileSizeBytes)
        {
            ScopedPointer <FileInputStream> in (logFile.createInputStream());
            jassert (in != nullptr);

            if (in != nullptr)
            {
                in->setPosition (fileSize - maxFileSizeBytes);
                String content;

                {
                    MemoryBlock contentToSave;
                    contentToSave.setSize ((size_t) maxFileSizeBytes + 4);
                    contentToSave.fillWith (0);

                    in->read (contentToSave.getData(), maxFileSizeBytes);
                    in = nullptr;

                    content = contentToSave.toString();
                }

                int newStart = 0;

                while (newStart < fileSize
                        && content[newStart] != '\n'
                        && content[newStart] != '\r')
                    ++newStart;

                logFile.deleteFile();
                logFile.appendText (content.substring (newStart), false, false);
            }
        }
    }
}

//==============================================================================
FileLogger* FileLogger::createDefaultAppLogger (const String& logFileSubDirectoryName,
                                                const String& logFileName,
                                                const String& welcomeMessage,
                                                const int maxInitialFileSizeBytes)
{
   #if JUCE_MAC
    File logFile ("~/Library/Logs");
    logFile = logFile.getChildFile (logFileSubDirectoryName)
                     .getChildFile (logFileName);

   #else
    File logFile (File::getSpecialLocation (File::userApplicationDataDirectory));

    if (logFile.isDirectory())
    {
        logFile = logFile.getChildFile (logFileSubDirectoryName)
                         .getChildFile (logFileName);
    }
   #endif

    return new FileLogger (logFile, welcomeMessage, maxInitialFileSizeBytes);
}
