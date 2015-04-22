/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2013 - Raw Material Software Ltd.

   Permission is granted to use this software under the terms of either:
   a) the GPL v2 (or any later version)
   b) the Affero GPL v3

   Details of these licenses can be found at: www.gnu.org/licenses

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

   ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.juce.com for more information.

  ==============================================================================
*/

namespace ValueTreeSynchroniserHelpers
{
    enum ChangeType
    {
        propertyChanged  = 1,
        fullSync         = 2,
        childAdded       = 3,
        childRemoved     = 4,
        childMoved       = 5
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
            stream.writeCompressedInt (path.getUnchecked(i));
    }

    static ValueTree readSubTreeLocation (MemoryInputStream& input, ValueTree v)
    {
        const int numLevels = input.readCompressedInt();

        if (! isPositiveAndBelow (numLevels, 65536)) // sanity-check
            return ValueTree();

        for (int i = numLevels; --i >= 0;)
        {
            const int index = input.readCompressedInt();

            if (! isPositiveAndBelow (index, v.getNumChildren()))
                return ValueTree();

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
    ValueTreeSynchroniserHelpers::writeHeader (*this, m, ValueTreeSynchroniserHelpers::propertyChanged, vt);
    m.writeString (property.toString());
    vt.getProperty (property).writeToStream (m);
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

void ValueTreeSynchroniser::valueTreeParentChanged (ValueTree&)  {} // (No action needed here)

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

        default:
            jassertfalse; // Seem to have received some corrupt data?
            break;
    }

    return false;
}
