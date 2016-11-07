
#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "BlockComponents.h"

/**
    The main component where the Block components will be displayed
*/
class MainComponent   : public Component,
                        public TopologySource::Listener,
                        private Timer
{
public:
    MainComponent()
    {
        setSize (600, 600);

        noBlocksLabel.setText ("No BLOCKS connected...", dontSendNotification);
        noBlocksLabel.setJustificationType (Justification::centred);
        addAndMakeVisible (noBlocksLabel);

        // Register MainComponent as a listener to the PhysicalTopologySource object
        topologySource.addListener (this);

        startTimer (10000);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (Colours::lightgrey);
    }

    void resized() override
    {
        noBlocksLabel.setVisible (false);
        const int numBlockComponents = blockComponents.size();

        // If there are no currently connected Blocks then display some text on the screen
        if (numBlockComponents == 0)
        {
            noBlocksLabel.setVisible (true);
            noBlocksLabel.setBounds (0, (getHeight() / 2) - 50, getWidth(), 100);
            return;
        }

        // Work out the maximum diplay area for each Block
        const Rectangle<int> bounds = getLocalBounds().reduced (20);

        auto squareRoot = sqrt (numBlockComponents);
        int gridSize = (int)squareRoot;

        if (squareRoot - gridSize > 0)
            gridSize++;

        int sideLength = bounds.getWidth() / gridSize;

        int xCounter = 0;
        int yCounter = 0;
        bool hasSpaceForControlBlock = false;
        Rectangle<int> lastControlBlockBounds;

        for (auto block : blockComponents)
        {
            Rectangle<int> blockBounds;
            const Block::Type type = block->block->getType();

            // Can fit 2 ControlBlockComponents in the space of one LightpadBlockComponent
            if (type == Block::liveBlock || type == Block::loopBlock)
            {
                if (hasSpaceForControlBlock)
                {
                    blockBounds = lastControlBlockBounds.withY (lastControlBlockBounds.getY() + (int)(sideLength * 0.5));
                    hasSpaceForControlBlock = false;
                }
                else
                {
                    blockBounds = Rectangle<int> (bounds.getX() + (xCounter * sideLength), bounds.getY() + (yCounter * sideLength),
                                                  sideLength, (int)(sideLength * 0.5));
                    hasSpaceForControlBlock = true;
                    lastControlBlockBounds = blockBounds;
                }
            }
            else
            {
                blockBounds = Rectangle<int> (bounds.getX() + (xCounter * sideLength), bounds.getY() + (yCounter * sideLength),
                                              sideLength, sideLength);
            }

            block->setBounds (blockBounds.reduced (5));

            if (++xCounter >= gridSize)
            {
                yCounter++;
                xCounter = 0;
            }
        }
    }

    /** Overridden from TopologySource::Listener, called when the topology changes */
    void topologyChanged() override
    {
        // Clear the array of Block components
        blockComponents.clear();

        // Get the array of currently connected Block objects from the PhysicalTopologySource
        Block::Array blocksArray = topologySource.getCurrentTopology().blocks;

        // Create a BlockComponent object for each Block object
        for (auto& block : blocksArray)
            if (BlockComponent* blockComponent = createBlockComponent (block))
                addAndMakeVisible (blockComponents.add (blockComponent));

        // Update the display
        resized();
    }

private:
    /** Creates a BlockComponent object for a new Block and adds it to the content component */
    BlockComponent* createBlockComponent (Block::Ptr newBlock)
    {
        const Block::Type type = newBlock->getType();

        if (type == Block::lightPadBlock)
            return new LightpadComponent (newBlock);
        if (type == Block::loopBlock || type == Block::liveBlock)
            return new ControlBlockComponent (newBlock);

        // should only be connecting a Lightpad or Control Block!
        jassertfalse;

        return nullptr;
    }

    /** Periodically updates the displayed BlockComponent tooltips */
    void timerCallback() override
    {
        for (auto c : blockComponents)
            c->updateStatsAndTooltip();
    }

    //==============================================================================
    PhysicalTopologySource topologySource;
    OwnedArray<BlockComponent> blockComponents;

    Label noBlocksLabel;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};


#endif  // MAINCOMPONENT_H_INCLUDED
