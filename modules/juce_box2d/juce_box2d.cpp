/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#ifdef JUCE_BOX2D_H_INCLUDED
 /* When you add this cpp file to your project, you mustn't include it in a file where you've
    already included any other headers - just put it inside a file on its own, possibly with your config
    flags preceding it, but don't include anything else. That also includes avoiding any automatic prefix
    header files that the compiler may be using.
 */
 #error "Incorrect use of JUCE cpp file"
#endif

#if defined (__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wfloat-conversion"
#elif defined (__GNUC__)
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
 #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

#include "juce_box2d.h"

#include <cstdarg>

using int8   = juce::int8;
using int16  = juce::int16;
using int32  = juce::int32;
using uint8  = juce::uint8;
using uint16 = juce::uint16;
using uint32 = juce::uint32;

#include "box2d/Collision/b2BroadPhase.cpp"
#include "box2d/Collision/b2CollideCircle.cpp"
#include "box2d/Collision/b2CollideEdge.cpp"
#include "box2d/Collision/b2CollidePolygon.cpp"
#include "box2d/Collision/b2Collision.cpp"
#include "box2d/Collision/b2Distance.cpp"
#include "box2d/Collision/b2DynamicTree.cpp"
#include "box2d/Collision/b2TimeOfImpact.cpp"
#include "box2d/Collision/Shapes/b2ChainShape.cpp"
#include "box2d/Collision/Shapes/b2CircleShape.cpp"
#include "box2d/Collision/Shapes/b2EdgeShape.cpp"
#include "box2d/Collision/Shapes/b2PolygonShape.cpp"
#include "box2d/Common/b2BlockAllocator.cpp"
#include "box2d/Common/b2Draw.cpp"
#include "box2d/Common/b2Math.cpp"
#include "box2d/Common/b2Settings.cpp"
#include "box2d/Common/b2StackAllocator.cpp"
#include "box2d/Common/b2Timer.cpp"
#include "box2d/Dynamics/b2Body.cpp"
#include "box2d/Dynamics/b2ContactManager.cpp"
#include "box2d/Dynamics/b2Fixture.cpp"
#include "box2d/Dynamics/b2Island.cpp"
#include "box2d/Dynamics/b2World.cpp"
#include "box2d/Dynamics/b2WorldCallbacks.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2ChainAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2CircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2Contact.cpp"
#include "box2d/Dynamics/Contacts/b2ContactSolver.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2EdgeAndPolygonContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonAndCircleContact.cpp"
#include "box2d/Dynamics/Contacts/b2PolygonContact.cpp"
#include "box2d/Dynamics/Joints/b2DistanceJoint.cpp"
#include "box2d/Dynamics/Joints/b2FrictionJoint.cpp"
#include "box2d/Dynamics/Joints/b2GearJoint.cpp"
#include "box2d/Dynamics/Joints/b2Joint.cpp"
#include "box2d/Dynamics/Joints/b2MouseJoint.cpp"
#include "box2d/Dynamics/Joints/b2PrismaticJoint.cpp"
#include "box2d/Dynamics/Joints/b2PulleyJoint.cpp"
#include "box2d/Dynamics/Joints/b2RevoluteJoint.cpp"
#include "box2d/Dynamics/Joints/b2RopeJoint.cpp"
#include "box2d/Dynamics/Joints/b2WeldJoint.cpp"
#include "box2d/Dynamics/Joints/b2WheelJoint.cpp"
#include "box2d/Rope/b2Rope.cpp"

#include "utils/juce_Box2DRenderer.cpp"

#if defined (__clang__)
 #pragma clang diagnostic pop
#elif defined (__GNUC__)
 #pragma GCC diagnostic pop
#endif
