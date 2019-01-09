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

class UpdaterDialogModalCallback;

//==============================================================================
class LatestVersionChecker  : private Thread,
                              private Timer
{
public:
    struct JuceVersionTriple
    {
        JuceVersionTriple();
        JuceVersionTriple (int juceVersionNumber);
        JuceVersionTriple (int majorInt, int minorInt, int buildNumber);

        static bool fromString (const String& versionString, JuceVersionTriple& result);
        String toString() const;

        bool operator> (const JuceVersionTriple& b) const noexcept;

        int major, minor, build;
    };

    //==============================================================================
    struct JuceServerLocationsAndKeys
    {
        const char* updateSeverHostname;
        const char* publicAPIKey;
        int apiVersion;
        const char* updatePath;
    };

    //==============================================================================
    LatestVersionChecker();
    ~LatestVersionChecker() override;

    static String getOSString();

    URL getLatestVersionURL (String& headers, const String& path) const;
    URL getLatestVersionURL (String& headers) const;

    void checkForNewVersion();
    bool processResult (var reply, const String& downloadPath);

    bool askUserAboutNewVersion (const JuceVersionTriple& version,
                                 const String& releaseNotes,
                                 URL& newVersionToDownload,
                                 const String& extraHeaders);

    void askUserForLocationToDownload (URL& newVersionToDownload, const String& extraHeaders);

    static bool isZipFolder (const File&);

    virtual Result performUpdate (const MemoryBlock& data, File& targetFolder);

protected:
    const JuceServerLocationsAndKeys& getJuceServerURLsAndKeys() const;
    int getProductVersionNumber() const;
    const char* getProductName() const;
    bool allowCustomLocation() const;

private:
    //==============================================================================
    friend class UpdaterDialogModalCallback;

    // callbacks
    void timerCallback() override;
    void run() override;
    void modalStateFinished (int result,
                             URL& newVersionToDownload,
                             const String& extraHeaders,
                             File appParentFolder);

    int statusCode;
    var jsonReply;
    bool hasAttemptedToReadWebsite;
    String newRelativeDownloadPath;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LatestVersionChecker)
};
