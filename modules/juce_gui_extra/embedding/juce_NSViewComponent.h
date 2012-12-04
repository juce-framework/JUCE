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

#ifndef __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
#define __JUCE_NSVIEWCOMPONENT_JUCEHEADER__

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
        it is needed. To remove the current view, just call setView (nullptr).

        Note: a void* is used here to avoid including the cocoa headers as
        part of the juce.h, but the method expects an NSView*.
    */
    void setView (void* nsView);

    /** Returns the current NSView.

        Note: a void* is returned here to avoid the needing to include the cocoa
        headers, so you should just cast the return value to an NSView*.
    */
    void* getView() const;


    /** Resizes this component to fit the view that it contains. */
    void resizeToFitView();

    //==============================================================================
    /** @internal */
    void paint (Graphics& g);

    /** @internal */
    static ReferenceCountedObject* attachViewToComponent (Component&, void*);

private:
    ReferenceCountedObjectPtr<ReferenceCountedObject> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewComponent)
};

#endif
#endif   // __JUCE_NSVIEWCOMPONENT_JUCEHEADER__
