package com.roli.juce;

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
