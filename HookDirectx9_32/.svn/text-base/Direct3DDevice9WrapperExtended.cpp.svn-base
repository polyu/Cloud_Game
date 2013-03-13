#include "Direct3DDevice9WrapperExtended.h"

DWORD Direct3DDevice9WrapperExtended::m_dwWireframe = D3DFILL_SOLID;
HWND Direct3DDevice9WrapperExtended::m_GameHwnd = NULL;
WNDPROC Direct3DDevice9WrapperExtended::m_lpOldWndProc = NULL;
set<IndexedPrimitive> *Direct3DDevice9WrapperExtended::m_setSelected = NULL;
int Direct3DDevice9WrapperExtended::m_iSelected = 0;
int Direct3DDevice9WrapperExtended::m_iCurrent = 0;
bool Direct3DDevice9WrapperExtended::m_bLogCycle = false;
bool Direct3DDevice9WrapperExtended::m_bIsUpdateSelectedCycle = false;
bool Direct3DDevice9WrapperExtended::m_bUpdateSelected = false;
bool Direct3DDevice9WrapperExtended::m_bPickedChanged = false;
bool Direct3DDevice9WrapperExtended::m_bIsPickedChangedCycle = false;
bool Direct3DDevice9WrapperExtended::m_bHideSelected = false;
bool Direct3DDevice9WrapperExtended::m_bCaptureScreen = false;
bool Direct3DDevice9WrapperExtended::m_bIsCaptureCycle = false;

Direct3DDevice9WrapperExtended::Direct3DDevice9WrapperExtended(IDirect3DDevice9* pDirect3DDevice9, IDirect3D9* pDirect3D9, D3DPRESENT_PARAMETERS *pPresentationParameters, HWND hFocusWindow):
Direct3DDevice9Wrapper(pDirect3DDevice9, pDirect3D9, pPresentationParameters)
{
	m_setSelected = new set<IndexedPrimitive>();
	m_GameHwnd = hFocusWindow;
	m_bIsTrackingCaptureCycle = m_bIsLogCycle = false;

	if (!m_GameHwnd){
		DXLOG("No Game Handler :(\n");
	}

	if (!m_lpOldWndProc && m_GameHwnd){
		DXLOG("Hooked on WndProc.\n");

		m_lpOldWndProc = (WNDPROC)(LONG_PTR)SetWindowLongPtr(m_GameHwnd, GWL_WNDPROC, (LONG)(LONG_PTR)Mine_WndProc);
	}


	//	Setting up pixel shaders
	ID3DXBuffer *tmp;
	if (D3DXCompileShaderFromFile(L"White.psh", 0, 0, "main", "ps_2_0", 0, &tmp, 0, 0) != D3D_OK)
		DXLOGALWAYS("Failed Compiling PS.\n");

	Direct3DDevice9->CreatePixelShader((DWORD*)tmp->GetBufferPointer(), &m_psWhite);

	tmp->Release();
}

Direct3DDevice9WrapperExtended::~Direct3DDevice9WrapperExtended()
{
	delete m_setSelected;
}

HRESULT Direct3DDevice9WrapperExtended::BeginScene()
{
	//	A rendering cycle has begun
	DXLOGCYCLE("Direct3DDevice9WrapperExtended: BeginScene\n");
	StartCycle();
	return Direct3DDevice9->BeginScene();
}

HRESULT Direct3DDevice9WrapperExtended::EndScene()
{
	DXLOGCYCLE("Direct3DDevice9WrapperExtended: EndScene\n");

	if (m_bIsCaptureCycle || m_bIsTrackingCaptureCycle){
		IDirect3DSurface9 *surf;
		Direct3DDevice9->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&surf);
		string filename;
		if (m_bIsCaptureCycle)
			filename = "Screen.bmp";
		else
			filename = "Tracking.bmp";

		DXLOGALWAYS("Taking pic.\n")

		D3DXSaveSurfaceToFileA(filename.c_str(), D3DXIFF_BMP, surf, 0, 0);

		if (m_bIsCaptureCycle)
			m_bIsTrackingCaptureCycle = true;
		else
			m_bIsTrackingCaptureCycle = false;

		surf->Release();
	}
	return Direct3DDevice9->EndScene();
}

HRESULT Direct3DDevice9WrapperExtended::DrawIndexedPrimitive( D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	char tmp[250];
	sprintf(tmp, "Direct3DDevice9WrapperExtended: DrawIndexedPrimitive. PrimitiveType = %u, BaseVertexIndex = %d, MinVertexIndex = %u, NumVertices = %u, startIndex = %u, primCount = %u\n", PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	DXLOGCYCLE(tmp);

	bool inSet = m_setSelected->find(IndexedPrimitive(BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount)) != m_setSelected->end();
	bool isCurrent = m_iSelected == m_iCurrent;

	//	Update the selected indexed primitive
	if ((m_bIsPickedChangedCycle || m_bIsUpdateSelectedCycle) && isCurrent){
		m_ipCurrentIndexedPrimitive.Set(BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount);
	}

	//	Insert or remove from selected set
	if (m_bIsPickedChangedCycle && isCurrent){
		if (inSet)
			m_setSelected->erase(m_ipCurrentIndexedPrimitive);
		else
			m_setSelected->insert(m_ipCurrentIndexedPrimitive);

		inSet = !inSet;
	}
	
	//	Draw the indexed primitive - if it's in the set or selected, use white pixel shader
	if ((!m_bHideSelected && isCurrent) || inSet){
		IDirect3DPixelShader9 *psTmp;
		if (!m_bIsCaptureCycle){
			Direct3DDevice9->GetPixelShader(&psTmp);
			Direct3DDevice9->SetPixelShader(m_psWhite);
		}

		HRESULT res = Direct3DDevice9->DrawIndexedPrimitive( PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount); 

		if (!m_bIsCaptureCycle)
			Direct3DDevice9->SetPixelShader(psTmp);

		m_iCurrent++;
		return res;
	}
	m_iCurrent++;

	if(LOWORD(GetKeyState(VK_CAPITAL)) & 1) // Caps is on
		return D3D_OK;

	HRESULT res = Direct3DDevice9->DrawIndexedPrimitive( PrimitiveType, BaseVertexIndex, MinVertexIndex, NumVertices, startIndex, primCount); 

	return res;
}

void Direct3DDevice9WrapperExtended::Lock(bool &action, bool &actionCycle)
{
	if (actionCycle)
		actionCycle = false;

	if (action){
		action = false;
		actionCycle = true;
	}
}

void Direct3DDevice9WrapperExtended::StartCycle()
{
	Lock(m_bLogCycle, m_bIsLogCycle);
	Lock(m_bUpdateSelected, m_bIsUpdateSelectedCycle);
	Lock(m_bPickedChanged, m_bIsPickedChangedCycle);
	Lock(m_bCaptureScreen, m_bIsCaptureCycle);
	m_iCurrent = 0;
}