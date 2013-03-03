/*************************************************************************

Framework\System.String.h

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

N System{N String{


	BSTR Alloc(LPCWSTR lpszString);

	void Free(BSTR bstrString);

	LPWSTR CopyTo(LPWSTR lpszDest, LPCWSTR lpszSrc);
	LPSTR CopyTo(LPSTR lpszDest, LPCSTR lpszSrc);

	LPWSTR Join(LPWSTR lpszDest, LPCWSTR lpszSrc);
	LPSTR Join(LPSTR lpszDest, LPCSTR lpszSrc);

	BOOL StartWith(LPCWSTR lpszString, LPCWSTR lpszSymbol);
	BOOL StartWith(LPCSTR lpszString, LPCSTR lpszSymbol);

	size_t Length(LPCWSTR lpszString);
	size_t Length(LPCSTR lpszString);

	int Compare(LPCWSTR lpszStr1, LPCWSTR lpszStr2);
	int Compare(LPCSTR lpszStr1, LPCSTR lpszStr2);

	int IndexOf(LPCWSTR lpszString, LPCWSTR lpszSymbol, int startIndex);
	int IndexOf(LPCSTR lpszString, LPCSTR lpszSymbol, int startIndex);

	BOOL Contains(LPCWSTR lpszString, LPCWSTR lpszSymbol);
	BOOL Contains(LPCSTR lpszString, LPCSTR lpszSymbol);

	int LastIndexOf(LPCWSTR lpszString, LPCWSTR lpszSymbol);
	int LastIndexOf(LPCSTR lpszString, LPCSTR lpszSymbol);

	int Ansic2Unicode(
		__in LPCSTR   lpMultiByteStr,
		__out_ecount_opt(cchWideChar) LPWSTR  lpWideCharStr,
		__in int      cchWideChar
		);

	int Unicode2Ansic(
		__in_opt LPCWSTR  lpWideCharStr,
		__out_bcount_opt(cbMultiByte) LPSTR   lpMultiByteStr,
		__in int      cbMultiByte
		);

	int SubString(LPCWSTR lpszSrc, OUT LPWSTR lpszBuffer, int nStartIndex, int nLength);
	int SubString(LPCSTR lpszSrc, OUT LPSTR lpszBuffer, int nStartIndex, int nLength);


}}