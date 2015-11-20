#pragma once
#include "player/AVPlayer.h"
#include "Duilib.h"
#include <vector>
#include<iostream>

using namespace std;

const int MAX_PLANE = 1;

class CDuiFrameDlg : public CXMLWnd
{
public:
	explicit CDuiFrameDlg(LPCTSTR pszXMLName);
	~CDuiFrameDlg();


	DUI_DECLARE_MESSAGE_MAP()
	virtual void InitWindow();
	virtual CControlUI* CreateControl(LPCTSTR pstrClassName);
	virtual void Notify(TNotifyUI& msg);
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnClick(TNotifyUI& msg);
	void OnMouseMove();
	LRESULT OnPlaying(HWND hwnd, WPARAM wParam, LPARAM lParam);     // 文件头读取完毕，开始播放
	LRESULT OnPosChanged(HWND hwnd, WPARAM wParam, LPARAM lParam);  // 进度改变，播放器传回来的进度
	LRESULT OnEndReached(HWND hwnd, WPARAM wParam, LPARAM lParam);  // 文件播放完毕
	void OnGetMinMaxInfo(HWND hwnd, LPMINMAXINFO lpMinMaxInfo);
	bool    OnPosChanged(void* param);                              // 进度改变，用户主动改变进度
	bool    OnVolumeChanged(void* param);                           // 音量改变
	
private:
	void ShowPlayWnd(BOOL show);
	void OpenFolderDlg();
	void ShowPlaylist(BOOL show);
	void SetFullScreen(BOOL full);
	BOOL IsPointAtRect(POINT p, int rcl, int rct, int rcr, int rcb);
	BOOL AddPlayFile(WCHAR *folder);
	int  GetPlayerNum();
	int  MinUnPlayerNum();
	int  GetMouseClickPlayer();

private:
	vector<wstring>   m_vcPlayFile;			//播放文件
	wstring m_strFolderName;				//播放文件夹
	BOOL m_bPingPong;
	BOOL m_bIsFullScreen;					//全屏标志
	BOOL m_bIsShowPlaylist;					//播放列表显示标识
	int m_iMonitorWidth;					//显示器宽度
	int m_iMonitorHeight;
	CSliderUI       *m_pSliderPlay;			// 文件播放进度
	CLabelUI        *m_pLabelTime;			// 文件播放时间
	WINDOWPLACEMENT m_OldWndPlacement;		// 保存窗口原来的位置
	CAVPlayer m_myPlayer;		//	播放器
};