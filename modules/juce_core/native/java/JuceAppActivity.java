/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

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
