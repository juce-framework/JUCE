/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

#if JUCE_IOS || DOXYGEN

//==============================================================================
/**
    An iOS-specific class that can create and embed an UIView inside itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use setView() to assign a UIView to it. The view will then be
    moved and resized to follow the movements of this component.

    Of course, since the view is a native object, it'll obliterate any
    juce components that may overlap this component, but that's life.

    @tags{GUI}
*/
class JUCE_API  UIViewComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    UIViewComponent();

    /** Destructor. */
    ~UIViewComponent();

    /** Assigns an UIView to this peer.

        The view will be retained and released by this component for as long as
        it is needed. To remove the current view, just call setView (nullptr).

        Note: a void* is used here to avoid including the cocoa headers as
        part of the juce.h, but the method expects an UIView*.
    */
    void setView (void* uiView);

    /** Returns the current UIView.

        Note: a void* is returned here to avoid the needing to include the cocoa
        headers, so you should just cast the return value to an UIView*.
    */
    void* getView() const;


    /** Resizes this component to fit the view that it contains. */
    void resizeToFitView();

    //==============================================================================
    /** @internal */
    void paint (Graphics&) override;


private:
    class Pimpl;
    friend class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIViewComponent)
};

#endif

} // namespace juce
