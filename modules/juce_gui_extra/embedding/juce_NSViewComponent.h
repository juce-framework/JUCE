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

#ifndef JUCE_NSVIEWCOMPONENT_H_INCLUDED
#define JUCE_NSVIEWCOMPONENT_H_INCLUDED

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
    void paint (Graphics&) override;
    /** @internal */
    static ReferenceCountedObject* attachViewToComponent (Component&, void*);

private:
    ReferenceCountedObjectPtr<ReferenceCountedObject> attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NSViewComponent)
};

#endif
#endif   // JUCE_NSVIEWCOMPONENT_H_INCLUDED
