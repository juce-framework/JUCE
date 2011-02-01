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
        : canvas (canvas_)
    {
    }

    ~AndroidLowLevelGraphicsContext()
    {
    }

    bool isVectorDevice() const { return false; }

    //==============================================================================
    void setOrigin (int x, int y)
    {
    }

    void addTransform (const AffineTransform& transform)
    {
    }

    float getScaleFactor()
    {
        return 1.0f;
    }

    bool clipToRectangle (const Rectangle<int>& r)
    {
        return true;
    }

    bool clipToRectangleList (const RectangleList& clipRegion)
    {
        return true;
    }

    void excludeClipRectangle (const Rectangle<int>& r)
    {
    }

    void clipToPath (const Path& path, const AffineTransform& transform)
    {
    }

    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform)
    {
    }

    bool clipRegionIntersects (const Rectangle<int>& r)
    {
        return true;
    }

    const Rectangle<int> getClipBounds() const
    {
        return Rectangle<int> (0, 0, 1000, 1000);
    }

    bool isClipEmpty() const
    {
        return false;
    }

    void saveState()
    {
    }

    void restoreState()
    {
    }

    void beginTransparencyLayer (float opacity)
    {
    }

    void endTransparencyLayer()
    {
    }

    //==============================================================================
    void setFill (const FillType& fillType)
    {
        currentPaint = android.env->NewObject (android.paintClass, android.paintClassConstructor);
        currentPaint.callVoidMethod (android.setColor, fillType.colour.getARGB());
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
                               currentPaint.get());
    }

    void fillPath (const Path& path, const AffineTransform& transform)
    {
    }

    void drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles)
    {
    }

    void drawLine (const Line <float>& line)
    {
    }

    void drawVerticalLine (int x, float top, float bottom)
    {
    }

    void drawHorizontalLine (int y, float left, float right)
    {
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

private:
    GlobalRef canvas, currentPaint;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidLowLevelGraphicsContext);
};


#endif
