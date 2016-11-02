/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.txt file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:               juce_tracktion_marketplace
  vendor:           juce
  version:          4.3.0
  name:             JUCE Tracktion marketplace support
  description:      Classes for online product authentication via the Tracktion marketplace.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_cryptography

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


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

//==============================================================================
#include <juce_cryptography/juce_cryptography.h>

#if JUCE_MODULE_AVAILABLE_juce_data_structures
 #include <juce_data_structures/juce_data_structures.h>
#endif

#if JUCE_MODULE_AVAILABLE_juce_gui_extra
 #include <juce_gui_extra/juce_gui_extra.h>
#endif

namespace juce
{
   #if JUCE_MODULE_AVAILABLE_juce_data_structures
    #include "marketplace/juce_OnlineUnlockStatus.h"
    #include "marketplace/juce_TracktionMarketplaceStatus.h"
   #endif
    #include "marketplace/juce_KeyFileGeneration.h"

   #if JUCE_MODULE_AVAILABLE_juce_gui_extra
    #include "marketplace/juce_OnlineUnlockForm.h"
   #endif
}


#endif   // JUCE_TRACKTION_MARKETPLACE_H_INCLUDED
