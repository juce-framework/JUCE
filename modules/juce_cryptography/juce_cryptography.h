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
