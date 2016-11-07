/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2015 - ROLI Ltd.

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

#include "../JuceDemoHeader.h"

//==============================================================================
class XmlTreeItem  : public TreeViewItem
{
public:
    XmlTreeItem (XmlElement& x)  : xml (x)
    {
    }

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
        g.setFont (height * 0.7f);

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

                forEachXmlChildElement (xml, child)
                {
                    jassert (child != nullptr);
                    addSubItem (new XmlTreeItem (*child));
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
    XmlElement& xml;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XmlTreeItem)
};

//==============================================================================
class JsonTreeItem  : public TreeViewItem
{
public:
    JsonTreeItem (Identifier i, var value)   : identifier (i), json (value)
    {
    }

    String getUniqueName() const override
    {
        return identifier.toString() + "_id";
    }

    bool mightContainSubItems() override
    {
        if (DynamicObject* obj = json.getDynamicObject())
            return obj->getProperties().size() > 0;

        return json.isArray();
    }

    void paintItem (Graphics& g, int width, int height) override
    {
        // if this item is selected, fill it with a background colour..
        if (isSelected())
            g.fillAll (Colours::blue.withAlpha (0.3f));

        g.setColour (Colours::black);
        g.setFont (height * 0.7f);

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
                        var& child (json[i]);
                        jassert (! child.isVoid());
                        addSubItem (new JsonTreeItem (Identifier(), child));
                    }
                }
                else if (DynamicObject* obj = json.getDynamicObject())
                {
                    NamedValueSet& props (obj->getProperties());

                    for (int i = 0; i < props.size(); ++i)
                    {
                        const Identifier id (props.getName (i));
                        var child (props[id]);
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
class StringsDemo   : public Component,
                      private ComboBox::Listener,
                      private CodeDocument::Listener
{
public:
    /** The type of database to parse. */
    enum Type
    {
        xml,
        json
    };

    StringsDemo()
        : codeDocumentComponent (codeDocument, nullptr)
    {
        setOpaque (true);

        addAndMakeVisible (typeBox);
        typeBox.addListener (this);
        typeBox.addItem ("XML", 1);
        typeBox.addItem ("JSON", 2);

        comboBoxLabel.setText ("Database Type:", dontSendNotification);
        comboBoxLabel.attachToComponent (&typeBox, true);

        addAndMakeVisible (codeDocumentComponent);
        codeDocument.addListener (this);

        addAndMakeVisible (resultsTree);
        resultsTree.setColour (TreeView::backgroundColourId, Colours::white);
        resultsTree.setDefaultOpenness (true);

        addAndMakeVisible (errorMessage);
        errorMessage.setReadOnly (true);
        errorMessage.setMultiLine (true);
        errorMessage.setCaretVisible (false);
        errorMessage.setColour (TextEditor::outlineColourId, Colours::transparentWhite);
        errorMessage.setColour (TextEditor::shadowColourId, Colours::transparentWhite);

        typeBox.setSelectedId (1);
    }

    ~StringsDemo()
    {
        resultsTree.setRootItem (nullptr);
    }

    void paint (Graphics& g) override
    {
        fillStandardDemoBackground (g);
    }

    void resized() override
    {
        Rectangle<int> area (getLocalBounds());
        typeBox.setBounds (area.removeFromTop (36).removeFromRight (150).reduced (8));
        codeDocumentComponent.setBounds (area.removeFromTop(area.getHeight() / 2).reduced (8));
        resultsTree.setBounds (area.reduced (8));
        errorMessage.setBounds (resultsTree.getBounds());
    }

private:
    ComboBox typeBox;
    Label comboBoxLabel;
    CodeDocument codeDocument;
    CodeEditorComponent codeDocumentComponent;
    TreeView resultsTree;

    ScopedPointer<TreeViewItem> rootItem;
    ScopedPointer<XmlElement> parsedXml;
    TextEditor errorMessage;

    void rebuildTree()
    {
        ScopedPointer<XmlElement> openness;

        if (rootItem != nullptr)
            openness = rootItem->getOpennessState();

        createNewRootNode();

        if (openness != nullptr && rootItem != nullptr)
            rootItem->restoreOpennessState (*openness);
    }

    void createNewRootNode()
    {
        // clear the current tree
        resultsTree.setRootItem (nullptr);
        rootItem = nullptr;

        // try and parse the editor's contents
        switch (typeBox.getSelectedItemIndex())
        {
            case xml:           rootItem = rebuildXml();        break;
            case json:          rootItem = rebuildJson();       break;
            default:            rootItem = nullptr;             break;
        }

        // if we have a valid TreeViewItem hide any old error messages and set our TreeView to use it
        if (rootItem != nullptr)
            errorMessage.clear();

        errorMessage.setVisible (! errorMessage.isEmpty());

        resultsTree.setRootItem (rootItem);
    }

    /** Parses the editors contects as XML. */
    TreeViewItem* rebuildXml()
    {
        parsedXml = nullptr;

        XmlDocument doc (codeDocument.getAllContent());
        parsedXml = doc.getDocumentElement();

        if (parsedXml == nullptr)
        {
            String error (doc.getLastParseError());

            if (error.isEmpty())
                error = "Unknown error";

            errorMessage.setText ("Error parsing XML: " + error, dontSendNotification);

            return nullptr;
        }

        return new XmlTreeItem (*parsedXml);
    }

    /** Parses the editors contects as JSON. */
    TreeViewItem* rebuildJson()
    {
        var parsedJson;
        Result result = JSON::parse (codeDocument.getAllContent(), parsedJson);

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
            case xml:   codeDocument.replaceAllContent (BinaryData::treedemo_xml);      break;
            case json:  codeDocument.replaceAllContent (BinaryData::juce_module_info);  break;
            default:    codeDocument.replaceAllContent (String());                      break;
        }
    }

    void comboBoxChanged (ComboBox* box) override
    {
        if (box == &typeBox)
        {
            if (typeBox.getSelectedId() == 1)
                reset (xml);
            else
                reset (json);
        }
    }

    void codeDocumentTextInserted (const String&, int) override     { rebuildTree(); }
    void codeDocumentTextDeleted (int, int) override                { rebuildTree(); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StringsDemo)
};


// This static object will register this demo type in a global list of demos..
static JuceDemoType<StringsDemo> demo ("40 XML & JSON");
