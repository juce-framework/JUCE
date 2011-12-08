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

#ifndef __JUCE_CACHEDCOMPONENTIMAGE_JUCEHEADER__
#define __JUCE_CACHEDCOMPONENTIMAGE_JUCEHEADER__

class Component;


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

    /** Invalidates all cached image data. */
    virtual void invalidateAll() = 0;

    /** Invalidates a section of the cached image data. */
    virtual void invalidate (const Rectangle<int>& area) = 0;

    /** Called to indicate that the component is no longer active, so
        any cached data should be released if possible.
    */
    virtual void releaseResources() = 0;
};

#endif   // __JUCE_CACHEDCOMPONENTIMAGE_JUCEHEADER__
