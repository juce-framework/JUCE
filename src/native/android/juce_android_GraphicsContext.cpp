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

// (This file gets included by juce_android_NativeCode.cpp, rather than being
// compiled on its own).
#if JUCE_INCLUDED_FILE


//==============================================================================
// TODO
class AndroidLowLevelGraphicsContext   : public LowLevelGraphicsContext
{
public:
    AndroidLowLevelGraphicsContext (const GlobalRef& canvas_)
        : canvas (canvas_),
          currentState (new SavedState()),
    {
        paintStack.add (new GlobalRef());
        setFill (Colours::black);
    }

    ~AndroidLowLevelGraphicsContext()
    {
    }

    bool isVectorDevice() const { return false; }

    //==============================================================================
    void setOrigin (int x, int y)
    {
        canvas.callVoidMethod (android.translate, (float) x, (float) y);
    }

    void addTransform (const AffineTransform& transform)
    {
        canvas.callVoidMethod (android.concat, createMatrix (canvas.getEnv(), transform));
    }

    float getScaleFactor()
    {
        return 1.0f;
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        return canvas.callBooleanMethod (android.clipRect, (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom());
    }

    bool clipToRectangleList (const RectangleList& clipRegion)
    {
        return canvas.callBooleanMethod (android.clipRegion, createRegion (canvas.getEnv(), clipRegion));
    }

    void excludeClipRectangle (const Rectangle<int>& r)
    {
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
        (void) canvas.callBooleanMethod (android.clipPath, createPath (canvas.getEnv(), path, transform));
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return getClipBounds().intersects (r);
    }

    const Rectangle<int> getClipBounds() const
    {
        jobject rect = canvas.callObjectMethod (android.getClipBounds2);

        const int left = canvas.getEnv()->GetIntField (rect, android.rectLeft);
        const int top = canvas.getEnv()->GetIntField (rect, android.rectTop);
        const int right = canvas.getEnv()->GetIntField (rect, android.rectRight);
        const int bottom = canvas.getEnv()->GetIntField (rect, android.rectBottom);

        return Rectangle<int> (left, top, right - left, bottom - top);
    }

    bool isClipEmpty() const
    {
        return ! canvas.callBooleanMethod (android.getClipBounds,
                                           canvas.getEnv()->NewObject (android.rectClass, android.rectConstructor, 0, 0, 0, 0));
    }

    //==============================================================================
    void setFill (const FillType& fillType)
    {
        currentState->setFillType (fillType);
    }

    void setOpacity (float newOpacity)
    {
    }

    void setInterpolationQuality (Graphics::ResamplingQuality quality)
    {
    }

    //==============================================================================
    void fillRect (const Rectangle<int>& r, bool replaceExistingContents)
    {
        canvas.callVoidMethod (android.drawRect,
                               (float) r.getX(), (float) r.getY(), (float) r.getRight(), (float) r.getBottom(),
                               getCurrentPaint().get());
    }

    void fillPath (const Path& path, const AffineTransform& transform)
    {
        canvas.callVoidMethod (android.drawPath, createPath (canvas.getEnv(), path, transform),
                               getCurrentPaint().get());
    }

    void drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles)
    {
    }

    void drawLine (const Line <float>& line)
    {
        canvas.callVoidMethod (android.drawLine, line.getStartX(), line.getStartY(),
                               line.getEndX(), line.getEndY(),
                               getCurrentPaint().get());
    }

    void drawVerticalLine (int x, float top, float bottom)
    {
        canvas.callVoidMethod (android.drawRect, (float) x, top, x + 1.0f, bottom, getCurrentPaint().get());
    }

    void drawHorizontalLine (int y, float left, float right)
    {
        canvas.callVoidMethod (android.drawRect, left, (float) y, right, y + 1.0f, getCurrentPaint().get());
    }

    void setFont (const Font& newFont)
    {
    }

    const Font getFont()
    {
        return Font();
    }

    void drawGlyph (int glyphNumber, const AffineTransform& transform)
    {
    }

    //==============================================================================
    void saveState()
    {
        (void) canvas.callIntMethod (android.save);
        stateStack.add (new SavedState (*currentState));

    }

    void restoreState()
    {
        canvas.callVoidMethod (android.restore);

        SavedState* const top = stateStack.getLast();

        if (top != 0)
        {
            currentState = top;
            stateStack.removeLast (1, false);
        }
        else
        {
            jassertfalse; // trying to pop with an empty stack!
        }
    }

    void beginTransparencyLayer (float opacity)
    {
    }

    void endTransparencyLayer()
    {
    }

    class SavedState
    {
    public:
        SavedState()
            : font (1.0f), needsUpdate (true)
        {
        }

        SavedState (const SavedState& other)
            : fillType (other.fillType), font (other.font), needsUpdate (true)
        {
        }

        void setFillType (const FillType& newType)
        {
            needsUpdate = true;
            fillType = newType;
        }

        jobject getPaint (JNIEnv* env)
        {
            if (needsUpdate)
            {
                if (paint.get() == 0)
                    paint = GlobalRef (env, env->NewObject (android.paintClass, android.paintClassConstructor));

                if (fillType.isColour())
                {
                    paint.callVoidMethod (android.setShader, (jobject) 0);
                    paint.callVoidMethod (android.setColor, colourToInt (fillType.colour));
                }
                else if (fillType.isGradient())
                {
                    const ColourGradient& g = fillType.gradient;
                    const Point<float> p1 (g.point1);
                    const Point<float> p2 (g.point2);

                    const int numColours = g.getNumColours();
                    jintArray coloursArray = env->NewIntArray (numColours);
                    jfloatArray positionsArray = env->NewFloatArray (numColours);

                    {
                        HeapBlock<int> colours (numColours);
                        HeapBlock<float> positions (numColours);

                        for (int i = 0; i < numColours; ++i)
                        {
                            colours[i] = g.getColour (i);
                            positions[i] = (float) g.getColourPosition(i);
                        }

                        env->SetIntArrayRegion (coloursArray, 0, numColours, colours.getData());
                        env->SetFloatArrayRegion (positionsArray, 0, numColours, positions.getData());
                    }

                    jobject tileMode = xxxx

                    jobject shader;
                    if (fillType.gradient->isRadial)
                    {
                        shader = env->NewObject (android.radialGradientClass,
                                                 android.radialGradientConstructor,
                                                 p1.getX(), p1.getY(),
                                                 p1.getDistanceFrom (p2),
                                                 coloursArray, positionsArray,
                                                 tileMode));
                    }
                    else
                    {
                        shader = env->NewObject (android.linearGradientClass,
                                                 android.linearGradientConstructor,
                                                 p1.getX(), p1.getY(), p2.getX(), p2.getY(),
                                                 coloursArray, positionsArray,
                                                 tileMode));
                    }

                    env->CallVoidMethod (shader, android.setLocalMatrix, createMatrix (fillType.transform));
                    paint.callVoidMethod (android.setShader, shader);
                }
                else
                {x
                }
            }

            return paint.get();
        }

    private:
        FillType fillType;
        Font font;
        GlobalRef paint;
        bool needsUpdate;
    };

private:
    GlobalRef canvas;

    ScopedPointer <SavedState> currentState;
    OwnedArray <SavedState> stateStack;

    GlobalRef& getCurrentPaint() throw()    { return *paintStack.getUnchecked (paintStack.size() - 1); }

    static jobject createPath (JNIEnv* env, const Path& path)
    {
        jobject p = env->NewObject (android.pathClass, android.pathClassConstructor);

        Path::Iterator i (path);

        while (i.next())
        {
            switch (i.elementType)
            {
                case Path::Iterator::startNewSubPath:  env->CallVoidMethod (p, android.moveTo, i.x1, i.y1); break;
                case Path::Iterator::lineTo:           env->CallVoidMethod (p, android.lineTo, i.x1, i.y1); break;
                case Path::Iterator::quadraticTo:      env->CallVoidMethod (p, android.quadTo, i.x1, i.y1, i.x2, i.y2); break;
                case Path::Iterator::cubicTo:          env->CallVoidMethod (p, android.cubicTo, i.x1, i.y1, i.x2, i.y2, i.x3, i.y3); break;
                case Path::Iterator::closePath:        env->CallVoidMethod (p, android.closePath); break;
                default:                               jassertfalse; break;
            }
        }

        return p;
    }

    static jobject createPath (JNIEnv* env, const Path& path, const AffineTransform& transform)
    {
        if (transform.isIdentity())
            return createPath (env, path);

        Path tempPath (path);
        tempPath.applyTransform (transform);
        return createPath (env, tempPath);
    }

    static jobject createMatrix (JNIEnv* env, const AffineTransform& t)
    {
        jobject m = env->NewObject (android.matrixClass, android.matrixClassConstructor);

        jfloat values[9] = { t.mat00, t.mat01, t.mat02,
                             t.mat10, t.mat11, t.mat12,
                             0.0f, 0.0f, 1.0f };

        jfloatArray javaArray = env->NewFloatArray (9);
        env->SetFloatArrayRegion (javaArray, 0, 9, values);

        env->CallVoidMethod (m, android.setValues, javaArray);
        env->DeleteLocalRef (javaArray);

        return m;
    }

    static jobject createRect (JNIEnv* env, const Rectangle<int>& r)
    {
        return env->NewObject (android.rectClass, android.rectConstructor,
                               r.getX(), r.getY(), r.getRight(), r.getBottom());
    }

    static jobject createRegion (JNIEnv* env, const RectangleList& list)
    {
        jobject region = env->NewObject (android.regionClass, android.regionConstructor);

        const int numRects = list.getNumRectangles();

        for (int i = 0; i < numRects; ++i)
            env->CallVoidMethod (region, android.regionUnion, createRect (env, list.getRectangle(i)));

        return region;
    }

    static int colourToInt (const Colour& col) throw()
    {
        return col.getARGB();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidLowLevelGraphicsContext);
};


#endif
