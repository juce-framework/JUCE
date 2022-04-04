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

import android.media.session.MediaController;
import android.media.session.MediaSession;
import android.media.MediaMetadata;
import android.media.session.PlaybackState;

import java.util.List;

//==============================================================================
public class MediaControllerCallback extends MediaController.Callback
{
    private native void mediaControllerAudioInfoChanged (long host, MediaController.PlaybackInfo info);
    private native void mediaControllerMetadataChanged (long host, MediaMetadata metadata);
    private native void mediaControllerPlaybackStateChanged (long host, PlaybackState state);
    private native void mediaControllerSessionDestroyed (long host);

    MediaControllerCallback (long hostToUse)
    {
        host = hostToUse;
    }

    @Override
    public void onAudioInfoChanged (MediaController.PlaybackInfo info)
    {
        mediaControllerAudioInfoChanged (host, info);
    }

    @Override
    public void onMetadataChanged (MediaMetadata metadata)
    {
        mediaControllerMetadataChanged (host, metadata);
    }

    @Override
    public void onPlaybackStateChanged (PlaybackState state)
    {
        mediaControllerPlaybackStateChanged (host, state);
    }

    @Override
    public void onQueueChanged (List<MediaSession.QueueItem> queue)
    {
    }

    @Override
    public void onSessionDestroyed ()
    {
        mediaControllerSessionDestroyed (host);
    }

    private long host;
}
