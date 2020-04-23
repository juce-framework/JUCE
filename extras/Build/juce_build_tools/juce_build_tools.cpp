/*
  ==============================================================================

   This file is part of the JUCE 6 technical preview.
   Copyright (c) 2020 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For this technical preview, this file is not subject to commercial licensing.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_BUILD_TOOLS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_build_tools.h"

#include <numeric>

#include "utils/juce_CppTokeniserFunctions.cpp"
#include "utils/juce_BuildHelperFunctions.cpp"
#include "utils/juce_BinaryResourceFile.cpp"
#include "utils/juce_Icons.cpp"
#include "utils/juce_PlistOptions.cpp"
#include "utils/juce_ResourceFileHelpers.cpp"
#include "utils/juce_ResourceRc.cpp"
#include "utils/juce_VersionNumbers.cpp"
#include "utils/juce_Entitlements.cpp"
