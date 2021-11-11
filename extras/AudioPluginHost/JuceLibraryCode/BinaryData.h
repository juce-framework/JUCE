/* =========================================================================================

   This is an auto-generated file: Any edits you make may be overwritten!

*/

#pragma once

namespace BinaryData
{
    extern const char*   cassette_recorder_wav;
    const int            cassette_recorder_wavSize = 37902;

    extern const char*   cello_wav;
    const int            cello_wavSize = 46348;

    extern const char*   guitar_amp_wav;
    const int            guitar_amp_wavSize = 90246;

    extern const char*   proaudio_path;
    const int            proaudio_pathSize = 452;

    extern const char*   reverb_ir_wav;
    const int            reverb_ir_wavSize = 648404;

    extern const char*   singing_ogg;
    const int            singing_oggSize = 15354;

    // Number of elements in the namedResourceList and originalFileNames arrays.
    const int namedResourceListSize = 6;

    // Points to the start of a list of resource names.
    extern const char* namedResourceList[];

    // Points to the start of a list of resource filenames.
    extern const char* originalFilenames[];

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding data and its size (or a null pointer if the name isn't found).
    const char* getNamedResource (const char* resourceNameUTF8, int& dataSizeInBytes);

    // If you provide the name of one of the binary resource variables above, this function will
    // return the corresponding original, non-mangled filename (or a null pointer if the name isn't found).
    const char* getNamedResourceOriginalFilename (const char* resourceNameUTF8);
}
