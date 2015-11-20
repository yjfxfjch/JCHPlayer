#include<windows.h>
#include "DuiFrameDlg.h"



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	::CoInitialize(NULL);
	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath() + _T("skin"));

	CDuiFrameDlg *pFrame = new CDuiFrameDlg(_T("Player.xml"));
	pFrame->Create(NULL, _T("JCHPlayer"), UI_WNDSTYLE_FRAME, WS_EX_WINDOWEDGE | WS_EX_ACCEPTFILES);
	pFrame->ShowModal();

	delete pFrame;
	::CoUninitialize();
	return 0;
}