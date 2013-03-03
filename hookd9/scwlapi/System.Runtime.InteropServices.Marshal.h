/*************************************************************************

Framework\System.Runtime.InteropServices.Marshal.h

	-by Miles Chen (stainboyx@hotmail.com) 2008-12-27

*************************************************************************/

N System{N Runtime{N InteropServices{N Marshal{


	LPVOID AllocCoTaskMem(DWORD dwSize);

	LPVOID ReAllocCoTaskMem(LPVOID lpAddress, DWORD dwSize);

	BOOL FreeCoTaskMem(LPVOID lpAddress);

}}}}