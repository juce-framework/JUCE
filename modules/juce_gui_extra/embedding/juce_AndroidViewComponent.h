/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_ANDROID || DOXYGEN

//==============================================================================
/**
    An Android-specific class that can create and embed a View inside itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use setView() to assign a View to it. The view will then be
    moved and resized to follow the movements of this component.

    Of course, since the view is a native object, it'll obliterate any
    JUCE components that may overlap this component, but that's life.

    @tags{GUI}
*/
class JUCE_API  AndroidViewComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container */
    AndroidViewComponent();

    /** Destructor. */
    ~AndroidViewComponent() override;

    /** Assigns a View to this peer.

        The view will be retained and released by this component for as long as
        it is needed. To remove the current view, just call setView (nullptr).
    */
    void setView (void* uiView);

    /** Returns the current View. */
    void* getView() const;

    /** Resizes this component to fit the view that it contains. */
    void resizeToFitView();

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AndroidViewComponent)
};

#endif

} // namespace juce
