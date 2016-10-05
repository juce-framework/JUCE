/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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
