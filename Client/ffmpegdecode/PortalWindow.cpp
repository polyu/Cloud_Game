// PortalWindow.cpp : 实现文件
//

#include "stdafx.h"
#include "PortalWindow.h"


// PortalWindow

IMPLEMENT_DYNCREATE(PortalWindow, CWinApp)

PortalWindow::PortalWindow()
{
}

PortalWindow::~PortalWindow()
{
}

BOOL PortalWindow::InitInstance()
{
	// TODO: 在此执行任意逐线程初始化
	return TRUE;
}

int PortalWindow::ExitInstance()
{
	// TODO: 在此执行任意逐线程清理
	return CWinApp::ExitInstance();
}

BEGIN_MESSAGE_MAP(PortalWindow, CWinApp)
END_MESSAGE_MAP()


// PortalWindow 消息处理程序
