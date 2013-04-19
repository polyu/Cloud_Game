
// GUIDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include <json/json.h>
#include <afxinet.h>
#define RESERVEDMEMORY 256
#define RHEIGHT 720
#define RWIDTH 1280
#define SHAREDMEMSIZE 1440*900*32+RESERVEDMEMORY
#define MAXFPS 40
// CGUIDlg 对话框
class CGUIDlg : public CDialogEx
{
// 构造
public:
	CGUIDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_GUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPasswordChangeEdit();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnUsernameChangeEdit();
	afx_msg void OnCbnSelchangeCombo1();
	afx_msg void onLoginClicked();
	CComboBox mCombo;
	CComboBox gameCombo;
	CEdit gameIntroduction;
	CStatic gamePhoto;
	bool setupSharedMemory();
	void uninstallSharedMemory();
	BYTE* lpvMem;      // pointer to shared memory
	HANDLE hMapObject; 
	void resourceDeallocate();
	bool badMemory;
	long lastRecordTime;
	bool isMemoryWritable();
	void setMemoryReadable();
};
