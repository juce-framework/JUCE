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

  ID:               juce_cryptography
  vendor:           juce
  version:          4.3.0
  name:             JUCE cryptography classes
  description:      Classes for various basic cryptography functions, including RSA, Blowfish, MD5, SHA, etc.
  website:          http://www.juce.com/juce
  license:          GPL/Commercial

  dependencies:     juce_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#ifndef JUCE_CRYPTOGRAPHY_H_INCLUDED
#define JUCE_CRYPTOGRAPHY_H_INCLUDED

//==============================================================================
#include <juce_core/juce_core.h>

namespace juce
{

#include "encryption/juce_BlowFish.h"
#include "encryption/juce_Primes.h"
#include "encryption/juce_RSAKey.h"
#include "hashing/juce_MD5.h"
#include "hashing/juce_SHA256.h"
#include "hashing/juce_Whirlpool.h"

}

#endif   // JUCE_CRYPTOGRAPHY_H_INCLUDED
