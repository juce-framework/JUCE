/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 7 End-User License
   Agreement and JUCE Privacy Policy.

   End User License Agreement: www.juce.com/juce-7-licence
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

/**
    Objects of this type can be used to programmatically close message boxes.

    @see NativeMessageBox::showScopedAsync(), AlertWindow::showScopedAsync()

    @tags{GUI}
*/
class ScopedMessageBox
{
public:
    /** @internal */
    explicit ScopedMessageBox (std::shared_ptr<detail::ScopedMessageBoxImpl>);

    /** Constructor */
    ScopedMessageBox();

    /** Destructor */
    ~ScopedMessageBox() noexcept;

    /** Move constructor */
    ScopedMessageBox (ScopedMessageBox&&) noexcept;

    /** Move assignment operator */
    ScopedMessageBox& operator= (ScopedMessageBox&&) noexcept;

    /** Closes the message box, if it is currently showing.

        This is also called automatically during ~ScopedMessageBox. This is useful if you want
        to display a message corresponding to a particular view, and hide the message automatically
        when the view is hidden. This situation commonly arises when displaying messages in plugin
        editors.
    */
    void close();

private:
    std::shared_ptr<detail::ScopedMessageBoxImpl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ScopedMessageBox)
};

} // namespace juce
