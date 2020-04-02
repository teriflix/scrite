#ifndef __PHTRANSLATOR___9EA8D480_6CC6_4b31_9C41_C8E2DE16EBBF__
#define __PHTRANSLATOR___9EA8D480_6CC6_4b31_9C41_C8E2DE16EBBF__

#include <vector>
#include <string>

namespace PhTranslation
{
    typedef wchar_t tUnicode;

    struct VowelDef
    {
        char phRep[8];   // The English Phonetic Representation of the Vowel
        tUnicode uCode; // The Unicode character of the Vowel when occuring Independently
        tUnicode dCode; // The Unicode character code of the Vowel when Dependant on preceding Consonant
    };

    struct ConsonantDef
    {
        char phRep[8]; // The English Phonetic Representation of the Consonant
        tUnicode uCode; // The Unicode character code of the Consonant
    };

    struct DigitDef
    {
        char phRep[8]; // The English Phonetic Represenation of the Digit
        tUnicode uCode; // The Unicode character code of the Digit
    };

    struct SpecialSymbolDef
    {
        char phRep[8]; // The English Representation of the Special Symbol
        tUnicode uCode; // The Unicode character code of the Special Symbol
    };


    class PhTranslator
    {
    public:
        enum {VecLength = 256};
    protected:
        typedef std::vector<VowelDef>           VecVowels;
        typedef std::vector<ConsonantDef>       VecConsonants;
        typedef std::vector<DigitDef>           VecDigits;
        typedef std::vector<SpecialSymbolDef>   VecSpecialSymbols;

        VecVowels               m_Vowels[VecLength];       // Indexed by English Alphabet [a-z] [A-Z]
        VecConsonants           m_Consonants[VecLength];   // Indexed by English Alphabet [a-z] [A-Z]
        VecDigits               m_Digits[VecLength];       // Indexed by English Digits [0-9]
        VecSpecialSymbols       m_SpecialSymbols[VecLength]; // Indexed by ASCII symbols
        tUnicode                m_Halant;

        // Returns if the given character is defined to a Vowel identifier
        inline bool IsVowel(char ch) const
        {
            return m_Vowels[ch].size() > 0;
        }

        unsigned int ExtractMatchingVowel(const char* sz, const VowelDef* &pRetVal) const;
        unsigned int ExtractMatchingConsonant(const char* sz, const ConsonantDef* &pRetVal) const;
        unsigned int ExtractMatchingDigit(const char* sz, const DigitDef* &pRetVal) const;
        unsigned int ExtractMatchingSpecialSymbol(const char* sz, const SpecialSymbolDef* &pRetVal) const;

    public:
        PhTranslator(void);

        ~PhTranslator(void);

        // Constructor
        // Inputs:
        //     pVowels: The Vowels Array
        //     nVSize: length of the pVowels Array (no. of elements)
        //     pConsonants: The Consonants Array
        //     nCSize: length of the pConsonants Array (no. of elements)
        //     pDigits: The Digits Array
        //     nVSize: length of the pVowels Array (no. of elements)
        //     pSpSymbols: The Special Symbols Array
        //     nSPSize: length of the pSpSymbols Array (no. of elements)
        //     Halant: The Unicode code that is to be used as 'Virama'/Halant. Supply 0 if none exists.
        PhTranslator(const VowelDef* pVowels, int nVSize,
                    const ConsonantDef* pConsonants, int nCSize,
                    const DigitDef* pDigits, int nDSize,
                    const SpecialSymbolDef* pSpSymbols, int nSPSize,
                    const tUnicode Halant);

		// Loads the PhoneticTable from the specified file. This data will be used in the translations later on.
		// In case of load failures, the state of the tables is undefined and the later translations may not yield correct results.
		// Inputs:
		//    szFilePath: Path of the file that contains the PhoneticTable to be loaded.
		// Return value indicates the success or failure.
		bool LoadPhoneticTable(const char* szFilePath);

		// Saves the PhoneticTable to the specified file. This file can used to create Custom Translators later on.
		// Inputs:
		//    szFilePath: Path of the file the PhoneticTable should be saved to.
		// Return value indicates the success or failure.
		bool SavePhoneticTable(const char* strFilePath) const;

        // Translates the given English string Phonetically
        // Inputs:
        //      sz: The String in Phonetic English
        // Outputs:
        //      retStr: The Unicode representation. 
		//      If retStr is not empty on entry, translted string will be appended to it at the end automatically.
		//		If there are any untranslatable chracters in the input, they will be output as is.
		// Return value indicates the length of the new Unicode string generated as the result of translation.
		//		If retStr is empty on entry, return value would be same as the length of retStr upon return.
		//		If retStr is non-empty on entry, return value just indicates the length of the portion newly added, not the total string.
        size_t Translate(const char* sz, std::wstring& retStr) const;

        // Translates the given English string Phonetically.
		// If the input contains any Unicode characters already, they will be inserted into the output string as is.
        // Inputs:
        //      sz: The String in Phonetic English
        // Outputs:
        //      retStr: The Unicode representation
		//      If retStr is not empty on entry, translted string will be appended to it at the end automatically.
		//		If there are any untranslatable chracters in the input, they will be output as is.
		// Return value indicates the length of the new Unicode string generated as the result of translation.
		//		If retStr is empty on entry, return value would be same as the length of retStr upon return.
		//		If retStr is non-empty on entry, return value just indicates the length of the portion newly added, not the total string.
        size_t Translate(const wchar_t* sz, std::wstring& retStr) const;

    };

} // namespace PhTranslation

#endif // __PHTRANSLATOR___9EA8D480_6CC6_4b31_9C41_C8E2DE16EBBF__