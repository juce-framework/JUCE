package com.roli.juce;

import android.database.ContentObserver;
import android.app.Activity;
import android.net.Uri;

//==============================================================================
public class SystemVolumeObserver extends ContentObserver
{
    private native void mediaSessionSystemVolumeChanged (long host);

    SystemVolumeObserver (Activity activityToUse, long hostToUse)
    {
        super (null);

        activity = activityToUse;
        host = hostToUse;
    }

    void setEnabled (boolean shouldBeEnabled)
    {
        if (shouldBeEnabled)
            activity.getApplicationContext ().getContentResolver ().registerContentObserver (android.provider.Settings.System.CONTENT_URI, true, this);
        else
            activity.getApplicationContext ().getContentResolver ().unregisterContentObserver (this);
    }

    @Override
    public void onChange (boolean selfChange, Uri uri)
    {
        if (uri.toString ().startsWith ("content://settings/system/volume_music"))
            mediaSessionSystemVolumeChanged (host);
    }

    private Activity activity;
    private long host;
}
