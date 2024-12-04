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

namespace juce
{

//==============================================================================
/**
    Creates an openGL frame buffer.

    @tags{OpenGL}
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
    GLuint getFrameBufferID() const noexcept;

    /** Returns the current frame buffer ID for the current context. */
    static GLuint getCurrentFrameBufferTarget() noexcept;

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
    std::unique_ptr<Pimpl> pimpl;

    class SavedState;
    std::unique_ptr<SavedState> savedState;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OpenGLFrameBuffer)
};

} // namespace juce
