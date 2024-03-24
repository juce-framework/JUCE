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

import android.content.ContentProvider;
import android.content.ContentValues;
import android.content.res.AssetFileDescriptor;
import android.content.res.Resources;
import android.database.Cursor;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.FileObserver;
import android.os.ParcelFileDescriptor;

import java.lang.String;

public final class JuceSharingContentProvider extends ContentProvider
{
    private Object lock = new Object ();

    private native Cursor contentSharerQuery (Uri uri, String[] projection);
    private native AssetFileDescriptor contentSharerOpenFile (Uri uri, String mode);
    private native String[] contentSharerGetStreamTypes (Uri uri, String mimeTypeFilter);

    @Override
    public boolean onCreate ()
    {
        return true;
    }

    @Override
    public Cursor query (Uri url, String[] projection, String selection,
                         String[] selectionArgs, String sortOrder)
    {
        synchronized (lock)
        {
            return contentSharerQuery (url, projection);
        }
    }

    @Override
    public Uri insert (Uri uri, ContentValues values)
    {
        return null;
    }

    @Override
    public int update (Uri uri, ContentValues values, String selection,
                       String[] selectionArgs)
    {
        return 0;
    }

    @Override
    public int delete (Uri uri, String selection, String[] selectionArgs)
    {
        return 0;
    }

    @Override
    public String getType (Uri uri)
    {
        return null;
    }

    @Override
    public AssetFileDescriptor openAssetFile (Uri uri, String mode)
    {
        synchronized (lock)
        {
            return contentSharerOpenFile (uri, mode);
        }
    }

    @Override
    public ParcelFileDescriptor openFile (Uri uri, String mode)
    {
        synchronized (lock)
        {
            AssetFileDescriptor result = contentSharerOpenFile (uri, mode);

            if (result != null)
                return result.getParcelFileDescriptor ();

            return null;
        }
    }

    public String[] getStreamTypes (Uri uri, String mimeTypeFilter)
    {
        synchronized (lock)
        {
            return contentSharerGetStreamTypes (uri, mimeTypeFilter);
        }
    }
}
