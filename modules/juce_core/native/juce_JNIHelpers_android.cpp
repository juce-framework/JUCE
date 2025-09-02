/*
  ==============================================================================

   This file is part of the JUCE framework.
   Copyright (c) Raw Material Software Limited

   JUCE is an open source framework subject to commercial or open source
   licensing.

   By downloading, installing, or using the JUCE framework, or combining the
   JUCE framework with any other source code, object code, content or any other
   copyrightable work, you agree to the terms of the JUCE End User Licence
   Agreement, and all incorporated terms including the JUCE Privacy Policy and
   the JUCE Website Terms of Service, as applicable, which will bind you. If you
   do not agree to the terms of these agreements, we will not license the JUCE
   framework to you, and you must discontinue the installation or download
   process and cease use of the JUCE framework.

   JUCE End User Licence Agreement: https://juce.com/legal/juce-8-licence/
   JUCE Privacy Policy: https://juce.com/juce-privacy-policy
   JUCE Website Terms of Service: https://juce.com/juce-website-terms-of-service/

   Or:

   You may also use this code under the terms of the AGPLv3:
   https://www.gnu.org/licenses/agpl-3.0.en.html

   THE JUCE FRAMEWORK IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL
   WARRANTIES, WHETHER EXPRESSED OR IMPLIED, INCLUDING WARRANTY OF
   MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, ARE DISCLAIMED.

  ==============================================================================
*/

namespace juce
{

//==============================================================================
constexpr uint8 invocationHandleByteCode[]
{
    0x1f, 0x8b, 0x08, 0x08, 0xa2, 0x70, 0x87, 0x68, 0x00, 0x03, 0x63, 0x6c,
    0x61, 0x73, 0x73, 0x65, 0x73, 0x2e, 0x64, 0x65, 0x78, 0x00, 0x6d, 0x94,
    0xbd, 0x6f, 0xd3, 0x40, 0x14, 0xc0, 0xdf, 0x9d, 0x9d, 0xa4, 0x94, 0x12,
    0xd2, 0x0f, 0x10, 0x2a, 0x1d, 0x2a, 0x0b, 0x18, 0x50, 0xd2, 0x38, 0x25,
    0x90, 0xa4, 0x29, 0x15, 0x12, 0x88, 0x8f, 0xca, 0x80, 0x44, 0xab, 0x0e,
    0x05, 0x24, 0x2e, 0xf6, 0x85, 0xb8, 0x75, 0xec, 0x28, 0x4e, 0x43, 0xf8,
    0xaa, 0x0a, 0x42, 0x65, 0xab, 0x58, 0x3a, 0x30, 0x20, 0x18, 0x58, 0x91,
    0x18, 0xf8, 0x03, 0x90, 0x58, 0x40, 0x0c, 0xc0, 0xc6, 0x00, 0x13, 0x6c,
    0x74, 0x64, 0x64, 0xe0, 0x9d, 0xef, 0x4a, 0x22, 0x51, 0x4b, 0xbf, 0xbb,
    0xe7, 0xf7, 0xde, 0xbd, 0xf7, 0xee, 0xec, 0x7b, 0x0e, 0xef, 0xf4, 0x9b,
    0xc7, 0x0a, 0xf0, 0xf0, 0xfb, 0xa7, 0x8d, 0xb7, 0x9b, 0xe6, 0x6b, 0x72,
    0xee, 0xe9, 0x97, 0xe1, 0x1f, 0xfe, 0xe6, 0xc7, 0x5f, 0x3f, 0x7f, 0x7f,
    0x7b, 0xf1, 0x31, 0x98, 0x8f, 0x01, 0x34, 0x00, 0xa0, 0xb3, 0x90, 0x1f,
    0x02, 0xf5, 0x3c, 0xd1, 0x01, 0xf6, 0x83, 0xd4, 0xef, 0x42, 0xde, 0x23,
    0xa8, 0x82, 0x2d, 0x84, 0x20, 0x63, 0x38, 0x24, 0x70, 0x3e, 0x44, 0xe4,
    0xfb, 0x35, 0x1c, 0xbe, 0x6a, 0x00, 0xf7, 0x70, 0xbe, 0x42, 0x01, 0x16,
    0x91, 0xeb, 0xc8, 0x6d, 0x64, 0x0d, 0x59, 0x47, 0x5e, 0x21, 0x1f, 0x90,
    0xef, 0xc8, 0x6f, 0xe4, 0x20, 0xfa, 0x17, 0x90, 0x29, 0x64, 0x06, 0x99,
    0x43, 0x16, 0x11, 0x4f, 0xc4, 0x41, 0xd6, 0x91, 0x0d, 0xe4, 0x19, 0xf2,
    0x52, 0x93, 0x79, 0xb0, 0x54, 0x88, 0x83, 0xcc, 0xdd, 0xa7, 0x6a, 0xeb,
    0x47, 0x76, 0x23, 0x7b, 0x10, 0x4d, 0x91, 0xa7, 0xb2, 0x5e, 0x21, 0x9f,
    0xa2, 0xd2, 0x9e, 0x50, 0x7b, 0x1b, 0x50, 0xb2, 0x45, 0x65, 0xcc, 0xe1,
    0x68, 0x4f, 0x5a, 0x64, 0x23, 0x40, 0x21, 0xa9, 0xde, 0xf7, 0xaa, 0xbd,
    0xa6, 0x94, 0x7e, 0x30, 0x9a, 0x09, 0x0c, 0x45, 0x71, 0xa9, 0xf2, 0x97,
    0x88, 0xd5, 0x69, 0x54, 0x09, 0x6d, 0x07, 0x13, 0x5f, 0xd2, 0x65, 0xae,
    0x7e, 0xac, 0x42, 0xd4, 0x3b, 0x42, 0x65, 0x9e, 0x39, 0x86, 0xe7, 0x8a,
    0xce, 0x39, 0x20, 0x5a, 0x11, 0xab, 0x4a, 0xdc, 0x48, 0x84, 0x89, 0x47,
    0x89, 0xcd, 0x76, 0x5c, 0x44, 0x18, 0x48, 0x0c, 0x26, 0x86, 0x10, 0x11,
    0x4f, 0x93, 0xe7, 0x4c, 0xe5, 0x7e, 0x1b, 0x29, 0x31, 0x2e, 0xa2, 0x2a,
    0xa9, 0x2c, 0x00, 0x06, 0x95, 0xe7, 0x21, 0xbe, 0xd3, 0xe2, 0x38, 0x44,
    0x75, 0xc7, 0xd4, 0x4e, 0x8e, 0x50, 0xb9, 0xcf, 0xb9, 0x53, 0x68, 0x47,
    0x39, 0x47, 0x81, 0x16, 0x69, 0x0c, 0x1a, 0x26, 0x05, 0x9d, 0x24, 0xe1,
    0xa4, 0x70, 0x4a, 0xbe, 0x83, 0xa3, 0x04, 0x92, 0xd3, 0x60, 0x42, 0x1a,
    0x4e, 0x40, 0xf2, 0x71, 0xf7, 0x8c, 0x65, 0xf9, 0x1a, 0xe6, 0xee, 0x53,
    0x67, 0x1a, 0x8f, 0xce, 0x5b, 0x66, 0x8e, 0x4f, 0xbb, 0xbe, 0xdb, 0x9a,
    0x01, 0x32, 0x0b, 0xa3, 0xb3, 0x2b, 0x36, 0xbf, 0xe0, 0xb7, 0x03, 0x9b,
    0xb5, 0xdc, 0xc0, 0x3f, 0xcf, 0x7c, 0xc7, 0xe3, 0xcd, 0x89, 0x25, 0xd6,
    0x66, 0x10, 0xb3, 0x66, 0x2d, 0xcb, 0x02, 0xdd, 0x12, 0xe3, 0x61, 0xcb,
    0x0e, 0xea, 0xd9, 0x66, 0x3d, 0xf4, 0xb2, 0x4b, 0xb8, 0x24, 0xbb, 0xe3,
    0xba, 0x32, 0x8c, 0x5a, 0x0e, 0xf3, 0xda, 0xee, 0x72, 0x96, 0xf9, 0x7e,
    0xd0, 0x8a, 0x6c, 0xd9, 0xf9, 0x5a, 0x33, 0xb8, 0x15, 0x96, 0x61, 0xc8,
    0x12, 0x61, 0xb3, 0x1e, 0xf3, 0x6f, 0x66, 0x2f, 0x57, 0x96, 0xb8, 0xdd,
    0x2a, 0xc3, 0xbe, 0x1e, 0x5d, 0xe4, 0xc7, 0x2a, 0x1e, 0x2f, 0x63, 0xb6,
    0xae, 0xba, 0xc9, 0xab, 0x1e, 0xfa, 0x66, 0x77, 0xcc, 0xf6, 0xbf, 0xdb,
    0x45, 0xde, 0xaa, 0x05, 0x4e, 0x19, 0xc8, 0x02, 0xd0, 0x85, 0x59, 0x18,
    0xbe, 0xba, 0x43, 0xd6, 0x98, 0xed, 0x71, 0xd6, 0x84, 0x94, 0xe3, 0x86,
    0x0d, 0xd6, 0xb2, 0x6b, 0x67, 0x5d, 0x9f, 0x79, 0xee, 0x1d, 0x0e, 0xc9,
    0x6d, 0x8d, 0x48, 0xb6, 0xcc, 0xa1, 0xaf, 0xba, 0x6d, 0x89, 0xbb, 0x52,
    0xb3, 0xc7, 0xc7, 0x12, 0xda, 0xfc, 0x74, 0xe0, 0xb7, 0x78, 0xa7, 0x05,
    0xb1, 0x36, 0xf3, 0x56, 0x38, 0x3c, 0x27, 0xab, 0xab, 0x67, 0x8a, 0x77,
    0x8d, 0x0a, 0xb3, 0x97, 0xb9, 0xef, 0x18, 0x53, 0x86, 0xc3, 0x3b, 0x46,
    0xda, 0xc0, 0x33, 0x6b, 0xb8, 0x5e, 0x54, 0x74, 0xa6, 0x1e, 0x38, 0x1c,
    0x0d, 0x4d, 0x8e, 0xb9, 0x43, 0x8e, 0xc6, 0x1a, 0x0b, 0x33, 0x76, 0x8d,
    0xdb, 0xcb, 0xe1, 0x4a, 0x3d, 0x34, 0xa6, 0xaa, 0xcc, 0x0b, 0x79, 0xda,
    0xa8, 0xbb, 0x7e, 0x86, 0x35, 0x5c, 0x63, 0x6a, 0x32, 0x9f, 0x36, 0xc2,
    0x1a, 0xcb, 0xe4, 0x70, 0x11, 0x2b, 0x30, 0x27, 0x57, 0x64, 0x05, 0x33,
    0x7f, 0xc2, 0xac, 0x14, 0x4a, 0x25, 0xc7, 0xcc, 0x17, 0x27, 0x79, 0xbe,
    0x54, 0xb0, 0x73, 0x66, 0x89, 0x15, 0x8e, 0x57, 0xaa, 0x85, 0x6a, 0x29,
    0xe7, 0x88, 0xa8, 0x6d, 0xde, 0x0c, 0x31, 0x1d, 0x2e, 0x2a, 0x4e, 0xe4,
    0xcc, 0x89, 0x52, 0xc6, 0xe1, 0x6d, 0xe3, 0x3e, 0x50, 0x4a, 0x46, 0xc6,
    0xc8, 0x01, 0x1d, 0x7f, 0x5c, 0xf1, 0xf3, 0x3f, 0x58, 0xd3, 0xdf, 0x68,
    0xf4, 0x21, 0x05, 0x22, 0x20, 0x9f, 0x35, 0x8d, 0x6c, 0x69, 0x84, 0xfc,
    0x51, 0x77, 0xea, 0xac, 0x0e, 0xff, 0x1e, 0xa2, 0x66, 0xf1, 0x5f, 0x35,
    0x74, 0x79, 0x9f, 0x7a, 0xf5, 0x04, 0xba, 0xbd, 0x86, 0xf6, 0xf4, 0x1b,
    0xad, 0xa7, 0xe7, 0xe8, 0x3d, 0x7d, 0x27, 0x06, 0xdd, 0xde, 0x13, 0x87,
    0x6e, 0xff, 0x21, 0xe3, 0xd2, 0x4f, 0xf4, 0x20, 0x4d, 0xc9, 0xe2, 0xde,
    0x91, 0x94, 0x94, 0xa3, 0xfb, 0x39, 0x2e, 0x73, 0x89, 0x1e, 0xa5, 0x8f,
    0xf7, 0xd4, 0xaa, 0x64, 0x71, 0x6f, 0xb5, 0x94, 0x94, 0x45, 0xad, 0x71,
    0xa5, 0x17, 0x77, 0x1a, 0x94, 0x5e, 0xf4, 0xc6, 0xbf, 0x68, 0xe3, 0xb4,
    0x6f, 0x54, 0x05, 0x00, 0x00
};

constexpr uint8 activityCallbacksByteCode[]
{
    0x1f, 0x8b, 0x08, 0x08, 0x70, 0x70, 0x87, 0x68, 0x00, 0x03, 0x63, 0x6c,
    0x61, 0x73, 0x73, 0x65, 0x73, 0x2e, 0x64, 0x65, 0x78, 0x00, 0x9d, 0x94,
    0x41, 0x68, 0x13, 0x41, 0x14, 0x86, 0xdf, 0x6c, 0xd2, 0x56, 0x6b, 0xac,
    0xb1, 0x15, 0x0f, 0x2a, 0x52, 0x96, 0x5e, 0x02, 0x49, 0x36, 0xa9, 0xd1,
    0x4d, 0x9a, 0xa2, 0x58, 0x73, 0x51, 0x16, 0x2a, 0x16, 0x72, 0x2a, 0xc8,
    0x64, 0x77, 0x62, 0x36, 0xdd, 0xec, 0x2e, 0x3b, 0x9b, 0xd0, 0x22, 0x96,
    0x56, 0x3c, 0xa8, 0x07, 0x0f, 0x0a, 0x9e, 0x04, 0xa1, 0x17, 0xaf, 0x1e,
    0xbd, 0x0a, 0x1e, 0x3d, 0x88, 0x20, 0x88, 0x78, 0xf0, 0x24, 0x08, 0x7a,
    0x91, 0x82, 0x37, 0xf1, 0xcd, 0xee, 0x6c, 0x92, 0x1e, 0xbc, 0x74, 0xc2,
    0x97, 0x99, 0xf9, 0xe7, 0xbd, 0x37, 0x6f, 0x96, 0x99, 0x67, 0xb1, 0xcd,
    0xe9, 0xd2, 0x05, 0x1d, 0xde, 0x7f, 0xf9, 0xf4, 0x68, 0xf6, 0xf1, 0xce,
    0xf4, 0x46, 0xb7, 0xff, 0x67, 0xef, 0xf6, 0xae, 0xf1, 0xf6, 0xc9, 0xb3,
    0xc6, 0xf3, 0xd5, 0xdc, 0xde, 0xb7, 0x34, 0x80, 0x0f, 0x00, 0x9b, 0xcd,
    0xca, 0x2c, 0xc8, 0x66, 0xa0, 0x26, 0x26, 0x42, 0x9f, 0x44, 0xde, 0x20,
    0x29, 0xe4, 0x03, 0x8c, 0xda, 0x51, 0x64, 0x1f, 0x21, 0xc8, 0x32, 0xfe,
    0x3d, 0x40, 0x83, 0x75, 0xec, 0xcf, 0x29, 0x00, 0x0b, 0x48, 0x03, 0x59,
    0x47, 0x9e, 0x22, 0xaf, 0x90, 0x8f, 0xc8, 0x77, 0xe4, 0x07, 0xf2, 0x0b,
    0xf9, 0x8d, 0x4c, 0xa0, 0xcf, 0x19, 0x24, 0x8f, 0x5c, 0x11, 0xfe, 0x88,
    0x87, 0xdc, 0x47, 0x14, 0xb9, 0x27, 0xa6, 0x02, 0x13, 0x32, 0x8f, 0x29,
    0xc9, 0x84, 0xcc, 0xe1, 0x88, 0x1c, 0x67, 0x94, 0x38, 0x1f, 0x31, 0x3e,
    0xa5, 0xc4, 0x7e, 0x10, 0xf9, 0x2b, 0x30, 0x1d, 0xf5, 0x04, 0x8e, 0xc9,
    0x3e, 0x23, 0xfb, 0xe3, 0x72, 0x7d, 0x46, 0xce, 0x4f, 0xc8, 0x3e, 0x2b,
    0xf7, 0x04, 0xb9, 0x3f, 0x91, 0xf3, 0xb4, 0x12, 0x8f, 0x45, 0x5b, 0x90,
    0x06, 0x24, 0xfa, 0x01, 0xfc, 0x25, 0xb1, 0x8d, 0x9f, 0x15, 0x19, 0xcd,
    0xa0, 0x5f, 0x92, 0x41, 0xec, 0x23, 0xf6, 0x50, 0xa2, 0x13, 0x1d, 0x4e,
    0x3b, 0x7c, 0xbc, 0x5c, 0xa4, 0x93, 0x31, 0x00, 0x40, 0x5a, 0x29, 0x30,
    0xb9, 0x6c, 0xbb, 0x76, 0x78, 0x19, 0xce, 0xdf, 0xe8, 0x9b, 0xec, 0xaa,
    0x19, 0xda, 0x03, 0x3b, 0xdc, 0xba, 0x46, 0x1d, 0xa7, 0x45, 0xcd, 0x0d,
    0xbe, 0x42, 0x39, 0x2b, 0x76, 0xe9, 0x80, 0xc2, 0x69, 0x83, 0xba, 0x56,
    0xe0, 0xd9, 0x96, 0x46, 0x7d, 0x5f, 0x4b, 0x0c, 0xeb, 0x50, 0x39, 0xa8,
    0xfb, 0xbe, 0x63, 0x9b, 0x34, 0xb4, 0x3d, 0x77, 0x21, 0xb1, 0x31, 0xec,
    0x36, 0x33, 0xb7, 0x4c, 0x87, 0x0d, 0xa3, 0xd6, 0x61, 0x6e, 0xe8, 0xe5,
    0x71, 0x6d, 0xa5, 0xef, 0x5a, 0x0e, 0xab, 0x43, 0xce, 0x30, 0xbd, 0x9e,
    0x16, 0xf4, 0xb8, 0xa3, 0x75, 0x31, 0x19, 0xed, 0xbf, 0x19, 0xd5, 0x61,
    0xd6, 0x10, 0x49, 0x69, 0x0e, 0x75, 0xef, 0x68, 0xab, 0xad, 0x2e, 0x33,
    0xc3, 0x3a, 0x90, 0x26, 0x28, 0x4d, 0x03, 0x52, 0x4d, 0xc3, 0x80, 0x93,
    0x9e, 0x3b, 0xf4, 0x0c, 0x18, 0x0d, 0x99, 0x05, 0x73, 0x23, 0xa9, 0xc1,
    0x78, 0x18, 0x78, 0x5b, 0x28, 0x66, 0x47, 0xe2, 0x4d, 0xda, 0xe7, 0xa8,
    0x8c, 0x79, 0xde, 0x62, 0xbc, 0xdf, 0x43, 0xe9, 0xec, 0x48, 0x5a, 0xa3,
    0x03, 0x76, 0xdd, 0xe5, 0x21, 0x75, 0x4d, 0xb6, 0x16, 0x62, 0xdc, 0x71,
    0x7b, 0x14, 0x82, 0xf0, 0x60, 0x88, 0xb5, 0xd0, 0xf3, 0x7d, 0x94, 0x5e,
    0x92, 0xed, 0xed, 0x46, 0xf5, 0xae, 0x2a, 0x8e, 0xc0, 0x5c, 0x4b, 0x5d,
    0x52, 0x2d, 0xb6, 0xa9, 0xe6, 0x55, 0x3c, 0xb0, 0x6f, 0x3b, 0xd1, 0xf7,
    0x2a, 0xf4, 0x3c, 0x8b, 0xe1, 0x42, 0xc0, 0x1c, 0x86, 0x67, 0xc4, 0xc5,
    0x0e, 0xe5, 0x05, 0xb3, 0xc3, 0xf0, 0xd0, 0xfd, 0x1e, 0x57, 0x97, 0xda,
    0xd4, 0xe1, 0x2c, 0xaf, 0xf6, 0x6c, 0xb7, 0x40, 0x7d, 0x5b, 0x5d, 0x5a,
    0xac, 0xe4, 0x55, 0xde, 0xa1, 0x85, 0x32, 0x3a, 0x51, 0x9d, 0x5a, 0xe5,
    0x2a, 0xd5, 0x4b, 0x95, 0x4b, 0xa5, 0x96, 0x5e, 0xab, 0x59, 0xa5, 0x4a,
    0x75, 0x91, 0x55, 0x6a, 0xba, 0x59, 0x2e, 0xd5, 0xa8, 0x7e, 0xb1, 0xd5,
    0xd6, 0xdb, 0xb5, 0xb2, 0x25, 0xa2, 0x0e, 0x58, 0xc0, 0x71, 0x3b, 0x74,
    0xaa, 0x16, 0xcb, 0xa5, 0x62, 0xad, 0x60, 0xb1, 0x81, 0x7a, 0x4f, 0xdc,
    0x8c, 0x29, 0xd8, 0xdd, 0x49, 0x7f, 0x55, 0x08, 0xd9, 0x47, 0x1e, 0xa6,
    0x08, 0x79, 0x81, 0xbc, 0x46, 0xde, 0x21, 0x9f, 0x91, 0x9f, 0x78, 0x05,
    0x33, 0x63, 0x77, 0x2b, 0xe9, 0x93, 0x3a, 0x21, 0xee, 0xd4, 0x78, 0xad,
    0x48, 0xea, 0x85, 0x78, 0x93, 0x49, 0xad, 0x98, 0x84, 0x51, 0xbd, 0x20,
    0xf3, 0xf1, 0xfb, 0x15, 0x35, 0x23, 0x35, 0x0f, 0xc3, 0x77, 0x44, 0xb2,
    0xf2, 0xdd, 0x63, 0x40, 0x65, 0x3e, 0x8e, 0x2f, 0x6a, 0x0a, 0x48, 0x9b,
    0xe8, 0xed, 0x65, 0xe3, 0xb1, 0xa8, 0x53, 0xff, 0x00, 0xdd, 0xea, 0xf5,
    0x4e, 0xe0, 0x04, 0x00, 0x00
};

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 STATICMETHOD (newProxyInstance, "newProxyInstance", "(Ljava/lang/ClassLoader;[Ljava/lang/Class;Ljava/lang/reflect/InvocationHandler;)Ljava/lang/Object;") \

 DECLARE_JNI_CLASS (JavaProxy, "java/lang/reflect/Proxy")
#undef JNI_CLASS_MEMBERS

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>", "(J)V") \
 METHOD (clear, "clear", "()V") \
 CALLBACK (juce_invokeImplementer, "dispatchInvoke", "(JLjava/lang/Object;Ljava/lang/reflect/Method;[Ljava/lang/Object;)Ljava/lang/Object;") \
 CALLBACK (juce_dispatchDelete, "dispatchFinalize", "(J)V")

 DECLARE_JNI_CLASS_WITH_BYTECODE (JuceInvocationHandler, "com/rmsl/juce/JuceInvocationHandler", 10, invocationHandleByteCode)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD       (loadClass,            "loadClass",            "(Ljava/lang/String;Z)Ljava/lang/Class;") \
 STATICMETHOD (getSystemClassLoader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;")

 DECLARE_JNI_CLASS (JavaClassLoader, "java/lang/ClassLoader")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V")

 DECLARE_JNI_CLASS (AndroidDexClassLoader, "dalvik/system/DexClassLoader")
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V")

 DECLARE_JNI_CLASS_WITH_MIN_SDK (AndroidInMemoryDexClassLoader, "dalvik/system/InMemoryDexClassLoader", 26)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (constructor, "<init>", "()V")

 DECLARE_JNI_CLASS_WITH_BYTECODE (JuceActivityCallbacksBase, "com/rmsl/juce/JuceActivityCallbacksBase", 24, activityCallbacksByteCode)
#undef JNI_CLASS_MEMBERS

//==============================================================================
struct SystemJavaClassComparator
{
    static int compareElements (JNIClassBase* first, JNIClassBase* second)
    {
        auto isSysClassA = isSystemClass (first);
        auto isSysClassB = isSystemClass (second);

        if ((! isSysClassA) && (! isSysClassB))
        {
            return DefaultElementComparator<bool>::compareElements (first  != nullptr && first->byteCode  != nullptr,
                                                                    second != nullptr && second->byteCode != nullptr);
        }

        return DefaultElementComparator<bool>::compareElements (isSystemClass (first),
                                                                isSystemClass (second));
    }

    static bool isSystemClass (JNIClassBase* cls)
    {
        if (cls == nullptr)
            return false;

        String path (cls->getClassPath());

        return path.startsWith ("java/")
            || path.startsWith ("android/")
            || path.startsWith ("dalvik/");
    }
};

//==============================================================================
JNIClassBase::JNIClassBase (const char* cp, int classMinSDK, const uint8* bc, size_t n)
    : classPath (cp), byteCode (bc), byteCodeSize (n), minSDK (classMinSDK), classRef (nullptr)
{
    SystemJavaClassComparator comparator;

    getClasses().addSorted (comparator, this);
}

JNIClassBase::~JNIClassBase()
{
    getClasses().removeFirstMatchingValue (this);
}

Array<JNIClassBase*>& JNIClassBase::getClasses()
{
    static Array<JNIClassBase*> classes;
    return classes;
}

// Get code cache directory without yet having a context object
static File getCodeCacheDirectory()
{
    int pid = getpid();
    File cmdline ("/proc/" + String (pid) + "/cmdline");

    auto bundleId = cmdline.loadFileAsString().trimStart().trimEnd();

    if (bundleId.isEmpty())
        return {};

    return File ("/data/data/" + bundleId + "/code_cache");
}

void JNIClassBase::initialise (JNIEnv* env, jobject context)
{
    auto sdkVersion = getAndroidSDKVersion();

    if (sdkVersion >= minSDK)
    {
        LocalRef<jstring> classNameAndPackage (javaString (String (classPath).replaceCharacter (L'/', L'.')));
        static Array<GlobalRef> byteCodeLoaders;

        if (! SystemJavaClassComparator::isSystemClass (this))
        {
            // We use the context's class loader, rather than the 'system' class loader, because we
            // may need to load classes from our library dependencies (such as the BillingClient
            // library), and the system class loader is not aware of those libraries.
            const LocalRef<jobject> defaultClassLoader { env->CallObjectMethod (context,
                                                                                env->GetMethodID (env->FindClass ("android/content/Context"),
                                                                                                  "getClassLoader",
                                                                                                  "()Ljava/lang/ClassLoader;")) };

            tryLoadingClassWithClassLoader (env, defaultClassLoader.get());

            if (classRef == nullptr)
            {
                for (auto& byteCodeLoader : byteCodeLoaders)
                {
                    tryLoadingClassWithClassLoader (env, byteCodeLoader.get());

                    if (classRef != nullptr)
                        break;
                }

                // fallback by trying to load the class from bytecode
                if (classRef == nullptr && byteCode != nullptr)
                {
                    LocalRef<jobject> byteCodeClassLoader;

                    MemoryOutputStream uncompressedByteCode;

                    {
                        MemoryInputStream rawGZipData (byteCode, byteCodeSize, false);
                        GZIPDecompressorInputStream gzipStream (&rawGZipData, false, GZIPDecompressorInputStream::gzipFormat);
                        uncompressedByteCode.writeFromInputStream (gzipStream, -1);
                    }

                    if (sdkVersion >= 26)
                    {
                        LocalRef<jbyteArray> byteArray (env->NewByteArray ((jsize) uncompressedByteCode.getDataSize()));
                        jboolean isCopy;
                        auto* dst = env->GetByteArrayElements (byteArray.get(), &isCopy);
                        memcpy (dst, uncompressedByteCode.getData(), uncompressedByteCode.getDataSize());
                        env->ReleaseByteArrayElements (byteArray.get(), dst, 0);

                        LocalRef<jobject> byteBuffer (env->CallStaticObjectMethod (JavaByteBuffer, JavaByteBuffer.wrap, byteArray.get()));

                        byteCodeClassLoader = LocalRef<jobject> (env->NewObject (AndroidInMemoryDexClassLoader,
                                                                                 AndroidInMemoryDexClassLoader.constructor,
                                                                                 byteBuffer.get(), defaultClassLoader.get()));
                    }
                    else if (uncompressedByteCode.getDataSize() >= 32)
                    {
                        auto codeCacheDir = getCodeCacheDirectory();

                        // The dex file has an embedded 20-byte long SHA-1 signature at offset 12
                        auto fileName = String::toHexString ((char*)uncompressedByteCode.getData() + 12, 20, 0) + ".dex";
                        auto dexFile = codeCacheDir.getChildFile (fileName);
                        auto optimizedDirectory = codeCacheDir.getChildFile ("optimized_cache");
                        optimizedDirectory.createDirectory();

                        if (dexFile.replaceWithData (uncompressedByteCode.getData(), uncompressedByteCode.getDataSize()))
                        {
                            byteCodeClassLoader = LocalRef<jobject> (env->NewObject (AndroidDexClassLoader,
                                                                                     AndroidDexClassLoader.constructor,
                                                                                     javaString (dexFile.getFullPathName()).get(),
                                                                                     javaString (optimizedDirectory.getFullPathName()).get(),
                                                                                     nullptr,
                                                                                     defaultClassLoader.get()));
                        }
                        else
                        {
                            // can't write to cache folder
                            jassertfalse;
                        }
                    }

                    if (byteCodeClassLoader != nullptr)
                    {
                        tryLoadingClassWithClassLoader (env, byteCodeClassLoader.get());
                        byteCodeLoaders.add (GlobalRef (byteCodeClassLoader));
                    }
                }
            }
        }

        if (classRef == nullptr)
            classRef = (jclass) env->NewGlobalRef (LocalRef<jobject> (env->FindClass (classPath)));

        jassert (classRef != nullptr);
        initialiseFields (env);
    }
}

void JNIClassBase::tryLoadingClassWithClassLoader (JNIEnv* env, jobject classLoader)
{
    LocalRef<jstring> classNameAndPackage (javaString (String (classPath).replaceCharacter (L'/', L'.')));

    // Android SDK <= 19 has a bug where the class loader might throw an exception but still return
    // a non-nullptr. So don't assign the result of this call to a jobject just yet...
    auto classObj = env->CallObjectMethod (classLoader, JavaClassLoader.loadClass, classNameAndPackage.get(), (jboolean) true);

    if (jthrowable exception = env->ExceptionOccurred())
    {
        env->ExceptionClear();
        classObj = nullptr;
    }

    // later versions of Android don't throw at all, so re-check the object
    if (classObj != nullptr)
        classRef = (jclass) env->NewGlobalRef (LocalRef<jobject> (classObj));
}

void JNIClassBase::release (JNIEnv* env)
{
    if (classRef != nullptr)
        env->DeleteGlobalRef (classRef);
}

void JNIClassBase::initialiseAllClasses (JNIEnv* env, jobject context)
{
    const Array<JNIClassBase*>& classes = getClasses();
    for (int i = classes.size(); --i >= 0;)
        classes.getUnchecked (i)->initialise (env, context);
}

void JNIClassBase::releaseAllClasses (JNIEnv* env)
{
    const Array<JNIClassBase*>& classes = getClasses();
    for (int i = classes.size(); --i >= 0;)
        classes.getUnchecked (i)->release (env);
}

jmethodID JNIClassBase::resolveMethod (JNIEnv* env, const char* methodName, const char* params)
{
    jmethodID m = env->GetMethodID (classRef, methodName, params);
    jassert (m != nullptr);
    return m;
}

jmethodID JNIClassBase::resolveStaticMethod (JNIEnv* env, const char* methodName, const char* params)
{
    jmethodID m = env->GetStaticMethodID (classRef, methodName, params);
    jassert (m != nullptr);
    return m;
}

jfieldID JNIClassBase::resolveField (JNIEnv* env, const char* fieldName, const char* signature)
{
    jfieldID f = env->GetFieldID (classRef, fieldName, signature);
    jassert (f != nullptr);
    return f;
}

jfieldID JNIClassBase::resolveStaticField (JNIEnv* env, const char* fieldName, const char* signature)
{
    jfieldID f = env->GetStaticFieldID (classRef, fieldName, signature);
    jassert (f != nullptr);
    return f;
}

void JNIClassBase::resolveCallbacks (JNIEnv* env, const Array<JNINativeMethod>& nativeCallbacks)
{
    if (nativeCallbacks.size() > 0)
        env->RegisterNatives (classRef, nativeCallbacks.begin(), (jint) nativeCallbacks.size());
}

//==============================================================================
LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const StringArray& interfaceNames,
                                       LocalRef<jobject> subclass)
{
    auto* env = getEnv();

    implementer->javaSubClass = GlobalRef (subclass);

    // you need to override at least one interface
    jassert (interfaceNames.size() > 0);

    auto classArray = LocalRef<jobject> (env->NewObjectArray (interfaceNames.size(), JavaClass, nullptr));
    LocalRef<jobject> classLoader;

    for (auto i = 0; i < interfaceNames.size(); ++i)
    {
        auto aClass = LocalRef<jobject> (env->FindClass (interfaceNames[i].toRawUTF8()));

        if (aClass != nullptr)
        {
            if (i == 0)
                classLoader = LocalRef<jobject> (env->CallObjectMethod (aClass, JavaClass.getClassLoader));

            env->SetObjectArrayElement ((jobjectArray) classArray.get(), i, aClass);
        }
        else
        {
            // interface class not found
            jassertfalse;
        }
    }

    auto invocationHandler = LocalRef<jobject> (env->NewObject (JuceInvocationHandler, JuceInvocationHandler.constructor,
                                                                reinterpret_cast<jlong> (implementer)));

    // CreateJavaInterface() is expected to be called just once for a given implementer
    jassert (implementer->invocationHandler == nullptr);

    implementer->invocationHandler = GlobalRef (invocationHandler);

    return LocalRef<jobject> (env->CallStaticObjectMethod (JavaProxy, JavaProxy.newProxyInstance,
                                                           classLoader.get(), classArray.get(),
                                                           invocationHandler.get()));
}

LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const StringArray& interfaceNames)
{
    return CreateJavaInterface (implementer, interfaceNames,
                                LocalRef<jobject> (getEnv()->NewObject (JavaObject,
                                                                        JavaObject.constructor)));
}

LocalRef<jobject> CreateJavaInterface (AndroidInterfaceImplementer* implementer,
                                       const String& interfaceName)
{
    return CreateJavaInterface (implementer, StringArray (interfaceName));
}

AndroidInterfaceImplementer::~AndroidInterfaceImplementer()
{
    clear();
}

void AndroidInterfaceImplementer::clear()
{
    if (invocationHandler != nullptr)
        getEnv()->CallVoidMethod (invocationHandler,
                                  JuceInvocationHandler.clear);
}

jobject AndroidInterfaceImplementer::invoke (jobject /*proxy*/, jobject method, jobjectArray args)
{
    auto* env = getEnv();
    return env->CallObjectMethod (method, JavaMethod.invoke, javaSubClass.get(), args);
}

jobject juce_invokeImplementer (JNIEnv*, jobject /*object*/, jlong host, jobject proxy,
                                jobject method, jobjectArray args)
{
    if (auto* myself = reinterpret_cast<AndroidInterfaceImplementer*> (host))
        return myself->invoke (proxy, method, args);

    return nullptr;
}

void juce_dispatchDelete (JNIEnv*, jobject /*object*/, jlong host)
{
    if (auto* myself = reinterpret_cast<AndroidInterfaceImplementer*> (host))
        delete myself;
}

//==============================================================================
ActivityLifecycleCallbackForwarder::ActivityLifecycleCallbackForwarder (GlobalRef ctx, ActivityLifecycleCallbacks* cb)
    : appContext (ctx),
      myself (std::invoke ([&]
      {
          const LocalRef<jobject> subclass { getEnv()->NewObject (JuceActivityCallbacksBase,
                                                                  JuceActivityCallbacksBase.constructor) };
          return CreateJavaInterface (this,
                                      { "android/app/Application$ActivityLifecycleCallbacks" },
                                      subclass);
      })),
      callbacks (cb)
{
    if (appContext != nullptr && myself != nullptr)
        getEnv()->CallVoidMethod (appContext, AndroidApplication.registerActivityLifecycleCallbacks, myself.get());
}

ActivityLifecycleCallbackForwarder::~ActivityLifecycleCallbackForwarder()
{
    if (appContext != nullptr && myself != nullptr)
        getEnv()->CallVoidMethod (appContext, AndroidApplication.unregisterActivityLifecycleCallbacks, myself.get());
}

jobject ActivityLifecycleCallbackForwarder::invoke (jobject proxy, jobject method, jobjectArray args)
{
    auto* env = getEnv();

    struct Comparator
    {
        bool operator() (const char* a, const char* b) const
        {
            return CharPointer_ASCII { a }.compare (CharPointer_ASCII { b }) < 0;
        }
    };

    static const std::map<const char*, void (*) (ActivityLifecycleCallbacks&, jobject, jobject), Comparator> entries
    {
        { "onActivityConfigurationChanged",   [] (auto& t, auto activity, auto       ) { t.onActivityConfigurationChanged (activity);          } },
        { "onActivityCreated",                [] (auto& t, auto activity, auto bundle) { t.onActivityCreated (activity, bundle);               } },
        { "onActivityDestroyed",              [] (auto& t, auto activity, auto       ) { t.onActivityDestroyed (activity);                     } },
        { "onActivityPaused",                 [] (auto& t, auto activity, auto       ) { t.onActivityPaused (activity);                        } },
        { "onActivityPostCreated",            [] (auto& t, auto activity, auto bundle) { t.onActivityPostCreated (activity, bundle);           } },
        { "onActivityPostDestroyed",          [] (auto& t, auto activity, auto       ) { t.onActivityPostDestroyed (activity);                 } },
        { "onActivityPostPaused",             [] (auto& t, auto activity, auto       ) { t.onActivityPostPaused (activity);                    } },
        { "onActivityPostResumed",            [] (auto& t, auto activity, auto       ) { t.onActivityPostResumed (activity);                   } },
        { "onActivityPostSaveInstanceState",  [] (auto& t, auto activity, auto bundle) { t.onActivityPostSaveInstanceState (activity, bundle); } },
        { "onActivityPostStarted",            [] (auto& t, auto activity, auto       ) { t.onActivityPostStarted (activity);                   } },
        { "onActivityPostStopped",            [] (auto& t, auto activity, auto       ) { t.onActivityPostStopped (activity);                   } },
        { "onActivityPreCreated",             [] (auto& t, auto activity, auto bundle) { t.onActivityPreCreated (activity, bundle);            } },
        { "onActivityPreDestroyed",           [] (auto& t, auto activity, auto       ) { t.onActivityPreDestroyed (activity);                  } },
        { "onActivityPrePaused",              [] (auto& t, auto activity, auto       ) { t.onActivityPrePaused (activity);                     } },
        { "onActivityPreResumed",             [] (auto& t, auto activity, auto       ) { t.onActivityPreResumed (activity);                    } },
        { "onActivityPreSaveInstanceState",   [] (auto& t, auto activity, auto bundle) { t.onActivityPreSaveInstanceState (activity, bundle);  } },
        { "onActivityPreStarted",             [] (auto& t, auto activity, auto       ) { t.onActivityPreStarted (activity);                    } },
        { "onActivityPreStopped",             [] (auto& t, auto activity, auto       ) { t.onActivityPreStopped (activity);                    } },
        { "onActivityResumed",                [] (auto& t, auto activity, auto       ) { t.onActivityResumed (activity);                       } },
        { "onActivitySaveInstanceState",      [] (auto& t, auto activity, auto bundle) { t.onActivitySaveInstanceState (activity, bundle);     } },
        { "onActivityStarted",                [] (auto& t, auto activity, auto       ) { t.onActivityStarted (activity);                       } },
        { "onActivityStopped",                [] (auto& t, auto activity, auto       ) { t.onActivityStopped (activity);                       } },
    };

    const auto methodName = juceString ((jstring) env->CallObjectMethod (method, JavaMethod.getName));
    const auto iter = entries.find (methodName.toRawUTF8());

    if (iter == entries.end())
        return AndroidInterfaceImplementer::invoke (proxy, method, args);

    const auto activity = env->GetArrayLength (args) > 0 ? env->GetObjectArrayElement (args, 0) : (jobject) nullptr;
    const auto bundle   = env->GetArrayLength (args) > 1 ? env->GetObjectArrayElement (args, 1) : (jobject) nullptr;
    (iter->second) (*callbacks, activity, bundle);

    return nullptr;
}

//==============================================================================
int getAndroidSDKVersion()
{
    // this is used so often that we need to cache this
    static int sdkVersion = []
    {
        // don't use any jni helpers as they might not have been initialised yet
        // when this method is used
        auto* env = getEnv();

        auto buildVersion = env->FindClass ("android/os/Build$VERSION");
        jassert (buildVersion != nullptr);

        auto sdkVersionField = env->GetStaticFieldID (buildVersion, "SDK_INT", "I");
        jassert (sdkVersionField != nullptr);

        return env->GetStaticIntField (buildVersion, sdkVersionField);
    }();

    return sdkVersion;
}

bool isPermissionDeclaredInManifest (const String& requestedPermission)
{
    auto* env = getEnv();

    LocalRef<jobject> pkgManager (env->CallObjectMethod (getAppContext().get(), AndroidContext.getPackageManager));
    LocalRef<jobject> pkgName (env->CallObjectMethod (getAppContext().get(), AndroidContext.getPackageName));
    LocalRef<jobject> pkgInfo (env->CallObjectMethod (pkgManager.get(), AndroidPackageManager.getPackageInfo,
                                                      pkgName.get(), 0x00001000 /* PERMISSIONS */));

    LocalRef<jobjectArray> permissions ((jobjectArray) env->GetObjectField (pkgInfo.get(), AndroidPackageInfo.requestedPermissions));
    int n = env->GetArrayLength (permissions);

    for (int i = 0; i < n; ++i)
    {
        LocalRef<jstring> jstr ((jstring) env->GetObjectArrayElement (permissions, i));
        String permissionId (juceString (jstr));

        if (permissionId == requestedPermission)
            return true;
    }

    return false;
}

//==============================================================================
// This byte-code is generated from native/java/com/rmsl/juce/FragmentOverlay.java with min sdk version 16
// See juce_core/native/java/README.txt on how to generate this byte-code.
static const uint8 javaFragmentOverlay[] =
{31,139,8,8,26,116,161,94,0,3,106,97,118,97,70,114,97,103,109,101,110,116,79,118,101,114,108,97,121,46,100,101,120,0,133,149,
77,136,28,69,20,199,255,53,253,181,159,179,147,221,184,140,235,198,140,43,70,197,224,172,104,36,56,99,216,152,32,204,100,226,71,
54,204,97,227,165,153,105,39,189,206,118,79,186,123,150,4,20,53,4,146,131,8,6,252,130,28,114,80,65,48,8,226,65,196,83,8,66,64,
65,146,75,252,184,152,179,160,160,4,17,5,255,175,187,58,27,150,136,195,252,250,189,122,245,234,189,170,215,213,85,93,239,248,
216,226,163,187,96,79,85,156,198,103,91,86,175,30,189,252,253,193,79,203,15,189,242,199,245,246,129,179,245,238,53,27,24,0,56,
222,126,108,26,250,183,155,182,7,145,217,199,200,86,149,201,58,37,255,248,156,143,18,229,87,186,93,47,0,47,155,192,11,148,87,
12,224,7,242,27,249,157,220,32,127,145,127,200,93,244,217,69,154,228,37,242,42,57,73,206,144,55,201,89,242,62,57,79,62,36,31,
147,11,228,34,185,76,174,144,107,228,103,242,43,249,147,216,22,80,38,139,228,9,210,36,47,146,51,228,45,114,158,92,32,95,146,175,
201,183,132,211,4,167,3,46,19,14,25,33,163,122,173,227,100,70,214,76,24,62,93,223,41,58,91,186,13,237,227,104,125,66,235,111,
208,103,82,235,239,81,47,106,253,3,234,83,90,255,196,200,234,38,250,23,212,183,104,253,18,245,105,173,127,147,230,82,152,133,
204,179,144,230,40,112,118,119,235,246,130,158,199,28,196,47,235,23,121,135,150,101,100,227,239,76,165,129,249,84,218,216,150,
202,44,142,197,21,111,79,165,137,74,42,29,220,163,199,47,164,210,194,189,200,214,172,0,157,37,211,229,55,98,103,210,160,69,108,
87,173,172,134,131,146,248,202,204,87,42,82,129,188,255,71,221,159,247,4,37,155,126,69,214,209,76,223,193,117,43,91,255,50,55,
220,44,147,61,194,48,187,217,187,28,177,38,199,212,41,245,182,243,209,186,61,202,88,69,200,72,89,255,47,28,35,107,10,43,10,135,
25,209,161,117,2,115,106,22,65,197,96,149,199,177,178,196,136,75,183,70,116,210,246,96,137,121,159,47,166,239,49,203,127,227,
127,242,59,105,254,201,52,191,212,86,246,142,12,148,247,23,150,100,62,183,205,179,56,5,83,21,117,221,108,189,231,160,101,166,
143,166,117,81,154,124,191,73,111,174,139,71,33,213,77,237,99,215,253,192,79,246,96,235,211,145,219,91,243,130,228,217,117,47,
234,187,39,30,94,117,215,93,168,6,84,19,133,102,11,170,133,249,150,27,116,163,208,239,86,221,193,160,186,223,119,251,97,47,31,
85,67,249,102,111,39,12,18,154,170,141,84,212,48,115,179,39,140,171,79,13,131,110,223,171,97,123,171,19,174,85,163,181,184,95,
93,29,118,188,234,166,244,53,76,183,100,6,213,190,27,244,170,203,73,228,7,189,26,84,27,102,187,209,104,201,179,213,66,161,221,
132,213,110,138,65,4,45,70,187,41,102,114,164,129,153,35,183,9,97,117,250,97,236,193,233,12,6,135,143,250,49,204,174,155,184,
112,186,126,188,230,199,49,38,122,94,178,55,234,13,101,42,49,28,182,90,97,208,163,57,114,131,228,144,23,15,251,52,151,194,96,
111,39,241,215,253,228,68,102,194,236,102,203,51,46,91,30,70,194,96,95,228,185,137,135,98,174,233,158,185,48,56,228,29,27,122,
113,242,156,23,73,106,63,12,98,29,173,242,223,125,122,180,19,6,203,137,27,37,152,212,138,182,143,15,54,6,96,60,202,130,236,11,
187,30,198,162,116,124,170,91,113,34,83,50,19,41,192,54,56,197,194,206,26,246,83,30,168,99,143,177,227,254,178,83,60,253,14,22,
212,3,78,177,126,233,244,10,30,55,118,220,55,79,219,187,216,73,167,39,105,129,178,248,121,155,175,191,102,254,100,90,39,121,
146,220,130,165,254,54,13,117,206,42,168,239,200,57,155,210,158,220,244,205,139,204,239,4,217,143,249,189,96,96,227,110,200,247,
172,220,15,114,118,228,119,132,141,141,123,66,85,178,182,220,21,170,148,157,11,114,190,22,42,89,124,185,63,12,237,35,231,138,
28,80,42,63,115,74,153,46,247,211,191,81,33,150,205,216,6,0,0,0,0};

//==============================================================================
#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (construct,   "<init>",   "()V") \
 METHOD (close,       "close",    "()V") \
 CALLBACK (generatedCallback<&FragmentOverlay::onActivityResultCallback>,           "onActivityResultNative",           "(JIILandroid/content/Intent;)V") \
 CALLBACK (generatedCallback<&FragmentOverlay::onCreatedCallback>,                  "onCreateNative",                   "(JLandroid/os/Bundle;)V") \
 CALLBACK (generatedCallback<&FragmentOverlay::onStartCallback>,                    "onStartNative",                    "(J)V") \
 CALLBACK (generatedCallback<&FragmentOverlay::onRequestPermissionsResultCallback>, "onRequestPermissionsResultNative", "(JI[Ljava/lang/String;[I)V")

 DECLARE_JNI_CLASS_WITH_BYTECODE (JuceFragmentOverlay, "com/rmsl/juce/FragmentOverlay", 16, javaFragmentOverlay)
#undef JNI_CLASS_MEMBERS

#define JNI_CLASS_MEMBERS(METHOD, STATICMETHOD, FIELD, STATICFIELD, CALLBACK) \
 METHOD (show,   "show",   "(Landroid/app/FragmentManager;Ljava/lang/String;)V")

 DECLARE_JNI_CLASS (AndroidDialogFragment, "android/app/DialogFragment")
#undef JNI_CLASS_MEMBERS

//==============================================================================
FragmentOverlay::FragmentOverlay()
    : native (LocalRef<jobject> (getEnv()->NewObject (JuceFragmentOverlay, JuceFragmentOverlay.construct)))
{}

FragmentOverlay::~FragmentOverlay()
{
    auto* env = getEnv();

    env->CallVoidMethod (native.get(), JuceFragmentOverlay.close);
}

void FragmentOverlay::open()
{
    auto* env = getEnv();

    LocalRef<jobject> bundle (env->NewObject (AndroidBundle, AndroidBundle.constructor));
    env->CallVoidMethod (bundle.get(), AndroidBundle.putLong, javaString ("cppThis").get(), (jlong) this);
    env->CallVoidMethod (native.get(), AndroidFragment.setArguments, bundle.get());

    LocalRef<jobject> fm (env->CallObjectMethod (getCurrentActivity().get(), AndroidActivity.getFragmentManager));
    env->CallVoidMethod (native.get(), AndroidDialogFragment.show, fm.get(), javaString ("FragmentOverlay").get());
}

void FragmentOverlay::onCreatedCallback (JNIEnv* env, FragmentOverlay& t, jobject obj)
{
    t.onCreated (LocalRef<jobject> { env->NewLocalRef (obj) });
}

void FragmentOverlay::onStartCallback (JNIEnv*, FragmentOverlay& t)
{
    t.onStart();
}

void FragmentOverlay::onRequestPermissionsResultCallback (JNIEnv* env, FragmentOverlay& t, jint requestCode, jobjectArray jPermissions, jintArray jGrantResults)
{
    Array<int> grantResults;
    int n = (jGrantResults != nullptr ? env->GetArrayLength (jGrantResults) : 0);

    if (n > 0)
    {
        auto* data = env->GetIntArrayElements (jGrantResults, nullptr);

        for (int i = 0; i < n; ++i)
            grantResults.add (data[i]);

        env->ReleaseIntArrayElements (jGrantResults, data, 0);
    }

    t.onRequestPermissionsResult (requestCode,
                                  javaStringArrayToJuce (LocalRef<jobjectArray> (jPermissions)),
                                  grantResults);
}

void FragmentOverlay::onActivityResultCallback (JNIEnv* env, FragmentOverlay& t, jint requestCode, jint resultCode, jobject data)
{
    t.onActivityResult (requestCode, resultCode, LocalRef<jobject> (env->NewLocalRef (data)));
}

jobject FragmentOverlay::getNativeHandle()
{
    return native.get();
}

//==============================================================================
void startAndroidActivityForResult (const LocalRef<jobject>& intent,
                                    int requestCode,
                                    std::function<void (int, int, LocalRef<jobject>)>&& callback)
{
    auto* launcher = new ActivityLauncher (intent, requestCode);
    launcher->callback = [launcher, c = std::move (callback)] (auto&&... args)
    {
        NullCheckedInvocation::invoke (c, args...);
        delete launcher;
    };
    launcher->open();
}

//==============================================================================
bool androidHasSystemFeature (const String& property)
{
    LocalRef<jobject> appContext (getAppContext());

    if (appContext != nullptr)
    {
        auto* env = getEnv();

        LocalRef<jobject> packageManager (env->CallObjectMethod (appContext.get(), AndroidContext.getPackageManager));

        if (packageManager != nullptr)
            return env->CallBooleanMethod (packageManager.get(),
                                           AndroidPackageManager.hasSystemFeature,
                                           javaString (property).get()) != 0;
    }

    // unable to get app's context
    jassertfalse;
    return false;
}

String audioManagerGetProperty (const String& property)
{
    auto* env = getEnv();
    LocalRef<jobject> audioManager (env->CallObjectMethod (getAppContext().get(), AndroidContext.getSystemService,
                                                           javaString ("audio").get()));

    if (audioManager != nullptr)
    {
        LocalRef<jstring> jProperty (javaString (property));

        auto methodID = env->GetMethodID (AndroidAudioManager, "getProperty", "(Ljava/lang/String;)Ljava/lang/String;");

        if (methodID != nullptr)
            return juceString (LocalRef<jstring> ((jstring) env->CallObjectMethod (audioManager.get(),
                                                                                   methodID,
                                                                                   javaString (property).get())));
    }

    return {};
}

}
