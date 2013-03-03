#include "stdafx.h"

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <d3d9.h>
#include <d3dx9.h>
#include <fstream>
#include <iostream>
#include <crtdbg.h>
#include <atlconv.h>
#include <atlbase.h>
#include <direct.h> 
#include <shlobj.h>

#include "log.h"
#include "Offsets.h"
#include "Utils.h"

using namespace std;

#pragma warning (disable: 4278)
#pragma warning (disable: 4996) //sprintf_s
#pragma warning (disable: 4244) //conversion from 'float' to 'int'

//  global data section so we can control things from the client via pinvoke
#pragma data_seg(".D3D9")
bool g_bCapturing = false;
bool g_bDIRECT3DHooked = false; // We have hooked d3d9 and have our virtual overrides in place
bool g_bFreeResourcesRequested = false;
bool g_bTextureCreated = false;
bool g_bPatchesCreated = false;
bool g_bResourcesCreated = false;
bool g_bLogging = true;
int g_width = 720;
int g_height = 480;
int g_bpp = 4; // bits per pixel calculated later from the backbuffer surface.
int g_srcPitch = 0;
#pragma data_seg()
#pragma comment(linker, "/section:.D3D9,rws") //End of global section

#pragma comment(lib, "d3dx9.lib")

////Begin Shared Memory Mapping
#define SHAREDMEMSIZE 16777216 //16MB
static BYTE* lpvMem = NULL;      // pointer to shared memory
static HANDLE hMapObject = NULL;  // handle to file mapping
////End Shared Memory Mapping

bool bResetInProgress = false;
bool bDeviceResetHookInstalled = false;
static float sgFPS = 0.0; // fps that gets calculated and displayed on-screen
char path_d3d9_dll[MAX_PATH]; //path d3d9.dll
HMODULE module_self = NULL; //handle to us

// DirectX objects used
ID3DXFont* font_object = NULL;
IDirect3DSurface9* g_renderTargetSurface = NULL;
IDirect3DTexture9* g_texture = NULL;

//used to position the overlay image
int nFromRectWidth = 0;
int nFromRectHeight = 0;

std::string workingdir; //used to locate the overlay iamge

//forward delcares
void WriteHook(void* address, unsigned char* patch);
void DeleteRenderSurfaces();
DWORD WINAPI ReleaseResources(void* parameter);
bool getSharedShouldWrite();
void setSharedShouldWrite(bool b);

//------------------------------------------- logging---------------------------------------------------------//

bool bClientLoaded = false;
bool bTargetLoaded = false;
CLog InternalLog;
CLog CallerLog;

void closeLogFile()
{
	if(!g_bLogging)
		return;
	
	if(bClientLoaded)
	{
		CallerLog.closeLogFile();
	}
	else if(bTargetLoaded)
	{
		InternalLog.closeLogFile();
	}
}

void DXLog(char* msg)
{
	if(!g_bLogging)
		return;

	if(bClientLoaded)
	{
		CallerLog.openLogFile("D3D9Hook-HookTestClient.log");
		CallerLog.Log(msg);
	}
	else if(bTargetLoaded)
	{
		InternalLog.openLogFile("D3D9Hook-HookedTarget.log");
		InternalLog.Log(msg);
	}
}

bool GetUsersDocumentsDirectory()
{
	HANDLE ProcToken = NULL;
	if(!OpenProcessToken( GetCurrentProcess(), TOKEN_READ, &ProcToken ))
	{
		return false;
	}

	TCHAR szBuffer[MAX_PATH*2] = { 0 };
	SHGetFolderPath( NULL, CSIDL_PERSONAL, ProcToken, SHGFP_TYPE_CURRENT, szBuffer );
	//CString SpecialFolderPath = szBuffer;

	CloseHandle( ProcToken );	
	
	return true;
}

// http://nibuthomas.com/2009/02/27/using-shgetfolderpath-to-retrieve-special-folder-paths-for-eg-my-documents/

//------------------------------------------- Utils ---------------------------------------------------//

// Get current working dir so we can grab the overlay sprite source image
void GetWorkingDir()
{
	//char* buffer;
	char buf[_MAX_PATH] = {0};
	getcwd(buf, _MAX_PATH);
	workingdir = buf;
	workingdir += "\\";
}

//--------------------------------------Overlay bitmap stuff-------------------------------------------//

LPD3DXSPRITE lpSprite = NULL;
LPDIRECT3DTEXTURE9 lpSpriteImage = NULL;
bool bSpriteCreated = false;

bool CreateOverlaySprite(IDirect3DDevice9* pd3dDevice)
{
	HRESULT hr;

	// make sure we have the working dir path 
	if(workingdir.length() == 0)
		return false;

	//cat dir and filename
	std::string filename = "d3d9hookoverlay.png";
	std::string path = workingdir + filename;

	hr = D3DXCreateTextureFromFile(pd3dDevice, path.c_str(), &lpSpriteImage);
	if(FAILED(hr))
	{
		//DXLog("D3DXCreateTextureFromFile failed");
		char buf[256];
		sprintf(buf, "D3DXCreateTextureFromFile failed: %s", path.c_str());
		DXLog(buf);

		bSpriteCreated = false;
		return false;
	}

	hr = D3DXCreateSprite(pd3dDevice, &lpSprite);
	if(FAILED(hr))
	{
		DXLog("D3DXCreateSprite failed");
		bSpriteCreated = false;
		return false;
	}

	bSpriteCreated = true;
	
	return true;
}


//--------------------------------------Surface processing routines-------------------------------------------//

void dumpSurfaceSettings(char* devName, LPDIRECT3DSURFACE9 surface)
{
	if(surface == NULL)
		return;

	DXLog(devName);

	D3DSURFACE_DESC desc;
	surface->GetDesc(&desc);
	char buf[256];
	
	sprintf(buf, "\tWidth: %d", desc.Width);
	DXLog(buf);
	sprintf(buf, "\tHeight: %d", desc.Height);
	DXLog(buf);
		
	switch(desc.Format)
	{
		case D3DFMT_R8G8B8:
			DXLog("\tD3DFMT_R8G8B8");
			break;
		case D3DFMT_A8R8G8B8:
			DXLog("\tD3DFMT_A8R8G8B8");
			break;
		case D3DFMT_X8R8G8B8:
			DXLog("\tD3DFMT_X8R8G8B8");
			break;
		case D3DFMT_R5G6B5:
			DXLog("\tD3DFMT_R5G6B5");
			break;
		case D3DFMT_X1R5G5B5:
			DXLog("\tD3DFMT_X1R5G5B5");
			break;
		case D3DFMT_A1R5G5B5:
			DXLog("\tD3DFMT_A1R5G5B5");
			break;
		case D3DFMT_A4R4G4B4:
			DXLog("\tD3DFMT_A4R4G4B4");
			break;
		case D3DFMT_R3G3B2:
			DXLog("\tD3DFMT_R3G3B2");
			break;
		case D3DFMT_A8R3G3B2:
			DXLog("\tD3DFMT_A8R3G3B2");
			break;
		case D3DFMT_X4R4G4B4:
			DXLog("\tD3DFMT_X4R4G4B4");
			break;
		case D3DFMT_A2B10G10R10:
			DXLog("\tD3DFMT_A2B10G10R10");
			break;
		case D3DFMT_A8B8G8R8:
			DXLog("\tD3DFMT_A8B8G8R8");
			break;
		case D3DFMT_A2R10G10B10:
			DXLog("\tD3DFMT_A2R10G10B10");
			break;
		case D3DFMT_A16B16G16R16:
			DXLog("\tD3DFMT_A16B16G16R16");
			break;
		case D3DFMT_UYVY:
			DXLog("\tD3DFMT_UYVY");
			break;
		case D3DFMT_R8G8_B8G8:
			DXLog("\tD3DFMT_R8G8_B8G8");
			break;
		case D3DFMT_YUY2:
			DXLog("\tD3DFMT_YUY2");
			break;
		case D3DFMT_G8R8_G8B8:
			DXLog("\tD3DFMT_G8R8_G8B8");
			break;
		case D3DFMT_DXT1:
			DXLog("\tD3DFMT_DXT1");
			break;
		case D3DFMT_DXT2:
			DXLog("\tD3DFMT_DXT2");
			break;
		case D3DFMT_DXT3:
			DXLog("\tD3DFMT_DXT3");
			break;
		case D3DFMT_DXT4:
			DXLog("\tD3DFMT_DXT4");
			break;
		case D3DFMT_DXT5:
			DXLog("\tD3DFMT_DXT5");
			break;
		
		default:
			DXLog("Unknown Format");
			break;
	}
}


// This is where we copy the current (about to be rendered backbuffer) to our texutre surface
static bool bLoggedSurfaceToTextureSurfaces = false;
HRESULT copySurfaceToTexture(IDirect3DDevice9* pd3dDevice, LPDIRECT3DSURFACE9 to, LPDIRECT3DSURFACE9 from)
{
	HRESULT hr = S_FALSE;

	//DXLog("copySurfaceToTexture");

	if (to != NULL && from != NULL)
	{
		//debug
		if(!bLoggedSurfaceToTextureSurfaces)
		{
			dumpSurfaceSettings("Surface we are copying to:", to);
			dumpSurfaceSettings("Surface we are copying from:", from);
			bLoggedSurfaceToTextureSurfaces = true;
		}
		
		D3DSURFACE_DESC desc;

		from->GetDesc(&desc);
		RECT fromRect;
		fromRect.left = 0;
		fromRect.top = 0;
		fromRect.right = desc.Width;
		fromRect.bottom = desc.Height;
		//save w/h for later when we position text and image sprite
		nFromRectWidth = desc.Width;
		nFromRectHeight = desc.Height;

		to->GetDesc(&desc);
		RECT toRect;
		toRect.left = 0;
		toRect.top = 0;
		toRect.right = desc.Width;
		toRect.bottom = desc.Height;

		hr = pd3dDevice->StretchRect(from, &fromRect, to, &toRect, D3DTEXF_NONE);
		
		if(FAILED(hr))
			DXLog("StretchRect failed when copying suface to texture");
	}

	return hr;
}

// Copies the surface from our texture surface and copies it to shared memory
HRESULT CopySurfaceToTextureBuffer(IDirect3DDevice9* device)
{
	HRESULT hRes = S_FALSE;

	//DEBUG - remove	
	if(bResetInProgress)
	{
		DXLog("Reset inprogress. Not copying surface"); 
		return S_OK;
	}
	
	// Get the render target and save it off
    LPDIRECT3DSURFACE9 orig_renderTarget = NULL;
    hRes = device->GetRenderTarget(0, &orig_renderTarget);
    if (hRes != D3D_OK) {
		DXLog("GetRenderTarget failed in CopySurfaceToTextureBuffer");
		return S_FALSE;
	}
	
	// copy the current backbuffer to our texutre surface
	hRes = copySurfaceToTexture(device, g_renderTargetSurface, orig_renderTarget);
	if (FAILED(hRes))
	{
		DXLog("copySurfaceToTexture: FAILED");
		SafeRelease(orig_renderTarget);
		return hRes;
	}		
	
	// Create an tempory surface on the GPU, read from the backbuffer via GetRenderTargetData,
    // and lock the new temporary surface.
	
	// Get the target surface description
	D3DSURFACE_DESC surfaceDesc;
	g_renderTargetSurface->GetDesc(&surfaceDesc);

	IDirect3DSurface9* pSurfTemp = NULL;
	hRes = device->CreateOffscreenPlainSurface( g_width,
												g_height,
												//surfaceDesc.Format,
												D3DFMT_X8R8G8B8, //Seems to work best. Using the target surface's doesn't always work
												D3DPOOL_SYSTEMMEM,
												&pSurfTemp,
												NULL);
	if (FAILED(hRes))
	{
		DXLog("CreateOffscreenPlainSurface: FAILED to create image surface");
		return hRes;
	}
	
	// now get the data from the temporary target surface
	hRes = device->GetRenderTargetData(g_renderTargetSurface, pSurfTemp);
	if (FAILED(hRes))
	{
		// This method will fail if:
		// The render target is multisampled.
		// The source render target is a different size than the destination surface.
		// The source render target and destination surface formats do not match.
		DXLog("ERROR: GetRenderTargetData() FAILED for image surface");
		SafeRelease(pSurfTemp);
		return hRes;
	}

	if(getSharedShouldWrite()) // is the shared buffer available?
	{
		// now lock the temporary texture surface and get the bytes
		D3DLOCKED_RECT lockedRect;
		if(FAILED(pSurfTemp->LockRect(&lockedRect, NULL, D3DLOCK_READONLY)))
		{
			// bad!
			DXLog("LockRect failed");
			device->SetRenderTarget( 0, orig_renderTarget);
			SafeRelease(orig_renderTarget);
			return hRes;
		}

		// set the image size and copy from GPU to system memory
		int newSize = g_width * g_height;
		int srcPitch = lockedRect.Pitch;
		g_srcPitch = lockedRect.Pitch;
					
		memcpy((BYTE*)lpvMem, (BYTE*)lockedRect.pBits, newSize* g_bpp);

		// finished with the temp suface, unlock and release
		pSurfTemp->UnlockRect();
		SafeRelease(pSurfTemp);

		// reset to original render target - NOTE: Not sure this is needed anymore, as we are not setting the render target
		device->SetRenderTarget( 0, orig_renderTarget);
		SafeRelease(orig_renderTarget);

		setSharedShouldWrite(false); //This should be a synchronization notify, but this works
	}
	else
	{
		//unlock and release
		pSurfTemp->UnlockRect();
		SafeRelease(pSurfTemp);

		SafeRelease(orig_renderTarget);
	}
	
	return S_OK;
}


//----------------------------------------DirectX Overrides-----------------------------------------------------//


// This is when the device is lost, for whatever reason - resized, minimized/maximized, etc.
HRESULT __stdcall ResetHook(IDirect3DDevice9* device, D3DPRESENT_PARAMETERS* pPresentationParameters)
{
	bResetInProgress = true;
	//DXLog("ResetHook - START");

	//device is being reset, delete all our surfaces. They get recreated when PrePresent() is called
	DeleteRenderSurfaces();
	
	WriteHook(address_Reset, backup_Reset);

	HRESULT hresult = S_OK;
	__asm {
		PUSH pPresentationParameters
		PUSH device

		CALL address_Reset
		MOV hresult, EAX
	}

	WriteHook(address_Reset, patch_Reset);
	bResetInProgress = false;
	return hresult;
	//return S_OK;
}

//Where we hook Device::Reset() - TODO: Move this to Initialize()
void InstallDeviceResetHook(IDirect3DDevice9* device)
{
	if (bDeviceResetHookInstalled)
		return;

	DXLog("InstallDeviceResetHook");

	// Create backup
	DWORD old_protect = 0;

	if (VirtualProtect(address_Reset, 5, PAGE_EXECUTE_READWRITE, &old_protect) == FALSE) return;
	memcpy(backup_Reset, address_Reset, 5);

	// Create patch
	DWORD from_int, to_int, offset;
	from_int = reinterpret_cast<DWORD> (address_Reset);
	to_int = reinterpret_cast<DWORD> (&ResetHook);
	offset = to_int - from_int - 5;

	patch_Reset[0] = 0xE9; // Rel32 JMP
	*(reinterpret_cast<DWORD*> (patch_Reset + 1)) = offset;

	// Install hook
	WriteHook(address_Reset, patch_Reset);

	bDeviceResetHookInstalled = true;
}

// Delete work surfaces when device gets reset
void DeleteRenderSurfaces()
{	
	if(g_renderTargetSurface)
	{
		DXLog("SafeRelease(g_renderTargetSurface)");
		//g_renderTargetSurface->Release();
		SafeRelease(g_renderTargetSurface);
		//g_renderTargetSurface = NULL;
	}

	if(g_texture)
	{
		DXLog("SafeRelease(g_texture)");
		SafeRelease(g_texture);
		//g_texture = NULL;
	}

	if (font_object != NULL)
	{
		DXLog("SafeRelease(font_object)");
		SafeRelease(font_object);
		//font_object = NULL;
	}

	if(lpSprite != NULL)
	{
		DXLog("SafeRelease(lpSprite)");
		SafeRelease(lpSprite);
	}
	
	bSpriteCreated = false;

	g_bTextureCreated = false;
	g_bResourcesCreated = false;
}

// Create our texture surface
void CreateTextureSurface(IDirect3DDevice9* device)
{
	HRESULT hRes = S_OK;
	IDirect3DSurface9 *pBackBuffer;
	D3DSURFACE_DESC desc;
	D3DFORMAT bbFormat;

	if(FAILED(device->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&pBackBuffer)))
	{
		DXLog("GetBackBuffer failed in CreateTextureSurface");
		return;
	}

	pBackBuffer->GetDesc(&desc);
	pBackBuffer->Release();

	bbFormat = desc.Format;
	long x = desc.Width;
	long y = desc.Height;

	//calc bpp from format
	switch(bbFormat)
	{
		case D3DFMT_R8G8B8:
			g_bpp = 3;
			break;
		case D3DFMT_R5G6B5:
		case D3DFMT_X1R5G5B5:
			g_bpp = 2;
			break;
		case D3DFMT_A8R8G8B8:
		case D3DFMT_X8R8G8B8:
			g_bpp = 4;
			break;
		default:
			g_bpp = 4; //Most cards are gonna support A8R8G8B8/X8R8G8B8 anyway
	}

	char buf[256];
	sprintf(buf, "BPP: %d", g_bpp);
	DXLog(buf);	

	// create our render target surface
    if(FAILED(device->CreateTexture(
								g_width,		// Width
								g_height,		// Height
								1,						// Levels (was using 1) - If this is zero, Direct3D will generate all texture sublevels down to 1
								D3DUSAGE_RENDERTARGET,  // Usage
								//bbFormat,               // Format
								D3DFMT_X8R8G8B8,		//This works with Aion
								D3DPOOL_DEFAULT,        // Pool (must be default)
								&g_texture, NULL)))
	{
		DXLog("CreateTexture FAILED");
		return;
	}

    // get pointer to highest mipmap surface. GPU will downsample to our selected width/height
    if(FAILED(g_texture->GetSurfaceLevel(0, &g_renderTargetSurface)))
	{
		DXLog("GetSurfaceLevel FAILED");
		g_bTextureCreated = false;
	}
	else
		g_bTextureCreated = true;
}

// create font object for displaying FPS and screen text
void CreateResourcesD3D9(IDirect3DDevice9* device)
{
	if SUCCEEDED(D3DXCreateFont(device,
							 20,			// Height (was using 40)
							 0,				// Width (0 default)
							 FW_NORMAL,		// Weight
							 1,				// MipLevels
							 FALSE,			// Italic
							 DEFAULT_CHARSET,
							 OUT_DEFAULT_PRECIS,
							 DEFAULT_QUALITY,
							 DEFAULT_PITCH,
							 "Franklin Gothic Medium",
							 &font_object)) {
		g_bResourcesCreated = true;
	}
}

// This gets called right before the frame is presented on-screen - Device::Present().
// First, create the display text, FPS and info message, on-screen. Then then call
// CopySurfaceToTextureBuffer() to downsample the image and copy to shared memory
void PrePresent(IDirect3DDevice9* device)
{
	//bail if we're not capturing
	if(!g_bCapturing)
		return;

	// crudely calculate FPS
	std::string message;
	static DWORD last = GetTickCount();
	static DWORD count = 0;
	DWORD now = GetTickCount();
	++count;
	if( now - last > 1000 )
	{
		sgFPS = (float)count / ( now - last ) * 1000;
		count = 0;
		last = now;
	}

	int textOffsetLeft;
	
	//draw sprite
	if(bSpriteCreated)
	{
		if(lpSprite != NULL && lpSprite != NULL)
		{
			D3DXVECTOR3 position;
			position.x = nFromRectWidth / 2.0f;
			position.y = 10.0f;
			position.z = 0.0f;

			textOffsetLeft = position.x; //for later to offset text from image

			lpSprite->Begin(D3DXSPRITE_ALPHABLEND);
			lpSprite->Draw(lpSpriteImage, NULL, NULL, &position, 0xFFFFFFFF);
			lpSprite->End();
		}
	}	

	// draw text
	if (font_object != NULL)
	{
		RECT target_rect;
		char buf[256];
		sprintf(buf, "(%.2f fps)", sgFPS);
		message.append(buf);

		//position text relative to sprite bottom
		target_rect.left = textOffsetLeft;
		target_rect.top = 80;
		target_rect.bottom = 750;
		target_rect.right = 1200;
		
		font_object->DrawTextA(NULL, message.c_str(), static_cast<int> (message.size()), &target_rect, 0, 0xFF404040);
		target_rect.left += 2; target_rect.top -= 2;
		font_object->DrawTextA(NULL, message.c_str(), static_cast<int> (message.size()), &target_rect, 0, 0xFFE0FFA0);
	}

	// if we're capturing, grab the frame buffer
	CopySurfaceToTextureBuffer(device);

	//check to see if the client has asked us to clean up
	if(g_bFreeResourcesRequested)
	{
		DXLog("Free Resources Requested");
		ReleaseResources(0);
	}
}

void PostPresent(IDirect3DDevice9* device)
{
	/* noop */
}

// This will get called before Device::Clear(). If the device has been reset
// then all the work surfaces will be created again.
void PreClear(IDirect3DDevice9* device)
{
	//hook Reset
	InstallDeviceResetHook(device);

	// Create our texture surface if needed
	if(!g_bTextureCreated)
		CreateTextureSurface(device);

	// Create our display text if needed
	if (!g_bResourcesCreated)
		CreateResourcesD3D9(device);

	if(!bSpriteCreated)
		CreateOverlaySprite(device);
}

void PostClear(IDirect3DDevice9* device)
{
	/* noop */
}

void PreCreateDevice(IDirect3DDevice9* device)
{
	/* noop */
}

// The hook function for IDirect3DSwapChain9::Present
HRESULT __stdcall SwapChainPresentHook(IDirect3DSwapChain9* swap_chain, const RECT* pSourceRect, const RECT* pDestRect,
									   HWND hDestWindowOverride, const RGNDATA* pDirtyRegion, DWORD dwFlags)
{
	//DXLog("SwapChainPresentHook");
	
	IDirect3DDevice9* device = NULL;
	if FAILED(swap_chain->GetDevice(&device))
	{
		// This is pretty unlikely, but it doesn't pay to gamble
		//DXLog("swap_chain->GetDevice FAILED");
		return swap_chain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	}

	//PrePresent(device);
	WriteHook(address_SwapChainPresent, backup_SwapChainPresent);
    HRESULT return_value = swap_chain->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
	WriteHook(address_SwapChainPresent, patch_SwapChainPresent);
    //PostPresent(device);

	return return_value;
}

// The hook function for IDirect3DDevice9::Present
HRESULT __stdcall DevicePresentHook(IDirect3DDevice9* device, const RECT* pSourceRect, const RECT* pDestRect,
									HWND hDestWindowOverride, const RGNDATA* pDirtyRegion)
{
	//DXLog("DevicePresentHook");
	
	PrePresent(device);
	WriteHook(address_DevicePresent, backup_DevicePresent);
    HRESULT return_value = device->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
	WriteHook(address_DevicePresent, patch_DevicePresent);
	//PostPresent(device);

    return return_value;
}

// The hook function for IDirect3DDevice9::Clear
HRESULT __stdcall ClearHook(IDirect3DDevice9* device, DWORD Count, CONST D3DRECT * pRects, DWORD Flags,
							D3DCOLOR Color, float Z, DWORD Stencil)
{
	//DXLog("ClearHook");
	PreClear(device);
    WriteHook(address_Clear, backup_Clear);
    HRESULT return_value = device->Clear(Count, pRects, Flags, Color, Z, Stencil);
	WriteHook(address_Clear, patch_Clear);
	//PostClear(device);

    return return_value;
}

void CreatePath()
{
	//DXLog("CreatePath()");
	GetSystemDirectory(path_d3d9_dll, MAX_PATH);
	strncat_s(path_d3d9_dll, MAX_PATH, "\\d3d9.dll", 10);
	DXLog(path_d3d9_dll);
}

void WriteHook(void* address, unsigned char* patch)
{
	// This will write the five-byte buffer at 'patch' to 'address',
	// after making sure that it's safe to write there.

	// Set access
	DWORD old_protect = 0;
	if (VirtualProtect(address, 5, PAGE_EXECUTE_READWRITE, &old_protect) == FALSE)
	{
		DXLog("VirtualProtect FAILED");
		return;
	}

	if (IsBadWritePtr(address, 5) != FALSE)
	{
		DXLog("ERROR: IsBadWritePtr");
		return;
	}

	memcpy(address, reinterpret_cast<void*> (patch), 5);
}

bool VerifyAddresses()
{
	// We can't afford to go overwriting arbitrary memory addresses without being
	// absolutely sure that they are indeed the right functions

	if (memcmp(backup_DevicePresent, EXPECTED_OPCODES_DEVICE_PRESENT, 5) != 0)
	{
		DXLog("verify backup_DevicePresent FAILED");
		return false;
	}

	if (memcmp(backup_SwapChainPresent, EXPECTED_OPCODES_SWAP_CHAIN_PRESENT, 5) != 0)
	{
		DXLog("verify backup_SwapChainPresent FAILED");
		return false;
	}

	if (memcmp(backup_Clear, EXPECTED_OPCODES_CLEAR, 5) != 0)
	{
		DXLog("verify backup_Clear FAILED");
		return false;
	}

	DXLog("VerifyAddresses succeeded");

	return true;
}

// Called when dll is loaded by LoadLibrary
DWORD WINAPI InitHooks(__in  LPVOID lpParameter)
{
	// NOTE: I don't right now, but we might want to define this as a ThreadProc so that
	// it may be used with CreateRemoteThread. This function may be called many
	// times, but must succeed at least once for the hooks to work.

	// This might not work, as we may miss a hook
	if(g_bDIRECT3DHooked)
	{
		DXLog("Dll already loaded and hooked into another process");
		return -1;
	}

	// Look for a suitable DLL
	char* address_d3d9 = reinterpret_cast<char*> (GetModuleHandleA(path_d3d9_dll));

	if (address_d3d9 == NULL)
	{
		DXLog("No d3d loaded by target process");
		return -1;
	}

	DWORD offset_device_present = 0;
	DWORD offset_swap_chain_present = 0;
	DWORD offset_clear = 0;
	DWORD offset_reset = 0;

	// Create backup
	DWORD old_protect = 0;

	// Identify OS
	OSVERSIONINFO version_info;
	version_info.dwOSVersionInfoSize = sizeof(version_info);
	GetVersionEx(&version_info);

	//match OS to offsets
	if (version_info.dwMajorVersion == 5)
	{
		// XP

		if(GetHostOS() == WIN64BIT)
		{
			offset_device_present = XP_x64_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = XP_x64_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = XP_x64_OFFSET_CLEAR;
			offset_reset = XP_x64_OFFSET_DEVICE_RESET;
		}
		else
		{
			offset_device_present = XP_x86_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = XP_x86_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = XP_x86_OFFSET_CLEAR;
			offset_reset = XP_x86_OFFSET_DEVICE_RESET;
		}
	}
	else if (version_info.dwMajorVersion == 6 && version_info.dwMinorVersion == 0)
	{
		// Vista (6.0)
		
		if(GetHostOS() == WIN64BIT)
		{
			offset_device_present = VISTA_x64_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = VISTA_x64_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = VISTA_x64_OFFSET_CLEAR;
			offset_reset = VISTA_x64_OFFSET_DEVICE_RESET;
		}
		else
		{
			offset_device_present = VISTA_x86_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = VISTA_x86_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = VISTA_x86_OFFSET_CLEAR;
			offset_reset = VISTA_x86_OFFSET_DEVICE_RESET;
		}
	}
	else if (version_info.dwMajorVersion == 6 && version_info.dwMinorVersion == 1)
	{
		// Windows 7 (6.1)

		if(GetHostOS() == WIN64BIT)
		{
			offset_device_present = WIN7_x64_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = WIN7_x64_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = WIN7_x64_OFFSET_CLEAR;
			offset_reset = WIN7_x64_OFFSET_DEVICE_RESET;
		}
		else
		{
			offset_device_present = WIN7_x86_OFFSET_DEVICE_PRESENT;
			offset_swap_chain_present = WIN7_x86_OFFSET_SWAP_CHAIN_PRESENT;
			offset_clear = WIN7_x86_OFFSET_CLEAR;
			offset_reset = WIN7_x86_OFFSET_DEVICE_RESET;
		}
	}
	else
	{
		DXLog("Unknown Windows version");
		return -2;
	}

	// Calculate addresses
	address_DevicePresent = reinterpret_cast<void*> (address_d3d9 + offset_device_present);
	address_SwapChainPresent = reinterpret_cast<void*> (address_d3d9 + offset_swap_chain_present);
	address_Clear = reinterpret_cast<void*> (address_d3d9 + offset_clear);
	address_Reset = reinterpret_cast<void*> (address_d3d9 + offset_reset);

	// Create backups
	if (!g_bPatchesCreated)
	{
		if (VirtualProtect(address_DevicePresent, 5, PAGE_EXECUTE_READWRITE, &old_protect) == FALSE) return -3;
		memcpy(backup_DevicePresent, address_DevicePresent, 5);

		if (VirtualProtect(address_SwapChainPresent, 5, PAGE_EXECUTE_READWRITE, &old_protect) == FALSE) return -4;
		memcpy(backup_SwapChainPresent, address_SwapChainPresent, 5);

		if (VirtualProtect(address_Clear, 5, PAGE_EXECUTE_READWRITE, &old_protect) == FALSE) return -5;
		memcpy(backup_Clear, address_Clear, 5);

		if (!VerifyAddresses())
		{
			DXLog("VerifyAddresses FAILED");
			return -6;
		}

		// We need the DLL to add a reference to itself here to prevent it being unloaded
		// before the process terminates, as there is no way to guarantee that unloading won't
		// occur at an inopportune moment and send everything down the pan.
		// This means that the DLL can't ever be unloaded, but that's the price we pay.
		char file_name_self[MAX_PATH];
		GetModuleFileName(module_self, file_name_self, MAX_PATH);
		LoadLibrary(file_name_self);

		// Create patches
		DWORD from_int, to_int, offset;

		// Device::Present
		from_int = reinterpret_cast<DWORD> (address_DevicePresent);
		to_int = reinterpret_cast<DWORD> (&DevicePresentHook);
		offset = to_int - from_int - 5;

		patch_DevicePresent[0] = 0xE9; // Rel32 JMP
		*(reinterpret_cast<DWORD*> (patch_DevicePresent + 1)) = offset;

		// SwapChain::Present
		from_int = reinterpret_cast<DWORD> (address_SwapChainPresent);
		to_int = reinterpret_cast<DWORD> (&SwapChainPresentHook);
		offset = to_int - from_int - 5;

		patch_SwapChainPresent[0] = 0xE9; // Rel32 JMP
		*(reinterpret_cast<DWORD*> (patch_SwapChainPresent + 1)) = offset;

		// Clear
		from_int = reinterpret_cast<DWORD> (address_Clear);
		to_int = reinterpret_cast<DWORD> (&ClearHook);
		offset = to_int - from_int - 5;

		patch_Clear[0] = 0xE9; // Rel32 JMP
		*(reinterpret_cast<DWORD*> (patch_Clear + 1)) = offset;

		g_bPatchesCreated = true;

		DXLog("InitHooks: patches created");
	}
	else
	{
		DXLog("InitHooks: patches already created");
	}

	// Install hooks
	WriteHook(address_DevicePresent, patch_DevicePresent);
	WriteHook(address_SwapChainPresent, patch_SwapChainPresent);
	WriteHook(address_Clear, patch_Clear);

	g_bDIRECT3DHooked = true;

	return 0;
}

// Here we release all that we created
DWORD WINAPI ReleaseResources(void* parameter)
{
	DXLog("ReleaseResources");

	//turn capturing off
	g_bCapturing = false;

	if(g_bDIRECT3DHooked)
	{
		DXLog("Removing all hooks and data...");
		
		if(g_renderTargetSurface)
		{
			//DXLog("SafeRelease(g_renderTargetSurface)");
			SafeRelease(g_renderTargetSurface);
		}

		if(g_texture)
		{
			//DXLog("SafeRelease(g_texture)");
			SafeRelease(g_texture);
		}

		// Release resources
		if (font_object != NULL)
			SafeRelease(font_object);

		if(lpSprite != NULL)
			SafeRelease(lpSprite);

		// Remove all patches
		WriteHook(address_DevicePresent, backup_DevicePresent);
		WriteHook(address_SwapChainPresent, backup_SwapChainPresent);
		WriteHook(address_Clear, backup_Clear);
		WriteHook(address_Reset, backup_Reset);

		// Reset access
		DWORD old_protect = 0;
		DWORD new_protect = PAGE_EXECUTE_READ;
		VirtualProtect(address_DevicePresent, 5, new_protect, &old_protect);
		VirtualProtect(address_SwapChainPresent, 5, new_protect, &old_protect);
		VirtualProtect(address_Clear, 5, new_protect, &old_protect);
		VirtualProtect(address_Reset, 5, new_protect, &old_protect);

		g_bDIRECT3DHooked = false;
	}
	
	DXLog("All hooks and data removed");
	g_bFreeResourcesRequested = false; //reset

	closeLogFile();

	return 0;
}

//client calls to toggle on
void StartCapturing()
{
	g_bCapturing = true;
}

//client calls to toggle off
void StopCapturing()
{
	g_bCapturing = false;
}

// Client exe calls this to see if this dll has been loaded and hooked into the target app.
int HostHooked()
{
	if(g_bDIRECT3DHooked)
	{
		DXLog("HostHooked == TRUE");
		return 1;
	}
	else
	{
		DXLog("HostHooked == FALSE");
		return 0;
	}
}

// Client exe calls this
void SetCapSize(int width, int height)
{
	DXLog("SetCapSize");
	g_width = width;
	g_height = height;
}

__declspec(dllexport) int __cdecl FillBuffer( BYTE *pbByteArray, int size )
{
	while(getSharedShouldWrite())
		Sleep(5); //This should be a synchronization wait, but it works as is

	// set the size
	int newSize = g_width * g_height;
	const int nSizeOfData = newSize * g_bpp;

    // make sure the memory is allocated and size is correct
	if (pbByteArray != NULL)
	{
        if (nSizeOfData > size)
			return -1;

        ::memcpy(pbByteArray, lpvMem, nSizeOfData);
	}

	setSharedShouldWrite(true);

	return nSizeOfData;
}

void setSharedShouldWrite(bool b)
{
	lpvMem[720*480*4] = b;
}

bool getSharedShouldWrite()
{
	return lpvMem[720*480*4] == 1;
}

// Client calls this to completly tear down everything
void FreeResources()
{
	DXLog("FreeResources called");
	g_bFreeResourcesRequested = true;
}

HINSTANCE hinstance = NULL;
static char buffer[256];

// Entry point
BOOL APIENTRY DllMain(HINSTANCE hModule, DWORD dwReason, PVOID lpReserved)
{
	string client = "HookTestClient";
	string caller;
	size_t found;
	BOOL fInit, fIgnore;

	switch (dwReason)
	{
    case DLL_PROCESS_ATTACH:
		
		//MEMORY MAPPED FILE HANDLING
		hMapObject = CreateFileMapping( 
                INVALID_HANDLE_VALUE,   // use paging file
                NULL,                   // default security attributes
                PAGE_READWRITE,         // read/write access
                0,                      // size: high 32-bits
                SHAREDMEMSIZE,              // size: low 32-bits
                TEXT("ded9dllmemfilemap")); // name of map object
        if (hMapObject == NULL) 
            return FALSE; 
		fInit = (GetLastError() != ERROR_ALREADY_EXISTS); 
		lpvMem = (BYTE*)MapViewOfFile( 
                hMapObject,     // object to map view of
                FILE_MAP_WRITE, // read/write access
                0,              // high offset:  map from
                0,              // low offset:   beginning
                0);             // default: map entire file
		if (lpvMem == NULL) 
           return FALSE; 
		if (fInit) 
           memset(lpvMem, '\0', SHAREDMEMSIZE);

		// Get the process name that's loading us
		char dbBuffer[256];
		GetModuleFileName( GetModuleHandle( NULL ), buffer, sizeof(buffer) );
		PathStripPath( buffer );
		sprintf(dbBuffer, "Process loading: %s", buffer);

		//get working dir
		GetWorkingDir();
				
		//look for "HookTestClient" in the caller name so we know what file to log to
		client = "HookTestClient";
		caller = buffer;
		found = caller.find(client);
		if (found != string::npos)
		{
			bClientLoaded = true;
		}
		else
		{
			bTargetLoaded = true;
		}

		DXLog("--------- DLL_PROCESS_ATTACH ---------");
		DXLog(dbBuffer);

		module_self = hModule; //ref to us
		CreatePath();
		InitHooks(0);
		break;

	case DLL_PROCESS_DETACH:
		// Unmap shared memory from the process's address space
        fIgnore = UnmapViewOfFile(lpvMem); 
        // Close the process's handle to the file-mapping object
        fIgnore = CloseHandle(hMapObject);

		DXLog("--------- DLL_PROCESS_DETACH ---------");
		ReleaseResources(0);
		closeLogFile();
		break;
	}

	return TRUE;
}