#include "stdafx.h"
#include "mainwindow.h"
#include "openai.h"

#ifdef _DEBUG
#pragma comment(lib, "DuiLib_d.lib")
#else
#pragma comment(lib, "DuiLib.lib")
#endif // _DEBUG

#ifdef _DEBUG
#pragma comment(lib, "LibCurl_d.lib")
#else
#pragma comment(lib, "LibCurl.lib")
#endif // _DEBUG

#ifdef _DEBUG
#pragma comment(lib, "Cjson_d.lib")
#else
#pragma comment(lib, "Cjson.lib")
#endif // _DEBUG

#pragma comment(lib, "shlwapi.lib")

using namespace DuiLib;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	::CoInitialize(NULL);
	CPaintManagerUI::SetInstance(hInstance);

	OpenAI::InitOpenAI();

	MainWindow test;
	HWND mainwin_hwnd = test.Create(NULL, _T("MainWindow"), WS_OVERLAPPEDWINDOW, NULL);
	if (!mainwin_hwnd)
	{
		MessageBox(NULL, _T("窗口创建失败"), _T("警告"), MB_OK);

		return 1;
	}
	test.CenterWindow();
	test.ShowWindow(SW_SHOW);

	CPaintManagerUI::MessageLoop();
	::CoUninitialize();

	OpenAI::DeinitOpenAI();

	return 0;
}