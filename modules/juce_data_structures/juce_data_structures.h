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

#ifndef __JUCE_DATA_STRUCTURES_JUCEHEADER__
#define __JUCE_DATA_STRUCTURES_JUCEHEADER__

//=============================================================================
#include "../juce_events/juce_events.h"

namespace juce
{

// START_AUTOINCLUDE values, undomanager, app_properties
#ifndef __JUCE_VALUE_JUCEHEADER__
 #include "values/juce_Value.h"
#endif
#ifndef __JUCE_VALUETREE_JUCEHEADER__
 #include "values/juce_ValueTree.h"
#endif
#ifndef __JUCE_UNDOABLEACTION_JUCEHEADER__
 #include "undomanager/juce_UndoableAction.h"
#endif
#ifndef __JUCE_UNDOMANAGER_JUCEHEADER__
 #include "undomanager/juce_UndoManager.h"
#endif
#ifndef __JUCE_APPLICATIONPROPERTIES_JUCEHEADER__
 #include "app_properties/juce_ApplicationProperties.h"
#endif
#ifndef __JUCE_PROPERTIESFILE_JUCEHEADER__
 #include "app_properties/juce_PropertiesFile.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_DATA_STRUCTURES_JUCEHEADER__
