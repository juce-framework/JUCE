/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

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

#ifndef __JUCE_MAC_COREGRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_MAC_COREGRAPHICSCONTEXT_JUCEHEADER__

//==============================================================================
class CoreGraphicsContext   : public LowLevelGraphicsContext
{
public:
    CoreGraphicsContext (CGContextRef context, const float flipHeight, const float targetScale);
    ~CoreGraphicsContext();

    //==============================================================================
    bool isVectorDevice() const         { return false; }

    void setOrigin (int x, int y);
    void addTransform (const AffineTransform& transform);
    float getScaleFactor();
    float getTargetDeviceScaleFactor()  { return targetScale; }
    bool clipToRectangle (const Rectangle<int>& r);
    bool clipToRectangleList (const RectangleList& clipRegion);
    void excludeClipRectangle (const Rectangle<int>& r);
    void clipToPath (const Path& path, const AffineTransform& transform);
    void clipToImageAlpha (const Image& sourceImage, const AffineTransform& transform);
    bool clipRegionIntersects (const Rectangle<int>& r);
    Rectangle<int> getClipBounds() const;
    bool isClipEmpty() const;

    //==============================================================================
    void saveState();
    void restoreState();
    void beginTransparencyLayer (float opacity);
    void endTransparencyLayer();

    //==============================================================================
    void setFill (const FillType& fillType);
    void setOpacity (float newOpacity);
    void setInterpolationQuality (Graphics::ResamplingQuality quality);

    //==============================================================================
    void fillRect (const Rectangle<int>& r, const bool replaceExistingContents);
    void fillCGRect (const CGRect& cgRect, const bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform);
    void drawImage (const Image& sourceImage, const AffineTransform& transform);

    //==============================================================================
    void drawLine (const Line<float>& line);
    void drawVerticalLine (const int x, float top, float bottom);
    void drawHorizontalLine (const int y, float left, float right);
    void setFont (const Font& newFont);
    const Font& getFont();
    void drawGlyph (int glyphNumber, const AffineTransform& transform);
    bool drawTextLayout (const AttributedString& text, const Rectangle<float>&);

private:
    CGContextRef context;
    const CGFloat flipHeight;
    float targetScale;
    CGColorSpaceRef rgbColourSpace, greyColourSpace;
    CGFunctionCallbacks gradientCallbacks;
    mutable Rectangle<int> lastClipRect;
    mutable bool lastClipRectIsValid;

    struct SavedState
    {
        SavedState();
        SavedState (const SavedState& other);
        ~SavedState();

        void setFill (const FillType& newFill);
        CGShadingRef getShading (CoreGraphicsContext& owner);

        static void gradientCallback (void* info, const CGFloat* inData, CGFloat* outData);

        FillType fillType;
        Font font;
        CGFontRef fontRef;
        CGAffineTransform fontTransform;

    private:
        CGShadingRef shading;
        HeapBlock <PixelARGB> gradientLookupTable;
        int numGradientLookupEntries;
    };

    ScopedPointer <SavedState> state;
    OwnedArray <SavedState> stateStack;

    void drawGradient();
    void createPath (const Path& path) const;
    void createPath (const Path& path, const AffineTransform& transform) const;
    void flip() const;
    void applyTransform (const AffineTransform& transform) const;
    void drawImage (const Image& sourceImage, const AffineTransform& transform, bool fillEntireClipAsTiles);
    bool clipToRectangleListWithoutTest (const RectangleList&);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CoreGraphicsContext)
};

#endif   // __JUCE_MAC_COREGRAPHICSCONTEXT_JUCEHEADER__
