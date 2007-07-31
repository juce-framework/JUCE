/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../../juce_core/basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_ApplicationCommandInfo.h"


//==============================================================================
ApplicationCommandInfo::ApplicationCommandInfo (const CommandID commandID_) throw()
    : commandID (commandID_),
      flags (0)
{
}

void ApplicationCommandInfo::setInfo (const String& shortName_,
                                      const String& description_,
                                      const String& categoryName_,
                                      const int flags_) throw()
{
    shortName = shortName_;
    description = description_;
    categoryName = categoryName_;
    flags = flags_;
}

void ApplicationCommandInfo::setActive (const bool b) throw()
{
    if (b)
        flags &= ~isDisabled;
    else
        flags |= isDisabled;
}

void ApplicationCommandInfo::setTicked (const bool b) throw()
{
    if (b)
        flags |= isTicked;
    else
        flags &= ~isTicked;
}

void ApplicationCommandInfo::addDefaultKeypress (const int keyCode, const ModifierKeys& modifiers) throw()
{
    defaultKeypresses.add (KeyPress (keyCode, modifiers, 0));
}


END_JUCE_NAMESPACE
