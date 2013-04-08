
// GUIDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "GUI.h"
#include "GUIDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CGUIDlg 对话框




CGUIDlg::CGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CGUIDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO1, mCombo);
}

BEGIN_MESSAGE_MAP(CGUIDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_PASSWORDEDIT, &CGUIDlg::OnPasswordChangeEdit)
	ON_BN_CLICKED(IDCANCEL, &CGUIDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK,&CGUIDlg::onLoginClicked)
	ON_EN_CHANGE(IDC_USERNAMEEDIT, &CGUIDlg::OnUsernameChangeEdit)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CGUIDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()


// CGUIDlg 消息处理程序
void CGUIDlg::onLoginClicked()
{
	CInternetSession session;
	session.SetOption(INTERNET_OPTION_CONNECT_TIMEOUT, 1000 * 20);
	session.SetOption(INTERNET_OPTION_CONNECT_BACKOFF, 1000);
	session.SetOption(INTERNET_OPTION_CONNECT_RETRIES, 1);
	CString szHeaders = L"Accept: audio/x-aiff, audio/basic, audio/midi,\
						 audio/mpeg, audio/wav, image/jpeg, image/gif, image/jpg, image/png,\
						 image/mng, image/bmp, text/plain, text/html, text/htm\r\n";
	CString szHeaders2   = L"Content-Type: application/x-www-form-urlencoded;charset=UTF-8";
	CString url;
	int quality=mCombo.GetCurSel()+1;
	url.Format(L"/play?quality=%d",quality);
	CHttpConnection* pConnection = session.GetHttpConnection(TEXT("222.200.182.75"),(INTERNET_PORT)8090);
	CHttpFile* pFile = pConnection->OpenRequest( CHttpConnection::HTTP_VERB_GET,
												 url);



	pFile->AddRequestHeaders(szHeaders);
	pFile->AddRequestHeaders(szHeaders2);
	pFile->SendRequest();

	DWORD dwRet;
	pFile->QueryInfoStatusCode(dwRet);

	if(dwRet != HTTP_STATUS_OK)
	{
		CString errText;
		errText.Format(L"请求出错，错误码：%d", dwRet);
		AfxMessageBox(errText);
		exit(-1);
	}
	else
	{
		int len = pFile->GetLength();
		char buf[2000];
		int numread;
		
		std::string jsonString;
		while ((numread = pFile->Read(buf,sizeof(buf)-1)) > 0)
		{
			//Write to a string
			
			jsonString.append(buf);
			
		}
		  Json::Reader reader;
		  Json::Value root;
		  if (!reader.parse(jsonString, root))
		  {
			  AfxMessageBox(TEXT("无法解析服务器返回的结果"));
			  exit(-1);
		  }
		 bool isSuccessful= root["successful"].asBool();
		 if(isSuccessful)
		 {
			 CString program;
			 char *serverAddr=(char *)root["serverIp"].asCString();
			
			 program.Format(TEXT("client.exe -a %s -q %d -p %d"),CString(serverAddr),quality,root["port"].asInt());
			 PROCESS_INFORMATION pi;  	
			STARTUPINFO si; 	
			si.cb = sizeof(STARTUPINFO); 
			si.lpReserved = NULL;   
			si.lpDesktop = NULL;   
			si.lpTitle = NULL;   
			si.dwFlags = 0;   
			si.cbReserved2 = 0;   
			si.lpReserved2 = NULL; 
			CreateProcess(NULL, (LPWSTR)(LPCTSTR)program, NULL, NULL, false, 
                            HIGH_PRIORITY_CLASS, NULL, NULL, &si, &pi);
			exit(0);
			 //AfxMessageBox(program);
			 //system(strbuf.c_str());
		 }
		 else
		 {
			 AfxMessageBox(TEXT("云没有准备好，请重试"));
			 exit(-1);
		 }
	}

	session.Close();
	pFile->Close(); 
	delete pFile;
	
}
BOOL CGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	mCombo.SetCurSel(0);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CGUIDlg::OnPasswordChangeEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CGUIDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnCancel();
}


void CGUIDlg::OnUsernameChangeEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CGUIDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
}
