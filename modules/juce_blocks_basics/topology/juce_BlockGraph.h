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

#pragma once

namespace juce
{

/**
    Represents traversal paths from master blocks and any connected blocks.

    @tags{Blocks}
*/
class BlockGraph
{
public:
    /** Creates a BlockGraph object from a BlockTopology with an optional filter function. This
        will build a block graph of traversal paths for each master.
    */
    BlockGraph (const BlockTopology topology, std::function<bool (Block::Ptr block)> filter = nullptr);
    BlockGraph (BlockGraph&&);

    using BlockTraversalPaths = Array<Block::Array, CriticalSection>;

    /** Get traversal paths for each master block in a topology */
    BlockTraversalPaths getTraversalPaths() const;

    /** Get traversal path for a specific master block in a topology */
    Block::Array getTraversalPathFromMaster (Block::Ptr masterBlock) const;

    /** Gets a string representation of all traversal paths */
    String asString() const;

private:
    void buildGraph();
    Block::Array buildPathFromMaster (Block::Ptr masterBlock);
    void addAllConnectedToArray (Block::Ptr startBlock, Block::Array& store);
    bool shouldIncludeBlock (Block::Ptr block) const;

    BlockTraversalPaths traversalPaths; // one path for each master block
    BlockTopology topology;
    std::function<bool (Block::Ptr)> filter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BlockGraph)
};

} // namespace juce
