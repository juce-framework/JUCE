/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

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
