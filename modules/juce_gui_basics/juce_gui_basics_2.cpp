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

#ifdef JUCE_GUI_BASICS_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#include "juce_gui_basics.h"

#include "detail/juce_LookAndFeelHelpers.h"

#include "lookandfeel/juce_LookAndFeel.cpp"
#include "lookandfeel/juce_LookAndFeel_V1.cpp"
#include "lookandfeel/juce_LookAndFeel_V2.cpp"
#include "lookandfeel/juce_LookAndFeel_V3.cpp"
#include "lookandfeel/juce_LookAndFeel_V4.cpp"

#include "positioning/juce_MarkerList.cpp"
#include "positioning/juce_RelativeCoordinate.cpp"
#include "positioning/juce_RelativeCoordinatePositioner.cpp"
#include "positioning/juce_RelativeParallelogram.cpp"
#include "positioning/juce_RelativePoint.cpp"
#include "positioning/juce_RelativePointPath.cpp"
#include "positioning/juce_RelativeRectangle.cpp"

#include "drawables/juce_Drawable.cpp"
#include "drawables/juce_DrawableComposite.cpp"
#include "drawables/juce_DrawableImage.cpp"
#include "drawables/juce_DrawablePath.cpp"
#include "drawables/juce_DrawableRectangle.cpp"
#include "drawables/juce_DrawableShape.cpp"
#include "drawables/juce_DrawableText.cpp"
#include "drawables/juce_SVGParser.cpp"
