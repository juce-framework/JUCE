/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2017 - ROLI Ltd.

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#pragma once

#include "../Utility/Helpers/jucer_VersionInfo.h"

class DownloadAndInstallThread;

class LatestVersionCheckerAndUpdater   : public DeletedAtShutdown,
                                         private Thread
{
public:
    LatestVersionCheckerAndUpdater();
    ~LatestVersionCheckerAndUpdater() override;

    void checkForNewVersion (bool showAlerts);

    //==============================================================================
    JUCE_DECLARE_SINGLETON_SINGLETHREADED_MINIMAL (LatestVersionCheckerAndUpdater)

private:
    //==============================================================================
    void run() override;
    void askUserAboutNewVersion (const String&, const String&, const VersionInfo::Asset&);
    void askUserForLocationToDownload (const VersionInfo::Asset&);
    void downloadAndInstall (const VersionInfo::Asset&, const File&);

    //==============================================================================
    bool showAlertWindows = false;

    std::unique_ptr<DownloadAndInstallThread> installer;
    std::unique_ptr<Component> dialogWindow;
};
