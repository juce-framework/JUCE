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

#ifndef JUCER_AUTOUPDATER_H_INCLUDED
#define JUCER_AUTOUPDATER_H_INCLUDED

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
    ~LatestVersionChecker();

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


#endif   // JUCER_AUTOUPDATER_H_INCLUDED
