// PhTranslateLib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "PhTranslateLib.h"

#include "LanguageCodes.h"
using namespace PhTranslation;

#ifndef __countof
#define __countof(x)    (sizeof(x) /  sizeof(x[0]))
#endif 

template<typename T>
inline size_t TranslateT(void* Translator, const T szInput, std::wstring& retStr)
{
	PhTranslator* pTranslator = (PhTranslator*) Translator;
	if(pTranslator != NULL && szInput != NULL)
	{
		// Do the Translation using the Translator
		return pTranslator->Translate(szInput, retStr);
	}
	return 0;
}

// Translates the given Phonetic English string. If the input string already contains Unicode characters they will be
// inserted into output as is.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the CreateXYZTranslator() methods
//  [in]  szInput: The Phonetic English String that is to be translated
//  [out] szOutput: The Translated String in Unicode representation
//  [in]  nLen: Maximum number of wide characters to be filled in szOutput
template<typename T>
inline size_t TranslateT(void* Translator, const T szInput, 
                                  wchar_t* szOutput, const int nLen)
{
    std::wstring retStr;
    TranslateT(Translator, szInput, retStr);
    if(szOutput != NULL && nLen > 0)
    {
        // Copy the generated Unicode string to the output buffer
        wcsncpy(szOutput, retStr.c_str(), nLen-1);

        szOutput[nLen-1] = L'\0';
    }

    return retStr.length();
}

// Translates the given string and returns the required buffer size to be allocated to hold the output.
// The actual translated buffer can be later retrieved by supplying the pHint to the GetTranslatedBuffer() method.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the CreateXYZTranslator() methods
//  [in]  szInput: The Phonetic English String that is to be translated
//  [Out] ppHint: Returns a Hint object pointer that can be used to retrieve the translated buffer later
template<typename T>
inline size_t GetTranslatedBufferLengthT(void* Translator, const T szInput, void** ppHint)
{
	std::wstring* pRetStr = new std::wstring();	// Allocate a string on heap. Will be released in GetTranslatedBuffer() later.

	//TODO: How about internally maintaining a AutoPtr map in case user forgets to call GetTranslatedBuffer to release the memory

	TranslateT(Translator, szInput, *pRetStr);

	*ppHint = pRetStr;	// Set the translated string as the Hint object.

	return pRetStr->length() + 1;	// Add 1 to give space for '\0'
}

extern "C"
{
    // Creates a Telugu Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetTeluguTranslator()
    {            
        static PhTranslator Translator (Telugu::Vowels, __countof(Telugu::Vowels),
                        Telugu::Consonants, __countof(Telugu::Consonants),
                        Telugu::Digits, __countof(Telugu::Digits),
						Telugu::SpecialSymbols, __countof(Telugu::SpecialSymbols),
                        Telugu::uHalant);

        return &Translator;
    }

    // Creates a Bengali Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetBengaliTranslator()
	{
        static PhTranslator Translator (Bengali::Vowels, __countof(Bengali::Vowels),
                        Bengali::Consonants, __countof(Bengali::Consonants),
                        Bengali::Digits, __countof(Bengali::Digits),
						Bengali::SpecialSymbols, __countof(Bengali::SpecialSymbols),
                        Bengali::uHalant);

        return &Translator;		
	}

    // Creates a Gujarati Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetGujaratiTranslator()
	{
        static PhTranslator Translator (Gujarati::Vowels, __countof(Gujarati::Vowels),
                        Gujarati::Consonants, __countof(Gujarati::Consonants),
                        Gujarati::Digits, __countof(Gujarati::Digits),
						Gujarati::SpecialSymbols, __countof(Gujarati::SpecialSymbols),
                        Gujarati::uHalant);

        return &Translator;		
	}

    // Creates a Hindi Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetHindiTranslator()
	{
        static PhTranslator Translator (Hindi::Vowels, __countof(Hindi::Vowels),
                        Hindi::Consonants, __countof(Hindi::Consonants),
                        Hindi::Digits, __countof(Hindi::Digits),
						Hindi::SpecialSymbols, __countof(Hindi::SpecialSymbols),
                        Hindi::uHalant);

        return &Translator;		
	}

    // Creates a Kannada Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetKannadaTranslator()
	{
        static PhTranslator Translator (Kannada::Vowels, __countof(Kannada::Vowels),
                        Kannada::Consonants, __countof(Kannada::Consonants),
                        Kannada::Digits, __countof(Kannada::Digits),
						Kannada::SpecialSymbols, __countof(Kannada::SpecialSymbols),
                        Kannada::uHalant);

        return &Translator;		
	}

    // Creates a Malayalam Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetMalayalamTranslator()
	{
        static PhTranslator Translator (Malayalam::Vowels, __countof(Malayalam::Vowels),
                        Malayalam::Consonants, __countof(Malayalam::Consonants),
                        Malayalam::Digits, __countof(Malayalam::Digits),
						Malayalam::SpecialSymbols, __countof(Malayalam::SpecialSymbols),
                        Malayalam::uHalant);

        return &Translator;		
	}

    // Creates a Punjabi Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetPunjabiTranslator()
	{
        static PhTranslator Translator (Punjabi::Vowels, __countof(Punjabi::Vowels),
                        Punjabi::Consonants, __countof(Punjabi::Consonants),
                        Punjabi::Digits, __countof(Punjabi::Digits),
						Punjabi::SpecialSymbols, __countof(Punjabi::SpecialSymbols),
                        Punjabi::uHalant);

        return &Translator;		
	}

    // Creates a Oriya Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetOriyaTranslator()
	{
        static PhTranslator Translator (Oriya::Vowels, __countof(Oriya::Vowels),
                        Oriya::Consonants, __countof(Oriya::Consonants),
                        Oriya::Digits, __countof(Oriya::Digits),
						Oriya::SpecialSymbols, __countof(Oriya::SpecialSymbols),
                        Oriya::uHalant);

        return &Translator;		
	}
	
	// Creates a Sanskrit Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetSanskritTranslator()
	{
        static PhTranslator Translator (Sanskrit::Vowels, __countof(Sanskrit::Vowels),
                        Sanskrit::Consonants, __countof(Sanskrit::Consonants),
                        Sanskrit::Digits, __countof(Sanskrit::Digits),
						Sanskrit::SpecialSymbols, __countof(Sanskrit::SpecialSymbols),
                        Sanskrit::uHalant);

        return &Translator;		
	}
	
	// Creates a Tamil Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetTamilTranslator()
	{
        static PhTranslator Translator (Tamil::Vowels, __countof(Tamil::Vowels),
                        Tamil::Consonants, __countof(Tamil::Consonants),
                        Tamil::Digits, __countof(Tamil::Digits),
						Tamil::SpecialSymbols, __countof(Tamil::SpecialSymbols),
                        Tamil::uHalant);

        return &Translator;		
	}

	// Creates a Translator based on the PhoneticTables loaded from the specified file.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* CreateCustomTranslator(const char* szPhoneticTableFilePath)
	{
		PhTranslator* pTranslator = new PhTranslator();
		if(pTranslator->LoadPhoneticTable(szPhoneticTableFilePath) == false)
		{
			delete pTranslator;
			return NULL;
		}
		return pTranslator;
	}

    // Releases a Translator previously created with CreateCustomTranslator method
    PHTRANSLATELIB_API void ReleaseCustomTranslator(void* Translator)
    {
        PhTranslator* pTranslator = (PhTranslator*) Translator;
        if(pTranslator)
            delete pTranslator;
    }

    PHTRANSLATELIB_API size_t Translate(void* Translator, const char* szInput, 
                                      wchar_t* szOutput, const int nLen)
    {
		return TranslateT(Translator, szInput, szOutput, nLen);
    }

    PHTRANSLATELIB_API size_t TranslateW(void* Translator, const wchar_t* szInput, 
                                      wchar_t* szOutput, const int nLen)
    {
		return TranslateT(Translator, szInput, szOutput, nLen);
    }

    PHTRANSLATELIB_API size_t GetTranslatedBufferLength(void* Translator, const char* szInput, void** ppHint)
	{
		return GetTranslatedBufferLengthT(Translator, szInput, ppHint);
	}

    PHTRANSLATELIB_API size_t GetTranslatedBufferLengthW(void* Translator, const wchar_t* szInput, void** ppHint)
	{
		return GetTranslatedBufferLengthT(Translator, szInput, ppHint);
	}

    // Retrieves the translatedand buffer previously computed with GetTranslatedBufferLength() method. 
	// Upon success, the Hint object will be destroyed and reset to NULL, so that it will not be used in any further calls.
	// Parameters:
    //  [out] szOutput: The buffer to hold the Translated String in Unicode representation
	//  [in/Out] ppHint: The Hint object pointer that was returned from GetTranslatedBufferLength(). The Object will be reset to NULL.
    PHTRANSLATELIB_API void GetTranslatedBuffer(wchar_t* szOutput, void** pHint)
	{
		if(szOutput != NULL && pHint != NULL)
		{
			std::wstring* pStr = (std::wstring*) *pHint;	
			wcsncpy(szOutput, pStr->c_str(), pStr->length()+1);
			delete pStr; // pStr must have been allocated previously in GetTranslatedBufferLength(). Lets release it now. We don't need it.
			*pHint = NULL;
		}
	}

    PHTRANSLATELIB_API bool SavePhoneticTable(void* Translator, const char* szFilePath)
	{
		PhTranslator* pTranslator = (PhTranslator*) Translator;
		if(pTranslator != NULL && szFilePath != NULL)
		{
			return pTranslator->SavePhoneticTable(szFilePath);
		}
		return false;
	}

}   // extern "C"



PHTRANSLATELIB_API size_t Translate(void* Translator, const char* szInput, std::wstring& retStr)
{
	return TranslateT(Translator, szInput, retStr);
}

PHTRANSLATELIB_API size_t Translate(void* Translator, const wchar_t* szInput, std::wstring& retStr)
{
	return TranslateT(Translator, szInput, retStr);
}

PHTRANSLATELIB_API std::wstring Translate(void* Translator, const char* szInput)
{
	std::wstring retStr;
	TranslateT(Translator, szInput, retStr);
	return retStr;
}

PHTRANSLATELIB_API std::wstring Translate(void* Translator, const wchar_t* szInput)
{
	std::wstring retStr;
	TranslateT(Translator, szInput, retStr);
	return retStr;
}