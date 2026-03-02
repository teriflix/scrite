#include "stdafx.h"
#include "PhTranslator.h"

#include <string.h>
#include <algorithm>

namespace PhTranslation
{

#define ph_iswascii(_c)    ( unsigned(_c) < 0x80 )

    PhTranslator::PhTranslator(void)
    {
    }

    PhTranslator::~PhTranslator(void)
    {
    }

    // Return if the first element is greater than the second
    template<typename T>
    bool phRepComparator(const T& input1, const T& input2)
    {
        return strlen(input1.phRep) > strlen(input2.phRep);
    }

    template<typename T>
    void LoadVector(std::vector<T> vec[], const T* inputArr, int nInputSize)
    {
        // For each entry in the definition
        for(int i=0; i < nInputSize; ++i)
        {
            // Find the First character of its phonetic representation
            const T& defObj = inputArr[i];
            const char chIndex = defObj.phRep[0];
            // Store the definition, indexed at its first character 
            vec[int(chIndex)].push_back(defObj);
        }

        // Sort each vector such that longer strings come first
        for(int i=0, nMax = PhTranslator::VecLength; i < nMax; ++i)
        {
            std::sort(vec[i].begin(), vec[i].end(), phRepComparator<T>);
        }
    }

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
    PhTranslator::PhTranslator(const VowelDef* pVowels, int nVSize,
                const ConsonantDef* pConsonants, int nCSize,
                const DigitDef* pDigits, int nDSize,
                const SpecialSymbolDef* pSpSymbols, int nSPSize,
                const tUnicode Halant /*= 0*/)                
    {
        // Load the input Arrays into internal structures
        if(pVowels != nullptr &&  nVSize != 0)
            LoadVector(this->m_Vowels, pVowels, nVSize);

        if(pConsonants != nullptr &&  nCSize != 0)
            LoadVector(this->m_Consonants, pConsonants, nCSize);

        if(pDigits != nullptr &&  nDSize != 0)
            LoadVector(this->m_Digits, pDigits, nDSize);

        if(pSpSymbols != nullptr &&  nSPSize != 0)
            LoadVector(this->m_SpecialSymbols, pSpSymbols, nSPSize);

        m_Halant = Halant;
    }

    // Checks if the given prefix string is complete present in the string.
    // Return value would be same as the length of the prefix string, if successful.
    // Return value would be zero, in case the prefix string is not present completely.
    inline unsigned int IsPrefixMatching(const char* sz, const char* pfx)
    {
        unsigned int nMatched = 0;
        
        while(*sz && *pfx && *sz == *pfx) 
        {
            sz++;
            pfx++;
            nMatched++;
        }
        
        return (*pfx == 0) ? nMatched : 0;
    }

    // Searches the vectors to find the best prefix that matches the sequence of
    // characters pointed by sz.
    template<typename T>
    inline unsigned int ExtractMatchingObject(const std::vector<T> vec[], const char* sz, const T* &retVal)
    {
        const char chIndex =  sz[0];

        const std::vector<T>& vecObjects = vec[int(chIndex)];

        unsigned int nMatched = 0;

        for(size_t i=0, nMax = vecObjects.size(); i <  nMax; ++i)
        {
            retVal = &vecObjects[i];

            if((nMatched = IsPrefixMatching(sz, retVal->phRep)) > 0)
                return nMatched;
        }

        retVal = nullptr;

        return 0;
    }

    unsigned int PhTranslator::ExtractMatchingVowel(const char* sz, const VowelDef* &retVal) const
    {
        return ExtractMatchingObject(this->m_Vowels, sz, retVal);
    }

    unsigned int PhTranslator::ExtractMatchingConsonant(const char* sz, const ConsonantDef* &retVal) const
    {
        return ExtractMatchingObject(this->m_Consonants, sz, retVal);
    }

    unsigned int PhTranslator::ExtractMatchingDigit(const char* sz, const DigitDef* &retVal) const
    {
        return ExtractMatchingObject(this->m_Digits, sz, retVal);
    }

    unsigned int PhTranslator::ExtractMatchingSpecialSymbol(const char* sz, const SpecialSymbolDef* &retVal) const
    {
        return ExtractMatchingObject(this->m_SpecialSymbols, sz, retVal);
    }

    inline bool IsASCIIAlphabet(const char ch)
    {
        return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
    }

    inline void AppendUCODE(std::wstring& Str, tUnicode uCode)
    {
        if(uCode != 0)
            Str += uCode;
    }

    size_t PhTranslator::Translate(const char* sz, std::wstring& retStr) const
    {
        if(sz == nullptr || *sz == 0) return 0;

        const char* psz = sz;

        const VowelDef* pVowel = nullptr;
        const ConsonantDef* pConsonant = nullptr;
        const DigitDef* pDigit = nullptr;
        const SpecialSymbolDef* pSpecialSymbol = nullptr;

        bool bFollowingConsonant = false;

        size_t nMatched = 0, nRetStrInitialLength = retStr.length();

        do
        {
            // Try Vowels
            {   
                nMatched = ExtractMatchingVowel(psz, pVowel);
                if(nMatched > 0)
                {
                    // if this vowel is following a consontant then use it as a dependant character
                    // otherwise output as an independent vowel
                    AppendUCODE(retStr, (bFollowingConsonant ? pVowel->dCode : pVowel->uCode));
                    psz += nMatched;
                    bFollowingConsonant = false;

                    continue;
                }
            }

            // If this character is classified as Vowel, but reached here
            // then it is a false positive. In such case, we might have missed
            // inserting the Halant for the preceding consonant (thinking that this would be a vowel).
            // Now that it is confirmed as not a vowel, lets insert it now.
            if(bFollowingConsonant && IsVowel(*psz))
                AppendUCODE(retStr, this->m_Halant);

            // Try Consonants
            {   
                nMatched = ExtractMatchingConsonant(psz, pConsonant);
                if(nMatched > 0)
                {
                    AppendUCODE(retStr, pConsonant->uCode);
                    psz += nMatched;
                    bFollowingConsonant = true;

                    // if the next character is not vowel, insert the Virama/Halant
                    if(*psz != 0 && IsVowel(*psz) == false)
                        AppendUCODE(retStr, this->m_Halant);

                    if(*psz == 0)
                        AppendUCODE(retStr, this->m_Halant);

                    continue;
                }
            }

            // Try Digits
            {   
                nMatched = ExtractMatchingDigit(psz, pDigit);
                if(nMatched > 0)
                {
                    AppendUCODE(retStr, pDigit->uCode);
                    psz += nMatched;
                    bFollowingConsonant = false;

                    continue;
                }
            }

            // Try Special Symbols
            {   
                nMatched = ExtractMatchingSpecialSymbol(psz, pSpecialSymbol);
                if(nMatched > 0)
                {
                    AppendUCODE(retStr, pSpecialSymbol->uCode);
                    psz += nMatched;
                    bFollowingConsonant = false;

                    continue;
                }
            }

            // This character, what ever it is, did not match anything. 
            // Insert it as is.
            {
                retStr += *psz++;
                bFollowingConsonant = false;
            }

        }while(*psz != 0);

        return retStr.length() - nRetStrInitialLength; // return the length of the newly generated portion
    }


	// Extracts the congiguous ASCII portion encountered at the beginnging of the given input string
	int ExtractASCIICodes(const wchar_t* pSz, std::string& retStr)
	{
		retStr = "";
		int nCount =0;
        while(*pSz != 0 && ph_iswascii(*pSz))
		{
            retStr += char(*pSz++);
			++nCount;
		}
		return nCount;
	}

	// Extracts the congiguous Non-ASCII portion encountered at the beginnging of the given input string
	int ExtractUNICODECodes(const wchar_t* pSz, std::wstring& retStr)
	{
		retStr = L"";
		int nCount =0;
        while(*pSz != 0 && ph_iswascii(*pSz) == false)
		{
			retStr += *pSz++;
			++nCount;
		}
		return nCount;
	}

	size_t PhTranslator::Translate(const wchar_t* sz, std::wstring& retStr) const
	{		
        if(sz == nullptr) return 0;

		size_t nRetStrInitialLength = retStr.length(); // Store the initial length of the retStr

        const wchar_t* psz = sz;

		std::string strAscii;
		std::wstring strNonAscii;

		do
		{
			// Extract the Ascii codes and translate them
			psz += ExtractASCIICodes(psz, strAscii);	
			Translate(strAscii.c_str(), retStr);

			// Extract the Non-Ascii codes and insert them into output as is
			psz += ExtractUNICODECodes(psz, strNonAscii); 
			retStr += strNonAscii;

        }while(*psz != 0);

		return retStr.length() - nRetStrInitialLength; // return the length of the newly generated portion
	}


	template<typename T>
    inline int SaveToFile(FILE* fp, const std::vector<T> vec[])
    {
		int nLineCount = 0;
        for(int i=0, iMax = PhTranslator::VecLength; i < iMax; ++i)
        {
            for(size_t j=0, jMax = vec[i].size(); j < jMax; ++j, ++nLineCount)
                fprintf(fp, "\n%-8s %-8u", vec[i][j].phRep, vec[i][j].uCode);
        }
		return nLineCount;
    }

	bool PhTranslator::SavePhoneticTable(const char* szFilePath) const
	{
		FILE* fp = fopen(szFilePath, "w");
        if(fp != nullptr)
		{
			int nVowelCount=0, nConsonantCount = 0, nDigitCount =0, nSpecialSymbolCount=0;

			// Insert dummy header. We will update it later once we have the correct values
			fprintf(fp, "PhTranslation %-8u %-8u %-8u %-8u %-8u", nVowelCount, nConsonantCount, nDigitCount, nSpecialSymbolCount, this->m_Halant);

			for(int i=0, iMax = PhTranslator::VecLength; i < iMax; ++i)
			{
				for(size_t j=0, jMax = this->m_Vowels[i].size(); j < jMax; ++j, ++nVowelCount)
					fprintf(fp, "\n%-8s %-8u %-8u", this->m_Vowels[i][j].phRep, this->m_Vowels[i][j].uCode, this->m_Vowels[i][j].dCode);
			}

			nConsonantCount = SaveToFile(fp, this->m_Consonants);

			nDigitCount = SaveToFile(fp, this->m_Digits);

			nSpecialSymbolCount = SaveToFile(fp, this->m_SpecialSymbols);

			// Go back to the header and update it with correct values
			fseek(fp, 0, 0);
			fprintf(fp, "PhTranslation %-8u %-8u %-8u %-8u %-8u", nVowelCount, nConsonantCount, nDigitCount, nSpecialSymbolCount, this->m_Halant);

			fclose(fp);

			return true;
		}
		return false;
	}

    bool PhTranslator::LoadPhoneticTable(const char* /*szFilePath*/)
	{
#if 0 // We dont need loading of language codes from external files as yet.
		FILE* fp = fopen(szFilePath, "r");
        if(fp != nullptr)
		{
			char szHeader[16];
			int nVowelCount=0, nConsonantCount = 0, nDigitCount =0, nSpecialSymbolCount=0, nHalant=0;

			fscanf(fp, "%13s %u %u %u %u %u", szHeader, &nVowelCount, &nConsonantCount, &nDigitCount, &nSpecialSymbolCount, &nHalant);
			
			if(strcmp(szHeader, "PhTranslation")) return false;

            VowelDef* pVowels = nullptr; ConsonantDef* pConsonants = nullptr; DigitDef* pDigits = nullptr; SpecialSymbolDef* pSpecialSymbols = nullptr;

			if(nVowelCount)			pVowels = new VowelDef[nVowelCount];
			if(nConsonantCount)		pConsonants = new ConsonantDef[nConsonantCount];
			if(nDigitCount)			pDigits = new DigitDef[nDigitCount];
			if(nSpecialSymbolCount)	pSpecialSymbols = new SpecialSymbolDef[nSpecialSymbolCount];

			{
				for(int i=0; i < nVowelCount; ++i)
					fscanf(fp, "%8s %hu %hu", pVowels[i].phRep, &pVowels[i].uCode, &pVowels[i].dCode);

				for(int i=0; i < nConsonantCount; ++i)
					fscanf(fp, "%8s %hu", pConsonants[i].phRep, &pConsonants[i].uCode);

				for(int i=0; i < nDigitCount; ++i)
					fscanf(fp, "%8s %hu", pDigits[i].phRep, &pDigits[i].uCode);

				for(int i=0; i < nSpecialSymbolCount; ++i)
					fscanf(fp, "%8s %hu", pSpecialSymbols[i].phRep, &pSpecialSymbols[i].uCode);

				fclose(fp);
			}

			// Load the read Arrays into internal structures
			{
                if(pVowels != nullptr &&  nVowelCount != 0)
					LoadVector(this->m_Vowels, pVowels, nVowelCount);

                if(pConsonants != nullptr &&  nConsonantCount != 0)
					LoadVector(this->m_Consonants, pConsonants, nConsonantCount);

                if(pDigits != nullptr &&  nDigitCount != 0)
					LoadVector(this->m_Digits, pDigits, nDigitCount);

                if(pSpecialSymbols != nullptr &&  nSpecialSymbolCount != 0)
					LoadVector(this->m_SpecialSymbols, pSpecialSymbols, nSpecialSymbolCount);

				m_Halant = nHalant;
			}

			delete pVowels; 
			delete pConsonants; 
			delete pDigits; 
			delete pSpecialSymbols;

			return true;
		}
#endif // We dont need loading of language codes from external files as yet.
		return false;
	}

} // namespace PhTranslation


