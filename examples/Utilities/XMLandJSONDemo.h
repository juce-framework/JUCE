/*
  ==============================================================================

   This file is part of the JUCE examples.
   Copyright (c) 2020 - Raw Material Software Limited

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   THE SOFTWARE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES,
   WHETHER EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR
   PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

/*******************************************************************************
 The block below describes the properties of this PIP. A PIP is a short snippet
 of code that can be read by the Projucer and used to generate a JUCE project.

 BEGIN_JUCE_PIP_METADATA

 name:             XMLandJSONDemo
 version:          1.0.0
 vendor:           JUCE
 website:          http://juce.com
 description:      Reads XML and JSON files.

 dependencies:     juce_core, juce_data_structures, juce_events, juce_graphics,
                   juce_gui_basics, juce_gui_extra
 exporters:        xcode_mac, vs2019, linux_make, androidstudio, xcode_iphone

 moduleFlags:      JUCE_STRICT_REFCOUNTEDPOINTER=1

 type:             Component
 mainClass:        XMLandJSONDemo

 useLocalCopy:     1

 END_JUCE_PIP_METADATA

*******************************************************************************/

#pragma once

#include "../Assets/DemoUtilities.h"

//==============================================================================
class XmlTreeItem  : public TreeViewItem
{
public:
    XmlTreeItem (XmlElement& x)  : xml (x)    {}

    String getUniqueName() const override
    {
        if (xml.getTagName().isEmpty())
            return "unknown";

        return xml.getTagName();
    }

    bool mightContainSubItems() override
    {
        return xml.getFirstChildElement() != nullptr;
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colours::blue.withAlpha (0.3f));

        // use a "colour" attribute in the xml tag for this node to set the text colour..
        g.setColour (Colour::fromString (xml.getStringAttribute ("colour", "ff000000")));
        g.setFont ((float) height * 0.7f);

        // draw the xml element's tag name..
        g.drawText (xml.getTagName(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                // create and add sub-items to this node of the tree, corresponding to
                // each sub-element in the XML..

                for (auto* child : xml.getChildIterator())
                    if (child != nullptr)
                        addSubItem (new XmlTreeItem (*child));
            }
        }
        else
        {
            // in this case, we'll leave any sub-items in the tree when the node gets closed,
            // though you could choose to delete them if that's more appropriate for
            // your application.
        }
    }

private:
    XmlElement& xml;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlTreeItem)
};

//==============================================================================
class JsonTreeItem  : public TreeViewItem
{
public:
    JsonTreeItem (Identifier i, var value)
        : identifier (i),
          json (value)
    {}

    String getUniqueName() const override
    {
        return identifier.toString() + "_id";
    }

    bool mightContainSubItems() override
    {
        if (auto* obj = json.getDynamicObject())
            return obj->getProperties().size() > 0;

        return json.isArray();
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colours::blue.withAlpha (0.3f));

        g.setColour (Colours::black);
        g.setFont ((float) height * 0.7f);

        // draw the element's tag name..
        g.drawText (getText(),
                    4, 0, width - 4, height,
                    Justification::centredLeft, true);
    }

    void itemOpennessChanged (bool isNowOpen) override
    {
        if (isNowOpen)
        {
            // if we've not already done so, we'll now add the tree's sub-items. You could
            // also choose to delete the existing ones and refresh them if that's more suitable
            // in your app.
            if (getNumSubItems() == 0)
            {
                // create and add sub-items to this node of the tree, corresponding to
                // the type of object this var represents

                if (json.isArray())
                {
                    for (int i = 0; i < json.size(); ++i)
                    {
                        auto& child = json[i];
                        jassert (! child.isVoid());
                        addSubItem (new JsonTreeItem ({}, child));
                    }
                }
                else if (auto* obj = json.getDynamicObject())
                {
                    auto& props = obj->getProperties();

                    for (int i = 0; i < props.size(); ++i)
                    {
                        auto id = props.getName (i);

                        auto child = props[id];
                        jassert (! child.isVoid());

                        addSubItem (new JsonTreeItem (id, child));
                    }
                }
            }
        }
        else
        {
            // in this case, we'll leave any sub-items in the tree when the node gets closed,
            // though you could choose to delete them if that's more appropriate for
            // your application.
        }
    }

private:
    Identifier identifier;
    var json;

    /** Returns the text to display in the tree.
        This is a little more complex for JSON than XML as nodes can be strings, objects or arrays.
     */
    String getText() const
    {
        String text;

        if (identifier.isValid())
            text << identifier.toString();

        if (! json.isVoid())
        {
            if (text.isNotEmpty() && (! json.isArray()))
                text << ": ";

            if (json.isObject() && (! identifier.isValid()))
                text << "[Array]";
            else if (! json.isArray())
                text << json.toString();
        }

        return text;
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsonTreeItem)
};

//==============================================================================
class XMLandJSONDemo   : public Component,
                         private CodeDocument::Listener
{
public:
    /** The type of database to parse. */
    enum Type
    {
        xml,
        json
    };

    XMLandJSONDemo()
    {
        setOpaque (true);

        addAndMakeVisible (typeBox);
        typeBox.addItem ("XML",  1);
        typeBox.addItem ("JSON", 2);

        typeBox.onChange = [this]
        {
            if (typeBox.getSelectedId() == 1)
                reset (xml);
            else
                reset (json);
        };

        comboBoxLabel.attachToComponent (&typeBox, true);

        addAndMakeVisible (codeDocumentComponent);
        codeDocument.addListener (this);

        resultsTree.setTitle ("Results");
        addAndMakeVisible (resultsTree);
        resultsTree.setColour (TreeView::backgroundColourId, Colours::white);
        resultsTree.setDefaultOpenness (true);

        addAndMakeVisible (errorMessage);
        errorMessage.setReadOnly (true);
        errorMessage.setMultiLine (true);
        errorMessage.setCaretVisible (false);
        errorMessage.setColour (TextEditor::outlineColourId, Colours::transparentWhite);
        errorMessage.setColour (TextEditor::shadowColourId,  Colours::transparentWhite);

        typeBox.setSelectedId (1);

        setSize (500, 500);
    }

    ~XMLandJSONDemo() override
    {
        resultsTree.setRootItem (nullptr);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground));
    }

    void resized() override
    {
        auto area = getLocalBounds();

        typeBox.setBounds (area.removeFromTop (36).removeFromRight (150).reduced (8));
        codeDocumentComponent.setBounds (area.removeFromTop(area.getHeight() / 2).reduced (8));
        resultsTree          .setBounds (area.reduced (8));
        errorMessage         .setBounds (resultsTree.getBounds());
    }

private:
    ComboBox typeBox;
    Label comboBoxLabel { {}, "Database Type:" };
    CodeDocument codeDocument;
    CodeEditorComponent codeDocumentComponent  { codeDocument, nullptr };
    TreeView resultsTree;

    std::unique_ptr<TreeViewItem> rootItem;
    std::unique_ptr<XmlElement> parsedXml;
    TextEditor errorMessage;

    void rebuildTree()
    {
        std::unique_ptr<XmlElement> openness;

        if (rootItem.get() != nullptr)
            openness = rootItem->getOpennessState();

        createNewRootNode();

        if (openness.get() != nullptr && rootItem.get() != nullptr)
            rootItem->restoreOpennessState (*openness);
    }

    void createNewRootNode()
    {
        // clear the current tree
        resultsTree.setRootItem (nullptr);
        rootItem.reset();

        // try and parse the editor's contents
        switch (typeBox.getSelectedItemIndex())
        {
            case xml:           rootItem.reset (rebuildXml());        break;
            case json:          rootItem.reset (rebuildJson());       break;
            default:            rootItem.reset();                     break;
        }

        // if we have a valid TreeViewItem hide any old error messages and set our TreeView to use it
        if (rootItem.get() != nullptr)
            errorMessage.clear();

        errorMessage.setVisible (! errorMessage.isEmpty());

        resultsTree.setRootItem (rootItem.get());
    }

    /** Parses the editor's contents as XML. */
    TreeViewItem* rebuildXml()
    {
        parsedXml.reset();

        XmlDocument doc (codeDocument.getAllContent());
        parsedXml = doc.getDocumentElement();

        if (parsedXml.get() == nullptr)
        {
            auto error = doc.getLastParseError();

            if (error.isEmpty())
                error = "Unknown error";

            errorMessage.setText ("Error parsing XML: " + error, dontSendNotification);

            return nullptr;
        }

        return new XmlTreeItem (*parsedXml);
    }

    /** Parses the editor's contents as JSON. */
    TreeViewItem* rebuildJson()
    {
        var parsedJson;
        auto result = JSON::parse (codeDocument.getAllContent(), parsedJson);

        if (! result.wasOk())
        {
            errorMessage.setText ("Error parsing JSON: " + result.getErrorMessage());
            return nullptr;
        }

        return new JsonTreeItem (Identifier(), parsedJson);
    }

    /** Clears the editor and loads some default text. */
    void reset (Type type)
    {
        switch (type)
        {
            case xml:   codeDocument.replaceAllContent (loadEntireAssetIntoString ("treedemo.xml")); break;
            case json:  codeDocument.replaceAllContent (loadEntireAssetIntoString ("juce_module_info")); break;
            default:    codeDocument.replaceAllContent ({}); break;
        }
    }

    void codeDocumentTextInserted (const String&, int) override     { rebuildTree(); }
    void codeDocumentTextDeleted (int, int) override                { rebuildTree(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XMLandJSONDemo)
};
