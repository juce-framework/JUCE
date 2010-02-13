/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "../jucedemo_headers.h"


//==============================================================================
/**
    This class shows how to implement a TableListBoxModel to show in a TableListBox.
*/
class TableDemoComponent    : public Component,
                              public TableListBoxModel
{
public:
    //==============================================================================
    TableDemoComponent()
        : font (14.0f),
          demoData (0)
    {
        // Load some data from an embedded XML file..
        loadData();

        // Create our table component and add it to this component..
        addAndMakeVisible (table = new TableListBox (T("demo table"), this));

        // give it a border
        table->setColour (ListBox::outlineColourId, Colours::grey);
        table->setOutlineThickness (1);

        // Add some columns to the table header, based on the column list in our database..
        forEachXmlChildElement (*columnList, columnXml)
        {
            table->getHeader()->addColumn (columnXml->getStringAttribute T("name"),
                                           columnXml->getIntAttribute T("columnId"),
                                           columnXml->getIntAttribute T("width"),
                                           50, 400,
                                           TableHeaderComponent::defaultFlags);
        }

        // we could now change some initial settings..
        table->getHeader()->setSortColumnId (1, true); // sort forwards by the ID column
        table->getHeader()->setColumnVisible (7, false); // hide the "length" column until the user shows it

        // un-comment this line to have a go of stretch-to-fit mode
        // table->getHeader()->setStretchToFitActive (true);

        table->setMultipleSelectionEnabled (true);
    }

    ~TableDemoComponent()
    {
        deleteAllChildren();

        delete demoData;
    }

    //==============================================================================
    // This is overloaded from TableListBoxModel, and must return the total number of rows in our table
    int getNumRows()
    {
        return numRows;
    }

    // This is overloaded from TableListBoxModel, and should fill in the background of the whole row
    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
    {
        if (rowIsSelected)
            g.fillAll (Colours::lightblue);
    }

    // This is overloaded from TableListBoxModel, and must paint any cells that aren't using custom
    // components.
    void paintCell (Graphics& g,
                    int rowNumber,
                    int columnId,
                    int width, int height,
                    bool rowIsSelected)
    {
        g.setColour (Colours::black);
        g.setFont (font);

        const XmlElement* rowElement = dataList->getChildElement (rowNumber);

        if (rowElement != 0)
        {
            const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));

            g.drawText (text, 2, 0, width - 4, height, Justification::centredLeft, true);
        }

        g.setColour (Colours::black.withAlpha (0.2f));
        g.fillRect (width - 1, 0, 1, height);
    }

    // This is overloaded from TableListBoxModel, and tells us that the user has clicked a table header
    // to change the sort order.
    void sortOrderChanged (int newSortColumnId, const bool isForwards)
    {
        if (newSortColumnId != 0)
        {
            DemoDataSorter sorter (getAttributeNameForColumnId (newSortColumnId), isForwards);
            dataList->sortChildElements (sorter);

            table->updateContent();
        }
    }

    // This is overloaded from TableListBoxModel, and must update any custom components that we're using
    Component* refreshComponentForCell (int rowNumber, int columnId, bool isRowSelected,
                                        Component* existingComponentToUpdate)
    {
        if (columnId == 5) // If it's the ratings column, we'll return our custom component..
        {
            RatingColumnCustomComponent* ratingsBox = (RatingColumnCustomComponent*) existingComponentToUpdate;

            // If an existing component is being passed-in for updating, we'll re-use it, but
            // if not, we'll have to create one.
            if (ratingsBox == 0)
                ratingsBox = new RatingColumnCustomComponent (*this);

            ratingsBox->setRowAndColumn (rowNumber, columnId);

            return ratingsBox;
        }
        else
        {
            // for any other column, just return 0, as we'll be painting these columns directly.

            jassert (existingComponentToUpdate == 0);
            return 0;
        }
    }

    // This is overloaded from TableListBoxModel, and should choose the best width for the specified
    // column.
    int getColumnAutoSizeWidth (int columnId)
    {
        if (columnId == 5)
            return 100; // (this is the ratings column, containing a custom component)

        int widest = 32;

        // find the widest bit of text in this column..
        for (int i = getNumRows(); --i >= 0;)
        {
            const XmlElement* rowElement = dataList->getChildElement (i);

            if (rowElement != 0)
            {
                const String text (rowElement->getStringAttribute (getAttributeNameForColumnId (columnId)));

                widest = jmax (widest, font.getStringWidth (text));
            }
        }

        return widest + 8;
    }

    // A couple of quick methods to set and get the "rating" value when the user
    // changes the combo box
    int getRating (const int rowNumber, const int columnId) const
    {
        return dataList->getChildElement (rowNumber)->getIntAttribute (T("Rating"));
    }

    void setRating (const int rowNumber, const int columnId, const int newRating)
    {
        dataList->getChildElement (rowNumber)->setAttribute (T("Rating"), newRating);
    }

    //==============================================================================
    void resized()
    {
        // position our table with a gap around its edge
        table->setBoundsInset (BorderSize (8));
    }

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    TableListBox* table;    // the table component itself
    Font font;

    XmlElement* demoData;   // This is the XML document loaded from the embedded file "demo table data.xml"
    XmlElement* columnList; // A pointer to the sub-node of demoData that contains the list of columns
    XmlElement* dataList;   // A pointer to the sub-node of demoData that contains the list of data rows
    int numRows;            // The number of rows of data we've got

    //==============================================================================
    // This is a custom component containing a combo box, which we're going to put inside
    // our table's "rating" column.
    class RatingColumnCustomComponent    : public Component,
                                           public ComboBoxListener
    {
    public:
        RatingColumnCustomComponent (TableDemoComponent& owner_)
            : owner (owner_)
        {
            // just put a combo box inside this component
            addAndMakeVisible (comboBox = new ComboBox (String::empty));
            comboBox->addItem (T("fab"), 1);
            comboBox->addItem (T("groovy"), 2);
            comboBox->addItem (T("hep"), 3);
            comboBox->addItem (T("neat"), 4);
            comboBox->addItem (T("wild"), 5);
            comboBox->addItem (T("swingin"), 6);
            comboBox->addItem (T("mad for it"), 7);

            // when the combo is changed, we'll get a callback.
            comboBox->addListener (this);
            comboBox->setWantsKeyboardFocus (false);
        }

        ~RatingColumnCustomComponent()
        {
            deleteAllChildren();
        }

        void resized()
        {
            comboBox->setBoundsInset (BorderSize (2));
        }

        // Our demo code will call this when we may need to update our contents
        void setRowAndColumn (const int newRow, const int newColumn)
        {
            row = newRow;
            columnId = newColumn;
            comboBox->setSelectedId (owner.getRating (row, columnId), true);
        }

        void comboBoxChanged (ComboBox* comboBoxThatHasChanged)
        {
            owner.setRating (row, columnId, comboBox->getSelectedId());
        }

    private:
        TableDemoComponent& owner;
        ComboBox* comboBox;
        int row, columnId;
    };

    //==============================================================================
    // A comparator used to sort our data when the user clicks a column header
    class DemoDataSorter
    {
    public:
        DemoDataSorter (const String attributeToSort_, bool forwards)
            : attributeToSort (attributeToSort_),
              direction (forwards ? 1 : -1)
        {
        }

        int compareElements (XmlElement* first, XmlElement* second) const
        {
            int result = first->getStringAttribute (attributeToSort)
                           .compareLexicographically (second->getStringAttribute (attributeToSort));

            if (result == 0)
                result = first->getStringAttribute (T("ID"))
                           .compareLexicographically (second->getStringAttribute (T("ID")));

            return direction * result;
        }

    private:
        const String attributeToSort;
        const int direction;
    };

    //==============================================================================
    // this loads the embedded database XML file into memory
    void loadData()
    {
        XmlDocument dataDoc (String ((const char*) BinaryData::demo_table_data_xml));
        demoData = dataDoc.getDocumentElement();

        dataList = demoData->getChildByName (T("DATA"));
        columnList = demoData->getChildByName (T("COLUMNS"));

        numRows = dataList->getNumChildElements();
    }

    // (a utility method to search our XML for the attribute that matches a column ID)
    const String getAttributeNameForColumnId (const int columnId) const
    {
        forEachXmlChildElement (*columnList, columnXml)
        {
            if (columnXml->getIntAttribute T("columnId") == columnId)
                return columnXml->getStringAttribute T("name");
        }

        return String::empty;
    }
};


//==============================================================================
Component* createTableDemo()
{
    return new TableDemoComponent();
}
