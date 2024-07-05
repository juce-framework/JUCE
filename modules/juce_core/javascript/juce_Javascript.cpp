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

JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wdeprecated-copy-with-dtor",
                                     "-Wunused-but-set-variable",
                                     "-Wdeprecated",
                                     "-Wunused-function",
                                     "-Wpedantic")
JUCE_BEGIN_IGNORE_WARNINGS_MSVC (6011 6246 6255 6262 6297 6308 6323 6340 6385 6386 28182)
#include <juce_core/javascript/choc/javascript/choc_javascript_QuickJS.h>
#include <juce_core/javascript/choc/javascript/choc_javascript.h>
JUCE_END_IGNORE_WARNINGS_MSVC
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

namespace juce
{

//==============================================================================
// On Linux int64 and int64_t don't resolve to the same type and don't have C style casts between
// each other, hence we need a two-step conversion.
template <typename T>
static int64_t fromJuceInt64 (const T& convertible) { return (int64_t) (int64) convertible; }

template <typename T>
static int64_t toJuceInt64   (const T& convertible) { return (int64) (int64_t) convertible; }

template<>
struct VariantConverter<choc::value::Value>
{
    static choc::value::Value fromVar (const var& variant)
    {
        if (variant.isInt())
            return choc::value::Value { (int) variant };

        if (variant.isInt64())
            return choc::value::Value { fromJuceInt64 (variant) };

        if (variant.isDouble())
            return choc::value::Value { (double) variant };

        if (variant.isInt())
            return choc::value::Value { (int) variant };

        if (variant.isBool())
            return choc::value::Value { (bool) variant };

        if (variant.isString())
            return choc::value::Value { variant.toString().toStdString() };

        if (variant.isArray())
        {
            choc::value::Value value { choc::value::Type::createEmptyArray() };
            const auto& array = *variant.getArray();

            for (int i = 0; i < array.size(); ++i)
                value.addArrayElement (fromVar (array[i]));

            return value;
        }

        if (variant.isObject())
        {
            if (auto* dynamicObject = dynamic_cast<DynamicObject*> (variant.getObject()))
            {
                choc::value::Value value { choc::value::Type::createObject ("") };

                for (const auto& [name, prop] : dynamicObject->getProperties())
                    value.setMember (name.toString().toRawUTF8(), fromVar (prop));

                return value;
            }
        }

        if (variant.isUndefined())
            return {};

        jassertfalse;
        return {};
    }

    static var toVar (const choc::value::Value& value)
    {
        if (value.isVoid())
            return {};

        if (value.isInt32())
            return { value.getInt32() };

        if (value.isInt64())
            return { (int64) value.getInt64() };

        if (value.isFloat32())
            return { value.getFloat32() };

        if (value.isFloat64())
            return { value.getFloat64() };

        if (value.isBool())
            return { value.getBool() };

        if (value.isString())
        {
            const auto tmp = value.toString();
            return { String { CharPointer_UTF8 { tmp.c_str() } } };
        }

        if (value.isVector() || value.isArray())
        {
            var variant { Array<var>{} };

            for (uint32_t i = 0; i < value.size(); ++i)
            {
                jassert (i < (uint32_t) std::numeric_limits<int>::max());
                variant.insert ((int) i, toVar (choc::value::Value { value[i] }));
            }

            return variant;
        }

        if (value.isObject())
        {
            auto dynamicObject = std::make_unique<DynamicObject>();

            for (uint32_t i = 0; i < value.size(); ++i)
            {
                const auto& [name, type] = value.getObjectMemberAt (i);
                dynamicObject->setProperty (name, toVar (choc::value::Value { type }));
            }

            return { dynamicObject.release() };
        }

        jassertfalse;
        return {};
    }
};

//==============================================================================
namespace qjs = choc::javascript::quickjs;

static choc::value::Value quickJSToChoc (const qjs::QuickJSContext::ValuePtr& ptr, Result* result = nullptr)
{
    if (result != nullptr)
        *result = Result::ok();

    try
    {
        return ptr.toChocValue();
    }
    catch (const choc::javascript::Error& error)
    {
        if (result != nullptr)
            *result = Result::fail (error.what());
    }

    return {};
}

/* Does not release the passed in JSValue. */
static choc::value::Value quickJSToChoc (qjs::JSValue value, qjs::JSContext* ctx, Result* result = nullptr)
{
    qjs::QuickJSContext::ValuePtr valuePtr { value, ctx };
    ScopeGuard releaseJSValue { [&] { valuePtr.release(); } };

    return quickJSToChoc (valuePtr, result);
}

/*  Returns a new JSValue object with a reference count of 1. This can be passed into most QuickJS
    functions such as JS_SetPropertyStr(), which take ownership of this value and will decrement
    its reference count. When used without passing to QuickJS use ValuePtr to wrap, ensuring
    cleanup when the ValuePtr goes out of scope.
*/
static qjs::JSValue chocToQuickJS (choc::value::Value value, qjs::JSContext* ctx)
{
    auto* context = static_cast<qjs::QuickJSContext*> (qjs::JS_GetContextOpaque (ctx));
    return context->valueToJS (value).release();
}

static var quickJSToJuce (qjs::JSValueConst value, qjs::JSContext* ctx)
{
    return VariantConverter<choc::value::Value>::toVar (quickJSToChoc (value, ctx));
}

static std::vector<var> quickJSToJuce (Span<qjs::JSValueConst> args, qjs::JSContext* ctx)
{
    std::vector<var> argList;

    for (const auto& arg : args)
        argList.push_back (quickJSToJuce (arg, ctx));

    return argList;
}

/*  See chocToQuickJS() for a discussion about ownership of the returned value. */
static qjs::JSValue juceToQuickJs (var variant, qjs::JSContext* ctx)
{
    return chocToQuickJS (VariantConverter<choc::value::Value>::fromVar (variant), ctx);
}

//==============================================================================
// Any type that references the QuickJS types inside the anonymous namespace added by us requires
// this with GCC. Suppressing this warning is fine, since these classes are only visible and used
// in a single translation unit.
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsubobject-linkage")
class detail::QuickJSWrapper
{
public:
    qjs::JSContext* getQuickJSContext() const
    {
        return static_cast<qjs::QuickJSContext*> (context.getPimpl())->context;
    }

    qjs::JSRuntime* getQuickJSRuntime() const
    {
        return static_cast<qjs::QuickJSContext*> (context.getPimpl())->runtime;
    }

    choc::javascript::Context& getContext()
    {
        return context;
    }

    /*  Returning a value > 0 will interrupt the QuickJS engine.
    */
    void setInterruptHandler (std::function<int()> interruptHandlerIn)
    {
        interruptHandler = std::move (interruptHandlerIn);
        qjs::JS_SetInterruptHandler (getQuickJSRuntime(), handleInterrupt, (void*) this);
    }

private:
    static int handleInterrupt (qjs::JSRuntime*, void* opaque)
    {
        auto& self = *static_cast<QuickJSWrapper*> (opaque);

        if (self.interruptHandler != nullptr)
            return self.interruptHandler();

        return 0;
    }

    choc::javascript::Context context = choc::javascript::createQuickJSContext();
    std::function<int()> interruptHandler;
};
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

using SetterFn = qjs::JSValue (*) (qjs::JSContext* ctx,
                                   qjs::JSValueConst thisVal,
                                   qjs::JSValueConst val,
                                   int magic);
using GetterFn = qjs::JSValue (*) (qjs::JSContext* ctx, qjs::JSValueConst thisVal, int magic);

// A replacement for the JS_CGETSET_MAGIC_DEF macro in QuickJS
static qjs::JSCFunctionListEntry makeFunctionListEntry (const char* name,
                                                        GetterFn getter,
                                                        SetterFn setter,
                                                        int16_t magic)
{
    qjs::JSCFunctionListEntry e { name, JS_PROP_CONFIGURABLE, qjs::JS_DEF_CGETSET_MAGIC, magic, {} };
    e.u.getset.get.getter_magic = getter;
    e.u.getset.set.setter_magic = setter;
    return e;
}

// A replacement for the JS_UNDEFINED macro in QuickJS
static qjs::JSValue makeUndefined()
{
   #if defined(JS_NAN_BOXING) // Differentiates between 32 and 64 bit builds
    return (((uint64_t)(qjs::JS_TAG_UNDEFINED) << 32) | (uint32_t)(0));
   #else
    return qjs::JSValue (static_cast<int32_t> (0), qjs::JS_TAG_UNDEFINED);
   #endif
}

static qjs::JSClassID createClassId()
{
    // A passed in value of 0 asks QuickJS to allocate us a new unique ID. QuickJS uses global
    // variables for the bookkeeping, so it's safe to use this function to initialise globals.
    qjs::JSClassID newId = 0;
    return qjs::JS_NewClassID (&newId);
}

//==============================================================================
/*  Attached as an opaque pointer to the corresponding JS object. Its lifetime is managed by the
    QuickJS engine, which calls the finalise function when the corresponding JSValue is deleted.
*/
struct DynamicObjectWrapper
{
    DynamicObjectWrapper (detail::QuickJSWrapper& engineIn, DynamicObject* objectIn)
        : engine (engineIn), object (objectIn)
    {
        getDynamicObjects().insert (this);
    }

    int16_t getOrdinal (const Identifier& identifier)
    {
        if (const auto& it = ordinals.find (identifier); it != ordinals.end())
            return it->second;

        identifiers.emplace_back (identifier);
        const auto newSize = identifiers.size() - 1;
        jassert (newSize <= (size_t) std::numeric_limits<int16_t>::max());
        const auto newOrdinal = (int16_t) newSize;
        ordinals[identifier] = newOrdinal;
        return newOrdinal;
    }

    auto getIdentifier (int ordinal) const
    {
        jassert ((size_t) ordinal < identifiers.size());
        return identifiers[(size_t) ordinal];
    }

    NamedValueSet& getProperties() const
    {
        return object->getProperties();
    }

    static void finaliser (qjs::JSRuntime*, qjs::JSValue val)
    {
        auto* wrapper = static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque (val, getClassId()));
        wrapper->finalise();
    }

    void finalise()
    {
        getDynamicObjects().erase (this);
        delete this;
    }

    static void createClass (qjs::JSRuntime* runtime)
    {
        qjs::JSClassDef classDef {};
        classDef.class_name = "juce_DynamicObject";
        classDef.finalizer  = finaliser;
        qjs::JS_NewClass (runtime, getClassId(), &classDef);
    }

    //==============================================================================
    static choc::javascript::quickjs::JSValue callDispatcher (qjs::JSContext* ctx,
                                                              qjs::JSValueConst thisValue,
                                                              int numArgs,
                                                              qjs::JSValueConst* args,
                                                              int ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisValue, getClassId()));
        const auto argList = quickJSToJuce (Span { args, (size_t) numArgs }, ctx);
        const auto identifier = self.getIdentifier (ordinal);
        auto result = self.object->invokeMethod (identifier,
                                                 { self.object.get(), argList.data(), (int) argList.size() });
        return juceToQuickJs (result, ctx);
    }

    static qjs::JSValue setDispatcher (qjs::JSContext* ctx,
                                       qjs::JSValueConst thisVal,
                                       qjs::JSValueConst val,
                                       int ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisVal, getClassId()));
        self.object->setProperty (self.getIdentifier (ordinal), quickJSToJuce (val, ctx));

        // In case there is a problem we could return e.g. `JS_EXCEPTION` or
        // `JS_ThrowRangeError(ctx, "invalid precision");` here.
        return makeUndefined();
    }

    static qjs::JSValue getDispatcher (qjs::JSContext* ctx, qjs::JSValueConst thisVal, int ordinal)
    {
        auto& self = *static_cast<DynamicObjectWrapper*> (qjs::JS_GetOpaque2 (ctx, thisVal, getClassId()));
        return juceToQuickJs (self.object->getProperty (self.getIdentifier (ordinal)), ctx);
    }

    static qjs::JSClassID getClassId()
    {
        static qjs::JSClassID classId = createClassId();
        return classId;
    }

    static std::set<void*>& getDynamicObjects()
    {
        // Used to check if an opaque ptr attached to a JSValue is a DynamicObjectWrapper
        static std::set<void*> dynamicObjects;
        return dynamicObjects;
    }

    //==============================================================================
    detail::QuickJSWrapper& engine;
    DynamicObject::Ptr object;
    std::map<Identifier, int16_t> ordinals;
    std::vector<Identifier> identifiers;
};

//==============================================================================
class JavascriptEngine::Impl
{
public:
    using ValuePtr = qjs::QuickJSContext::ValuePtr;

    //==============================================================================
    Impl()
    {
        DynamicObjectWrapper::createClass (engine.getQuickJSRuntime());
    }

    void registerNativeObject (const Identifier& name,
                               DynamicObject* dynamicObject,
                               std::optional<qjs::JSValue> parent = std::nullopt)
    {
        auto wrapper  = std::make_unique<DynamicObjectWrapper> (engine, dynamicObject);
        auto* ctx     = engine.getQuickJSContext();
        auto jsObject = JS_NewObjectClass (ctx, (int) DynamicObjectWrapper::getClassId());
        qjs::JS_SetOpaque (jsObject, (void*) wrapper.get());

        std::vector<qjs::JSCFunctionListEntry> propertyFunctionList;

        for (const auto& [identifier, prop] : wrapper->getProperties())
        {
            auto* jsIdentifier = identifier.toString().toRawUTF8();

            if (prop.isMethod())
            {
                qjs::JS_SetPropertyStr (ctx,
                                        jsObject,
                                        jsIdentifier,
                                        JS_NewCFunctionMagic (ctx,
                                                              DynamicObjectWrapper::callDispatcher,
                                                              jsIdentifier,
                                                              0,
                                                              qjs::JS_CFUNC_generic_magic,
                                                              wrapper->getOrdinal (identifier)));
            }
            else if (prop.isObject())
            {
                if (auto* embeddedObject = prop.getDynamicObject())
                    registerNativeObject (identifier, embeddedObject, jsObject);
            }
            else
            {
                const auto entry = makeFunctionListEntry (jsIdentifier,
                                                          DynamicObjectWrapper::getDispatcher,
                                                          DynamicObjectWrapper::setDispatcher,
                                                          wrapper->getOrdinal (identifier));
                propertyFunctionList.push_back (entry);
            }
        }

        if (! propertyFunctionList.empty())
        {
            qjs::JS_SetPropertyFunctionList (ctx,
                                             jsObject,
                                             propertyFunctionList.data(),
                                             (int) propertyFunctionList.size());
        }

        const auto jsObjectName = name.toString().toRawUTF8();

        if (parent.has_value())
        {
            qjs::JS_SetPropertyStr (ctx, *parent, jsObjectName, jsObject);
        }
        else
        {
            auto globalObject = ValuePtr { qjs::JS_GetGlobalObject (ctx), ctx };
            qjs::JS_SetPropertyStr (ctx, globalObject.get(), jsObjectName, jsObject);
        }

        wrapper.release();
    }

    var evaluate (const String& code, Result* errorMessage, RelativeTime maxExecTime)
    {
        shouldStop = false;

        engine.setInterruptHandler ([this, maxExecTime, started = Time::getMillisecondCounterHiRes()]()
                                    {
                                        if (shouldStop)
                                            return 1;

                                        const auto elapsed = RelativeTime::milliseconds ((int64) (Time::getMillisecondCounterHiRes() - started));
                                        return elapsed > maxExecTime ? 1 : 0;
                                    });

        if (errorMessage != nullptr)
            *errorMessage = Result::ok();

        try
        {
            auto result = engine.getContext().evaluate (code.toStdString());
            return VariantConverter<choc::value::Value>::toVar (result);
        }
        catch (const choc::javascript::Error& error)
        {
            if (errorMessage != nullptr)
                *errorMessage = Result::fail (error.what());
        }

        return var::undefined();
    }

    Result execute (const String& code, RelativeTime maxExecTime)
    {
        auto result = Result::ok();
        evaluate (code, &result, maxExecTime);
        return result;
    }

    var callFunction (const Identifier& function, const var::NativeFunctionArgs& args, Result* errorMessage)
    {
        std::vector<choc::value::Value> argList;
        argList.reserve ((size_t) args.numArguments);

        for (int i = 0; i < args.numArguments; ++i)
            argList.emplace_back (VariantConverter<choc::value::Value>::fromVar (args.arguments[i]));

        if (errorMessage != nullptr)
            *errorMessage = Result::ok();

        try
        {
            auto& ctx = engine.getContext();
            return VariantConverter<choc::value::Value>::toVar (ctx.invokeWithArgList (function.toString().toRawUTF8(),
                                                                                       argList));
        }
        catch (const choc::javascript::Error& error)
        {
            if (errorMessage != nullptr)
                *errorMessage = Result::fail (error.what());
        }

        return var::undefined();
    }

    void stop() noexcept
    {
        shouldStop = true;
    }

    JSObject getRootObject() const
    {
        return JSObject { &engine };
    }

    const detail::QuickJSWrapper& getEngine() const
    {
        return engine;
    }

private:
    //==============================================================================
    detail::QuickJSWrapper engine;
    std::atomic<bool> shouldStop = false;
};

//==============================================================================
JavascriptEngine::JavascriptEngine() : impl (std::make_unique<Impl>())
{
}

JavascriptEngine::~JavascriptEngine() = default;

void JavascriptEngine::registerNativeObject (const Identifier& name, DynamicObject* object)
{
    impl->registerNativeObject (name, object);
}

Result JavascriptEngine::execute (const String& javascriptCode)
{
    return impl->execute (javascriptCode, maximumExecutionTime);
}

var JavascriptEngine::evaluate (const String& javascriptCode, Result* errorMessage)
{
    return impl->evaluate (javascriptCode, errorMessage, maximumExecutionTime);
}

var JavascriptEngine::callFunction (const Identifier& function,
                                    const var::NativeFunctionArgs& args,
                                    Result* errorMessage)
{
    return impl->callFunction (function, args, errorMessage);
}

void JavascriptEngine::stop() noexcept
{
    impl->stop();
}

JSObject JavascriptEngine::getRootObject() const
{
    return impl->getRootObject();
}

NamedValueSet JavascriptEngine::getRootObjectProperties() const
{
    return getRootObject().getProperties();
}

//==============================================================================
static bool hasProperty (qjs::JSContext* ctx, qjs::JSValueConst object, const char* name)
{
    qjs::JSAtom atom = JS_NewAtom (ctx, name);
    ScopeGuard freeAtom { [&] { qjs::JS_FreeAtom (ctx, atom); } };

    return JS_HasProperty (ctx, object, atom) > 0;
}

static qjs::JSValue getOrCreateProperty (qjs::JSContext* ctx, qjs::JSValueConst object, const char* name)
{
    if (! hasProperty (ctx, object, name))
        qjs::JS_SetPropertyStr (ctx, object, name, JS_NewObject (ctx));

    return qjs::JS_GetPropertyStr (ctx, object, name);
}

static uint32_t toUint32 (int64 value)
{
    jassert (0 <= value && value <= (int64) std::numeric_limits<uint32_t>::max());
    return (uint32_t) value;
}

//==============================================================================
JUCE_BEGIN_IGNORE_WARNINGS_GCC_LIKE ("-Wsubobject-linkage")
struct JSFunctionArguments
{
    explicit JSFunctionArguments (qjs::JSContext* contextIn) : context (contextIn)
    {
    }

    ~JSFunctionArguments()
    {
        for (const auto& value : values)
            qjs::JS_FreeValue (context, value);
    }

    void add (const var& arg)
    {
        values.push_back (juceToQuickJs (arg, context));
    }

    qjs::JSValue* getArguments()
    {
        return values.data();
    }

    int getSize() const
    {
        return (int) values.size();
    }

private:
    qjs::JSContext* context;
    std::vector<qjs::JSValue> values;

    JUCE_DECLARE_NON_COPYABLE (JSFunctionArguments)
    JUCE_DECLARE_NON_MOVEABLE (JSFunctionArguments)
};

//==============================================================================
class JSObject::Impl
{
public:
    using ValuePtr = qjs::QuickJSContext::ValuePtr;

    explicit Impl (const detail::QuickJSWrapper* engineIn)
        : Impl (engineIn,
                { qjs::JS_GetGlobalObject (engineIn->getQuickJSContext()), engineIn->getQuickJSContext() })
    {
    }

    Impl (const Impl& other)
        : Impl (other.engine,
                { qjs::JS_DupValue (other.engine->getQuickJSContext(), other.valuePtr.get()),
                  other.engine->getQuickJSContext() })
    {
    }

    std::unique_ptr<Impl> getChild (const Identifier& prop) const
    {
        return rawToUniquePtr (new Impl (engine, { getOrCreateProperty (engine->getQuickJSContext(),
                                                                        valuePtr.get(),
                                                                        prop.toString().toRawUTF8()),
                                                   engine->getQuickJSContext() }));
    }

    std::unique_ptr<Impl> getChild (int64 index) const
    {
        jassert (isArray());
        return rawToUniquePtr (new Impl (engine, valuePtr[toUint32 (index)]));
    }

    bool hasProperty (const Identifier& name) const
    {
        return juce::hasProperty (engine->getQuickJSContext(), valuePtr.get(), name.toString().toRawUTF8());
    }

    void setProperty (const Identifier& name, const var& value) const
    {
        auto* ctx = engine->getQuickJSContext();

        qjs::JS_SetPropertyStr (ctx, valuePtr.get(), name.toString().toRawUTF8(), juceToQuickJs (value, ctx));
    }

    void setProperty (int64 index, const var& value) const
    {
        auto* ctx = engine->getQuickJSContext();

        qjs::JS_SetPropertyInt64 (ctx, valuePtr.get(), index, juceToQuickJs (value, ctx));
    }

    var get() const
    {
        if (auto* opaque = qjs::JS_GetOpaque (valuePtr.get(), DynamicObjectWrapper::getClassId()))
            if (DynamicObjectWrapper::getDynamicObjects().count (opaque) != 0)
                return { static_cast<DynamicObjectWrapper*> (opaque)->object.get() };

        return quickJSToJuce (valuePtr.get(), engine->getQuickJSContext());
    }

    var invokeMethod (const Identifier& methodName, Span<const var> args, Result* result) const
    {
        if (! hasProperty (methodName))
        {
            jassertfalse;
            return {};
        }

        auto* ctx = engine->getQuickJSContext();
        const auto methodAtom = JS_NewAtom (ctx, methodName.toString().toRawUTF8());
        ScopeGuard scope { [&] { qjs::JS_FreeAtom (ctx, methodAtom); } };

        JSFunctionArguments arguments { ctx };

        for (const auto& arg : args)
            arguments.add (arg);

        ValuePtr returnVal { qjs::JS_Invoke (ctx,
                                             valuePtr.get(),
                                             methodAtom,
                                             arguments.getSize(),
                                             arguments.getArguments()),
                             ctx };

        return VariantConverter<choc::value::Value>::toVar (quickJSToChoc (returnVal, result));
    }

    NamedValueSet getProperties() const
    {
        NamedValueSet result;

        auto* ctx = engine->getQuickJSContext();
        auto names = ValuePtr { qjs::JS_GetOwnPropertyNames2 (ctx,
                                                              valuePtr.get(),
                                                              qjs::JS_GPN_ENUM_ONLY | qjs::JS_GPN_STRING_MASK,
                                                              qjs::JS_ITERATOR_KIND_KEY),
                                ctx };


        if (const auto propertyNames = quickJSToChoc (names); propertyNames.isArray())
        {
            for (const auto& name : propertyNames)
            {
                if (name.isString())
                {
                    const Identifier prop { name.toString() };
                    result.set (prop, getChild (prop)->get());
                }
            }
        }

        return result;
    }

    bool isArray() const
    {
        return qjs::JS_IsArray (engine->getQuickJSContext(), valuePtr.get());
    }

    int64 getSize() const
    {
        if (! isArray())
        {
            jassertfalse;
            return 0;
        }

        auto lengthProp = valuePtr["length"];
        uint32_t length = 0;
        qjs::JS_ToUint32 (engine->getQuickJSContext(), &length, lengthProp.get());
        return (int64) length;
    }

private:
    Impl (const detail::QuickJSWrapper* e, ValuePtr&& ptr)
        : engine (e), valuePtr (std::move (ptr))
    {
    }

    const detail::QuickJSWrapper* engine = nullptr;
    ValuePtr valuePtr;
};
JUCE_END_IGNORE_WARNINGS_GCC_LIKE

JSObject::JSObject (const detail::QuickJSWrapper* engine)
    : impl (new Impl (engine))
{
}

JSObject::JSObject (std::unique_ptr<Impl> implIn)
    : impl (std::move (implIn))
{
}

JSObject::JSObject (const JSObject& other)
    : impl (new Impl (*other.impl))
{
}

JSObject::~JSObject() = default;

JSObject::JSObject (JSObject&&) noexcept = default;

JSObject& JSObject::operator= (const JSObject& other)
{
    JSObject { other }.swap (*this);
    return *this;
}

JSObject& JSObject::operator= (JSObject&& other) noexcept = default;

JSObject JSObject::getChild (const Identifier& name) const
{
    return JSObject { impl->getChild (name) };
}

JSObject JSObject::operator[] (const Identifier& name) const
{
    return getChild (name);
}

bool JSObject::isArray() const
{
    return impl->isArray();
}

int64 JSObject::getSize() const
{
    return impl->getSize();
}

JSObject JSObject::getChild (int64 index) const
{
    jassert (isArray());
    return JSObject { impl->getChild (index) };
}

JSObject JSObject::operator[] (int64 index) const
{
    return getChild (index);
}

bool JSObject::hasProperty (const Identifier& name) const
{
    return impl->hasProperty (name);
}

var JSObject::get() const
{
    return impl->get();
}

void JSObject::setProperty (const Identifier& name, const var& value) const
{
    impl->setProperty (name, value);
}

void JSObject::setProperty (int64 index, const var& value) const
{
    impl->setProperty (index, value);
}

var JSObject::invokeMethod (const Identifier& methodName,
                            Span<const var> args,
                            Result* result) const
{
    return impl->invokeMethod (methodName, args, result);
}

NamedValueSet JSObject::getProperties() const
{
    return impl->getProperties();
}

void JSObject::swap (JSObject& other) noexcept
{
    std::swap (impl, other.impl);
}

//==============================================================================
JSCursor::JSCursor (JSObject rootIn) : root (std::move (rootIn))
{
}

var JSCursor::get() const
{
    if (const auto resolved = getFullResolution())
        return resolved->get();

    return var::undefined();
}

void JSCursor::set (const var& value) const
{
    const auto resolved = getPartialResolution();

    if (! resolved.has_value())
    {
        jassertfalse;  // Can't resolve an Object to change along the path stored in the cursor
        return;
    }

    const auto& [object, property] = *resolved;

    if (! property.has_value())
    {
        jassertfalse;  // Can't set the value of the root Object
        return;
    }

    if (auto* prop = std::get_if<Identifier> (&(*property)))
    {
        object.setProperty (*prop, value);
        return;
    }

    if (auto* prop = std::get_if<int64> (&(*property)))
    {
        object.setProperty (*prop, value);
        return;
    }
}

JSCursor JSCursor::getChild (const Identifier& name) const
{
    auto copy = *this;
    copy.path.emplace_back (name);
    return copy;
}

JSCursor JSCursor::operator[] (const Identifier& name) const
{
    return getChild (name);
}

JSCursor JSCursor::getChild (int64 index) const
{
    auto copy = *this;
    copy.path.emplace_back (index);
    return copy;
}

JSCursor JSCursor::operator[] (int64 index) const
{
    return getChild (index);
}

JSObject JSCursor::getOrCreateObject() const
{
    const auto resolved = getPartialResolution();
    jassert (resolved.has_value());

    const auto& [object, property] = *resolved;

    if (! property.has_value())
        return object;

    auto* integerValue = std::get_if<int64> (&(*property));

    jassert   (integerValue == nullptr
            || (object.isArray() && (*integerValue) < object.getSize()));

    if (integerValue != nullptr)
        return object[*integerValue];

    auto* prop = std::get_if<Identifier> (&(*property));
    jassert(prop != nullptr);
    return object[*prop];
}

bool JSCursor::isValid() const
{
    return getPartialResolution().has_value();
}

bool JSCursor::isArray() const
{
    if (auto resolved = getFullResolution())
        return resolved->isArray();

    return false;
}

var JSCursor::invoke (Span<const var> args, Result* result) const
{
    const auto resolved = getPartialResolution();

    if (! resolved.has_value())
    {
        jassertfalse;
        return {};
    }

    const auto& [object, property] = *resolved;
    if (! property.has_value())
    {
        jassertfalse;
        return {};
    }

    return object.invokeMethod (*std::get_if<Identifier> (&(*property)), args, result);
}

std::optional<JSObject> JSCursor::resolve (JSObject object, Property property)
{
    if (auto* index = std::get_if<int64> (&property))
    {
        if (! object.isArray())
            return std::nullopt;

        if (! (*index < object.getSize()))
            return std::nullopt;

        return object[*index];
    }

    if (auto* key = std::get_if<Identifier> (&property))
    {
        if (! object.hasProperty (*key))
            return std::nullopt;

        return object[*key];
    }

    jassertfalse;
    return std::nullopt;
}

std::optional<JSCursor::PartialResolution> JSCursor::getPartialResolution() const
{
    auto object = root;

    for (int i = 0, iEnd = (int) path.size() - 1; i < iEnd; ++i)
    {
        const auto& property = path[(size_t) i];
        auto objectOpt = resolve (object, property);

        if (! objectOpt.has_value())
            return std::nullopt;

        object = *objectOpt;
    }

    return std::make_optional<PartialResolution> (std::move (object),
                                                  path.empty() ? std::nullopt
                                                               : std::make_optional (path.back()));
}

std::optional<JSObject> JSCursor::getFullResolution() const
{
    if (auto partiallyResolved = getPartialResolution())
    {
        if (! partiallyResolved->second.has_value())
            return partiallyResolved->first;

        return resolve (partiallyResolved->first, *(partiallyResolved->second));
    }

    return std::nullopt;
}

//==============================================================================
#if JUCE_UNIT_TESTS

static constexpr const char javascriptTestSource[] = R"x(
var testObject = new Object();
testObject.value = 9;
testObject.add = function(a, b)
                 {
                     return a + b;
                 };
var array = [1.1, 1.9, -1.25, -1.9];
)x";

static constexpr const char accessNewObject[] = R"x(
var ref = newObject;
)x";

static constexpr const char createAccumulator[] = R"x(
class CommunicationsObject
{
    constructor()
    {
        this.value = 0;
    }
}

class DataAccumulator
{
    constructor()
    {
        this.commObject = new CommunicationsObject();
        this.sum = 0;
    }

    getCommObject()
    {
        return this.commObject;
    }

    accumulate()
    {
        this.sum += this.commObject.value;
        this.commObject.value = 0;
        return this.sum;
    }
}

var accumulator = new DataAccumulator();
var commObject = accumulator.getCommObject();
)x";

static constexpr const char replaceObjectAtCommHandleLocation[] = R"x(
var commObject = new CommunicationsObject();
)x";


class JavascriptTests  : public UnitTest
{
public:
    JavascriptTests() : UnitTest ("Javascript", UnitTestCategories::gui)
    {
    }

    void runTest() override
    {
        JavascriptEngine engine;
        engine.maximumExecutionTime = RelativeTime::seconds (5);

        beginTest ("Basic evaluations");
        {
            auto result = Result::ok();

            auto value = engine.evaluate ("[]", &result);
            expect (result.wasOk() && value == var { Array<var>{} }, "An empty array literal should evaluate correctly");
        }

        //==============================================================================
        engine.evaluate (javascriptTestSource);

        beginTest ("JSCursor::invokeMethod");
        {
            JSCursor root { engine.getRootObject() };
            const auto result = root["testObject"]["add"] (Span { std::initializer_list<var> { 5, 2 } });
            expect (result.isDouble());
            expect (exactlyEqual ((double) result, 7.0));
        }

        beginTest ("JSCursor Array access");
        {
            JSCursor root { engine.getRootObject() };
            expect (root["array"].isArray());
            expectEquals ((double) root["array"][2].get(), -1.25);
        }

        beginTest ("JSObjectCursor references");
        {
            auto rootObject = engine.getRootObject();
            rootObject["child"]["value"];

            JSCursor root { rootObject };
            auto child = root["child"];
            auto value = child["value"];
            value.set (9);

            auto directReference = value;
            directReference.set (10);
            expectEquals ((double) value.get(), 10.0);

            auto indirectReference = child["value"];
            indirectReference.set (11);
            expectEquals ((double) value.get(), 11.0);

            auto indirectReference2 = root["child"]["value"];
            indirectReference2.set (12);
            expectEquals ((double) value.get(), 12.0);
        }

        //==============================================================================
        beginTest ("The object referenced by the cursor should be accessible from Javascript");
        {
            auto rooObject = engine.getRootObject();
            auto newObject = rooObject["newObject"];

            auto result = Result::ok();
            engine.evaluate (accessNewObject, &result);
            expect (result.wasOk(), "Failed to access newObject: " + result.getErrorMessage());
        }

        beginTest ("The object referenced by the cursor shouldn't disappear/change");
        {
            engine.execute (createAccumulator);
            JSCursor rootCursor { engine.getRootObject() };
            auto commObjectCursor = rootCursor["commObject"];
            commObjectCursor["value"].set (5);
            auto accumulatorCursor = rootCursor["accumulator"];

            // The Accumulator and our cursor refer to the same object, through which they can
            // communicate.
            expectEquals ((int) accumulatorCursor["accumulate"]({}), 5);

            // A cursor contains an owning reference to the Object passed into its constructor. We
            // can bind a cursor to the Object at the current location by reseating it. Without this
            // step the test would fail.
            commObjectCursor = JSCursor { commObjectCursor.getOrCreateObject() };

            // This changes the object under the previous location.
            engine.execute (replaceObjectAtCommHandleLocation);
            commObjectCursor["value"].set (2);

            expectEquals ((int) accumulatorCursor["accumulate"]({}), 7,
                          "We aren't referring to the Accumulator's object anymore");
        }

        beginTest ("A JSCursor instance can be used to retrieve whatever value is at a given location");
        {
            engine.execute ("var path = new Object();"
                            "path.to  = new Object();"
                            "path.to.location = 5;");

            auto cursor = JSCursor { engine.getRootObject() }["path"]["to"]["location"];

            expectEquals ((int) cursor.get(), 5);

            engine.execute ("path.to = new Object();"
                            "path.to.location = 6;");

            expectEquals ((int) cursor.get(), 6);
        }
    }
};

static JavascriptTests javascriptTests;

#endif

} // namespace juce
