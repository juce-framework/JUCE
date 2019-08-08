

namespace juce
{

template <typename SampleType, typename CoefficientType, typename InterpolatorType>
class ResamplerBase
{
public:
    ResamplerBase()  noexcept
    {
        // In case a complex SampleType is specified but no real-valued CoefficientType the CoefficientType will have
        // the same type as SampleType. However, the CoefficientType needs to be a real-valued float.
        static_assert (std::is_floating_point<CoefficientType>::value, "The coefficient must be a real valued floating point type");
        reset();
    }
    ~ResamplerBase() noexcept {};

    /** Resets the state of the interpolator.
        Call this when there's a break in the continuity of the input data stream.
    */
    void reset() noexcept
    {
        subSamplePos = 1.0;

        for (auto& s : lastInputSamples)
            s = SampleType (0);
    }

    /** Resamples a stream of samples.

    @param speedRatio       the number of input samples to use for each output sample
    @param inputSamples     the source data to read from. This must contain at
                            least (speedRatio * numOutputSamplesToProduce) samples.
    @param outputSamples    the buffer to write the results into
    @param numOutputSamplesToProduce    the number of output samples that should be created

    @returns the actual number of input samples that were used
*/
    int process (double speedRatio,
            const SampleType* inputSamples,
            SampleType* outputSamples,
            int numOutputSamplesToProduce) noexcept
    {
        return interpolate (lastInputSamples, subSamplePos, speedRatio, inputSamples, outputSamples, numOutputSamplesToProduce);
    }


    /** Resamples a stream of samples.

        @param speedRatio       the number of input samples to use for each output sample
        @param inputSamples     the source data to read from. This must contain at
                                least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples    the buffer to write the results into
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param available        the number of available input samples. If it needs more samples
                                than available, it either wraps back for wrapAround samples, or
                                it feeds zeroes
        @param wrapAround       if the stream exceeds available samples, it wraps back for
                                wrapAround samples. If wrapAround is set to 0, it will feed zeroes.

        @returns the actual number of input samples that were used
    */
    int process (double speedRatio,
            const SampleType* inputSamples,
            SampleType* outputSamples,
            int numOutputSamplesToProduce,
            int available,
            int wrapAround) noexcept
    {
        return interpolate (lastInputSamples, subSamplePos, speedRatio, inputSamples, outputSamples, numOutputSamplesToProduce, available, wrapAround);
    }

    /** Resamples a stream of samples, adding the results to the output data
        with a gain.

        @param speedRatio       the number of input samples to use for each output sample
        @param inputSamples     the source data to read from. This must contain at
                                least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples    the buffer to write the results to - the result values will be added
                                to any pre-existing data in this buffer after being multiplied by
                                the gain factor
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param gain             a gain factor to multiply the resulting samples by before
                                adding them to the destination buffer

        @returns the actual number of input samples that were used
    */
    int processAdding (double speedRatio,
            const SampleType* inputSamples,
            SampleType* outputSamples,
            int numOutputSamplesToProduce,
            CoefficientType gain) noexcept
    {
        return interpolateAdding (lastInputSamples, subSamplePos, speedRatio, inputSamples, outputSamples, numOutputSamplesToProduce, gain);
    }

    /** Resamples a stream of samples, adding the results to the output data
        with a gain.

        @param speedRatio       the number of input samples to use for each output sample
        @param inputSamples     the source data to read from. This must contain at
                                least (speedRatio * numOutputSamplesToProduce) samples.
        @param outputSamples    the buffer to write the results to - the result values will be added
                                to any pre-existing data in this buffer after being multiplied by
                                the gain factor
        @param numOutputSamplesToProduce    the number of output samples that should be created
        @param available        the number of available input samples. If it needs more samples
                                than available, it either wraps back for wrapAround samples, or
                                it feeds zeroes
        @param wrapAround       if the stream exceeds available samples, it wraps back for
                                wrapAround samples. If wrapAround is set to 0, it will feed zeroes.
        @param gain             a gain factor to multiply the resulting samples by before
                                adding them to the destination buffer

        @returns the actual number of input samples that were used
    */
    int processAdding (double speedRatio,
            const SampleType* inputSamples,
            SampleType* outputSamples,
            int numOutputSamplesToProduce,
            int available,
            int wrapAround,
            CoefficientType gain) noexcept
    {
        return interpolateAdding (lastInputSamples, subSamplePos, speedRatio, inputSamples, outputSamples, numOutputSamplesToProduce, available, wrapAround, gain);
    }

protected:
    SampleType lastInputSamples[5];
    double subSamplePos;

    static forcedinline void pushInterpolationSample (SampleType* lastInputSamples, SampleType newValue) noexcept
    {
        lastInputSamples[4] = lastInputSamples[3];
        lastInputSamples[3] = lastInputSamples[2];
        lastInputSamples[2] = lastInputSamples[1];
        lastInputSamples[1] = lastInputSamples[0];
        lastInputSamples[0] = newValue;
    }

    static forcedinline void pushInterpolationSamples (SampleType* lastInputSamples, const SampleType* input, int numOut) noexcept
    {
        if (numOut >= 5)
        {
            for (int i = 0; i < 5; ++i)
                lastInputSamples[i] = input[--numOut];
        }
        else
        {
            for (int i = 0; i < numOut; ++i)
                pushInterpolationSample (lastInputSamples, input[i]);
        }
    }

    static forcedinline void pushInterpolationSamples (SampleType* lastInputSamples, const SampleType* input,
            int numOut, int available, int wrapAround) noexcept
    {
        if (numOut >= 5)
        {
            if (available >= 5)
            {
                for (int i = 0; i < 5; ++i)
                    lastInputSamples[i] = input[--numOut];
            }
            else
            {
                for (int i = 0; i < available; ++i)
                    lastInputSamples[i] = input[--numOut];

                if (wrapAround > 0)
                {
                    numOut -= wrapAround;

                    for (int i = available; i < 5; ++i)
                        lastInputSamples[i] = input[--numOut];
                }
                else
                {
                    for (int i = available; i < 5; ++i)
                        lastInputSamples[i] = 0.0f;
                }
            }
        }
        else
        {
            if (numOut > available)
            {
                for (int i = 0; i < available; ++i)
                    pushInterpolationSample (lastInputSamples, input[i]);

                if (wrapAround > 0)
                {
                    for (int i = 0; i < numOut - available; ++i)
                        pushInterpolationSample (lastInputSamples, input[i + available - wrapAround]);
                }
                else
                {
                    for (int i = 0; i < numOut - available; ++i)
                        pushInterpolationSample (lastInputSamples, 0);
                }
            }
            else
            {
                for (int i = 0; i < numOut; ++i)
                    pushInterpolationSample (lastInputSamples, input[i]);
            }
        }
    }

    static int interpolate (SampleType* lastInputSamples, double& subSamplePos, double actualRatio,
            const SampleType* in, SampleType* out, int numOut) noexcept
    {
        auto pos = subSamplePos;

        if (actualRatio == 1.0 && pos == 1.0)
        {
            memcpy (out, in, (size_t) numOut * sizeof (SampleType));
            pushInterpolationSamples (lastInputSamples, in, numOut);
            return numOut;
        }

        int numUsed = 0;

        while (numOut > 0)
        {
            while (pos >= 1.0)
            {
                pushInterpolationSample (lastInputSamples, in[numUsed++]);
                pos -= 1.0;
            }

            *out++ = InterpolatorType::valueAtOffset (lastInputSamples, (CoefficientType) pos);
            pos += actualRatio;
            --numOut;
        }

        subSamplePos = pos;
        return numUsed;
    }

    static int interpolate (SampleType* lastInputSamples, double& subSamplePos, double actualRatio,
            const SampleType* in, SampleType* out, int numOut, int available, int wrap) noexcept
    {
        if (actualRatio == 1.0)
        {
            if (available >= numOut)
            {
                memcpy (out, in, (size_t) numOut * sizeof (SampleType));
                pushInterpolationSamples (lastInputSamples, in, numOut, available, wrap);
            }
            else
            {
                memcpy (out, in, (size_t) available * sizeof (SampleType));
                pushInterpolationSamples (lastInputSamples, in, numOut, available, wrap);

                if (wrap > 0)
                {
                    memcpy (out + available, in + available - wrap, (size_t) (numOut - available) * sizeof (SampleType));
                    pushInterpolationSamples (lastInputSamples, in, numOut, available, wrap);
                }
                else
                {
                    for (int i = 0; i < numOut - available; ++i)
                        pushInterpolationSample (lastInputSamples, 0);
                }
            }

            return numOut;
        }

        auto originalIn = in;
        auto pos = subSamplePos;
        bool exceeded = false;

        if (actualRatio < 1.0)
        {
            for (int i = numOut; --i >= 0;)
            {
                if (pos >= 1.0)
                {
                    if (exceeded)
                    {
                        pushInterpolationSample (lastInputSamples, 0);
                    }
                    else
                    {
                        pushInterpolationSample (lastInputSamples, *in++);

                        if (--available <= 0)
                        {
                            if (wrap > 0)
                            {
                                in -= wrap;
                                available += wrap;
                            }
                            else
                            {
                                exceeded = true;
                            }
                        }
                    }

                    pos -= 1.0;
                }

                *out++ = InterpolatorType::valueAtOffset (lastInputSamples, (CoefficientType) pos);
                pos += actualRatio;
            }
        }
        else
        {
            for (int i = numOut; --i >= 0;)
            {
                while (pos < actualRatio)
                {
                    if (exceeded)
                    {
                        pushInterpolationSample (lastInputSamples, 0);
                    }
                    else
                    {
                        pushInterpolationSample (lastInputSamples, *in++);

                        if (--available <= 0)
                        {
                            if (wrap > 0)
                            {
                                in -= wrap;
                                available += wrap;
                            }
                            else
                            {
                                exceeded = true;
                            }
                        }
                    }

                    pos += 1.0;
                }

                pos -= actualRatio;
                *out++ = InterpolatorType::valueAtOffset (lastInputSamples, jmax (CoefficientType (0.0), CoefficientType (1.0) - (CoefficientType) pos));
            }
        }

        subSamplePos = pos;

        if (wrap == 0)
            return (int) (in - originalIn);

        return ((int) (in - originalIn) + wrap) % wrap;
    }

    static int interpolateAdding (SampleType* lastInputSamples, double& subSamplePos, double actualRatio,
            const SampleType* in, SampleType* out, int numOut,
            int available, int wrap, SampleType gain) noexcept
    {
        if (actualRatio == 1.0)
        {
            if (available >= numOut)
            {
                FloatVectorOperations::addWithMultiply (out, in, gain, numOut);
                pushInterpolationSamples (lastInputSamples, in, numOut, available, wrap);
            }
            else
            {
                FloatVectorOperations::addWithMultiply (out, in, gain, available);
                pushInterpolationSamples (lastInputSamples, in, available, available, wrap);

                if (wrap > 0)
                {
                    FloatVectorOperations::addWithMultiply (out, in - wrap, gain, numOut - available);
                    pushInterpolationSamples (lastInputSamples, in - wrap, numOut - available, available, wrap);
                }
                else
                {
                    for (int i = 0; i < numOut-available; ++i)
                        pushInterpolationSample (lastInputSamples, 0.0);
                }
            }

            return numOut;
        }

        auto originalIn = in;
        auto pos = subSamplePos;
        bool exceeded = false;

        if (actualRatio < 1.0)
        {
            for (int i = numOut; --i >= 0;)
            {
                if (pos >= 1.0)
                {
                    if (exceeded)
                    {
                        pushInterpolationSample (lastInputSamples, 0.0);
                    }
                    else
                    {
                        pushInterpolationSample (lastInputSamples, *in++);

                        if (--available <= 0)
                        {
                            if (wrap > 0)
                            {
                                in -= wrap;
                                available += wrap;
                            }
                            else
                            {
                                exceeded = true;
                            }
                        }
                    }

                    pos -= 1.0;
                }

                *out++ += gain * InterpolatorType::valueAtOffset (lastInputSamples, (CoefficientType) pos);
                pos += actualRatio;
            }
        }
        else
        {
            for (int i = numOut; --i >= 0;)
            {
                while (pos < actualRatio)
                {
                    if (exceeded)
                    {
                        pushInterpolationSample (lastInputSamples, 0.0);
                    }
                    else
                    {
                        pushInterpolationSample (lastInputSamples, *in++);

                        if (--available <= 0)
                        {
                            if (wrap > 0)
                            {
                                in -= wrap;
                                available += wrap;
                            }
                            else
                            {
                                exceeded = true;
                            }
                        }
                    }

                    pos += 1.0;
                }

                pos -= actualRatio;
                *out++ += gain * InterpolatorType::valueAtOffset (lastInputSamples, jmax (CoefficientType (0.0), CoefficientType(1.0) - (CoefficientType) pos));
            }
        }

        subSamplePos = pos;

        if (wrap == 0)
            return (int) (in - originalIn);

        return ((int) (in - originalIn) + wrap) % wrap;
    }

    static int interpolateAdding (SampleType* lastInputSamples, double& subSamplePos, double actualRatio,
            const SampleType* in, SampleType* out, int numOut, CoefficientType gain) noexcept
    {
        auto pos = subSamplePos;

        if (actualRatio == 1.0 && pos == 1.0)
        {
            FloatVectorOperations::addWithMultiply (out, in, gain, numOut);
            pushInterpolationSamples (lastInputSamples, in, numOut);
            return numOut;
        }

        int numUsed = 0;

        while (numOut > 0)
        {
            while (pos >= 1.0)
            {
                pushInterpolationSample (lastInputSamples, in[numUsed++]);
                pos -= 1.0;
            }

            *out++ += gain * InterpolatorType::valueAtOffset (lastInputSamples, (CoefficientType) pos);
            pos += actualRatio;
            --numOut;
        }

        subSamplePos = pos;
        return numUsed;
    }
};
}