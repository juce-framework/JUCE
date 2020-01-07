/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   By using JUCE, you agree to the terms of both the JUCE 5 End-User License
   Agreement and JUCE 5 Privacy Policy (both updated and effective as of the
   27th April 2017).

   End User License Agreement: www.juce.com/juce-5-licence
   Privacy Policy: www.juce.com/juce-5-privacy-policy

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
/** A class encapsulating a group of AudioProcessorParameters and nested
    AudioProcessorParameterGroups.

    This class is predominantly write-only; there are methods for adding group
    members but none for removing them. Ultimately you will probably want to
    add a fully constructed group to an AudioProcessor.

    @see AudioProcessor::addParameterGroup

    @tags{Audio}
*/
class AudioProcessorParameterGroup
{
public:
    //==============================================================================
    /** A child of an AudioProcessorParameterGroup.

        This can contain either an AudioProcessorParameter or an
        AudioProcessorParameterGroup. You can query which using the
        getParameter and getGroup methods.

        @code
        for (auto* child : group)
            if (auto* parameter = node.getParameter())
                parameter->setValueNotifyingHost (0.5f);
            else
                node.getGroup()->AddChild (new Parameter());
        @endcode
    */
    class AudioProcessorParameterNode
    {
    public:
        //==============================================================================
        AudioProcessorParameterNode (AudioProcessorParameterNode&&);
        ~AudioProcessorParameterNode();

        /** Returns the parent group or nullptr if this is a top-level group. */
        AudioProcessorParameterGroup* getParent() const;

        /** Returns a pointer to a parameter if this node contains a parameter, nullptr otherwise. */
        AudioProcessorParameter* getParameter() const;

        /** Returns a pointer to a group if this node contains a group, nullptr otherwise. */
        AudioProcessorParameterGroup* getGroup() const;

    private:
        //==============================================================================
        AudioProcessorParameterNode (std::unique_ptr<AudioProcessorParameter>, AudioProcessorParameterGroup*);
        AudioProcessorParameterNode (std::unique_ptr<AudioProcessorParameterGroup>, AudioProcessorParameterGroup*);

        std::unique_ptr<AudioProcessorParameterGroup> group;
        std::unique_ptr<AudioProcessorParameter> parameter;
        AudioProcessorParameterGroup* parent = nullptr;

        friend class AudioProcessorParameterGroup;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterNode)
    };

    //==============================================================================
    /** Creates an empty AudioProcessorParameterGroup with no name or ID. */
    AudioProcessorParameterGroup();

    /** Creates an empty AudioProcessorParameterGroup.

        @param groupID             A unique identifier for the group. Keep it basic; don't use any special
                                   characters like "." and avoid pure integer strings which could collide with
                                   legacy parameter IDs.
        @param groupName           The group's name, which will be displayed in the host.
        @param subgroupSeparator   A separator string to use between the name of this group and the name of any
                                   subgroups if this group is flattened. AUv3 and VST3 plug-ins can have multiple
                                   layers of nested subgroups, but AU plug-ins cannot have any subgroups.

    */
    AudioProcessorParameterGroup (String groupID, String groupName, String subgroupSeparator);

    /** Creates an AudioProcessorParameterGroup with a single child.

        @param groupID             A unique identifier for the group. Keep it basic; don't use any special
                                   characters like "." and avoid pure integer strings which could collide with
                                   legacy parameter IDs.
        @param groupName           The group's name, which will be displayed in the host.
        @param subgroupSeparator   A separator string to use between the name of this group and the name of any
                                   subgroups if this group is flattened. AUv3 and VST3 plug-ins can have multiple
                                   layers of nested subgroups, but AU plug-ins cannot have any subgroups.
        @param child               An AudioProcessorParameter or an AudioProcessorParameterGroup to add to the group.
    */
    template <typename ParameterOrGroup>
    AudioProcessorParameterGroup (String groupID, String groupName, String subgroupSeparator,
                                  std::unique_ptr<ParameterOrGroup> child)
        : AudioProcessorParameterGroup (groupID, groupName, subgroupSeparator)
    {
        addChild (std::move (child));
    }

    /** Creates an AudioProcessorParameterGroup with multiple children.

        @param groupID             A unique identifier for the group. Keep it basic; don't use any special
                                   characters like "." and avoid pure integer strings which could collide with
                                   legacy parameter IDs.
        @param groupName           The group's name, which will be displayed in the host.
        @param subgroupSeparator   A separator string to use between the name of this group and the name of any
                                   subgroups if this group is flattened. AUv3 and VST3 plug-ins can have multiple
                                   layers of nested subgroups, but AU plug-ins cannot have any subgroups.
        @param firstChild          An AudioProcessorParameter or an AudioProcessorParameterGroup to add to the group.
        @param remainingChildren   A list of more AudioProcessorParameters or AudioProcessorParameterGroups to add to the group.
    */
    template <typename ParameterOrGroup, typename... Args>
    AudioProcessorParameterGroup (String groupID, String groupName, String subgroupSeparator,
                                  std::unique_ptr<ParameterOrGroup> firstChild, Args&&... remainingChildren)
        : AudioProcessorParameterGroup (groupID, groupName, subgroupSeparator, std::move (firstChild))
    {
        addChild (std::forward<Args> (remainingChildren)...);
    }

    /** Once a group has been added to an AudioProcessor don't try to mutate it by
        moving or swapping it - this will crash most hosts.
    */
    AudioProcessorParameterGroup (AudioProcessorParameterGroup&&);

    /** Once a group has been added to an AudioProcessor don't try to mutate it by
        moving or swapping it - this will crash most hosts.
    */
    AudioProcessorParameterGroup& operator= (AudioProcessorParameterGroup&&);

    /** Destructor. */
    ~AudioProcessorParameterGroup();

    //==============================================================================
    /** Returns the group's ID. */
    String getID() const;

    /** Returns the group's name. */
    String getName() const;

    /** Returns the group's separator string. */
    String getSeparator() const;

    /** Returns the parent of the group, or nullptr if this is a top-levle group. */
    const AudioProcessorParameterGroup* getParent() const noexcept;

    //==============================================================================
    /** Changes the name of the group. If you do this after the group has been added
        to an AudioProcessor, call updateHostDisplay() to inform the host of the
        change. Not all hosts support dynamic group name changes.
    */
    void setName (String newName);

    //==============================================================================
    const AudioProcessorParameterNode* const* begin() const noexcept;
    const AudioProcessorParameterNode* const* end()   const noexcept;

    //==============================================================================
    /** Returns all subgroups of this group.

        @param recursive   If this is true then this method will fetch all nested
                           subgroups using a depth first search.
    */
    Array<const AudioProcessorParameterGroup*> getSubgroups (bool recursive) const;

    /** Returns all the parameters in this group.

        @param recursive   If this is true then this method will fetch all nested
                           parameters using a depth first search.
    */
    Array<AudioProcessorParameter*> getParameters (bool recursive) const;

    /** Searches this group recursively for a parameter and returns a depth ordered
        list of the groups it belongs to.
    */
    Array<const AudioProcessorParameterGroup*> getGroupsForParameter (AudioProcessorParameter*) const;

    //==============================================================================
    /** Adds a child to the group.

        Do not add children to a group which has itself already been added to the
        AudioProcessor - the new elements will be ignored.
    */
    template <typename ParameterOrGroup>
    void addChild (std::unique_ptr<ParameterOrGroup> child)
    {
        // If you hit a compiler error here then you are attempting to add a
        // child that is neither a pointer to an AudioProcessorParameterGroup
        // nor a pointer to an AudioProcessorParameter.
        append (std::move (child));
    }

    /** Adds multiple parameters or sub-groups to this group.

        Do not add children to a group which has itself already been added to the
        AudioProcessor - the new elements will be ignored.
    */
    template <typename ParameterOrGroup, typename... Args>
    void addChild (std::unique_ptr<ParameterOrGroup> firstChild, Args&&... remainingChildren)
    {
        addChild (std::move (firstChild));
        addChild (std::forward<Args> (remainingChildren)...);
    }

   #ifndef DOXYGEN
    // This class now has a move operator, so if you're trying to move them around, you
    // should use that, or if you really need to swap two groups, just call std::swap.
    // However, remember that swapping a group that's already owned by an AudioProcessor
    // will most likely crash the host, so don't do that.
    JUCE_DEPRECATED_WITH_BODY (void swapWith (AudioProcessorParameterGroup& other), { std::swap (*this, other); })
   #endif

private:
    //==============================================================================
    void getSubgroups (Array<const AudioProcessorParameterGroup*>&, bool recursive) const;
    void getParameters (Array<AudioProcessorParameter*>&, bool recursive) const;
    const AudioProcessorParameterGroup* getGroupForParameter (AudioProcessorParameter*) const;
    void updateChildParentage();
    void append (std::unique_ptr<AudioProcessorParameter>);
    void append (std::unique_ptr<AudioProcessorParameterGroup>);

    //==============================================================================
    String identifier, name, separator;
    OwnedArray<AudioProcessorParameterNode> children;
    AudioProcessorParameterGroup* parent = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterGroup)
};

} // namespace juce
