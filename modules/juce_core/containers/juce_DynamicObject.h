/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2016 - ROLI Ltd.

   Permission is granted to use this software under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license/

   Permission to use, copy, modify, and/or distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
   FITNESS. IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
   OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF
   USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
   TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
   OF THIS SOFTWARE.

   -----------------------------------------------------------------------------

   To release a closed-source product which uses other parts of JUCE not
   licensed under the ISC terms, commercial licenses are available: visit
   www.juce.com for more information.

  ==============================================================================
*/

#ifndef JUCE_DYNAMICOBJECT_H_INCLUDED
#define JUCE_DYNAMICOBJECT_H_INCLUDED


//==============================================================================
/**
    Represents a dynamically implemented object.

    This class is primarily intended for wrapping scripting language objects,
    but could be used for other purposes.

    An instance of a DynamicObject can be used to store named properties, and
    by subclassing hasMethod() and invokeMethod(), you can give your object
    methods.
*/
class JUCE_API  DynamicObject  : public ReferenceCountedObject
{
public:
    //==============================================================================
    DynamicObject();
    DynamicObject (const DynamicObject&);
    ~DynamicObject();

    typedef ReferenceCountedObjectPtr<DynamicObject> Ptr;

    //==============================================================================
    /** Returns true if the object has a property with this name.
        Note that if the property is actually a method, this will return false.
    */
    virtual bool hasProperty (const Identifier& propertyName) const;

    /** Returns a named property.
        This returns var() if no such property exists.
    */
    virtual const var& getProperty (const Identifier& propertyName) const;

    /** Sets a named property. */
    virtual void setProperty (const Identifier& propertyName, const var& newValue);

    /** Removes a named property. */
    virtual void removeProperty (const Identifier& propertyName);

    //==============================================================================
    /** Checks whether this object has the specified method.

        The default implementation of this just checks whether there's a property
        with this name that's actually a method, but this can be overridden for
        building objects with dynamic invocation.
    */
    virtual bool hasMethod (const Identifier& methodName) const;

    /** Invokes a named method on this object.

        The default implementation looks up the named property, and if it's a method
        call, then it invokes it.

        This method is virtual to allow more dynamic invocation to used for objects
        where the methods may not already be set as properies.
    */
    virtual var invokeMethod (Identifier methodName,
                              const var::NativeFunctionArgs& args);

    /** Adds a method to the class.

        This is basically the same as calling setProperty (methodName, (var::NativeFunction) myFunction), but
        helps to avoid accidentally invoking the wrong type of var constructor. It also makes
        the code easier to read,
    */
    void setMethod (Identifier methodName, var::NativeFunction function);

    //==============================================================================
    /** Removes all properties and methods from the object. */
    void clear();

    /** Returns the NamedValueSet that holds the object's properties. */
    NamedValueSet& getProperties() noexcept     { return properties; }

    /** Calls var::clone() on all the properties that this object contains. */
    void cloneAllProperties();

    //==============================================================================
    /** Returns a clone of this object.
        The default implementation of this method just returns a new DynamicObject
        with a (deep) copy of all of its properties. Subclasses can override this to
        implement their own custom copy routines.
    */
    virtual Ptr clone();

    //==============================================================================
    /** Writes this object to a text stream in JSON format.
        This method is used by JSON::toString and JSON::writeToStream, and you should
        never need to call it directly, but it's virtual so that custom object types
        can stringify themselves appropriately.
    */
    virtual void writeAsJSON (OutputStream&, int indentLevel, bool allOnOneLine);

private:
    //==============================================================================
    NamedValueSet properties;

   #if JUCE_CATCH_DEPRECATED_CODE_MISUSE
    // These methods have been deprecated - use var::invoke instead
    virtual void invokeMethod (const Identifier&, const var*, int) {}
   #endif

    JUCE_LEAK_DETECTOR (DynamicObject)
};



#endif   // JUCE_DYNAMICOBJECT_H_INCLUDED
