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

namespace juce
{

void ComponentListener::componentMovedOrResized (Component&, bool, bool)    {}
void ComponentListener::componentBroughtToFront (Component&)                {}
void ComponentListener::componentVisibilityChanged (Component&)             {}
void ComponentListener::componentChildrenChanged (Component&)               {}
void ComponentListener::componentParentHierarchyChanged (Component&)        {}
void ComponentListener::componentNameChanged (Component&)                   {}
void ComponentListener::componentBeingDeleted (Component&)                  {}
void ComponentListener::componentEnablementChanged (Component&)             {}

} // namespace juce
