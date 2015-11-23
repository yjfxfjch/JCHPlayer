#include "DuiFrameDlg.h"
#include <windows.h>
//选择文件夹对话框  
#include<Shlobj.h>  
#include <time.h>
#pragma comment(lib,"Shell32.lib")  

#define WM_USER_PLAYING         WM_USER + 1     // 开始播放文件
#define WM_USER_POS_CHANGED     WM_USER + 2     // 文件播放位置改变
#define WM_USER_END_REACHED     WM_USER + 3     // 播放完毕
const UINT_PTR U_TAG_PLAYLIST = 1;
std::string WstringToString(const std::wstring wstr);


void CALLBACK TimeProc(
	HWND hwnd,
	UINT message,
	UINT idTimer,
	DWORD dwTime);
BOOL CALLBACK EnumerateVLC(HWND hWndvlc, LPARAM lParam);

void CallbackPlayer(void *data, UINT uMsg)
{
	CAVPlayer *pAVPlayer = (CAVPlayer *)data;

	if (pAVPlayer)
	{
		HWND hWnd = pAVPlayer->GetHWND();

		if (::IsWindow(hWnd) && ::IsWindow(::GetParent(hWnd)))
		{
			::PostMessage(::GetParent(hWnd), uMsg, (WPARAM)data, 0);
		}
	}
}

void CallbackPlaying(void *data)
{
	CallbackPlayer(data, WM_USER_PLAYING);
}

void CallbackPosChanged(void *data)
{
	CallbackPlayer(data, WM_USER_POS_CHANGED);
}

void CallbackEndReached(void *data)
{
	CallbackPlayer(data, WM_USER_END_REACHED);
}


CDuiFrameDlg::CDuiFrameDlg(LPCTSTR pszXMLName)
	: CXMLWnd(pszXMLName),
	m_bIsFullScreen(FALSE),
	m_pSliderPlay(NULL),
	m_bIsShowPlaylist(TRUE),
	m_bPingPong(TRUE)
{

}

void CDuiFrameDlg::InitWindow()
{
	CenterWindow();

	HMONITOR monitor = MonitorFromWindow(*this, MONITOR_DEFAULTTONEAREST);
	MONITORINFO info;
	info.cbSize = sizeof(MONITORINFO);
	GetMonitorInfo(monitor, &info);
	m_iMonitorWidth = info.rcMonitor.right - info.rcMonitor.left;
	m_iMonitorHeight = info.rcMonitor.bottom - info.rcMonitor.top;


	// 几个常用控件做为成员变量
	CSliderUI* pSilderVol = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("sliderVol")));
	m_pSliderPlay = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("sliderPlay")));
	m_pLabelTime = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("labelPlayTime")));

	if (!pSilderVol || !m_pSliderPlay || !m_pLabelTime)
	{
		return;
	}

	pSilderVol->OnNotify += MakeDelegate(this, &CDuiFrameDlg::OnVolumeChanged);
	m_pSliderPlay->OnNotify += MakeDelegate(this, &CDuiFrameDlg::OnPosChanged);

	CWndUI *pWnd = static_cast<CWndUI*>(m_PaintManager.FindControl(_T("WndMedia")));
	if (pWnd)
	{
		m_myPlayer.SetHWND(pWnd->GetHWND());
		m_myPlayer.SetCallbackPlaying(CallbackPlaying);
		m_myPlayer.SetCallbackPosChanged(CallbackPosChanged);
		m_myPlayer.SetCallbackEndReached(CallbackEndReached);
	}

}

CDuiFrameDlg::~CDuiFrameDlg()
{

}

DUI_BEGIN_MESSAGE_MAP(CDuiFrameDlg, CNotifyPump)
	DUI_ON_MSGTYPE(DUI_MSGTYPE_CLICK, OnClick)
DUI_END_MESSAGE_MAP()


/************************************************************************/
/* 
功能：创建自定义控件
*/
/************************************************************************/
CControlUI* CDuiFrameDlg::CreateControl(LPCTSTR pstrClassName)
{
	CDuiString     strXML;
	CDialogBuilder builder;

	if (_tcsicmp(pstrClassName, _T("Caption")) == 0)
	{
		strXML = _T("Caption.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("PlayPanel")) == 0)
	{
		strXML = _T("PlayPanel.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("Playlist")) == 0)
	{
		strXML = _T("Playlist.xml");
	}
	else if (_tcsicmp(pstrClassName, _T("WndMediaDisplay")) == 0)
	{
		CWndUI *pUI = new CWndUI;
		HWND   hWnd = CreateWindow(_T("#32770"), _T("WndMediaDisplay"), WS_VISIBLE | WS_CHILD, 0, 0, 0, 0, m_PaintManager.GetPaintWindow(), (HMENU)0, NULL, NULL);
		pUI->Attach(hWnd);
		return pUI;
	}

	if (!strXML.IsEmpty())
	{
		CControlUI* pUI = builder.Create(strXML.GetData(), NULL, NULL, &m_PaintManager, NULL); // 这里必须传入m_PaintManager，不然子XML不能使用默认滚动条等信息。
		return pUI;
	}

	return NULL;
}


/************************************************************************/
/* 
功能：按钮点击响应
*/
/************************************************************************/
void CDuiFrameDlg::OnClick(TNotifyUI& msg)
{
	if (msg.pSender->GetName() == _T("btnPlay"))
	{
		CControlUI *pBtnPlay = m_PaintManager.FindControl(_T("btnPlay"));
		CControlUI *pbtnPause = m_PaintManager.FindControl(_T("btnPause"));
		if (!m_myPlayer.IsPlaying() && m_myPlayer.IsOpen())
		{
			m_myPlayer.Play();
			pBtnPlay->SetVisible(FALSE);
			pbtnPause->SetVisible(TRUE);
		}
		
	}
	else if (msg.pSender->GetName() == _T("btnPause"))
	{
		CControlUI *pBtnPlay = m_PaintManager.FindControl(_T("btnPlay"));
		CControlUI *pbtnPause = m_PaintManager.FindControl(_T("btnPause"));
		m_myPlayer.Pause();
		pBtnPlay->SetVisible(TRUE);
		pbtnPause->SetVisible(FALSE);
	}
	else if (msg.pSender->GetName() == _T("btnScreenFull"))
	{
		SetFullScreen(TRUE);
	}
	else if (msg.pSender->GetName() == _T("btnScreenNormal"))
	{
		SetFullScreen(FALSE);
	}
	else if (msg.pSender->GetName() == _T("btnOpenMini"))
	{
		OpenFolderDlg();
	}else if (msg.pSender->GetName() == _T("btnPlaylistShow"))
	{
		ShowPlaylist(TRUE);
		m_bIsShowPlaylist = TRUE;
	}
	else if (msg.pSender->GetName() == _T("btnPlaylistHide"))
	{
		ShowPlaylist(FALSE);
		m_bIsShowPlaylist = FALSE;
	}
	else if (msg.pSender->GetName() == _T("btnVolume"))
	{
		m_myPlayer.Volume(0);
		m_PaintManager.FindControl(_T("btnVolumeZero"))->SetVisible(true);
		msg.pSender->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("btnVolumeZero"))
	{
		CSliderUI* pUI = static_cast<CSliderUI*>(m_PaintManager.FindControl(_T("sliderVol")));
		m_myPlayer.Volume(pUI->GetValue());
		m_PaintManager.FindControl(_T("btnVolume"))->SetVisible(true);
		msg.pSender->SetVisible(false);
	}
	else if (msg.pSender->GetName() == _T("btnNext"))
	{
		

	}
	else if (msg.pSender->GetName() == _T("btnFastForward"))
	{
		m_myPlayer.SeekForward();
		::PostMessage(*this, WM_USER_POS_CHANGED, 0, m_myPlayer.GetPos());
	}
	else if (msg.pSender->GetName() == _T("btnFastBackward"))
	{
		m_myPlayer.SeekBackward();
		::PostMessage(*this, WM_USER_POS_CHANGED, 0, m_myPlayer.GetPos());
	}
	else if (msg.pSender->GetName() == _T("btnStop"))
	{
		if (m_myPlayer.IsPlaying())
		{
			m_myPlayer.Stop();
			CControlUI *pbtnWndBkg = m_PaintManager.FindControl(L"MediaBkg");
			CControlUI *pbtnWnd = m_PaintManager.FindControl(L"WndMedia");
			pbtnWndBkg->SetVisible(TRUE);
			pbtnWnd->SetVisible(FALSE);
		}
		
	}
	
	__super::OnClick(msg);
}

/************************************************************************/
/* 
功能：设置窗口开始播放
*/
/************************************************************************/
void CDuiFrameDlg::ShowPlayWnd(BOOL show)
{

	CControlUI *pbtnWnd = m_PaintManager.FindControl(L"WndMedia");
	CControlUI *pbtnWndBkg = m_PaintManager.FindControl(L"MediaBkg");
	if (pbtnWnd && pbtnWndBkg)
	{
		pbtnWnd->SetVisible(show);
		pbtnWndBkg->SetVisible(!show);
	}
	
}


/************************************************************************/
/* 
功能：打开文件夹对话框
*/
/************************************************************************/
void CDuiFrameDlg::OpenFolderDlg()
{
	BROWSEINFO bifolder;
	WCHAR FileName[MAX_PATH];
	ZeroMemory(&bifolder, sizeof(BROWSEINFO));
	bifolder.hwndOwner = *this;              // 拥有者句柄  
	bifolder.pszDisplayName = FileName;     // 存放目录路径缓冲区  
	bifolder.lpszTitle = TEXT("请选择文件夹");    // 标题  
	bifolder.ulFlags = BIF_NEWDIALOGSTYLE | BIF_EDITBOX; // 新的样式,带编辑框  
	LPITEMIDLIST idl = SHBrowseForFolder(&bifolder);

	if (SHGetPathFromIDList(idl, FileName)) 
	{
		if (!AddPlayFile(FileName))
		{
			MessageBox(NULL, L"该目录下没有找到可播放视频文件", L"提示", NULL);
			return;
		}
		m_strFolderName = FileName;
	}
}


/************************************************************************/
/* 
功能：向播放列表添加文件夹
*/
/************************************************************************/
BOOL CDuiFrameDlg::AddPlayFile(WCHAR *folder)
{
	m_vcPlayFile.clear();
	int iFileNum = 0;
	WCHAR szFolder[MAX_PATH] = {0};
	WIN32_FIND_DATA data;
	swprintf_s(szFolder, _T("%s\\*.*"), folder);
	HANDLE h = ::FindFirstFile(szFolder, &data);
	if (INVALID_HANDLE_VALUE != h)
	{
		do
		{
			//枚举文件而不是目录
			if (!(data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				wstring strFileName(data.cFileName);
				if (-1 != strFileName.find(L"MP4") || -1 != strFileName.find(L"mp4") || -1 != strFileName.find(L"rmvb")
					|| -1 != strFileName.find(L"mkv"))
				{
					m_vcPlayFile.push_back(data.cFileName);
					iFileNum++;
				}
			}

		} while (::FindNextFile(h, &data));
		::FindClose(h);
	}

	CTreeNodeUI *pNodeTmp;
	CTreeViewUI *pTreeList;
	pTreeList = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("treePlaylist")));
	pTreeList->RemoveAll();

	for (auto it = m_vcPlayFile.begin(); it != m_vcPlayFile.end();++it)
	{
		pNodeTmp = new CTreeNodeUI;
		pNodeTmp->SetItemTextColor(0xFF000000);
		pNodeTmp->SetItemHotTextColor(0xFFC8C6CB);
		pNodeTmp->SetSelItemTextColor(0xFFFFFFFF);
		pNodeTmp->SetTag(U_TAG_PLAYLIST);
		pNodeTmp->SetItemText((*it).c_str());
		pNodeTmp->SetAttribute(_T("height"), _T("23"));
		pNodeTmp->SetAttribute(_T("inset"), _T("0,1,1,1"));
		pNodeTmp->SetAttribute(_T("itemattr"), _T("valign=\"vcenter\" font=\"4\""));
		pNodeTmp->SetAttribute(_T("folderattr"), _T("width=\"0\" float=\"true\""));
		pTreeList->Add(pNodeTmp);
	}

	CControlUI *pbtnScreen = m_PaintManager.FindControl(_T("btnScreenFull"));
	pbtnScreen->SetEnabled(true);

	return iFileNum > 0 ? TRUE : FALSE;
}

/************************************************************************/
/* 
功能：显示播放列表
*/
/************************************************************************/
void CDuiFrameDlg::ShowPlaylist(BOOL show)
{
	CControlUI *pctnPlaylist = m_PaintManager.FindControl(_T("ctnPlaylist"));
	CControlUI *pbtnHide = m_PaintManager.FindControl(_T("btnPlaylistHide"));
	CControlUI *pbtnShow = m_PaintManager.FindControl(_T("btnPlaylistShow"));
	

	if (pctnPlaylist && pbtnHide && pbtnShow/* && pctnSlider*/)
	{
		pctnPlaylist->SetVisible(show);
		pbtnHide->SetVisible(show);
		pbtnShow->SetVisible(!show);
	}
}

LRESULT CDuiFrameDlg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes = __super::HandleMessage(uMsg, wParam, lParam);

	switch (uMsg)
	{
		//HANDLE_MSG(*this, WM_DROPFILES, OnDropFiles);
		//HANDLE_MSG(*this, WM_DISPLAYCHANGE, OnDisplayChange);
		HANDLE_MSG(*this, WM_GETMINMAXINFO, OnGetMinMaxInfo);

	case WM_USER_PLAYING:
		return OnPlaying(*this, wParam, lParam);
	case WM_USER_POS_CHANGED:
		return OnPosChanged(*this, wParam, lParam);
	case WM_USER_END_REACHED:
		return OnEndReached(*this, wParam, lParam);
	case WM_MOUSEMOVE:
		OnMouseMove();
		break;

	case WM_LBUTTONDBLCLK:
	{
		if (IsClickPlayWnd())
		{
			SetFullScreen(m_bPingPong);
			//用作全屏切换标识
			m_bPingPong = !m_bPingPong;
		}
		
	}
	
	}
	return lRes;
	
}

/************************************************************************/
/* 
功能：duilib自定义消息
*/
/************************************************************************/
void CDuiFrameDlg::Notify(TNotifyUI& msg)
{
	int index;
	

	if(msg.sType == DUI_MSGTYPE_ITEMACTIVATE)
	{
		CTreeViewUI* pTree = static_cast<CTreeViewUI*>(m_PaintManager.FindControl(_T("treePlaylist")));
		CTreeNodeUI* pTreePlayChannel = static_cast<CTreeNodeUI*>(m_PaintManager.FindControl(_T("ctnPlayChannel")));
		if (pTree && -1 != pTree->GetItemIndex(msg.pSender) && U_TAG_PLAYLIST == msg.pSender->GetTag())
		{
			index = pTree->GetItemIndex(msg.pSender);
			ShowPlayWnd(TRUE);
			m_myPlayer.Play(WstringToString(m_strFolderName + L"\\" + m_vcPlayFile[index]));

		   SetTimer(NULL, 1, 1000, TimeProc);
		
			
		}

	}
	__super::Notify(msg);
}

void CDuiFrameDlg::OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo)
{
	if (m_bIsFullScreen)
	{
		lpMinMaxInfo->ptMaxTrackSize.x = GetSystemMetrics(SM_CXSCREEN) + 2 * (GetSystemMetrics(SM_CXFIXEDFRAME) + GetSystemMetrics(SM_CXBORDER));
		lpMinMaxInfo->ptMaxTrackSize.y = GetSystemMetrics(SM_CYSCREEN) + 2 * (GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYBORDER));
	}
}


/************************************************************************/
/* 
功能：设置全屏
*/
/************************************************************************/
void CDuiFrameDlg::SetFullScreen(BOOL full)
{
	CControlUI* pbtnFull = m_PaintManager.FindControl(_T("btnScreenFull"));
	CControlUI* pbtnNormal = m_PaintManager.FindControl(_T("btnScreenNormal"));
	CControlUI* pbtnTitle = m_PaintManager.FindControl(_T("title"));
	CControlUI* pbtnPlayPanel = m_PaintManager.FindControl(_T("ctnPlayWnd"));
	
	int iBorderX = 0;
	int iBorderY = 0;

	if (pbtnFull && pbtnNormal)
	{
		m_bIsFullScreen = full;

		if (full)
		{
			::GetWindowPlacement(*this, &m_OldWndPlacement);
			if (::IsZoomed(*this))
			{
				::ShowWindow(*this, SW_SHOWDEFAULT);
			}

			::SetWindowPos(*this, HWND_TOPMOST, -iBorderX, -iBorderY, GetSystemMetrics(SM_CXSCREEN) + 2 * iBorderX, GetSystemMetrics(SM_CYSCREEN) + 2 * iBorderY, 0);
			ShowPlaylist(FALSE);
			//ShowCursor(FALSE);
		}
		else
		{
			::SetWindowPlacement(*this, &m_OldWndPlacement);
			::SetWindowPos(*this, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			ShowPlaylist(m_bIsShowPlaylist);
		}

		pbtnTitle->SetVisible(!full);
		pbtnNormal->SetVisible(full);
		pbtnFull->SetVisible(!full);
		pbtnPlayPanel->SetVisible(!full);
	}
}


LRESULT CDuiFrameDlg::OnPosChanged(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	CDuiString  strTime;
	struct tm   tmTotal, tmCurrent;
	time_t      timeTotal = m_myPlayer.GetTotalTime() / 1000;
	time_t      timeCurrent = m_myPlayer.GetTime() / 1000;
	TCHAR       szTotal[MAX_PATH], szCurrent[MAX_PATH];

	gmtime_s(&tmTotal, &timeTotal);
	gmtime_s(&tmCurrent, &timeCurrent);
	_tcsftime(szTotal, MAX_PATH, _T("%X"), &tmTotal);
	_tcsftime(szCurrent, MAX_PATH, _T("%X"), &tmCurrent);
	strTime.Format(_T("%s / %s"), szCurrent, szTotal);

	m_pLabelTime->SetText(strTime);
	m_pSliderPlay->SetValue(m_myPlayer.GetPos());
	return TRUE;
}


bool CDuiFrameDlg::OnPosChanged(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;

	if (pMsg->sType == _T("valuechanged"))
	{
		m_myPlayer.SeekTo((static_cast<CSliderUI*>(pMsg->pSender))->GetValue() + 1); // 获取的值少了1，导致设置的值也少了1，所以这里+1
	}

	return true;
}


bool CDuiFrameDlg::OnVolumeChanged(void* param)
{
	TNotifyUI* pMsg = (TNotifyUI*)param;

	if (pMsg->sType == _T("valuechanged"))
	{
		m_myPlayer.Volume((static_cast<CSliderUI*>(pMsg->pSender))->GetValue());
	}

	return true;
}

/************************************************************************/
/* 
功能：文件播放结束
*/
/************************************************************************/
LRESULT CDuiFrameDlg::OnEndReached(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	//Play(GetNextPath(true));
	if (!m_myPlayer.IsPlaying())
	{
		CControlUI *pbtnWndBkg = m_PaintManager.FindControl(L"MediaBkg");
		CControlUI *pbtnWnd = m_PaintManager.FindControl(L"WndMedia");
		pbtnWndBkg->SetVisible(TRUE);
		pbtnWnd->SetVisible(FALSE);
	}
	return TRUE;
}

//判断某个是否在给定矩形内
BOOL CDuiFrameDlg::IsPointAtRect(POINT p, int rcl, int rct, int rcr, int rcb)
{
	if ((rcl<p.x && p.x< rcr) && (rct<p.y && p.y<rcb))
	{
		return TRUE;
	}
	return FALSE;
}

/************************************************************************/
/* 
功能：当前获取鼠标点击在哪个播放窗口
*/
/************************************************************************/
int  CDuiFrameDlg::IsClickPlayWnd()
{
	POINT p;
	GetCursorPos(&p);
	RECT rect = { 0,0,0,0 };  
	GetWindowRect(this->GetHWND(), &rect);

	CControlUI* pbtnPlayPanel = m_PaintManager.FindControl(_T("ctnPlayWnd"));
	if (pbtnPlayPanel->IsVisible())
	{
		rect.bottom -= 75;
	}
	if (m_bIsShowPlaylist && !m_bIsFullScreen)
	{
		rect.right -= 225;
	}
	
	if (!m_bIsFullScreen)
	{
		rect.top += 30;
	}

	//每个播放窗口单元长宽
	int iPlayWndWidth = rect.right - rect.left;
	int iPlayWndHeight = rect.bottom - rect.top;

	//判断当前鼠标在哪个播放窗口
	if(IsPointAtRect(p, rect.left, rect.top, rect.left + iPlayWndWidth, rect.top + iPlayWndHeight ))
	{
		return TRUE;
	}
	

	return FALSE;
}

std::string WstringToString(const std::wstring wstr)
{// wstring转string
	std::string str;
	for (auto it = wstr.begin();it != wstr.end();++it)
	{
		str.push_back(static_cast<char>(*it));
	}
	return str;
}

LRESULT CDuiFrameDlg::OnPlaying(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
	return TRUE;
}

//定时器回调
void CALLBACK TimeProc(
	HWND hwnd,
	UINT message,
	UINT idTimer,
	DWORD dwTime)
{
	//WCHAR *strWindwoName = L"JCHPlayer";
	WCHAR *strWindwoName = L"JCHPlayer";
	HWND hNMMainWnd = FindWindowEx(NULL, NULL, NULL, strWindwoName);
	if (hNMMainWnd != NULL)
	{
		EnumChildWindows(hNMMainWnd, EnumerateVLC, NULL);
	}
}
//枚举VLC窗口
BOOL CALLBACK EnumerateVLC(HWND hWndvlc, LPARAM lParam) 
{
	TCHAR szWndTitle[1024];
	int nLen = GetWindowText(hWndvlc, szWndTitle, 1024);
	if (0!= nLen)
	{
		//禁用鼠标消息
		EnableWindow(hWndvlc, FALSE);
	}
	KillTimer(NULL, 1);
	return TRUE;
}


/************************************************************************/
/* 
功能:鼠标移动消息监听，主要用于全屏时隐藏播放面板
*/
/************************************************************************/
void CDuiFrameDlg::OnMouseMove()
{
	POINT p;
	GetCursorPos(&p);
	ScreenToClient(NULL, &p);
	RECT rect = { 0,0,0,0 }; 
	GetWindowRect(this->GetHWND(),&rect);
	if (m_bIsFullScreen)
	{
		//ShowCursor(TRUE);
		CControlUI* pbtnPlay = m_PaintManager.FindControl(_T("ctnPlayWnd"));
		if (p.y > (m_iMonitorHeight - 75))
		{
			pbtnPlay->SetVisible(TRUE);
		}
		else
		{
			pbtnPlay->SetVisible(FALSE);
		}
	}

}