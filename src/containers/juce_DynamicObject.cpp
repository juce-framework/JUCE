/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-10 by Raw Material Software Ltd.

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

#include "../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_DynamicObject.h"


//==============================================================================
DynamicObject::DynamicObject()
{
}

DynamicObject::~DynamicObject()
{
}

bool DynamicObject::hasProperty (const var::identifier& propertyName) const
{
    var* const v = properties.getItem (propertyName);
    return v != 0 && ! v->isMethod();
}

const var DynamicObject::getProperty (const var::identifier& propertyName) const
{
    return properties [propertyName];
}

void DynamicObject::setProperty (const var::identifier& propertyName, const var& newValue)
{
    properties.set (propertyName, newValue);
}

void DynamicObject::removeProperty (const var::identifier& propertyName)
{
    properties.remove (propertyName);
}

bool DynamicObject::hasMethod (const var::identifier& methodName) const
{
    return getProperty (methodName).isMethod();
}

const var DynamicObject::invokeMethod (const var::identifier& methodName,
                                       const var* parameters,
                                       int numParameters)
{
    return properties [methodName].invoke (var (this), parameters, numParameters);
}

void DynamicObject::setMethod (const var::identifier& name,
                               var::MethodFunction methodFunction)
{
    properties.set (name, var (methodFunction));
}

void DynamicObject::clear()
{
    properties.clear();
}


END_JUCE_NAMESPACE
