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
    JavascriptTests() : UnitTest ("Javascript", UnitTestCategories::javascript)
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

        beginTest ("Native functions returning objects with native functions work as expected");
        {
            JavascriptEngine temporaryEngine;

            temporaryEngine.registerNativeObject ("ObjGetter", [&]
            {
                auto* objGetter = new DynamicObject();

                objGetter->setMethod ("getObj", [&] (const auto&)
                {
                    auto* obj = new DynamicObject();

                    obj->setMethod ("getVal", [] (const auto&)
                    {
                        return 42;
                    });

                    return obj;
                });

                return objGetter;
            }());

            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("let objGetter = ObjGetter; let obj = objGetter.getObj(); obj.getVal();", &res);
            expect (res.wasOk());
            expect (static_cast<int> (val) == 42);
        }

        beginTest ("Methods of javascript objects can be called from C++");
        {
            JavascriptEngine temporaryEngine;
            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("var result = { bar: 5, foo (a) { return a + this.bar; } }; result;", &res);
            expect (res.wasOk());

            auto* obj = val.getDynamicObject();

            if (obj == nullptr)
            {
                expect (false);
                return;
            }

            expect (obj->hasMethod ("foo"));
            expect (obj->hasProperty ("bar"));

            expect (obj->getProperty ("bar") == var (5));

            const var a[] { var { 10 } };
            const auto aResult = obj->invokeMethod ("foo", { val, std::data (a), (int) std::size (a) });
            expect (aResult == var (15));

            temporaryEngine.evaluate ("result.bar = -5;", &res);
            expect (res.wasOk());

            const var b[] { var { -10 } };
            const auto bResult = obj->invokeMethod ("foo", { val, std::data (b), (int) std::size (b) });
            expect (bResult == var (-15));
        }

        beginTest ("Destructors of custom callables are called, eventually");
        {
            struct CustomCallable
            {
                explicit CustomCallable (int& instances)
                    : liveInstances (instances)
                {
                    ++liveInstances;
                }

                CustomCallable (const CustomCallable& other)
                    : liveInstances (other.liveInstances)
                {
                    ++liveInstances;
                }

                CustomCallable (CustomCallable&& other) noexcept
                    : liveInstances (other.liveInstances)
                {
                    ++liveInstances;
                }

                ~CustomCallable()
                {
                    --liveInstances;
                }

                CustomCallable& operator= (const CustomCallable&) = delete;
                CustomCallable& operator= (CustomCallable&&) noexcept = delete;

                var operator() (const var::NativeFunctionArgs&) const { return "hello world"; }

                int& liveInstances;
            };

            int methodInstances = 0;

            {
                JavascriptEngine temporaryEngine;

                temporaryEngine.registerNativeObject ("ObjGetter", [&]
                {
                    auto* objGetter = new DynamicObject();

                    objGetter->setMethod ("getObj", [&] (const auto&)
                    {
                        auto* obj = new DynamicObject;
                        obj->setMethod ("getVal", CustomCallable { methodInstances });
                        return obj;
                    });

                    return objGetter;
                }());

                auto res = juce::Result::fail ("");
                const auto value = temporaryEngine.evaluate ("ObjGetter.getObj().getVal();", &res);
                expect (res.wasOk());
                expect (value == "hello world");
            }

            expect (methodInstances == 0);
        }

        beginTest ("null and undefined return values are distinctly represented");
        {
            JavascriptEngine temporaryEngine;
            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("var result = { returnsNull (a) { return null; }, returnsUndefined (a) { 5 + 2; } }; result;", &res);
            expect (res.wasOk());

            auto* obj = val.getDynamicObject();

            if (obj == nullptr)
            {
                expect (false);
                return;
            }

            expect (obj->hasMethod ("returnsNull"));
            const auto aResult = obj->invokeMethod ("returnsNull", { val, nullptr, 0 });
            expect (aResult.isVoid());

            expect (obj->hasMethod ("returnsUndefined"));
            const auto bResult = obj->invokeMethod ("returnsUndefined", { val, nullptr, 0 });
            expect (bResult.isUndefined());
        }

        beginTest ("calling a C function that returns void is converted correctly");
        {
            int numCalls = 0;

            JavascriptEngine temporaryEngine;

            temporaryEngine.registerNativeObject ("Obj", [&]
            {
                auto* objGetter = new DynamicObject();

                objGetter->setMethod ("getObj", [&] (const auto&)
                {
                    auto* obj = new DynamicObject;

                    obj->setMethod ("mutate", [&] (const auto&)
                    {
                        ++numCalls;
                        return var{};
                    });

                    return obj;
                });

                return objGetter;
            }());

            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("let foo = Obj.getObj(); foo.mutate(); foo.mutate();", &res);
            expect (res.wasOk());

            expect (numCalls == 2);
        }

        beginTest ("Properties of registered native objects are enumerable");
        {
            auto obj = rawToUniquePtr (new DynamicObject);
            obj->setMethod ("methodA", nullptr);
            obj->setProperty ("one", 1);
            obj->setMethod ("methodB", nullptr);
            obj->setProperty ("hello", "world");
            obj->setMethod ("methodC", nullptr);
            obj->setProperty ("nested",
                              std::invoke ([]
                                           {
                                               auto result = rawToUniquePtr (new DynamicObject);
                                               result->setProperty ("present", true);
                                               return result.release();
                                           }));

            JavascriptEngine temporaryEngine;
            temporaryEngine.registerNativeObject ("obj", obj.release());

            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("JSON.stringify (obj);", &res);
            expect (res.wasOk());
            expectEquals (val.toString(), String (R"({"nested":{"present":true},"one":1,"hello":"world"})"));
        }

        beginTest ("native objects survive being passed as arguments and return values");
        {
            JavascriptEngine temporaryEngine;

            int numCalls = 0;

            auto objWithProps = rawToUniquePtr (new DynamicObject);
            objWithProps->setProperty ("one", 1);
            objWithProps->setProperty ("hello", "world");
            objWithProps->setMethod ("nativeFn", [&numCalls] (const auto&)
            {
                ++numCalls;
                return "called a native fn";
            });

            auto objWithFn = rawToUniquePtr (new DynamicObject);

            var passedToFn;
            objWithFn->setMethod ("fn", [&passedToFn] (const auto& v)
            {
                passedToFn = v.arguments[0];
                return passedToFn;
            });

            temporaryEngine.registerNativeObject ("withProps", objWithProps.release());
            temporaryEngine.registerNativeObject ("withFn", objWithFn.release());

            auto res = juce::Result::fail ("");
            const auto val = temporaryEngine.evaluate ("withFn.fn (withProps);", &res);
            expect (res.wasOk());

            for (auto& v : { val, passedToFn })
            {
                expect (v.getProperty ("one", 0) == var { 1 });
                expect (v.getProperty ("hello", "") == var { "world" });
                expect (v.call ("nativeFn") == var ("called a native fn"));
            }

            expect (numCalls == 2);
        }
    }
};

static JavascriptTests javascriptTests;

} // namespace juce
