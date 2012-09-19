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

FileLogger::FileLogger (const File& file,
                        const String& welcomeMessage,
                        const int64 maxInitialFileSizeBytes)
    : logFile (file)
{
    if (maxInitialFileSizeBytes >= 0)
        trimFileSize (maxInitialFileSizeBytes);

    if (! file.exists())
        file.create();  // (to create the parent directories)

    String welcome;
    welcome << newLine
            << "**********************************************************" << newLine
            << welcomeMessage << newLine
            << "Log started: " << Time::getCurrentTime().toString (true, true) << newLine;

    FileLogger::logMessage (welcome);
}

FileLogger::~FileLogger() {}

//==============================================================================
void FileLogger::logMessage (const String& message)
{
    const ScopedLock sl (logLock);
    DBG (message);
    FileOutputStream out (logFile, 256);
    out << message << newLine;
}

void FileLogger::trimFileSize (int64 maxFileSizeBytes) const
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
            TemporaryFile tempFile (logFile);

            {
                FileOutputStream out (tempFile.getFile());
                FileInputStream in (logFile);

                if (! (out.openedOk() && in.openedOk()))
                    return;

                in.setPosition (fileSize - maxFileSizeBytes);

                for (;;)
                {
                    const char c = in.readByte();
                    if (c == 0)
                        return;

                    if (c == '\n' || c == '\r')
                    {
                        out << c;
                        break;
                    }
                }

                out.writeFromInputStream (in, -1);
            }

            tempFile.overwriteTargetFileWithTemporary();
        }
    }
}

//==============================================================================
File FileLogger::getSystemLogFileFolder()
{
   #if JUCE_MAC
    return File ("~/Library/Logs");
   #else
    return File::getSpecialLocation (File::userApplicationDataDirectory);
   #endif
}

FileLogger* FileLogger::createDefaultAppLogger (const String& logFileSubDirectoryName,
                                                const String& logFileName,
                                                const String& welcomeMessage,
                                                const int64 maxInitialFileSizeBytes)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileName),
                           welcomeMessage, maxInitialFileSizeBytes);
}

FileLogger* FileLogger::createDateStampedLogger (const String& logFileSubDirectoryName,
                                                 const String& logFileNameRoot,
                                                 const String& logFileNameSuffix,
                                                 const String& welcomeMessage)
{
    return new FileLogger (getSystemLogFileFolder().getChildFile (logFileSubDirectoryName)
                                                   .getChildFile (logFileNameRoot + Time::getCurrentTime().formatted ("%Y-%m-%d_%H-%M-%S"))
                                                   .withFileExtension (logFileNameSuffix)
                                                   .getNonexistentSibling(),
                           welcomeMessage, 0);
}
