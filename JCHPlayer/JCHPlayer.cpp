#include<windows.h>
#include<stdio.h>
#include "duilib/include/UIlib.h"

#include "vlc/vlc.h"

using namespace DuiLib;


#ifdef _DEBUG
#pragma comment(lib, "duilib/lib/DuiLib_ud.lib")
#else
#pragma comment(lib, "duilib/lib/DuiLib_u.lib")
#endif

#pragma comment(lib, "vlc/lib/libvlc.lib")
#pragma comment(lib, "vlc/lib/libvlccore.lib")

void play(HWND hw);

class CDuiFrameWnd : public CWindowWnd, public INotifyUI
{
public:
	virtual LPCTSTR GetWindowClassName() const { return _T("DUIMainFrame"); }
	virtual void    Notify(TNotifyUI& msg) 
	{
		if (msg.sType == _T("click"))
		{
			if (msg.pSender->GetName() == _T("play"))
			{
				play(m_hWnd);
			}
		}

	
	}

	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		LRESULT lRes = 0;

		if (uMsg == WM_CREATE)
		{
			CControlUI *pWnd = new CButtonUI;
			pWnd->SetName(_T("play"));
			pWnd->SetText(_T("Hello World"));   // 设置文字
			pWnd->SetBkColor(0xFFFFFFFF);       // 设置背景色

			m_PaintManager.Init(m_hWnd);
			m_PaintManager.AttachDialog(pWnd);
			m_PaintManager.AddNotifier(this);   // 添加控件等消息响应，这样消息就会传达到duilib的消息循环，我们可以在Notify函数里做消息处理

			return lRes;
		}

		// 以下3个消息WM_NCACTIVATE、WM_NCCALCSIZE、WM_NCPAINT用于屏蔽系统标题栏
		else if (uMsg == WM_NCACTIVATE)
		{
			if (!::IsIconic(m_hWnd))
			{
				return (wParam == 0) ? TRUE : FALSE;
			}
		}
		else if (uMsg == WM_NCCALCSIZE)
		{
			return 0;
		}
		else if (uMsg == WM_NCPAINT)
		{
			return 0;
		}

		if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
		{
			return lRes;
		}


		if (m_PaintManager.MessageHandler(uMsg, wParam, lParam, lRes))
		{
			return lRes;
		}

		return __super::HandleMessage(uMsg, wParam, lParam);
	}

protected:
	CPaintManagerUI m_PaintManager;
};


int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	CPaintManagerUI::SetInstance(hInstance);

	CDuiFrameWnd duiFrame;
	duiFrame.Create(NULL, _T("DUIWnd"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE);
	duiFrame.ShowModal();

	::CoUninitialize();


	return 0;
}

void play(HWND hw)
{
	libvlc_instance_t * inst;
	libvlc_media_player_t *mp;
	libvlc_media_t *m;

	inst = libvlc_new(0, NULL);        /* Load the VLC engine */
	m = libvlc_media_new_path(inst, "G:\\download\\video\\1.MP4");  // 必须为英文路径  
	mp = libvlc_media_player_new_from_media(m);    /* Create a media player playing environement */
	libvlc_media_release(m);           /* No need to keep the media now */
	libvlc_media_player_set_hwnd(mp, hw);

									   // 播放文件
	libvlc_media_player_play(mp);      /* play the media_player */
	Sleep(200 * 1000);                 /* Let it play a bit */

	libvlc_media_player_stop(mp);      /* Stop playing */
	libvlc_media_player_release(mp);   /* Free the media_player */
	libvlc_release(inst);
}