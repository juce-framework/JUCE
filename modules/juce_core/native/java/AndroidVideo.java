$$VideoApi21
    //==============================================================================
    public class MediaControllerCallback  extends MediaController.Callback
    {
        private native void mediaControllerAudioInfoChanged     (long host, MediaController.PlaybackInfo info);
        private native void mediaControllerMetadataChanged      (long host, MediaMetadata metadata);
        private native void mediaControllerPlaybackStateChanged (long host, PlaybackState state);
        private native void mediaControllerSessionDestroyed     (long host);

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
        public void onQueueChanged (List<MediaSession.QueueItem> queue) {}

        @Override
        public void onSessionDestroyed()
        {
            mediaControllerSessionDestroyed (host);
        }

        private long host;
    }

    //==============================================================================
    public class MediaSessionCallback  extends MediaSession.Callback
    {
        private native void mediaSessionPause           (long host);
        private native void mediaSessionPlay            (long host);
        private native void mediaSessionPlayFromMediaId (long host, String mediaId, Bundle extras);
        private native void mediaSessionSeekTo          (long host, long pos);
        private native void mediaSessionStop            (long host);


        MediaSessionCallback (long hostToUse)
        {
            host = hostToUse;
        }

        @Override
        public void onPause()
        {
            mediaSessionPause (host);
        }

        @Override
        public void onPlay()
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
        public void onStop()
        {
            mediaSessionStop (host);
        }

        @Override
        public void onFastForward() {}

        @Override
        public boolean onMediaButtonEvent (Intent mediaButtonIntent)
        {
            return true;
        }

        @Override
        public void onRewind() {}

        @Override
        public void onSkipToNext() {}

        @Override
        public void onSkipToPrevious() {}

        @Override
        public void onSkipToQueueItem (long id) {}

        private long host;
    }

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
                activity.getApplicationContext().getContentResolver().registerContentObserver (android.provider.Settings.System.CONTENT_URI, true, this);
            else
                activity.getApplicationContext().getContentResolver().unregisterContentObserver (this);
        }

        @Override
        public void onChange (boolean selfChange, Uri uri)
        {
            if (uri.toString().startsWith ("content://settings/system/volume_music"))
                mediaSessionSystemVolumeChanged (host);
        }

        private Activity activity;
        private long host;
    }

VideoApi21$$
