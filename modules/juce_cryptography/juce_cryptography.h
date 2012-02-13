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

#ifndef __JUCE_CRYPTOGRAPHY_JUCEHEADER__
#define __JUCE_CRYPTOGRAPHY_JUCEHEADER__

//=============================================================================
#include "../juce_core/juce_core.h"

namespace juce
{

// START_AUTOINCLUDE encryption, hashing
#ifndef __JUCE_BLOWFISH_JUCEHEADER__
 #include "encryption/juce_BlowFish.h"
#endif
#ifndef __JUCE_PRIMES_JUCEHEADER__
 #include "encryption/juce_Primes.h"
#endif
#ifndef __JUCE_RSAKEY_JUCEHEADER__
 #include "encryption/juce_RSAKey.h"
#endif
#ifndef __JUCE_MD5_JUCEHEADER__
 #include "hashing/juce_MD5.h"
#endif
#ifndef __JUCE_SHA256_JUCEHEADER__
 #include "hashing/juce_SHA256.h"
#endif
// END_AUTOINCLUDE

}

#endif   // __JUCE_CRYPTOGRAPHY_JUCEHEADER__
