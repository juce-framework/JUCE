/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

package com.juce;

import android.content.Context;
import android.view.*;
import android.graphics.*;

//==============================================================================
public class ComponentPeerView extends View
{
    public ComponentPeerView (Context context)
    {
        super (context);
    }

    //==============================================================================
    private native void handlePaint (Canvas canvas);

    @Override
    public void draw (Canvas canvas)
    {
        handlePaint (canvas);
    }

    //==============================================================================
    private native void handleMouseDown (float x, float y, long time);
    private native void handleMouseDrag (float x, float y, long time);
    private native void handleMouseUp (float x, float y, long time);

    @Override
    public boolean onTouchEvent (MotionEvent event)
    {
        switch (event.getAction())
        {
            case MotionEvent.ACTION_DOWN:  handleMouseDown (event.getX(), event.getY(), event.getEventTime()); return true;
            case MotionEvent.ACTION_MOVE:  handleMouseDrag (event.getX(), event.getY(), event.getEventTime()); return true;
            case MotionEvent.ACTION_CANCEL:
            case MotionEvent.ACTION_UP:    handleMouseUp (event.getX(), event.getY(), event.getEventTime()); return true;
            default: break;
        }

        return false;
    }

    //==============================================================================
    @Override
    protected void onSizeChanged (int w, int h, int oldw, int oldh)
    {
    }

    @Override
    protected void onLayout (boolean changed, int left, int top, int right, int bottom)
    {
    }

    public void setViewName (String newName)
    {
    }

    public boolean isVisible()
    {
        return true;
    }

    public void setVisible (boolean b)
    {
    }
}
