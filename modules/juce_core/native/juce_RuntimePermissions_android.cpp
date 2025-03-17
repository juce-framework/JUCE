/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
static StringArray jucePermissionToAndroidPermissions (RuntimePermissions::PermissionID permission)
{
    const auto externalStorageOrMedia = [] (const auto* newPermission)
    {
        return getAndroidSDKVersion() < 33 ? "android.permission.READ_EXTERNAL_STORAGE" : newPermission;
    };

    switch (permission)
    {
        case RuntimePermissions::recordAudio:           return { "android.permission.RECORD_AUDIO" };
        case RuntimePermissions::bluetoothMidi:
        {
            if (getAndroidSDKVersion() < 31)
                return { "android.permission.ACCESS_FINE_LOCATION" };

            return { "android.permission.BLUETOOTH_SCAN",
                     "android.permission.BLUETOOTH_CONNECT" };
        }

        // WRITE_EXTERNAL_STORAGE has no effect on SDK 29+
        case RuntimePermissions::writeExternalStorage:
            return getAndroidSDKVersion() < 29 ? StringArray { "android.permission.WRITE_EXTERNAL_STORAGE" }
                                               : StringArray{};

        case RuntimePermissions::camera:                return { "android.permission.CAMERA" };

        case RuntimePermissions::readExternalStorage:
        {
            // See: https://developer.android.com/reference/android/Manifest.permission#READ_EXTERNAL_STORAGE
            if (getAndroidSDKVersion() < 33)
                return { "android.permission.READ_EXTERNAL_STORAGE" };

            return { "android.permission.READ_MEDIA_AUDIO",
                     "android.permission.READ_MEDIA_IMAGES",
                     "android.permission.READ_MEDIA_VIDEO" };
        }

        case RuntimePermissions::readMediaAudio:
            return { externalStorageOrMedia ("android.permission.READ_MEDIA_AUDIO") };

        case RuntimePermissions::readMediaImages:
            return { externalStorageOrMedia ("android.permission.READ_MEDIA_IMAGES") };

        case RuntimePermissions::readMediaVideo:
            return { externalStorageOrMedia ("android.permission.READ_MEDIA_VIDEO") };

        case RuntimePermissions::postNotification:
            return { "android.permission.POST_NOTIFICATIONS" };
    }

    // invalid permission
    jassertfalse;
    return {};
}

static RuntimePermissions::PermissionID androidPermissionToJucePermission (const String& permission)
{
    static const std::map<String, RuntimePermissions::PermissionID> map
    {
        { "android.permission.RECORD_AUDIO",            RuntimePermissions::recordAudio },
        { "android.permission.ACCESS_FINE_LOCATION",    RuntimePermissions::bluetoothMidi },
        { "android.permission.READ_EXTERNAL_STORAGE",   RuntimePermissions::readExternalStorage },
        { "android.permission.WRITE_EXTERNAL_STORAGE",  RuntimePermissions::writeExternalStorage },
        { "android.permission.CAMERA",                  RuntimePermissions::camera },
        { "android.permission.READ_MEDIA_AUDIO",        RuntimePermissions::readMediaAudio },
        { "android.permission.READ_MEDIA_IMAGES",       RuntimePermissions::readMediaImages },
        { "android.permission.READ_MEDIA_VIDEO",        RuntimePermissions::readMediaVideo },
        { "android.permission.BLUETOOTH_SCAN",          RuntimePermissions::bluetoothMidi },
        { "android.permission.POST_NOTIFICATIONS",      RuntimePermissions::postNotification },
    };

    const auto iter = map.find (permission);
    return iter != map.cend() ? iter->second
                              : static_cast<RuntimePermissions::PermissionID> (-1);
}

//==============================================================================
struct PermissionsRequest
{
    RuntimePermissions::Callback callback;
    RuntimePermissions::PermissionID permission = static_cast<RuntimePermissions::PermissionID> (-1);
};

//==============================================================================
struct PermissionsOverlay final : public FragmentOverlay
{
    explicit PermissionsOverlay (CriticalSection& cs) : overlayGuard (cs) {}

    struct PermissionResult
    {
        PermissionsRequest request;
        bool granted;
    };

    void onStart() override    { onRequestPermissionsResult (0, {}, {}); }

    void onRequestPermissionsResult (int /*requestCode*/,
                                     const StringArray& permissions,
                                     const Array<int>& grantResults) override
    {
        std::vector<PermissionResult> results;

        {
            ScopedLock lock (overlayGuard);

            for (auto it = requests.begin(); it != requests.end();)
            {
                auto& request = *it;

                if (RuntimePermissions::isGranted (request.permission))
                {
                    results.push_back ({std::move (request), true});
                    it = requests.erase (it);
                }
                else
                {
                    ++it;
                }
            }

            auto n = permissions.size();

            for (int i = 0; i < n; ++i)
            {
                auto permission = androidPermissionToJucePermission (permissions[i]);
                auto granted = (grantResults.getReference (i) == 0);

                for (auto it = requests.begin(); it != requests.end();)
                {
                    auto& request = *it;

                    if (request.permission == permission)
                    {
                        results.push_back ({std::move (request), granted});
                        it = requests.erase (it);
                    }
                    else
                    {
                        ++it;
                    }
                }
            }
        }

        for (const auto& result : results)
            if (result.request.callback)
                result.request.callback (result.granted);

        {
            auto* env = getEnv();
            ScopedLock lock (overlayGuard);

            if (requests.size() > 0)
            {
                auto &request = requests.front();

                auto permissionsArray = jucePermissionToAndroidPermissions (request.permission);
                auto jPermissionsArray = juceStringArrayToJava (permissionsArray);


                auto requestPermissionsMethodID
                    = env->GetMethodID (AndroidFragment, "requestPermissions", "([Ljava/lang/String;I)V");

                // this code should only be reached for SDKs >= 23, so this method should be
                // be available
                jassert (requestPermissionsMethodID != nullptr);

                env->CallVoidMethod (getNativeHandle(), requestPermissionsMethodID, jPermissionsArray.get(), 0);
            }
            else
            {
                getSingleton() = nullptr;
            }
        }
    }

    static std::unique_ptr<PermissionsOverlay>& getSingleton()
    {
        static std::unique_ptr<PermissionsOverlay> instance;
        return instance;
    }

    CriticalSection& overlayGuard;
    std::vector<PermissionsRequest> requests;
};

//==============================================================================
void RuntimePermissions::request (PermissionID permission, Callback callback)
{
    const auto requestedPermissions = jucePermissionToAndroidPermissions (permission);

    const auto allPermissionsInManifest = std::all_of (requestedPermissions.begin(),
                                                       requestedPermissions.end(),
                                                       [] (const auto& p)
                                                       {
                                                           return isPermissionDeclaredInManifest (p);
                                                       });

    if (! allPermissionsInManifest)
    {
        // Error! If you want to be able to request this runtime permission, you
        // also need to declare it in your app's manifest. You can do so via
        // the Projucer. Otherwise this can't work.
        jassertfalse;

        callback (false);
        return;
    }

    auto alreadyGranted = isGranted (permission);

    if (alreadyGranted)
    {
        callback (alreadyGranted);
        return;
    }

    PermissionsRequest request { std::move (callback), permission };

    static CriticalSection overlayGuard;
    ScopedLock lock (overlayGuard);

    std::unique_ptr<PermissionsOverlay>& overlay = PermissionsOverlay::getSingleton();

    bool alreadyOpen = true;

    if (overlay == nullptr)
    {
        overlay.reset (new PermissionsOverlay (overlayGuard));
        alreadyOpen = false;
    }

    overlay->requests.push_back (std::move (request));

    if (! alreadyOpen)
        overlay->open();
}

bool RuntimePermissions::isRequired (PermissionID /*permission*/)
{
    return true;
}

bool RuntimePermissions::isGranted (PermissionID permission)
{
    auto* env = getEnv();

    const auto requestedPermissions = jucePermissionToAndroidPermissions (permission);

    return std::all_of (requestedPermissions.begin(), requestedPermissions.end(), [env] (const auto& p)
    {
        return 0 == env->CallIntMethod (getAppContext().get(),
                                        AndroidContext.checkCallingOrSelfPermission,
                                        javaString (p).get());
    });

}

} // namespace juce
