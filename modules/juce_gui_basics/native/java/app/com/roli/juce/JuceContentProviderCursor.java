package com.roli.juce;

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
