/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

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
