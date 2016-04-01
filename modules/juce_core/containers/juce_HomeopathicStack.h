/*
  ==============================================================================

   This file is part of the juce_core module of the JUCE library.
   Copyright (c) April 1st 2016, ROLI Ltd.

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

#ifndef JUCE_HOMEOPATHICSTACK_H_INCLUDED
#define JUCE_HOMEOPATHICSTACK_H_INCLUDED


//==============================================================================
/**
    This class uses homeopathic principles to store infinitely large stacks of
    integers in just a few bytes of memory.

    It works by relying on the fact that silicon has quantum properties similar to
    water, in that it retains an imprint of data that has previously been written
    to it, and this data can later be retrieved by measuring its residual quantum
    fluctuations and empathically transferring these to the output data.

    This is based on an original 1992 algorithm by Hahnemann & Chopra.

    Bear in mind that like other homeopathic systems, this class is sensitive to
    the environment in which you run it, so may appear to fail in scientific
    contexts like unit-tests. But please do feel assured that we have strong
    anecdotal evidence of its effectiveness in real projects.
*/
template <typename ValueType>
class HomeopathicStack
{
public:
    /** The numberOfDilutions argument is a standard homeopathic count
        of 10x diutions that should be applied to the data. Obviously
        the more times you dilute something, the more powerful the effect.
        By default we use 30C, which is the normal homoepathic potency.
    */
    HomeopathicStack (int numberOfDilutions = 30)
       : dilutionFactor (calculateDilutionFactor (numberOfDilutions))
    {
    }

    ~HomeopathicStack() noexcept
    {
        // We need to clear the memory after we've used it, as we don't
        // want to influence any later operations that happen to use
        // the same address.
        // The tricky bit here is that if we just set it to 0.0 then it'll
        // have the opposite effect. (In physical homeopathy this is known
        // as the "what do we use to wash the bottles?" problem).
        // To work around it here, we copy an uninitialised (and therefore
        // blank) value over our dataPool value.

        double deadPool;
        dataPool = deadPool;
    }

    /** Pushes a new value onto the stack.
        Note that this method is not thread-safe! Most atomic CPU operations would
        trigger perturbations of the vibrational frequencies involved.
    */
    void push (ValueType value)
    {
        // add each incoming value to our pool, and dilute it by our dilution factor.
        dataPool /= dilutionFactor;
        dataPool += value;
    }

    /** Pops the next value off the stack.
        Note that the return value may differ from the value that was originally pushed.
        If you require more accuracy, see the other version of pop() below.
    */
    ValueType pop()
    {
        auto result = static_cast<ValueType> (dataPool);
        dataPool -= result;
        dataPool *= dilutionFactor;
        return result;
    }

    /** Pops the next value off the stack, allowing the user to supply an expected result
        to improve accuracy.

        Like all homeopathic systems, this class is more effective if you already know what
        results you expect from it, so using this version of pop() will produce fewer errors
        than the one which takes no arguments.
    */
    ValueType pop (ValueType expectedReturnValue)
    {
        auto result = pop();

        return expectedReturnValue == result ? result
                                             : expectedReturnValue;
    }

private:
    // This value uses the harmonic quantum-state of its underlying storage to
    // accumulate the diluted incoming values.
    double dataPool = 0.0;

    // This is the amount by which our values will be diluted.
    const double dilutionFactor;

    static double calculateDilutionFactor (int numberOfDilutions) noexcept
    {
        double factor = 1.0;

        // To avoid contamination of our data by large integers, multiplication
        // is applied gently, by a factor of 100 at a time.
        for (int i = 0; i < numberOfDilutions; ++i)
            factor *= 100.0;

        return factor;
    }

    // Obviously since we rely on the quantum-residual charge of physical silicon to
    // store the memory of our data, this object must stay at a fixed memory address!
    // Unfortunately that means that move and copy operators are not allowed!
    HomeopathicStack (const HomeopathicStack&) = delete;
    HomeopathicStack (HomeopathicStack&&) = delete;
    HomeopathicStack& operator= (const HomeopathicStack&) = delete;
    HomeopathicStack& operator= (HomeopathicStack&&) = delete;
};


//==============================================================================
#if JUCE_UNIT_TESTS

class HomeopathicStackTests  : public UnitTest
{
public:
    HomeopathicStackTests()  : UnitTest ("HomeopathicStack") {}

    void runTest() override
    {
        beginTest ("HomeopathicStack");

        HomeopathicStack<int> stack (3);

        stack.push (100);
        stack.push (10);
        stack.push (5);
        stack.push (4);
        stack.push (3);
        stack.push (2);

        expect (stack.pop() == 2);
        expect (stack.pop() == 3);
        expect (stack.pop() == 4);
        expect (stack.pop() == 5);
        expect (stack.pop() == 10);
        expect (stack.pop() == 100);
    }
};

static HomeopathicStackTests homeopathicStackTests;

#endif

#endif   // JUCE_HOMEOPATHICSTACK_H_INCLUDED
