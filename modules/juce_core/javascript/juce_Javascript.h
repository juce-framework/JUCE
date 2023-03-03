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
    explicit JSObject (const detail::QuickJSWrapper* engine);

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

/**
    A high-level wrapper around an owning root JSObject and a hierarchical path relative to it.

    It can be used to query and manipulate the location relative to the root JSObject in the
    Javascript Object graph. A cursor only maintains ownership of the root Object. So as long as a
    cursor points at the root it will always remain in a valid state, and isValid will return true.

    Using getChild you can add elements to the cursor's relative path. You need to ensure that the
    cursor is in a valid state when calling get or set in such cases. You can use the isValid
    function to determine if the cursor currently points to a reachable location.

    @tags{Core}
*/
class JUCE_API  JSCursor
{
public:
    /** Creates a JSCursor that points to the provided root object and also participates in its
        ownership. This guarantees that this root object will remain valid for the lifetime of
        this cursor.

        Child JSCursors created by getChild() will contain this same root object and each will
        further ensure that this root remains valid through reference counting.

        While the validity of the root is ensured through shared ownership, the JSCursor itself is
        not guaranteed to be valid, unless its also pointing directly at the root.

        @see isValid
    */
    explicit JSCursor (JSObject root);

    /** Returns an owning reference to the Javascript Object at the cursor's location. If there is
        no Object at the location but the cursor is valid, a new Object will be created.

        You must only call this function on a valid JSCursor.

        By creating an owning reference, you can create a new JSCursor object that owns the
        underlying object and is guaranteed to remain in a valid state e.g.

        @code
        JSCursor rootCursor { engine.getRootObject() };
        auto nonOwningCursor = rootCursor["path"]["to"]["object"];

        jassert (nonOwningCursor.isValid());

        JSCursor owningCursor { nonOwningCursor.getOrCreateObject(); };
        engine.execute (arbitraryScript);

        // owningCursor is guaranteed to remain valid even after subsequent script evaluations
        jassert (owningCursor.isValid());
        @endcode

        @see isValid
    */
    JSObject getOrCreateObject() const;

    /** Returns the value corresponding to the Object that the cursor points to. If there is no
        Object at the cursor's location var::undefined() is returned.

        This function is safe to call for invalid cursors.

        @see isValid
    */
    var get() const;

    /** Sets the Object under the cursor's location to the specified value.

        You must only call this function for valid cursors.

        @see isValid
    */
    void set (const var& value) const;

    /** Invokes this node as though it were a method. If the optional Result pointer is provided it
        will contain Result::ok() in case of success, or an error message in case an exception was
        thrown during evaluation.

        You must only call this function for valid cursors.
    */
    var invoke (Span<const var> args, Result* result = nullptr) const;

    /** Equivalent to invoke(). */
    var operator() (Span<const var> args, Result* result = nullptr) const
    {
        return invoke (args, result);
    }

    /** Returns a new cursor that has the same root Object as the parent and has the name parameter
        appended to the cursor's location.

        If the new path points to a location unreachable from the root, the resulting JSCursor
        object will be invalid. This however can change due to subsequent script executions.
    */
    JSCursor getChild (const Identifier& name) const;

    /** Returns a new cursor that has the same root Object as the parent and has the name parameter
        appended to the cursor's location.

        If the new path points to a location unreachable from the root, the resulting JSCursor
        object will be invalid. This however can change due to subsequent script executions.
        Shorthand for getChild.
    */
    JSCursor operator[] (const Identifier& name) const;

    /** Returns a new cursor that has the same root Object as the parent and has the index parameter
        appended to the cursor's location. This overload will create a path that indexes into an
        Array.

        If the new path points to a location unreachable from the root, the resulting JSCursor
        object will be invalid. This however can change due to subsequent script executions.
    */
    JSCursor getChild (int64 index) const;

    /** Returns a new cursor that has the same root Object as the parent and has the index parameter
        appended to the cursor's location. This overload will create a path that indexes into an
        Array.

        If the new path points to a location unreachable from the root, the resulting JSCursor
        object will be invalid. This however can change due to subsequent script executions.
        Shorthand for getChild.
    */
    JSCursor operator[] (int64 index) const;

    /** Returns true if the location of the cursor is reachable from the cursor's JSObject root.
        This means it is safe to call set on this JSCursor and the location will then point to an
        Object corresponding to the supplied value.

        It isn't guaranteed that there is already an Object at this location, in which case calling
        get will return var::undefined().
    */
    bool isValid() const;

    /** Returns true if there is an Array under the cursor's location.

        It is safe to call this function on an invalid cursor.
    */
    bool isArray() const;

private:
    using Property = std::variant<Identifier, int64>;
    using PartialResolution = std::pair<JSObject, std::optional<Property>>;

    static std::optional<JSObject> resolve (JSObject reference, Property property);

    // Resolves the path to the second to last element. By taking ownership (creating an object for)
    // of the second to last element, the result of a successful partial resolution can be used to
    // construct the last element if it doesn't yet exist.
    std::optional<PartialResolution> getPartialResolution() const;

    // Fully resolves the path and takes ownership of the object that was specified by it.
    std::optional<JSObject> getFullResolution() const;

    JSObject root;
    std::vector<Property> path;
};

//==============================================================================
/**
    A simple javascript interpreter!

    It's not fully standards-compliant, and won't be as fast as the fancy JIT-compiled
    engines that you get in browsers, but this is an extremely compact, low-overhead javascript
    interpreter, which is integrated with the juce var and DynamicObject classes. If you need
    a few simple bits of scripting in your app, and want to be able to easily let the JS
    work with native objects defined as DynamicObject subclasses, then this might do the job.

    To use, simply create an instance of this class and call execute() to run your code.
    Variables that the script sets can be retrieved with evaluate(), and if you need to provide
    native objects for the script to use, you can add them with registerNativeObject().

    One caveat: Because the values and objects that the engine works with are DynamicObject
    and var objects, they use reference-counting rather than garbage-collection, so if your
    script creates complex connections between objects, you run the risk of creating cyclic
    dependencies and hence leaking.

    @tags{Core}
*/
class JUCE_API  JavascriptEngine  final
{
public:
    /** Creates an instance of the engine.
    */
    JavascriptEngine();

    /** Destructor. */
    ~JavascriptEngine();

    /** Attempts to parse and run a block of javascript code.
        If there's a parse or execution error, the error description is returned in
        the result.
        You can specify a maximum time for which the program is allowed to run, and
        it'll return with an error message if this time is exceeded.
    */
    Result execute (const String& javascriptCode);

    /** Attempts to parse and run a javascript expression, and returns the result.
        If there's a syntax error, or the expression can't be evaluated, the return value
        will be var::undefined(). The errorMessage parameter gives you a way to find out
        any parsing errors.
        If the expression is successfully evaluated but yields no result the return value
        will be a void var.
        You can specify a maximum time for which the program is allowed to run, and
        it'll return with an error message if this time is exceeded.
    */
    var evaluate (const String& javascriptCode,
                  Result* errorMessage = nullptr);

    /** Calls a function in the root namespace, and returns the result.
        The function arguments are passed in the same format as used by native
        methods in the var class.
    */
    var callFunction (const Identifier& function,
                      const var::NativeFunctionArgs& args,
                      Result* errorMessage = nullptr);

    /** Adds a native object to the root namespace.
        The object passed-in is reference-counted, and will be retained by the
        engine until the engine is deleted. The name must be a simple JS identifier,
        without any dots.
    */
    void registerNativeObject (const Identifier& objectName, DynamicObject* object);

    /** This value indicates how long a call to one of the evaluate methods is permitted
        to run before timing-out and failing.
        The default value is a number of seconds, but you can change this to whatever value
        suits your application.
    */
    RelativeTime maximumExecutionTime;

    /** When called from another thread, causes the interpreter to time-out as soon as possible */
    void stop() noexcept;

    /** Returns the object from which all Javascript objects are reachable in the engine's context.
    */
    JSObject getRootObject() const;

    /** Provides access to the set of properties of the root namespace object. */
    NamedValueSet getRootObjectProperties() const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptEngine)
    JUCE_DECLARE_NON_MOVEABLE (JavascriptEngine)
};

} // namespace juce
