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

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "BlockComponents.h"

/**
    The main component where the Block components will be displayed
*/
class MainComponent   : public Component,
                        public TopologySource::Listener,
                        private Timer,
                        private Button::Listener
{
public:
    MainComponent()
    {
        setSize (600, 600);

        noBlocksLabel.setText ("No BLOCKS connected...", dontSendNotification);
        noBlocksLabel.setJustificationType (Justification::centred);

        zoomOutButton.setButtonText ("+");
        zoomOutButton.addListener (this);
        zoomOutButton.setAlwaysOnTop (true);

        zoomInButton.setButtonText ("-");
        zoomInButton.addListener (this);
        zoomInButton.setAlwaysOnTop (true);

        // Register MainComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

        startTimer (10000);

        addAndMakeVisible (noBlocksLabel);
        addAndMakeVisible (zoomOutButton);
        addAndMakeVisible (zoomInButton);

       #if JUCE_IOS
        connectButton.setButtonText ("Connect");
        connectButton.addListener (this);
        connectButton.setAlwaysOnTop (true);
        addAndMakeVisible (connectButton);
       #endif
    }

    void paint (Graphics& g) override
    {
    }

    void resized() override
    {
       #if JUCE_IOS
        connectButton.setBounds (getRight() - 100, 20, 80, 30);
       #endif

        noBlocksLabel.setVisible (false);
        const int numBlockComponents = blockComponents.size();

        // If there are no currently connected Blocks then display some text on the screen
        if (numBlockComponents == 0)
        {
            noBlocksLabel.setVisible (true);
            noBlocksLabel.setBounds (0, (getHeight() / 2) - 50, getWidth(), 100);
            return;
        }

        zoomOutButton.setBounds (10, getHeight() - 40, 40, 30);
        zoomInButton.setBounds  (zoomOutButton.getRight(), zoomOutButton.getY(), 40, 30);

        if (isInitialResized)
        {
            // Work out the area needed in terms of Block units
            Rectangle<float> maxArea;
            for (auto blockComponent : blockComponents)
            {
                auto topLeft = blockComponent->topLeft;
                int rotation = blockComponent->rotation;
                int blockSize = 0;

                if (rotation == 180)
                    blockSize = blockComponent->block->getWidth();
                else if (rotation == 90)
                    blockSize = blockComponent->block->getHeight();

                if (topLeft.x - blockSize < maxArea.getX())
                    maxArea.setX (topLeft.x - blockSize);

                blockSize = 0;
                if (rotation == 0)
                    blockSize = blockComponent->block->getWidth();
                else if (rotation == 270)
                    blockSize = blockComponent->block->getHeight();

                if (topLeft.x + blockSize > maxArea.getRight())
                    maxArea.setWidth (topLeft.x + blockSize);

                blockSize = 0;
                if (rotation == 180)
                    blockSize = blockComponent->block->getHeight();
                else if (rotation == 270)
                    blockSize = blockComponent->block->getWidth();

                if (topLeft.y - blockSize < maxArea.getY())
                    maxArea.setY (topLeft.y - blockSize);

                blockSize = 0;
                if (rotation == 0)
                    blockSize = blockComponent->block->getHeight();
                else if (rotation == 90)
                    blockSize = blockComponent->block->getWidth();

                if (topLeft.y + blockSize > maxArea.getBottom())
                    maxArea.setHeight (topLeft.y + blockSize);
            }

            float totalWidth  = std::abs (maxArea.getX()) + maxArea.getWidth();
            float totalHeight = std::abs (maxArea.getY()) + maxArea.getHeight();

            blockUnitInPixels = static_cast<int> (jmin ((getHeight() / totalHeight) - 50, (getWidth() / totalWidth) - 50));

            masterBlockComponent->centreWithSize (masterBlockComponent->block->getWidth()  * blockUnitInPixels,
                                                  masterBlockComponent->block->getHeight() * blockUnitInPixels);

            isInitialResized = false;
        }
        else
        {
            masterBlockComponent->setSize (masterBlockComponent->block->getWidth() * blockUnitInPixels, masterBlockComponent->block->getHeight() * blockUnitInPixels);
        }

        for (auto blockComponent : blockComponents)
        {
            if (blockComponent == masterBlockComponent)
                continue;

            blockComponent->setBounds (masterBlockComponent->getX() + static_cast<int> (blockComponent->topLeft.x * blockUnitInPixels),
                                       masterBlockComponent->getY() + static_cast<int> (blockComponent->topLeft.y * blockUnitInPixels),
                                       blockComponent->block->getWidth()  * blockUnitInPixels,
                                       blockComponent->block->getHeight() * blockUnitInPixels);

            if (blockComponent->rotation != 0)
                blockComponent->setTransform (AffineTransform::rotation (blockComponent->rotation * (static_cast<float> (double_Pi) / 180.0f),
                                                                         static_cast<float> (blockComponent->getX()),
                                                                         static_cast<float> (blockComponent->getY())));
        }
    }

    /** Overridden from TopologySource::Listener, called when the topology changes */
    void topologyChanged() override
    {
        // Clear the array of Block components
        blockComponents.clear();
        masterBlockComponent = nullptr;

        // Get the current topology
        auto topology = topologySource.getCurrentTopology();

        // Create a BlockComponent object for each Block object and store a pointer to the master
        for (auto& block : topology.blocks)
        {
            if (auto* blockComponent = createBlockComponent (block))
            {
                addAndMakeVisible (blockComponents.add (blockComponent));

                if (blockComponent->block->isMasterBlock())
                    masterBlockComponent = blockComponent;
            }
        }

        // Must have a master Block!
        if (topology.blocks.size() > 0)
            jassert (masterBlockComponent != nullptr);

        // Calculate the relative position and rotation for each Block
        positionBlocks (topology);

        // Update the display
        isInitialResized = true;
        resized();
    }

private:
    /** Creates a BlockComponent object for a new Block and adds it to the content component */
    BlockComponent* createBlockComponent (Block::Ptr newBlock)
    {
        auto type = newBlock->getType();

        if (type == Block::lightPadBlock)
            return new LightpadComponent (newBlock);

        if (type == Block::loopBlock || type == Block::liveBlock)
            return new ControlBlockComponent (newBlock);

        // Should only be connecting a Lightpad or Control Block!
        jassertfalse;
        return nullptr;
    }

    /** Periodically updates the displayed BlockComponent tooltips */
    void timerCallback() override
    {
        for (auto c : blockComponents)
            c->updateStatsAndTooltip();
    }

    /** Zooms the display in or out */
    void buttonClicked (Button* button) override
    {
       #if JUCE_IOS
        if (button == &connectButton)
        {
            BluetoothMidiDevicePairingDialogue::open();
            return;
        }
       #endif

        if (button == &zoomOutButton || button == &zoomInButton)
        {
            blockUnitInPixels *= (button == &zoomOutButton ? 1.05f : 0.95f);
            resized();
        }
    }

    /** Calculates the position and rotation of each connected Block relative to the master Block */
    void positionBlocks (BlockTopology topology)
    {
        Array<BlockComponent*> blocksConnectedToMaster;

        float maxDelta = std::numeric_limits<float>::max();
        int maxLoops = 50;

        // Store all the connections to the master Block
        Array<BlockDeviceConnection> masterBlockConnections;
        for (auto connection : topology.connections)
            if (connection.device1 == masterBlockComponent->block->uid || connection.device2 == masterBlockComponent->block->uid)
                masterBlockConnections.add (connection);

        // Position all the Blocks that are connected to the master Block
        while (maxDelta > 0.001f && --maxLoops)
        {
            maxDelta = 0.0f;

            // Loop through each connection on the master Block
            for (auto connection : masterBlockConnections)
            {
                // Work out whether the master Block is device 1 or device 2 in the BlockDeviceConnection struct
                bool isDevice1 = true;
                if (masterBlockComponent->block->uid == connection.device2)
                    isDevice1 = false;

                // Get the connected ports
                auto masterPort = isDevice1 ? connection.connectionPortOnDevice1 : connection.connectionPortOnDevice2;
                auto otherPort  = isDevice1 ? connection.connectionPortOnDevice2 : connection.connectionPortOnDevice1;

                for (auto otherBlockComponent : blockComponents)
                {
                    // Get the other block
                    if (otherBlockComponent->block->uid == (isDevice1 ? connection.device2 : connection.device1))
                    {
                        blocksConnectedToMaster.addIfNotAlreadyThere (otherBlockComponent);

                        // Get the rotation of the other Block relative to the master Block
                        otherBlockComponent->rotation = getRotation (masterPort.edge, otherPort.edge);

                        // Get the offsets for the connected ports
                        auto masterBlockOffset = masterBlockComponent->getOffsetForPort (masterPort);
                        auto otherBlockOffset  = otherBlockComponent->topLeft + otherBlockComponent->getOffsetForPort (otherPort);

                        // Work out the distance between the two connected ports
                        auto delta = masterBlockOffset - otherBlockOffset;

                        // Move the other block half the distance to the connection
                        otherBlockComponent->topLeft += delta / 2.0f;

                        // Work out whether we are close enough for the loop to end
                        maxDelta = jmax (maxDelta, std::abs (delta.x), std::abs (delta.y));
                    }
                }
            }
        }

        // Check if there are any Blocks that have not been positioned yet
        Array<BlockComponent*> unpositionedBlocks;

        for (auto blockComponent : blockComponents)
            if (blockComponent != masterBlockComponent && ! blocksConnectedToMaster.contains (blockComponent))
                unpositionedBlocks.add (blockComponent);

        if (unpositionedBlocks.size() > 0)
        {
            // Reset the loop conditions
            maxDelta = std::numeric_limits<float>::max();
            maxLoops = 50;

            // Position all the remaining Blocks
            while (maxDelta > 0.001f && --maxLoops)
            {
                maxDelta = 0.0f;

                // Loop through each unpositioned Block
                for (auto blockComponent : unpositionedBlocks)
                {
                    // Store all the connections to this Block
                    Array<BlockDeviceConnection> blockConnections;
                    for (auto connection : topology.connections)
                        if (connection.device1 == blockComponent->block->uid || connection.device2 == blockComponent->block->uid)
                            blockConnections.add (connection);

                    // Loop through each connection on this Block
                    for (auto connection : blockConnections)
                    {
                        // Work out whether this Block is device 1 or device 2 in the BlockDeviceConnection struct
                        bool isDevice1 = true;
                        if (blockComponent->block->uid == connection.device2)
                            isDevice1 = false;

                        // Get the connected ports
                        auto thisPort  = isDevice1 ? connection.connectionPortOnDevice1 : connection.connectionPortOnDevice2;
                        auto otherPort = isDevice1 ? connection.connectionPortOnDevice2 : connection.connectionPortOnDevice1;

                        // Get the other Block
                        for (auto otherBlockComponent : blockComponents)
                        {
                            if (otherBlockComponent->block->uid == (isDevice1 ? connection.device2 : connection.device1))
                            {
                                // Get the rotation
                                int rotation = getRotation (otherPort.edge, thisPort.edge) + otherBlockComponent->rotation;
                                if (rotation > 360)
                                    rotation -= 360;

                                blockComponent->rotation = rotation;

                                // Get the offsets for the connected ports
                                auto otherBlockOffset = (otherBlockComponent->topLeft + otherBlockComponent->getOffsetForPort (otherPort));
                                auto thisBlockOffset  = (blockComponent->topLeft + blockComponent->getOffsetForPort (thisPort));

                                // Work out the distance between the two connected ports
                                auto delta = otherBlockOffset - thisBlockOffset;

                                // Move this block half the distance to the connection
                                blockComponent->topLeft += delta / 2.0f;

                                // Work out whether we are close enough for the loop to end
                                maxDelta = jmax (maxDelta, std::abs (delta.x), std::abs (delta.y));
                            }
                        }
                    }
                }
            }
        }
    }

    /** Returns a rotation in degrees based on the connected edges of two blocks */
    int getRotation (Block::ConnectionPort::DeviceEdge staticEdge, Block::ConnectionPort::DeviceEdge rotatedEdge)
    {
        using edge = Block::ConnectionPort::DeviceEdge;

        switch (staticEdge)
        {
            case edge::north:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 180;
                    case edge::south:
                        return 0;
                    case edge::east:
                        return 90;
                    case edge::west:
                        return 270;
                }
            }
            case edge::south:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 0;
                    case edge::south:
                        return 180;
                    case edge::east:
                        return 270;
                    case edge::west:
                        return 90;
                }
            }
            case edge::east:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 270;
                    case edge::south:
                        return 90;
                    case edge::east:
                        return 180;
                    case edge::west:
                        return 0;
                }
            }

            case edge::west:
            {
                switch (rotatedEdge)
                {
                    case edge::north:
                        return 90;
                    case edge::south:
                        return 270;
                    case edge::east:
                        return 0;
                    case edge::west:
                        return 180;
                }
            }
        }

        return 0;
    }

    //==============================================================================
    PhysicalTopologySource topologySource;
    OwnedArray<BlockComponent> blockComponents;
    BlockComponent* masterBlockComponent = nullptr;

    Label noBlocksLabel;

    TextButton zoomOutButton;
    TextButton zoomInButton;

    int blockUnitInPixels;
    bool isInitialResized;

   #if JUCE_IOS
    TextButton connectButton;
   #endif

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
