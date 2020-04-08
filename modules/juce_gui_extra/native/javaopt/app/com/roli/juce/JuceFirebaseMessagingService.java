/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

package com.roli.juce;

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
