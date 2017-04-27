/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "../jucer_Headers.h"
#include "jucer_DownloadCompileEngineThread.h"
#include "../LiveBuildEngine/projucer_CompileEngineDLL.h"

bool DownloadCompileEngineThread::downloadAndInstall()
{
    DownloadCompileEngineThread d;

    if (d.runThread())
    {
        if (d.result.failed())
            return withError (d.result.getErrorMessage());

        return true;
    }

    if (d.cancelledByUser)
        return false;

    return withError (d.result.getErrorMessage());
}

DownloadCompileEngineThread::DownloadCompileEngineThread()
    : ThreadWithProgressWindow ("Downloading live-build engine", true, true),
      result (Result::ok()), cancelledByUser (false)
{
}

void DownloadCompileEngineThread::threadComplete (bool userPressedCancel)
{
    cancelledByUser = userPressedCancel;
}

void DownloadCompileEngineThread::run()
{
    setProgress (-1.0);
    setStatusMessage ("Downloading...");

    MemoryBlock zipData;
    result = download (zipData);

    if (result.failed())
        return;

    setStatusMessage ("Installing...");

    File installFolder = getInstallFolder();
    if (! installFolder.createDirectory())
    {
        result = Result::fail ("Install error: cannot create target directory");
        return;
    }

    result = install (zipData, installFolder);
}

Result DownloadCompileEngineThread::download (MemoryBlock& dest)
{
    int statusCode = 302;
    const int timeoutMs = 10000;
    StringPairArray responseHeaders;

    URL url = getDownloadUrl();
    ScopedPointer<InputStream> in = url.createInputStream (false, nullptr, nullptr,
                                                           String(), timeoutMs, &responseHeaders,
                                                           &statusCode, 0);

    if (in == nullptr || statusCode != 200)
        return Result::fail ("Download error: cannot establish connection");

    MemoryOutputStream mo (dest, true);

    int64 size = in->getTotalLength();
    int64 bytesReceived = -1;
    String msg("Downloading...  (123)");

    for (int64 pos = 0; pos < size; pos += bytesReceived)
    {
        setStatusMessage (msg.replace ("123", File::descriptionOfSizeInBytes (pos)));

        if (threadShouldExit())
            return Result::fail ("Download error: operation interrupted");

        bytesReceived = mo.writeFromInputStream (*in, 8192);

        if (bytesReceived == 0)
            return Result::fail ("Download error: lost connection");
    }

    return Result::ok();
}

Result DownloadCompileEngineThread::install (const MemoryBlock& data, File& targetFolder)
{
    MemoryInputStream input (data, false);
    ZipFile zip (input);

    if (zip.getNumEntries() == 0)
        return Result::fail ("Install error: downloaded file is corrupt");

    if (threadShouldExit())
        return Result::fail ("Install error: operation interrupted");

    return zip.uncompressTo (targetFolder);
}

URL DownloadCompileEngineThread::getDownloadUrl()
{
    String urlStub ("http://assets.roli.com/juce/JUCECompileEngine_");

   #if JUCE_MAC
    urlStub << "osx_";
   #elif JUCE_WINDOWS
    urlStub << "windows_";
   #else
    jassertfalse;
   #endif

    return urlStub + ProjectInfo::versionString + ".zip";
}

File DownloadCompileEngineThread::getInstallFolder()
{
    return CompileEngineDLL::getVersionedUserAppSupportFolder();
}

bool DownloadCompileEngineThread::withError(const String& msg)
{
    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                 "Download and install", msg);
    return false;
}
