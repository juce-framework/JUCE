/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

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
