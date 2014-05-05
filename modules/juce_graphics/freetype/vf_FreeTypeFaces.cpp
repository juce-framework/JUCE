/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

//------------------------------------------------------------------------------
/*
 * Obviously the first thing people are going to want is to be able to get
 * hinted output from the currently installed system fonts instead of a
 * font embedded in the application's static variables but that's not something
 * I need. But here's a pointer to locating the font files on windows:
 *
 */
//http://www.codeproject.com/script/Articles/ViewDownloads.aspx?aid=4190&zep=XFont.cpp&rzp=%2fKB%2fGDI%2fxfont%2f%2fxfont_demo.zip
//

// This allows you to use FreeType to render the bitmaps instead of Juce
// and in theory, could produce better results but in my tests, its exactly
// the same output. Which is a good thing :-)
//
// In order to enable this you will need a patch to Juce
//#define TYPEFACE_BITMAP_RENDERING

// anonymous namespace
namespace {

// avoided the Singleton because of order of initialization issues
class FreeTypeLibrary : public ReferenceCountedObject
{
private:
  static FreeTypeLibrary* s_instance;
  FT_Library m_ft;

public:
  typedef ReferenceCountedObjectPtr <FreeTypeLibrary> Ptr;

  FreeTypeLibrary ()
  {
    FT_Error result;

    result = FT_Init_FreeType (&m_ft);
  }

  ~FreeTypeLibrary ()
  {
    s_instance = 0;
    FT_Done_FreeType (m_ft);
  }

  FT_Library getLibrary ()
  {
    return m_ft;
  }

  static Ptr getInstance()
  {
    if (s_instance)
    {
      return s_instance;
    }
    else
    {
      s_instance = new FreeTypeLibrary;
      return s_instance;
    }
  }
};

FreeTypeLibrary* FreeTypeLibrary::s_instance = 0;

//------------------------------------------------------------------------------

// A non-hinted CustomTypeface that uses FreeType to open the font file
// and extract the outlines. It is considered a match for any font whose
// name and style also match, and for which the font height lies outside
// the range specified by minHintedHeight and maxHintedHeight.
class FreeTypeFace : public CustomTypeface
{
private:
  // this is used as a substitute PositionedGlyph
  // so we can let FreeType do the rendering. This
  // only works for software rendering.
#ifdef TYPEFACE_BITMAP_RENDERING
  class PositionedGlyphImage : public PositionedGlyph
  {
  private:
    ReferenceCountedObjectPtr<FreeTypeFace> m_hf;

  public:
    PositionedGlyphImage (FreeTypeFace* face,
                          float x_,
                          float y_,
                          float w_,
                          const Font& font_,
                          juce_wchar character_,
                          int glyph_)
      : PositionedGlyph (x_, y_, w_, font_, character_, glyph_)
      , m_hf (face)
    {
    }

    void draw (const Graphics& g) const
    {
      if (!m_hf->drawGlyph (g, x, y, w, font, character, glyph))
        PositionedGlyph::draw (g);
    }
  };
#endif

private:
  FreeTypeLibrary::Ptr m_ft;
  bool m_useFreeTypeRendering; // true to use FreeType to render the outlines
  float m_minHintedHeight;
  float m_maxHintedHeight;
  float m_scale;
  float m_kerningScale;
  int m_kerningMode;

protected:
  FT_Face m_face;
  int m_loadFlags;

public:
  FreeTypeFace (float minHintedHeight,
                float maxHintedHeight,
                bool useFreeTypeRendering,
                const void* fileData,
                int fileBytes)
    : m_face(0)
    , m_minHintedHeight (minHintedHeight)
    , m_maxHintedHeight (maxHintedHeight)
    , m_useFreeTypeRendering (useFreeTypeRendering)
    , m_kerningScale (1)
  {
    // This doesn't work for non hinted faces yet because of the size
    m_useFreeTypeRendering = false;

    m_ft = FreeTypeLibrary::getInstance();

    openMemoryFace (fileData, fileBytes);
  }

  ~FreeTypeFace()
  { 
    closeFace();
  }

  bool isSuitableForFont (const Font& font) const
  {
    // sometimes Juce, during initialization and in the
    // menubar and documentwindow drawing code, tries to
    // produce fonts with zero heights. We never want to hint these
    if (font.getHeight()<1)
      return true; // never hint these

    // don't use this for heights within
    // the range for which hinting is desired.
    return (font.getHeight() > m_maxHintedHeight ||
            font.getHeight() < m_minHintedHeight);
  }

  bool isHinted () const
  {
    return false;
  }

  float getHeightToPointsFactor() const
  {
    float factor;

    float scale = 1.f;
    // convert from font units to Juce normalized
    scale *= 1.f/m_face->units_per_EM;
    // fudge since Juce produces smaller paths than FreeType
    // when it uses the Win32 API to extract the curves (?)
    float boxHeight = float(m_face->bbox.yMax - m_face->bbox.yMin);
    float fudge = m_face->units_per_EM / boxHeight;
    // this small adjustment produces output identical to Juce under win32
    fudge *= 1.0059625f;

    factor = 1.f / fudge;

    return factor;
  }

#ifdef TYPEFACE_BITMAP_RENDERING
  PositionedGlyph* createPositionedGlyph (float x, float y, float w, const Font& font, juce_wchar character, int glyph)
  {
    if (m_useFreeTypeRendering)
      return new PositionedGlyphImage (this, x, y, w, font, character, glyph);
    else
      return Typeface::createPositionedGlyph (x, y, w, font, character, glyph);
  }
#endif

protected:
  explicit FreeTypeFace (bool useFreeTypeRendering)
    : m_face (0)
    , m_useFreeTypeRendering (useFreeTypeRendering)
    , m_kerningScale (1)
  {
    m_ft = FreeTypeLibrary::getInstance();
  }

  bool openMemoryFace (const void* fileData, int fileBytes )
  {
    bool wasOpened = false;

    closeFace();

    FT_Error error;

    error = FT_New_Memory_Face (m_ft->getLibrary(),
                                (FT_Byte*)fileData,
                                fileBytes,
                                0,
                                &m_face);
    if (!error)
    {
      error = FT_Select_Charmap (m_face, FT_ENCODING_UNICODE);
      if (error)
        error = FT_Set_Charmap (m_face, m_face->charmaps[0]);

      if (!error)
      {
        prepareFace ();

        updateCharacteristics();

        wasOpened = true;
      }
    }

    return wasOpened;
  }

  void closeFace ()
  {
    clear();
    if (m_face)
    {
      /*FT_Error error =*/ FT_Done_Face (m_face);
      m_face = 0;
    }
  }

  void setParameters (float scale,
                      int loadFlags,
                      float kerningScale,
                      FT_UInt kerningMode)
  {
    m_scale = scale;
    m_loadFlags = loadFlags;
    m_kerningScale = kerningScale;
    m_kerningMode = kerningMode;
  }

  // Juce has this function
  // bool convertOutlineToPath (Path& destShape, const FT_Outline* outline);

  bool convertOutlineToPath (Path& destShape, const FT_Outline* outline)
  {   
    typedef float value_type;

    FT_Vector v_last;
    FT_Vector v_control;
    FT_Vector v_start;
    value_type x1, y1, x2, y2, x3, y3;

    FT_Vector* point;
    FT_Vector* limit;
    char* tags;

    int n;         // index of contour in outline
    int first;     // index of first point in contour
    char tag;      // current point's state

    first = 0;

    for(n = 0; n < outline->n_contours; n++)
    {
      int last; // index of last point in contour

      last  = outline->contours[n];
      limit = outline->points + last;

      v_start = outline->points[first];
      v_last  = outline->points[last];

      v_control = v_start;

      point = outline->points + first;
      tags  = outline->tags  + first;
      tag   = FT_CURVE_TAG(tags[0]);

      // A contour cannot start with a cubic control point!
      if(tag == FT_CURVE_TAG_CUBIC)
        return false;

      // check first point to determine origin
      if( tag == FT_CURVE_TAG_CONIC)
      {
        // first point is conic control.  Yes, this happens.
        if(FT_CURVE_TAG(outline->tags[last]) == FT_CURVE_TAG_ON)
        {
          // start at last point if it is on the curve
          v_start = v_last;
          limit--;
        }
        else
        {
          // if both first and last points are conic,
          // start at their middle and record its position
          // for closure
          v_start.x = (v_start.x + v_last.x) / 2;
          v_start.y = (v_start.y + v_last.y) / 2;

          v_last = v_start;
        }
        point--;
        tags--;
      }

      x1 = value_type(v_start.x);
      y1 = value_type(v_start.y);
      destShape.startNewSubPath (x1, y1);

      while(point < limit)
      {
        point++;
        tags++;

        tag = FT_CURVE_TAG(tags[0]);
        switch(tag)
        {
        case FT_CURVE_TAG_ON:  // emit a single lineTo
          {
            x1 = value_type(point->x);
            y1 = value_type(point->y);
            destShape.lineTo(x1, y1);
            continue;
          }

        case FT_CURVE_TAG_CONIC:  // consume conic arcs
          {
            v_control.x = point->x;
            v_control.y = point->y;

Do_Conic:
            if(point < limit)
            {
              FT_Vector vec;
              FT_Vector v_middle;

              point++;
              tags++;
              tag = FT_CURVE_TAG(tags[0]);

              vec.x = point->x; vec.y = point->y;

              if(tag == FT_CURVE_TAG_ON)
              {
                x1 = value_type(v_control.x);
                y1 = value_type(v_control.y);
                x2 = value_type(vec.x);
                y2 = value_type(vec.y);
                destShape.quadraticTo (x1, y1, x2, y2);
                continue;
              }

              if(tag != FT_CURVE_TAG_CONIC)
                return false;

              v_middle.x = (v_control.x + vec.x) / 2;
              v_middle.y = (v_control.y + vec.y) / 2;

              x1 = value_type(v_control.x);
              y1 = value_type(v_control.y);
              x2 = value_type(v_middle.x);
              y2 = value_type(v_middle.y);

              destShape.quadraticTo (x1, y1, x2, y2);
              v_control = vec;
              goto Do_Conic;
            }

            x1 = value_type(v_control.x);
            y1 = value_type(v_control.y);
            x2 = value_type(v_start.x);
            y2 = value_type(v_start.y);

            destShape.quadraticTo (x1, y1, x2, y2);
            goto Close;
          }

        default:  // FT_CURVE_TAG_CUBIC
          {
            FT_Vector vec1, vec2;

            if(point + 1 > limit || FT_CURVE_TAG(tags[1]) != FT_CURVE_TAG_CUBIC)
              return false;

            vec1.x = point[0].x;  vec1.y = point[0].y;
            vec2.x = point[1].x;  vec2.y = point[1].y;

            point += 2;
            tags  += 2;

            if(point <= limit)
            {
              FT_Vector vec;

              vec.x = point->x; vec.y = point->y;

              x1 = value_type(vec1.x);
              y1 = value_type(vec1.y);
              x2 = value_type(vec2.x);
              y2 = value_type(vec2.y);
              x3 = value_type(vec.x);
              y3 = value_type(vec.y);

              destShape.cubicTo(x1, y1, x2, y2, x3, y3);

              continue;
            }

            x1 = value_type(vec1.x);
            y1 = value_type(vec1.y);
            x2 = value_type(vec2.x);
            y2 = value_type(vec2.y);
            x3 = value_type(v_start.x);
            y3 = value_type(v_start.y);
            destShape.cubicTo (x1, y1, x2, y2, x3, y3);

            goto Close;
          }
        }
      }

      destShape.closeSubPath();

Close:
      first = last + 1; 
    }

    return true;
  }

  virtual void prepareFace ()
  {
    // calculate outline scale factor
    float scale = 1.f;
    // convert from font units to Juce normalized
    scale *= 1.f/m_face->units_per_EM;
    // fudge since Juce produces smaller paths than FreeType
    // when it uses the Win32 API to extract the curves (?)
    float boxHeight = float(m_face->bbox.yMax - m_face->bbox.yMin);
    float fudge = m_face->units_per_EM / boxHeight;
    // this small adjustment produces output identical to Juce under win32
    fudge *= 1.0059625f;

    setParameters (scale * fudge,
                   FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE,
                   fudge/ float(m_face->ascender - m_face->descender),
                   FT_KERNING_UNSCALED);
  }

private:
  void updateCharacteristics()
  {
    String name = m_face->family_name;
    float ascent = float(m_face->bbox.yMax) / (m_face->bbox.yMax-m_face->bbox.yMin);
    bool isBold = (m_face->style_flags & FT_STYLE_FLAG_BOLD) != 0;
    bool isItalic = (m_face->style_flags & FT_STYLE_FLAG_ITALIC) != 0;
    
    // ??? what do I put here?
    juce_wchar defaultChar = 0;//255;

    setCharacteristics (name, ascent, isBold, isItalic, defaultChar);

    addKerningPairs ();
  }

  void addKerningPairs()
  {
    FT_Error error = 0;

    FT_UInt leftGlyphIndex;
    FT_ULong leftCharCode = FT_Get_First_Char (m_face, &leftGlyphIndex);
    while (!error && leftGlyphIndex)
    {
      addKerningPairsForGlyph (leftGlyphIndex, leftCharCode);

      leftCharCode = FT_Get_Next_Char (m_face, leftCharCode, &leftGlyphIndex);
    }
  }

  FT_Error addKerningPairsForGlyph (FT_UInt leftGlyphIndex, FT_ULong leftCharCode)
  {
    FT_Error error = 0;

    if ((m_face->face_flags & FT_FACE_FLAG_KERNING) != 0)
    {
      FT_UInt rightGlyphIndex;
      FT_ULong rightCharCode = FT_Get_First_Char (m_face, &rightGlyphIndex);
      while (!error && rightGlyphIndex)
      {
        FT_Vector kerning;
        error = FT_Get_Kerning (m_face,
                                leftGlyphIndex,
                                rightGlyphIndex,
                                m_kerningMode,
                                &kerning);
        if (!error)
        {
          if (kerning.x != 0)
          {
            float extraAmount = m_kerningScale * kerning.x;
            addKerningPair (leftCharCode, rightCharCode, extraAmount);
          }

          rightCharCode = FT_Get_Next_Char (m_face, rightCharCode, &rightGlyphIndex);
        }
      }
    }

    return error;
  }

  bool loadGlyphIfPossible (juce_wchar characterNeeded)
  {
    bool wasLoaded = false;
    
    FT_Error error = 0;
    
    FT_UInt glyph_index = FT_Get_Char_Index (m_face, characterNeeded);
    if (glyph_index == 0)
      error = -1;

    if (!error)
      error = FT_Load_Glyph (m_face, glyph_index, m_loadFlags);

    if (!error)
    {
      Path path;
      wasLoaded = convertOutlineToPath (path, &m_face->glyph->outline);
      if (wasLoaded)
      {
#if 0
        // This stuff doesn't work yet

        // I'm assuming that we never get called to load a char twice.
        jassert (m_face->glyph->generic.data == 0);
        m_face->glyph->generic.data = (void*)1;

        // Delay loading of kerning pairs since there could be a
        // lot of code points in a multilanguage-aware face
        addKerningPairsForGlyph (glyph_index, characterNeeded);
#endif

        float advance = float(m_face->glyph->metrics.horiAdvance);
        // convert to juce normalized units
        path.applyTransform (AffineTransform::scale(m_scale, -m_scale));
        advance *= m_scale;
        addGlyph (characterNeeded, path, advance);
      }
    }

    return wasLoaded;
  }

  //----------------------------------------------------------------------------

  class GlyphImage : public ImagePixelData
  {
  private:
    int m_pixelStride;
    int m_lineStride;
    uint8* m_imageData;

  public:
    GlyphImage (int width_,
                int height_,
                int lineStride,
                uint8* imageData)
    : ImagePixelData (Image::SingleChannel, width_, height_)
    {
      m_pixelStride = 1;
      m_lineStride = lineStride;
      m_imageData = imageData;
    }

    ~GlyphImage()
    {
    }

    LowLevelGraphicsContext* createLowLevelContext()
    {
      // this image is read-only
      jassertfalse;
      return 0;
    }

    ImagePixelData* clone()
    {
      ImagePixelData* dup = new GlyphImage (width, height, m_lineStride, m_imageData);
      //dup->userData = userData; /* unfortunate */
      return dup;
    }

    ImageType* createType () const
    {
      return new SoftwareImageType;
    }

    void initialiseBitmapData (Image::BitmapData& bitmapData, int x, int y, Image::BitmapData::ReadWriteMode mode)
    {
      bitmapData.data = m_imageData + y * m_lineStride + x * m_pixelStride;
      bitmapData.pixelFormat = Image::SingleChannel;
      bitmapData.lineStride = m_lineStride;
      bitmapData.pixelStride = m_pixelStride;
      bitmapData.width = width - x;
      bitmapData.height = height - y;
    }
  };

  bool drawGlyph (const Graphics& g,
                  float x,
                  float y,
                  float w,
                  const Font& font,
                  juce_wchar character,
                  int glyph)
  {
    bool couldDraw = false;
    LowLevelGraphicsContext& lg = g.getInternalContext();

    FT_Error error=0;
    
    if (lg.isVectorDevice())
      error = -1;

    if (!error)
      error = FT_Load_Char (m_face, character, m_loadFlags);

    if (!error)
      error = FT_Render_Glyph (m_face->glyph, FT_RENDER_MODE_NORMAL);

    if (!error)
    {
      int w = m_face->glyph->bitmap.width;
      int h = m_face->glyph->bitmap.rows;
      if (w>0 && h>0)
      {
#if 0
        uint8* srcRow = m_face->glyph->bitmap.buffer;

        Image im (Image::SingleChannel, w, h, false);
        {
          Image::BitmapData dest (im, 0, 0, w, h, true);

          for (int y = 0; y<h; y++ )
          {
            uint8* destRow = dest.getLinePointer (y);
            memcpy (destRow, srcRow, w);
            srcRow += m_face->glyph->bitmap.pitch;
          }
        }

#else
        GlyphImage* fim = new GlyphImage(w, h,
          m_face->glyph->bitmap.pitch, m_face->glyph->bitmap.buffer);
        Image im (fim);
#endif

#if 0
        lg.drawImage (im, AffineTransform::translation(
          int(x + m_face->glyph->bitmap_left+.5f),
          int(y - m_face->glyph->bitmap_top+.5f)), false);
#else
        g.drawImage (im,
          int(x + m_face->glyph->bitmap_left+.5f),
          int(y - m_face->glyph->bitmap_top+.5f),
          w, h, 0, 0, w, h, true);
#endif
      }
    }

    if (!error)
      couldDraw = true;

    return couldDraw;
  }
};

//------------------------------------------------------------------------------

class FreeTypeHintedFace : public FreeTypeFace
{
private:
  float m_fontHeight; // the original font height that Juce sees

public:
  FreeTypeHintedFace (float fontHeight,
                      bool useFreeTypeRendering,
                      const void* fileData,
                      int fileBytes)
    : FreeTypeFace (useFreeTypeRendering)
    , m_fontHeight (fontHeight)
  {
    openMemoryFace (fileData, fileBytes);
  }

  bool useTypefaceFor (const Font& font) const
  {
    return m_fontHeight == font.getHeight();
  }

  bool isHinted () const
  {
    return true;
  }

  bool isSuitableForFont (const Font& font) const
  {
    return font.getHeight () == m_fontHeight;
  }

protected:
  void prepareFace ()
  {
    // calculate a fudged font scale to make things match Juce
    float fontScale;
    fontScale = 0.854f; // empirical
    float adjustedHeight = m_fontHeight * fontScale;

    // calculate outline scale factor
    float scale = 1.f;
    // convert 26p6 screen pixels to fractional screen pixels
    scale *= 1.f / 64;
    // convert to normalized based on created height
    scale *= 1.f / adjustedHeight;
    // account for the discrepancy in the juce height and the created height
    scale *= adjustedHeight / m_fontHeight;

    FT_Int gasp = FT_Get_Gasp (m_face,
                               FT_UInt(m_fontHeight * m_face->units_per_EM));

    
    bool useBytecodeHints = (gasp & FT_GASP_DO_GRIDFIT) != 0;

    // Unfortunately, this doesn't work quite right yet.
    useBytecodeHints = false;

    int loadFlags = 0;

    loadFlags |= FT_LOAD_NO_BITMAP;

    if (!useBytecodeHints)
      loadFlags |= FT_LOAD_FORCE_AUTOHINT;

    //loadFlags |= FT_LOAD_TARGET_LIGHT;

    setParameters(scale, loadFlags,
                  scale, FT_KERNING_DEFAULT);

    FT_Set_Char_Size (m_face, 0, FT_F26Dot6 (adjustedHeight * 64.f + 0.5f), 0, 0);
  }
};

//------------------------------------------------------------------------------

class FreeTypeFacesImplementation : public DeletedAtShutdown
{
private:
  struct MemoryFace
  {
    int flags;
    String faceName;
    String actualName; // family + style
    float minHintedHeight;
    float maxHintedHeight;
    bool useFreeTypeRendering;
    const void* faceFileData;
    int faceFileBytes;
  };

  FreeTypeLibrary::Ptr m_ft;
  Array<MemoryFace> m_faces;

public:
  FreeTypeFacesImplementation()
    : m_ft (FreeTypeLibrary::getInstance())
  {
  }

  ~FreeTypeFacesImplementation()
  {
    clearSingletonInstance();
  }

  void addFaceFromMemory (float minHintedHeight,
                          float maxHintedHeight,
                          bool useFreeTypeRendering,
                          const void* faceFileData,
                          int faceFileBytes,
                          bool appendStyleToFaceName)
  {
    FT_Error error;
    
    FT_Face face;
    error = FT_New_Memory_Face (m_ft->getLibrary(),
                                (FT_Byte*)faceFileData,
                                faceFileBytes,
                                0,
                                &face);
    if (!error)
    {
      if (face->face_flags & FT_FACE_FLAG_SCALABLE)
      {
        MemoryFace mf;

        mf.flags = 0;
        if (face->style_flags & FT_STYLE_FLAG_BOLD)
          mf.flags |= Font::bold;
        if (face->style_flags & FT_STYLE_FLAG_ITALIC)
          mf.flags |= Font::italic;

        mf.actualName = face->family_name;
        mf.actualName << ' ' << face->style_name;

        if (appendStyleToFaceName)
          mf.faceName = mf.actualName;
        else
          mf.faceName = face->family_name;

        mf.minHintedHeight = minHintedHeight;
        mf.maxHintedHeight = maxHintedHeight;
        mf.useFreeTypeRendering = useFreeTypeRendering;
        mf.faceFileData = faceFileData;
        mf.faceFileBytes = faceFileBytes;

        String s ("Added FreeType family '");
        s << face->family_name
          << "' with style '"
          << face->style_name
          << "'";

        if (mf.flags == (Font::bold | Font::italic))
          s << " as bold+italic";
        else if (mf.flags == Font::bold)
          s << " as bold";
        else if (mf.flags == Font::italic)
          s << " as italic";
        else if (mf.flags != 0)
          s << " with flags=" << String(mf.flags);

        Logger::outputDebugString (s);

        FT_Done_Face (face);

        m_faces.add (mf);
      }
    }
  }

  Typeface::Ptr createTypefaceForFont (const Font& font)
  {
    Typeface::Ptr typeFace = 0;

    for (int i=0; i<m_faces.size(); i++)
    {
      MemoryFace mf (m_faces[i]);
      if (mf.faceName == font.getTypefaceName() &&
          mf.flags == (font.getStyleFlags() & (Font::bold | Font::italic)))
      {
        bool useHinting = (font.getHeight() >= mf.minHintedHeight) &&
                          (font.getHeight() <= mf.maxHintedHeight);

        if (useHinting)
          typeFace = new FreeTypeHintedFace(
            font.getHeight(),
            mf.useFreeTypeRendering,
            mf.faceFileData,
            mf.faceFileBytes);
        else
          typeFace = new FreeTypeFace(
            mf.minHintedHeight,
            mf.maxHintedHeight,
            mf.useFreeTypeRendering,
            mf.faceFileData,
            mf.faceFileBytes);

        String s ("Created FreeType face '");
        s << mf.actualName << "'";
        if (useHinting)
          s << " at hinted size " << String(useHinting ? font.getHeight() : 0., 2);
        const int flags = font.getStyleFlags() & (Font::bold | Font::italic);
        if (flags)
        {
          if (flags == (Font::bold | Font::italic))
            s << " as bold+italic";
          else if (flags == Font::bold)
            s << " as bold";
          else if (flags == Font::italic)
            s << " as italic";
          else if (flags != 0)
            s << " with flags=" << String(flags);
        }

        Logger::outputDebugString (s);
        break;
      }
    }

    return typeFace;
  }

  juce_DeclareSingleton (FreeTypeFacesImplementation, false);
};

juce_ImplementSingleton (FreeTypeFacesImplementation)

}

//------------------------------------------------------------------------------

void FreeTypeFaces::addFaceFromMemory (float minHintedHeight,
                                       float maxHintedHeight,
                                       bool useFreeTypeRendering,
                                       const void* faceFileData,
                                       int faceFileBytes,
                                       bool appendStyleToFaceName)
{
  return FreeTypeFacesImplementation::getInstance()->addFaceFromMemory(
                                                      minHintedHeight,
                                                      maxHintedHeight,
                                                      useFreeTypeRendering,
                                                      faceFileData,
                                                      faceFileBytes,
                                                      appendStyleToFaceName);
}

Typeface::Ptr FreeTypeFaces::createTypefaceForFont (const Font& font)
{
  return FreeTypeFacesImplementation::getInstance()->createTypefaceForFont (font);
}
