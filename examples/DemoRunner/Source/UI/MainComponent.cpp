/*
  ==============================================================================

   This file is part of the JUCE framework examples.
   Copyright (c) Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   to use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
   REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
   AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
   INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
   LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
   OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
   PERFORMANCE OF THIS SOFTWARE.

  ==============================================================================
*/

#include "MainComponent.h"

//==============================================================================
struct SidePanelHeader final : public Component
{
    SidePanelHeader (MainComponent& o)
        : owner (o)
    {
        setOpaque (true);

        static const unsigned char homeIconPathData[]
            = { 110,109,0,0,64,65,0,0,64,64,98,0,0,64,65,0,0,64,64,74,12,186,64,164,112,5,65,24,217,22,64,70,182,
                51,65,98,35,219,9,64,240,167,54,65,0,0,0,64,170,241,58,65,0,0,0,64,0,0,64,65,98,0,0,0,64,22,217,
                72,65,166,155,28,64,0,0,80,65,0,0,64,64,0,0,80,65,108,0,0,160,64,0,0,80,65,108,0,0,160,64,0,0,
                160,65,98,0,0,160,64,139,108,164,65,211,77,174,64,0,0,168,65,0,0,192,64,0,0,168,65,108,0,0,16,65,
                0,0,168,65,98,22,217,24,65,0,0,168,65,0,0,32,65,127,106,164,65,0,0,32,65,0,0,160,65,108,0,0,32,65,0,0,128,
                65,108,0,0,96,65,0,0,128,65,108,0,0,96,65,0,0,160,65,98,0,0,96,65,127,106,164,65,233,38,103,65,0,0,168,
                65,0,0,112,65,0,0,168,65,108,0,0,144,65,0,0,168,65,98,139,108,148,65,0,0,168,65,0,0,152,65,139,108,164,
                65,0,0,152,65,0,0,160,65,108,0,0,152,65,0,0,80,65,108,0,0,168,65,0,0,80,65,98,139,108,172,65,0,0,80,65,0,
                0,176,65,23,217,72,65,0,0,176,65,0,0,64,65,98,0,0,176,65,170,241,58,65,156,196,174,65,240,167,54,65,158,239,
                172,65,70,182,51,65,98,213,120,145,65,164,112,5,65,0,0,64,65,0,0,64,64,0,0,64,65,0,0,64,64,99,
                101,0,0 };

        static const unsigned char settingsIconPathData[]
            = { 110,109,202,111,210,64,243,226,61,64,108,0,0,224,64,0,0,0,0,108,0,0,48,65,0,0,0,0,108,27,200,54,65,243,
                226,61,64,98,91,248,63,65,174,170,76,64,95,130,72,65,231,138,96,64,46,46,80,65,180,163,120,64,108,42,
                181,124,65,20,38,49,64,108,149,90,142,65,246,108,199,64,108,68,249,118,65,2,85,1,65,98,112,166,119,65,
                201,31,6,65,0,0,120,65,111,5,11,65,0,0,120,65,0,0,16,65,98,0,0,120,65,145,250,20,65,108,166,119,65,55,
                224,25,65,72,249,118,65,254,170,30,65,108,151,90,142,65,133,73,60,65,108,46,181,124,65,123,182,115,65,
                108,50,46,80,65,18,215,97,65,98,99,130,72,65,70,221,103,65,96,248,63,65,83,213,108,65,32,200,54,65,66,
                135,112,65,108,0,0,48,65,0,0,144,65,108,0,0,224,64,0,0,144,65,108,202,111,210,64,67,135,112,65,98,74,
                15,192,64,84,213,108,65,65,251,174,64,70,221,103,65,164,163,159,64,19,215,97,65,108,92,43,13,64,123,182,
                115,65,108,187,181,82,62,133,73,60,65,108,244,26,36,64,254,170,30,65,98,64,102,33,64,55,224,25,5,0,0,32,
                64,145,250,20,65,0,0,32,64,0,0,16,65,98,0,0,32,64,111,5,11,65,64,102,33,64,201,31,6,65,244,26,36,64,2,85,
                1,65,108,187,181,82,62,246,108,199,64,108,92,43,13,64,20,38,49,64,108,164,163,159,64,180,163,120,64,98,65,
                251,174,64,231,138,96,64,74,15,192,64,175,170,76,64,202,111,210,64,243,226,61,64,99,109,0,0,16,65,0,0,64,
                65,98,121,130,42,65,0,0,64,65,0,0,64,65,121,130,42,65,0,0,64,65,0,0,16,65,98,0,0,64,65,13,251,234,64,121,
                130,42,65,0,0,192,64,0,0,16,65,0,0,192,64,98,13,251,234,64,0,0,192,64,0,0,192,64,13,251,234,64,0,0,192,64,
                0,0,16,65,98,0,0,192,64,121,130,42,65,13,251,234,64,0,0,64,65,0,0,16,65,0,0,64,65,99,101,0,0 };

        Path p;
        p.loadPathFromData (homeIconPathData, sizeof (homeIconPathData));
        homeButton.setShape (p, true, true, false);

        p.clear();
        p.loadPathFromData (settingsIconPathData, sizeof (settingsIconPathData));
        settingsButton.setShape (p, true, true, false);

        titleLabel.setText (owner.getSidePanel().getTitleText(), NotificationType::dontSendNotification);
        addAndMakeVisible (titleLabel);

        homeButton.onClick = [this] { owner.homeButtonClicked(); };
        addAndMakeVisible (homeButton);

        addAndMakeVisible (settingsButton);
        settingsButton.onClick = [this] { owner.settingsButtonClicked(); };

        updateLookAndFeel();
    }

    void paint (Graphics& g) override
    {
        g.fillAll (owner.getSidePanel().findColour (SidePanel::backgroundColour));
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        auto buttonWidth = owner.getSidePanel().getTitleBarHeight();

        bounds.removeFromLeft (10);
        homeButton.setBounds (bounds.removeFromLeft (buttonWidth).reduced (7));
        settingsButton.setBounds (bounds.removeFromLeft (buttonWidth).reduced (7));
        bounds.removeFromLeft (10);

        bounds.removeFromRight (10);
        titleLabel.setBounds (bounds);
    }

    void updateLookAndFeel()
    {
        auto& sidePanel = owner.getSidePanel();
        auto& lf = sidePanel.getLookAndFeel();

        titleLabel.setFont (lf.getSidePanelTitleFont (sidePanel));
        titleLabel.setJustificationType (lf.getSidePanelTitleJustification (sidePanel));
        titleLabel.setColour (Label::textColourId, owner.findColour (SidePanel::titleTextColour));

        auto normal = sidePanel.findColour (SidePanel::dismissButtonNormalColour);
        auto over = sidePanel.findColour (SidePanel::dismissButtonOverColour);
        auto down = sidePanel.findColour (SidePanel::dismissButtonDownColour);

        homeButton.setColours (normal, over, down);
        settingsButton.setColours (normal, over, down);

    }

    void lookAndFeelChanged() override
    {
        updateLookAndFeel();
    }

    MainComponent& owner;
    Label titleLabel;
    ShapeButton homeButton      { "Home", Colours::transparentBlack, Colours::transparentBlack, Colours::transparentBlack },
                settingsButton  { "Settings", Colours::transparentBlack, Colours::transparentBlack, Colours::transparentBlack };
};

//==============================================================================
class DemoList final : public Component,
                       public ListBoxModel
{
public:
    DemoList (DemoContentComponent& holder)
        : demoHolder (holder)
    {
        addAndMakeVisible (demos);
        demos.setModel (this);
        demos.setRowHeight (40);
    }

    void resized() override
    {
        demos.setBounds (getLocalBounds());
    }

    //==============================================================================
    void paintListBoxItem (int rowNumber, Graphics& g, int width, int height, bool rowIsSelected) override
    {
        Rectangle<int> bounds (0, 0, width, height);

        auto textColour = findColour (Label::textColourId);

        g.setColour (textColour.withAlpha (0.4f));

        if (rowNumber == 0)
            g.fillRect (bounds.removeFromTop (2).reduced (7, 0));

        g.fillRect (bounds.removeFromBottom (2).reduced (7, 0));

        if (rowIsSelected)
        {
            g.setColour (findColour (TextEditor::highlightColourId).withAlpha (0.4f));
            g.fillRect (bounds);
            textColour = findColour (TextEditor::highlightedTextColourId);
        }

        g.setColour (textColour);
        g.drawFittedText (getNameForRow (rowNumber), bounds, Justification::centred, 1);
    }

    int getNumRows() override
    {
        return (int) (selectedCategory.isEmpty() ? JUCEDemos::getCategories().size()
                                                 : JUCEDemos::getCategory (selectedCategory).demos.size());
    }

    String getNameForRow (int rowNumber) override
    {
        if (selectedCategory.isEmpty())
        {
            if (isPositiveAndBelow (rowNumber, JUCEDemos::getCategories().size()))
                return JUCEDemos::getCategories()[(size_t) rowNumber].name;
        }
        else
        {
            auto& category = JUCEDemos::getCategory (selectedCategory);

            if (isPositiveAndBelow (rowNumber, category.demos.size()))
                return category.demos[(size_t) rowNumber].demoFile.getFileName();
        }

        return {};
    }

    void returnKeyPressed (int row) override                       { selectRow (row); }
    void listBoxItemClicked (int row, const MouseEvent&) override  { selectRow (row); }

    //==============================================================================
    void showCategory (const String& categoryName) noexcept
    {
        selectedCategory = categoryName;

        demos.deselectAllRows();
        demos.setHeaderComponent (categoryName.isEmpty() ? nullptr
                                                         : std::make_unique<CategoryListHeaderComponent> (*this));
        demos.updateContent();
    }

private:
    //==============================================================================
    class CategoryListHeaderComponent final : public Button
    {
    public:
        explicit CategoryListHeaderComponent (DemoList& o)
            : Button ({}),
              owner (o)
        {
            setTitle ("Previous");
            setSize (0, 30);
        }

        void paintButton (Graphics& g, bool, bool) override
        {
            g.setColour (findColour (Label::textColourId));
            g.drawFittedText ("<", getLocalBounds().reduced (20, 0), Justification::centredLeft, 1);
        }

        void clicked() override
        {
            owner.showCategory ({});
        }

        using Button::clicked;

    private:
        DemoList& owner;
    };

    //==============================================================================
    void selectRow (int row)
    {
        if (row < 0)
            return;

        if (selectedCategory.isEmpty())
            showCategory (JUCEDemos::getCategories()[(size_t) row].name);
        else
            demoHolder.setDemo (selectedCategory, row);

        if (demos.isShowing())
            selectFirstRow();
    }

    void selectFirstRow()
    {
        if (auto* handler = demos.getAccessibilityHandler())
        {
            for (auto* child : handler->getChildren())
            {
                if (child->getRole() == AccessibilityRole::listItem)
                {
                    child->grabFocus();
                    break;
                }
            }
        }
    }

    //==============================================================================
    String selectedCategory;

    DemoContentComponent& demoHolder;
    ListBox demos;
};

//==============================================================================
MainComponent::MainComponent()
{
    contentComponent.reset (new DemoContentComponent (*this, [this] (bool isHeavyweight)
    {
        demosPanel.showOrHide (false);

        if (isHeavyweight)
        {
           #if JUCE_MAC && USE_COREGRAPHICS_RENDERING
            setRenderingEngine (1);
           #elif ! JUCE_WINDOWS
            setRenderingEngine (0);
           #endif
        }

        isShowingHeavyweightDemo = isHeavyweight;
        resized();
    }));

    demosPanel.setContent (new DemoList (*contentComponent));
    demosPanel.setTitleBarComponent (new SidePanelHeader (*this), true);

    addAndMakeVisible (contentComponent.get());
    addAndMakeVisible (showDemosButton);
    addAndMakeVisible (demosPanel);

    demosPanel.setTitle ("Demos");
    demosPanel.setFocusContainerType (FocusContainerType::focusContainer);

    showDemosButton.onClick = [this] { demosPanel.showOrHide (true); };

    demosPanel.onPanelMove = [this]
    {
        sidePanelWidth = jmax (0, demosPanel.getRight());

        if (isShowingHeavyweightDemo)
            resized();
    };

    demosPanel.onPanelShowHide = [this] (bool isShowing)
    {
        if (isShowing)
        {
            sidePanelWidth = jmax (0, demosPanel.getWidth());

            if (isShowingHeavyweightDemo)
                resized();

            if (auto* handler = demosPanel.getAccessibilityHandler())
                handler->grabFocus();
        }
        else
        {
            sidePanelWidth = 0;

            if (isShowingHeavyweightDemo)
                Timer::callAfterDelay (250, [this] { resized(); });
        }
    };

    contentComponent->showHomeScreen();

    setOpaque (true);
    setSize (800, 800);
}

MainComponent::~MainComponent()
{
    contentComponent->clearCurrentDemo();
}

//==============================================================================
void MainComponent::paint (Graphics& g)
{
    g.fillAll (findColour (ResizableWindow::backgroundColourId));
}

void MainComponent::resized()
{
    auto safeBounds = [this]
    {
        auto bounds = getLocalBounds();

       #if JUCE_IOS || JUCE_ANDROID
        if (auto* display = Desktop::getInstance().getDisplays().getDisplayForRect (getScreenBounds()))
            return display->safeAreaInsets.subtractedFrom (display->keyboardInsets.subtractedFrom (bounds));
       #endif

        return bounds;
    }();

    showDemosButton.setBounds (safeBounds.getX(), safeBounds.getY(), 150, contentComponent->getTabBarDepth());

    if (isShowingHeavyweightDemo)
    {
        safeBounds.removeFromLeft (sidePanelWidth);
        contentComponent->setTabBarIndent (jmax (0, 150 - sidePanelWidth));
    }
    else
    {
        contentComponent->setTabBarIndent (150);
    }

    contentComponent->setBounds (safeBounds);
}

void MainComponent::homeButtonClicked()
{
    if (auto* list = dynamic_cast<DemoList*> (demosPanel.getContent()))
        list->showCategory ({});

    if (contentComponent != nullptr)
    {
        if (contentComponent->isShowingHomeScreen())
            return;

        contentComponent->showHomeScreen();

        if (isShowingHeavyweightDemo)
        {
            isShowingHeavyweightDemo = false;
            resized();
        }
    }
}

void MainComponent::settingsButtonClicked()
{
    if (contentComponent != nullptr)
        contentComponent->setCurrentTabIndex (2);
}

void MainComponent::setRenderingEngine (int renderingEngineIndex)
{
    if (renderingEngineIndex != currentRenderingEngineIdx)
        updateRenderingEngine (renderingEngineIndex);
}

void MainComponent::parentHierarchyChanged()
{
    auto* newPeer = getPeer();

    if (peer != newPeer)
    {
        peer = newPeer;

        auto previousRenderingEngine = renderingEngines[currentRenderingEngineIdx];

        renderingEngines.clear();
        if (peer != nullptr)
            renderingEngines = peer->getAvailableRenderingEngines();

        renderingEngines.add ("OpenGL Renderer");

        currentRenderingEngineIdx = renderingEngines.indexOf (previousRenderingEngine);

        if (currentRenderingEngineIdx < 0)
        {
           #if JUCE_ANDROID
            currentRenderingEngineIdx = (renderingEngines.size() - 1);
           #else
            currentRenderingEngineIdx = peer->getCurrentRenderingEngine();
           #endif
        }

        updateRenderingEngine (currentRenderingEngineIdx);
    }
}

void MainComponent::updateRenderingEngine (int renderingEngineIndex)
{
    if (renderingEngineIndex == (renderingEngines.size() - 1))
    {
        if (isShowingHeavyweightDemo)
            return;

        openGLContext.attachTo (*getTopLevelComponent());
    }
    else
    {
        openGLContext.detach();
        peer->setCurrentRenderingEngine (renderingEngineIndex);
    }

    currentRenderingEngineIdx = renderingEngineIndex;
}
