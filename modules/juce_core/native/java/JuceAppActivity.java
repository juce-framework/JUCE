/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

package com.juce;

import android.app.Activity;
import android.os.Bundle;
import android.media.AudioManager;


//==============================================================================
public class JuceAppActivity   extends Activity
{
    private JuceBridge juceBridge;

    @Override
    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        juceBridge = JuceBridge.getInstance();
        juceBridge.setActivityContext(this);
        juceBridge.setScreenSaver(true);
        juceBridge.hideActionBar();
        setContentView(juceBridge.getViewHolder());

        setVolumeControlStream(AudioManager.STREAM_MUSIC);
    }

    @Override
    protected void onDestroy()
    {
        juceBridge.quitApp();
        super.onDestroy();

        juceBridge.clearDataCache();
    }

    @Override
    protected void onPause()
    {
        juceBridge.suspendApp();

        try
        {
            Thread.sleep(1000); // This is a bit of a hack to avoid some hard-to-track-down
                                // openGL glitches when pausing/resuming apps..
        } catch (InterruptedException e) {}

        super.onPause();
    }

    @Override
    protected void onResume()
    {
        super.onResume();
        juceBridge.resumeApp();
    }

    @Override
    public void onRequestPermissionsResult (int permissionID, String permissions[], int[] grantResults)
    {
        juceBridge.onRequestPermissionsResult (permissionID, permissions, grantResults);
    }
}
