/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2020 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

BlockGraph::BlockGraph (const BlockTopology t, std::function<bool (Block::Ptr)> filterIn)
    : topology (t), filter (std::move (filterIn))
{
    buildGraph();
}

BlockGraph::BlockGraph (BlockGraph&& other)
{
    traversalPaths = std::move (other.traversalPaths);
    topology = std::move (other.topology);
    filter = std::move (other.filter);
}

BlockGraph::BlockTraversalPaths BlockGraph::getTraversalPaths() const
{
    return traversalPaths;
}

Block::Array BlockGraph::getTraversalPathFromMaster (Block::Ptr masterBlock) const
{
    for (const auto& path : traversalPaths)
    {
        if (! path.isEmpty() && path[0] == masterBlock)
            return path;
    }

    return {};
}

String BlockGraph::asString() const
{
    String outputString = "Traversal Path(s):";

    for (const auto& path : traversalPaths)
    {
        outputString += "\n[master]-->";

        for (auto& block : path)
            outputString += block->serialNumber + "-->";

        outputString += "[last]";
    }

    return outputString;
}

void BlockGraph::buildGraph()
{
    traversalPaths.clear();

    for (const auto block : topology.blocks)
    {
        if (block->isMasterBlock() && shouldIncludeBlock (block))
            traversalPaths.add (buildPathFromMaster (block));
    }
}

bool BlockGraph::shouldIncludeBlock (Block::Ptr block) const
{
    if (filter == nullptr)
        return true;

    return filter (block);
}

Block::Array BlockGraph::buildPathFromMaster (Block::Ptr masterBlock)
{
    jassert (masterBlock->isMasterBlock());

    Block::Array orderedBlockList;
    addAllConnectedToArray (masterBlock, orderedBlockList);

    return orderedBlockList;
}

void BlockGraph::addAllConnectedToArray (Block::Ptr startBlock, Block::Array& store)
{
    store.addIfNotAlreadyThere (startBlock);

    for (const auto block : topology.getDirectlyConnectedBlocks (startBlock->uid))
    {
        if (shouldIncludeBlock (block) && store.addIfNotAlreadyThere (block))
            addAllConnectedToArray (block, store);
    }
}

} // namespace juce
