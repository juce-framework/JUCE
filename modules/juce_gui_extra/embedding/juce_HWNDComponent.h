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

#if JUCE_WINDOWS || DOXYGEN

//==============================================================================
/**
    A Windows-specific class that can create and embed a HWND inside itself.

    To use it, create one of these, put it in place and make sure it's visible in a
    window, then use setHWND() to assign a HWND to it. The window will then be
    moved and resized to follow the movements of this component.

    Of course, since the window is a native object, it'll obliterate any
    JUCE components that may overlap this component, but that's life.

    @tags{GUI}
*/
class JUCE_API  HWNDComponent   : public Component
{
public:
    //==============================================================================
    /** Create an initially-empty container. */
    HWNDComponent();

    /** Destructor. */
    ~HWNDComponent() override;

    /** Assigns a HWND to this peer.

        The window will be retained and released by this component for as long as
        it is needed. To remove the current HWND, just call setHWND (nullptr).

        Note: A void* is used here to avoid including the Windows headers as
        part of JuceHeader.h, but the method expects a HWND.
    */
    void setHWND (void* hwnd);

    /** Returns the current HWND.

        Note: A void* is returned here to avoid the needing to include the Windows
        headers, so you should just cast the return value to a HWND.
    */
    void* getHWND() const;

    /** Resizes this component to fit the HWND that it contains. */
    void resizeToFit();

    /** @internal */
    void paint (Graphics&) override;

private:
    class Pimpl;
    std::unique_ptr<Pimpl> pimpl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HWNDComponent)
};

#endif

} // namespace juce
