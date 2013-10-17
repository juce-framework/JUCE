/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission to use, copy, modify, and/or distribute this software for any purpose with
   or without fee is hereby granted, provided that the above copyright notice and this
   permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
   TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
   NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
   DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
   IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

   ------------------------------------------------------------------------------

   NOTE! This permissive ISC license applies ONLY to files within the juce_core module!
   All other JUCE modules are covered by a dual GPL/commercial license, so if you are
   using any other modules, be sure to check that you also comply with their license.

   For more details, visit www.juce.com

  ==============================================================================
*/


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
*/
class JavascriptEngine
{
public:
    /** Creates an instance of the engine.
        This creates a root namespace and defines some basic Object, String, Array
        and Math library methods.
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
        You can specify a maximum time for which the program is allowed to run, and
        it'll return with an error message if this time is exceeded.
    */
    var evaluate (const String& javascriptCode,
                  Result* errorMessage = nullptr);

    /** Calls a function in the root namespace, and returns the result.
        The function arguments are passed in the same format as used by native
        methods in the var class.
    */
    var callFunction (Identifier function,
                      const var::NativeFunctionArgs& args,
                      Result* errorMessage = nullptr);

    /** Adds a native object to the root namespace.
        The object passed-in is reference-counted, and will be retained by the
        engine until the engine is deleted. The name must be a simple JS identifier,
        without any dots.
    */
    void registerNativeObject (Identifier objectName, DynamicObject* object);

    /** This value indicates how long a call to one of the evaluate methods is permitted
        to run before timing-out and failing.
        The default value is a number of seconds, but you can change this to whatever value
        suits your application.
    */
    RelativeTime maximumExecutionTime;

private:
    struct RootObject;
    ReferenceCountedObjectPtr<RootObject> root;
    void prepareTimeout() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptEngine)
};
