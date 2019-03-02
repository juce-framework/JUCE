package com.roli.juce;

import android.view.OrientationEventListener;
import android.content.Context;

public class JuceOrientationEventListener extends OrientationEventListener
{
    private native void deviceOrientationChanged (long host, int orientation);

    public JuceOrientationEventListener (long hostToUse, Context context, int rate)
    {
        super (context, rate);

        host = hostToUse;
    }

    @Override
    public void onOrientationChanged (int orientation)
    {
        deviceOrientationChanged (host, orientation);
    }

    private long host;
}
