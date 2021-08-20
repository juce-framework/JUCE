/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 6 End-User License
   Agreement and JUCE Privacy Policy (both effective as of the 16th June 2020).

   End User License Agreement: www.juce.com/juce-6-licence
   Privacy Policy: www.juce.com/juce-privacy-policy

   Or: You may also use this code under the terms of the GPL v3 (see
   www.gnu.org/licenses).

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
/**
    A component effect that blurs the component using the Stackblur algorithm.

    You can control the blur radius, as well as whether or not to use a thread
    pool, which may improve performance.

    @see Component::setComponentEffect

    @tags{Graphics}
 */
class JUCE_API StackBlurEffect : public ImageEffectFilter
{
public:
    //==============================================================================
    /** Creates a default blur effect with a radius of 0. */
    StackBlurEffect() = default;

    /** Destructor. */
    ~StackBlurEffect() override = default;

    //==============================================================================
    /** Sets the blur radius for this effect.
        
        The radius must be greater than 0.
    */
    void setBlurRadius (int newBlurRadius);

    /** Specified whether or not to use a thread pool for the rendering.
        
        If enabled, each row and column of the image will be blurred in a
        separate job. You can control the number of jobs run on each thread in
        the pool using the jobsPerThread parameter.

        Enabled by default, using 5 jobs per thread.
    */
    void setUseThreadPool (bool shouldUseThreadPool, int jobsPerThread = 5);

    /** Returns true if this effect will use a thread pool for its rendering.
        
        True by default.
    */
    bool isUsingThreadPool() const { return threadPoolEnabled; };

    //==============================================================================
    /** Blurs the source image and draws the result to the destination context.
        The original image will be left un-blurred, so you probably want to make
        sure the destination context isn't going to draw to that image.
    */
    void applyEffect (Image& sourceImage,
                      Graphics& destContext,
                      float scaleFactor,
                      float alpha) override;

private:
    //==============================================================================
    void blurHorizontally (Image& image, int channel, float scale);
    void blurVertically (Image& image, int channel, float scale);

    void waitForAllThreadPoolJobsToFinish() const;

    //==============================================================================
    int blurRadius { 0 };
    bool threadPoolEnabled {true};
    int numJobsPerThread {5};
    juce::ThreadPool threadPool;

    JUCE_LEAK_DETECTOR (StackBlurEffect)
};

} // namespace juce
