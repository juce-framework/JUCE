package com.juce;

import android.content.Context;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.ViewGroup;

/**
 * Created by adamwilson on 14/08/2016.
 */
    public class JuceViewHolder extends ViewGroup
    {
        public JuceViewHolder(Context context)
        {
            super (context);
            juceBridge = JuceBridge.getInstance();
            setDescendantFocusability (ViewGroup.FOCUS_AFTER_DESCENDANTS);
            setFocusable (false);
        }

        protected final void onLayout (boolean changed, int left, int top, int right, int bottom)
        {
            Log.d("JuceViewHolder", "onLayout: "+left+","+top+","+right+","+bottom);
            juceBridge.setScreenSize (getWidth(), getHeight(), getDPI());
            Log.d("JuceViewHolder", "screenSize: "+getWidth()+","+getHeight());

            // TODO: Move callApplauncher to JuceBridge constructor?
            if (isFirstResize && !juceBridge.appInitialised())
            {
                isFirstResize = false;
                juceBridge.callAppLauncher();
            }
        }

        private final int getDPI()
        {
            DisplayMetrics metrics = getContext().getResources().getDisplayMetrics();
            return metrics.densityDpi;
        }

        private boolean isFirstResize = true;
        private JuceBridge juceBridge;
    }

