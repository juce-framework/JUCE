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
        /** Returns the parent group or nullptr if this is a top-level group. */
        AudioProcessorParameterGroup* getParent() const              { return parent; }

        /** Returns a pointer to a parameter if this node contains a parameter, nullptr otherwise. */
        AudioProcessorParameter* getParameter() const                { return parameter.get(); }

        /** Returns a pointer to a group if this node contains a group, nullptr otherwise. */
        AudioProcessorParameterGroup* getGroup() const               { return group.get(); }

    private:
        //==============================================================================
        AudioProcessorParameterNode (std::unique_ptr<AudioProcessorParameter> param,
                                     AudioProcessorParameterGroup* parentGroup)
            : parameter (std::move (param)), parent (parentGroup)
        {}

        AudioProcessorParameterNode (std::unique_ptr<AudioProcessorParameterGroup> grp,
                                     AudioProcessorParameterGroup* parentGroup)
            : group (std::move (grp)), parent (parentGroup)
        {
            group->parent = parent;
        }

        std::unique_ptr<AudioProcessorParameterGroup> group;
        std::unique_ptr<AudioProcessorParameter> parameter;
        AudioProcessorParameterGroup* parent = nullptr;

        friend class AudioProcessorParameterGroup;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterNode)
    };

    //==============================================================================
    /** Creates an empty AudioProcessorParameterGroup.

        @param groupID             A unique identifier for the group. Keep it basic; don't use any special
                                   characters like "." and avoid pure integer strings which could collide with
                                   legacy parameter IDs.
        @param groupName           The group's name, which will be displayed in the host.
        @param subgroupSeparator   A separator string to use between the name of this group and the name of any
                                   subgroups if this group is flattened. AUv3 and VST3 plug-ins can have multiple
                                   layers of nested subgroups, but AU plug-ins cannot have any subgroups.

    */
    AudioProcessorParameterGroup (const String& groupID, const String& groupName, const String& subgroupSeparator)
        : identifier (groupID), name (groupName), separator (subgroupSeparator)
    {
    }

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
    template <typename ChildType>
    AudioProcessorParameterGroup (const String& groupID, const String& groupName, const String& subgroupSeparator,
                                  std::unique_ptr<ChildType> child)
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
    template <typename ChildType, typename... Args>
    AudioProcessorParameterGroup (const String& groupID, const String& groupName, const String& subgroupSeparator,
                                  std::unique_ptr<ChildType> firstChild, Args&&... remainingChildren)
        : AudioProcessorParameterGroup (groupID, groupName, subgroupSeparator, std::move (firstChild))
    {
        addChild (std::forward<Args> (remainingChildren)...);
    }

    //==============================================================================
    /** Returns the group's ID. */
    String getID() const                                                     { return identifier; }

    /** Returns the group's name. */
    String getName() const                                                   { return name; }

    /** Returns the group's separator string. */
    String getSeparator() const                                              { return separator; }

    /** Returns the parent of the group, or nullptr if this is a top-levle group. */
    const AudioProcessorParameterGroup* getParent() const noexcept           { return parent; }

    //==============================================================================
    const AudioProcessorParameterNode** begin() const noexcept               { return children.begin(); }
    const AudioProcessorParameterNode** end()   const noexcept               { return children.end(); }

    //==============================================================================
    /** Swaps the content of this group with another. */
    void swapWith (AudioProcessorParameterGroup& other) noexcept
    {
        children.swapWith (other.children);

        auto refreshParentPtr = [] (AudioProcessorParameterGroup& parentGroup)
        {
            for (auto* child : parentGroup)
                if (auto* group = child->getGroup())
                    group->parent = &parentGroup;
        };

        refreshParentPtr (*this);
        refreshParentPtr (other);
    }

    //==============================================================================
    /** Returns all subgroups of this group.

        @param recursive   If this is true then this method will fetch all nested
                           subgroups using a depth first search.
    */
    Array<const AudioProcessorParameterGroup*> getSubgroups (bool recursive) const
    {
        Array<const AudioProcessorParameterGroup*> groups;
        getSubgroups (groups, recursive);
        return groups;
    }

    /** Returns all the parameters in this group.

        @param recursive   If this is true then this method will fetch all nested
                           parameters using a depth first search.
    */
    Array<AudioProcessorParameter*> getParameters (bool recursive) const
    {
        Array<AudioProcessorParameter*> parameters;
        getParameters (parameters, recursive);
        return parameters;
    }

    /** Searches this group recursively for a parameter and returns a depth ordered
        list of the groups it belongs to.
    */
    Array<const AudioProcessorParameterGroup*> getGroupsForParameter (AudioProcessorParameter* parameter) const
    {
        Array<const AudioProcessorParameterGroup*> groups;

        if (auto* group = getGroupForParameter (parameter))
        {
            while (group != this)
            {
                groups.insert (0, group);
                group = group->getParent();
            }
        }

        return groups;
    }

    //==============================================================================
    /** Adds a child to the group. */
    template <typename ChildType>
    void addChild (std::unique_ptr<ChildType> child)
    {
        // If you hit a compiler error here then you are attempting to add a
        // child that is neither a pointer to an AudioProcessorParameterGroup
        // nor a pointer to an AudioProcessorParameter.
        children.add (new AudioProcessorParameterNode (std::move (child), this));
    }

    /** Adds multiple children to the group. */
    template <typename ChildType, typename... Args>
    void addChild (std::unique_ptr<ChildType> firstChild, Args&&... remainingChildren)
    {
        addChild (std::move (firstChild));
        addChild (std::forward<Args> (remainingChildren)...);
    }

private:
    //==============================================================================
    void getSubgroups (Array<const AudioProcessorParameterGroup*>& previousGroups, bool recursive) const
    {
        for (auto* child : children)
        {
            if (auto* group = child->getGroup())
            {
                previousGroups.add (group);

                if (recursive)
                    group->getSubgroups (previousGroups, true);
            }
        }
    }

    void getParameters (Array<AudioProcessorParameter*>& previousParameters, bool recursive) const
    {
        for (auto* child : children)
        {
            if (auto* parameter = child->getParameter())
                previousParameters.add (parameter);
            else if (recursive)
                child->getGroup()->getParameters (previousParameters, true);
        }
    }

    const AudioProcessorParameterGroup* getGroupForParameter (AudioProcessorParameter* parameter) const
    {
        for (auto* child : children)
        {
            if (child->getParameter() == parameter)
                return this;

            if (auto* group = child->getGroup())
                if (auto* foundGroup = group->getGroupForParameter (parameter))
                    return foundGroup;
        }

        return nullptr;
    }

    //==============================================================================
    const String identifier, name, separator;
    OwnedArray<const AudioProcessorParameterNode> children;
    AudioProcessorParameterGroup* parent = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioProcessorParameterGroup)
};

} // namespace juce
