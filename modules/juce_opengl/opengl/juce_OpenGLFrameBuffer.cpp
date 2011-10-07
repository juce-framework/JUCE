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

BEGIN_JUCE_NAMESPACE

#if JUCE_WINDOWS
enum
{
    GL_FRAMEBUFFER_EXT = 0x8D40,
    GL_RENDERBUFFER_EXT = 0x8D41,
    GL_COLOR_ATTACHMENT0_EXT = 0x8CE0,
    GL_DEPTH_ATTACHMENT_EXT = 0x8D00,
    GL_STENCIL_ATTACHMENT_EXT = 0x8D20,
    GL_FRAMEBUFFER_COMPLETE_EXT = 0x8CD5,
    GL_DEPTH24_STENCIL8_EXT = 0x88F0,
    GL_RENDERBUFFER_DEPTH_SIZE_EXT = 0x8D54
};
#endif

#if JUCE_WINDOWS || JUCE_LINUX

#define FRAMEBUFFER_FUNCTION_LIST(USE_FUNCTION) \
    USE_FUNCTION (glIsRenderbufferEXT,                          GLboolean, (GLuint renderbuffer))\
    USE_FUNCTION (glBindRenderbufferEXT,                        void, (GLenum target, GLuint renderbuffer))\
    USE_FUNCTION (glDeleteRenderbuffersEXT,                     void, (GLsizei n, const GLuint *renderbuffers))\
    USE_FUNCTION (glGenRenderbuffersEXT,                        void, (GLsizei n, GLuint *renderbuffers))\
    USE_FUNCTION (glRenderbufferStorageEXT,                     void, (GLenum target, GLenum internalformat, GLsizei width, GLsizei height))\
    USE_FUNCTION (glGetRenderbufferParameterivEXT,              void, (GLenum target, GLenum pname, GLint* params))\
    USE_FUNCTION (glIsFramebufferEXT,                           GLboolean, (GLuint framebuffer))\
    USE_FUNCTION (glBindFramebufferEXT,                         void, (GLenum target, GLuint framebuffer))\
    USE_FUNCTION (glDeleteFramebuffersEXT,                      void, (GLsizei n, const GLuint *framebuffers))\
    USE_FUNCTION (glGenFramebuffersEXT,                         void, (GLsizei n, GLuint *framebuffers))\
    USE_FUNCTION (glCheckFramebufferStatusEXT,                  GLenum, (GLenum target))\
    USE_FUNCTION (glFramebufferTexture1DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
    USE_FUNCTION (glFramebufferTexture2DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level))\
    USE_FUNCTION (glFramebufferTexture3DEXT,                    void, (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint zoffset))\
    USE_FUNCTION (glFramebufferRenderbufferEXT,                 void, (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer))\
    USE_FUNCTION (glGetFramebufferAttachmentParameterivEXT,     void, (GLenum target, GLenum attachment, GLenum pname, GLint *params))\
    USE_FUNCTION (glGenerateMipmapEXT,                          void, (GLenum target))\

#if JUCE_WINDOWS
 #define APICALLTYPE __stdcall
#else
 #define APICALLTYPE
#endif

#define DECLARE_FUNCTION(name, returnType, params) \
  typedef returnType (APICALLTYPE * type_ ## name) params; static type_ ## name name;
FRAMEBUFFER_FUNCTION_LIST (DECLARE_FUNCTION)
#undef DECLARE_FUNCTION

static bool framebufferFunctionsInitialised = false;

static void initialiseFrameBufferFunctions()
{
    if (! framebufferFunctionsInitialised)
    {
        framebufferFunctionsInitialised = true;

       #if JUCE_LINUX
        #define JUCE_LOOKUP_FUNCTION(name) glXGetProcAddress ((const GLubyte*) name)
       #else
        #define JUCE_LOOKUP_FUNCTION(name) wglGetProcAddress (name)
       #endif

       #define FIND_FUNCTION(name, returnType, params) name = (type_ ## name) JUCE_LOOKUP_FUNCTION (#name);
        FRAMEBUFFER_FUNCTION_LIST (FIND_FUNCTION)
       #undef FIND_FUNCTION
       #undef JUCE_LOOKUP_FUNCTION
    }
}

#undef FRAMEBUFFER_FUNCTION_LIST

//==============================================================================
#elif JUCE_OPENGL_ES

#define glIsRenderbufferEXT                         glIsRenderbufferOES
#define glBindRenderbufferEXT                       glBindRenderbufferOES
#define glDeleteRenderbuffersEXT                    glDeleteRenderbuffersOES
#define glGenRenderbuffersEXT                       glGenRenderbuffersOES
#define glRenderbufferStorageEXT                    glRenderbufferStorageOES
#define glGetRenderbufferParameterivEXT             glGetRenderbufferParameterivOES
#define glIsFramebufferEXT                          glIsFramebufferOES
#define glBindFramebufferEXT                        glBindFramebufferOES
#define glDeleteFramebuffersEXT                     glDeleteFramebuffersOES
#define glGenFramebuffersEXT                        glGenFramebuffersOES
#define glCheckFramebufferStatusEXT                 glCheckFramebufferStatusOES
#define glFramebufferTexture1DEXT                   glFramebufferTexture1DOES
#define glFramebufferTexture2DEXT                   glFramebufferTexture2DOES
#define glFramebufferTexture3DEXT                   glFramebufferTexture3DOES
#define glFramebufferRenderbufferEXT                glFramebufferRenderbufferOES
#define glGetFramebufferAttachmentParameterivEXT    glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapEXT                         glGenerateMipmapOES

#define GL_FRAMEBUFFER_EXT                          GL_FRAMEBUFFER_OES
#define GL_RGBA8                                    GL_RGBA
#define GL_COLOR_ATTACHMENT0_EXT                    GL_COLOR_ATTACHMENT0_OES
#define GL_RENDERBUFFER_EXT                         GL_RENDERBUFFER_OES
#define GL_DEPTH24_STENCIL8_EXT                     GL_DEPTH24_STENCIL8_OES
#define GL_RENDERBUFFER_DEPTH_SIZE_EXT              GL_RENDERBUFFER_DEPTH_SIZE_OES
#define GL_DEPTH_ATTACHMENT_EXT                     GL_DEPTH_ATTACHMENT_OES
#define GL_STENCIL_ATTACHMENT_EXT                   GL_STENCIL_ATTACHMENT_OES
#define GL_FRAMEBUFFER_COMPLETE_EXT                 GL_FRAMEBUFFER_COMPLETE_OES
#define GL_FRAMEBUFFER_UNSUPPORTED_EXT              GL_FRAMEBUFFER_UNSUPPORTED_OES
#define GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT    GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_OES

#endif

//==============================================================================
class OpenGLFrameBuffer::Pimpl
{
public:
    Pimpl (const int width_, const int height_,
           const bool wantsDepthBuffer, const bool wantsStencilBuffer,
           const GLenum textureType = GL_TEXTURE_2D)
        : width (width_),
          height (height_),
          textureID (0),
          frameBufferHandle (0),
          depthOrStencilBuffer (0),
          hasDepthBuffer (false),
          hasStencilBuffer (false),
          ok (false)
    {
       #if JUCE_WINDOWS || JUCE_LINUX
        initialiseFrameBufferFunctions();

        if (glGenFramebuffersEXT == nullptr)
            return;
       #endif

        OpenGLHelpers::resetErrorState();

        glGenFramebuffersEXT (1, &frameBufferHandle);
        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, frameBufferHandle);

        glGenTextures (1, &textureID);
        glBindTexture (textureType, textureID);
        glTexImage2D (textureType, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glTexParameterf (textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameterf (textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterf (textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterf (textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glFramebufferTexture2DEXT (GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, textureType, textureID, 0);

        if (wantsDepthBuffer || wantsStencilBuffer)
        {
            glGenRenderbuffersEXT (1, &depthOrStencilBuffer);
            glBindRenderbufferEXT (GL_RENDERBUFFER_EXT, depthOrStencilBuffer);
            jassert (glIsRenderbufferEXT (depthOrStencilBuffer));

            if (wantsDepthBuffer && wantsStencilBuffer)
            {
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH24_STENCIL8_EXT, width, height);

                GLint params = 0;
                glGetRenderbufferParameterivEXT (GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &params);

                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);
                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_STENCIL_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);

                hasDepthBuffer = true;
                hasStencilBuffer = true;
            }
            else
            {
               #if JUCE_OPENGL_ES
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT16, width, height);
               #else
                glRenderbufferStorageEXT (GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height);
               #endif

                GLint params = 0;
                glGetRenderbufferParameterivEXT (GL_RENDERBUFFER_EXT, GL_RENDERBUFFER_DEPTH_SIZE_EXT, &params);
                glFramebufferRenderbufferEXT (GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depthOrStencilBuffer);

                hasDepthBuffer = true;
            }
        }

        ok = checkStatus();

        glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, 0);
    }

    ~Pimpl()
    {
        if (textureID != 0)
            glDeleteTextures (1, &textureID);

        if (depthOrStencilBuffer != 0)
            glDeleteRenderbuffersEXT (1, &depthOrStencilBuffer);

        if (frameBufferHandle != 0)
            glDeleteFramebuffersEXT (1, &frameBufferHandle);
    }

    bool bind()    { return bind (frameBufferHandle); }
    bool unbind()  { return bind (0); }

    const int width, height;
    GLuint textureID, frameBufferHandle, depthOrStencilBuffer;
    bool hasDepthBuffer, hasStencilBuffer, ok;

private:
    bool bind (GLuint buffer)
    {
        if (ok)
        {
            glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, buffer);
            ok = checkStatus();
        }

        return ok;
    }

    static bool checkStatus() noexcept
    {
        const GLenum status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);

        return status == GL_NO_ERROR
            || status == GL_FRAMEBUFFER_COMPLETE_EXT;
    }

    JUCE_DECLARE_NON_COPYABLE (Pimpl);
};


//==============================================================================
OpenGLFrameBuffer::OpenGLFrameBuffer() {}
OpenGLFrameBuffer::~OpenGLFrameBuffer() {}

bool OpenGLFrameBuffer::initialise (int width, int height)
{
    release();
    pimpl = new Pimpl (width, height, true, false);

    if (! pimpl->ok)
        pimpl = nullptr;

    return pimpl != nullptr;
}

void OpenGLFrameBuffer::release()
{
    pimpl = nullptr;
}

int OpenGLFrameBuffer::getWidth() const noexcept            { return pimpl != nullptr ? pimpl->width : 0; }
int OpenGLFrameBuffer::getHeight() const noexcept           { return pimpl != nullptr ? pimpl->height : 0; }
GLuint OpenGLFrameBuffer::getTextureID() const noexcept     { return pimpl != nullptr ? pimpl->textureID : 0; }

bool OpenGLFrameBuffer::makeCurrentTarget()
{
    return pimpl != nullptr && pimpl->bind();
}

void OpenGLFrameBuffer::releaseCurrentTarget()
{
    if (pimpl != nullptr)
        pimpl->unbind();
}

void OpenGLFrameBuffer::clear (const Colour& colour)
{
    if (makeCurrentTarget())
    {
        OpenGLHelpers::clear (colour);
        releaseCurrentTarget();
    }
}

void OpenGLFrameBuffer::draw2D (float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                const Colour& colour) const
{
    if (pimpl != nullptr)
    {
        glBindTexture (GL_TEXTURE_2D, pimpl->textureID);
        OpenGLHelpers::drawQuad2D (x1, y1, x2, y2, x3, y3, x4, y4, colour);
        glBindTexture (GL_TEXTURE_2D, 0);
    }
}

void OpenGLFrameBuffer::draw3D (float x1, float y1, float z1,
                                float x2, float y2, float z2,
                                float x3, float y3, float z3,
                                float x4, float y4, float z4,
                                const Colour& colour) const
{
    if (pimpl != nullptr)
    {
        glBindTexture (GL_TEXTURE_2D, pimpl->textureID);
        OpenGLHelpers::drawQuad3D (x1, y1, z1, x2, y2, z2, x3, y3, z3, x4, y4, z4, colour);
        glBindTexture (GL_TEXTURE_2D, 0);
    }
}

//==============================================================================
// This breaks down a path into a series of horizontal strips of trapezoids..
class TrapezoidedPath
{
public:
    TrapezoidedPath (const Path& p)
        : firstSlice (nullptr),
          windingMask (p.isUsingNonZeroWinding() ? -1 : 1)
    {
        PathFlatteningIterator iter (p);

        while (iter.next())
            addLine (floatToInt (iter.x1), floatToInt (iter.y1),
                     floatToInt (iter.x2), floatToInt (iter.y2));
    }

    ~TrapezoidedPath()
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr;)
        {
            const ScopedPointer<HorizontalSlice> deleter (s);
            s = s->next;
        }
    }

    template <class Consumer>
    void iterate (Consumer& consumer) const
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr; s = s->next)
            s->iterate (consumer, windingMask);
    }

private:
    void addLine (int x1, int y1, int x2, int y2)
    {
        int winding = 1;

        if (y2 < y1)
        {
            std::swap (x1, x2);
            std::swap (y1, y2);
            winding = -1;
        }

        HorizontalSlice* last = nullptr;
        HorizontalSlice* s = firstSlice;

        while (y2 > y1)
        {
            if (s == nullptr)
            {
                insert (last, new HorizontalSlice (nullptr, x1, y1, x2, y2, winding));
                break;
            }

            if (s->y2 > y1)
            {
                if (y1 < s->y1)
                {
                    if (y2 <= s->y1)
                    {
                        insert (last, new HorizontalSlice (s, x1, y1, x2, y2, winding));
                        break;
                    }
                    else
                    {
                        const int newX = x1 + (s->y1 - y1) * (x2 - x1) / (y2 - y1);
                        HorizontalSlice* const newSlice = new HorizontalSlice (s, x1, y1, newX, s->y1, winding);
                        insert (last, newSlice);
                        last = newSlice;
                        x1 = newX;
                        y1 = s->y1;
                        continue;
                    }
                }
                else if (y1 > s->y1)
                {
                    s->split (y1);
                    s = s->next;
                    jassert (s != nullptr);
                }

                jassert (y1 == s->y1);

                if (y2 > s->y2)
                {
                    const int newY = s->y2;
                    const int newX = x1 + (newY - y1) * (x2 - x1) / (y2 - y1);
                    s->addLine (x1, newX, winding);
                    x1 = newX;
                    y1 = newY;
                }
                else
                {
                    if (y2 < s->y2)
                        s->split (y2);

                    jassert (y2 == s->y2);
                    s->addLine (x1, x2, winding);
                    break;
                }
            }

            last = s;
            s = s->next;
        }
    }

    struct HorizontalSlice
    {
        HorizontalSlice (const HorizontalSlice& other, HorizontalSlice* const next_, int y1_, int y2_)
            : next (next_), y1 (y1_), y2 (y2_), segments (other.segments)
        {
        }

        HorizontalSlice (HorizontalSlice* const next_, int x1, int y1_, int x2, int y2_, int winding)
            : next (next_), y1 (y1_), y2 (y2_)
        {
            jassert (next != this);
            jassert (y2 > y1);
            segments.ensureStorageAllocated (32);
            segments.add (LineSegment (x1, x2, winding));
        }

        void addLine (const int x1, const int x2, int winding)
        {
            const int dy = y2 - y1;

            for (int i = 0; i < segments.size(); ++i)
            {
                const LineSegment& l = segments.getReference (i);

                const int diff1 = l.x1 - x1;
                const int diff2 = l.x2 - x2;

                if ((diff1 < 0) == (diff2 > 0))
                {
                    const int dx1 = l.x2 - l.x1;
                    const int dx2 = x2 - x1;
                    const int dxDiff = dx2 - dx1;

                    if (dxDiff != 0)
                    {
                        const int intersectionY = (dy * diff1) / dxDiff;

                        if (intersectionY > 0 && intersectionY < dy)
                        {
                            const int intersectionX = x1 + (intersectionY * dx2) / dy;
                            split (intersectionY + y1);
                            next->addLine (intersectionX, x2, winding);
                            addLine (x1, intersectionX, winding);
                            return;
                        }
                    }
                }

                if (diff1 + diff2 > 0)
                {
                    segments.insert (i, LineSegment (x1, x2, winding));
                    return;
                }
            }

            segments.add (LineSegment (x1, x2, winding));
        }

        void split (const int newY)
        {
            jassert (newY > y1 && newY < y2);

            const int dy1 = newY - y1;
            const int dy2 = y2 - y1;
            next = new HorizontalSlice (*this, next, newY, y2);
            y2 = newY;

            LineSegment* const oldSegments = segments.getRawDataPointer();
            LineSegment* const newSegments = next->segments.getRawDataPointer();

            for (int i = 0; i < segments.size(); ++i)
            {
                LineSegment& l = oldSegments[i];
                const int newX = l.x1 + dy1 * (l.x2 - l.x1) / dy2;
                newSegments[i].x1 = newX;
                l.x2 = newX;
            }
        }

        template <class Consumer>
        void iterate (Consumer& consumer, const int windingMask)
        {
            jassert (segments.size() > 0);

            const float fy1 = intToFloat (y1);
            const float fy2 = intToFloat (y2);

            const LineSegment* s1 = segments.getRawDataPointer();
            const LineSegment* s2 = s1;
            int winding = s1->winding;

            for (int i = segments.size(); --i > 0;)
            {
                ++s2;
                winding += s2->winding;

                if ((winding & windingMask) == 0)
                {
                    const float ax1 = intToFloat (s1->x1);
                    const float ax2 = intToFloat (s1->x2);

                    if (s1->x1 == s2->x1)
                        consumer.addTriangle (ax1, fy1, ax2, fy2, intToFloat (s2->x2), fy2);
                    else if (s1->x2 == s2->x2)
                        consumer.addTriangle (ax1, fy1, intToFloat (s2->x1), fy1, ax2, fy2);
                    else
                        consumer.addTrapezoid (fy1, fy2, ax1, ax2, intToFloat (s2->x1), intToFloat (s2->x2));

                    s1 = s2 + 1;
                }
            }
        }

        HorizontalSlice* next;
        int y1, y2;

    private:
        struct LineSegment
        {
            inline LineSegment (int x1_, int x2_, int winding_) noexcept
                : x1 (x1_), x2 (x2_), winding (winding_) {}

            int x1, x2;
            int winding;
        };

        Array<LineSegment> segments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HorizontalSlice);
    };

    HorizontalSlice* firstSlice;
    const int windingMask;

    inline void insert (HorizontalSlice* const last, HorizontalSlice* const newOne) noexcept
    {
        if (last == nullptr)
            firstSlice = newOne;
        else
            last->next = newOne;
    }

    enum { factor = 128 };
    static inline int floatToInt (const float n) noexcept     { return roundToInt (n * (float) factor); }
    static inline float intToFloat (const int n) noexcept     { return n * (1.0f/ (float) factor); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrapezoidedPath);
};

//==============================================================================
// Breaks a path into a set of openGL triangles..
class TriangulatedPath
{
public:
    TriangulatedPath (const Path& path)
    {
        startNewBlock();

        TrapezoidedPath (path).iterate (*this);
    }

    void draw (const int oversamplingLevel) const
    {
        glColor4f (1.0f, 1.0f, 1.0f, 1.0f / (oversamplingLevel * oversamplingLevel));

        glTranslatef (-0.5f, -0.5f, 0.0f);
        const float inc = 1.0f / oversamplingLevel;

        for (int y = oversamplingLevel; --y >= 0;)
        {
            for (int x = oversamplingLevel; --x >= 0;)
            {
                glTranslatef (inc, 0.0f, 0.0f);

                for (int i = 0; i < blocks.size(); ++i)
                    blocks.getUnchecked(i)->draw();
            }

            glTranslatef (-1.0f, inc, 0.0f);
        }
    }

    void addTriangle (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
    {
        if (currentBlock->numDone >= trianglesPerBlock)
            startNewBlock();

        GLfloat* t = currentBlock->getNextTriangle();
        *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y3;

        currentBlock->numDone++;
    }

    void addTrapezoid (GLfloat y1, GLfloat y2, GLfloat x1, GLfloat x2, GLfloat x3, GLfloat x4)
    {
        if (currentBlock->numDone >= trianglesPerBlock - 1)
            startNewBlock();

        GLfloat* t = currentBlock->getNextTriangle();
        *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;
        *t++ = x4; *t++ = y2; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;

        currentBlock->numDone += 2;
    }

private:
    // Some GL implementations can't take very large triangle lists, so store
    // the list as a series of blocks containing this max number of triangles.
    enum { trianglesPerBlock = 2048 };

    struct TriangleBlock
    {
        TriangleBlock() noexcept : numDone (0) {}

        void draw() const
        {
            glVertexPointer (2, GL_FLOAT, 0, triangles);
            glDrawArrays (GL_TRIANGLES, 0, numDone * 3);
        }

        inline GLfloat* getNextTriangle() noexcept    { return triangles + numDone * 6; }

        int numDone;
        GLfloat triangles [trianglesPerBlock * 6];
    };

    void startNewBlock()
    {
        currentBlock = new TriangleBlock();
        blocks.add (currentBlock);
    }

    OwnedArray<TriangleBlock> blocks;
    TriangleBlock* currentBlock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TriangulatedPath);
};

//==============================================================================
void OpenGLFrameBuffer::createAlphaChannelFromPath (const Path& path, const int oversamplingLevel)
{
    makeCurrentTarget();

    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glDisable (GL_TEXTURE_2D);
    glDisable (GL_DEPTH_TEST);
    glColorMask (GL_FALSE, GL_FALSE, GL_FALSE, GL_TRUE);
    glEnable (GL_BLEND);
    glBlendFunc (GL_ONE, GL_ONE);

    OpenGLHelpers::prepareFor2D (getWidth(), getHeight());

    TriangulatedPath (path).draw (oversamplingLevel);
}

END_JUCE_NAMESPACE
