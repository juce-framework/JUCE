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

enum class BracketType
{
    none,
    open,
    close
};

static inline BracketType getBracketType (uint32_t cp)
{
   #define OPEN(code)  case code: return BracketType::open
   #define CLOSE(code) case code: return BracketType::close

    switch (cp)
    {
        OPEN(0x0028);
        CLOSE(0x0029);
        OPEN(0x005B);
        CLOSE(0x005D);
        OPEN(0x007B);
        CLOSE(0x007D);
        OPEN(0x0F3A);
        CLOSE(0x0F3B);
        OPEN(0x0F3C);
        CLOSE(0x0F3D);
        OPEN(0x169B);
        CLOSE(0x169C);
        OPEN(0x2045);
        CLOSE(0x2046);
        OPEN(0x207D);
        CLOSE(0x207E);
        OPEN(0x208D);
        CLOSE(0x208E);
        OPEN(0x2308);
        CLOSE(0x2309);
        OPEN(0x230A);
        CLOSE(0x230B);
        OPEN(0x2329);
        CLOSE(0x232A);
        OPEN(0x2768);
        CLOSE(0x2769);
        OPEN(0x276A);
        CLOSE(0x276B);
        OPEN(0x276C);
        CLOSE(0x276D);
        OPEN(0x276E);
        CLOSE(0x276F);
        OPEN(0x2770);
        CLOSE(0x2771);
        OPEN(0x2772);
        CLOSE(0x2773);
        OPEN(0x2774);
        CLOSE(0x2775);
        OPEN(0x27C5);
        CLOSE(0x27C6);
        OPEN(0x27E6);
        CLOSE(0x27E7);
        OPEN(0x27E8);
        CLOSE(0x27E9);
        OPEN(0x27EA);
        CLOSE(0x27EB);
        OPEN(0x27EC);
        CLOSE(0x27ED);
        OPEN(0x27EE);
        CLOSE(0x27EF);
        OPEN(0x2983);
        CLOSE(0x2984);
        OPEN(0x2985);
        CLOSE(0x2986);
        OPEN(0x2987);
        CLOSE(0x2988);
        OPEN(0x2989);
        CLOSE(0x298A);
        OPEN(0x298B);
        CLOSE(0x298C);
        OPEN(0x298D);
        CLOSE(0x298E);
        OPEN(0x298F);
        CLOSE(0x2990);
        OPEN(0x2991);
        CLOSE(0x2992);
        OPEN(0x2993);
        CLOSE(0x2994);
        OPEN(0x2995);
        CLOSE(0x2996);
        OPEN(0x2997);
        CLOSE(0x2998);
        OPEN(0x29D8);
        CLOSE(0x29D9);
        OPEN(0x29DA);
        CLOSE(0x29DB);
        OPEN(0x29FC);
        CLOSE(0x29FD);
        OPEN(0x2E22);
        CLOSE(0x2E23);
        OPEN(0x2E24);
        CLOSE(0x2E25);
        OPEN(0x2E26);
        CLOSE(0x2E27);
        OPEN(0x2E28);
        CLOSE(0x2E29);
        OPEN(0x2E55);
        CLOSE(0x2E56);
        OPEN(0x2E57);
        CLOSE(0x2E58);
        OPEN(0x2E59);
        CLOSE(0x2E5A);
        OPEN(0x2E5B);
        CLOSE(0x2E5C);
        OPEN(0x3008);
        CLOSE(0x3009);
        OPEN(0x300A);
        CLOSE(0x300B);
        OPEN(0x300C);
        CLOSE(0x300D);
        OPEN(0x300E);
        CLOSE(0x300F);
        OPEN(0x3010);
        CLOSE(0x3011);
        OPEN(0x3014);
        CLOSE(0x3015);
        OPEN(0x3016);
        CLOSE(0x3017);
        OPEN(0x3018);
        CLOSE(0x3019);
        OPEN(0x301A);
        CLOSE(0x301B);
        OPEN(0xFE59);
        CLOSE(0xFE5A);
        OPEN(0xFE5B);
        CLOSE(0xFE5C);
        OPEN(0xFE5D);
        CLOSE(0xFE5E);
        OPEN(0xFF08);
        CLOSE(0xFF09);
        OPEN(0xFF3B);
        CLOSE(0xFF3D);
        OPEN(0xFF5B);
        CLOSE(0xFF5D);
        OPEN(0xFF5F);
        CLOSE(0xFF60);
        OPEN(0xFF62);
        CLOSE(0xFF63);
    }

   #undef OPEN
   #undef CLOSE

    return BracketType::none;
}

static inline auto isMatchingBracketPair (uint32_t b1, uint32_t b2)
{
    // TODO: remove duplicates
    if (b1 == 0x0028 && b2 == 0x0029) return true;
    if (b1 == 0x0029 && b2 == 0x0028) return true;
    if (b1 == 0x0029 && b2 == 0x0028) return true;
    if (b1 == 0x0028 && b2 == 0x0029) return true;
    if (b1 == 0x005B && b2 == 0x005D) return true;
    if (b1 == 0x005D && b2 == 0x005B) return true;
    if (b1 == 0x005D && b2 == 0x005B) return true;
    if (b1 == 0x005B && b2 == 0x005D) return true;
    if (b1 == 0x007B && b2 == 0x007D) return true;
    if (b1 == 0x007D && b2 == 0x007B) return true;
    if (b1 == 0x007D && b2 == 0x007B) return true;
    if (b1 == 0x007B && b2 == 0x007D) return true;
    if (b1 == 0x0F3A && b2 == 0x0F3B) return true;
    if (b1 == 0x0F3B && b2 == 0x0F3A) return true;
    if (b1 == 0x0F3B && b2 == 0x0F3A) return true;
    if (b1 == 0x0F3A && b2 == 0x0F3B) return true;
    if (b1 == 0x0F3C && b2 == 0x0F3D) return true;
    if (b1 == 0x0F3D && b2 == 0x0F3C) return true;
    if (b1 == 0x0F3D && b2 == 0x0F3C) return true;
    if (b1 == 0x0F3C && b2 == 0x0F3D) return true;
    if (b1 == 0x169B && b2 == 0x169C) return true;
    if (b1 == 0x169C && b2 == 0x169B) return true;
    if (b1 == 0x169C && b2 == 0x169B) return true;
    if (b1 == 0x169B && b2 == 0x169C) return true;
    if (b1 == 0x2045 && b2 == 0x2046) return true;
    if (b1 == 0x2046 && b2 == 0x2045) return true;
    if (b1 == 0x2046 && b2 == 0x2045) return true;
    if (b1 == 0x2045 && b2 == 0x2046) return true;
    if (b1 == 0x207D && b2 == 0x207E) return true;
    if (b1 == 0x207E && b2 == 0x207D) return true;
    if (b1 == 0x207E && b2 == 0x207D) return true;
    if (b1 == 0x207D && b2 == 0x207E) return true;
    if (b1 == 0x208D && b2 == 0x208E) return true;
    if (b1 == 0x208E && b2 == 0x208D) return true;
    if (b1 == 0x208E && b2 == 0x208D) return true;
    if (b1 == 0x208D && b2 == 0x208E) return true;
    if (b1 == 0x2308 && b2 == 0x2309) return true;
    if (b1 == 0x2309 && b2 == 0x2308) return true;
    if (b1 == 0x2309 && b2 == 0x2308) return true;
    if (b1 == 0x2308 && b2 == 0x2309) return true;
    if (b1 == 0x230A && b2 == 0x230B) return true;
    if (b1 == 0x230B && b2 == 0x230A) return true;
    if (b1 == 0x230B && b2 == 0x230A) return true;
    if (b1 == 0x230A && b2 == 0x230B) return true;
    if (b1 == 0x2329 && b2 == 0x232A) return true;
    if (b1 == 0x232A && b2 == 0x2329) return true;
    if (b1 == 0x232A && b2 == 0x2329) return true;
    if (b1 == 0x2329 && b2 == 0x232A) return true;
    if (b1 == 0x2768 && b2 == 0x2769) return true;
    if (b1 == 0x2769 && b2 == 0x2768) return true;
    if (b1 == 0x2769 && b2 == 0x2768) return true;
    if (b1 == 0x2768 && b2 == 0x2769) return true;
    if (b1 == 0x276A && b2 == 0x276B) return true;
    if (b1 == 0x276B && b2 == 0x276A) return true;
    if (b1 == 0x276B && b2 == 0x276A) return true;
    if (b1 == 0x276A && b2 == 0x276B) return true;
    if (b1 == 0x276C && b2 == 0x276D) return true;
    if (b1 == 0x276D && b2 == 0x276C) return true;
    if (b1 == 0x276D && b2 == 0x276C) return true;
    if (b1 == 0x276C && b2 == 0x276D) return true;
    if (b1 == 0x276E && b2 == 0x276F) return true;
    if (b1 == 0x276F && b2 == 0x276E) return true;
    if (b1 == 0x276F && b2 == 0x276E) return true;
    if (b1 == 0x276E && b2 == 0x276F) return true;
    if (b1 == 0x2770 && b2 == 0x2771) return true;
    if (b1 == 0x2771 && b2 == 0x2770) return true;
    if (b1 == 0x2771 && b2 == 0x2770) return true;
    if (b1 == 0x2770 && b2 == 0x2771) return true;
    if (b1 == 0x2772 && b2 == 0x2773) return true;
    if (b1 == 0x2773 && b2 == 0x2772) return true;
    if (b1 == 0x2773 && b2 == 0x2772) return true;
    if (b1 == 0x2772 && b2 == 0x2773) return true;
    if (b1 == 0x2774 && b2 == 0x2775) return true;
    if (b1 == 0x2775 && b2 == 0x2774) return true;
    if (b1 == 0x2775 && b2 == 0x2774) return true;
    if (b1 == 0x2774 && b2 == 0x2775) return true;
    if (b1 == 0x27C5 && b2 == 0x27C6) return true;
    if (b1 == 0x27C6 && b2 == 0x27C5) return true;
    if (b1 == 0x27C6 && b2 == 0x27C5) return true;
    if (b1 == 0x27C5 && b2 == 0x27C6) return true;
    if (b1 == 0x27E6 && b2 == 0x27E7) return true;
    if (b1 == 0x27E7 && b2 == 0x27E6) return true;
    if (b1 == 0x27E7 && b2 == 0x27E6) return true;
    if (b1 == 0x27E6 && b2 == 0x27E7) return true;
    if (b1 == 0x27E8 && b2 == 0x27E9) return true;
    if (b1 == 0x27E9 && b2 == 0x27E8) return true;
    if (b1 == 0x27E9 && b2 == 0x27E8) return true;
    if (b1 == 0x27E8 && b2 == 0x27E9) return true;
    if (b1 == 0x27EA && b2 == 0x27EB) return true;
    if (b1 == 0x27EB && b2 == 0x27EA) return true;
    if (b1 == 0x27EB && b2 == 0x27EA) return true;
    if (b1 == 0x27EA && b2 == 0x27EB) return true;
    if (b1 == 0x27EC && b2 == 0x27ED) return true;
    if (b1 == 0x27ED && b2 == 0x27EC) return true;
    if (b1 == 0x27ED && b2 == 0x27EC) return true;
    if (b1 == 0x27EC && b2 == 0x27ED) return true;
    if (b1 == 0x27EE && b2 == 0x27EF) return true;
    if (b1 == 0x27EF && b2 == 0x27EE) return true;
    if (b1 == 0x27EF && b2 == 0x27EE) return true;
    if (b1 == 0x27EE && b2 == 0x27EF) return true;
    if (b1 == 0x2983 && b2 == 0x2984) return true;
    if (b1 == 0x2984 && b2 == 0x2983) return true;
    if (b1 == 0x2984 && b2 == 0x2983) return true;
    if (b1 == 0x2983 && b2 == 0x2984) return true;
    if (b1 == 0x2985 && b2 == 0x2986) return true;
    if (b1 == 0x2986 && b2 == 0x2985) return true;
    if (b1 == 0x2986 && b2 == 0x2985) return true;
    if (b1 == 0x2985 && b2 == 0x2986) return true;
    if (b1 == 0x2987 && b2 == 0x2988) return true;
    if (b1 == 0x2988 && b2 == 0x2987) return true;
    if (b1 == 0x2988 && b2 == 0x2987) return true;
    if (b1 == 0x2987 && b2 == 0x2988) return true;
    if (b1 == 0x2989 && b2 == 0x298A) return true;
    if (b1 == 0x298A && b2 == 0x2989) return true;
    if (b1 == 0x298A && b2 == 0x2989) return true;
    if (b1 == 0x2989 && b2 == 0x298A) return true;
    if (b1 == 0x298B && b2 == 0x298C) return true;
    if (b1 == 0x298C && b2 == 0x298B) return true;
    if (b1 == 0x298C && b2 == 0x298B) return true;
    if (b1 == 0x298B && b2 == 0x298C) return true;
    if (b1 == 0x298D && b2 == 0x2990) return true;
    if (b1 == 0x2990 && b2 == 0x298D) return true;
    if (b1 == 0x298E && b2 == 0x298F) return true;
    if (b1 == 0x298F && b2 == 0x298E) return true;
    if (b1 == 0x298F && b2 == 0x298E) return true;
    if (b1 == 0x298E && b2 == 0x298F) return true;
    if (b1 == 0x2990 && b2 == 0x298D) return true;
    if (b1 == 0x298D && b2 == 0x2990) return true;
    if (b1 == 0x2991 && b2 == 0x2992) return true;
    if (b1 == 0x2992 && b2 == 0x2991) return true;
    if (b1 == 0x2992 && b2 == 0x2991) return true;
    if (b1 == 0x2991 && b2 == 0x2992) return true;
    if (b1 == 0x2993 && b2 == 0x2994) return true;
    if (b1 == 0x2994 && b2 == 0x2993) return true;
    if (b1 == 0x2994 && b2 == 0x2993) return true;
    if (b1 == 0x2993 && b2 == 0x2994) return true;
    if (b1 == 0x2995 && b2 == 0x2996) return true;
    if (b1 == 0x2996 && b2 == 0x2995) return true;
    if (b1 == 0x2996 && b2 == 0x2995) return true;
    if (b1 == 0x2995 && b2 == 0x2996) return true;
    if (b1 == 0x2997 && b2 == 0x2998) return true;
    if (b1 == 0x2998 && b2 == 0x2997) return true;
    if (b1 == 0x2998 && b2 == 0x2997) return true;
    if (b1 == 0x2997 && b2 == 0x2998) return true;
    if (b1 == 0x29D8 && b2 == 0x29D9) return true;
    if (b1 == 0x29D9 && b2 == 0x29D8) return true;
    if (b1 == 0x29D9 && b2 == 0x29D8) return true;
    if (b1 == 0x29D8 && b2 == 0x29D9) return true;
    if (b1 == 0x29DA && b2 == 0x29DB) return true;
    if (b1 == 0x29DB && b2 == 0x29DA) return true;
    if (b1 == 0x29DB && b2 == 0x29DA) return true;
    if (b1 == 0x29DA && b2 == 0x29DB) return true;
    if (b1 == 0x29FC && b2 == 0x29FD) return true;
    if (b1 == 0x29FD && b2 == 0x29FC) return true;
    if (b1 == 0x29FD && b2 == 0x29FC) return true;
    if (b1 == 0x29FC && b2 == 0x29FD) return true;
    if (b1 == 0x2E22 && b2 == 0x2E23) return true;
    if (b1 == 0x2E23 && b2 == 0x2E22) return true;
    if (b1 == 0x2E23 && b2 == 0x2E22) return true;
    if (b1 == 0x2E22 && b2 == 0x2E23) return true;
    if (b1 == 0x2E24 && b2 == 0x2E25) return true;
    if (b1 == 0x2E25 && b2 == 0x2E24) return true;
    if (b1 == 0x2E25 && b2 == 0x2E24) return true;
    if (b1 == 0x2E24 && b2 == 0x2E25) return true;
    if (b1 == 0x2E26 && b2 == 0x2E27) return true;
    if (b1 == 0x2E27 && b2 == 0x2E26) return true;
    if (b1 == 0x2E27 && b2 == 0x2E26) return true;
    if (b1 == 0x2E26 && b2 == 0x2E27) return true;
    if (b1 == 0x2E28 && b2 == 0x2E29) return true;
    if (b1 == 0x2E29 && b2 == 0x2E28) return true;
    if (b1 == 0x2E29 && b2 == 0x2E28) return true;
    if (b1 == 0x2E28 && b2 == 0x2E29) return true;
    if (b1 == 0x2E55 && b2 == 0x2E56) return true;
    if (b1 == 0x2E56 && b2 == 0x2E55) return true;
    if (b1 == 0x2E56 && b2 == 0x2E55) return true;
    if (b1 == 0x2E55 && b2 == 0x2E56) return true;
    if (b1 == 0x2E57 && b2 == 0x2E58) return true;
    if (b1 == 0x2E58 && b2 == 0x2E57) return true;
    if (b1 == 0x2E58 && b2 == 0x2E57) return true;
    if (b1 == 0x2E57 && b2 == 0x2E58) return true;
    if (b1 == 0x2E59 && b2 == 0x2E5A) return true;
    if (b1 == 0x2E5A && b2 == 0x2E59) return true;
    if (b1 == 0x2E5A && b2 == 0x2E59) return true;
    if (b1 == 0x2E59 && b2 == 0x2E5A) return true;
    if (b1 == 0x2E5B && b2 == 0x2E5C) return true;
    if (b1 == 0x2E5C && b2 == 0x2E5B) return true;
    if (b1 == 0x2E5C && b2 == 0x2E5B) return true;
    if (b1 == 0x2E5B && b2 == 0x2E5C) return true;
    if (b1 == 0x3008 && b2 == 0x3009) return true;
    if (b1 == 0x3009 && b2 == 0x3008) return true;
    if (b1 == 0x3009 && b2 == 0x3008) return true;
    if (b1 == 0x3008 && b2 == 0x3009) return true;
    if (b1 == 0x300A && b2 == 0x300B) return true;
    if (b1 == 0x300B && b2 == 0x300A) return true;
    if (b1 == 0x300B && b2 == 0x300A) return true;
    if (b1 == 0x300A && b2 == 0x300B) return true;
    if (b1 == 0x300C && b2 == 0x300D) return true;
    if (b1 == 0x300D && b2 == 0x300C) return true;
    if (b1 == 0x300D && b2 == 0x300C) return true;
    if (b1 == 0x300C && b2 == 0x300D) return true;
    if (b1 == 0x300E && b2 == 0x300F) return true;
    if (b1 == 0x300F && b2 == 0x300E) return true;
    if (b1 == 0x300F && b2 == 0x300E) return true;
    if (b1 == 0x300E && b2 == 0x300F) return true;
    if (b1 == 0x3010 && b2 == 0x3011) return true;
    if (b1 == 0x3011 && b2 == 0x3010) return true;
    if (b1 == 0x3011 && b2 == 0x3010) return true;
    if (b1 == 0x3010 && b2 == 0x3011) return true;
    if (b1 == 0x3014 && b2 == 0x3015) return true;
    if (b1 == 0x3015 && b2 == 0x3014) return true;
    if (b1 == 0x3015 && b2 == 0x3014) return true;
    if (b1 == 0x3014 && b2 == 0x3015) return true;
    if (b1 == 0x3016 && b2 == 0x3017) return true;
    if (b1 == 0x3017 && b2 == 0x3016) return true;
    if (b1 == 0x3017 && b2 == 0x3016) return true;
    if (b1 == 0x3016 && b2 == 0x3017) return true;
    if (b1 == 0x3018 && b2 == 0x3019) return true;
    if (b1 == 0x3019 && b2 == 0x3018) return true;
    if (b1 == 0x3019 && b2 == 0x3018) return true;
    if (b1 == 0x3018 && b2 == 0x3019) return true;
    if (b1 == 0x301A && b2 == 0x301B) return true;
    if (b1 == 0x301B && b2 == 0x301A) return true;
    if (b1 == 0x301B && b2 == 0x301A) return true;
    if (b1 == 0x301A && b2 == 0x301B) return true;
    if (b1 == 0xFE59 && b2 == 0xFE5A) return true;
    if (b1 == 0xFE5A && b2 == 0xFE59) return true;
    if (b1 == 0xFE5A && b2 == 0xFE59) return true;
    if (b1 == 0xFE59 && b2 == 0xFE5A) return true;
    if (b1 == 0xFE5B && b2 == 0xFE5C) return true;
    if (b1 == 0xFE5C && b2 == 0xFE5B) return true;
    if (b1 == 0xFE5C && b2 == 0xFE5B) return true;
    if (b1 == 0xFE5B && b2 == 0xFE5C) return true;
    if (b1 == 0xFE5D && b2 == 0xFE5E) return true;
    if (b1 == 0xFE5E && b2 == 0xFE5D) return true;
    if (b1 == 0xFE5E && b2 == 0xFE5D) return true;
    if (b1 == 0xFE5D && b2 == 0xFE5E) return true;
    if (b1 == 0xFF08 && b2 == 0xFF09) return true;
    if (b1 == 0xFF09 && b2 == 0xFF08) return true;
    if (b1 == 0xFF09 && b2 == 0xFF08) return true;
    if (b1 == 0xFF08 && b2 == 0xFF09) return true;
    if (b1 == 0xFF3B && b2 == 0xFF3D) return true;
    if (b1 == 0xFF3D && b2 == 0xFF3B) return true;
    if (b1 == 0xFF3D && b2 == 0xFF3B) return true;
    if (b1 == 0xFF3B && b2 == 0xFF3D) return true;
    if (b1 == 0xFF5B && b2 == 0xFF5D) return true;
    if (b1 == 0xFF5D && b2 == 0xFF5B) return true;
    if (b1 == 0xFF5D && b2 == 0xFF5B) return true;
    if (b1 == 0xFF5B && b2 == 0xFF5D) return true;
    if (b1 == 0xFF5F && b2 == 0xFF60) return true;
    if (b1 == 0xFF60 && b2 == 0xFF5F) return true;
    if (b1 == 0xFF60 && b2 == 0xFF5F) return true;
    if (b1 == 0xFF5F && b2 == 0xFF60) return true;
    if (b1 == 0xFF62 && b2 == 0xFF63) return true;
    if (b1 == 0xFF63 && b2 == 0xFF62) return true;
    if (b1 == 0xFF63 && b2 == 0xFF62) return true;
    if (b1 == 0xFF62 && b2 == 0xFF63) return true;

    return false;
}

} // namespace juce
