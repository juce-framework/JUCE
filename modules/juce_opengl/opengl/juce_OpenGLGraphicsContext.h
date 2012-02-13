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

#ifndef __JUCE_OPENGLGRAPHICSCONTEXT_JUCEHEADER__
#define __JUCE_OPENGLGRAPHICSCONTEXT_JUCEHEADER__


//==============================================================================
/** A LowLevelGraphicsContext for rendering into an OpenGL framebuffer or window.
*/
class JUCE_API  OpenGLGraphicsContext   : public LowLevelGraphicsContext
{
public:
    explicit OpenGLGraphicsContext (OpenGLComponent& target);
    OpenGLGraphicsContext (OpenGLContext& context, OpenGLFrameBuffer& target);
    OpenGLGraphicsContext (OpenGLContext& context, unsigned int frameBufferID, int width, int height);
    ~OpenGLGraphicsContext();

    bool isVectorDevice() const;
    void setOrigin (int x, int y);
    void addTransform (const AffineTransform&);
    float getScaleFactor();
    bool clipToRectangle (const Rectangle<int>&);
    bool clipToRectangleList (const RectangleList&);
    void excludeClipRectangle (const Rectangle<int>&);
    void clipToPath (const Path& path, const AffineTransform&);
    void clipToImageAlpha (const Image& sourceImage, const AffineTransform&);
    bool clipRegionIntersects (const Rectangle<int>&);
    Rectangle<int> getClipBounds() const;
    bool isClipEmpty() const;

    void saveState();
    void restoreState();

    void beginTransparencyLayer (float opacity);
    void endTransparencyLayer();

    void setFill (const FillType& fillType);
    void setOpacity (float newOpacity);
    void setInterpolationQuality (Graphics::ResamplingQuality);

    void fillRect (const Rectangle<int>& r, bool replaceExistingContents);
    void fillPath (const Path& path, const AffineTransform& transform);
    void drawImage (const Image& sourceImage, const AffineTransform& transform);
    void drawLine (const Line <float>& line);
    void drawVerticalLine (int x, float top, float bottom);
    void drawHorizontalLine (int y, float left, float right);

    void setFont (const Font&);
    const Font& getFont();

    void drawGlyph (int glyphNumber, const AffineTransform&);

   #ifndef DOXYGEN
    class SavedState;
    class GLState;
   #endif

private:
    ScopedPointer<GLState> glState;
    RenderingHelpers::SavedStateStack<SavedState> stack;
};

#endif   // __JUCE_OPENGLGRAPHICSCONTEXT_JUCEHEADER__
