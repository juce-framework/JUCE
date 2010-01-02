/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#include "juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE


#include "juce_FileLogger.h"
#include "../io/files/juce_FileOutputStream.h"
#include "../io/files/juce_FileInputStream.h"
#include "../threads/juce_ScopedLock.h"
#include "../containers/juce_ScopedPointer.h"
#include "juce_SystemStats.h"


//==============================================================================
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

    logStream = logFile_.createOutputStream (256);
    jassert (logStream != 0);

    String welcome;
    welcome << "\r\n**********************************************************\r\n"
            << welcomeMessage
            << "\r\nLog started: " << Time::getCurrentTime().toString (true, true)
            << "\r\n";

    logMessage (welcome);
}

FileLogger::~FileLogger()
{
}

//==============================================================================
void FileLogger::logMessage (const String& message)
{
    if (logStream != 0)
    {
        Logger::outputDebugString (message);

        const ScopedLock sl (logLock);
        (*logStream) << message << T("\r\n");
        logStream->flush();
    }
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
            jassert (in != 0);

            if (in != 0)
            {
                in->setPosition (fileSize - maxFileSizeBytes);
                String content;

                {
                    MemoryBlock contentToSave;
                    contentToSave.setSize (maxFileSizeBytes + 4);
                    contentToSave.fillWith (0);

                    in->read (contentToSave.getData(), maxFileSizeBytes);
                    in = 0;

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
    logFile = logFile.getChildFile (logFileName);

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

END_JUCE_NAMESPACE
