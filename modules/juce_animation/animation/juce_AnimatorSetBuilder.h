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

//==============================================================================
/**
    A builder class that can be used to construct an Animator wrapping an AnimatorSet
    implementation. It allows you to compose larger, complex animations by executing multiple
    constituent Animator instances in a coordinated manner. It essentially builds an Animator
    with an execution graph referencing other Animators.

    Unlike ValueAnimatorBuilder, objects of AnimatorSetBuilder returned by its member functions
    reference the same underlying, modifiable builder instance. For this reason build() can be
    called only once on an underlying builder instance. This is to allow you to attach Animators to
    different points of the execution graph.

    E.g. to have two functions followed by different amounts of delay, each followed by another
    function you would write the following.

    @code
    // Both objects reference the same execution graph, but also refer to different Animators in it
    auto builderReferencingFirst = AnimatorSetBuilder { firstFunction };
    auto builderReferencingSecond = builderReferencingFirst.togetherWith (secondFunction);

    builderReferencingFirst.followedBy (200).followedBy (thirdFunction);
    builderReferencingSecond.followedBy (500).followedBy (fourthFunction);

    // You could use any one of the builder objects that refer to the same execution graph
    auto animator = builderReferencingFirst.build();
    @endcode

    @tags{Animations}
*/
class JUCE_API  AnimatorSetBuilder
{
public:
    /** Creates a new builder instance specifying the startingAnimator as the first Animator that is
        started.
    */
    explicit AnimatorSetBuilder (Animator startingAnimator);

    /** Creates a builder with an empty starting animation that completes after delayMs.
    */
    explicit AnimatorSetBuilder (double delayMs);

    /** Creates a builder with a starting animation that completes at the first update and executes
        the provided callback function.
    */
    explicit AnimatorSetBuilder (std::function<void()> cb);

    /** Adds an Animator to the execution graph that will start executing at the same time as the
        Animator provided last to this builder object.
    */
    AnimatorSetBuilder togetherWith (Animator animator);

    /** Adds an empty Animator to the execution graph that will start executing at the same time as
        the Animator provided last to this builder object, and completes in delayMs.
    */
    AnimatorSetBuilder togetherWith (double delayMs);

    /** Adds an empty Animator to the execution graph that will start executing at the same time as
        the Animator provided last to this builder object, completes upon its first update, and
        executes the provided callback.
    */
    AnimatorSetBuilder togetherWith (std::function<void()> cb);

    /** Adds an Animator to the execution graph that will start executing after the Animator
        provided last to this builder object completes.
    */
    AnimatorSetBuilder followedBy (Animator animator);

    /** Adds an empty Animator to the execution graph that will start executing after the Animator
        provided last to this builder object
    */
    AnimatorSetBuilder followedBy (double delayMs);

    /** Adds an empty Animator to the execution graph that will start executing after the Animator
        provided last to this builder object, completes upon its first update, and executes the
        provided callback.
    */
    AnimatorSetBuilder followedBy (std::function<void()> cb);

    /** Specifies a time transformation function that the built Animator should utilise, allowing
        accelerating and decelerating the entire set of Animators.

        The provided function should be monotonically increasing.
    */
    AnimatorSetBuilder withTimeTransform (std::function<double (double)> transform);

    /** Builds an Animator that executes the previously described and parameterised execution graph.

        This function should only be called once for every AnimatorSetBuilder created by its public
        constructor.
    */
    Animator build();

private:
    struct AnimatorSetBuilderState;

    explicit AnimatorSetBuilder (std::shared_ptr<AnimatorSetBuilderState> dataIn);

    AnimatorSetBuilder (Animator cursorIn, std::shared_ptr<AnimatorSetBuilderState> dataIn);

    void add (std::optional<Animator> parent, Animator child);

    Animator cursor;
    std::shared_ptr<AnimatorSetBuilderState> state;
};

} // namespace juce
