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

import com.google.firebase.messaging.*;

public final class JuceFirebaseMessagingService extends FirebaseMessagingService
{
    private native void firebaseRemoteMessageReceived (RemoteMessage message);
    private native void firebaseRemoteMessagesDeleted();
    private native void firebaseRemoteMessageSent (String messageId);
    private native void firebaseRemoteMessageSendError (String messageId, String error);

    @Override
    public void onMessageReceived (RemoteMessage message)
    {
        firebaseRemoteMessageReceived (message);
    }

    @Override
    public void onDeletedMessages()
    {
        firebaseRemoteMessagesDeleted();
    }

    @Override
    public void onMessageSent (String messageId)
    {
        firebaseRemoteMessageSent (messageId);
    }

    @Override
    public void onSendError (String messageId, Exception e)
    {
        firebaseRemoteMessageSendError (messageId, e.toString());
    }
}
