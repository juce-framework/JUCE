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

import android.app.DialogFragment;
import android.content.Intent;
import android.os.Bundle;

public class FragmentOverlay extends DialogFragment
{
    @Override
    public void onCreate (Bundle state)
    {
        super.onCreate (state);
        cppThis = getArguments ().getLong ("cppThis");

        if (cppThis != 0)
            onCreateNative (cppThis, state);
    }

    @Override
    public void onStart ()
    {
        super.onStart ();

        if (cppThis != 0)
            onStartNative (cppThis);
    }

    public void onRequestPermissionsResult (int requestCode,
                                            String[] permissions,
                                            int[] grantResults)
    {
        if (cppThis != 0)
            onRequestPermissionsResultNative (cppThis, requestCode,
                    permissions, grantResults);
    }

    @Override
    public void onActivityResult (int requestCode, int resultCode, Intent data)
    {
        if (cppThis != 0)
            onActivityResultNative (cppThis, requestCode, resultCode, data);
    }

    public void close ()
    {
        cppThis = 0;
        dismiss ();
    }

    //==============================================================================
    private long cppThis = 0;

    private native void onActivityResultNative (long myself, int requestCode, int resultCode, Intent data);
    private native void onCreateNative (long myself, Bundle state);
    private native void onStartNative (long myself);
    private native void onRequestPermissionsResultNative (long myself, int requestCode,
                                                          String[] permissions, int[] grantResults);
}
