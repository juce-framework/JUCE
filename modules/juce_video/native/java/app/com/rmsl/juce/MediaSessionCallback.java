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
