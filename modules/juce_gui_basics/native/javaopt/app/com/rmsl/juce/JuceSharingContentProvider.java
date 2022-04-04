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
