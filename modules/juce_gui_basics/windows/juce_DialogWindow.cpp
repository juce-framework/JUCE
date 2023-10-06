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

DialogWindow::DialogWindow (const String& name, Colour colour,
                            const bool escapeCloses, const bool onDesktop,
                            const float scale)
    : DocumentWindow (name, colour, DocumentWindow::closeButton, onDesktop),
      desktopScale (scale),
      escapeKeyTriggersCloseButton (escapeCloses)
{
}

DialogWindow::~DialogWindow() = default;

bool DialogWindow::escapeKeyPressed()
{
    if (escapeKeyTriggersCloseButton)
    {
        setVisible (false);
        return true;
    }

    return false;
}

bool DialogWindow::keyPressed (const KeyPress& key)
{
    if (key == KeyPress::escapeKey && escapeKeyPressed())
        return true;

    return DocumentWindow::keyPressed (key);
}

void DialogWindow::resized()
{
    DocumentWindow::resized();

    if (escapeKeyTriggersCloseButton)
    {
        if (auto* close = getCloseButton())
        {
            const KeyPress esc (KeyPress::escapeKey, 0, 0);

            if (! close->isRegisteredForShortcut (esc))
                close->addShortcut (esc);
        }
    }
}

//==============================================================================
class DefaultDialogWindow final : public DialogWindow
{
public:
    DefaultDialogWindow (LaunchOptions& options)
        : DialogWindow (options.dialogTitle, options.dialogBackgroundColour,
                        options.escapeKeyTriggersCloseButton, true,
                        options.componentToCentreAround != nullptr
                            ? Component::getApproximateScaleFactorForComponent (options.componentToCentreAround)
                            : 1.0f)
    {
        if (options.content.willDeleteObject())
            setContentOwned (options.content.release(), true);
        else
            setContentNonOwned (options.content.release(), true);

        centreAroundComponent (options.componentToCentreAround, getWidth(), getHeight());
        setResizable (options.resizable, options.useBottomRightCornerResizer);

        setUsingNativeTitleBar (options.useNativeTitleBar);
        setAlwaysOnTop (WindowUtils::areThereAnyAlwaysOnTopWindows());
    }

    void closeButtonPressed() override
    {
        setVisible (false);
    }

private:
    JUCE_DECLARE_NON_COPYABLE (DefaultDialogWindow)
};

DialogWindow::LaunchOptions::LaunchOptions() noexcept {}

DialogWindow* DialogWindow::LaunchOptions::create()
{
    jassert (content != nullptr); // You need to provide some kind of content for the dialog!

    return new DefaultDialogWindow (*this);
}

DialogWindow* DialogWindow::LaunchOptions::launchAsync()
{
    auto* d = create();
    d->enterModalState (true, nullptr, true);
    return d;
}

#if JUCE_MODAL_LOOPS_PERMITTED
int DialogWindow::LaunchOptions::runModal()
{
    return launchAsync()->runModalLoop();
}
#endif

//==============================================================================
void DialogWindow::showDialog (const String& dialogTitle,
                               Component* const contentComponent,
                               Component* const componentToCentreAround,
                               Colour backgroundColour,
                               const bool escapeKeyTriggersCloseButton,
                               const bool resizable,
                               const bool useBottomRightCornerResizer)
{
    LaunchOptions o;
    o.dialogTitle = dialogTitle;
    o.content.setNonOwned (contentComponent);
    o.componentToCentreAround = componentToCentreAround;
    o.dialogBackgroundColour = backgroundColour;
    o.escapeKeyTriggersCloseButton = escapeKeyTriggersCloseButton;
    o.useNativeTitleBar = false;
    o.resizable = resizable;
    o.useBottomRightCornerResizer = useBottomRightCornerResizer;

    o.launchAsync();
}

#if JUCE_MODAL_LOOPS_PERMITTED
int DialogWindow::showModalDialog (const String& dialogTitle,
                                   Component* const contentComponent,
                                   Component* const componentToCentreAround,
                                   Colour backgroundColour,
                                   const bool escapeKeyTriggersCloseButton,
                                   const bool resizable,
                                   const bool useBottomRightCornerResizer)
{
    LaunchOptions o;
    o.dialogTitle = dialogTitle;
    o.content.setNonOwned (contentComponent);
    o.componentToCentreAround = componentToCentreAround;
    o.dialogBackgroundColour = backgroundColour;
    o.escapeKeyTriggersCloseButton = escapeKeyTriggersCloseButton;
    o.useNativeTitleBar = false;
    o.resizable = resizable;
    o.useBottomRightCornerResizer = useBottomRightCornerResizer;

    return o.runModal();
}
#endif

//==============================================================================
std::unique_ptr<AccessibilityHandler> DialogWindow::createAccessibilityHandler()
{
    return std::make_unique<AccessibilityHandler> (*this, AccessibilityRole::dialogWindow);
}

} // namespace juce
