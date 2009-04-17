/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-7 by Raw Material Software ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the
   GNU General Public License, as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later version.

   JUCE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with JUCE; if not, visit www.gnu.org/licenses or write to the
   Free Software Foundation, Inc., 59 Temple Place, Suite 330,
   Boston, MA 02111-1307 USA

  ------------------------------------------------------------------------------

   If you'd like to release a closed-source product which uses JUCE, commercial
   licenses are also available: visit www.rawmaterialsoftware.com/juce for
   more information.

  ==============================================================================
*/

#include "../basics/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "juce_Variant.h"


//==============================================================================
var::var() throw()
   : type (voidType)
{
    value.doubleValue = 0;
}

void var::releaseValue() throw()
{
    if (type == stringType)
        delete value.stringValue;
    else if (type == objectType && value.objectValue != 0)
        value.objectValue->decReferenceCount();
}

var::~var()
{
    releaseValue();
}

//==============================================================================
var::var (const var& valueToCopy) throw()
   : type (valueToCopy.type),
     value (valueToCopy.value)
{
    if (type == stringType)
        value.stringValue = new String (*(value.stringValue));
    else if (type == objectType && value.objectValue != 0)
        value.objectValue->incReferenceCount();
}

var::var (const int value_) throw()
   : type (intType)
{
    value.intValue = value_;
}

var::var (const bool value_) throw()
   : type (boolType)
{
    value.boolValue = value_;
}

var::var (const double value_) throw()
   : type (doubleType)
{
    value.doubleValue = value_;
}

var::var (const String& value_) throw()
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (const char* const value_) throw()
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (const juce_wchar* const value_) throw()
   : type (stringType)
{
    value.stringValue = new String (value_);
}

var::var (DynamicObject* const object) throw()
   : type (objectType)
{
    value.objectValue = object;

    if (object != 0)
        object->incReferenceCount();
}

//==============================================================================
const var& var::operator= (const var& valueToCopy) throw()
{
    if (this != &valueToCopy)
    {
        if (type == stringType)
            delete value.stringValue;

        DynamicObject* const oldObject = getObject();

        type = valueToCopy.type;
        value = valueToCopy.value;

        if (type == stringType)
            value.stringValue = new String (*(value.stringValue));
        else if (type == objectType && value.objectValue != 0)
            value.objectValue->incReferenceCount();

        if (oldObject != 0)
            oldObject->decReferenceCount();
    }

    return *this;
}

const var& var::operator= (const int value_) throw()
{
    releaseValue();
    type = intType;
    value.intValue = value_;
    return *this;
}

const var& var::operator= (const bool value_) throw()
{
    releaseValue();
    type = boolType;
    value.boolValue = value_;
    return *this;
}

const var& var::operator= (const double value_) throw()
{
    releaseValue();
    type = doubleType;
    value.doubleValue = value_;
    return *this;
}

const var& var::operator= (const char* const value_) throw()
{
    releaseValue();
    type = stringType;
    value.stringValue = new String (value_);
    return *this;
}

const var& var::operator= (const juce_wchar* const value_) throw()
{
    releaseValue();
    type = stringType;
    value.stringValue = new String (value_);
    return *this;
}

const var& var::operator= (const String& value_) throw()
{
    releaseValue();
    type = stringType;
    value.stringValue = new String (value_);
    return *this;
}

const var& var::operator= (DynamicObject* const value_) throw()
{
    value_->incReferenceCount();
    releaseValue();
    type = objectType;
    value.objectValue = value_;
    return *this;
}

//==============================================================================
var::operator int() const throw()
{
    switch (type)
    {
        case voidType:
        case objectType:    break;
        case intType:       return value.intValue;
        case boolType:      return value.boolValue ? 1 : 0;
        case doubleType:    return (int) value.doubleValue;
        case stringType:    return value.stringValue->getIntValue();
        default:            jassertfalse; break;
    }

    return 0;
}

var::operator bool() const throw()
{
    switch (type)
    {
        case voidType:      break;
        case objectType:    return value.objectValue != 0;
        case intType:       return value.intValue != 0;
        case boolType:      return value.boolValue;
        case doubleType:    return value.doubleValue != 0;
        case stringType:    return value.stringValue->getIntValue() != 0
                                    || value.stringValue->trim().equalsIgnoreCase (T("true"))
                                    || value.stringValue->trim().equalsIgnoreCase (T("yes"));
        default:            jassertfalse; break;
    }

    return false;
}

var::operator double() const throw()
{
    switch (type)
    {
        case voidType:
        case objectType:    break;
        case intType:       return value.intValue;
        case boolType:      return value.boolValue ? 1.0 : 0.0;
        case doubleType:    return value.doubleValue;
        case stringType:    return value.stringValue->getDoubleValue();
        default:            jassertfalse; break;
    }

    return 0;
}

const String var::toString() const throw()
{
    switch (type)
    {
        case voidType:
        case objectType:    return "Object 0x" + String::toHexString ((pointer_sized_int) value.objectValue);
        case intType:       return String (value.intValue);
        case boolType:      return value.boolValue ? T("1") : T("0");
        case doubleType:    return String (value.doubleValue);
        case stringType:    return *(value.stringValue);
        default:            jassertfalse; break;
    }

    return String::empty;
}

DynamicObject* var::getObject() const throw()
{
    return type == objectType ? value.objectValue : 0;
}


//==============================================================================
//==============================================================================
DynamicObject::DynamicObject()
{
}

DynamicObject::~DynamicObject()
{
}

bool DynamicObject::hasProperty (const String& propertyName) const
{
    return propertyNames.contains (propertyName, false);
}

const var DynamicObject::getProperty (const String& propertyName) const
{
    const int index = propertyNames.indexOf (propertyName, false);
    if (index >= 0)
        return *propertyValues.getUnchecked (index);

    return var();
}

void DynamicObject::setProperty (const String& propertyName, const var& newValue)
{
    const int index = propertyNames.indexOf (propertyName, false);

    if (index >= 0)
    {
        propertyValues.set (index, new var (newValue));
    }
    else
    {
        propertyNames.add (propertyName);
        propertyValues.add (new var (newValue));
    }
}

void DynamicObject::removeProperty (const String& propertyName)
{
    const int index = propertyNames.indexOf (propertyName, false);

    if (index >= 0)
    {
        propertyNames.remove (index);
        propertyValues.remove (index);
    }
}

bool DynamicObject::hasMethod (const String& methodName) const
{
    return false;
}

const var DynamicObject::invokeMethod (const String& methodName,
                                       const var* parameters,
                                       int numParameters)
{
    return var();
}

const var DynamicObject::invoke (const String& methodName)
{
    return invokeMethod (methodName, 0, 0);
}

const var DynamicObject::invoke (const String& methodName, const var& arg1)
{
    return invokeMethod (methodName, &arg1, 1);
}

const var DynamicObject::invoke (const String& methodName, const var& arg1, const var& arg2)
{
    var params[] = { arg1, arg2 };
    return invokeMethod (methodName, params, 2);
}

const var DynamicObject::invoke (const String& methodName, const var& arg1, const var& arg2, const var& arg3)
{
    var params[] = { arg1, arg2, arg3 };
    return invokeMethod (methodName, params, 3);
}

const var DynamicObject::invoke (const String& methodName, const var& arg1, const var& arg2, const var& arg3, const var& arg4)
{
    var params[] = { arg1, arg2, arg3, arg4 };
    return invokeMethod (methodName, params, 4);
}


END_JUCE_NAMESPACE
