/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

package com.rmsl.juce;

import android.media.session.MediaSession;

import java.lang.String;

import android.os.Bundle;
import android.content.Intent;

import java.util.List;

//==============================================================================
public class MediaSessionCallback extends MediaSession.Callback
{
    private native void mediaSessionPause (long host);
    private native void mediaSessionPlay (long host);
    private native void mediaSessionPlayFromMediaId (long host, String mediaId, Bundle extras);
    private native void mediaSessionSeekTo (long host, long pos);
    private native void mediaSessionStop (long host);

    MediaSessionCallback (long hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public void onPause ()
    {
        mediaSessionPause (host);
    }

    @Override
    public void onPlay ()
    {
        mediaSessionPlay (host);
    }

    @Override
    public void onPlayFromMediaId (String mediaId, Bundle extras)
    {
        mediaSessionPlayFromMediaId (host, mediaId, extras);
    }

    @Override
    public void onSeekTo (long pos)
    {
        mediaSessionSeekTo (host, pos);
    }

    @Override
    public void onStop ()
    {
        mediaSessionStop (host);
    }

    @Override
    public void onFastForward () {}

    @Override
    public boolean onMediaButtonEvent (Intent mediaButtonIntent)
    {
        return true;
    }

    @Override
    public void onRewind () {}

    @Override
    public void onSkipToNext () {}

    @Override
    public void onSkipToPrevious () {}

    @Override
    public void onSkipToQueueItem (long id) {}

    private long host;
}
