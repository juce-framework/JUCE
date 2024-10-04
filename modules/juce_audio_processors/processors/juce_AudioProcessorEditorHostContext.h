/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

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
