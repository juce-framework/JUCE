/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_cryptography
  vendor:             juce
  version:            6.1.6
  name:               JUCE cryptography classes
  description:        Classes for various basic cryptography functions, including RSA, Blowfish, MD5, SHA, etc.
  website:            http://www.juce.com/juce
  license:            GPL/Commercial
  minimumCppStandard: 14

  dependencies:       juce_core

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/


#pragma once
#define JUCE_CRYPTOGRAPHY_H_INCLUDED

//==============================================================================
#include <juce_core/juce_core.h>

#include "encryption/juce_BlowFish.h"
#include "encryption/juce_Primes.h"
#include "encryption/juce_RSAKey.h"
#include "hashing/juce_MD5.h"
#include "hashing/juce_SHA256.h"
#include "hashing/juce_Whirlpool.h"
