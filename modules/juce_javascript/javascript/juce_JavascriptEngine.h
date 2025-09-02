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
    This class is a wrapper around QuickJS, an ES2023 compliant, embeddable javascript
    engine. It may not be as fast as the fancy JIT-compiled
    engines that you get in browsers, but this is an extremely compact, low-overhead javascript
    interpreter, which is integrated with the juce var and DynamicObject classes. It allows you
    to easily let the JS work with native objects defined as DynamicObject subclasses.

    To use, simply create an instance of this class and call execute() to run your code.
    Variables that the script sets can be retrieved with evaluate(), and if you need to provide
    native objects for the script to use, you can add them with registerNativeObject().

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
