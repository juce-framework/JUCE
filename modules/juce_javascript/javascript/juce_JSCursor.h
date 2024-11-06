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

} // namespace juce
