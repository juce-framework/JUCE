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
    Combines a parameter ID and a version hint.

    @tags{Audio}
*/
class ParameterID
{
public:
    ParameterID() = default;

    /** Constructs an instance.

        Note that this constructor implicitly converts from Strings and string-like types.

        @param identifier       A string that uniquely identifies a single parameter
        @param versionHint      Influences parameter ordering in Audio Unit plugins.
                                Used to provide backwards compatibility of Audio Unit plugins in
                                Logic and GarageBand.
                                @see AudioProcessorParameter (int)
    */
    template <typename StringLike, typename = DisableIfSameOrDerived<ParameterID, StringLike>>
    ParameterID (StringLike&& identifier, int versionHint = 0)
        : paramID (std::forward<StringLike> (identifier)), version (versionHint) {}

    /** @see AudioProcessorParameterWithID::paramID */
    auto getParamID()               const { return paramID; }

    /** @see AudioProcessorParameter (int) */
    auto getVersionHint()           const { return version; }

private:
    String paramID;
    int version = 0;
};

/**
    An instance of this class may be passed to the constructor of an AudioProcessorParameterWithID
    to set optional characteristics of that parameter.

    @tags{Audio}
*/
class AudioProcessorParameterWithIDAttributes
{
    using This = AudioProcessorParameterWithIDAttributes;

public:
    using Category = AudioProcessorParameter::Category;

    /** An optional label for the parameter's value */
    [[nodiscard]] auto withLabel (String x)            const { return withMember (*this, &This::label,          std::move (x)); }

    /** The semantics of this parameter */
    [[nodiscard]] auto withCategory (Category x)       const { return withMember (*this, &This::category,       std::move (x)); }

    /** @see AudioProcessorParameter::isMetaParameter() */
    [[nodiscard]] auto withMeta (bool x)               const { return withMember (*this, &This::meta,           std::move (x)); }

    /** @see AudioProcessorParameter::isAutomatable() */
    [[nodiscard]] auto withAutomatable (bool x)        const { return withMember (*this, &This::automatable,    std::move (x)); }

    /** @see AudioProcessorParameter::isOrientationInverted() */
    [[nodiscard]] auto withInverted (bool x)           const { return withMember (*this, &This::inverted,       std::move (x)); }

    /** An optional label for the parameter's value */
    [[nodiscard]] auto getLabel()                      const { return label; }

    /** The semantics of this parameter */
    [[nodiscard]] auto getCategory()                   const { return category; }

    /** @see AudioProcessorParameter::isMetaParameter() */
    [[nodiscard]] auto getMeta()                       const { return meta; }

    /** @see AudioProcessorParameter::isAutomatable() */
    [[nodiscard]] auto getAutomatable()                const { return automatable; }

    /** @see AudioProcessorParameter::isOrientationInverted() */
    [[nodiscard]] auto getInverted()                   const { return inverted; }

private:
    String label;
    Category category = AudioProcessorParameter::genericParameter;
    bool meta = false, automatable = true, inverted = false;
};

//==============================================================================
/**
    This abstract base class is used by some AudioProcessorParameter helper classes.

    @see AudioParameterFloat, AudioParameterInt, AudioParameterBool, AudioParameterChoice

    @tags{Audio}
*/
class JUCE_API  AudioProcessorParameterWithID  : public HostedAudioProcessorParameter
{
public:
    /** The creation of this object requires providing a name and ID which will be constant for its lifetime.

        Given that AudioProcessorParameterWithID is abstract, you'll probably call this constructor
        from a derived class constructor, e.g.
        @code
        MyParameterType (String paramID, String name, String label, bool automatable)
            : AudioProcessorParameterWithID (paramID, name, AudioProcessorParameterWithIDAttributes().withLabel (label)
                                                                                                     .withAutomatable (automatable))
        {
        }
        @endcode

        @param parameterID      Specifies the identifier, and optionally the parameter's version hint.
        @param parameterName    The user-facing parameter name.
        @param attributes       Other parameter properties.
    */
    AudioProcessorParameterWithID (const ParameterID& parameterID,
                                   const String& parameterName,
                                   const AudioProcessorParameterWithIDAttributes& attributes = {});

    /** The creation of this object requires providing a name and ID which will be
        constant for its lifetime.

        @param parameterID          Used to uniquely identify the parameter
        @param parameterName        The user-facing name of the parameter
        @param parameterLabel       An optional label for the parameter's value
        @param parameterCategory    The semantics of this parameter
    */
    [[deprecated ("Prefer the signature taking an Attributes argument")]]
    AudioProcessorParameterWithID (const ParameterID& parameterID,
                                   const String& parameterName,
                                   const String& parameterLabel,
                                   Category parameterCategory = AudioProcessorParameter::genericParameter)
        : AudioProcessorParameterWithID (parameterID,
                                         parameterName,
                                         AudioProcessorParameterWithIDAttributes().withLabel (parameterLabel)
                                                                                  .withCategory (parameterCategory))
    {
    }

    /** Provides access to the parameter's ID string. */
    const String paramID;

    /** Provides access to the parameter's name. */
    const String name;

    /** Provides access to the parameter's label. */
    const String label;

    /** Provides access to the parameter's category. */
    const Category category;

    String getName (int) const override;
    String getLabel() const override;
    Category getCategory() const override;

    String getParameterID()             const override { return paramID; }
    bool isMetaParameter()              const override { return meta; }
    bool isAutomatable()                const override { return automatable; }
    bool isOrientationInverted()        const override { return inverted; }

private:
    bool meta = false, automatable = true, inverted = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterWithID)
};

} // namespace juce
