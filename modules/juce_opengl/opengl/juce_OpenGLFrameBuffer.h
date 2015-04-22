/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_OPENGLFRAMEBUFFER_H_INCLUDED
#define JUCE_OPENGLFRAMEBUFFER_H_INCLUDED


//==============================================================================
/**
    Creates an openGL frame buffer.
*/
class JUCE_API  OpenGLFrameBuffer
{
public:
    /** Creates an uninitialised buffer.
        To actually allocate the buffer, use initialise().
    */
    OpenGLFrameBuffer();

    /** Destructor. */
    ~OpenGLFrameBuffer();

    //==============================================================================
    /** Tries to allocates a buffer of the given size.
        Note that a valid openGL context must be selected when you call this method,
        or it will fail.
    */
    bool initialise (OpenGLContext& context, int width, int height);

    /** Tries to allocates a buffer containing a copy of a given image.
        Note that a valid openGL context must be selected when you call this method,
        or it will fail.
    */
    bool initialise (OpenGLContext& context, const Image& content);

    /** Tries to allocate a copy of another framebuffer.
    */
    bool initialise (OpenGLFrameBuffer& other);

    /** Releases the buffer, if one has been allocated.
        Any saved state that was created with saveAndRelease() will also be freed by this call.
    */
    void release();

    /** If the framebuffer is active, this will save a stashed copy of its contents in main memory,
        and will release the GL buffer.
        After saving, the original state can be restored again by calling reloadSavedCopy().
    */
    void saveAndRelease();

    /** Restores the framebuffer content that was previously saved using saveAndRelease().
        After saving to main memory, the original state can be restored by calling restoreToGPUMemory().
    */
    bool reloadSavedCopy (OpenGLContext& context);

    //==============================================================================
    /** Returns true if a valid buffer has been allocated. */
    bool isValid() const noexcept                       { return pimpl != nullptr; }

    /** Returns the width of the buffer. */
    int getWidth() const noexcept;

    /** Returns the height of the buffer. */
    int getHeight() const noexcept;

    /** Returns the texture ID number for using this buffer as a texture. */
    GLuint getTextureID() const noexcept;

    //==============================================================================
    /** Selects this buffer as the current OpenGL rendering target. */
    bool makeCurrentRenderingTarget();

    /** Deselects this buffer as the current OpenGL rendering target. */
    void releaseAsRenderingTarget();

    /** Returns the ID of this framebuffer, or 0 if it isn't initialised. */
    GLuint getFrameBufferID() const;

    /** Returns the current frame buffer ID for the current context. */
    static GLuint getCurrentFrameBufferTarget();

    /** Clears the framebuffer with the specified colour. */
    void clear (Colour colour);

    /** Selects the framebuffer as the current target, and clears it to transparent. */
    void makeCurrentAndClear();

    /** Reads an area of pixels from the framebuffer into a 32-bit ARGB pixel array.
        The lineStride is measured as a number of pixels, not bytes - pass a stride
        of 0 to indicate a packed array.
    */
    bool readPixels (PixelARGB* targetData, const Rectangle<int>& sourceArea);

    /** Writes an area of pixels into the framebuffer from a specified pixel array.
        The lineStride is measured as a number of pixels, not bytes - pass a stride
        of 0 to indicate a packed array.
    */
    bool writePixels (const PixelARGB* srcData, const Rectangle<int>& targetArea);

private:
    class Pimpl;
    friend struct ContainerDeletePolicy<Pimpl>;
    ScopedPointer<Pimpl> pimpl;

    class SavedState;
    friend struct ContainerDeletePolicy<SavedState>;
    ScopedPointer<SavedState> savedState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBuffer)
};


#endif   // JUCE_OPENGLFRAMEBUFFER_H_INCLUDED
