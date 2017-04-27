/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

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

#pragma once

struct LicenseThread;
class LicenseWebview;

//==============================================================================
struct LicenseState
{
    enum class Type
    {
        notLoggedIn,  // only used when the webview is open and the user has not logged in yet
        noLicenseChosenYet,

        GPL,          // GPL is used when the user enables the GPL compile flag
        personal,
        edu,
        indie,
        pro
    };

    Type type = Type::notLoggedIn;
    String username;
    String email;
    String authToken;

    static const char* licenseTypeToString (Type licenseType);

    bool isPaidOrGPL() const noexcept     { return (type == Type::GPL || type == Type::indie || type == Type::pro); }

    Image avatar;
};

//==============================================================================
class LicenseController
{
public:
    //==============================================================================
    struct StateChangedCallback
    {
        virtual ~StateChangedCallback() {}
        virtual void licenseStateChanged (const LicenseState&) = 0;
    };

    //==============================================================================
    LicenseController();
    ~LicenseController();

    //==============================================================================
    const LicenseState& getState() const noexcept { return state; }
    void logout();
    void chooseNewLicense();

    //==============================================================================
    void addLicenseStatusChangedCallback    (StateChangedCallback* callback) { listeners.add    (callback); }
    void removeLicenseStatusChangedCallback (StateChangedCallback* callback) { listeners.remove (callback); }

private:
    //==============================================================================
    struct ModalCompletionCallback;
    friend struct ModalCompletionCallback;

    friend class ScopedPointer<LicenseThread>;
    friend struct LicenseThread;

    //==============================================================================
    void closeWebview (int);
    void modalStateFinished (int);
    void ensureLicenseWebviewIsOpenWithPage (const String&);
    void queryWebview (const String&, const String&, HashMap<String, String>&);
    void updateState (const LicenseState&);

    static LicenseState licenseStateFromSettings (PropertiesFile&);
    static void licenseStateToSettings (const LicenseState&, PropertiesFile&);

    //==============================================================================
    LicenseState state;
    ScopedPointer<LicenseThread> thread;
    LicenseWebview* licenseWebview = nullptr;
    ListenerList<LicenseController::StateChangedCallback> listeners;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LicenseController)
};
