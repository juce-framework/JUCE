//
//    ██████ ██   ██  ██████   ██████
//   ██      ██   ██ ██    ██ ██            ** Classy Header-Only Classes **
//   ██      ███████ ██    ██ ██
//   ██      ██   ██ ██    ██ ██           https://github.com/Tracktion/choc
//    ██████ ██   ██  ██████   ██████
//
//   CHOC is (C)2022 Tracktion Corporation, and is offered under the terms of the ISC license:
//
//   Permission to use, copy, modify, and/or distribute this software for any purpose with or
//   without fee is hereby granted, provided that the above copyright notice and this permission
//   notice appear in all copies. THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
//   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
//   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR
//   CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
//   WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
//   CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

#ifndef CHOC_JAVASCRIPT_HEADER_INCLUDED
#define CHOC_JAVASCRIPT_HEADER_INCLUDED

#include <stdexcept>
#include <functional>
#include <optional>
#include "../containers/choc_Value.h"
#include "../text/choc_JSON.h"

/**
    Wrapper classes for encapsulating different javascript engines such as
    Duktape and QuickJS.

    Just use one of the context-creation functions such as
    choc::javascript::createQuickJSContext() to create a context for running
    javascript code.
*/
namespace
{
namespace choc::javascript
{
    /// This is thrown by any javascript functions that need to report an error
    struct Error  : public std::runtime_error
    {
        Error (const std::string& error) : std::runtime_error (error) {}
    };

    //==============================================================================
    /// Helper class to hold and provide access to the arguments in a javascript
    /// function callback.
    struct ArgumentList
    {
        const choc::value::Value* args = nullptr;
        size_t numArgs = 0;

        /// Returns the number of arguments
        size_t size() const noexcept                                    { return numArgs; }
        /// Returns true if there are no arguments
        bool empty() const noexcept                                     { return numArgs == 0; }

        /// Returns an argument, or a nullptr if the index is out of range.
        const choc::value::Value* operator[] (size_t index) const       { return index < numArgs ? args + index : nullptr; }

        /// Gets an argument as a primitive type (or a string).
        /// If the index is out of range or the object isn't a suitable type,
        /// then the default value provided will be returned instead.
        template <typename PrimitiveType>
        PrimitiveType get (size_t argIndex, PrimitiveType defaultValue = {}) const;

        /// Standard iterator
        const choc::value::Value* begin() const noexcept                { return args; }
        const choc::value::Value* end() const noexcept                  { return args + numArgs; }
    };

    //==============================================================================
    /**
        An execution context which you use for running javascript code.

        These are really simple to use: call one of the creation functions such
        as choc::javascript::createQuickJSContext() which will give you a shared_ptr to
        a context. Then you can add any native bindings that you need with
        registerFunction(), and call evaluate() or invoke() to execute code or call
        functions directly.

        These contexts are not thread-safe, so it's up to the caller to handle thread
        synchronisation if using a single context from multiple threads.

        They're also definitely not realtime-safe: any of the methods may allocate,
        block, or make system calls.
    */
    class Context
    {
    public:
        /// To create a Context, use a function such as choc::javascript::createQuickJSContext();
        Context() = default;
        Context (Context&&);
        Context& operator= (Context&&);
        ~Context();

        /// When parsing modules, this function is expected to take a path to a module, and
        /// to return the content of that module, or an empty optional if not found.
        using ReadModuleContentFn = std::function<std::optional<std::string>(std::string_view)>;

        //==============================================================================
        /// Evaluates the given chunk of javascript.
        /// If there are any parse errors, this will throw a choc::javascript::Error exception.
        /// If the engine supports modules, then providing a value for the resolveModuleContent
        /// function will treat the code as a module and will call your function to read the
        /// content of any dependencies.
        /// None of the methods in this class are either thread-safe or realtime-safe, so you'll
        /// need to organise your own locking if you're calling into a single Context from
        /// multiple threads.
        choc::value::Value evaluate (const std::string& javascriptCode,
                                     ReadModuleContentFn* resolveModuleContent = nullptr);

        /// Attempts to invoke a global function with no arguments.
        /// Any errors will throw a choc::javascript::Error exception.
        /// None of the methods in this class are either thread-safe or realtime-safe, so you'll
        /// need to organise your own locking if you're calling into a single Context from
        /// multiple threads.
        choc::value::Value invoke (std::string_view functionName);

        /// Attempts to invoke a global function with the arguments provided.
        /// The arguments can be primitives, strings, choc::value::ValueView or
        /// choc::value::Value types.
        /// Any errors will throw a choc::javascript::Error exception.
        /// None of the methods in this class are either thread-safe or realtime-safe, so you'll
        /// need to organise your own locking if you're calling into a single Context from
        /// multiple threads.
        template <typename... Args>
        choc::value::Value invoke (std::string_view functionName, Args&&... args);

        /// Attempts to invoke a global function with an array of arguments.
        /// The objects in the argument list can be primitives, strings, choc::value::ValueView
        /// or choc::value::Value types.
        /// Any errors will throw a choc::javascript::Error exception.
        /// None of the methods in this class are either thread-safe or realtime-safe, so you'll
        /// need to organise your own locking if you're calling into a single Context from
        /// multiple threads.
        template <typename ArgList>
        choc::value::Value invokeWithArgList (std::string_view functionName, const ArgList& args);

        /// This is the prototype for a lambda which can be bound as a javascript function.
        using NativeFunction = std::function<choc::value::Value(ArgumentList)>;

        /// Binds a lambda function to a global name so that javascript code can invoke it.
        void registerFunction (const std::string& name, NativeFunction fn);

        /// Pumps the message loop in an engine-specific way - may have no effect on some platforms.
        void pumpMessageLoop();

        //==============================================================================
        /// @internal
        struct Pimpl;
        /// @internal
        Context (std::unique_ptr<Pimpl>);

        Pimpl* getPimpl() const;

    private:
        std::unique_ptr<Pimpl> pimpl;
    };

    //==============================================================================
    /// Creates a QuickJS-based context. If you call this, then you'll need to
    /// include choc_javascript_QuickJS.h in one (and only one!) of your source files.
    Context createQuickJSContext();

    /// Creates a Duktape-based context. If you call this, then you'll need to
    /// include choc_javascript_Duktape.h in one (and only one!) of your source files.
    Context createDuktapeContext();

    /// Creates a V8-based context. If you call this, then you'll need to
    /// make sure that your project also has the V8 header folder in its
    /// search path, and that you statically link the appropriate V8 libs.
    Context createV8Context();
}
} // anonymous namespace


//==============================================================================
//        _        _           _  _
//     __| |  ___ | |_   __ _ (_)| | ___
//    / _` | / _ \| __| / _` || || |/ __|
//   | (_| ||  __/| |_ | (_| || || |\__ \ _  _  _
//    \__,_| \___| \__| \__,_||_||_||___/(_)(_)(_)
//
//   Code beyond this point is implementation detail...
//
//==============================================================================

namespace
{
namespace choc::javascript
{

template <typename PrimitiveType>
PrimitiveType ArgumentList::get (size_t index, PrimitiveType defaultValue) const
{
    if (auto a = (*this)[index])
        return a->getWithDefault<PrimitiveType> (defaultValue);

    return defaultValue;
}

struct Context::Pimpl
{
    Pimpl() = default;
    virtual ~Pimpl() = default;

    virtual void registerFunction (const std::string&, NativeFunction) = 0;
    virtual choc::value::Value evaluate (const std::string&, ReadModuleContentFn*) = 0;
    virtual void prepareForCall (std::string_view, uint32_t numArgs) = 0;
    virtual choc::value::Value performCall() = 0;
    virtual void pushObjectOrArray (const choc::value::ValueView&) = 0;
    virtual void pushArg (std::string_view) = 0;
    virtual void pushArg (int32_t) = 0;
    virtual void pushArg (int64_t) = 0;
    virtual void pushArg (uint32_t) = 0;
    virtual void pushArg (double) = 0;
    virtual void pushArg (bool) = 0;
    virtual void pumpMessageLoop() = 0;

    void pushArg (const std::string& v)   { pushArg (std::string_view (v)); }
    void pushArg (const char* v)          { pushArg (std::string_view (v)); }
    void pushArg (uint64_t v)             { pushArg (static_cast<int64_t> (v)); }
    void pushArg (float v)                { pushArg (static_cast<double> (v)); }

    void pushArg (const choc::value::ValueView& v)
    {
        if (v.isInt32())    return pushArg (v.getInt32());
        if (v.isInt64())    return pushArg (v.getInt64());
        if (v.isFloat32())  return pushArg (v.getFloat32());
        if (v.isFloat64())  return pushArg (v.getFloat64());
        if (v.isString())   return pushArg (v.getString());
        if (v.isBool())     return pushArg (v.getBool());
        if (v.isVoid())     throw Error ("Function arguments cannot be void!");

        pushObjectOrArray (v);
    }
};

inline Context::Context (std::unique_ptr<Pimpl> p) : pimpl (std::move (p)) {}
inline Context::~Context() = default;
inline Context::Context (Context&&) = default;
inline Context& Context::operator= (Context&&) = default;

inline choc::value::Value Context::invoke (std::string_view functionName)
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    pimpl->prepareForCall (functionName, 0);
    return pimpl->performCall();
}

template <typename... Args>
choc::value::Value Context::invoke (std::string_view functionName, Args&&... args)
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    pimpl->prepareForCall (functionName, sizeof...(args));
    (pimpl->pushArg (std::forward<Args> (args)), ...);
    return pimpl->performCall();
}

template <typename ArgList>
choc::value::Value Context::invokeWithArgList (std::string_view functionName, const ArgList& args)
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    pimpl->prepareForCall (functionName, static_cast<uint32_t> (args.size()));

    for (auto& arg : args)
        pimpl->pushArg (arg);

    return pimpl->performCall();
}

inline void Context::registerFunction (const std::string& name, NativeFunction fn)
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    pimpl->registerFunction (name, std::move (fn));
}

inline choc::value::Value Context::evaluate (const std::string& javascriptCode, ReadModuleContentFn* resolveModule)
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    return pimpl->evaluate (javascriptCode, resolveModule);
}

inline void Context::pumpMessageLoop()
{
    CHOC_ASSERT (pimpl != nullptr); // cannot call this on a moved-from context!
    pimpl->pumpMessageLoop();
}

Context::Pimpl* Context::getPimpl() const
{
    return pimpl.get();
}

} // namespace choc::javascript
} // anonymous namespace

#endif // CHOC_JAVASCRIPT_HEADER_INCLUDED

#ifdef CHOC_JAVASCRIPT_IMPLEMENTATION
 // The way the javascript classes work has changed: instead of
 // setting CHOC_JAVASCRIPT_IMPLEMENTATION in one of your source files, just
 // include the actual engine that you want to use, e.g. choc_javascript_QuickJS.h
 // in (only!) one of your source files, and use that to create instances of that engine.
 #error "CHOC_JAVASCRIPT_IMPLEMENTATION is deprecated"
#endif
