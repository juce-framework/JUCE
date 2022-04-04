/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

/** This wraps a context menu for a specific parameter, as provided by the host.

    You can choose to create a standard PopupMenu to display the host-provided
    options. Alternatively, you can ask the host to display a native menu at
    a specific location.

    @tags{Audio}
*/
struct HostProvidedContextMenu
{
    virtual ~HostProvidedContextMenu() = default;

    /** Get a PopupMenu holding entries specified by the host.

        Most hosts will populate this menu with options that relate to the
        parameter, such as displaying its automation lane. You are free
        to modify this menu before displaying it, if you wish to add additional
        options.
    */
    virtual PopupMenu getEquivalentPopupMenu() const = 0;

    /** Asks the host to display its native menu at a location relative
        to the top left corner of the editor.

        The position you provide should be in logical pixels. To display
        the menu next to the mouse cursor, call Component::getMouseXYRelative()
        on your editor and pass the result to this function.
    */
    virtual void showNativeMenu (Point<int> pos) const = 0;
};

/** Calling AudioProcessorEditor::getHostContext() may return a pointer to an
    instance of this class.

    At the moment, this can be used to retrieve context menus for parameters in
    compatible VST3 hosts. Additional extensions may be added here in the future.

    @tags{Audio}
*/
struct AudioProcessorEditorHostContext
{
    virtual ~AudioProcessorEditorHostContext() = default;

    /** Returns an object which can be used to display a context menu for the
        parameter with the given index.
    */
    virtual std::unique_ptr<HostProvidedContextMenu> getContextMenuForParameter (const AudioProcessorParameter *) const = 0;

    /** The naming of this function is misleading. Use getContextMenuForParameter() instead.

        Returns an object which can be used to display a context menu for the
        parameter with the given index.
    */
    [[deprecated ("The naming of this function has been fixed, use getContextMenuForParameter instead")]]
    virtual std::unique_ptr<HostProvidedContextMenu> getContextMenuForParameterIndex (const AudioProcessorParameter * p) const
    {
        return getContextMenuForParameter (p);
    }
};

} // namespace juce
