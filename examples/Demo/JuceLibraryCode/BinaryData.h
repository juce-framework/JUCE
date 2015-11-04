/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#ifndef BINARYDATA_H_134119534_INCLUDED
#define BINARYDATA_H_134119534_INCLUDED

namespace BinaryData
{
    extern const char*   tile_background_png;
    const int            tile_background_pngSize = 151;

    extern const char*   cello_wav;
    const int            cello_wavSize = 46348;

    extern const char*   demo_table_data_xml;
    const int            demo_table_data_xmlSize = 5239;

    extern const char*   icons_zip;
    const int            icons_zipSize = 83876;

    extern const char*   juce_icon_png;
    const int            juce_icon_pngSize = 45854;

    extern const char*   juce_module_info;
    const int            juce_module_infoSize = 1404;

    extern const char*   portmeirion_jpg;
    const int            portmeirion_jpgSize = 145904;

    extern const char*   teapot_obj;
    const int            teapot_objSize = 95000;

    extern const char*   treedemo_xml;
    const int            treedemo_xmlSize = 1126;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Number of elements in the namedResourceList array.
    const int namedResourceListSize = 9;

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes) throw();
}

#endif
