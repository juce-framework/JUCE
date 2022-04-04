/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

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
