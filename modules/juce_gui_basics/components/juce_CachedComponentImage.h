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

namespace juce
{

//==============================================================================
/**
    Base class used internally for structures that can store cached images of
    component state.

    Most people are unlikely to ever need to know about this class - it's really
    only for power-users!

    @see Component::setCachedComponentImage

    @tags{GUI}
*/
class JUCE_API  CachedComponentImage
{
public:
    CachedComponentImage() = default;
    virtual ~CachedComponentImage() = default;

    //==============================================================================
    /** Called as part of the parent component's paint method, this must draw
        the given component into the target graphics context, using the cached
        version where possible.
    */
    virtual void paint (Graphics&) = 0;

    /** Invalidates all cached image data.
        @returns true if the peer should also be repainted, or false if this object
                 handles all repaint work internally.
    */
    virtual bool invalidateAll() = 0;

    /** Invalidates a section of the cached image data.
        @returns true if the peer should also be repainted, or false if this object
                 handles all repaint work internally.
    */
    virtual bool invalidate (const Rectangle<int>& area) = 0;

    /** Called to indicate that the component is no longer active, so
        any cached data should be released if possible.
    */
    virtual void releaseResources() = 0;
};

} // namespace juce
