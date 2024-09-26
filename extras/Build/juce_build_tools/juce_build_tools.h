/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this module, and is read by
 the Projucer to automatically generate project code that uses it.
 For details about the syntax and how to create or use a module, see the
 JUCE Module Format.md file.


 BEGIN_JUCE_MODULE_DECLARATION

  ID:                 juce_build_tools
  vendor:             juce
  version:            8.0.2
  name:               JUCE Build Tools
  description:        Classes for generating intermediate files for JUCE projects.
  website:            http://www.juce.com/juce
  license:            AGPLv3/Commercial

  dependencies:       juce_gui_basics

 END_JUCE_MODULE_DECLARATION

*******************************************************************************/

// This module is shared by juceaide and the Projucer, but should not be
// considered 'public'. That is, its API, functionality, and contents (and
// existence!) may change between releases without warning.

#pragma once
#define JUCE_BUILD_TOOLS_H_INCLUDED

#include <juce_gui_basics/juce_gui_basics.h>

#include "utils/juce_ProjectType.h"
#include "utils/juce_BuildHelperFunctions.h"
#include "utils/juce_BinaryResourceFile.h"
#include "utils/juce_RelativePath.h"
#include "utils/juce_Icons.h"
#include "utils/juce_PlistOptions.h"
#include "utils/juce_ResourceFileHelpers.h"
#include "utils/juce_ResourceRc.h"
#include "utils/juce_VersionNumbers.h"
#include "utils/juce_Entitlements.h"
