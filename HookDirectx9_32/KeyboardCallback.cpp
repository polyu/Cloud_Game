#include "Direct3DDevice9WrapperExtended.h"

LRESULT CALLBACK Direct3DDevice9WrapperExtended::Mine_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	if (uMsg == WM_KEYDOWN){
		switch (wParam)
		{
		case VK_F1:
			m_dwWireframe = (m_dwWireframe == D3DFILL_SOLID)? D3DFILL_WIREFRAME : D3DFILL_SOLID;
			if (Direct3DDevice9){
				Direct3DDevice9->SetRenderState(D3DRS_FILLMODE, m_dwWireframe);
			}
			break;

		case VK_F2:
			m_iSelected--;
			if (m_iSelected < 0)
				m_iSelected = 0;
			else
				m_bUpdateSelected = true;
			break;

		case VK_F3:
			m_iSelected++;
			m_bUpdateSelected = true;
			break;

		case VK_F4:
			m_bPickedChanged = true;
			break;

		case VK_DELETE:									
			m_setSelected->clear();
			m_iSelected = 0;
			m_bUpdateSelected = true;
			break;

		case VK_INSERT:									
			m_bLogCycle = true;
			break;

		case VK_F5:
			m_bCaptureScreen = true;
			break;

		case VK_F11:
			m_bHideSelected = !m_bHideSelected;
			break;
		}
	}
	return CallWindowProc(m_lpOldWndProc, hWnd, uMsg, wParam, lParam);
}