/*************************************************************************

Framework\System.String.cpp

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

#pragma once

#include "commdef-int.h"

N System{N String{


	BSTR Alloc(LPCWSTR lpszString)
	{
		return SysAllocString(lpszString);
	}

	void Free(BSTR bstrString)
	{
		SysFreeString(bstrString);
	}

	LPWSTR CopyTo(LPWSTR lpszDest, LPCWSTR lpszSrc)
	{
        wchar_t * cp = lpszDest;

        while( *cp++ = *lpszSrc++ )
                ;               /* Copy src over dst */

        return( lpszDest );
	}
	LPSTR CopyTo(LPSTR lpszDest, LPCSTR lpszSrc)
	{
        char * cp = lpszDest;

        while( *cp++ = *lpszSrc++ )
                ;               /* Copy src over dst */

        return( lpszDest );
	}

	LPWSTR Join(LPWSTR lpszDest, LPCWSTR lpszSrc)
	{
        wchar_t * cp = lpszDest;

        while( *cp )
                cp++;                   /* find end of dst */

        while( *cp++ = *lpszSrc++ ) ;   /* Copy src to end of dst */

        return( lpszDest );             /* return dst */
	}
	LPSTR Join(LPSTR lpszDest, LPCSTR lpszSrc)
	{
        char * cp = lpszDest;

        while( *cp )
                cp++;                   /* find end of dst */

        while( *cp++ = *lpszSrc++ ) ;   /* Copy src to end of dst */

        return( lpszDest );             /* return dst */
	}

	BOOL StartWith(LPCWSTR lpszString, LPCWSTR lpszSymbol)
	{
		while(*lpszSymbol)
		{
			if(*lpszSymbol++ != *lpszString++)
				return FALSE;
		}
		return TRUE;
	}
	BOOL StartWith(LPCSTR lpszString, LPCSTR lpszSymbol)
	{
		while(*lpszSymbol)
		{
			if(*lpszSymbol++ != *lpszString++)
				return FALSE;
		}
		return TRUE;
	}

	size_t Length(LPCWSTR lpszString)
	{
        const wchar_t *eos = lpszString;

        while( *eos++ ) ;

        return( eos - lpszString - 1 );
	}
	size_t Length(LPCSTR lpszString)
	{
		const char *eos = lpszString;

		while( *eos++ ) ;

		return( eos - lpszString - 1 );
	}

	int Compare(LPCWSTR lpszStr1, LPCWSTR lpszStr2)
	{
        int ret = 0 ;

        while( ! (ret = (int)(*lpszStr1 - *lpszStr2)) && *lpszStr2)
                ++lpszStr1, ++lpszStr2;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
	}
	int Compare(LPCSTR lpszStr1, LPCSTR lpszStr2)
	{
        int ret = 0 ;

        while( ! (ret = (int)(*lpszStr1 - *lpszStr2)) && *lpszStr2)
                ++lpszStr1, ++lpszStr2;

        if ( ret < 0 )
                ret = -1 ;
        else if ( ret > 0 )
                ret = 1 ;

        return( ret );
	}

	int IndexOf(LPCWSTR lpszString, LPCWSTR lpszSymbol, int startIndex)
	{
		LPWSTR p = (LPWSTR)lpszString + startIndex, q = (LPWSTR)lpszSymbol;
		int index = 0;

		while(*p)
		{
			if(*q == *p)
			{
				q++;
				p++;
			}
			else
			{
				p++;
				q = (LPWSTR)lpszSymbol;
			}

			index++;

			if(!*q)
				return index - Length(lpszSymbol) + startIndex;
		}

		return -1;
	}
	int IndexOf(LPCSTR lpszString, LPCSTR lpszSymbol, int startIndex)
	{
		LPCSTR p = (LPCSTR)lpszString + startIndex, q = (LPCSTR)lpszSymbol;
		int index = 0;

		while(*p)
		{
			if(*q == *p)
			{
				q++;
				p++;
			}
			else
			{
				p++;
				q = (LPCSTR)lpszSymbol;
			}

			index++;

			if(!*q)
				return index - Length(lpszSymbol) + startIndex;
		}

		return -1;
	}

	BOOL Contains(LPCWSTR lpszString, LPCWSTR lpszSymbol)
	{
		return IndexOf(lpszString, lpszSymbol, 0) != -1;
	}
	BOOL Contains(LPCSTR lpszString, LPCSTR lpszSymbol)
	{
		return IndexOf(lpszString, lpszSymbol, 0) != -1;
	}

	int LastIndexOf(LPCWSTR lpszString, LPCWSTR lpszSymbol)
	{
		int nLastIndex = -1, index = -1;

		while((index = IndexOf(lpszString, lpszSymbol, index + 1)) != -1)
		{
			nLastIndex = index;
		}

		return nLastIndex;
	}
	int LastIndexOf(LPCSTR lpszString, LPCSTR lpszSymbol)
	{
		int nLastIndex = -1, index = -1;

		while((index = IndexOf(lpszString, lpszSymbol, index + 1)) != -1)
		{
			nLastIndex = index;
		}

		return nLastIndex;
	}

	int Ansic2Unicode(
		__in LPCSTR   lpMultiByteStr,
		__out_ecount_opt(cchWideChar) LPWSTR  lpWideCharStr,
		__in int      cchWideChar
		)
	{
		return MultiByteToWideChar(CP_ACP, 0, lpMultiByteStr, -1, lpWideCharStr, cchWideChar);
	}

	int Unicode2Ansic(
		__in_opt LPCWSTR  lpWideCharStr,
		__out_bcount_opt(cbMultiByte) LPSTR   lpMultiByteStr,
		__in int      cbMultiByte
		)
	{
		return WideCharToMultiByte(CP_ACP, 0, lpWideCharStr, -1, lpMultiByteStr, cbMultiByte, NULL, NULL);
	}

	int SubString(LPCWSTR lpszSrc, OUT LPWSTR lpszBuffer, int nStartIndex, int nLength)
	{
		wchar_t *p = lpszBuffer, *q = (wchar_t*)lpszSrc + nStartIndex;

		for(int i=0; i<nLength; i++)
			*p++ = *q++;
		*p = L'\0';

		return nLength;
	}

	int SubString(LPCSTR lpszSrc, OUT LPSTR lpszBuffer, int nStartIndex, int nLength)
	{
		char *p = lpszBuffer, *q = (char*)lpszSrc + nStartIndex;

		for(int i=0; i<nLength; i++)
			*p++ = *q++;
		*p = '\0';

		return nLength;
	}

}}