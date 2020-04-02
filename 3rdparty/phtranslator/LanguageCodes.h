#ifndef _LANGUAGECODES_H__195ADDC0_CF4A_44c0_AC65_8247CD83E6AA__
#define _LANGUAGECODES_H__195ADDC0_CF4A_44c0_AC65_8247CD83E6AA__

#include "PhTranslator.h"

namespace PhTranslation
{
	namespace Bengali
	{
        const VowelDef Vowels[] =
        {
            "a",    2437, 0,    
            "aa",   2438, 2494, 
            "A",    2438, 2494, 
            "i",    2439, 2495, 
            "ee",   2440, 2496, 
            "I",    2440, 2496, 
            "u",    2441, 2497, 
            "oo",   2442, 2498, 
            "U",    2442, 2498, 
            "zr",   0x098B, 0x09C3, 
            "zl",   0x098C, 0x09E2, 
            "e",    0x098F, 0x09C7, 
            "E",    0x098F, 0x09C7, 
            "ai",   0x0990, 0x09C8, 
            "o",    0x0993, 0x09CB, 
            "O",    0x0993, 0x09CB, 
            "au",   0x0994, 0x09CC, 
            "zR",   0x09E0, 0x09C4, 
            "zL",   0x09E1, 0x09E3, 
            "AO",   2433, 2433, 
            "M",    2434, 2434, 
            "H",    2435, 2435, 
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    2453,
            "K",    2453,
            "kh",    2454,
            "Kh",    2454,
            "g",    2455,
            "G",    2455,
            "gh",    2456,
            "Gh",    2456,
            "NGN",    2457,
            "ch",    2458,
            "Ch",    2459,
            "j",    2460,
            "J",    2460,
            "jh",    2461,
            "Jh",    2461,
            "NY",    2462,
            "T",    2463,
            "Th",    2464,
            "D",    2465,
            "Dh",    2466,
            "N",    2467,
            "t",    2468,
            "th",    2469,
            "d",    2470,
            "dh",    2471,
            "n",    2472,
            "p",    2474,
            "ph",    2475,
            "f",    2475,
            "b",    2476,
            "B",    2476,
            "bh",    2477,
            "Bh",    2477,
            "m",    2478,
            "y",    2479,
            "r",    2480,
            "R",    2480,
            "l",    2482,
            "L",    2482,
            "v",    2476,
            "V",    2476,
            "w",    2476,
            "w",    2476,
            "sh",    2486,
            "Sh",    2487,
            "s",    2488,
            "h",    2489,
			"zd",	0x09DC,
			"zdh",	0x09DD,
			"zy",	0x09DF
        };

        const unsigned int uHalant = 0x09CD;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = // Currently no special Digits. Replace with any actual content if needed.
        {
            "z0",    0x09E6,
            "z1",    0x09E7,
            "z2",    0x09E8,
            "z3",    0x09E9,
            "z4",    0x09EA,
            "z5",    0x09EB,
            "z6",    0x09EC,
            "z7",    0x09ED,
            "z8",    0x09EE,
            "z9",    0x09EF
        };

        const SpecialSymbolDef SpecialSymbols[] = 
		{ 
			"|",	2404,
			"||",	2405
		};

	}	// namespace Bengali

	namespace Gujarati
	{
        const VowelDef Vowels[] =
        {
            "a",    0x0A85, 0,    
            "aa",   0x0A86, 0x0ABE, 
            "A",    0x0A86, 0x0ABE, 
            "i",    0x0A87, 0x0ABF, 
            "ee",   0x0A88, 0x0AC0, 
            "I",    0x0A88, 0x0AC0, 
            "u",    0x0A89, 0x0AC1, 
            "oo",   0x0A8A, 0x0AC2, 
            "U",    0x0A8A, 0x0AC2, 
            "zr",   0x0A8B, 0x0AC3, 
            "zl",   0x0A8C, 0x0AE2, 
            "e",    0x0A8D, 0x0AC5, 
            "E",    0X0A8F, 0X0AC7, 
            "ai",   0x0A90, 0x0AC8, 
            "o",    0x0A91, 0x0AC9, 
            "O",    0X0A93, 0X0ACB, 
            "au",   0x0A94, 0x0ACC, 
            "zR",   0x0AE0, 0x0AC4, 
            "zL",   0x0AE1, 0x0AE3, 
            "AO",   0x0A81, 0x0A81, 
            "M",    0x0A82, 0x0A82, 
            "H",    0x0A83, 0x0A83, 
            "OM",    0x0AD0, 0x0AD0, 
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    0x0A95,
            "K",    0x0A95,
            "kh",    0x0A96,
            "Kh",    0x0A96,
            "g",    0x0A97,
            "G",    0x0A97,
            "gh",    0x0A98,
            "Gh",    0x0A98,
            "NGN",    0x0A99,
            "ch",    0x0A9A,
            "Ch",    0X0A9B,
            "j",    0x0A9C,
            "J",    0x0A9C,
            "jh",    0x0A9D,
            "Jh",    0x0A9D,
            "NY",    0x0A9E,
            "T",    0x0A9F,
            "Th",    0x0AA0,
            "D",    0x0AA1,
            "Dh",    0x0AA2,
            "N",    0x0AA3,
            "t",    0x0AA4,
            "th",    0x0AA5,
            "d",    0x0AA6,
            "dh",    0x0AA7,
            "n",    0x0AA8,
            "p",    0x0AAA,
            "ph",    0x0AAB,
            "f",    0x0AAB,
            "b",    0x0AAC,
            "B",    0x0AAC,
            "bh",    0x0AAD,
            "Bh",    0x0AAD,
            "m",    0x0AAE,
            "y",    0x0AAF,
            "r",    0x0AB0,
            "R",    0x0AB0,
            "l",    0x0AB2,
            "L",    0x0AB3,
            "v",    0x0AB5,
            "V",    0x0AB5,
            "w",    0x0AB5,
            "W",    0x0AB5,
            "sh",    0x0AB6,
            "Sh",    0x0AB7,
            "s",    0x0AB8,
            "h",    0x0AB9,
        };

        const unsigned int uHalant = 0x0ACD;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = // Currently no special Digits. Replace with any actual content if needed.
        {
            "z0",    0x0AE6,
            "z1",    0x0AE7,
            "z2",    0x0AE8,
            "z3",    0x0AE9,
            "z4",    0x0AEA,
            "z5",    0x0AEB,
            "z6",    0x0AEC,
            "z7",    0x0AED,
            "z8",    0x0AEE,
            "z9",    0x0AEF
        };

        const SpecialSymbolDef SpecialSymbols[] = 
		{
			"zS",	0x0ABD,
			"|",	2404,
			"||",	2405
		};

	}	// namespace Gujarati

    namespace Hindi
    {
        const VowelDef Vowels[] =
        {
            "a",    2309, 0,    // అ
            "aa",   2310, 2366, // ఆ
            "A",    2310, 2366, // ఆ
            "i",    2311, 2367, // ఇ
            "ee",   2312, 2368, // ఈ
            "I",    2312, 2368, // ఈ
            "u",    2313, 2369, // ఉ
            "oo",   2314, 2370, // ఊ
            "U",    2314, 2370, // ఊ
            "zr",   0x090B, 0x0943, // ఋ
            "zl",   0x090C, 0x0962, // ఌ
            "e",    0x090D, 0X0945, // ఎ
            "E",    2319, 2375, // ఏ
            "ai",   2320, 2376, // ఐ
            "o",    0X0911, 0X0949, // ఒ
            "O",    2323, 2379, // ఓ
            "au",   2324, 2380, // ఔ
            "zR",   0x0960, 0x0944, // ౠ
            "zL",   0x0961, 0x0963, // ౡ
            "AO",   0x0901, 0x0901, // ఁ
            "M",    2306, 2306, // ం
            "H",    2307, 2307, // ః
			"OM",	2384, 2384	// ॐ
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    2325,
            "K",    2325,
            "kh",    2326,
            "Kh",    2326,
            "g",    2327,
            "G",    2327,
            "gh",    2328,
            "Gh",    2328,
            "NGN",    2329,
            "ch",    2330,
            "Ch",    2331,
            "j",    2332,
            "J",    2332,
            "jh",    2333,
            "Jh",    2333,
            "NY",    2334,
            "T",    2335,
            "Th",    2336,
            "D",    2337,
            "Dh",    2338,
            "N",    2339,
            "t",    2340,
            "th",    2341,
            "d",    2342,
            "dh",    2343,
            "n",    2344,
            "zN",    2345,
            "p",    2346,
            "ph",    2347,
            "f",    2347,
            "b",    2348,
            "B",    2348,
            "bh",    2349,
            "Bh",    2349,
            "m",    2350,
            "y",    2351,
            "r",    2352,
            "R",    2353,
            "l",    2354,
            "L",    2355,
            "zL",    2356,
            "v",    2357,
            "V",    2357,
            "w",    2357,
            "W",    2357,
            "sh",    2358,
            "Sh",    2359,
            "s",    2360,
            "h",    2361,
			"zk",	2392,
			"zkh",	2393,
			"zg",	2394,
			"zj",	2395,
			"zd",	2396,
			"zdh",	2397,
			"zph",	2398,
			"zy",	2399
        };

        const unsigned int uHalant = 2381;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0966,
            "z1",    0x0967,
            "z2",    0x0968,
            "z3",    0x0969,
            "z4",    0x096A,
            "z5",    0x096B,
            "z6",    0x096C,
            "z7",    0x096D,
            "z8",    0x096E,
            "z9",    0x096F
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"zS",	2365,
			"|",	2404,
			"||",	2405
		};

    } // namespace Hindi

    namespace Kannada
    {
        const VowelDef Vowels[] =
        {
            "a",    3205, 0,    // అ
            "aa",   3206, 3262, // ఆ
            "A",    3206, 3262, // ఆ
            "i",    3207, 3263, // ఇ
            "ee",   3208, 3264, // ఈ
            "I",    3208, 3264, // ఈ
            "u",    3209, 3265, // ఉ
            "oo",   3210, 3266, // ఊ
            "U",    3210, 3266, // ఊ
            "zr",   0x0C8B, 0x0CC3, // ఋ
            "zl",   0x0C8C, 0x0CE2, // ఌ
            "e",    3214, 3270, // ఎ
            "E",    3215, 3271, // ఏ
            "ai",   3216, 3272, // ఐ
            "o",    3218, 3274, // ఒ
            "O",    3219, 3275, // ఓ
            "au",   3220, 3276, // ఔ
            "zR",   0x0CE0, 0x0CC4, // ౠ
            "zL",   0x0CE1, 0x0CE3, // ౡ
            //"AO",   3073, 3073, // ఁ
            "M",    3202, 3202, // ం
            "H",    3203, 3203, // ః
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    3221,
            "K",    3221,
            "kh",    3222,
            "Kh",    3222,
            "g",    3223,
            "G",    3223,
            "gh",    3224,
            "Gh",    3224,
            "NGN",    3225,
            "ch",    3226,
            "Ch",    3227,
            "j",    3228,
            "J",    3228,
            "jh",    3229,
            "Jh",    3229,
            "NY",    3230,
            "T",    3231,
            "Th",    3232,
            "D",    3233,
            "Dh",    3234,
            "N",    3235,
            "t",    3236,
            "th",    3237,
            "d",    3238,
            "dh",    3239,
            "n",    3240,
            "p",    3242,
            "ph",    3243,
            "f",    3294,
            "b",    3244,
            "B",    3244,
            "bh",    3245,
            "Bh",    3245,
            "m",    3246,
            "y",    3247,
            "r",    3248,
            "R",    3249,
            "l",    3250,
            "L",    3251,
            "v",    3253,
            "V",    3253,
            "w",    3253,
            "sh",    3254,
            "Sh",    3255,
            "s",    3256,
            "h",    3257,
        };

        const unsigned int uHalant = 0xCCD;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0CE6,
            "z1",    0x0CE7,
            "z2",    0x0CE8,
            "z3",    0x0CE9,
            "z4",    0x0CEA,
            "z5",    0x0CEB,
            "z6",    0x0CEC,
            "z7",    0x0CED,
            "z8",    0x0CEE,
            "z9",    0x0CEF
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"zs",	0x0CBD,
			"|",	2404,
			"||",	2405
		};

    } // namespace Kannada

    namespace Malayalam
    {
        const VowelDef Vowels[] =
        {
            "a",    3333, 0,    // అ
            "aa",   3334, 3390, // ఆ
            "A",    3334, 3390, // ఆ
            "i",    3335, 3391, // ఇ
            "ee",   3336, 3392, // ఈ
            "I",    3336, 3392, // ఈ
            "u",    3337, 3393, // ఉ
            "oo",   3338, 3394, // ఊ
            "U",    3338, 3394, // ఊ
            "zr",   3339, 3395, // ఋ
            "zl",   3340, 3426, // ఌ
            "e",    3342, 3398, // ఎ
            "E",    3343, 3399, // ఏ
            "ai",   3344, 3400, // ఐ
            "o",    3346, 3402, // ఒ
            "O",    3347, 3403, // ఓ
            "au",   3348, 3404, // ఔ
            "zR",   3424, 3396, // ౠ
            "zL",   3425, 3427, // ౡ
            //"AO",   2305, 2305, // ఁ
            "M",    3330, 3330, // ం
            "H",    3331, 3331, // ః
            //"OM",   2384, 2384, //
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    3349,
            "K",    3349,
            "kh",    3350,
            "Kh",    3350,
            "g",    3351,
            "G",    3351,
            "gh",    3352,
            "Gh",    3352,
            "NGN",    3353,
            "ch",    3354,
            "Ch",    3355,
            "j",    3356,
            "J",    3356,
            "jh",    3357,
            "Jh",    3357,
            "NY",    3358,
            "T",    3359,
            "Th",    3360,
            "D",    3361,
            "Dh",    3362,
            "N",    3363,
            "t",    3364,
            "th",    3365,
            "d",    3366,
            "dh",    3367,
            "n",    3368,
            "p",    3370,
            "ph",    3371,
            "f",    3371,
            "b",    3372,
            "B",    3372,
            "bh",    3373,
            "Bh",    3373,
            "m",    3374,
            "y",    3375,
            "r",    3376,
            "R",    3377,
            "l",    3378,
            "L",    3379,
            "zL",    3380,
            "v",    3381,
            "V",    3381,
            "w",    3381,
            "W",    3381,
            "sh",    3382,
            "Sh",    3383,
            "s",    3384,
            "h",    3385,
			//"zk",	2392,
			//"zkh",	2393,
			//"zg",	2394,
			//"zj",	2395,
			//"zd",	2396,
			//"zdh",	2397,
			//"zph",	2398,
			//"zy",	2399
        };

        const unsigned int uHalant = 0xD4D;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0D66,
            "z1",    0x0D67,
            "z2",    0x0D68,
            "z3",    0x0D69,
            "z4",    0x0D6A,
            "z5",    0x0D6B,
            "z6",    0x0D6C,
            "z7",    0x0D6D,
            "z8",    0x0D6E,
            "z9",    0x0D6F
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"|",	2404,
			"||",	2405
		};

    } // namespace Malayalam

    namespace Oriya
    {
        const VowelDef Vowels[] =
        {
            "a",    2821, 0,    // అ
            "aa",   2822, 2878, // ఆ
            "A",    2822, 2878, // ఆ
            "i",    2823, 2879, // ఇ
            "ee",   2824, 2880, // ఈ
            "I",    2824, 2880, // ఈ
            "u",    2825, 2881, // ఉ
            "oo",   2826, 2882, // ఊ
            "U",    2826, 2882, // ఊ
            "zr",   2827, 2883, // ఋ
            "zl",   2828, 2914, // ఌ
            "e",    2831, 2887, // ఎ
            "E",    2831, 2887, // ఏ
            "ai",   2832, 2888, // ఐ
            "o",    2835, 2891, // ఒ
            "O",    2835, 2891, // ఓ
            "au",   2836, 2892, // ఔ
            "zR",   2912, 2884, // ౠ
            "zL",   2913, 2915, // ౡ
            "AO",   2817, 2817, // ఁ
            "M",    2818, 2818, // ం
            "H",    2819, 2819, // ః
        };

		const ConsonantDef Consonants[] = 
        {
            "k",    2837,
            "K",    2837,
            "kh",    2838,
            "Kh",    2838,
            "g",    2839,
            "G",    2839,
            "gh",    2840,
            "Gh",    2840,
            "NGN",    2841,
            "ch",    2842,
            "Ch",    2843,
            "j",    2844,
            "J",    2844,
            "jh",    2845,
            "Jh",    2845,
            "NY",    2846,
            "T",    2847,
            "Th",    2848,
            "D",    2849,
            "Dh",    2850,
            "N",    2851,
            "t",    2852,
            "th",    2853,
            "d",    2854,
            "dh",    2855,
            "n",    2856,
            "p",    2858,
            "ph",    2859,
            "f",    2859,
            "b",    2860,
            "B",    2860,
            "bh",    2861,
            "Bh",    2861,
            "m",    2862,
            "y",    2863,
            "r",    2864,
            //"R",    2353,
            "l",    2866,
            "L",    2867,
            //"zL",    2356,
            "v",    2869,
            "V",    2869,
            "w",    2869,
            "W",    2869,
            "sh",    2870,
            "Sh",    2871,
            "s",    2872,
            "h",    2873,
			//"zk",	2392,
			//"zkh",	2393,
			//"zg",	2394,
			//"zj",	2395,
			"zd",	2908,
			"zdh",	2909,
			//"zph",	2398,
			"zy",	2911
        };

        const unsigned int uHalant = 0xB4D;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0B66,
            "z1",    0x0B67,
            "z2",    0x0B68,
            "z3",    0x0B69,
            "z4",    0x0B6A,
            "z5",    0x0B6B,
            "z6",    0x0B6C,
            "z7",    0x0B6D,
            "z8",    0x0B6E,
            "z9",    0x0B6F
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"|",	2404,
			"||",	2405
		};

    } // namespace Oriya

    namespace Punjabi
    {
        const VowelDef Vowels[] =
        {
            "a",    2565, 0,    // అ
            "aa",   2566, 2622, // ఆ
            "A",    2566, 2622, // ఆ
            "i",    2567, 2623, // ఇ
            "ee",   2568, 2624, // ఈ
            "I",    2568, 2624, // ఈ
            "u",    2569, 2625, // ఉ
            "oo",   2570, 2626, // ఊ
            "U",    2570, 2626, // ఊ
            //"zr",   2315, 2371, // ఋ
            //"zl",   3084, 3170, // ఌ
            "e",    2575, 2631, // ఎ
            "E",    2575, 2631, // ఏ
            "ai",   2576, 2632, // ఐ
            "o",    2579, 2635, // ఒ
            "O",    2579, 2635, // ఓ
            "au",   2580, 2636, // ఔ
            //"zR",   2400, 2372, // ౠ
            //"zL",   3169, 3171, // ౡ
            "AO",   2561, 2561, // ఁ
            "M",    2562, 2562, // ం
            "H",    2563, 2563, // ః
            //"OM",   2384, 2384, //
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    2581,
            "K",    2581,
            "kh",    2582,
            "Kh",    2582,
            "g",    2583,
            "G",    2583,
            "gh",    2584,
            "Gh",    2584,
            "NGN",    2585,
            "ch",    2586,
            "Ch",    2587,
            "j",    2588,
            "J",    2588,
            "jh",    2589,
            "Jh",    2589,
            "NY",    2590,
            "T",    2591,
            "Th",    2592,
            "D",    2593,
            "Dh",    2594,
            "N",    2595,
            "t",    2596,
            "th",    2597,
            "d",    2598,
            "dh",    2599,
            "n",    2600,
            "p",    2602,
            "ph",    2603,
            //"f",    2347,
            "b",    2604,
            "B",    2604,
            "bh",    2605,
            "Bh",    2605,
            "m",    2606,
            "y",    2607,
            "r",    2608,
            //"R",    2353,
            "l",    2610,
            "L",    2611,
            //"zL",    2356,
            "v",    2613,
            "V",    2613,
            "w",    2613,
            "W",    2613,
            "sh",    2614,
            "Sh",    2614,
            "s",    2616,
            "h",    2617,
			"zk",	2649,
			"zkh",	2649,
			"zg",	2650,
			"zj",	2651,
			"zd",	2652,
			"zdh",	2654,
			//"zph",	2398,
			//"zy",	2399
        };

        const unsigned int uHalant = 0xA4D;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0A66,
            "z1",    0x0A67,
            "z2",    0x0A68,
            "z3",    0x0A69,
            "z4",    0x0A6A,
            "z5",    0x0A6B,
            "z6",    0x0A6C,
            "z7",    0x0A6D,
            "z8",    0x0A6E,
            "z9",    0x0A6F
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"zS",	2365,
			"|",	2404,
			"||",	2405
		};

    } // namespace Punjabi
    namespace Sanskrit
    {
        const VowelDef Vowels[] =
        {
            "a",    2309, 0,    // అ
            "aa",   2310, 2366, // ఆ
            "A",    2310, 2366, // ఆ
            "i",    2311, 2367, // ఇ
            "ee",   2312, 2368, // ఈ
            "I",    2312, 2368, // ఈ
            "u",    2313, 2369, // ఉ
            "oo",   2314, 2370, // ఊ
            "U",    2314, 2370, // ఊ
            "zr",   0x090B, 0x0943, // ఋ
            "zl",   0x090C, 0x0962, // ఌ
            "e",    2319, 2375, // ఎ
            "E",    2319, 2375, // ఏ
            "ai",   2320, 2376, // ఐ
            "o",    2323, 2379, // ఒ
            "O",    2323, 2379, // ఓ
            "au",   2324, 2380, // ఔ
            "zR",   0x0960, 0x0944, // ౠ
            "zL",   0x0961, 0x0963, // ౡ
            "AO",   0x0901, 0x0901, // ఁ
            "M",    2306, 2306, // ం
            "H",    2307, 2307, // ః
			"OM",	2384, 2384	// ॐ
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    2325,
            "K",    2325,
            "kh",    2326,
            "Kh",    2326,
            "g",    2327,
            "G",    2327,
            "gh",    2328,
            "Gh",    2328,
            "NGN",    2329,
            "ch",    2330,
            "Ch",    2331,
            "j",    2332,
            "J",    2332,
            "jh",    2333,
            "Jh",    2333,
            "NY",    2334,
            "T",    2335,
            "Th",    2336,
            "D",    2337,
            "Dh",    2338,
            "N",    2339,
            "t",    2340,
            "th",    2341,
            "d",    2342,
            "dh",    2343,
            "n",    2344,
            "zN",    2345,
            "p",    2346,
            "ph",    2347,
            "f",    2347,
            "b",    2348,
            "B",    2348,
            "bh",    2349,
            "Bh",    2349,
            "m",    2350,
            "y",    2351,
            "r",    2352,
            "R",    2353,
            "l",    2354,
            "L",    2355,
            "zL",    2356,
            "v",    2357,
            "V",    2357,
            "w",    2357,
            "W",    2357,
            "sh",    2358,
            "Sh",    2359,
            "s",    2360,
            "h",    2361,
			"zx",	0x093C
        };

        const unsigned int uHalant = 0x094D;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0966,
            "z1",    0x0967,
            "z2",    0x0968,
            "z3",    0x0969,
            "z4",    0x096A,
            "z5",    0x096B,
            "z6",    0x096C,
            "z7",    0x096D,
            "z8",    0x096E,
            "z9",    0x096F
        };


        const SpecialSymbolDef SpecialSymbols[] = 
		{
			"zS",	2365,
			"|",	2404,
			"||",	2405
		};

	}	// namespace Sanskrit

	namespace Tamil
	{
        const VowelDef Vowels[] =
        {
            "a",    0x0B85, 0,    
            "aa",   0x0B86, 0x0BBE, 
            "A",    0x0B86, 0x0BBE, 
            "i",    0x0B87, 0x0BBF, 
            "ee",   0x0B88, 0x0BC0, 
            "I",    0x0B88, 0x0BC0, 
            "u",    0x0B89, 0x0BC1, 
            "oo",   0x0B8A, 0x0BC2, 
            "U",    0x0B8A, 0x0BC2, 
            //"zr",   0x0B8B, 0x0BC3, 
            //"zl",   0x0B8C, 0x0BE2, 
            "e",    0x0B8E, 0x0BC6, 
            "E",    0X0B8F, 0X0BC7, 
            "ai",   0x0B90, 0x0BC8, 
            "o",    0x0B92, 0x0BCA, 
            "O",    0X0B93, 0X0BCB, 
            "au",   0x0B94, 0x0BCC, 
            //"zR",   0x0BE0, 0x0BC4, 
            //"zL",   0x0BE1, 0x0BE3, 
            //"AO",   0x0B81, 0x0B81, 
            "M",    0x0B82, 0x0B82, 
            "H",    0x0B83, 0x0B83, 
            "OM",    0x0BD0, 0x0BD0, 
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    0x0B95,
            "K",    0x0B95,
            "kh",   0x0B95, // 0x0B96,
            "Kh",   0x0B95, //  0x0B96,
            "g",    0x0B95, // 0x0B97,
            "G",    0x0B95, // 0x0B97,
            "gh",   0x0B95, //  0x0B98,
            "Gh",   0x0B95, //  0x0B98,
            "NGN",  0x0B99,
            "ch",   0x0B9A,
            "Ch",   0x0B9A, // 0X0B9B,
            "j",    0x0B9C,
            "J",    0x0B9C,
            "jh",   0x0B9C, // 0x0B9D,
            "Jh",   0x0B9C, //  0x0B9D,
            "NY",   0x0B9E,
            "T",    0x0B9F,
            "Th",   0x0B9F, // 0x0BA0,
            "D",    0x0B9F, // 0x0BA1,
            "Dh",   0x0B9F, //  0x0BA2,
            "N",    0x0BA3,
            "t",    0x0BA4, // 0x0BA4,
            "th",   0x0BA4, //  0x0BA5,
            "d",    0x0BA4, // 0x0BA6,
            "dh",   0x0BA4, //  0x0BA7,
            "n",    0x0BA8,
            "zN",   0x0BA9,
            "p",    0x0BAA,
            "ph",   0x0BAA, // 0x0BAB,
            "f",    0x0BAA, // 0x0BAB,
            "b",    0x0BAA, // 0x0BAC,
            "B",    0x0BAA, // 0x0BAC,
            "bh",   0x0BAA, //  0x0BAD,
            "Bh",   0x0BAA, //  0x0BAD,
            "m",    0x0BAE,
            "y",    0x0BAF,
            "r",    0x0BB0,
            "R",    0x0BB1,
            "l",    0x0BB2,
            "L",    0x0BB3,
            "zL",   0x0BB4,
            "v",    0x0BB5,
            "V",    0x0BB5,
            "w",    0x0BB5,
            "W",    0x0BB5,
            "sh",   0x0BB6,
            "Sh",   0x0BB7,
            "s",	0x0BB8,
            "h",    0x0BB9,
        };

        const unsigned int uHalant = 0x0BCD;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = // Currently no special Digits. Replace with any actual content if needed.
        {
            "z0",    0x0BE6,
            "z1",    0x0BE7,
            "z2",    0x0BE8,
            "z3",    0x0BE9,
            "z4",    0x0BEA,
            "z5",    0x0BEB,
            "z6",    0x0BEC,
            "z7",    0x0BED,
            "z8",    0x0BEE,
            "z9",    0x0BEF,
            "z10",   0x0BF0,
            "z100",  0x0BF1,
            "Z1000", 0x0BF2
        };

        const SpecialSymbolDef SpecialSymbols[] = 
		{
			"|",	2404,
			"||",	2405,
            "zRs",  0x0BF9        // Rupee sign
		};

	}	// namespace Tamil

    namespace Telugu
    {
        const VowelDef Vowels[] =
        {
            "a",    3077, 0,    // అ
            "aa",   3078, 3134, // ఆ
            "A",    3078, 3134, // ఆ
            "i",    3079, 3135, // ఇ
            "ee",   3080, 3136, // ఈ
            "I",    3080, 3136, // ఈ
            "u",    3081, 3137, // ఉ
            "oo",   3082, 3138, // ఊ
            "U",    3082, 3138, // ఊ
            "zr",   3083, 3139, // ఋ
            "zl",   3084, 3170, // ఌ
            "e",    3086, 3142, // ఎ
            "E",    3087, 3143, // ఏ
            "ai",   3088, 3144, // ఐ
            "o",    3090, 3146, // ఒ
            "O",    3091, 3147, // ఓ
            "au",   3092, 3148, // ఔ
            "zR",   3168, 3140, // ౠ
            "zL",   3169, 3171, // ౡ
            "AO",   3073, 3073, // ఁ
            "M",    3074, 3074, // ం
            "H",    3075, 3075, // ః
        };

        const ConsonantDef Consonants[] = 
        {
            "k",    3093,
            "K",    3093,
            "kh",    3094,
            "Kh",    3094,
            "g",    3095,
            "G",    3095,
            "gh",    3096,
            "Gh",    3096,
            "NGN",    3097,
            "ch",    3098,
            "Ch",    3099,
            "j",    3100,
            "J",    3100,
            "jh",    3101,
            "Jh",    3101,
            "NY",    3102,
            "T",    3103,
            "Th",    3104,
            "D",    3105,
            "Dh",    3106,
            "N",    3107,
            "t",    3108,
            "th",    3109,
            "d",    3110,
            "dh",    3111,
            "n",    3112,
            "p",    3114,
            "ph",    3115,
            "f",    3115,
            "b",    3116,
            "B",    3116,
            "bh",    3117,
            "Bh",    3117,
            "m",    3118,
            "y",    3119,
            "r",    3120,
            "R",    3121,
            "l",    3122,
            "L",    3123,
            "v",    3125,
            "V",    3125,
            "w",    3125,
            "W",    3125,
            "sh",    3126,
            "Sh",    3127,
            "s",    3128,
            "h",    3129
        };

        const unsigned int uHalant = 0x0C4D;   // The Unicode character code for Virama (Halant)

        const DigitDef Digits[] = 
        {
            "z0",    0x0C66,
            "z1",    0x0C67,
            "z2",    0x0C68,
            "z3",    0x0C69,
            "z4",    0x0C6A,
            "z5",    0x0C6B,
            "z6",    0x0C6C,
            "z7",    0x0C6D,
            "z8",    0x0C6E,
            "z9",    0x0C6F
        };

        const SpecialSymbolDef SpecialSymbols[] = // Currently no special Symbols, this is just dummy entry. Replace it with any actual content if needed.
		{ 
			"z",	'?'
		};

    } // namespace Telugu

} // namespace PhTranslation

#endif // _LANGUAGECODES_H__195ADDC0_CF4A_44c0_AC65_8247CD83E6AA__
