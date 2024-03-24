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

namespace ValueTreeSynchroniserHelpers
{
    enum ChangeType
    {
        propertyChanged  = 1,
        fullSync         = 2,
        childAdded       = 3,
        childRemoved     = 4,
        childMoved       = 5,
        propertyRemoved  = 6
    };

    static void getValueTreePath (ValueTree v, const ValueTree& topLevelTree, Array<int>& path)
    {
        while (v != topLevelTree)
        {
            ValueTree parent (v.getParent());

            if (! parent.isValid())
                break;

            path.add (parent.indexOf (v));
            v = parent;
        }
    }

    static void writeHeader (MemoryOutputStream& stream, ChangeType type)
    {
        stream.writeByte ((char) type);
    }

    static void writeHeader (ValueTreeSynchroniser& target, MemoryOutputStream& stream,
                             ChangeType type, ValueTree v)
    {
        writeHeader (stream, type);

        Array<int> path;
        getValueTreePath (v, target.getRoot(), path);

        stream.writeCompressedInt (path.size());

        for (int i = path.size(); --i >= 0;)
            stream.writeCompressedInt (path.getUnchecked (i));
    }

    static ValueTree readSubTreeLocation (MemoryInputStream& input, ValueTree v)
    {
        const int numLevels = input.readCompressedInt();

        if (! isPositiveAndBelow (numLevels, 65536)) // sanity-check
            return {};

        for (int i = numLevels; --i >= 0;)
        {
            const int index = input.readCompressedInt();

            if (! isPositiveAndBelow (index, v.getNumChildren()))
                return {};

            v = v.getChild (index);
        }

        return v;
    }
}

ValueTreeSynchroniser::ValueTreeSynchroniser (const ValueTree& tree)  : valueTree (tree)
{
    valueTree.addListener (this);
}

ValueTreeSynchroniser::~ValueTreeSynchroniser()
{
    valueTree.removeListener (this);
}

void ValueTreeSynchroniser::sendFullSyncCallback()
{
    MemoryOutputStream m;
    writeHeader (m, ValueTreeSynchroniserHelpers::fullSync);
    valueTree.writeToStream (m);
    stateChanged (m.getData(), m.getDataSize());
}

void ValueTreeSynchroniser::valueTreePropertyChanged (ValueTree& vt, const Identifier& property)
{
    MemoryOutputStream m;

    if (auto* value = vt.getPropertyPointer (property))
    {
        ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::propertyChanged, vt);
        m.writeString (property.toString());
        value->writeToStream (m);
    }
    else
    {
        ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::propertyRemoved, vt);
        m.writeString (property.toString());
    }

    stateChanged (m.getData(), m.getDataSize());
}

void ValueTreeSynchroniser::valueTreeChildAdded (ValueTree& parentTree, ValueTree& childTree)
{
    const int index = parentTree.indexOf (childTree);
    jassert (index >= 0);

    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childAdded, parentTree);
    m.writeCompressedInt (index);
    childTree.writeToStream (m);
    stateChanged (m.getData(), m.getDataSize());
}

void ValueTreeSynchroniser::valueTreeChildRemoved (ValueTree& parentTree, ValueTree&, int oldIndex)
{
    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childRemoved, parentTree);
    m.writeCompressedInt (oldIndex);
    stateChanged (m.getData(), m.getDataSize());
}

void ValueTreeSynchroniser::valueTreeChildOrderChanged (ValueTree& parent, int oldIndex, int newIndex)
{
    MemoryOutputStream m;
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::childMoved, parent);
    m.writeCompressedInt (oldIndex);
    m.writeCompressedInt (newIndex);
    stateChanged (m.getData(), m.getDataSize());
}

bool ValueTreeSynchroniser::applyChange (ValueTree& root, const void* data, size_t dataSize, UndoManager* undoManager)
{
    MemoryInputStream input (data, dataSize, false);

    const ValueTreeSynchroniserHelpers::ChangeType type = (ValueTreeSynchroniserHelpers::ChangeType) input.readByte();

    if (type == ValueTreeSynchroniserHelpers::fullSync)
    {
        root = ValueTree::readFromStream (input);
        return true;
    }

    ValueTree v (ValueTreeSynchroniserHelpers::readSubTreeLocation (input, root));

    if (! v.isValid())
        return false;

    switch (type)
    {
        case ValueTreeSynchroniserHelpers::propertyChanged:
        {
            Identifier property (input.readString());
            v.setProperty (property, var::readFromStream (input), undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::propertyRemoved:
        {
            Identifier property (input.readString());
            v.removeProperty (property, undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::childAdded:
        {
            const int index = input.readCompressedInt();
            v.addChild (ValueTree::readFromStream (input), index, undoManager);
            return true;
        }

        case ValueTreeSynchroniserHelpers::childRemoved:
        {
            const int index = input.readCompressedInt();

            if (isPositiveAndBelow (index, v.getNumChildren()))
            {
                v.removeChild (index, undoManager);
                return true;
            }

            jassertfalse; // Either received some corrupt data, or the trees have drifted out of sync
            break;
        }

        case ValueTreeSynchroniserHelpers::childMoved:
        {
            const int oldIndex = input.readCompressedInt();
            const int newIndex = input.readCompressedInt();

            if (isPositiveAndBelow (oldIndex, v.getNumChildren())
                 && isPositiveAndBelow (newIndex, v.getNumChildren()))
            {
                v.moveChild (oldIndex, newIndex, undoManager);
                return true;
            }

            jassertfalse; // Either received some corrupt data, or the trees have drifted out of sync
            break;
        }

        case ValueTreeSynchroniserHelpers::fullSync:
            break;

        default:
            jassertfalse; // Seem to have received some corrupt data?
            break;
    }

    return false;
}

} // namespace juce
