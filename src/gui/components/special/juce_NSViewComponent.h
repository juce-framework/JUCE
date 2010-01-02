/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

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

#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_NSVIEWCOMPONENT_JUCEHEADER__

#include "../juce_Component.h"
#if ! DOXYGEN
 class NSViewComponentInternal;
#endif

#if JUCE_MAC || DOXYGEN

//==============================================================================
/**
    A Mac-specific class that can create and embed an NSView inside itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use setView() to assign an NSView to it. The view will then be
    moved and resized to follow the movements of this component.

    Of course, since the view is a native object, it'll obliterate any
    juce components that may overlap this component, but that's life.
*/
class JUCE_API  NSViewComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    NSViewComponent();

    /** Destructor. */
    ~NSViewComponent();

    /** Assigns an NSView to this peer.

        The view will be retained and released by this component for as long as
        it is needed. To remove the current view, just call setView (0).

        Note: a void* is used here to avoid including the cocoa headers as
        part of the juce.h, but the method expects an NSView*.
    */
    void setView (void* nsView);

    /** Returns the current NSView.

        Note: a void* is returned here to avoid including the cocoa headers as
        a requirement of juce.h, so you should just cast the object to an NSView*.
    */
    void* getView() const;

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

    juce_UseDebuggingNewOperator

private:
    friend class NSViewComponentInternal;
    ScopedPointer <NSViewComponentInternal> info;

    NSViewComponent (const NSViewComponent&);
    const NSViewComponent& operator= (const NSViewComponent&);
};

#endif

#endif   // __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
