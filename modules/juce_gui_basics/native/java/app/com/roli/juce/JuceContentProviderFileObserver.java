package com.roli.juce;

import android.os.FileObserver;

import java.lang.String;

public final class JuceContentProviderFileObserver extends FileObserver
{
    public JuceContentProviderFileObserver (long hostToUse, String path, int mask)
    {
        super (path, mask);

        host = hostToUse;
    }

    public void onEvent (int event, String path)
    {
        contentSharerFileObserverEvent (host, event, path);
    }

    private long host;

    private native void contentSharerFileObserverEvent (long host, int event, String path);
}
