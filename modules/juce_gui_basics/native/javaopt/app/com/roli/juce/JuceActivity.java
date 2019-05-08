package com.roli.juce;

import android.app.Activity;
import android.content.Intent;

//==============================================================================
public class JuceActivity   extends Activity
{
    //==============================================================================
    private native void appNewIntent (Intent intent);

    @Override
    protected void onNewIntent (Intent intent)
    {
        super.onNewIntent(intent);
        setIntent(intent);

        appNewIntent (intent);
    }
}