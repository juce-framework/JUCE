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

#include "detail/juce_ViewportHelpers.h"
#include "detail/juce_ButtonAccessibilityHandler.h"

#include "buttons/juce_ArrowButton.cpp"
#include "buttons/juce_Button.cpp"
#include "buttons/juce_DrawableButton.cpp"
#include "buttons/juce_HyperlinkButton.cpp"
#include "buttons/juce_ImageButton.cpp"
#include "buttons/juce_ShapeButton.cpp"
#include "buttons/juce_TextButton.cpp"
#include "buttons/juce_ToggleButton.cpp"
#include "buttons/juce_ToolbarButton.cpp"

#include "layout/juce_ComponentAnimator.cpp"
#include "layout/juce_ComponentBoundsConstrainer.cpp"
#include "layout/juce_BorderedComponentBoundsConstrainer.cpp"
#include "layout/juce_ComponentBuilder.cpp"
#include "layout/juce_ComponentMovementWatcher.cpp"
#include "layout/juce_ConcertinaPanel.cpp"
#include "layout/juce_FlexBox.cpp"
#include "layout/juce_Grid.cpp"
#include "layout/juce_GridItem.cpp"
#include "layout/juce_GroupComponent.cpp"
#include "layout/juce_MultiDocumentPanel.cpp"
#include "layout/juce_ResizableBorderComponent.cpp"
#include "layout/juce_ResizableCornerComponent.cpp"
#include "layout/juce_ResizableEdgeComponent.cpp"
#include "layout/juce_ScrollBar.cpp"
#include "layout/juce_SidePanel.cpp"
#include "layout/juce_StretchableLayoutManager.cpp"
#include "layout/juce_StretchableLayoutResizerBar.cpp"
#include "layout/juce_StretchableObjectResizer.cpp"
#include "layout/juce_TabbedButtonBar.cpp"
#include "layout/juce_TabbedComponent.cpp"
#include "layout/juce_Viewport.cpp"

#include "properties/juce_BooleanPropertyComponent.cpp"
#include "properties/juce_ButtonPropertyComponent.cpp"
#include "properties/juce_ChoicePropertyComponent.cpp"
#include "properties/juce_MultiChoicePropertyComponent.cpp"
#include "properties/juce_PropertyComponent.cpp"
#include "properties/juce_PropertyPanel.cpp"
#include "properties/juce_SliderPropertyComponent.cpp"
#include "properties/juce_TextPropertyComponent.cpp"
