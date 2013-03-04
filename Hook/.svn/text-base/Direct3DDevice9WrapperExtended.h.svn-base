#pragma once
#include "id3d9wrapper_device.h"
#include "Direct3DDevice9WrapperExtended.h"
#include "Window.h"
#include <set>
using std::set;
#include "IndexedPrimitive.h"

//-----------------------------------------------------------------------------------
//	class:	Direct3DDevice9WrapperExtended
//	--------------------------------------
//	Description:	This class is derived from Direct3DDevice9Wrapper, which 
//					implements a basic interception to all direct3ddevice9's 
//					functions. This class override the desired functions only, 
//					leaving all the clumsiness of the device interface untouched, 
//					in the base class.
//-----------------------------------------------------------------------------------

class Direct3DDevice9WrapperExtended : public Direct3DDevice9Wrapper
{
public:
	Direct3DDevice9WrapperExtended(IDirect3DDevice9* pDirect3DDevice9, IDirect3D9* pDirect3D9, D3DPRESENT_PARAMETERS *pPresentationParameters, HWND hFocusWindow);
	virtual ~Direct3DDevice9WrapperExtended();

	//	----------------------------------------------------------------------
	//	----------------------------------------------------------------------
	//
	//							Overridden Functions
	//
	//	----------------------------------------------------------------------
	//	----------------------------------------------------------------------

	//------------------------------------------------------------------------------------------
	//	Function name:	BeginScene
	//	Description:	The intercepted begin scene order. In it we reset all cycles flags - 
	//					and turn on sync flags to be used on an entire cycle.
	//------------------------------------------------------------------------------------------
	STDMETHOD(BeginScene)(THIS);

	//------------------------------------------------------------------------------------------
	//	Function name:	EndScene
	//	Description:	In here we draw the current scene to a bitmap, if it is indeed a screen
	//					capture cycle.
	//------------------------------------------------------------------------------------------
	STDMETHOD(EndScene)(THIS);

	//------------------------------------------------------------------------------------------
	//	Function name:	DrawIndexedPrimitive
	//	Description:	The main logic is implemented here. It handles a selection change (according to 
	//					the m_iCurrent counter increasing every primitive), a tracking set change (insertion
	//					or extraction from the tracking set), and the actual draw (If it is the selected
	//					primitive or if it's in the tracking set we use the white pixel shader to mark it).
	//	Parameters:		Direct3D9 parameters.
	//	Returns:		HRESULT - DX result constants
	//------------------------------------------------------------------------------------------
	STDMETHOD(DrawIndexedPrimitive)(THIS_ D3DPRIMITIVETYPE,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount);

	//------------------------------------------------------------------------------------------
	//	Function name:	Mine_WndProc
	//	Description:	Static function - hooked up as the application Message handler.
	//					We understand which input was given and act accordingly.
	//------------------------------------------------------------------------------------------
	static LRESULT CALLBACK Mine_WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

private:

	//------------------------------------------------------------------------------------------
	//	Function name:	StartCycle
	//	Description:	Updating all the flags - a new cycle had just started.
	//------------------------------------------------------------------------------------------
	void StartCycle();

	//------------------------------------------------------------------------------------------
	//	Function name:	Lock
	//	Description:	Updating the flags of a single action (used for syncronization on a draw cycle)
	//	Parameters:		The action flag and the sync flag of this action.
	//------------------------------------------------------------------------------------------
	void Lock(bool &action, bool &actionCycle);

	//	----------------------------------------------------------------------
	//	----------------------------------------------------------------------
	//
	//							Private Members
	//
	//	----------------------------------------------------------------------
	//	----------------------------------------------------------------------
	static DWORD m_dwWireframe;						//	Fillmode
	static WNDPROC	m_lpOldWndProc;					//	The old WndProc pointer
	static HWND m_GameHwnd;							//	The application window handler

	static int m_iSelected;							//	The selected indexed primitive, serially speaking
	static int m_iCurrent;							//	Indexed primitive counter for the current cycle.
	static set<IndexedPrimitive> *m_setSelected;	//	Set of all the Indexed Primitive we are currently tracking
	static bool m_bLogCycle;						//	Keyboard input flag - should we log?
	static bool m_bUpdateSelected;					//	Keyboard input flag - did we change selection?
	static bool m_bIsUpdateSelectedCycle;			//	Sync flag - Is this a cycle in which we should update our selection?
	static bool m_bPickedChanged;					//	Keyboard input flag - did we changed the tracking set?
	static bool m_bIsPickedChangedCycle;			//	Sync flag - Is this a cycle in which we should change the trcking set?
	static bool m_bHideSelected;					//	Keyboard input flag - should we hide the selection marking?
	static bool m_bCaptureScreen;					//	Keyboard input flag - should we capture the screen
	static bool m_bIsCaptureCycle;					//	Sync flag - Is this a cycle in which we should capture the screen as is?
	bool m_bIsTrackingCaptureCycle;					//	Sync flag - Is this a cycle in which we should capture the tracked primitives in the screen?
	IDirect3DPixelShader9 *m_psWhite;				//	The White pixel shader - used for tracking.
	IndexedPrimitive m_ipCurrentIndexedPrimitive;	//	The current selected Indexed primitive.
};
