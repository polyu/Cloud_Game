#pragma once



// PortalWindow

class PortalWindow : public CWinApp
{
	DECLARE_DYNCREATE(PortalWindow)

protected:
	PortalWindow();           // 动态创建所使用的受保护的构造函数
	virtual ~PortalWindow();

public:
	virtual BOOL InitInstance();
	virtual int ExitInstance();

protected:
	DECLARE_MESSAGE_MAP()
};


