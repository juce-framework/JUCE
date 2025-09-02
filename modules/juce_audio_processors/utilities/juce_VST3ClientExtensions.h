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

/** @cond */

// Forward declaration to avoid leaking implementation details.
namespace Steinberg
{
    class FUnknown;
    using TUID = char[16];
} // namespace Steinberg

/** @endcond */

namespace juce
{

/**
    An interface to allow an AudioProcessor to implement extended VST3-specific functionality.

    To use this class, create an object that inherits from it, implement the methods, then return
    a pointer to the object in your AudioProcessor::getVST3ClientExtensions() method.

    @see AudioProcessor, AAXClientExtensions, VST2ClientExtensions

    @tags{Audio}
*/
struct VST3ClientExtensions
{
    virtual ~VST3ClientExtensions() = default;

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IEditController to return
        additional supported interfaces.
    */
    virtual int32_t queryIEditController (const Steinberg::TUID, void** obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This function may be used by implementations of queryInterface()
        in the VST3's implementation of IAudioProcessor to return
        additional supported interfaces.
    */
    virtual int32_t queryIAudioProcessor (const Steinberg::TUID, void** obj)
    {
        *obj = nullptr;
        return -1;
    }

    /** This may be called by the VST3 wrapper when the host sets an
        IComponentHandler for the plugin to use.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual void setIComponentHandler (Steinberg::FUnknown*) {}

    /** This may be called shortly after the AudioProcessor is constructed
        with the current IHostApplication.

        You should not make any assumptions about how and when this will be
        called - this function may not be called at all!
    */
    virtual void setIHostApplication  (Steinberg::FUnknown*) {}

    /** This function will be called to check whether the first input bus
        should be designated as "kMain" or "kAux". Return true if the
        first bus should be kMain, or false if the bus should be kAux.

        All other input buses will always be designated kAux.
    */
    virtual bool getPluginHasMainInput() const  { return true; }

    /** This function should return a map of VST3 parameter IDs and the JUCE
        parameters they map to.

        This information is used to implement the IRemapParamID interface.
        Hosts can use this to preserve automation data when a session was saved
        using a compatible plugin that has different parameter IDs.

        Not all hosts will take this information into account. Therefore,
        parameter IDs should be maintained between plugin versions. For JUCE
        plugins migrating from VST2 to VST3 the best method for achieving this
        is enabling JUCE_FORCE_LEGACY_PARAM_IDS. However, if a plugin has
        already been released without enabling this flag, this method offers an
        alternative approach that won't cause any further compatibility issues.

        The key in the map is an integer which may represent a VST3 parameter
        identifier (Vst::ParamID) or VST2 parameter index.
        You should include a map entry for every parameter ID in the compatible
        plugin.
        For VST2 or JUCE plugins these IDs can be determined in the following ways.
        - Use the parameter index if any of the following apply:
          - the InterfaceId argument refers to a compatible VST2 plugin, or
          - the InterfaceId argument refers to a JUCE VST3 plugin with
            JUCE_FORCE_LEGACY_PARAM_IDS enabled, or
          - the InterfaceId argument refers to a JUCE plugin, but the parameter
            in the compatible plugin doesn't inherit from
            HostedAudioProcessorParameter (this case is unlikely).
        - Otherwise, use convertJuceParameterId() for JUCE VST3 plugins where
          JUCE_FORCE_LEGACY_PARAM_IDS is disabled, and where the compatible
          parameter derives from HostedAudioProcessorParameter.
        - For non-JUCE VST3s, use the Vst::ParamIDs exported by the compatible
          VST3.

        The value in the map is the JUCE parameter ID for the parameter to map
        to, or an empty string to indicate that there is no parameter to map to.
        If a parameter doesn't inherit from HostedAudioProcessorParameter its ID
        will be the parameter index as a string, for example "1". Otherwise,
        always use the actual parameter ID (even if JUCE_FORCE_LEGACY_PARAM_IDS
        is enabled).

        In the unlikely event that two plugins share the same plugin ID, and
        both have a different parameters that share the same parameter ID, it
        may be possible to determine which version of the plugin is being loaded
        during setStateInformation(). This method will always be called after
        setStateInformation(), so that the map with the correct mapping can be
        provided when queried.

        Below is an example of how you might implement this function for a JUCE
        VST3 plugin where JUCE_VST3_CAN_REPLACE_VST2 is enabled, but
        JUCE_FORCE_LEGACY_PARAM_IDS is disabled.

        @code
        std::map<uint32_t, String> getCompatibleParameterIds (const InterfaceId&) const override
        {
            return { { 0, "Frequency" },
                     { 1, "CutOff" },
                     { 2, "Gain" },
                     { 3, "Bypass" } };
        }
        @endcode

        @param compatibleClass  A plugin identifier, either for the current
                                plugin or one listed in JUCE_VST3_COMPATIBLE_CLASSES.
                                This parameter allows the implementation to
                                return a different parameter map for each
                                compatible class. Use VST3Interface::jucePluginId()
                                and VST3Interface::vst2PluginId() to determine
                                the class IDs used by JUCE plugins. When
                                JUCE_VST3_CAN_REPLACE_VST2 is set, the ID
                                denoting the VST2 version of the plugin will
                                match the ID of the VST3 that replaces it. In
                                this case, assuming there are no collisions
                                between the VST2 parameter indices and the VST3
                                ParamIDs you should only include the VST2
                                mappings in the returned map. In the unlikely
                                event of a collision you should inspect the
                                state that was most recently passed to
                                setStateInformation() to determine whether the
                                host is loading a VST2 state that requires
                                parameter remapping. If you determine that no
                                remapping is necessary, you can indicate this by
                                returning an empty map.

        @returns    A map where each key is a VST3 parameter ID in the compatible
                    plugin, and the value is the unique JUCE parameter ID in the
                    current plugin that it should be mapped to.

        @see JUCE_VST3_COMPATIBLE_CLASSES, VST3Interface::jucePluginId, VST3Interface::vst2PluginId, VST3Interface::hexStringToId
    */
    virtual std::map<uint32_t, String> getCompatibleParameterIds (const VST3Interface::Id& compatibleClass) const;


    /** Returns the VST3 compatible parameter ID reported for a given JUCE
        parameter.

        Internally JUCE will use this method to determine the Vst::ParamID for
        a HostedAudioProcessorParameter, unless JUCE_FORCE_LEGACY_PARAM_IDS is
        enabled, in which case it will use the parameter index.

        @see getCompatibleParameterIds
    */
    static uint32_t convertJuceParameterId (const String& parameterId,
                                            bool studioOneCompatible = true);

private:
    /** Instead of overriding this function you should define the preprocessor
        definition JUCE_VST3_COMPATIBLE_CLASSES as described in the docs.

        @see JUCE_VST3_COMPATIBLE_CLASSES
    */
    virtual std::vector<VST3Interface::Id> getCompatibleClasses() const final
    {
        return {};
    }
};

#if DOXYGEN
 /** An optional user defined preprocessor definition for declaring a comma
     separated list of VST2 and VST3 plugin identifiers that this VST3 plugin
     can replace in a DAW session.

     The definition of this preprocessor must be defined at the project
     level, normally in your CMake or Projucer project files.

     This information will be used to implement the IPluginCompatibility
     interface.

     If JUCE_VST3_CAN_REPLACE_VST2 is enabled, the VST3 plugin will have the
     same identifier as the VST2 plugin and therefore you don't need to
     implement this preprocessor definition.

     This preprocessor definition can contain code that depends on any class
     or function defined as part of the VST3Interface struct but should avoid
     any other dependencies!

     Each compatible class is a 16-byte array that corresponds to the VST3
     interface identifier as reported by a plugins IComponent interface.
     For VST2 or JUCE plugins these identifiers can be determined in the
     following ways:
     - Use VST3Interface::vst2PluginId() for any VST2 plugins or JUCE VST3
         plugin with JUCE_VST3_CAN_REPLACE_VST3 enabled
     - Use VST3Interface::jucePluginId() for any other JUCE VST3 plugins

     Examples

     @code
     // Defines a VST2 plugin this VST3 can replace
     JUCE_VST3_COMPATIBLE_CLASSES=VST3Interface::vst2PluginId ('Plug', "Plugin Name")

     // Defines a VST3 plugin this VST3 can replace
     JUCE_VST3_COMPATIBLE_CLASSES=VST3Interface::jucePluginId ('Manu', 'Plug')

     // Defines both a VST2 and a VST3 plugin this VST3 can replace
     JUCE_VST3_COMPATIBLE_CLASSES=VST3Interface::vst2PluginId ('Plug', "Plugin Name"), VST3Interface::jucePluginId ('Manu', 'Plug')

     // Defines a non-JUCE VST3 plugin this VST3 can replace
     JUCE_VST3_COMPATIBLE_CLASSES=VST3Interface::hexStringToId ("0F1E2D3C4B5A69788796A5B4C3D2E1F0")
     @endcode

     If the parameter IDs between compatible versions differ
     VST3ClientExtensions::getCompatibleParameterIds() should also be overridden.

     @see VST3Interface VST3ClientExtensions::getCompatibleParameterIds()
 */
 #define JUCE_VST3_COMPATIBLE_CLASSES
#endif // DOXYGEN

} // namespace juce
