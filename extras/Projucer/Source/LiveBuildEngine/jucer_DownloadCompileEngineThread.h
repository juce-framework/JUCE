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
