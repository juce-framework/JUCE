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

namespace juce
{

/**
    A JSObject represents an owning reference to the underlying JS object, meaning it will remain
    valid even if a subsequent script execution deletes other handles to it.

    Objects of this class can be used to traverse the current object graph inside the specified
    Javascript engine.

    This is a low-level providing only operations that map directly to the underlying Javascript
    Object implementation. The JSCursor class generally provides a more convenient interface with
    functions that may fail based on the Javascript engine's current state.

    @see JSCursor
    @tags{Core}
*/
class JUCE_API  JSObject
{
public:
    /** Constructor, used internally by the JavascriptEngine implementation.

        To create a new JSObject pointing at the root object of the engine's context use
        JavascriptEngine::getRootObject().
    */
    explicit JSObject (detail::QuickJSWrapper* engine);

    /** Destructor. */
    ~JSObject();

    /** Copy constructor. */
    JSObject (const JSObject&);

    /** Move constructor. */
    JSObject (JSObject&&) noexcept;

    /** Copy assignment operator. */
    JSObject& operator= (const JSObject&);

    /** Move assignment operator. */
    JSObject& operator= (JSObject&&) noexcept;

    /** Returns a new cursor pointing to a JS object that is a property of the parent cursor's
        underlying object and has the provided name.

        You can use hasProperty() to check if such a property exists prior to the creation of this
        cursor. If no such property exists, this constructor will create a new JS Object and attach
        it to the parent under the specified name. This can be used to manipulate the object graph.
    */
    JSObject getChild (const Identifier& name) const;

    /** Returns a cursor object pointing to the property with the given name. If such property
        doesn't exist it will be created as an empty JS Object. Shorthand for getChild.
    */
    JSObject operator[] (const Identifier& name) const;

    /** Returns a new cursor object pointing to the specified element in an Array.

        You must ensure that the cursor points to an Array before calling this function.

        @see isArray
    */
    JSObject getChild (int64 index) const;

    /** Returns a new cursor object pointing to the specified element in an Array. This function is
        a shorthand for getChild (int64).

        You must ensure that the cursor points to an Array before calling this function.

        @see isArray
    */
    JSObject operator[] (int64 index) const;

    /** Returns true if the JS Object under the cursor is an Array.

        You can use getChild() or operator[]() to get a cursor to individual elements in the
        array or get() to obtain a JUCE variant wrapping all array elements.
    */
    bool isArray() const;

    /** Returns the size of the underlying JS Array.

        You must ensure that the cursor points to an Array before calling this function.

        @see isArray
    */
    int64 getSize() const;

    /** Returns true if the object under the cursor has a property with the given name. */
    bool hasProperty (const Identifier& name) const;

    /** Returns a variant with a value of the property under the given name. If no such property
        exists an undefined variant is returned.

        If this property points to an object created by JavascriptEngine::registerNativeObject(),
        then the returned variant will contain a pointer to the original object and can be acquired
        by variant::getDynamicObject().
    */
    var get() const;

    /** Adds a named property to the underlying Object with the provided value, or assigns this
        value to an existing property with this name.
    */
    void setProperty (const Identifier& name, const var& value) const;

    /** Adds a property with an integral identifier and the provided value to the underlying Object,
        or assigns the value to an existing property.

        If the underlying Object is also an Array, then the provided value will be assigned to the
        specified element of this Array, and ensure that it will have a size of at least index - 1.
    */
    void setProperty (int64 index, const var& value) const;

    /** Invokes this node as though it were a method.

        If the optional Result pointer is provided it will contain Result::ok() in case of success,
        or an error message in case an exception was thrown during evaluation.
    */
    var invokeMethod (const Identifier& methodName, Span<const var> args, Result* result = nullptr) const;

    /** Returns all properties of the current object that are own properties, i.e. not inherited. */
    NamedValueSet getProperties() const;

private:
    class Impl;

    explicit JSObject (std::unique_ptr<Impl> implIn);

    void swap (JSObject& other) noexcept;

    std::unique_ptr<Impl> impl;
};

} // namespace juce
