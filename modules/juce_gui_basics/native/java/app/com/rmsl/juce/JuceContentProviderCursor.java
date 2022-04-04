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

import android.database.Cursor;
import android.database.MatrixCursor;

import java.lang.String;

public final class JuceContentProviderCursor extends MatrixCursor
{
    public JuceContentProviderCursor (long hostToUse, String[] columnNames)
    {
        super (columnNames);

        host = hostToUse;
    }

    @Override
    public void close ()
    {
        super.close ();

        contentSharerCursorClosed (host);
    }

    private native void contentSharerCursorClosed (long host);

    private long host;
}
