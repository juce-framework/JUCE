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

#ifndef JUCE_TRACKTION_MARKETPLACE_H_INCLUDED
#define JUCE_TRACKTION_MARKETPLACE_H_INCLUDED

/**
    The Tracktion Marketplace module is a simple user-registration system for
    allowing you to build apps/plugins with features that are unlocked by a
    user having a suitable account on a webserver.

    Although originally designed for use with products that are sold on the
    Tracktion Marketplace web-store, the module itself is fully open, and can
    be used to connect to your own web-store instead, if you implement your
    own compatible web-server back-end.
*/

//=============================================================================
#include "../juce_cryptography/juce_cryptography.h"
#include "../juce_data_structures/juce_data_structures.h"

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include "../juce_gui_extra/juce_gui_extra.h"
#endif

namespace juce
{
    #include "marketplace/juce_TracktionMarketplaceStatus.h"
    #include "marketplace/juce_TracktionMarketplaceServer.h"

    #if JUCE_MODULE_AVAILABLE_juce_gui_extra
     #include "marketplace/juce_TracktionMarketplaceUnlockForm.h"
    #endif
}


#endif   // JUCE_TRACKTION_MARKETPLACE_H_INCLUDED
