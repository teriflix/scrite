#ifndef PH_TRANSLATE_LIB_H
#define PH_TRANSLATE_LIB_H

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the PHTRANSLATELIB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// PHTRANSLATELIB_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifndef PHTRANSLATE_STATICLIB
#ifdef PHTRANSLATELIB_EXPORTS
#define PHTRANSLATELIB_API __declspec(dllexport)
#pragma message("----------Defining PHTRANSLATELIB_API to be dllexport---------")
#else
#define PHTRANSLATELIB_API __declspec(dllimport)
#pragma message("----------Defining PHTRANSLATELIB_API to be dllimport---------")
#endif
#else // Compile PhTranslateLib as Static lib
#define PHTRANSLATELIB_API
#endif

#include <string>

extern "C"
{
    // Get the Telugu Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetTeluguTranslator();

    // Get the Bengali Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetBengaliTranslator();

    // Get the Gujarati Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetGujaratiTranslator();

    // Get the Hindi Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetHindiTranslator();

    // Get the Marathi Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetMarathiTranslator();

    // Get the Kannada Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetKannadaTranslator();

    // Get the Malayalam Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetMalayalamTranslator();

    // Get the Punjabi Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetPunjabiTranslator();

    // Get the Oriya Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetOriyaTranslator();

    // Get the Sanskrit Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetSanskritTranslator();

    // Get the Telugu Translator.
    // The output of this method must be sent as input to the Translate Method.
    PHTRANSLATELIB_API void* GetTamilTranslator();

    // Creates a Translator based on the PhoneticTables loaded from the specified file.
    // The output of this method must be sent as input to the Translate Method.
	// Use ReleaseCustomTranslator method to release the created translator.
    PHTRANSLATELIB_API void* CreateCustomTranslator(const char* szPhoneticTableFilePath);

    // Releases a Translator previously created with the CreateCustomTranslator() method
    PHTRANSLATELIB_API void ReleaseCustomTranslator(void* Translator);

    // Translates the given Phonetic English string.
    // Parameters:
    //  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
    //  [in]  szInput: The Phonetic English String that is to be translated
    //  [out] szOutput: The Translated String in Unicode representation
    //  [in]  nLen: Max no.of wide chars to be filled. szOutput[nLen-1] will be '\0' if the buffer is small.
    //  [return] Returns the length of the full converted string. szOutput might be holding only a fraction of it, if nLen is small.
    //  Remarks: Send szInput as NULL and to get the required length of the buffer.
    PHTRANSLATELIB_API size_t Translate(void* Translator, const char* szInput, 
                                      wchar_t* szOutput, const int nLen);

    // Translates the given Phonetic English string.If the string contains non-Ascii characters they will be 
	// inserted into the output string as is.
    // Parameters:
    //  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
    //  [in]  szInput: The Phonetic English String that is to be translated
    //  [out] szOutput: The Translated String in Unicode representation
    //  [in]  nLen: Max no.of wide chars to be filled. szOutput[nLen-1] will be '\0' if the buffer is small.
    //  [return] Returns the length of the full converted string. szOutput might be holding only a fraction of it, if nLen is small.
    //  Remarks: Send szInput as NULL and to get the required length of the buffer.
    PHTRANSLATELIB_API size_t TranslateW(void* Translator, const wchar_t* szInput, 
                                      wchar_t* szOutput, const int nLen);

    // Translates the given string and returns the required buffer size to be allocated to hold the output.
	// You can directly use the return value to allocate the buffer size as wchar_t* psz = new wchar_t[GetTranslatedBufferLength(...)];
	// The actual translated buffer can later be filled into the allocated string by supplying it along with the pHint to the GetTranslatedBuffer() method.
	// Parameters:
    //  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
    //  [in]  szInput: The Phonetic English String that is to be translated
	//  [Out] ppHint: Returns a Hint object pointer that can be used to retrieve the translated buffer later
    PHTRANSLATELIB_API size_t GetTranslatedBufferLength(void* Translator, const char* szInput, void** ppHint);

    // Translates the given string and returns the required buffer size to be allocated to hold the output.
	// You can directly use the return value to allocate the buffer size as wchar_t* psz = new wchar_t[GetTranslatedBufferLengthW(...)];
	// The actual translated buffer can later be filled into the allocated string by supplying it along with the pHint to the GetTranslatedBuffer() method.
	// Parameters:
    //  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
    //  [in]  szInput: The Phonetic English String that is to be translated
	//  [Out] ppHint: Returns a Hint object pointer that can be used to retrieve the translated buffer later
    PHTRANSLATELIB_API size_t GetTranslatedBufferLengthW(void* Translator, const wchar_t* szInput, void** ppHint);

    // Retrieves the translatedand buffer previously computed with GetTranslatedBufferLength/GetTranslatedBufferLengthW method.
	// Upon success, the Hint object will be reset to NULL to restrict its further usage.
	// Parameters:
    //  [out] szOutput: The buffer to hold the Translated String in Unicode representation
	//  [in/Out] ppHint: The Hint object pointer. This will be reset to NULL upon return.
    PHTRANSLATELIB_API void GetTranslatedBuffer(wchar_t* szOutput, void** ppHint);

	// Saves the current PhoneticTable used by the Translator to the specified file.
	// This saved file can later be used to create Custom Translators.
	// Inputs:
	//    szFilePath: Path of the file the PhoneticTable should be saved to.
	// Return value indicates the success or failure.
    PHTRANSLATELIB_API bool SavePhoneticTable(void* Translator, const char* szFilePath);

} // extern "C"

// Translates the given Phonetic English string.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
//  [in]  szInput: The Phonetic English String that is to be translated
//  [out] retStr: The Translated String in Unicode representation
//      If retStr is not empty on entry, translted string will be appended to it at the end automatically.
//		If there are any untranslatable chracters in the input, they will be output as is.
// Return value indicates the length of the new Unicode string generated as the result of translation.
//		If retStr is empty on entry, return value would be same as the length of retStr upon return.
//		If retStr is non-empty on entry, return value just indicates the length of the portion newly added, not the total string.
PHTRANSLATELIB_API size_t Translate(void* Translator, const char* szInput, std::wstring& retStr);

// Translates the given Phonetic English string. If the string contains non-Ascii characters they will be 
// inserted into the output string as is.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
//  [in]  szInput: The Phonetic English String that is to be translated
//  [out] retStr: The Translated String in Unicode representation
//      If retStr is not empty on entry, translted string will be appended to it at the end automatically.
//		If there are any untranslatable chracters in the input, they will be output as is.
// Return value indicates the length of the new Unicode string generated as the result of translation.
//		If retStr is empty on entry, return value would be same as the length of retStr upon return.
//		If retStr is non-empty on entry, return value just indicates the length of the portion newly added, not the total string.
PHTRANSLATELIB_API size_t Translate(void* Translator, const wchar_t* szInput, std::wstring& retStr);

// Translates the given Phonetic English string.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
//  [in]  szInput: The Phonetic English String that is to be translated
//			If there are any untranslatable chracters in the input, they will be output as is.
//  Returns the Translated Unicode String
PHTRANSLATELIB_API std::wstring Translate(void* Translator, const char* szInput);

// Translates the given Phonetic English string. If the string contains non-Ascii characters they will be 
// inserted into the output string as is.
// Parameters:
//  [in]  Translator: This must be a value returned by one of the GetTranslator methods or the CreateCustomTranslator method
//  [in]  szInput: The Phonetic English String that is to be translated
//			If there are any untranslatable chracters in the input, they will be output as is.
//  Returns the Translated Unicode String
PHTRANSLATELIB_API std::wstring Translate(void* Translator, const wchar_t* szInput);

#endif // PH_TRANSLATE_LIB_H

