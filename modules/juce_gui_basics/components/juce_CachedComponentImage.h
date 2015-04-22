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

#ifndef JUCE_CACHEDCOMPONENTIMAGE_H_INCLUDED
#define JUCE_CACHEDCOMPONENTIMAGE_H_INCLUDED


//==============================================================================
/**
    Base class used internally for structures that can store cached images of
    component state.

    Most people are unlikely to ever need to know about this class - it's really
    only for power-users!

    @see Component::setCachedComponentImage
*/
class JUCE_API  CachedComponentImage
{
public:
    CachedComponentImage() noexcept {}
    virtual ~CachedComponentImage() {}

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

#endif   // JUCE_CACHEDCOMPONENTIMAGE_H_INCLUDED
