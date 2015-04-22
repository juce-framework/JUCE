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

ApplicationCommandInfo::ApplicationCommandInfo (const CommandID cid) noexcept
    : commandID (cid), flags (0)
{
}

void ApplicationCommandInfo::setInfo (const String& shortName_,
                                      const String& description_,
                                      const String& categoryName_,
                                      const int flags_) noexcept
{
    shortName = shortName_;
    description = description_;
    categoryName = categoryName_;
    flags = flags_;
}

void ApplicationCommandInfo::setActive (const bool b) noexcept
{
    if (b)
        flags &= ~isDisabled;
    else
        flags |= isDisabled;
}

void ApplicationCommandInfo::setTicked (const bool b) noexcept
{
    if (b)
        flags |= isTicked;
    else
        flags &= ~isTicked;
}

void ApplicationCommandInfo::addDefaultKeypress (const int keyCode, ModifierKeys modifiers) noexcept
{
    defaultKeypresses.add (KeyPress (keyCode, modifiers, 0));
}
