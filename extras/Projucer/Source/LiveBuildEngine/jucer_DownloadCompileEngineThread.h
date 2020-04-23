/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once


//==============================================================================
class DownloadCompileEngineThread   : public ThreadWithProgressWindow
{
public:
    static bool downloadAndInstall();

protected:
    void run() override;
    void threadComplete (bool userPressedCancel) override;

private:
    DownloadCompileEngineThread();

    Result download (MemoryBlock& dest);
    Result install (const MemoryBlock& data, File& targetFolder);

    static URL getDownloadUrl();
    static File getInstallFolder();
    static bool withError(const String& msg);

    Result result;
    bool cancelledByUser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DownloadCompileEngineThread)
};
